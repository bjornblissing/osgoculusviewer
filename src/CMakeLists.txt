# Target name
SET(TARGET_LIBRARYNAME OsgOculus)
SET(TARGET_LIBRARYNAME_DEPRECATED OsgOculus_deprecated)
SET(TARGET_TARGETNAME OsgOculusViewer)
SET(TARGET_TARGETNAME_DEPRECATED OsgOculusViewer_deprecated)

# Target source files
SET(TARGET_SRC
	oculusdevice.cpp
	oculuseventhandler.cpp
	oculusviewconfig.cpp
)
# Target header files
SET(TARGET_H
	oculusdevice.h
	oculuseventhandler.h
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

	# Link to libraries needed for Oculus on Visual Studio
	IF(MSVC)
		TARGET_LINK_LIBRARIES(${TARGET_LIBRARYNAME} winmm.lib ws2_32.lib)
	ENDIF(MSVC)

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
# Copy shaders if we are doing out of source builds
#####################################################################

IF (CMAKE_OUT_OF_SOURCE_BUILD)
	CONFIGURE_FILE(warp_mesh.vert warp_mesh.vert COPYONLY)
	CONFIGURE_FILE(warp_mesh_with_timewarp.vert warp_mesh_with_timewarp.vert COPYONLY)
	CONFIGURE_FILE(warp_mesh.frag warp_mesh.frag COPYONLY)
ENDIF()

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
			IF (USE_VIEW_CONFIG)
				CONFIGURE_FILE(osgoculusviewer.vcxproj.template ${TARGET_TARGETNAME}.vcxproj.user @ONLY)
			ENDIF()
			IF (BUILD_DEPRECATED)
				CONFIGURE_FILE(osgoculusviewer.vcxproj.template ${TARGET_TARGETNAME_DEPRECATED}.vcxproj.user @ONLY)
			ENDIF()
		ENDIF()

	ENDIF (OPENSCENEGRAPH_FOUND)
ENDIF(MSVC)

INSTALL(FILES ${TARGET_H} DESTINATION include)
INSTALL(FILES
warp_mesh_with_timewarp.vert
warp_mesh.frag
warp_mesh.vert
DESTINATION share)
INSTALL(TARGETS ${TARGET_LIBRARYNAME} ${TARGET_TARGETNAME}
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
)

