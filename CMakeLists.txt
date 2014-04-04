CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
SET(CMAKE_ECLIPSE_GENERATE_SOURCE_PROJECT TRUE)
SET(CMAKE_ECLIPSE_GENERATE_LINKED_RESOURCES FALSE)

PROJECT(OsgOculusViewer)

SET(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Show relative paths options
OPTION(CMAKE_USE_RELATIVE_PATHS "If true, cmake will use relative paths in makefiles and projects." ON)

# Build example viewer
OPTION(BUILD_EXAMPLE "Enable to build viewer example" ON)


IF (WIN32)
	# Path to find OpenSceneGraph
	SET(OSG_DIR $ENV{OSG_DIR} CACHE PATH "Path where to find the OpenSceneGraph")
	IF(NOT OSG_DIR)
		MESSAGE(FATAL_ERROR "Error: OpenSceneGraph not found.")
	ENDIF(NOT OSG_DIR)
	
	# Where to find OpenSceneGraph third party dependencies
	SET(OSG_THIRD_PARTY_DIR CACHE PATH "Path where to find the OpenSceneGraph third party dependencies")
	IF(NOT OSG_THIRD_PARTY_DIR)
		MESSAGE(FATAL_ERROR "Error: OpenSceneGraph 3rd Party Directory not found.")
	ENDIF(NOT OSG_THIRD_PARTY_DIR)
	
	# Where to find Oculus SDK
	SET(OCULUS_SDK_ROOT_DIR ENV{OCULUS_SDK_ROOT_DIR} CACHE PATH "Path where to find the Oculus SDK")
	IF(NOT OCULUS_SDK_ROOT_DIR)
		MESSAGE(FATAL_ERROR "Error: Oculus SDK not found.")
	ENDIF(NOT OCULUS_SDK_ROOT_DIR)
ENDIF(WIN32)

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
    ${OCULUS_SDK_INCLUDE_DIRS}
)

# osgViewer::ViewConfig concept requires osg 3.2 or later
IF(NOT OPENSCENEGRAPH_VERSION VERSION_EQUAL 3.2.0 AND 
NOT OPENSCENEGRAPH_VERSION VERSION_GREATER 3.2.0)
	SET (USE_VIEW_CONFIG OFF)
	SET (BUILD_DEPRECATED ON)
ELSE()
	SET (USE_VIEW_CONFIG ON)
	OPTION(BUILD_DEPRECATED "Enable to build deprecated version as well" OFF)
ENDIF()

###############################################################################
# Compiler Options
################################################################################
IF(WIN32)
	# Make sure we use minimal windows.h library without min max macros
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D \"WIN32_LEAN_AND_MEAN\"")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D \"NOMINMAX\"")
	
	# Use Link Time Code Generation
	SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /LTCG")
	
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