/*
 * oculusdevice.h
 *
 *  Created on: Jul 03, 2013
 *      Author: Bjorn Blissing
 */

#ifndef _OSG_OCULUSDEVICE_H_
#define _OSG_OCULUSDEVICE_H_

#include "oculustexture.h"

class OculusDevice : public osg::Referenced
{

public:
	typedef enum Eye_
	{
		LEFT = 0,
		RIGHT = 1,
		COUNT = 2
	} Eye;

	typedef enum TrackingOrigin_
	{
		EYE_LEVEL = 0,
		FLOOR_LEVEL = 1
	} TrackingOrigin;

	OculusDevice(float nearClip, float farClip, const float pixelsPerDisplayPixel, const float worldUnitsPerMetre, const int samples, TrackingOrigin origin, const int mirrorTextureWidth);
	void createRenderBuffers(osg::ref_ptr<osg::State> state);
	void init();

	void destroyTextures(osg::GraphicsContext* gc);
	bool hmdPresent() const;

	unsigned int screenResolutionWidth() const { return m_hmdDesc.Resolution.w; }
	unsigned int screenResolutionHeight() const { return  m_hmdDesc.Resolution.h; }

	float nearClip() const { return m_nearClip;	}
	float farClip() const { return m_farClip; }

	void resetSensorOrientation() const { ovr_RecenterTrackingOrigin(m_session); }

	void updatePose(long long frameIndex);

	osg::Vec3 position(Eye eye) const;
	osg::Quat orientation(Eye eye) const;

	osg::Matrixf viewMatrix(Eye eye) const;
	osg::Matrixf projectionMatrix(Eye eye) const;

	osg::Camera* createRTTCamera(OculusDevice::Eye eye, osg::Transform::ReferenceFrame referenceFrame, const osg::Vec4& clearColor, osg::GraphicsContext* gc = 0) const;

	bool waitToBeginFrame(long long frameIndex = 0);
	bool beginFrame(long long frameIndex = 0);

	bool submitFrame(long long frameIndex = 0);
	void blitMirrorTexture(osg::GraphicsContext* gc);

	void setPerfHudMode(int mode);

	bool touchControllerAvailable() const;
	ovrInputState touchControllerState() const { return m_controllerState; }

	ovrPoseStatef handPoseLeft() const { return m_handPoses[ovrHand_Left]; }
	ovrPoseStatef handPoseRigth() const { return m_handPoses[ovrHand_Right]; }

	osg::GraphicsContext::Traits* graphicsContextTraits() const;
protected:
	~OculusDevice(); // Since we inherit from osg::Referenced we must make destructor protected

	void printHMDDebugInfo();

	void getEyeRenderDesc();

	void setTrackingOrigin();

	void setupLayers();

	void trySetProcessAsHighPriority() const;

	ovrSession m_session;
	ovrHmdDesc m_hmdDesc;
	ovrInputState m_controllerState;
	ovrPoseStatef m_handPoses[2];

	const float m_pixelsPerDisplayPixel;
	const float m_worldUnitsPerMetre;

	osg::ref_ptr<OculusTextureBuffer> m_textureBuffer[2];
	osg::ref_ptr<OculusMirrorTexture> m_mirrorTexture;

	unsigned int m_mirrorTextureWidth;

	ovrEyeRenderDesc m_eyeRenderDesc[2];
	ovrVector2f m_UVScaleOffset[2][2];
	double m_sensorSampleTime;
	ovrPosef m_eyeRenderPose[2];
	ovrLayerEyeFov m_layerEyeFov;

	osg::Vec3 m_position;
	osg::Quat m_orientation;

	float m_nearClip;
	float m_farClip;
	int m_samples;
	TrackingOrigin m_origin;
private:
	OculusDevice(const OculusDevice&); // Do not allow copy
	OculusDevice& operator=(const OculusDevice&); // Do not allow assignment operator.
};


class OculusRealizeOperation : public osg::GraphicsOperation
{
public:
	explicit OculusRealizeOperation(osg::ref_ptr<OculusDevice> device) :
		osg::GraphicsOperation("OculusRealizeOperation", false), m_device(device), m_realized(false) {}
	virtual void operator () (osg::GraphicsContext* gc);
	bool realized() const { return m_realized; }
protected:
	OpenThreads::Mutex  _mutex;
	osg::observer_ptr<OculusDevice> m_device;
	bool m_realized;
};


class OculusCleanUpOperation : public osg::GraphicsOperation
{
	public:
		explicit OculusCleanUpOperation(osg::ref_ptr<OculusDevice> device) :
		osg::GraphicsOperation("OculusCleanupOperation", false), m_device(device) {}
		virtual void operator () (osg::GraphicsContext* gc);
	protected:
		osg::observer_ptr<OculusDevice> m_device;
};

class OculusSwapCallback : public osg::GraphicsContext::SwapCallback
{
public:
	explicit OculusSwapCallback(osg::ref_ptr<OculusDevice> device) : m_device(device), m_frameIndex(0) {}
	void swapBuffersImplementation(osg::GraphicsContext* gc);
	long long frameIndex() const { return m_frameIndex; }
private:
	osg::observer_ptr<OculusDevice> m_device;
	long long m_frameIndex;
};

class OculusInitialDrawCallback : public osg::Camera::DrawCallback
{
public:
   virtual void operator()(osg::RenderInfo& renderInfo) const;
};

#endif /* _OSG_OCULUSDEVICE_H_ */
