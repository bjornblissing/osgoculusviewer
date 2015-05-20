/*
* oculusdevicesdk.h
*
*  Created on: Oct 01, 2014
*      Author: Bjorn Blissing
*/
#ifndef _OSG_OCULUSDEVICESDK_H_
#define _OSG_OCULUSDEVICESDK_H_

#ifdef _WIN32
#include <Windows.h>

#elif __linux__
#include <osgViewer/api/X11/GraphicsWindowX11>
#endif

// Include the OculusVR SDK
#include <OVR_CAPI.h>

#include <osg/Referenced>
#include <osg/Matrix>
#include <osg/Texture2D>
#include <osg/Drawable>
#include <osg/Vec3>
#include <osgViewer/Viewer>

#include "oculushealthwarning.h"

class OculusTexture2D : public osg::Texture2D {
	public:
		void setViewPort(int x, int y, int w, int h);
		void setContext(osg::ref_ptr<osg::GraphicsContext> context) { m_context = context; }
		ovrTexture getOvrTexture();
	private:
		ovrRecti m_viewPort;
		osg::ref_ptr<osg::GraphicsContext> m_context;
};

class OculusSwapCallbackSDK : public osg::GraphicsContext::SwapCallback {
	// Since Oculus calls swapbuffers we do not need OSG to do it
	 void swapBuffersImplementation(osg::GraphicsContext*) {}
};

// Forward declaration
class OculusRealizeOperation;

class OculusDeviceSDK : public osg::Referenced {
	friend class OculusRealizeOperation;
	public:
		enum Eye
		{
			LEFT = 0,
			RIGHT = 1,
			COUNT = 2
		};
		OculusDeviceSDK(float nearClip, float farClip, const float pixelsPerDisplayPixel = 1.0f, const float worldUnitsPerMetre = 1.0f);
		void resetSensorOrientation() const;
		void toggleMirrorToWindow();
		void toggleLowPersistence();
		void toggleDynamicPrediction();
		osg::GraphicsContext::Traits* graphicsContextTraits() const;
		void beginFrame();
		void endFrame();
		bool getHealthAndSafetyDisplayState();
		bool tryDismissHealthAndSafetyDisplay();
		osg::ref_ptr<OculusHealthAndSafetyWarning> healthWarning() const { return m_warning; }
	protected:
		~OculusDeviceSDK(); // Since we inherit from osg::Referenced we must make destructor protected

		int renderOrder(Eye eye) const; 
		
		void printHMDDebugInfo();
		
		void initializeEyeRenderDesc();
		// Note: this function requires you to run the previous function first.
		void calculateEyeAdjustment();
		// Note: this function requires you to run the previous function first.
		void calculateProjectionMatrices();

		unsigned int getCaps() const;
		unsigned int getDistortionCaps() const;

		osg::Matrix projectionMatrixCenter() const;
		osg::Matrix projectionMatrixLeft() const { return m_leftEyeProjectionMatrix; }
		osg::Matrix projectionMatrixRight() const { return m_rightEyeProjectionMatrix; }
		
		osg::Matrix projectionOffsetMatrixLeft() const;
		osg::Matrix projectionOffsetMatrixRight() const;
		
		osg::Matrix orthoProjectionMatrixLeft() const { return m_leftEyeOrthoProjectionMatrix; }
		osg::Matrix orthoProjectionMatrixRight() const { return m_rightEyeOrthoProjectionMatrix; }
		
		osg::Matrix viewMatrixLeft() const;
		osg::Matrix viewMatrixRight() const;

		float nearClip() const { return m_nearClip; }
		float farClip() const { return m_farClip; }

		void updatePose(const ovrPosef& pose, osg::Camera* camera, osg::Matrix viewMatrix);
	
		void initializeTextures(osg::ref_ptr<osg::GraphicsContext> context);
#if _WIN32
		void configureRendering(HWND window, HDC dc, int backBufferMultisample);
#elif __linux__
		void configureRendering(Display* disp, int backBufferMultisample);
#endif
		void setupSlaveCameras(osg::GraphicsContext* context, osgViewer::View* view);
		osg::Camera* createRTTCamera(osg::Texture* texture, osg::Texture* depthTexture, int order, osg::GraphicsContext* gc) const;
		// For direct mode
		bool attachToWindow(osg::ref_ptr<osg::GraphicsContext> gc);

		void setInitialized(bool state) { m_initialized = state; }
		

		void trySetProcessAsHighPriority() const;
		void applyExtendedModeSettings() const;

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
		osg::ref_ptr<OculusTexture2D> m_leftDepthTexture;
		osg::ref_ptr<OculusTexture2D> m_rightDepthTexture;
		osg::ref_ptr<osg::Camera> m_cameraRTTLeft;
		osg::ref_ptr<osg::Camera> m_cameraRTTRight;
		osg::ref_ptr<OculusHealthAndSafetyWarning> m_warning;
		osg::observer_ptr<osgViewer::View> m_view;

		const float m_pixelsPerDisplayPixel;
		const float m_worldUnitsPerMetre;
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

		bool m_positionTrackingEnabled;
		bool m_directMode;
	private:
		OculusDeviceSDK(const OculusDeviceSDK&); // Do not allow copy
		OculusDeviceSDK& operator=(const OculusDeviceSDK&); // Do not allow assignment operator.
};

class OculusRealizeOperation : public osg::GraphicsOperation {
public:
	OculusRealizeOperation(OculusDeviceSDK* device, osgViewer::View* view) :
		osg::GraphicsOperation("OculusRealizeOperation", false), m_oculusDevice(device), m_view(view), m_realized(false) {}
	virtual void operator () (osg::GraphicsContext* gc);
	bool realized() const { return m_realized; }
protected:
	OpenThreads::Mutex  _mutex;
	osg::observer_ptr<OculusDeviceSDK> m_oculusDevice;
	osg::observer_ptr<osgViewer::View> m_view;
	bool m_realized;
};

#endif /* _OSG_OCULUSDEVICESDK_H_ */
