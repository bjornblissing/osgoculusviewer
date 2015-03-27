# - Find OculusSDK
# Find the native OculusSDK headers and libraries.
#
#  OCULUS_SDK_INCLUDE_DIRS - where to find OVR.h, etc.
#  OCULUS_SDK_LIBRARIES    - List of libraries when using OculusSDK.
#  OCULUS_SDK_FOUND        - True if OculusSDK found.
#  OCULUS_SDK_VERSION      - Version of the OculusSDK if found

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
		SET(_OCULUS_SDK_LIB_ARCH "x86_64")
	ENDIF()
	IF(MSVC)
		SET(_OCULUS_SDK_LIB_ARCH "x64")
	ENDIF()
ELSE()
	IF(UNIX)
		SET(_OCULUS_SDK_LIB_ARCH "i386")
	ENDIF()
	IF(MSVC)
		SET(_OCULUS_SDK_LIB_ARCH "Win32")
	ENDIF()
ENDIF()

MARK_AS_ADVANCED(_OCULUS_SDK_LIB_ARCH)

# Append "d" to debug libs on windows platform
IF (WIN32)
	SET(CMAKE_DEBUG_POSTFIX d)
ENDIF()

# Determine the compiler version for Visual Studio
IF (MSVC)
	# Visual Studio 2010
	IF(MSVC10)
		SET(_OCULUS_MSVC_DIR "VS2010")
	ENDIF()
	# Visual Studio 2012
	IF(MSVC11)
		SET(_OCULUS_MSVC_DIR "VS2012")
	ENDIF()
	# Visual Studio 2013
	IF(MSVC12)
		SET(_OCULUS_MSVC_DIR "VS2013")
	ENDIF()
ENDIF()

# Try to ascertain the version of the SDK
IF(OCULUS_SDK_INCLUDE_DIRS)
	SET(_OCULUS_VERSION_FILE "${OCULUS_SDK_INCLUDE_DIRS}/OVR_Version.h")
	
	IF(EXISTS "${_OCULUS_VERSION_FILE}") 
		FILE(STRINGS "${_OCULUS_VERSION_FILE}" _OCULUS_VERSION_FILE_CONTENTS REGEX "#define OVR_[A-Z]+_VERSION[ \t]+[0-9]+")

		STRING(REGEX REPLACE ".*#define OVR_PRODUCT_VERSION[ \t]+([0-9]+).*" "\\1" OCULUS_SDK_VERSION_PRODUCT ${_OCULUS_VERSION_FILE_CONTENTS})
		STRING(REGEX REPLACE ".*#define OVR_MAJOR_VERSION[ \t]+([0-9]+).*" "\\1" OCULUS_SDK_VERSION_MAJOR ${_OCULUS_VERSION_FILE_CONTENTS})
		STRING(REGEX REPLACE ".*#define OVR_MINOR_VERSION[ \t]+([0-9]+).*" "\\1" OCULUS_SDK_VERSION_MINOR ${_OCULUS_VERSION_FILE_CONTENTS})
		STRING(REGEX REPLACE ".*#define OVR_PATCH_VERSION[ \t]+([0-9]+).*" "\\1" OCULUS_SDK_VERSION_PATCH ${_OCULUS_VERSION_FILE_CONTENTS})
		
		SET(OCULUS_SDK_VERSION "${OCULUS_SDK_VERSION_PRODUCT}.${OCULUS_SDK_VERSION_MAJOR}.${OCULUS_SDK_VERSION_MINOR}.${OCULUS_SDK_VERSION_PATCH}" CACHE INTERNAL "The version of Oculus SDK which was detected")
	ENDIF()
ENDIF()

# Locate Oculus license file
SET(_OCULUS_SDK_LICENSE_FILE "${OCULUS_SDK_ROOT_DIR}/LICENSE.txt")
IF(EXISTS "${_OCULUS_SDK_LICENSE_FILE}") 
	SET(OCULUS_SDK_LICENSE_FILE "${_OCULUS_SDK_LICENSE_FILE}" CACHE INTERNAL "The location of the Oculus SDK license file")
ENDIF()

# Look for the library.
FIND_LIBRARY(OCULUS_SDK_LIBRARY NAMES libovr libovr64 ovr HINTS ${OCULUS_SDK_ROOT_DIR} 
                                                      ${OCULUS_SDK_ROOT_DIR}/LibOVR/Lib/Windows/${_OCULUS_SDK_LIB_ARCH}/Release/${_OCULUS_MSVC_DIR}
                                                      ${OCULUS_SDK_ROOT_DIR}/LibOVR/Lib/Mac/Release
                                                      ${OCULUS_SDK_ROOT_DIR}/LibOVR/Lib/Linux/Release/${_OCULUS_SDK_LIB_ARCH}
                                                    )

# This will find release lib on Linux if no debug is available - on Linux this is no problem and avoids 
# having to compile in debug when not needed
FIND_LIBRARY(OCULUS_SDK_LIBRARY_DEBUG NAMES libovr${CMAKE_DEBUG_POSTFIX} libovr64${CMAKE_DEBUG_POSTFIX} ovr${CMAKE_DEBUG_POSTFIX} ovr libovr HINTS 
                                                      ${OCULUS_SDK_ROOT_DIR}/LibOVR/Lib/Windows/${_OCULUS_SDK_LIB_ARCH}/Debug/${_OCULUS_MSVC_DIR}
                                                      ${OCULUS_SDK_ROOT_DIR}/LibOVR/Lib/Mac/Debug
                                                      ${OCULUS_SDK_ROOT_DIR}/LibOVR/Lib/Mac/Release
                                                      ${OCULUS_SDK_ROOT_DIR}/LibOVR/Lib/Linux/Debug/${_OCULUS_SDK_LIB_ARCH}
                                                      ${OCULUS_SDK_ROOT_DIR}/LibOVR/Lib/Linux/Release/${_OCULUS_SDK_LIB_ARCH}
                                                    )
    
MARK_AS_ADVANCED(OCULUS_SDK_LIBRARY)
MARK_AS_ADVANCED(OCULUS_SDK_LIBRARY_DEBUG)

SET(OCULUS_SDK_LIBRARIES optimized ${OCULUS_SDK_LIBRARY} debug ${OCULUS_SDK_LIBRARY_DEBUG})

# handle the QUIETLY and REQUIRED arguments and set OCULUS_SDK_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OculusSDK DEFAULT_MSG OCULUS_SDK_LIBRARIES OCULUS_SDK_INCLUDE_DIRS)

MARK_AS_ADVANCED(OCULUS_SDK_LIBRARIES OCULUS_SDK_INCLUDE_DIRS)
