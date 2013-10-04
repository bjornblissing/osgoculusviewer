/*
 * oculusdevice.cpp
 *
 *  Created on: Jul 03, 2013
 *      Author: Bjorn Blissing
 */

#include "oculusdevice.h"


OculusDevice::OculusDevice() :
	m_deviceManager(0), m_hmdDevice(0), m_sensor(0), m_hmdInfo(0), m_sensorFusion(0),
	m_useCustomScaleFactor(false), m_customScaleFactor(1.0f),
	m_nearClip(0.3f), m_farClip(5000.0f), m_predictionDelta(0.03f)
{
	// Init Oculus HMD
	OVR::System::Init(OVR::Log::ConfigureDefaultLog(OVR::LogMask_All));
	m_deviceManager = *OVR::DeviceManager::Create();
	m_hmdDevice = *m_deviceManager->EnumerateDevices<OVR::HMDDevice>().CreateDevice();

	if (m_hmdDevice) {
		m_hmdInfo = new OVR::HMDInfo;

		if (!m_hmdDevice->GetDeviceInfo(m_hmdInfo)) {
			osg::notify(osg::FATAL) << "Error: Unable to get device info" << std::endl;
		} else {
			m_sensor = *m_hmdDevice->GetSensor();

			if (m_sensor) {
				m_sensorFusion = new OVR::SensorFusion;
				m_sensorFusion->AttachToSensor(m_sensor);
				m_sensorFusion->SetPredictionEnabled(true);
				// Get default sensor prediction delta
				m_predictionDelta = m_sensorFusion->GetPredictionDelta();
			}
		}
	} else {
		osg::notify(osg::WARN) << "Warning: Unable to find HMD Device, will use default renderpath instead." << std::endl;
	}
}

OculusDevice::~OculusDevice()
{
	if (m_sensorFusion) {
		// Detach sensor
		m_sensorFusion->AttachToSensor(NULL);
		delete m_sensorFusion;
		m_sensorFusion = NULL;
	}

	delete m_hmdInfo;
	m_hmdInfo = NULL;
	m_sensor.Clear();
	m_hmdDevice.Clear();
	m_deviceManager.Clear();

	// Do a nice shutdown of the Oculus HMD
	if (OVR::System::IsInitialized()) {
		OVR::System::Destroy();
	}
}

unsigned int OculusDevice::hScreenResolution() const
{
	if (m_hmdInfo) {
		return m_hmdInfo->HResolution;
	}

	// Default value from dev kit
	return 1280;
}

unsigned int OculusDevice::vScreenResolution() const
{
	if (m_hmdInfo) {
		return m_hmdInfo->VResolution;
	}

	// Default value from dev kit
	return 800;
}

float OculusDevice::hScreenSize() const
{
	if (m_hmdInfo) {
		return m_hmdInfo->HScreenSize;
	}

	// Default value from dev kit
	return 0.14976f;
}

float OculusDevice::vScreenSize() const
{
	if (m_hmdInfo) {
		return m_hmdInfo->VScreenSize;
	}

	// Default value from dev kit
	return 0.0936f;
}

float OculusDevice::vScreenCenter() const
{
	if (m_hmdInfo) {
		return m_hmdInfo->VScreenCenter;
	}

	return vScreenSize()*0.5f;
}

float OculusDevice::eyeToScreenDistance() const
{
	if (m_hmdInfo) {
		return m_hmdInfo->EyeToScreenDistance;
	}

	// Default value from dev kit
	return 0.041f;
}

float OculusDevice::lensSeparationDistance() const
{
	if (m_hmdInfo) {
		return m_hmdInfo->LensSeparationDistance;
	}

	// Default value from dev kit
	return 0.0635f;
}


float OculusDevice::interpupillaryDistance() const
{
	if (m_hmdInfo) {
		return m_hmdInfo->InterpupillaryDistance;
	}

	// Default value from dev kit, typical values for average human male
	return 0.064f;
}

osg::Matrix OculusDevice::viewMatrix(EyeSide eye)  const
{
	osg::Matrix viewMatrix;

	if (eye == LEFT_EYE) {
		viewMatrix.makeTranslate(osg::Vec3f(halfIPD(), 0.0f, 0.0f));
	} else if (eye == RIGHT_EYE) {
		viewMatrix.makeTranslate(osg::Vec3f(-halfIPD(), 0.0f, 0.0f));
	} else {
		viewMatrix.makeTranslate(osg::Vec3f(0.0f, 0.0f, 0.0f));
	}

	return viewMatrix;
}

osg::Matrix OculusDevice::projectionMatrix(EyeSide eye) const
{
	osg::Matrixf projectionMatrix = projectionOffsetMatrix(eye);
	projectionMatrix.preMult(projectionCenterMatrix());
	return projectionMatrix;
}

osg::Matrix OculusDevice::projectionCenterMatrix() const
{
	osg::Matrix projectionMatrix;
	float halfScreenDistance = vScreenSize() * 0.5f * distortionScale();
	float yFov = (180.0f/3.14159f) * 2.0f * atan(halfScreenDistance / eyeToScreenDistance());
	projectionMatrix.makePerspective(yFov, aspectRatio(), m_nearClip, m_farClip);
	return projectionMatrix;
}

osg::Matrix OculusDevice::projectionOffsetMatrix(EyeSide eye) const
{
	osg::Matrix projectionMatrix;
	float eyeProjectionShift = viewCenter() - lensSeparationDistance()*0.5f;
	float projectionCenterOffset = 4.0f * eyeProjectionShift / hScreenSize();

	if (eye == LEFT_EYE) {
		projectionMatrix.makeTranslate(osg::Vec3d(projectionCenterOffset, 0, 0));
	} else if (eye == RIGHT_EYE) {
		projectionMatrix.makeTranslate(osg::Vec3d(-projectionCenterOffset, 0, 0));
	} else {
		projectionMatrix.makeIdentity();
	}

	return projectionMatrix;
}

osg::Vec2f OculusDevice::lensCenter(EyeSide eye) const
{
	if (eye == LEFT_EYE) {
		return osg::Vec2f(1.0f-lensSeparationDistance()/hScreenSize(), 0.5f);
	} else if (eye == RIGHT_EYE) {
		return osg::Vec2f(lensSeparationDistance()/hScreenSize(), 0.5f);
	}

	return osg::Vec2f(0.5f, 0.5f);
}

osg::Vec2f OculusDevice::screenCenter() const
{
	return osg::Vec2f(0.5f, 0.5f);
}

osg::Vec2f OculusDevice::scale() const
{
	float scaleFactor = 1.0f/distortionScale();
	return osg::Vec2f(0.5*scaleFactor, 0.5f*scaleFactor*aspectRatio());
}

osg::Vec2f OculusDevice::scaleIn() const
{
	return osg::Vec2f(2.0f, 2.0f/aspectRatio());
}

osg::Vec4f OculusDevice::warpParameters() const
{
	if (m_hmdInfo) {
		return osg::Vec4f(m_hmdInfo->DistortionK[0], m_hmdInfo->DistortionK[1], m_hmdInfo->DistortionK[2], m_hmdInfo->DistortionK[3]);
	}

	// Default values from devkit
	return osg::Vec4f(1.0f, 0.22f, 0.24f, 0.0f);
}

osg::Vec4f OculusDevice::chromAbParameters() const
{
	if (m_hmdInfo) {
		return osg::Vec4f(m_hmdInfo->ChromaAbCorrection[0], m_hmdInfo->ChromaAbCorrection[1], m_hmdInfo->ChromaAbCorrection[2], m_hmdInfo->ChromaAbCorrection[3]);
	}

	// Default values from devkit
	return osg::Vec4f(0.996f, -0.004f, 1.014f, 0.0f);
}

osg::Quat OculusDevice::getOrientation() const
{
	// Create identity quaternion
	osg::Quat osgQuat(0.0f, 0.0f, 0.0f, 1.0f);

	if (m_sensorFusion && m_sensorFusion->IsAttachedToSensor()) {
		OVR::Quatf quat;

		if (m_sensorFusion->IsPredictionEnabled()) {
			quat = m_sensorFusion->GetPredictedOrientation(m_predictionDelta);
		} else {
			quat = m_sensorFusion->GetOrientation();
		}

		osgQuat.set(quat.x, quat.y, quat.z, -quat.w);
	}

	return osgQuat;
}

void OculusDevice::setSensorPredictionEnabled(bool prediction)
{
	if (m_sensorFusion) {
		m_sensorFusion->SetPredictionEnabled(prediction);
	}
}

float OculusDevice::distortionScale() const
{
	// Disable distortion scale calculation and use user suppied value instead
	if (m_useCustomScaleFactor) {
		return m_customScaleFactor;
	}

	float lensShift = hScreenSize() * 0.25f - lensSeparationDistance() * 0.5f;
	float lensViewportShift = 4.0f * lensShift / hScreenSize();
	float fitRadius = fabs(-1 - lensViewportShift);
	float rsq = fitRadius*fitRadius;
	osg::Vec4f k = warpParameters();
	float scale = (k[0] + k[1] * rsq + k[2] * rsq * rsq + k[3] * rsq * rsq * rsq);
	return scale;
}