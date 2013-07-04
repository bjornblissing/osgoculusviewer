CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(OsgOculusViewer)

SET(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" cmake)

SET(NGAST_MAJOR_VERSION 1)
SET(NGAST_MINOR_VERSION 0)
SET(NGAST_REVISION_VERSION 0)

# Show relative paths options
OPTION(CMAKE_USE_RELATIVE_PATHS "If true, cmake will use relative paths in makefiles and projects." ON)

IF (WIN32)
	# Where to find OSG directory
	SET(OSG_THIRD_PARTY_DIR CACHE PATH "Path where to find the osg third party dependencies")
	IF(NOT OSG_THIRD_PARTY_DIR)
		MESSAGE(FATAL_ERROR "Error: OpenSceneGraph 3rd Party Directory not found.")
	ENDIF(NOT OSG_THIRD_PARTY_DIR)
ENDIF(WIN32)

# Oculus SDK - LibOVR
SET(OCULUS_SDK_ROOT CACHE PATH "Path where to find Oculus SDK installation ie. the OculusSDK folder")

#Solution
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

#######################################
# Library files
#######################################
FIND_PACKAGE( OpenGL REQUIRED )
FIND_PACKAGE( OpenSceneGraph REQUIRED osgViewer osgDB osgGA)
FIND_PACKAGE( OculusSDK REQUIRED )


INCLUDE_DIRECTORIES(BEFORE
    ${OPENGL_INCLUDE_DIR}
    ${OPENSCENEGRAPH_INCLUDE_DIR}
    ${OculusSDK_INCLUDE_DIRS}
)

###############################################################################
# Compiler Options
################################################################################
IF(WIN32)
	# Make sure we use minimal windows.h library without min max macros
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D \"WIN32_LEAN_AND_MEAN\"")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D \"NOMINMAX\"")
	
	# Use Link Time Code Generation
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LTCG")
	
    IF(MSVC80)
		# Detech 64-bit portability Issues
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Wp64")
    ENDIF(MSVC80)
	
	OPTION(MULTI_PROCESSOR_COMPILATION "Use multiple processors when compiling" ON)
	IF(MULTI_PROCESSOR_COMPILATION)
		# Set multi processor build
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
	ENDIF(MULTI_PROCESSOR_COMPILATION)
	# Set warnings to level 4 
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	IF (WARNINGS_AS_ERRORS)
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
	ENDIF (WARNINGS_AS_ERRORS)
ENDIF(WIN32)

IF(UNIX)
	IF(CMAKE_COMPILER_IS_GNUCC)
		ADD_DEFINITIONS(-Wall -ansi -pedantic -Wextra)
		IF (WARNINGS_AS_ERRORS)
			ADD_DEFINITIONS(-Werror)		
		ENDIF(WARNINGS_AS_ERRORS)
	ENDIF(CMAKE_COMPILER_IS_GNUCC)
ENDIF(UNIX)

###############################################################################
# Compile subdirectory
################################################################################
ADD_SUBDIRECTORY(src)