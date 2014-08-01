/*
 * oculusdevice.cpp
 *
 *  Created on: Jul 03, 2013
 *      Author: Bjorn Blissing
 */

#include "oculusdevice.h"

#include <osg/Geometry>


OculusDevice::OculusDevice() : m_hmdDevice(0),
	m_nearClip(0.01f), m_farClip(10000.0f)
{
	ovr_Initialize();
	
	// Enumerate HMD devices
	int hmd_ovrHmd_Detect();

	// Get first available HMD
	m_hmdDevice = ovrHmd_Create(0);
	
	// If no HMD is found try an emulated device
	if (!m_hmdDevice) {
		osg::notify(osg::WARN) << "Warning: No device could be found. Creating emulated device " << std::endl;
		m_hmdDevice = ovrHmd_CreateDebug(ovrHmd_DK1);
	}

	if (m_hmdDevice) {
		// Print out some information about the HMD
		osg::notify(osg::ALWAYS) << "Product:         " << m_hmdDevice->ProductName << std::endl;
		osg::notify(osg::ALWAYS) << "Manufacturer:    " << m_hmdDevice->Manufacturer << std::endl;
		osg::notify(osg::ALWAYS) << "VendorId:          " << m_hmdDevice->VendorId << std::endl;
		osg::notify(osg::ALWAYS) << "ProductId:       " << m_hmdDevice->ProductId << std::endl;
		osg::notify(osg::ALWAYS) << "SerialNumber:    " << m_hmdDevice->SerialNumber << std::endl;
		osg::notify(osg::ALWAYS) << "FirmwareVersion: " << m_hmdDevice->FirmwareMajor << "."  << m_hmdDevice->FirmwareMinor << std::endl;

		// Get more details about the HMD.
		m_resolution = m_hmdDevice->Resolution;
		
		// Compute recommended rendertexture size
		float pixelsPerDisplayPixel = 1.0f; // Decrease this value to scale the size on render texture on lower performance hardware. Values above 1.0 is unnessesary.

		ovrSizei recommenedLeftTextureSize = ovrHmd_GetFovTextureSize(m_hmdDevice, ovrEye_Left, m_hmdDevice->DefaultEyeFov[0], pixelsPerDisplayPixel);
		ovrSizei recommenedRightTextureSize = ovrHmd_GetFovTextureSize(m_hmdDevice, ovrEye_Right, m_hmdDevice->DefaultEyeFov[1], pixelsPerDisplayPixel);

		// Compute size of render target
		m_renderTargetSize.w = recommenedLeftTextureSize.w + recommenedRightTextureSize.w;
		m_renderTargetSize.h = osg::maximum(recommenedLeftTextureSize.h, recommenedRightTextureSize.h);

		// Initialize ovrEyeRenderDesc struct.
		m_eyeRenderDesc[0] = ovrHmd_GetRenderDesc(m_hmdDevice, ovrEye_Left, m_hmdDevice->DefaultEyeFov[0]);
		m_eyeRenderDesc[1] = ovrHmd_GetRenderDesc(m_hmdDevice, ovrEye_Right, m_hmdDevice->DefaultEyeFov[1]);
		
		ovrVector3f leftEyeAdjust = m_eyeRenderDesc[0].ViewAdjust;
		m_leftEyeAdjust.set(leftEyeAdjust.x, leftEyeAdjust.y, leftEyeAdjust.z);
		ovrVector3f rightEyeAdjust = m_eyeRenderDesc[1].ViewAdjust;
		m_rightEyeAdjust.set(rightEyeAdjust.x, rightEyeAdjust.y, rightEyeAdjust.z);

		bool isRightHanded = true;
		
		ovrMatrix4f leftEyeProjectionMatrix = ovrMatrix4f_Projection(m_eyeRenderDesc[0].Fov, m_nearClip, m_farClip, isRightHanded);
		// Transpose matrix
		m_leftEyeProjectionMatrix.set(leftEyeProjectionMatrix.M[0][0], leftEyeProjectionMatrix.M[1][0], leftEyeProjectionMatrix.M[2][0], leftEyeProjectionMatrix.M[3][0],
			leftEyeProjectionMatrix.M[0][1], leftEyeProjectionMatrix.M[1][1], leftEyeProjectionMatrix.M[2][1], leftEyeProjectionMatrix.M[3][1],
			leftEyeProjectionMatrix.M[0][2], leftEyeProjectionMatrix.M[1][2], leftEyeProjectionMatrix.M[2][2], leftEyeProjectionMatrix.M[3][2],
			leftEyeProjectionMatrix.M[0][3], leftEyeProjectionMatrix.M[1][3], leftEyeProjectionMatrix.M[2][3], leftEyeProjectionMatrix.M[3][3]);

		ovrMatrix4f rightEyeProjectionMatrix = ovrMatrix4f_Projection(m_eyeRenderDesc[1].Fov, m_nearClip, m_farClip, isRightHanded);
		// Transpose matrix	
		m_rightEyeProjectionMatrix.set(rightEyeProjectionMatrix.M[0][0], rightEyeProjectionMatrix.M[1][0], rightEyeProjectionMatrix.M[2][0], rightEyeProjectionMatrix.M[3][0],
			rightEyeProjectionMatrix.M[0][1], rightEyeProjectionMatrix.M[1][1], rightEyeProjectionMatrix.M[2][1], rightEyeProjectionMatrix.M[3][1],
			rightEyeProjectionMatrix.M[0][2], rightEyeProjectionMatrix.M[1][2], rightEyeProjectionMatrix.M[2][2], rightEyeProjectionMatrix.M[3][2],
			rightEyeProjectionMatrix.M[0][3], rightEyeProjectionMatrix.M[1][3], rightEyeProjectionMatrix.M[2][3], rightEyeProjectionMatrix.M[3][3]);

		// Start the sensor which provides the Rift’s pose and motion.
		ovrHmd_ConfigureTracking(m_hmdDevice, ovrTrackingCap_Orientation |
			ovrTrackingCap_MagYawCorrection |
			ovrTrackingCap_Position, 0);
	}
}

OculusDevice::~OculusDevice()
{
	ovrHmd_Destroy(m_hmdDevice);
	ovr_Shutdown();
}

unsigned int OculusDevice::hScreenResolution() const
{
	if (m_hmdDevice) {
		return m_hmdDevice->Resolution.w;
	}
	
	// Default value from dev kit 1
	return 1280;
}

unsigned int OculusDevice::vScreenResolution() const
{
	if (m_hmdDevice) {
		return m_hmdDevice->Resolution.h;
	}

	// Default value from dev kit 1
	return 800;
}


unsigned int OculusDevice::hRenderTargetSize() const
{
	return m_renderTargetSize.w;
}

unsigned int OculusDevice::vRenderTargetSize() const
{
	return m_renderTargetSize.h;
}

osg::Matrix OculusDevice::projectionMatrixCenter() const
{
	osg::Matrix projectionMatrixCenter;
	projectionMatrixCenter = m_leftEyeProjectionMatrix;
	projectionMatrixCenter(2, 0) = 0; // Ugly hack to make left projection matrix into a center projection matrix
	return projectionMatrixCenter;
}

osg::Matrix OculusDevice::projectionMatrixLeft() const
{
	return m_leftEyeProjectionMatrix;
}

osg::Matrix OculusDevice::projectionMatrixRight() const
{
	return m_rightEyeProjectionMatrix;
}

osg::Matrix OculusDevice::projectionOffsetMatrixLeft() const
{
	osg::Matrix projectionOffsetMatrix;
	float offset = m_leftEyeProjectionMatrix(2, 0); // Ugly hack to extract projection offset
	projectionOffsetMatrix(3, 0) = offset;
	return projectionOffsetMatrix;
}

osg::Matrix OculusDevice::projectionOffsetMatrixRight() const
{
	osg::Matrix projectionOffsetMatrix;
	float offset = m_rightEyeProjectionMatrix(2, 0); // Ugly hack to extract projection offset
	projectionOffsetMatrix(3, 0) = offset;
	return projectionOffsetMatrix;
}
osg::Matrix OculusDevice::viewMatrixLeft() const
{
	osg::Matrix viewMatrix;
	viewMatrix.makeTranslate(m_leftEyeAdjust);
	return viewMatrix;
}

osg::Matrix OculusDevice::viewMatrixRight() const
{
	osg::Matrix viewMatrix;
	viewMatrix.makeTranslate(m_rightEyeAdjust);
	return viewMatrix;
}

osg::Quat OculusDevice::getOrientation() const
{
	// Create identity quaternion
	osg::Quat osgQuat(0.0f, 0.0f, 0.0f, 1.0f);

	// Query the HMD for the current tracking state.
	ovrTrackingState ts = ovrHmd_GetTrackingState(m_hmdDevice, ovr_GetTimeInSeconds());
	if (ts.StatusFlags & (ovrStatus_OrientationTracked | ovrStatus_PositionTracked)) {
		ovrPoseStatef headpose = ts.HeadPose;
		ovrPosef pose = headpose.ThePose;
		ovrQuatf quat = pose.Orientation;
		osgQuat.set(quat.x, quat.y, quat.z, -quat.w);
	}

	return osgQuat;
}

void OculusDevice::resetSensorOrientation() {
	ovrHmd_RecenterPose(m_hmdDevice);
}

osg::Geode* OculusDevice::distortionMesh(int eyeNum, osg::Program* program, int x, int y, int w, int h) {
	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	// Allocate & generate distortion mesh vertices.
	ovrDistortionMesh meshData;
	ovrHmd_CreateDistortionMesh(m_hmdDevice, m_eyeRenderDesc[eyeNum].Eye, m_eyeRenderDesc[eyeNum].Fov, ovrDistortionCap_Chromatic | ovrDistortionCap_TimeWarp, &meshData);
	
	// Now parse the vertex data and create a render ready vertex buffer from it
	ovrDistortionVertex* ov = meshData.pVertexData;
	osg::Vec2Array* positionArray = new osg::Vec2Array;
	osg::Vec4Array* colorArray = new osg::Vec4Array;
	osg::Vec2Array* textureRArray = new osg::Vec2Array;
	osg::Vec2Array* textureGArray = new osg::Vec2Array;
	osg::Vec2Array* textureBArray = new osg::Vec2Array;

	for (unsigned vertNum = 0; vertNum < meshData.VertexCount; ++vertNum)
	{
		positionArray->push_back(osg::Vec2f(ov[vertNum].ScreenPosNDC.x, ov[vertNum].ScreenPosNDC.y));
		colorArray->push_back(osg::Vec4f(ov[vertNum].VignetteFactor, ov[vertNum].VignetteFactor, ov[vertNum].VignetteFactor, ov[vertNum].TimeWarpFactor));
		textureRArray->push_back(osg::Vec2f(ov[vertNum].TanEyeAnglesR.x, ov[vertNum].TanEyeAnglesR.y));
		textureGArray->push_back(osg::Vec2f(ov[vertNum].TanEyeAnglesG.x, ov[vertNum].TanEyeAnglesG.y));
		textureBArray->push_back(osg::Vec2f(ov[vertNum].TanEyeAnglesB.x, ov[vertNum].TanEyeAnglesB.y));
	}
	
	// Get triangle indicies 
	osg::UShortArray* indexArray = new osg::UShortArray;
	unsigned short* index = meshData.pIndexData;
	for (unsigned indexNum = 0; indexNum < meshData.IndexCount; ++indexNum) {
		indexArray->push_back(index[indexNum]);
	}

	// Deallocate the mesh data
	ovrHmd_DestroyDistortionMesh(&meshData);
	
	osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
	osg::ref_ptr<osg::DrawElementsUShort> drawElement = new osg::DrawElementsUShort(osg::PrimitiveSet::TRIANGLES, indexArray->size(), (GLushort*) indexArray->getDataPointer());
	geometry->addPrimitiveSet(drawElement);

	GLuint positionLoc = 0;
	GLuint colorLoc = 1;
	GLuint texCoord0Loc = 2;
	GLuint texCoord1Loc = 3;
	GLuint texCoord2Loc = 4;

	program->addBindAttribLocation("Position", positionLoc);
	geometry->setVertexAttribArray(positionLoc, positionArray);
	geometry->setVertexAttribBinding(positionLoc, osg::Geometry::BIND_PER_VERTEX);

	program->addBindAttribLocation("Color", colorLoc);
	geometry->setVertexAttribArray(colorLoc, colorArray);
	geometry->setVertexAttribBinding(colorLoc, osg::Geometry::BIND_PER_VERTEX);

	program->addBindAttribLocation("TexCoord0", texCoord0Loc);
	geometry->setVertexAttribArray(texCoord0Loc, textureRArray);
	geometry->setVertexAttribBinding(texCoord0Loc, osg::Geometry::BIND_PER_VERTEX);

	program->addBindAttribLocation("TexCoord1", texCoord1Loc);
	geometry->setVertexAttribArray(texCoord1Loc, textureGArray);
	geometry->setVertexAttribBinding(texCoord1Loc, osg::Geometry::BIND_PER_VERTEX);

	program->addBindAttribLocation("TexCoord2", texCoord2Loc);
	geometry->setVertexAttribArray(texCoord2Loc, textureBArray);
	geometry->setVertexAttribBinding(texCoord2Loc, osg::Geometry::BIND_PER_VERTEX);


	// Compute UV scale and offset
	ovrRecti eyeRenderViewport;
	eyeRenderViewport.Pos.x = x;
	eyeRenderViewport.Pos.y = y;
	eyeRenderViewport.Size.w = w;
	eyeRenderViewport.Size.h = h;
	ovrHmd_GetRenderScaleAndOffset(m_eyeRenderDesc[eyeNum].Fov, m_renderTargetSize, eyeRenderViewport, m_UVScaleOffset[eyeNum]);
	geode->addDrawable(geometry);
	return geode.release();
}

osg::Vec2f OculusDevice::eyeToSourceUVScale(int eyeNum) const
{
	osg::Vec2f uvScale(m_UVScaleOffset[eyeNum][0].x, m_UVScaleOffset[eyeNum][0].y);
	return uvScale;
}
osg::Vec2f OculusDevice::eyeToSourceUVOffset(int eyeNum) const
{
	osg::Vec2f uvOffset(m_UVScaleOffset[eyeNum][1].x, m_UVScaleOffset[eyeNum][1].y);
	return uvOffset;
}

