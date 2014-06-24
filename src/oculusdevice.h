/*
 * oculusdevice.h
 *
 *  Created on: Jul 03, 2013
 *      Author: Bjorn Blissing
 */

#ifndef _OSG_OCULUSDEVICE_H_
#define _OSG_OCULUSDEVICE_H_

#include <OVR.h>
#include <osg/Matrix>


class OculusDevice : public osg::Referenced {

	public:
		OculusDevice();

		enum EyeSide {
			CENTER_EYE	= 0,
			LEFT_EYE	= 1,
			RIGHT_EYE	= 2
		};

		unsigned int hScreenResolution() const;
		unsigned int vScreenResolution() const;
		float hScreenSize() const;
		float vScreenSize() const;
		float vScreenCenter() const;
		float eyeToScreenDistance() const;
		float lensSeparationDistance() const;
		float interpupillaryDistance() const;

		osg::Matrix viewMatrix(EyeSide eye = CENTER_EYE) const;
		osg::Matrix projectionMatrix(EyeSide eye = CENTER_EYE) const;
		osg::Matrix projectionOffsetMatrix(EyeSide eye = CENTER_EYE) const;
		osg::Matrix projectionCenterMatrix() const;

		osg::Vec2f lensCenter(EyeSide eye = CENTER_EYE) const;
		osg::Vec2f screenCenter() const;
		osg::Vec4f warpParameters() const;
		osg::Vec4f chromAbParameters() const;
		osg::Vec2f scale() const;
		osg::Vec2f scaleIn() const;

		float scaleFactor() const { return distortionScale(); }

		float aspectRatio() const {  return float (hScreenResolution()/2) / float (vScreenResolution()); }
		osg::Quat getOrientation() const;

		void setNearClip(float nearClip) { m_nearClip = nearClip; }
		void setFarClip(float farclip) { m_farClip = farclip; }

		void setSensorPredictionEnabled(bool prediction);
		void setSensorPredictionDelta(float delta) { m_predictionDelta = delta; }
		void setCustomScaleFactor(const float& customScaleFactor) { m_useCustomScaleFactor = true; m_customScaleFactor = customScaleFactor; }

		void resetSensorOrientation() { if (m_sensorFusion) m_sensorFusion->Reset(); }

	protected:
		~OculusDevice(); // Since we inherit from osg::Referenced we must make destructor protected
		float viewCenter() const { return hScreenSize() * 0.25f; }
		float halfIPD() const { return interpupillaryDistance() * 0.5f; }
		float distortionScale() const;

		OVR::Ptr<OVR::DeviceManager> m_deviceManager;
		OVR::Ptr<OVR::HMDDevice> m_hmdDevice;
		OVR::Ptr<OVR::SensorDevice> m_sensor;
		OVR::HMDInfo* m_hmdInfo;
		OVR::SensorFusion* m_sensorFusion;
		bool m_useCustomScaleFactor;
		float m_customScaleFactor;
		float m_nearClip;
		float m_farClip;
		float m_predictionDelta;
	private:
		OculusDevice(const OculusDevice&); // Do not allow copy
};

#endif /* _OSG_OCULUSDEVICE_H_ */
