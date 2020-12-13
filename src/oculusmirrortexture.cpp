/*
 * oculusmirrortexture.cpp
 *
 *  Created on: Dec 13, 2020
 *      Author: Bjorn Blissing
 *
 */

#include <osg/FrameBufferObject>

#include <glextensions.h>
#include <oculusmirrortexture.h>

OculusMirrorTexture::OculusMirrorTexture(ovrSession& session,
                                         osg::ref_ptr<osg::State> state,
                                         int width,
                                         int height) :
    m_session(session),
    m_mirrorTexture(nullptr),
    m_width(width),
    m_height(height) {
  ovrMirrorTextureDesc desc;
  memset(&desc, 0, sizeof(desc));
  desc.Width = width;
  desc.Height = height;
  desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;

  // Create mirror texture and an FBO used to copy mirror texture to back buffer
  ovrResult result = ovr_CreateMirrorTextureWithOptionsGL(session, &desc, &m_mirrorTexture);
  if (!OVR_SUCCESS(result)) {
    osg::notify(osg::DEBUG_INFO) << "Failed to create mirror texture." << std::endl;
  }

  // Configure the mirror read buffer
  GLuint texId;
  ovr_GetMirrorTextureBufferGL(session, m_mirrorTexture, &texId);
  const OSG_GLExtensions* fbo_ext = getGLExtensions(*state);
  fbo_ext->glGenFramebuffers(1, &m_mirrorFBO);
  fbo_ext->glBindFramebuffer(GL_READ_FRAMEBUFFER_EXT, m_mirrorFBO);
  fbo_ext->glFramebufferTexture2D(GL_READ_FRAMEBUFFER_EXT,
                                  GL_COLOR_ATTACHMENT0_EXT,
                                  GL_TEXTURE_2D,
                                  texId,
                                  0);
  fbo_ext->glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER_EXT,
                                     GL_DEPTH_ATTACHMENT_EXT,
                                     GL_RENDERBUFFER_EXT,
                                     0);
  fbo_ext->glBindFramebuffer(GL_READ_FRAMEBUFFER_EXT, 0);
}

void OculusMirrorTexture::blitTexture(osg::GraphicsContext* gc) {
  const OSG_GLExtensions* fbo_ext = getGLExtensions(*(gc->getState()));
  // Blit mirror texture to back buffer
  fbo_ext->glBindFramebuffer(GL_READ_FRAMEBUFFER_EXT, m_mirrorFBO);
  fbo_ext->glBindFramebuffer(GL_DRAW_FRAMEBUFFER_EXT, 0);
  GLint w = m_width;
  GLint h = m_height;
  fbo_ext->glBlitFramebuffer(0, h, w, 0, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
  fbo_ext->glBindFramebuffer(GL_READ_FRAMEBUFFER_EXT, 0);
}

void OculusMirrorTexture::destroy(osg::GraphicsContext* gc) {
  if (gc) {
    const OSG_GLExtensions* fbo_ext = getGLExtensions(*(gc->getState()));
    fbo_ext->glDeleteFramebuffers(1, &m_mirrorFBO);
  }

  ovr_DestroyMirrorTexture(m_session, m_mirrorTexture);
}
