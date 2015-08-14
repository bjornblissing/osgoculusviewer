# Target name
SET(TARGET_LIBRARYNAME OsgOculus)
SET(TARGET_TARGETNAME_VIEWER OculusViewerExample)

# Source files for library
SET(TARGET_SRC
	oculusviewer.cpp
	oculusdevice.cpp
	oculuseventhandler.cpp
	oculusupdateslavecallback.cpp
)
# Header files for library
SET(TARGET_H
	oculusviewer.h
	oculusdevice.h
	oculuseventhandler.h
	oculusupdateslavecallback.h
)

#####################################################################
# Create library
#####################################################################

ADD_LIBRARY(${TARGET_LIBRARYNAME} ${TARGET_SRC} ${TARGET_H})

#####################################################################
# Linker options
#####################################################################

# Link to OpenSceneGraph libs
TARGET_LINK_LIBRARIES(${TARGET_LIBRARYNAME} ${OPENSCENEGRAPH_LIBRARIES} )

# Link to Oculus libs
TARGET_LINK_LIBRARIES(${TARGET_LIBRARYNAME} ${OCULUS_SDK_LIBRARIES}	)

# Link to open gl libs
TARGET_LINK_LIBRARIES(${TARGET_LIBRARYNAME} ${OPENGL_LIBRARIES} )

# Link to libraries needed for Oculus on Visual Studio
IF(MSVC)
	TARGET_LINK_LIBRARIES(${TARGET_LIBRARYNAME} winmm.lib ws2_32.lib)
ENDIF(MSVC)

#####################################################################
# Create executable
#####################################################################
IF(BUILD_EXAMPLES)
	ADD_EXECUTABLE(${TARGET_TARGETNAME_VIEWER} viewerexample.cpp)
	TARGET_LINK_LIBRARIES(${TARGET_TARGETNAME_VIEWER} ${TARGET_LIBRARYNAME})

	INSTALL(TARGETS ${TARGET_TARGETNAME_VIEWER} ${TARGET_TARGETNAME_COMPOSITE_VIEWER} RUNTIME DESTINATION bin)

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
			#CONFIGURE_FILE(osgoculusviewer.vcxproj.template ${TARGET_TARGETNAME_COMPOSITE_VIEWER}.vcxproj.user @ONLY)
			#CONFIGURE_FILE(osgoculusviewer.vcxproj.template ${TARGET_TARGETNAME_SDK_VIEWER}.vcxproj.user @ONLY)
			#IF (USE_VIEW_CONFIG)
			#	CONFIGURE_FILE(osgoculusviewer.vcxproj.template ${TARGET_TARGETNAME_VIEW_CONFIG}.vcxproj.user @ONLY)
			#ENDIF()
		ENDIF()

	ENDIF (OPENSCENEGRAPH_FOUND)
ENDIF(MSVC)

####################################################################
# Install library
#####################################################################

INSTALL(FILES ${TARGET_H} DESTINATION include)

INSTALL(TARGETS ${TARGET_LIBRARYNAME}
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

INSTALL(FILES "${PROJECT_SOURCE_DIR}/README.md" DESTINATION . RENAME readme.txt)
INSTALL(FILES "${PROJECT_SOURCE_DIR}/license.txt" DESTINATION . RENAME license_osgoculus.txt)
INSTALL(FILES ${OCULUS_SDK_LICENSE_FILE} DESTINATION . RENAME license_oculus.txt)

SET(CPACK_PACKAGE_NAME "${TARGET_LIBRARYNAME}-SDK-v")
SET(CPACK_PACKAGE_VERSION_MAJOR "${OCULUS_SDK_VERSION_MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${OCULUS_SDK_VERSION_MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "${OCULUS_SDK_VERSION_BUILD}")
SET(CPACK_RESOURCE_FILE_README  "${PROJECT_SOURCE_DIR}/README.md")
SET(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/license.txt")


IF(WIN32)
	SET(CPACK_GENERATOR "ZIP")
ENDIF()

# Pack files to package
INCLUDE(CPack)
