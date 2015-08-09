/*
 * oculusdevice.h
 *
 *  Created on: Jul 03, 2013
 *      Author: Bjorn Blissing
 */

#ifndef _OSG_OCULUSDEVICE_H_
#define _OSG_OCULUSDEVICE_H_

// Include the OculusVR SDK
#include <OVR_CAPI_GL.h>

#include <osg/Geode>
#include <osg/Texture2D>
#include <osg/FrameBufferObject>


class OculusTextureBuffer {
public:
	OculusTextureBuffer(const ovrHmd& hmd, osg::ref_ptr<osg::State> state, const ovrSizei& size);
	~OculusTextureBuffer() {}
	int textureWidth() const { return m_textureSize.x();  }
	int textureHeight() const { return m_textureSize.y(); }
	ovrSwapTextureSet* textureSet() const { return m_textureSet; }
	void createRenderBuffers(const ovrHmd& hmd, osg::ref_ptr<osg::State> state, const ovrSizei& size);
	osg::ref_ptr<osg::Texture2D> texture() const { return m_texture; }
	void advanceIndex() { m_textureSet->CurrentIndex = (m_textureSet->CurrentIndex + 1) % m_textureSet->TextureCount; }
	void setRenderSurface(const osg::FBOExtensions* fbo_ext);
	void initializeFboId(GLuint id) { m_fboId = id; m_fboIdInitialized = true; }
	bool isFboIdInitialized() const { return m_fboIdInitialized; }
protected:
	ovrSwapTextureSet* m_textureSet;
	osg::ref_ptr<osg::Texture2D> m_texture;
	osg::Vec2i m_textureSize;
	unsigned int m_contextId;
	GLuint m_fboId;
	bool m_fboIdInitialized;
};


class OculusDepthBuffer {
public:
	explicit OculusDepthBuffer(const ovrSizei& size, osg::ref_ptr<osg::State> state);
	~OculusDepthBuffer() {}
	int textureWidth() const { return m_textureSize.x(); }
	int textureHeight() const { return m_textureSize.y(); }
	osg::ref_ptr<osg::Texture2D> texture() const { return m_texture; }
	GLuint texId() const { return m_texId; }
	void setRenderSurface(const osg::FBOExtensions* fbo_ext);
protected:
	osg::ref_ptr<osg::Texture2D> m_texture;
	osg::Vec2i m_textureSize;
	GLuint m_texId;
};


class OculusPreDrawCallback : public osg::Camera::DrawCallback
{
public:
	OculusPreDrawCallback(osg::Camera* camera, OculusTextureBuffer* textureBuffer, OculusDepthBuffer* depthBuffer)
		: m_camera(camera)
		, m_textureBuffer(textureBuffer)
		, m_depthBuffer(depthBuffer)
	{
	}

	virtual void operator()(osg::RenderInfo& renderInfo) const;
protected:
	osg::Camera* m_camera;
	OculusTextureBuffer* m_textureBuffer;
	OculusDepthBuffer* m_depthBuffer;

};

class OculusDevice : public osg::Referenced {
	
	public:
		typedef enum Eye_
		{
			LEFT = 0,
			RIGHT = 1,
			COUNT = 2
		} Eye;
		OculusDevice(float nearClip, float farClip, const float pixelsPerDisplayPixel = 1.0f, const float worldUnitsPerMetre = 1.0f);
		void createRenderBuffers(osg::ref_ptr<osg::State> state);
		void init();

		unsigned int screenResolutionWidth() const;
		unsigned int screenResolutionHeight() const;

		osg::Matrix projectionMatrixCenter() const;
		osg::Matrix projectionMatrixLeft() const;
		osg::Matrix projectionMatrixRight() const;

		osg::Matrix projectionOffsetMatrixLeft() const;
		osg::Matrix projectionOffsetMatrixRight() const;

		osg::Matrix viewMatrixLeft() const;
		osg::Matrix viewMatrixRight() const;

		float nearClip() const { return m_nearClip;	}
		float farClip() const { return m_farClip; }
		bool useTimewarp() const { return m_useTimeWarp; }

		void resetSensorOrientation() const;
		void updatePose(unsigned int frameIndex = 0);
		
		osg::Vec3 position() const { return m_position; }
		osg::Quat orientation() const { return m_orientation;  }

		osg::Camera* createRTTCamera(OculusDevice::Eye eye, osg::Transform::ReferenceFrame referenceFrame, osg::GraphicsContext* gc = 0) const;
		
		bool submitFrame(unsigned int frameIndex = 0);

		void toggleMirrorToWindow();
		void toggleLowPersistence();
		void toggleDynamicPrediction();
		osg::GraphicsContext::Traits* graphicsContextTraits() const;
	protected:
		~OculusDevice(); // Since we inherit from osg::Referenced we must make destructor protected

		int renderOrder(Eye eye) const;

		void printHMDDebugInfo();

		void initializeEyeRenderDesc();
		// Note: this function requires you to run the previous function first.
		void calculateEyeAdjustment();
		// Note: this function requires you to run the previous function first.
		void calculateProjectionMatrices();

		void setupLayers();

		void trySetProcessAsHighPriority() const;
		
		ovrHmd m_hmdDevice;
		const float m_pixelsPerDisplayPixel;
		const float m_worldUnitsPerMetre;
		OculusTextureBuffer* m_textureBuffer[2];
		OculusDepthBuffer* m_depthBuffer[2];
		ovrSizei m_resolution;
		ovrEyeRenderDesc m_eyeRenderDesc[2];
		ovrVector2f m_UVScaleOffset[2][2];
		ovrFrameTiming m_frameTiming;
		ovrPosef m_headPose[2];
		ovrPosef m_eyeRenderPose[2];
		ovrLayerEyeFov m_layerEyeFov;
		ovrVector3f m_viewOffset[2];
		osg::Matrixf m_leftEyeProjectionMatrix;
		osg::Matrixf m_rightEyeProjectionMatrix;
		osg::Vec3f m_leftEyeAdjust;
		osg::Vec3f m_rightEyeAdjust;
		
		osg::Vec3 m_position;
		osg::Quat m_orientation;

		float m_nearClip;
		float m_farClip;
		bool m_useTimeWarp;
	private:
		OculusDevice(const OculusDevice&); // Do not allow copy
		OculusDevice& operator=(const OculusDevice&); // Do not allow assignment operator.
};


class OculusRealizeOperation : public osg::GraphicsOperation {
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


class OculusSwapCallback : public osg::GraphicsContext::SwapCallback {
public:
	explicit OculusSwapCallback(osg::ref_ptr<OculusDevice> device) : m_device(device), m_frameIndex(0) {}
	void swapBuffersImplementation(osg::GraphicsContext *gc);
	int frameIndex() const { return m_frameIndex; }
private:
	osg::observer_ptr<OculusDevice> m_device;
	int m_frameIndex;
};


#endif /* _OSG_OCULUSDEVICE_H_ */
