/* Copyright (C) 2017 Wildfire Games.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "precompiled.h"
#include "lib/file/vfs/vfs.h"

#include "lib/allocators/shared_ptr.h"
#include "lib/file/file_system.h"
#include "lib/file/common/file_stats.h"
#include "lib/file/common/trace.h"
#include "lib/file/archive/archive.h"
#include "lib/file/io/io.h"
#include "lib/file/vfs/vfs_tree.h"
#include "lib/file/vfs/vfs_lookup.h"
#include "lib/file/vfs/vfs_populate.h"

#include <mutex>
#include <thread>

static const StatusDefinition vfsStatusDefinitions[] = {
	{ ERR::VFS_DIR_NOT_FOUND, L"VFS directory not found" },
	{ ERR::VFS_FILE_NOT_FOUND, L"VFS file not found" },
	{ ERR::VFS_ALREADY_MOUNTED, L"VFS path already mounted" }
};
STATUS_ADD_DEFINITIONS(vfsStatusDefinitions);

static std::mutex vfs_mutex;

class VFS : public IVFS
{
public:
	VFS() : m_trace(CreateDummyTrace(8*MiB))
	{
	}

	virtual Status Mount(const VfsPath& mountPoint, const OsPath& path, size_t flags /* = 0 */, size_t priority /* = 0 */)
	{
		std::lock_guard<std::mutex> lock(vfs_mutex);
		if(!DirectoryExists(path))
		{
			if(flags & VFS_MOUNT_MUST_EXIST)
				return ERR::VFS_DIR_NOT_FOUND;	// NOWARN
			else
				RETURN_STATUS_IF_ERR(CreateDirectories(path, 0700));
		}

		VfsDirectory* directory;
		WARN_RETURN_STATUS_IF_ERR(vfs_Lookup(mountPoint, &m_rootDirectory, directory, 0, VFS_LOOKUP_ADD|VFS_LOOKUP_SKIP_POPULATE));

		PRealDirectory realDirectory(new RealDirectory(path, priority, flags));
		RETURN_STATUS_IF_ERR(vfs_Attach(directory, realDirectory));
		return INFO::OK;
	}

	virtual Status GetFileInfo(const VfsPath& pathname, CFileInfo* pfileInfo) const
	{
		std::lock_guard<std::mutex> lock(vfs_mutex);
		VfsDirectory* directory;
		VfsFile* file;

		Status ret = vfs_Lookup(pathname, &m_rootDirectory, directory, &file);
		if(!pfileInfo)	// just indicate if the file exists without raising warnings.
			return ret;
		WARN_RETURN_STATUS_IF_ERR(ret);
		*pfileInfo = CFileInfo(file->Name(), file->Size(), file->MTime());
		return INFO::OK;
	}

	virtual Status GetFilePriority(const VfsPath& pathname, size_t* ppriority) const
	{
		std::lock_guard<std::mutex> lock(vfs_mutex);
		VfsDirectory* directory; VfsFile* file;
		RETURN_STATUS_IF_ERR(vfs_Lookup(pathname, &m_rootDirectory, directory, &file));
		*ppriority = file->Priority();
		return INFO::OK;
	}

	virtual Status GetDirectoryEntries(const VfsPath& path, CFileInfos* fileInfos, DirectoryNames* subdirectoryNames) const
	{
		std::lock_guard<std::mutex> lock(vfs_mutex);
		VfsDirectory* directory;
		RETURN_STATUS_IF_ERR(vfs_Lookup(path, &m_rootDirectory, directory, 0));

		if(fileInfos)
		{
			const VfsDirectory::VfsFiles& files = directory->Files();
			fileInfos->clear();
			fileInfos->reserve(files.size());
			for(VfsDirectory::VfsFiles::const_iterator it = files.begin(); it != files.end(); ++it)
			{
				const VfsFile& file = it->second;
				fileInfos->push_back(CFileInfo(file.Name(), file.Size(), file.MTime()));
			}
		}

		if(subdirectoryNames)
		{
			const VfsDirectory::VfsSubdirectories& subdirectories = directory->Subdirectories();
			subdirectoryNames->clear();
			subdirectoryNames->reserve(subdirectories.size());
			for(VfsDirectory::VfsSubdirectories::const_iterator it = subdirectories.begin(); it != subdirectories.end(); ++it)
				subdirectoryNames->push_back(it->first);
		}

		return INFO::OK;
	}

	virtual Status CreateFile(const VfsPath& pathname, const shared_ptr<u8>& fileContents, size_t size)
	{
		std::lock_guard<std::mutex> lock(vfs_mutex);
		VfsDirectory* directory;
		Status st;
		st = vfs_Lookup(pathname, &m_rootDirectory, directory, 0, VFS_LOOKUP_ADD|VFS_LOOKUP_CREATE|VFS_LOOKUP_CREATE_ALWAYS);
		if (st == ERR::FILE_ACCESS)
			return ERR::FILE_ACCESS;

		WARN_RETURN_STATUS_IF_ERR(st);

		const PRealDirectory& realDirectory = directory->AssociatedDirectory();
		const OsPath name = pathname.Filename();
		RETURN_STATUS_IF_ERR(realDirectory->Store(name, fileContents, size));

		const VfsFile file(name, size, time(0), realDirectory->Priority(), realDirectory);
		directory->AddFile(file);

		m_trace->NotifyStore(pathname, size);
		return INFO::OK;
	}

	virtual Status ReplaceFile(const VfsPath& pathname, const shared_ptr<u8>& fileContents, size_t size)
	{
		std::unique_lock<std::mutex> lock(vfs_mutex);
		VfsDirectory* directory;
		VfsFile* file;
		Status st;
		st = vfs_Lookup(pathname, &m_rootDirectory, directory, &file, VFS_LOOKUP_ADD|VFS_LOOKUP_CREATE);

		// There is no such file, create it.
		if (st == ERR::VFS_FILE_NOT_FOUND)
		{
			lock.unlock();
			return CreateFile(pathname, fileContents, size);
		}

		WARN_RETURN_STATUS_IF_ERR(st);

		RealDirectory realDirectory(file->Loader()->Path(), file->Priority(), directory->AssociatedDirectory()->Flags());
		RETURN_STATUS_IF_ERR(realDirectory.Store(pathname.Filename(), fileContents, size));

		directory->AddFile(*file);

		m_trace->NotifyStore(pathname, size);
		return INFO::OK;
	}

	virtual Status LoadFile(const VfsPath& pathname, shared_ptr<u8>& fileContents, size_t& size)
	{
		std::lock_guard<std::mutex> lock(vfs_mutex);

		VfsDirectory* directory; VfsFile* file;
		// per 2010-05-01 meeting, this shouldn't raise 'scary error
		// dialogs', which might fail to display the culprit pathname
		// instead, callers should log the error, including pathname.
		RETURN_STATUS_IF_ERR(vfs_Lookup(pathname, &m_rootDirectory, directory, &file));

		fileContents = DummySharedPtr((u8*)0);
		size = file->Size();

		RETURN_STATUS_IF_ERR(AllocateAligned(fileContents, size, maxSectorSize));
		RETURN_STATUS_IF_ERR(file->Loader()->Load(file->Name(), fileContents, file->Size()));

		stats_io_user_request(size);
		m_trace->NotifyLoad(pathname, size);

		return INFO::OK;
	}

	virtual std::wstring TextRepresentation() const
	{
		std::lock_guard<std::mutex> lock(vfs_mutex);
		std::wstring textRepresentation;
		textRepresentation.reserve(100*KiB);
		DirectoryDescriptionR(textRepresentation, m_rootDirectory, 0);
		return textRepresentation;
	}

	virtual Status GetRealPath(const VfsPath& pathname, OsPath& realPathname)
	{
		std::lock_guard<std::mutex> lock(vfs_mutex);
		VfsDirectory* directory; VfsFile* file;
		WARN_RETURN_STATUS_IF_ERR(vfs_Lookup(pathname, &m_rootDirectory, directory, &file));
		realPathname = file->Loader()->Path() / pathname.Filename();
		return INFO::OK;
	}

	virtual Status GetDirectoryRealPath(const VfsPath& pathname, OsPath& realPathname)
	{
		std::lock_guard<std::mutex> lock(vfs_mutex);
		VfsDirectory* directory;
		WARN_RETURN_STATUS_IF_ERR(vfs_Lookup(pathname, &m_rootDirectory, directory, NULL));
		realPathname = directory->AssociatedDirectory()->Path();
		return INFO::OK;
	}

	virtual Status GetVirtualPath(const OsPath& realPathname, VfsPath& pathname)
	{
		std::lock_guard<std::mutex> lock(vfs_mutex);
		const OsPath realPath = realPathname.Parent()/"";
		VfsPath path;
		RETURN_STATUS_IF_ERR(FindRealPathR(realPath, m_rootDirectory, L"", path));
		pathname = path / realPathname.Filename();
		return INFO::OK;
	}

	virtual Status RemoveFile(const VfsPath& pathname)
	{
		std::lock_guard<std::mutex> lock(vfs_mutex);

		VfsDirectory* directory; VfsFile* file;
		RETURN_STATUS_IF_ERR(vfs_Lookup(pathname, &m_rootDirectory, directory, &file));
		directory->RemoveFile(file->Name());

		return INFO::OK;
	}

	virtual Status RepopulateDirectory(const VfsPath& path)
	{
		std::lock_guard<std::mutex> lock(vfs_mutex);

		VfsDirectory* directory;
		RETURN_STATUS_IF_ERR(vfs_Lookup(path, &m_rootDirectory, directory, 0));
		directory->RequestRepopulate();

		return INFO::OK;
	}

	virtual void Clear()
	{
		std::lock_guard<std::mutex> lock(vfs_mutex);
		m_rootDirectory.Clear();
	}

private:
	Status FindRealPathR(const OsPath& realPath, const VfsDirectory& directory, const VfsPath& curPath, VfsPath& path)
	{
		PRealDirectory realDirectory = directory.AssociatedDirectory();
		if(realDirectory && realDirectory->Path() == realPath)
		{
			path = curPath;
			return INFO::OK;
		}

		const VfsDirectory::VfsSubdirectories& subdirectories = directory.Subdirectories();
		for(VfsDirectory::VfsSubdirectories::const_iterator it = subdirectories.begin(); it != subdirectories.end(); ++it)
		{
			const OsPath& subdirectoryName = it->first;
			const VfsDirectory& subdirectory = it->second;
			Status ret = FindRealPathR(realPath, subdirectory, curPath / subdirectoryName/"", path);
			if(ret == INFO::OK)
				return INFO::OK;
		}

		return ERR::PATH_NOT_FOUND;	// NOWARN
	}

	PITrace m_trace;
	mutable VfsDirectory m_rootDirectory;
};

//-----------------------------------------------------------------------------

PIVFS CreateVfs()
{
	return PIVFS(new VFS());
}
