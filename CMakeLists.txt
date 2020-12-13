CMAKE_MINIMUM_REQUIRED(VERSION 3.14)

PROJECT(OsgOculusViewer LANGUAGES CXX)

SET(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Build example viewer
OPTION(OSGOCULUSVIEWER_BUILD_EXAMPLE "Enable to build viewer example" ON)

# Path to find OpenSceneGraph
SET(OSG_DIR $ENV{OSG_DIR} CACHE PATH "Path where to find the OpenSceneGraph")
IF(NOT OSG_DIR)
	MESSAGE(FATAL_ERROR "Error: OpenSceneGraph not found.")
ENDIF()

# Where to find OpenSceneGraph third party dependencies
SET(OSG_THIRD_PARTY_DIR $ENV{OSG_THIRD_PARTY_DIR} CACHE PATH "Path where to find the OpenSceneGraph third party dependencies")
IF(NOT OSG_THIRD_PARTY_DIR)
	MESSAGE(FATAL_ERROR "Error: OpenSceneGraph 3rd Party Directory not found.")
ENDIF()

# Where to find Oculus SDK
SET(OCULUS_SDK_ROOT_DIR $ENV{OCULUS_SDK_ROOT_DIR} CACHE PATH "Path where to find the Oculus SDK")
IF(NOT OCULUS_SDK_ROOT_DIR)
	MESSAGE(FATAL_ERROR "Error: Oculus SDK not found.")
ENDIF()

 # Find required libraies
FIND_PACKAGE(OpenGL REQUIRED )
FIND_PACKAGE(OpenSceneGraph REQUIRED osgViewer osgDB osgGA osgUtil)
FIND_PACKAGE(OculusSDK REQUIRED)

# Enable available configurations
IF(CMAKE_CONFIGURATION_TYPES)
	IF(OCULUS_SDK_LIBRARY_DEBUG_AVAILABLE)
		SET(CMAKE_CONFIGURATION_TYPES Debug Release)
	ELSE()
		SET(CMAKE_CONFIGURATION_TYPES Release)
	ENDIF()
	 SET(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING
		 "Reset the configurations to what we need"
		 FORCE)
	MARK_AS_ADVANCED(CMAKE_CONFIGURATION_TYPES)
ENDIF()

IF(MSVC)
	# Use Link Time Code Generation
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LTCG")
ENDIF()

# Compile subdirectory
ADD_SUBDIRECTORY(src)