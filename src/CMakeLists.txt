# Target name
SET(TARGET_TARGETNAME OsgOculusViewer)

# Target source files
SET(TARGET_SRC 
	main.cpp
	oculusdevice.cpp
)

# Target headerfiles
SET(TARGET_H 
	oculusdevice.h
)

#####################################################################
# Create executable
#####################################################################

ADD_EXECUTABLE(${TARGET_TARGETNAME} ${TARGET_SRC} ${TARGET_H})

#####################################################################
# Linker options
#####################################################################

# Link to open gl libs
TARGET_LINK_LIBRARIES(${TARGET_TARGETNAME}  ${OPENGL_LIBRARIES} )

# Link to OpenSceneGraph libs
TARGET_LINK_LIBRARIES(${TARGET_TARGETNAME}  ${OPENSCENEGRAPH_LIBRARIES} )

# Link to Oculus libs
TARGET_LINK_LIBRARIES(${TARGET_TARGETNAME} 	${OCULUS_LIBRARIES}	)

# Link to Windows Multi Media Libs needed for Oculus
TARGET_LINK_LIBRARIES(${TARGET_TARGETNAME} winmm.lib )

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
				CONFIGURE_FILE(osgoculusviewer.vcxproj.template ${TARGET_TARGETNAME}.vcxproj.user @ONLY)
			ENDIF(MSVC10)
			
		ENDIF (OPENSCENEGRAPH_FOUND)
    ENDIF(MSVC)
ENDIF(WIN32)
