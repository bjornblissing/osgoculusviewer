# - Find OculusSDK
# Find the native OculusSDK headers and libraries.
#
#  OCULUS_SDK_INCLUDE_DIRS - where to find OVR.h, etc.
#  OCULUS_SDK_LIBRARIES    - List of libraries when using OculusSDK.
#  OCULUS_SDK_FOUND        - True if OculusSDK found.

IF (DEFINED ENV{OCULUS_SDK_ROOT_DIR})
    SET(OCULUS_SDK_ROOT_DIR "$ENV{OCULUS_SDK_ROOT_DIR}")
ENDIF()
SET(OCULUS_SDK_ROOT_DIR
    "${OCULUS_SDK_ROOT_DIR}"
    CACHE
    PATH
    "Root directory to search for OculusSDK")

# Look for the header file.
FIND_PATH(OCULUS_SDK_INCLUDE_DIRS NAMES OVR.h HINTS 
	${OCULUS_SDK_ROOT_DIR}/LibOVR/Include )

# Determine architecture
IF(CMAKE_SIZEOF_VOID_P MATCHES "8")
	IF(UNIX)
		SET(OCULUS_SDK_LIB_ARCH "x86_64" CACHE STRING "library location")
	ENDIF()
	IF(MSVC)
		SET(OCULUS_SDK_LIB_ARCH "x64" CACHE STRING "library location")
	ENDIF()
ELSE()
	IF(UNIX)
		SET(OCULUS_SDK_LIB_ARCH "i386" CACHE STRING "library location")
	ENDIF()
	IF(MSVC)
		SET(OCULUS_SDK_LIB_ARCH "Win32" CACHE STRING "library location")
	ENDIF()
ENDIF()

MARK_AS_ADVANCED(OCULUS_SDK_LIB_ARCH)

# Append "d" to debug libs on windows platform
IF (WIN32)
	SET(CMAKE_DEBUG_POSTFIX d)
ENDIF()

# Determine the compiler version for Visual Studio
IF (MSVC)
	# Visual Studio 2010
	IF(MSVC10)
		SET(OCULUS_MSVC_DIR "VS2010" CACHE STRING "library location")
	ENDIF()
	# Visual Studio 2012
	IF(MSVC11)
		SET(OCULUS_MSVC_DIR "VS2012" CACHE STRING "library location")
	ENDIF()
	# Visual Studio 2013
	IF(MSVC12)
		SET(OCULUS_MSVC_DIR "VS2013" CACHE STRING "library location")
	ENDIF()
ENDIF()
MARK_AS_ADVANCED(OCULUS_MSVC_DIR)

# Look for the library.
FIND_LIBRARY(OCULUS_SDK_LIBRARY NAMES libovr libovr64 ovr HINTS ${OCULUS_SDK_ROOT_DIR} 
                                                      ${OCULUS_SDK_ROOT_DIR}/LibOVR/Lib/${OCULUS_SDK_LIB_ARCH}/${OCULUS_MSVC_DIR}
                                                      ${OCULUS_SDK_ROOT_DIR}/LibOVR/Lib/Linux/Release/${OCULUS_SDK_LIB_ARCH}
                                                    )

# This will find release lib on Linux if no debug is available - on Linux this is no problem and avoids 
# having to compile in debug when not needed
FIND_LIBRARY(OCULUS_SDK_LIBRARY_DEBUG NAMES libovr${CMAKE_DEBUG_POSTFIX} libovr64${CMAKE_DEBUG_POSTFIX} ovr${CMAKE_DEBUG_POSTFIX} ovr HINTS 
                                                      ${OCULUS_SDK_ROOT_DIR}/LibOVR/Lib/${OCULUS_SDK_LIB_ARCH}/${OCULUS_MSVC_DIR}
                                                      ${OCULUS_SDK_ROOT_DIR}/LibOVR/Lib/Linux/Debug/${OCULUS_SDK_LIB_ARCH}
                                                      ${OCULUS_SDK_ROOT_DIR}/LibOVR/Lib/Linux/Release/${OCULUS_SDK_LIB_ARCH}
                                                    )
    
MARK_AS_ADVANCED(OCULUS_SDK_LIBRARY)
MARK_AS_ADVANCED(OCULUS_SDK_LIBRARY_DEBUG)

SET(OCULUS_SDK_LIBRARIES optimized ${OCULUS_SDK_LIBRARY} debug ${OCULUS_SDK_LIBRARY_DEBUG})

# handle the QUIETLY and REQUIRED arguments and set OCULUS_SDK_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OculusSDK DEFAULT_MSG OCULUS_SDK_LIBRARIES OCULUS_SDK_INCLUDE_DIRS)

MARK_AS_ADVANCED(OCULUS_SDK_LIBRARIES OCULUS_SDK_INCLUDE_DIRS)
