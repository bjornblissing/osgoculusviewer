CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
SET(CMAKE_ECLIPSE_GENERATE_SOURCE_PROJECT TRUE)
SET(CMAKE_ECLIPSE_GENERATE_LINKED_RESOURCES FALSE)

PROJECT(OsgOculusViewer)

SET(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Check if we are doing out of source builds
STRING(COMPARE NOTEQUAL ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} CMAKE_OUT_OF_SOURCE_BUILD)

# Relative paths can be trouble when doing out of source builds
IF(CMAKE_OUT_OF_SOURCE_BUILD)
	SET(DEFAULT_RELATIVE_PATHS OFF)
ELSE()
	SET(DEFAULT_RELATIVE_PATHS ON)
ENDIF()

# Show relative paths options 
OPTION(CMAKE_USE_RELATIVE_PATHS "If true, cmake will use relative paths in makefiles and projects." ${DEFAULT_RELATIVE_PATHS})
MARK_AS_ADVANCED(CMAKE_USE_RELATIVE_PATHS)

# Build example viewers
OPTION(BUILD_EXAMPLES "Enable to build viewer examples" ON)


IF (WIN32)
	# Path to find OpenSceneGraph
	SET(OSG_DIR $ENV{OSG_DIR} CACHE PATH "Path where to find the OpenSceneGraph")
	IF(NOT OSG_DIR)
		MESSAGE(FATAL_ERROR "Error: OpenSceneGraph not found.")
	ENDIF(NOT OSG_DIR)

	# Where to find OpenSceneGraph third party dependencies
	SET(OSG_THIRD_PARTY_DIR $ENV{OSG_THIRD_PARTY_DIR} CACHE PATH "Path where to find the OpenSceneGraph third party dependencies")
	IF(NOT OSG_THIRD_PARTY_DIR)
		MESSAGE(FATAL_ERROR "Error: OpenSceneGraph 3rd Party Directory not found.")
	ENDIF(NOT OSG_THIRD_PARTY_DIR)

	# Where to find Oculus SDK
	SET(OCULUS_SDK_ROOT_DIR $ENV{OCULUS_SDK_ROOT_DIR} CACHE PATH "Path where to find the Oculus SDK")
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
FIND_PACKAGE( OculusSDK REQUIRED)

# Check that Oculus SDK match the desired version
IF(NOT OCULUS_SDK_VERSION VERSION_EQUAL 0.6.0.0 AND 
NOT OCULUS_SDK_VERSION VERSION_GREATER 0.6.0.0)
	MESSAGE(FATAL_ERROR "ERROR: This library required SDK version 0.6.0.0 or later!")
ENDIF()

INCLUDE_DIRECTORIES(BEFORE
    ${OPENGL_INCLUDE_DIR}
    ${OPENSCENEGRAPH_INCLUDE_DIR}
    ${OCULUS_SDK_INCLUDE_DIRS}
)

IF(VISUAL_STUDIO_EXPRESS)
	INCLUDE_DIRECTORIES(BEFORE
		${ATL_INCLUDE_DIR}
	)
ENDIF(VISUAL_STUDIO_EXPRESS)

###############################################################################
# Compiler Options
################################################################################
IF(MSVC)
	# Make sure we use minimal windows.h library without min max macros
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D \"WIN32_LEAN_AND_MEAN\"")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D \"NOMINMAX\"")

	# Use Link Time Code Generation
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LTCG")

	OPTION(BUILD_MULTI_PROCESSOR_COMPILATION "Use multiple processors when compiling" ON)
	MARK_AS_ADVANCED(BUILD_MULTI_PROCESSOR_COMPILATION)

	IF(BUILD_MULTI_PROCESSOR_COMPILATION)
		# Set multi processor build
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
	ENDIF(BUILD_MULTI_PROCESSOR_COMPILATION)
	# Set warnings to level 4 
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	IF (WARNINGS_AS_ERRORS)
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
	ENDIF (WARNINGS_AS_ERRORS)
ENDIF(MSVC)

###############################################################################
# Compile subdirectory
################################################################################
ADD_SUBDIRECTORY(src)