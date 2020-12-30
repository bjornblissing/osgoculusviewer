# Target name
SET(TARGET_LIBRARYNAME OsgOculus)
SET(TARGET_TARGETNAME_VIEWER OculusViewerExample)

# Source files for library
SET(TARGET_SRC
	oculusviewer.cpp
	oculusdevice.cpp
	oculusdrawcallbacks.cpp
	oculuseventhandler.cpp
	oculusgraphicsoperation.cpp
	oculusmirrortexture.cpp
	oculusswapcallback.cpp
	oculustexturebuffer.cpp
	oculusupdateslavecallback.cpp
	oculustouchmanipulator.cpp
)
# Header files for library
SET(HEADER_PATH ../include/)
SET(TARGET_H
	${HEADER_PATH}/oculusconfig.h
	${HEADER_PATH}/oculusviewer.h
	${HEADER_PATH}/oculusdevice.h
	${HEADER_PATH}/oculusdrawcallbacks.h
	${HEADER_PATH}/oculuseventhandler.h
	${HEADER_PATH}/oculusgraphicsoperation.h
	${HEADER_PATH}/oculusmirrortexture.h
	${HEADER_PATH}/oculusswapcallback.h
	${HEADER_PATH}/oculustexturebuffer.h
	${HEADER_PATH}/oculusupdateslavecallback.h
	${HEADER_PATH}/oculustouchmanipulator.h
	${HEADER_PATH}/glextensions.h
)

#####################################################################
# Create library
#####################################################################

# static or dinamic?
IF(OSGOCULUSLIB_BUILD_SHARED)
	ADD_LIBRARY(${TARGET_LIBRARYNAME} SHARED ${TARGET_SRC} ${TARGET_H})	
ELSE()
	ADD_LIBRARY(${TARGET_LIBRARYNAME} ${TARGET_SRC} ${TARGET_H})
ENDIF()

TARGET_COMPILE_OPTIONS(${TARGET_LIBRARYNAME}
	PUBLIC 
		$<$<CONFIG:Release>:/MT>
		$<$<CONFIG:Debug>:/MTd>
		$<$<CXX_COMPILER_ID:MSVC>:/MP>
	PRIVATE
		$<$<CXX_COMPILER_ID:MSVC>:/W4>
)

TARGET_INCLUDE_DIRECTORIES(${TARGET_LIBRARYNAME}
	PUBLIC
		${OPENSCENEGRAPH_INCLUDE_DIR}
		${OCULUS_SDK_INCLUDE_DIRS}
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${HEADER_PATH}>
		$<INSTALL_INTERFACE:include> 
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

ENDIF()

####################################################################
# Install library
#####################################################################

INSTALL(FILES ${TARGET_H} DESTINATION include)

INSTALL(TARGETS ${TARGET_LIBRARYNAME} EXPORT OsgOculusViewerTargets
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	RUNTIME DESTINATION bin
	INCLUDES DESTIONATION include
)

install(EXPORT OsgOculusViewerTargets
	FILE OsgOculusViewerTargets.cmake
	NAMESPACE OsgOculusViewer::
	DESTINATION lib/cmake/osgoculusviewer
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
