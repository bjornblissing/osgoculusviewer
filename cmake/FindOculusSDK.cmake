# - Find OculusSDK
# Find the native OculusSDK headers and libraries.
#
#  OculusSDK_INCLUDE_DIRS - where to find OVR.h, etc.
#  OculusSDK_LIBRARIES    - List of libraries when using OculusSDK.
#  OculusSDK_FOUND        - True if OculusSDK found.

set(OculusSDK_ROOT_DIR
    "${OculusSDK_ROOT_DIR}"
    CACHE
    PATH
    "Root directory to search for OculusSDK")

# Look for the header file.
FIND_PATH(OculusSDK_INCLUDE_DIRS NAMES OVR.h HINTS 
	${OculusSDK_ROOT_DIR}/LibOVR/Include )

# determine architecture
IF(CMAKE_SIZEOF_VOID_P MATCHES "8")
    SET(LIB_ARCH "x86_64" CACHE STRING "library location")
ELSE()
    SET(LIB_ARCH "i386" CACHE STRING "library location")
ENDIF()
MARK_AS_ADVANCED(LIB_ARCH)

# Look for the library.
FIND_LIBRARY(OculusSDK_RLIBRARY NAMES libovr ovr HINTS ${OculusSDK_ROOT_DIR} 
                                                      ${OculusSDK_ROOT_DIR}/LibOVR/Lib/Win32
                                                      ${OculusSDK_ROOT_DIR}/LibOVR/Lib/Linux/Release/${LIB_ARCH}
                                                    )

# Tthis will find release lib on Linux if no debug is available - on Linux this is no problem and avoids 
# having to compile in debug when not needed
FIND_LIBRARY(OculusSDK_DLIBRARY NAMES libovr${CMAKE_DEBUG_POSTFIX} ovr${CMAKE_DEBUG_POSTFIX} ovr HINTS 
                                                      ${OculusSDK_ROOT_DIR}/LibOVR/Lib/Win32
                                                      ${OculusSDK_ROOT_DIR}/LibOVR/Lib/Linux/Debug/${LIB_ARCH}
                                                      ${OculusSDK_ROOT_DIR}/LibOVR/Lib/Linux/Release/${LIB_ARCH}
                                                    )
    
MARK_AS_ADVANCED(OculusSDK_RLIBRARY)
MARK_AS_ADVANCED(OculusSDK_DLIBRARY)

SET(OculusSDK_LIBRARIES optimized ${OculusSDK_RLIBRARY} debug ${OculusSDK_DLIBRARY})

# handle the QUIETLY and REQUIRED arguments and set OculusSDK_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OculusSDK DEFAULT_MSG OculusSDK_LIBRARIES OculusSDK_INCLUDE_DIRS)

mark_as_advanced(OculusSDK_LIBRARIES OculusSDK_INCLUDE_DIRS)
