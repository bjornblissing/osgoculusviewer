# Target name
SET(TARGET_LIBRARYNAME OsgOculus)
SET(TARGET_LIBRARYNAME_DEPRECATED OsgOculus_deprecated)
SET(TARGET_TARGETNAME OsgOculusViewer)
SET(TARGET_TARGETNAME_DEPRECATED OsgOculusViewer_deprecated)

# Target source files
SET(TARGET_SRC
	oculusdevice.cpp
	oculusviewconfig.cpp
)
# Target header files
SET(TARGET_H
	oculusdevice.h
	oculusviewconfig.h
)

# Target source files
SET(TARGET_DEPRECATED_SRC
	oculusdevice.cpp
	hmdcamera.cpp
)
# Target header files
SET(TARGET_DEPRECATED_H
	oculusdevice.cpp
	hmdcamera.h
)

#####################################################################
# Create library
#####################################################################

IF(USE_VIEW_CONFIG)
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

	# Link to libraries needed for Oculus on Windows
	IF(WIN32)
		TARGET_LINK_LIBRARIES(${TARGET_LIBRARYNAME} winmm.lib )
	ENDIF(WIN32)

	# Link to libraries needed for Oculus on Linux
	IF(UNIX)
		TARGET_LINK_LIBRARIES(${TARGET_LIBRARYNAME} pthread udev X11 Xinerama)
	ENDIF(UNIX)
	
	#####################################################################
	# Create executable
	#####################################################################
	IF(BUILD_EXAMPLE)
		ADD_EXECUTABLE(${TARGET_TARGETNAME} osgoculusviewer.cpp)
		TARGET_LINK_LIBRARIES(${TARGET_TARGETNAME} ${TARGET_LIBRARYNAME})
	ENDIF(BUILD_EXAMPLE)
ENDIF()

IF (BUILD_DEPRECATED)
	ADD_LIBRARY(${TARGET_LIBRARYNAME_DEPRECATED} ${TARGET_DEPRECATED_SRC} ${TARGET_DEPRECATED_H})
	#####################################################################
	# Linker options
	#####################################################################

	# Link to open gl libs
	TARGET_LINK_LIBRARIES(${TARGET_LIBRARYNAME_DEPRECATED} ${OPENGL_LIBRARIES} )

	# Link to OpenSceneGraph libs
	TARGET_LINK_LIBRARIES(${TARGET_LIBRARYNAME_DEPRECATED} ${OPENSCENEGRAPH_LIBRARIES} )

	# Link to Oculus libs
	TARGET_LINK_LIBRARIES(${TARGET_LIBRARYNAME_DEPRECATED} ${OCULUS_SDK_LIBRARIES}	)

	# Link to libraries needed for Oculus on Windows
	IF(WIN32)
		TARGET_LINK_LIBRARIES(${TARGET_LIBRARYNAME_DEPRECATED} winmm.lib )
	ENDIF(WIN32)

	# Link to libraries needed for Oculus on Linux
	IF(UNIX)
		TARGET_LINK_LIBRARIES(${TARGET_LIBRARYNAME_DEPRECATED} pthread udev X11 Xinerama)
	ENDIF(UNIX)
	
	#####################################################################
	# Create executable
	#####################################################################
	IF(BUILD_EXAMPLE)
		ADD_EXECUTABLE(${TARGET_TARGETNAME_DEPRECATED} osgoculusviewer_deprecated.cpp)
		TARGET_LINK_LIBRARIES(${TARGET_TARGETNAME_DEPRECATED} ${TARGET_LIBRARYNAME_DEPRECATED})
	ENDIF(BUILD_EXAMPLE)
ENDIF()

####################################################################
# Create user file for correct environment string
#####################################################################
IF(WIN32)
    IF(MSVC)
		IF (OPENSCENEGRAPH_FOUND)
			# Build path
			SET(USERFILE_ENVIRONMENT_DEBUG "PATH=")
			SET(USERFILE_ENVIRONMENT_RELEASE "PATH=")

			# Add OpenSceneGraph to path
			STRING(REPLACE "\\" "/" OSG_DIR $ENV{OSG_DIR})
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

			# For visual studio 10
			IF(MSVC10)
				IF (USE_VIEW_CONFIG)
					CONFIGURE_FILE(osgoculusviewer.vcxproj.template ${TARGET_TARGETNAME}.vcxproj.user @ONLY)
				ENDIF()
				IF (BUILD_DEPRECATED)
					CONFIGURE_FILE(osgoculusviewer.vcxproj.template ${TARGET_TARGETNAME_DEPRECATED}.vcxproj.user @ONLY)
				ENDIF()
			ENDIF(MSVC10)

		ENDIF (OPENSCENEGRAPH_FOUND)
    ENDIF(MSVC)
ENDIF(WIN32)
