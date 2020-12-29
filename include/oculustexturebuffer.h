/*
 * oculustexturebuffer.h
 *
 *  Created on: Dec 05, 2017
 *      Author: Bjorn Blissing
 *
 *  MSAA support:
 *      Author: Chris Denham
 */

#ifndef _OSG_OCULUSTEXTURE_H_
#define _OSG_OCULUSTEXTURE_H_

#include "oculusconfig.h"

#include <osg/Texture2D>

#include <osg/FrameBufferObject>

#include <OVR_CAPI_GL.h>

class OSGOCULUS_EXPORT OculusTextureBuffer : public osg::Referenced {
 public:
  OculusTextureBuffer(const ovrSession& session,
                      osg::State* state,
                      const ovrSizei& size,
                      int msaaSamples);
  void destroy(osg::GraphicsContext* gc);
  int textureWidth() const {
    return m_textureSize.x();
  }
  int textureHeight() const {
    return m_textureSize.y();
  }
  int samples() const {
    return m_samples;
  }
  ovrTextureSwapChain colorTextureSwapChain() const {
    return m_colorTextureSwapChain;
  }
  ovrTextureSwapChain depthTextureSwapChain() const {
    return m_depthTextureSwapChain;
  }
  osg::Texture2D* colorBuffer() const {
    return m_colorBuffer.get();
  }
  osg::Texture2D* depthBuffer() const {
    return m_depthBuffer.get();
  }
  void onPreRender(osg::RenderInfo& renderInfo);
  void onPostRender(osg::RenderInfo& renderInfo);

 private:
  const ovrSession m_session;
  ovrTextureSwapChain m_colorTextureSwapChain = {nullptr};
  ovrTextureSwapChain m_depthTextureSwapChain = {nullptr};
  osg::ref_ptr<osg::Texture2D> m_colorBuffer = {nullptr};
  osg::ref_ptr<osg::Texture2D> m_depthBuffer = {nullptr};
  osg::Vec2i m_textureSize;

  void setup(osg::State* state);
  void setupMSAA(osg::State* state);

  GLuint m_Oculus_FBO = {0};     // MSAA FBO is copied to this FBO after render.
  GLuint m_MSAA_FBO = {0};       // framebuffer for MSAA texture
  GLuint m_MSAA_ColorTex = {0};  // color texture for MSAA
  GLuint m_MSAA_DepthTex = {0};  // depth texture for MSAA
  int m_samples = {1};           // sample width for MSAA
};

#endif /* _OSG_OCULUSTEXTURE_H_ */
