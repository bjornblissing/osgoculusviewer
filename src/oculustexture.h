/*
 * oculustexture.h
 *
 *  Created on: Dec 05, 2017
 *      Author: Bjorn Blissing
 *
 *  MSAA support:
 *      Author: Chris Denham
 */

#ifndef _OSG_OCULUSTEXTURE_H_
#define _OSG_OCULUSTEXTURE_H_

// Include the OculusVR SDK
#include <OVR_CAPI_GL.h>

#include <osg/Texture2D>
#include <osg/Version>
#include <osg/FrameBufferObject>


#if(OSG_VERSION_GREATER_OR_EQUAL(3, 4, 0))
typedef osg::GLExtensions OSG_GLExtensions;
typedef osg::GLExtensions OSG_Texture_Extensions;
#else
typedef osg::FBOExtensions OSG_GLExtensions;
typedef osg::Texture::Extensions OSG_Texture_Extensions;
#endif

class OculusTextureBuffer : public osg::Referenced
{
public:
	OculusTextureBuffer(const ovrSession& session, osg::ref_ptr<osg::State> state, const ovrSizei& size, int msaaSamples);
	void destroy();
	int textureWidth() const { return m_textureSize.x(); }
	int textureHeight() const { return m_textureSize.y(); }
	int samples() const { return m_samples; }
	ovrTextureSwapChain textureSwapChain() const { return m_textureSwapChain; }
	osg::ref_ptr<osg::Texture2D> colorBuffer() const { return m_colorBuffer; }
	osg::ref_ptr<osg::Texture2D> depthBuffer() const { return m_depthBuffer; }
	void onPreRender(osg::RenderInfo& renderInfo);
	void onPostRender(osg::RenderInfo& renderInfo);

protected:
	~OculusTextureBuffer() {}

	const ovrSession m_session;
	ovrTextureSwapChain m_textureSwapChain;
	osg::ref_ptr<osg::Texture2D> m_colorBuffer;
	osg::ref_ptr<osg::Texture2D> m_depthBuffer;
	osg::Vec2i m_textureSize;

	void setup(osg::State& state);
	void setupMSAA(osg::State& state);

	GLuint m_Oculus_FBO; // MSAA FBO is copied to this FBO after render.
	GLuint m_MSAA_FBO; // framebuffer for MSAA texture
	GLuint m_MSAA_ColorTex; // color texture for MSAA
	GLuint m_MSAA_DepthTex; // depth texture for MSAA
	int m_samples;  // sample width for MSAA

};

class OculusMirrorTexture : public osg::Referenced
{
public:
	OculusMirrorTexture(ovrSession& session, osg::ref_ptr<osg::State> state, int width, int height);
	void destroy(osg::GraphicsContext* gc = 0);
	GLint width() const { return m_width; }
	GLint height() const { return m_height; }
	void blitTexture(osg::GraphicsContext* gc);
protected:
	~OculusMirrorTexture() {}

	const ovrSession m_session;
	ovrMirrorTexture m_texture;
	GLint m_width;
	GLint m_height;
	GLuint m_mirrorFBO;
};


class OculusPreDrawCallback : public osg::Camera::DrawCallback
{
public:
	OculusPreDrawCallback(osg::Camera* camera, OculusTextureBuffer* textureBuffer)
		: m_camera(camera)
		, m_textureBuffer(textureBuffer)
	{
	}

	virtual void operator()(osg::RenderInfo& renderInfo) const;
protected:
	osg::observer_ptr<osg::Camera> m_camera;
	osg::observer_ptr<OculusTextureBuffer> m_textureBuffer;

};

class OculusPostDrawCallback : public osg::Camera::DrawCallback
{
public:
	OculusPostDrawCallback(osg::Camera* camera, OculusTextureBuffer* textureBuffer)
		: m_camera(camera)
		, m_textureBuffer(textureBuffer)
	{
	}

	virtual void operator()(osg::RenderInfo& renderInfo) const;
protected:
	osg::observer_ptr<osg::Camera> m_camera;
	osg::observer_ptr<OculusTextureBuffer> m_textureBuffer;

};


#endif /* _OSG_OCULUSTEXTURE_H_ */
