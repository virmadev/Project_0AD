/* Copyright (C) 2019 Wildfire Games.
 * This file is part of 0 A.D.
 *
 * 0 A.D. is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * 0 A.D. is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with 0 A.D.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "precompiled.h"

#include "SavedGame.h"

#include "graphics/GameView.h"
#include "i18n/L10n.h"
#include "lib/allocators/shared_ptr.h"
#include "lib/file/archive/archive_zip.h"
#include "lib/utf8.h"
#include "maths/Vector3D.h"
#include "ps/CLogger.h"
#include "ps/Filesystem.h"
#include "ps/Game.h"
#include "ps/Mod.h"
#include "ps/Pyrogenesis.h"
#include "scriptinterface/ScriptInterface.h"
#include "simulation2/Simulation2.h"

// TODO: we ought to check version numbers when loading files

Status SavedGames::SavePrefix(const CStrW& prefix, const CStrW& description, CSimulation2& simulation, const shared_ptr<ScriptInterface::StructuredClone>& guiMetadataClone)
{
	// Determine the filename to save under
	const VfsPath basenameFormat(L"saves/" + prefix + L"-%04d");
	const VfsPath filenameFormat = basenameFormat.ChangeExtension(L".0adsave");
	VfsPath filename;

	// Don't make this a static global like NextNumberedFilename expects, because
	// that wouldn't work when 'prefix' changes, and because it's not thread-safe
	size_t nextSaveNumber = 0;
	vfs::NextNumberedFilename(g_VFS, filenameFormat, nextSaveNumber, filename);

	return Save(filename.Filename().string(), description, simulation, guiMetadataClone);
}

Status SavedGames::Save(const CStrW& name, const CStrW& description, CSimulation2& simulation, const shared_ptr<ScriptInterface::StructuredClone>& guiMetadataClone)
{
	JSContext* cx = simulation.GetScriptInterface().GetContext();
	JSAutoRequest rq(cx);

	// Determine the filename to save under
	const VfsPath basenameFormat(L"saves/" + name);
	const VfsPath filename = basenameFormat.ChangeExtension(L".0adsave");

	// ArchiveWriter_Zip can only write to OsPaths, not VfsPaths,
	// but we'd like to handle saved games via VFS.
	// To avoid potential confusion from writing with non-VFS then
	// reading the same file with VFS, we'll just write to a temporary
	// non-VFS path and then load and save again via VFS,
	// which is kind of a hack.

	OsPath tempSaveFileRealPath;
	WARN_RETURN_STATUS_IF_ERR(g_VFS->GetDirectoryRealPath("cache/", tempSaveFileRealPath));
	tempSaveFileRealPath = tempSaveFileRealPath / "temp.0adsave";

	time_t now = time(NULL);

	// Construct the serialized state to be saved

	std::stringstream simStateStream;
	if (!simulation.SerializeState(simStateStream))
		WARN_RETURN(ERR::FAIL);

	JS::RootedValue initAttributes(cx, simulation.GetInitAttributes());
	JS::RootedValue mods(cx, Mod::GetLoadedModsWithVersions(simulation.GetScriptInterface()));

	JS::RootedValue metadata(cx);

	ScriptInterface::CreateObject(
		cx,
		&metadata,
		"engine_version", engine_version,
		"time", static_cast<double>(now),
		"playerID", g_Game->GetPlayerID(),
		"mods", mods,
		"initAttributes", initAttributes);

	JS::RootedValue guiMetadata(cx);
	simulation.GetScriptInterface().ReadStructuredClone(guiMetadataClone, &guiMetadata);

	// get some camera data
	const CVector3D cameraPosition = g_Game->GetView()->GetCameraPosition();
	const CVector3D cameraRotation = g_Game->GetView()->GetCameraRotation();

	JS::RootedValue cameraMetadata(cx);

	ScriptInterface::CreateObject(
		cx,
		&cameraMetadata,
		"PosX", cameraPosition.X,
		"PosY", cameraPosition.Y,
		"PosZ", cameraPosition.Z,
		"RotX", cameraRotation.X,
		"RotY", cameraRotation.Y,
		"Zoom", g_Game->GetView()->GetCameraZoom());

	simulation.GetScriptInterface().SetProperty(guiMetadata, "camera", cameraMetadata);

	simulation.GetScriptInterface().SetProperty(metadata, "gui", guiMetadata);
	simulation.GetScriptInterface().SetProperty(metadata, "description", description);

	std::string metadataString = simulation.GetScriptInterface().StringifyJSON(&metadata, true);

	// Write the saved game as zip file containing the various components
	PIArchiveWriter archiveWriter = CreateArchiveWriter_Zip(tempSaveFileRealPath, false);
	if (!archiveWriter)
		WARN_RETURN(ERR::FAIL);

	WARN_RETURN_STATUS_IF_ERR(archiveWriter->AddMemory((const u8*)metadataString.c_str(), metadataString.length(), now, "metadata.json"));
	WARN_RETURN_STATUS_IF_ERR(archiveWriter->AddMemory((const u8*)simStateStream.str().c_str(), simStateStream.str().length(), now, "simulation.dat"));
	archiveWriter.reset(); // close the file

	WriteBuffer buffer;
	CFileInfo tempSaveFile;
	WARN_RETURN_STATUS_IF_ERR(GetFileInfo(tempSaveFileRealPath, &tempSaveFile));
	buffer.Reserve(tempSaveFile.Size());
	WARN_RETURN_STATUS_IF_ERR(io::Load(tempSaveFileRealPath, buffer.Data().get(), buffer.Size()));
	WARN_RETURN_STATUS_IF_ERR(g_VFS->CreateFile(filename, buffer.Data(), buffer.Size()));

	OsPath realPath;
	WARN_RETURN_STATUS_IF_ERR(g_VFS->GetRealPath(filename, realPath));
	LOGMESSAGERENDER(g_L10n.Translate("Saved game to '%s'"), realPath.string8());
	debug_printf("Saved game to '%s'\n", realPath.string8().c_str());

	return INFO::OK;
}

/**
 * Helper class for retrieving data from saved game archives
 */
class CGameLoader
{
	NONCOPYABLE(CGameLoader);
public:

	/**
	 * @param scriptInterface the ScriptInterface used for loading metadata.
	 * @param[out] savedState serialized simulation state stored as string of bytes,
	 *	loaded from simulation.dat inside the archive.
	 *
	 * Note: We use a different approach for returning the string and the metadata JS::Value.
	 * We use a pointer for the string to avoid copies (efficiency). We don't use this approach
	 * for the metadata because it would be error prone with rooting and the stack-based rooting
	 * types and confusing (a chain of pointers pointing to other pointers).
	 */
	CGameLoader(const ScriptInterface& scriptInterface, std::string* savedState) :
		m_ScriptInterface(scriptInterface),
		m_Metadata(scriptInterface.GetJSRuntime()),
		m_SavedState(savedState)
	{
	}

	static void ReadEntryCallback(const VfsPath& pathname, const CFileInfo& fileInfo, PIArchiveFile archiveFile, uintptr_t cbData)
	{
		((CGameLoader*)cbData)->ReadEntry(pathname, fileInfo, archiveFile);
	}

	void ReadEntry(const VfsPath& pathname, const CFileInfo& fileInfo, PIArchiveFile archiveFile)
	{
		JSContext* cx = m_ScriptInterface.GetContext();
		JSAutoRequest rq(cx);

		if (pathname == L"metadata.json")
		{
			std::string buffer;
			buffer.resize(fileInfo.Size());
			WARN_IF_ERR(archiveFile->Load("", DummySharedPtr((u8*)buffer.data()), buffer.size()));
			m_ScriptInterface.ParseJSON(buffer, &m_Metadata);
		}
		else if (pathname == L"simulation.dat" && m_SavedState)
		{
			m_SavedState->resize(fileInfo.Size());
			WARN_IF_ERR(archiveFile->Load("", DummySharedPtr((u8*)m_SavedState->data()), m_SavedState->size()));
		}
	}

	JS::Value GetMetadata()
	{
		return m_Metadata.get();
	}

private:

	const ScriptInterface& m_ScriptInterface;
	JS::PersistentRooted<JS::Value> m_Metadata;
	std::string* m_SavedState;
};

Status SavedGames::Load(const std::wstring& name, const ScriptInterface& scriptInterface, JS::MutableHandleValue metadata, std::string& savedState)
{
	// Determine the filename to load
	const VfsPath basename(L"saves/" + name);
	const VfsPath filename = basename.ChangeExtension(L".0adsave");

	// Don't crash just because file isn't found, this can happen if the file is deleted from the OS
	if (!VfsFileExists(filename))
		return ERR::FILE_NOT_FOUND;

	OsPath realPath;
	WARN_RETURN_STATUS_IF_ERR(g_VFS->GetRealPath(filename, realPath));

	PIArchiveReader archiveReader = CreateArchiveReader_Zip(realPath);
	if (!archiveReader)
		WARN_RETURN(ERR::FAIL);

	CGameLoader loader(scriptInterface, &savedState);
	WARN_RETURN_STATUS_IF_ERR(archiveReader->ReadEntries(CGameLoader::ReadEntryCallback, (uintptr_t)&loader));
	metadata.set(loader.GetMetadata());

	return INFO::OK;
}

JS::Value SavedGames::GetSavedGames(const ScriptInterface& scriptInterface)
{
	TIMER(L"GetSavedGames");
	JSContext* cx = scriptInterface.GetContext();
	JSAutoRequest rq(cx);

	JS::RootedValue games(cx);
	ScriptInterface::CreateArray(cx, &games);

	Status err;

	VfsPaths pathnames;
	err = vfs::GetPathnames(g_VFS, "saves/", L"*.0adsave", pathnames);
	WARN_IF_ERR(err);

	for (size_t i = 0; i < pathnames.size(); ++i)
	{
		OsPath realPath;
		err = g_VFS->GetRealPath(pathnames[i], realPath);
		if (err < 0)
		{
			DEBUG_WARN_ERR(err);
			continue; // skip this file
		}

		PIArchiveReader archiveReader = CreateArchiveReader_Zip(realPath);
		if (!archiveReader)
		{
			// Triggered by e.g. the file being open in another program
			LOGWARNING("Failed to read saved game '%s'", realPath.string8());
			continue; // skip this file
		}

		CGameLoader loader(scriptInterface, NULL);
		err = archiveReader->ReadEntries(CGameLoader::ReadEntryCallback, (uintptr_t)&loader);
		if (err < 0)
		{
			DEBUG_WARN_ERR(err);
			continue; // skip this file
		}
		JS::RootedValue metadata(cx, loader.GetMetadata());

		JS::RootedValue game(cx);
		ScriptInterface::CreateObject(
			cx,
			&game,
			"id", pathnames[i].Basename(),
			"metadata", metadata);

		scriptInterface.SetPropertyInt(games, i, game);
	}

	return games;
}

bool SavedGames::DeleteSavedGame(const std::wstring& name)
{
	const VfsPath basename(L"saves/" + name);
	const VfsPath filename = basename.ChangeExtension(L".0adsave");
	OsPath realpath;

	// Make sure it exists in VFS and find its real path
	if (!VfsFileExists(filename) || g_VFS->GetRealPath(filename, realpath) != INFO::OK)
		return false; // Error

	// Remove from VFS
	if (g_VFS->RemoveFile(filename) != INFO::OK)
		return false; // Error

	// Delete actual file
	if (wunlink(realpath) != 0)
		return false; // Error

	// Successfully deleted file
	return true;
}
