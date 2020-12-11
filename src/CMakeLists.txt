# Target name
SET(TARGET_LIBRARYNAME OsgOculus)
SET(TARGET_TARGETNAME_VIEWER OculusViewerExample)

# Source files for library
SET(TARGET_SRC
	oculusviewer.cpp
	oculusdevice.cpp
	oculuseventhandler.cpp
	oculustexture.cpp
	oculusupdateslavecallback.cpp
	oculustouchmanipulator.cpp
)
# Header files for library
SET(TARGET_H
	oculusviewer.h
	oculusdevice.h
	oculuseventhandler.h
	oculustexture.h
	oculusupdateslavecallback.h
	oculustouchmanipulator.h
)

#####################################################################
# Create library
#####################################################################

ADD_LIBRARY(${TARGET_LIBRARYNAME} ${TARGET_SRC} ${TARGET_H})

TARGET_COMPILE_OPTIONS(${TARGET_LIBRARYNAME}
	PUBLIC 
		$<$<CONFIG:Release>:/MT>
		$<$<CONFIG:Debug>:/MTd>
	PRIVATE
		$<$<CXX_COMPILER_ID:MSVC>:/W4>
)

TARGET_INCLUDE_DIRECTORIES(${TARGET_LIBRARYNAME}
	PUBLIC
		${OPENSCENEGRAPH_INCLUDE_DIR}
		${OCULUS_SDK_INCLUDE_DIRS}
	PRIVATE
		${OPENGL_INCLUDE_DIR}
)


# Link to OpenSceneGraph libs
TARGET_LINK_LIBRARIES(${TARGET_LIBRARYNAME}
	PUBLIC
		${OPENSCENEGRAPH_LIBRARIES}
	PRIVATE
		${OCULUS_SDK_LIBRARIES}
		${OPENGL_LIBRARIES}
		$<$<CXX_COMPILER_ID:MSVC>:winmm.lib ws2_32.lib>
)

TARGET_COMPILE_DEFINITIONS(${TARGET_LIBRARYNAME}
	PUBLIC
		# Make sure we use minimal windows.h library without min max macros
		$<$<CXX_COMPILER_ID:MSVC>:WIN32_LEAN_AND_MEAN>
		$<$<CXX_COMPILER_ID:MSVC>:NOMINMAX>
)

IF (IPO_SUPPORTED)
	# Enable link time code generation
	SET_PROPERTY(TARGET ${TARGET_LIBRARYNAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
ENDIF()

#####################################################################
# Create example
#####################################################################
IF(OSGOCULUSVIEWER_BUILD_EXAMPLE)
	ADD_EXECUTABLE(${TARGET_TARGETNAME_VIEWER} viewerexample.cpp)

	TARGET_LINK_LIBRARIES(${TARGET_TARGETNAME_VIEWER}
		PRIVATE
			${TARGET_LIBRARYNAME}
	)
	
	TARGET_COMPILE_OPTIONS(${TARGET_LIBRARYNAME}
	PRIVATE
		$<$<CXX_COMPILER_ID:MSVC>:/W4>
)

	INSTALL(TARGETS ${TARGET_TARGETNAME_VIEWER} RUNTIME DESTINATION bin)

	# Set debugger environment and program argument for Visual Studio
	SET_TARGET_PROPERTIES(${TARGET_TARGETNAME_VIEWER} PROPERTIES 
		VS_DEBUGGER_ENVIRONMENT "PATH=%PATH%;${OSG_DIR}/bin;${OSG_THIRD_PARTY_DIR}/bin;"
	)
	IF (IPO_SUPPORTED)
		# Enable link time code generation
		SET_PROPERTY(TARGET ${TARGET_TARGETNAME_VIEWER} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
	ENDIF()

ENDIF()

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
