
INCLUDE(${NV_CMAKE_DIR}/DetermineProcessor.cmake)

# Set optimal options for gcc:
IF(CMAKE_COMPILER_IS_GNUCXX)

	IF(NV_SYSTEM_PROCESSOR STREQUAL "i586")
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=i586")
	ENDIF(NV_SYSTEM_PROCESSOR STREQUAL "i586")

	IF(NV_SYSTEM_PROCESSOR STREQUAL "i686")
		#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=i686")
		#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpmath=sse -mtune=i686 -msse3")
		#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=pentium4")
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=prescott")
	ENDIF(NV_SYSTEM_PROCESSOR STREQUAL "i686")

	IF(NV_SYSTEM_PROCESSOR STREQUAL "x86_64")
		#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=athlon64")
		#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=athlon64 -msse3")
	ENDIF(NV_SYSTEM_PROCESSOR STREQUAL "x86_64")

	IF(NV_SYSTEM_PROCESSOR STREQUAL "powerpc")
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcpu=powerpc -faltivec -maltivec -mabi=altivec -mpowerpc-gfxopt")
		
		# ibook G4:
		#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcpu=7450 -mtune=7450 -faltivec -maltivec -mabi=altivec -mpowerpc-gfxopt")

		# G5
		#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcpu=G5 -faltivec -maltivec -mabi=altivec -mpowerpc-gfxopt")

	ENDIF(NV_SYSTEM_PROCESSOR STREQUAL "powerpc")

#	IF(DARWIN)
#		SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk")
#		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk")
#	ENDIF(DARWIN)
	IF(APPLE)
		SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch i586 -arch x86_64 -msse3 -mmacosx-version-min=10.5")
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch i586 -arch x86_64 -msse3 -mmacosx-version-min=10.5")
	ENDIF(APPLE)

	IF(CMAKE_BUILD_TYPE STREQUAL "debug")
		ADD_DEFINITIONS(-D_DEBUG)
	ENDIF(CMAKE_BUILD_TYPE STREQUAL "debug")

	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

IF(MSVC)
	# @@ Some of these might only be available in VC8.
	# Code generation flags.
#	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /arch:SSE2 /fp:fast")
#	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE2 /fp:fast")

	# Optimization flags.
	SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} /O2 /Ob2 /Oi /Ot /Oy /GL")
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} /O2 /Ob2 /Oi /Ot /Oy /GL")
	SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
	SET(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /LTCG")
	SET(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS_RELEASE} /LTCG")

	# Definitions.
	ADD_DEFINITIONS(-D__SSE2__ -D__SSE__ -D__MMX__)
ENDIF(MSVC)
