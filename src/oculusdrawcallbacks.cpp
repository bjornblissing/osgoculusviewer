/*
 * oculusdrawcallbacks.cpp
 *
 *  Created on: Dec 12, 2020
 *      Author: Bjorn Blissing
 */
#include <osgViewer/Renderer>

#include <oculusdevice.h>
#include <oculusdrawcallbacks.h>
#include <oculustexturebuffer.h>

void OculusInitialDrawCallback::operator()(osg::RenderInfo& renderInfo) const {
  osg::GraphicsOperation* graphicsOperation = renderInfo.getCurrentCamera()->getRenderer();
  osgViewer::Renderer* renderer = dynamic_cast<osgViewer::Renderer*>(graphicsOperation);
  if (renderer != nullptr) {
    // Disable normal OSG FBO camera setup because it will undo the MSAA FBO configuration.
    renderer->setCameraRequiresSetUp(false);
  }
}

void OculusPreDrawCallback::operator()(osg::RenderInfo& renderInfo) const {
  m_textureBuffer->onPreRender(renderInfo);
}

OculusPostDrawCallback::OculusPostDrawCallback(osg::Camera* camera,
                                               OculusTextureBuffer* textureBuffer,
                                               const OculusDevice* device,
                                               bool blit) :
    m_camera(camera),
    m_textureBuffer(textureBuffer),
    m_device(device),
    m_blit(blit) {}

void OculusPostDrawCallback::operator()(osg::RenderInfo& renderInfo) const {
  m_textureBuffer->onPostRender(renderInfo);
  if (m_blit)
    m_device->blitMirrorTexture(m_camera->getGraphicsContext());
}