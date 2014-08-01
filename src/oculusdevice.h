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
		OculusDevice();

		unsigned int hScreenResolution() const;
		unsigned int vScreenResolution() const;

		unsigned int hRenderTargetSize() const;
		unsigned int vRenderTargetSize() const;

		osg::Matrix projectionMatrixCenter() const;
		osg::Matrix projectionMatrixLeft() const;
		osg::Matrix projectionMatrixRight() const;

		osg::Matrix projectionOffsetMatrixLeft() const;
		osg::Matrix projectionOffsetMatrixRight() const;

		osg::Matrix viewMatrixLeft() const;
		osg::Matrix viewMatrixRight() const;

		void setNearClip(float nearClip) { m_nearClip = nearClip; }
		void setFarClip(float farclip) { m_farClip = farclip; }

		void resetSensorOrientation();
		osg::Quat getOrientation() const;
		
		osg::Geode* distortionMesh(int eyeNum, osg::Program* program, int x, int y, int w, int h);
		osg::Vec2f eyeToSourceUVScale(int eyeNum) const;
		osg::Vec2f eyeToSourceUVOffset(int eyeNum) const;

	protected:
		~OculusDevice(); // Since we inherit from osg::Referenced we must make destructor protected

		ovrHmd m_hmdDevice;
		ovrSizei m_resolution;
		ovrSizei m_renderTargetSize;
		ovrEyeRenderDesc m_eyeRenderDesc[2];
		ovrVector2f m_UVScaleOffset[2][2];

		osg::Matrixf m_leftEyeProjectionMatrix;
		osg::Matrixf m_rightEyeProjectionMatrix;
		osg::Vec3f m_leftEyeAdjust;
		osg::Vec3f m_rightEyeAdjust;

		float m_nearClip;
		float m_farClip;
	private:
		OculusDevice(const OculusDevice&); // Do not allow copy
};

#endif /* _OSG_OCULUSDEVICE_H_ */
