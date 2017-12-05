/*
 * oculustexture.cpp
 *
 *  Created on: Dec 05, 2017
 *      Author: Bjorn Blissing
 *
 *  MSAA support:
 *      Author: Chris Denham
 */

#include "oculustexture.h"

#include <osgViewer/Renderer>


#ifndef GL_TEXTURE_MAX_LEVEL
#define GL_TEXTURE_MAX_LEVEL 0x813D
#endif

static const OSG_GLExtensions* getGLExtensions(const osg::State& state)
{
#if(OSG_VERSION_GREATER_OR_EQUAL(3, 4, 0))
	return state.get<osg::GLExtensions>();
#else
	return osg::FBOExtensions::instance(state.getContextID(), true);
#endif
}

static const OSG_Texture_Extensions* getTextureExtensions(const osg::State& state)
{
#if(OSG_VERSION_GREATER_OR_EQUAL(3, 4, 0))
	return state.get<osg::GLExtensions>();
#else
	return osg::Texture::getExtensions(state.getContextID(), true);
#endif
}

static osg::FrameBufferObject* getFrameBufferObject(osg::RenderInfo& renderInfo)
{
	osg::Camera* camera = renderInfo.getCurrentCamera();
	osgViewer::Renderer* camRenderer = (dynamic_cast<osgViewer::Renderer*>(camera->getRenderer()));

	if (camRenderer != nullptr)
	{
		osgUtil::SceneView* sceneView = camRenderer->getSceneView(0);

		if (sceneView != nullptr)
		{
			osgUtil::RenderStage* renderStage = sceneView->getRenderStage();

			if (renderStage != nullptr)
			{
				return renderStage->getFrameBufferObject();
			}
		}
	}

	return nullptr;
}

void OculusPreDrawCallback::operator()(osg::RenderInfo& renderInfo) const
{
	m_textureBuffer->onPreRender(renderInfo);
}

void OculusPostDrawCallback::operator()(osg::RenderInfo& renderInfo) const
{
	m_textureBuffer->onPostRender(renderInfo);
}

/* Public functions */
OculusTextureBuffer::OculusTextureBuffer(const ovrSession& session, osg::ref_ptr<osg::State> state, const ovrSizei& size, int msaaSamples) : m_session(session),
m_textureSwapChain(nullptr),
m_colorBuffer(nullptr),
m_depthBuffer(nullptr),
m_textureSize(osg::Vec2i(size.w, size.h)),
m_Oculus_FBO(0),
m_MSAA_FBO(0),
m_MSAA_ColorTex(0),
m_MSAA_DepthTex(0),
m_samples(msaaSamples)
{
	if (msaaSamples == 0)
	{
		setup(*state);
	}
	else
	{
		setupMSAA(*state);
	}

}

void OculusTextureBuffer::setup(osg::State& state)
{

	ovrTextureSwapChainDesc desc = {};
	desc.Type = ovrTexture_2D;
	desc.ArraySize = 1;
	desc.Width = m_textureSize.x();
	desc.Height = m_textureSize.y();
	desc.MipLevels = 1;
	desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
	desc.SampleCount = 1;
	desc.StaticImage = ovrFalse;

	ovrResult result = ovr_CreateTextureSwapChainGL(m_session, &desc, &m_textureSwapChain);

	int length = 0;
	ovr_GetTextureSwapChainLength(m_session, m_textureSwapChain, &length);

	if (OVR_SUCCESS(result))
	{
		for (int i = 0; i < length; ++i)
		{
			GLuint chainTexId;
			ovr_GetTextureSwapChainBufferGL(m_session, m_textureSwapChain, i, &chainTexId);

			osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D();

			osg::ref_ptr<osg::Texture::TextureObject> textureObject = new osg::Texture::TextureObject(texture.get(), chainTexId, GL_TEXTURE_2D);

			textureObject->setAllocated(true);

			texture->setTextureObject(state.getContextID(), textureObject.get());

			texture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
			texture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
			texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
			texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);

			texture->setTextureSize(m_textureSize.x(), m_textureSize.y());
			texture->setSourceFormat(GL_SRGB8_ALPHA8);

			// Set the color buffer to be the first texture generated
			if (i == 0)
			{
				m_colorBuffer = texture;
			}
		}

		osg::notify(osg::DEBUG_INFO) << "Successfully created the swap texture set!" << std::endl;
	}
	else
	{
		osg::notify(osg::WARN) << "Warning: Unable to create swap texture set! " << std::endl;
		return;
	}

	m_depthBuffer = new osg::Texture2D();
	m_depthBuffer->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
	m_depthBuffer->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
	m_depthBuffer->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	m_depthBuffer->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	m_depthBuffer->setSourceFormat(GL_DEPTH_COMPONENT);
	m_depthBuffer->setSourceType(GL_UNSIGNED_INT);
	m_depthBuffer->setInternalFormat(GL_DEPTH_COMPONENT24);
	m_depthBuffer->setTextureWidth(textureWidth());
	m_depthBuffer->setTextureHeight(textureHeight());
	m_depthBuffer->setBorderWidth(0);

}


void OculusTextureBuffer::setupMSAA(osg::State& state)
{

	const OSG_GLExtensions* fbo_ext = getGLExtensions(state);


	ovrTextureSwapChainDesc desc = {};
	desc.Type = ovrTexture_2D;
	desc.ArraySize = 1;
	desc.Width = m_textureSize.x();
	desc.Height = m_textureSize.y();
	desc.MipLevels = 1;
	desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
	desc.SampleCount = 1;
	desc.StaticImage = ovrFalse;

	ovrResult result = ovr_CreateTextureSwapChainGL(m_session, &desc, &m_textureSwapChain);

	int length = 0;
	ovr_GetTextureSwapChainLength(m_session, m_textureSwapChain, &length);

	if (OVR_SUCCESS(result))
	{
		for (int i = 0; i < length; ++i)
		{
			GLuint chainTexId;
			ovr_GetTextureSwapChainBufferGL(m_session, m_textureSwapChain, i, &chainTexId);
			glBindTexture(GL_TEXTURE_2D, chainTexId);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}

		osg::notify(osg::DEBUG_INFO) << "Successfully created the swap texture set!" << std::endl;
	}

	// Create an FBO for output to Oculus swap texture set.
	fbo_ext->glGenFramebuffers(1, &m_Oculus_FBO);

	// Create an FBO for primary render target.
	fbo_ext->glGenFramebuffers(1, &m_MSAA_FBO);

	// We don't want to support MIPMAP so, ensure only level 0 is allowed.
	const int maxTextureLevel = 0;

	const OSG_Texture_Extensions* extensions = getTextureExtensions(state);

	// Create MSAA color buffer
	glGenTextures(1, &m_MSAA_ColorTex);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSAA_ColorTex);
	extensions->glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_samples, GL_RGBA, m_textureSize.x(), m_textureSize.y(), false);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER_ARB);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER_ARB);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, maxTextureLevel);

	// Create MSAA depth buffer
	glGenTextures(1, &m_MSAA_DepthTex);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSAA_DepthTex);
	extensions->glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_samples, GL_DEPTH_COMPONENT, m_textureSize.x(), m_textureSize.y(), false);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, maxTextureLevel);
}

void OculusTextureBuffer::onPreRender(osg::RenderInfo& renderInfo)
{
	GLuint curTexId = 0;
	if (m_textureSwapChain)
	{
		int curIndex;
		ovr_GetTextureSwapChainCurrentIndex(m_session, m_textureSwapChain, &curIndex);
		ovr_GetTextureSwapChainBufferGL(m_session, m_textureSwapChain, curIndex, &curTexId);
	}

	osg::State& state = *renderInfo.getState();
	const OSG_GLExtensions* fbo_ext = getGLExtensions(state);

	if (m_samples == 0)
	{
		osg::FrameBufferObject* fbo = getFrameBufferObject(renderInfo);

		if (fbo == nullptr)
		{
			return;
		}

		const unsigned int ctx = state.getContextID();
		fbo_ext->glBindFramebuffer(GL_FRAMEBUFFER_EXT, fbo->getHandle(ctx));
		fbo_ext->glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, curTexId, 0);
		fbo_ext->glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, m_depthBuffer->getTextureObject(state.getContextID())->id(), 0);
	}
	else
	{
		fbo_ext->glBindFramebuffer(GL_FRAMEBUFFER_EXT, m_MSAA_FBO);
		fbo_ext->glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D_MULTISAMPLE, m_MSAA_ColorTex, 0);
		fbo_ext->glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D_MULTISAMPLE, m_MSAA_DepthTex, 0);
	}

}
void OculusTextureBuffer::onPostRender(osg::RenderInfo& renderInfo)
{

	if (m_textureSwapChain) {
		ovr_CommitTextureSwapChain(m_session, m_textureSwapChain);
	}

	if (m_samples == 0)
	{
		// Nothing to do here if MSAA not being used.
		return;
	}

	// Get texture id
	GLuint curTexId = 0;
	if (m_textureSwapChain)
	{
		int curIndex;
		ovr_GetTextureSwapChainCurrentIndex(m_session, m_textureSwapChain, &curIndex);
		ovr_GetTextureSwapChainBufferGL(m_session, m_textureSwapChain, curIndex, &curTexId);
	}

	osg::State& state = *renderInfo.getState();
	const OSG_GLExtensions* fbo_ext = getGLExtensions(state);

	fbo_ext->glBindFramebuffer(GL_READ_FRAMEBUFFER_EXT, m_MSAA_FBO);
	fbo_ext->glFramebufferTexture2D(GL_READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D_MULTISAMPLE, m_MSAA_ColorTex, 0);
	fbo_ext->glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, 0);

	fbo_ext->glBindFramebuffer(GL_DRAW_FRAMEBUFFER_EXT, m_Oculus_FBO);
	fbo_ext->glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, curTexId, 0);
	fbo_ext->glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, 0);

	int w = m_textureSize.x();
	int h = m_textureSize.y();
	fbo_ext->glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	fbo_ext->glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);

}

void OculusTextureBuffer::destroy()
{
	ovr_DestroyTextureSwapChain(m_session, m_textureSwapChain);
}

OculusMirrorTexture::OculusMirrorTexture(ovrSession& session, osg::ref_ptr<osg::State> state, int width, int height) : m_session(session), m_texture(nullptr), m_width(width), m_height(height)
{

	ovrMirrorTextureDesc desc;
	memset(&desc, 0, sizeof(desc));
	desc.Width = width;
	desc.Height = height;
	desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;

	// Create mirror texture and an FBO used to copy mirror texture to back buffer
	ovrResult result = ovr_CreateMirrorTextureGL(session, &desc, &m_texture);
	if (!OVR_SUCCESS(result))
	{
		osg::notify(osg::DEBUG_INFO) << "Failed to create mirror texture." << std::endl;
	}

	// Configure the mirror read buffer
	GLuint texId;
	ovr_GetMirrorTextureBufferGL(session, m_texture, &texId);
	const OSG_GLExtensions* fbo_ext = getGLExtensions(*state);
	fbo_ext->glGenFramebuffers(1, &m_mirrorFBO);
	fbo_ext->glBindFramebuffer(GL_READ_FRAMEBUFFER_EXT, m_mirrorFBO);
	fbo_ext->glFramebufferTexture2D(GL_READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, texId, 0);
	fbo_ext->glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, 0);
	fbo_ext->glBindFramebuffer(GL_READ_FRAMEBUFFER_EXT, 0);
}

void OculusMirrorTexture::blitTexture(osg::GraphicsContext* gc)
{
	const OSG_GLExtensions* fbo_ext = getGLExtensions(*(gc->getState()));
	// Blit mirror texture to back buffer
	fbo_ext->glBindFramebuffer(GL_READ_FRAMEBUFFER_EXT, m_mirrorFBO);
	fbo_ext->glBindFramebuffer(GL_DRAW_FRAMEBUFFER_EXT, 0);
	GLint w = m_width;
	GLint h = m_height;
	fbo_ext->glBlitFramebuffer(0, h, w, 0,
		0, 0, w, h,
		GL_COLOR_BUFFER_BIT, GL_NEAREST);
	fbo_ext->glBindFramebuffer(GL_READ_FRAMEBUFFER_EXT, 0);
}

void OculusMirrorTexture::destroy(osg::GraphicsContext* gc)
{
	if (gc)
	{
		const OSG_GLExtensions* fbo_ext = getGLExtensions(*(gc->getState()));
		fbo_ext->glDeleteFramebuffers(1, &m_mirrorFBO);
	}

	ovr_DestroyMirrorTexture(m_session, m_texture);
}
