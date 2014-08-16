# Target name
SET(TARGET_LIBRARYNAME OsgOculus)
SET(TARGET_TARGETNAME_VIEW_CONFIG OculusViewConfigExample)
SET(TARGET_TARGETNAME_VIEWER OculusViewerExample)
SET(TARGET_TARGETNAME_COMPOSITE_VIEWER OculusCompositeViewerExample)

# Source files for library
SET(TARGET_SRC
	oculusviewer.cpp
	oculusdevice.cpp
	oculuseventhandler.cpp
	oculusviewconfig.cpp
)
# Header files for library
SET(TARGET_H
	oculusviewer.h
	oculusdevice.h
	oculuseventhandler.h
	oculusviewconfig.h
)

# Add view config if available
IF (USE_VIEW_CONFIG)
	SET(TARGET_SRC ${TARGET_SRC}
		oculusviewconfig.cpp
	)

	SET(TARGET_H ${TARGET_H}
		oculusviewconfig.h
	)
ENDIF()

#####################################################################
# Create library
#####################################################################

ADD_LIBRARY(${TARGET_LIBRARYNAME} ${TARGET_SRC} ${TARGET_H})

#####################################################################
# Linker options
#####################################################################

# Link to open gl libs
TARGET_LINK_LIBRARIES(${TARGET_LIBRARYNAME} ${OPENGL_LIBRARIES} )

# Link to OpenSceneGraph libs
TARGET_LINK_LIBRARIES(${TARGET_LIBRARYNAME} ${OPENSCENEGRAPH_LIBRARIES} )

# Link to Oculus libs
TARGET_LINK_LIBRARIES(${TARGET_LIBRARYNAME} ${OCULUS_SDK_LIBRARIES}	)

# Link to libraries needed for Oculus on Visual Studio
IF(MSVC)
	TARGET_LINK_LIBRARIES(${TARGET_LIBRARYNAME} winmm.lib ws2_32.lib)
ENDIF(MSVC)

# Link to libraries needed for Oculus on Linux and Mac
IF(UNIX)
	# Oculus SDK dependencies for Mac OSX
	IF (APPLE)
		TARGET_LINK_LIBRARIES (
			${TARGET_LIBRARYNAME}
			${CORE_FOUNDATION}
			${CORE_GRAPHICS}
			${IOKIT}
		)
	ELSE(APPLE)
		# Oculus SDK dependencies for Linux
		TARGET_LINK_LIBRARIES(${TARGET_LIBRARYNAME} pthread udev X11 Xinerama)
	ENDIF(APPLE)
ENDIF(UNIX)

#####################################################################
# Create executable
#####################################################################
IF(BUILD_EXAMPLES)
	ADD_EXECUTABLE(${TARGET_TARGETNAME_VIEWER} viewerexample.cpp)
	TARGET_LINK_LIBRARIES(${TARGET_TARGETNAME_VIEWER} ${TARGET_LIBRARYNAME})

	ADD_EXECUTABLE(${TARGET_TARGETNAME_COMPOSITE_VIEWER} compositeviewerexample.cpp)
	TARGET_LINK_LIBRARIES(${TARGET_TARGETNAME_COMPOSITE_VIEWER} ${TARGET_LIBRARYNAME})

	IF (USE_VIEW_CONFIG)
		ADD_EXECUTABLE(${TARGET_TARGETNAME_VIEW_CONFIG} viewconfigexample.cpp)
		TARGET_LINK_LIBRARIES(${TARGET_TARGETNAME_VIEW_CONFIG} ${TARGET_LIBRARYNAME})
	ENDIF()

ENDIF(BUILD_EXAMPLES)


####################################################################
# Create user file for correct environment string
#####################################################################
IF(MSVC)
	IF (OPENSCENEGRAPH_FOUND)
		# Build path
		SET(USERFILE_ENVIRONMENT_DEBUG "PATH=")
		SET(USERFILE_ENVIRONMENT_RELEASE "PATH=")

		# Add OpenSceneGraph to path
		IF (EXISTS "${OSG_DIR}/bin")
			SET(OSG_BIN_DIR "${OSG_DIR}/bin" CACHE PATH "Path where to find OpenSceneGraph binaries")
		ELSE (EXISTS "${OSG_DIR}/bin")
			SET (OSG_BIN_DIR CACHE PATH "Path where to find OpenSceneGraph binaries")
		ENDIF(EXISTS "${OSG_DIR}/bin")
		SET(USERFILE_ENVIRONMENT_DEBUG "${USERFILE_ENVIRONMENT_DEBUG}${OSG_BIN_DIR}")
		SET(USERFILE_ENVIRONMENT_RELEASE "${USERFILE_ENVIRONMENT_RELEASE}${OSG_BIN_DIR}")

		IF (OSG_THIRD_PARTY_DIR)
			# Add third party to path
			SET(USERFILE_ENVIRONMENT_DEBUG "${USERFILE_ENVIRONMENT_DEBUG};${OSG_THIRD_PARTY_DIR}/bin")
			SET(USERFILE_ENVIRONMENT_RELEASE "${USERFILE_ENVIRONMENT_RELEASE};${OSG_THIRD_PARTY_DIR}/bin")
		ELSE (OSG_THIRD_PARTY_DIR)
			MESSAGE("Warning: OpenSceneGraph 3rd Party Directory not found.")
		ENDIF(OSG_THIRD_PARTY_DIR)

		# For visual studio 2010 (MSVC10), 2012 (MSVC11) and 2013 (MSVC12)
		IF(MSVC10 OR MSVC11 OR MSVC12)
			CONFIGURE_FILE(osgoculusviewer.vcxproj.template ${TARGET_TARGETNAME_VIEWER}.vcxproj.user @ONLY)
			CONFIGURE_FILE(osgoculusviewer.vcxproj.template ${TARGET_TARGETNAME_COMPOSITE_VIEWER}.vcxproj.user @ONLY)
			IF (USE_VIEW_CONFIG)
				CONFIGURE_FILE(osgoculusviewer.vcxproj.template ${TARGET_TARGETNAME_VIEW_CONFIG}.vcxproj.user @ONLY)
			ENDIF()

		ENDIF()
	ENDIF (OPENSCENEGRAPH_FOUND)
ENDIF(MSVC)
