#include "oculusgraphicsoperation.h"
/*
 * oculusgraphicsoperation.cpp
 *
 *  Created on: Dec 12, 2020
 *      Author: Bjorn Blissing
 */

#include <osgViewer/GraphicsWindow>

#include <oculusdevice.h>
#include <oculusgraphicsoperation.h>

void OculusRealizeOperation::operator()(osg::GraphicsContext* gc) {
  if (!m_realized) {
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    gc->makeCurrent();

    if (osgViewer::GraphicsWindow* window = dynamic_cast<osgViewer::GraphicsWindow*>(gc)) {
      // Run wglSwapIntervalEXT(0) to force VSync Off
      window->setSyncToVBlank(false);
    }

    // Set up the render buffers
    m_device->createRenderBuffers(gc->getState());

    // Init the oculus system
    m_device->init();
  }

  m_realized = true;
}

void OculusCleanUpOperation::operator()(osg::GraphicsContext* gc) {
  gc->makeCurrent();
  m_device->destroyTextures(gc);
}
