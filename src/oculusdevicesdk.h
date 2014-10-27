/*
* oculusdevicesdk.h
*
*  Created on: Oct 01, 2014
*      Author: Bjorn Blissing
*/
#ifndef _OSG_OCULUSDEVICESDK_H_
#define _OSG_OCULUSDEVICESDK_H_

// Include the OculusVR SDK
#include "OVR.h"
#include <Windows.h>

#include <osg/Referenced>
#include <osg/Matrix>
#include <osg/Texture2D>
#include <osg/Drawable>
#include <osg/Vec3>
#include <osgViewer/Viewer>

class OculusTexture2D : public osg::Texture2D {
	public:
		void setViewPort(int x, int y, int w, int h);
		void setContext(osg::ref_ptr<osg::GraphicsContext> context) { m_context = context; }
		ovrTexture getOvrTexture();
	private:
		ovrRecti m_viewPort;
		osg::ref_ptr<osg::GraphicsContext> m_context;
};

class OculusSwapCallback : public osg::GraphicsContext::SwapCallback {
	// Since Oculus calls swapbuffers we do not need OSG to do it
	 void swapBuffersImplementation(osg::GraphicsContext*) {}
};

class OculusDeviceSDK : public osg::Referenced {
	public:
		OculusDeviceSDK();
		void initialize();
		bool initialized() const { return m_initialized; }
		void initializeTexture(osg::ref_ptr<osg::GraphicsContext> context);
		void setupSlaveCameras(osgViewer::View* view);
		void configureRendering(HWND window, HDC dc, int backBufferMultisample);
		void attachToWindow(HWND window);
		void getEyePose();
		unsigned int screenResolutionWidth() const;
		unsigned int screenResolutionHeight() const;
		unsigned int renderTargetWidth() const { return	m_renderTargetSize.w; }
		unsigned int renderTargetHeight() const { return m_renderTargetSize.h; }
		osg::Matrix projectionMatrixLeft() const { return m_leftEyeProjectionMatrix; }
		osg::Matrix projectionMatrixRight() const { return m_rightEyeProjectionMatrix; }
		osg::Matrix projectionOffsetMatrixLeft() const;
		osg::Matrix projectionOffsetMatrixRight() const;
		osg::Matrix orthoProjectionMatrixLeft() const { return m_leftEyeOrthoProjectionMatrix; }
		osg::Matrix orthoProjectionMatrixRight() const { return m_rightEyeOrthoProjectionMatrix; }
		osg::Matrix viewMatrixLeft() const;
		osg::Matrix viewMatrixRight() const;
		osg::GraphicsContext::Traits* graphicsContextTraits() const;
		osg::Camera* createRTTCamera(osg::Texture* texture, int order, osg::GraphicsContext* gc) const;
		void beginFrame();
		void endFrame();
		void setInitialized(bool state) { m_initialized = state;  }
	protected:
		~OculusDeviceSDK(); // Since we inherit from osg::Referenced we must make destructor protected
		ovrHmd m_hmdDevice;
		ovrSizei m_resolution;
		ovrSizei m_renderTargetSize;
		ovrEyeRenderDesc m_eyeRenderDesc[2];
		ovrFovPort m_eyeFov[2];
		ovrPosef m_renderPose[2];
		osg::Matrixf m_leftEyeProjectionMatrix;
		osg::Matrixf m_rightEyeProjectionMatrix;
		osg::Matrixf m_leftEyeOrthoProjectionMatrix;
		osg::Matrixf m_rightEyeOrthoProjectionMatrix;
		osg::Vec3f m_leftEyeAdjust;
		osg::Vec3f m_rightEyeAdjust;
		osg::ref_ptr<OculusTexture2D> m_leftTexture;
		osg::ref_ptr<OculusTexture2D> m_rightTexture;
		unsigned int m_frameIndex;
		float m_nearClip;
		float m_farClip;
		bool m_initialized;
		bool m_frameBegin;

		bool m_vSyncEnabled;
		bool m_isLowPersistence;
		bool m_dynamicPrediction;
		bool m_displaySleep;
		bool m_mirrorToWindow;

		bool m_supportsSRGB;
		bool m_pixelLuminanceOverdrive;
		bool m_timewarpEnabled;
		bool m_timewarpNoJitEnabled;

		bool m_positionTrackingEnabled;
	private:
		OculusDeviceSDK(const OculusDeviceSDK&); // Do not allow copy
		OculusDeviceSDK& operator=(const OculusDeviceSDK&); // Do not allow assignment operator.
};

class OculusDrawable : public osg::Drawable {
public:
	OculusDrawable() : m_oculusDevice(0), m_view(0) {}
	/** Copy constructor using CopyOp to manage deep vs shallow copy. */
	OculusDrawable(const OculusDrawable& geometry, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);
	void setOculusDevice(OculusDeviceSDK* device) { m_oculusDevice = device; }
	void setView(osgViewer::View* view) { m_view = view;  }
	virtual void drawImplementation(osg::RenderInfo &renderInfo) const;
	virtual osg::Object* cloneType() const { return new OculusDrawable(); }
	virtual osg::Object* clone(const osg::CopyOp& copyop) const { return new OculusDrawable(*this, copyop); }
	virtual bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const OculusDrawable*>(obj) != NULL; }
	virtual const char* libraryName() const { return "osgOculus"; }
	virtual const char* className() const { return "OculusDrawable"; }
private:
	osg::observer_ptr<OculusDeviceSDK> m_oculusDevice;
	osg::observer_ptr<osgViewer::View> m_view;
};


#endif /* _OSG_OCULUSDEVICESDK_H_ */
