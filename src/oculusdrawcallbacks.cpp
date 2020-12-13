/*
 * oculusdrawcallbacks.cpp
 *
 *  Created on: Dec 12, 2020
 *      Author: Bjorn Blissing
 */
#include <osgViewer/Renderer>

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

void OculusPostDrawCallback::operator()(osg::RenderInfo& renderInfo) const {
  m_textureBuffer->onPostRender(renderInfo);
}