PROJECT(nvthread)

SET(THREAD_SRCS
	nvthread.h nvthread.cpp
	Atomic.h
	Event.h Event.cpp
	Mutex.h Mutex.cpp
	ParallelFor.h ParallelFor.cpp
	Thread.h Thread.cpp
	ThreadPool.h ThreadPool.cpp)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

# targets
ADD_DEFINITIONS(-DNVTHREAD_EXPORTS)

IF(NVTHREAD_SHARED)
	ADD_LIBRARY(nvthread SHARED ${THREAD_SRCS})
ELSE(NVTHREAD_SHARED)
	ADD_LIBRARY(nvthread ${THREAD_SRCS})
ENDIF(NVTHREAD_SHARED)

TARGET_LINK_LIBRARIES(nvthread ${LIBS} nvcore)

INSTALL(TARGETS nvthread
	RUNTIME DESTINATION ${BINDIR}
	LIBRARY DESTINATION ${LIBDIR}
	ARCHIVE DESTINATION ${LIBDIR})
