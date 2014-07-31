/*
 * oculusdevice.cpp
 *
 *  Created on: Jul 03, 2013
 *      Author: Bjorn Blissing
 */

#include "oculusdevice.h"


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
		ovrEyeRenderDesc eyeRenderDesc[2];
		eyeRenderDesc[0] = ovrHmd_GetRenderDesc(m_hmdDevice, ovrEye_Left, m_hmdDevice->DefaultEyeFov[0]);
		eyeRenderDesc[1] = ovrHmd_GetRenderDesc(m_hmdDevice, ovrEye_Right, m_hmdDevice->DefaultEyeFov[1]);
		
		ovrVector3f leftEyeAdjust = eyeRenderDesc[0].ViewAdjust;
		m_leftEyeAdjust.set(leftEyeAdjust.x, leftEyeAdjust.y, leftEyeAdjust.z);
		ovrVector3f rightEyeAdjust = eyeRenderDesc[1].ViewAdjust;
		m_rightEyeAdjust.set(rightEyeAdjust.x, rightEyeAdjust.y, rightEyeAdjust.z);

		bool isRightHanded = true;
		
		ovrMatrix4f leftEyeProjectionMatrix = ovrMatrix4f_Projection(eyeRenderDesc[0].Fov, m_nearClip, m_farClip, isRightHanded);
		// Test of transpose
		m_leftEyeProjectionMatrix.set(leftEyeProjectionMatrix.M[0][0], leftEyeProjectionMatrix.M[1][0], leftEyeProjectionMatrix.M[2][0], leftEyeProjectionMatrix.M[3][0],
			leftEyeProjectionMatrix.M[0][1], leftEyeProjectionMatrix.M[1][1], leftEyeProjectionMatrix.M[2][1], leftEyeProjectionMatrix.M[3][1],
			leftEyeProjectionMatrix.M[0][2], leftEyeProjectionMatrix.M[1][2], leftEyeProjectionMatrix.M[2][2], leftEyeProjectionMatrix.M[3][2],
			leftEyeProjectionMatrix.M[0][3], leftEyeProjectionMatrix.M[1][3], leftEyeProjectionMatrix.M[2][3], leftEyeProjectionMatrix.M[3][3]);
		//m_leftEyeProjectionMatrix.set(*leftEyeProjectionMatrix.M);

		ovrMatrix4f rightEyeProjectionMatrix = ovrMatrix4f_Projection(eyeRenderDesc[1].Fov, m_nearClip, m_farClip, isRightHanded);
			
		m_rightEyeProjectionMatrix.set(rightEyeProjectionMatrix.M[0][0], rightEyeProjectionMatrix.M[1][0], rightEyeProjectionMatrix.M[2][0], rightEyeProjectionMatrix.M[3][0],
			rightEyeProjectionMatrix.M[0][1], rightEyeProjectionMatrix.M[1][1], rightEyeProjectionMatrix.M[2][1], rightEyeProjectionMatrix.M[3][1],
			rightEyeProjectionMatrix.M[0][2], rightEyeProjectionMatrix.M[1][2], rightEyeProjectionMatrix.M[2][2], rightEyeProjectionMatrix.M[3][2],
			rightEyeProjectionMatrix.M[0][3], rightEyeProjectionMatrix.M[1][3], rightEyeProjectionMatrix.M[2][3], rightEyeProjectionMatrix.M[3][3]);


		//m_rightEyeProjectionMatrix.set(*rightEyeProjectionMatrix.M);

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

osg::Matrix OculusDevice::projectionMatrixLeft() const
{
	return m_leftEyeProjectionMatrix;
}

osg::Matrix OculusDevice::projectionMatrixRight() const
{
	return m_rightEyeProjectionMatrix;
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

	return osgQuat;
}