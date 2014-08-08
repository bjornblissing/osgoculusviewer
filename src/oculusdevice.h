/*
 * oculusdevice.h
 *
 *  Created on: Jul 03, 2013
 *      Author: Bjorn Blissing
 */

#ifndef _OSG_OCULUSDEVICE_H_
#define _OSG_OCULUSDEVICE_H_

// Include the OculusVR SDK
#include "OVR.h"
#include <osg/Matrix>
#include <osg/Vec3>
#include <osg/Geode>
#include <osg/Program>


class OculusDevice : public osg::Referenced {

	public:
		enum Eye
		{
			LEFT = 0,
			RIGHT = 1,
			COUNT = 2
		};
		OculusDevice();

		unsigned int hScreenResolution() const;
		unsigned int vScreenResolution() const;

		unsigned int hRenderTargetSize() const;
		unsigned int vRenderTargetSize() const;

		int displayId() const { return m_hmdDevice->DisplayId; }
		osg::Vec2i windowPos() const { return osg::Vec2i(m_hmdDevice->WindowsPos.x, m_hmdDevice->WindowsPos.y);  }

		osg::Matrix projectionMatrixCenter() const;
		osg::Matrix projectionMatrixLeft() const;
		osg::Matrix projectionMatrixRight() const;

		osg::Matrix projectionOffsetMatrixLeft() const;
		osg::Matrix projectionOffsetMatrixRight() const;

		osg::Matrix viewMatrixLeft() const;
		osg::Matrix viewMatrixRight() const;

		void setNearClip(float nearClip) { m_nearClip = nearClip; }
		void setFarClip(float farclip) { m_farClip = farclip; }

		void resetSensorOrientation() const;
		void updatePose(unsigned int frameIndex = 0);
		
		osg::Vec3 position() const { return m_position; }
		osg::Quat orientation() const { return m_orientation;  }
		
		int renderOrder(Eye eye) const;
		osg::Geode* distortionMesh(Eye eye, osg::Program* program, int x, int y, int w, int h);
		osg::Vec2f eyeToSourceUVScale(Eye eye) const;
		osg::Vec2f eyeToSourceUVOffset(Eye eye) const;
		osg::Matrixf eyeRotationStart(Eye eye) const;
		osg::Matrixf eyeRotationEnd(Eye eye) const;

		void beginFrameTiming(unsigned int frameIndex=0);
		void endFrameTiming() const;
		void waitTillTime();
	protected:
		~OculusDevice(); // Since we inherit from osg::Referenced we must make destructor protected
		float aspectRatio(Eye eye) const;

		ovrHmd m_hmdDevice;
		ovrSizei m_resolution;
		ovrSizei m_renderTargetSize;
		ovrEyeRenderDesc m_eyeRenderDesc[2];
		ovrVector2f m_UVScaleOffset[2][2];
		ovrFrameTiming m_frameTiming;
		ovrPosef m_headPose[2];
		ovrMatrix4f m_timeWarpMatrices[2][2];

		osg::Matrixf m_leftEyeProjectionMatrix;
		osg::Matrixf m_rightEyeProjectionMatrix;
		osg::Vec3f m_leftEyeAdjust;
		osg::Vec3f m_rightEyeAdjust;

		osg::Vec3 m_position;
		osg::Quat m_orientation;

		float m_nearClip;
		float m_farClip;
	private:
		OculusDevice(const OculusDevice&); // Do not allow copy
};

#endif /* _OSG_OCULUSDEVICE_H_ */
