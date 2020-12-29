/*
 * oculusswapcallback.cpp
 *
 *  Created on: Dec 13, 2020
 *      Author: Bjorn Blissing
 */

#include <oculusdevice.h>
#include <oculusswapcallback.h>

void OculusSwapCallback::swapBuffersImplementation(osg::GraphicsContext* gc) {
  // Submit rendered frame to compositor
  m_device->submitFrame(m_frameIndex++);

  // Blit mirror texture to backbuffer,
  // if not already doing the blit on post draw
  if (!m_device->blitOnPostDraw())
    m_device->blitMirrorTexture(gc);

  // Run the default system swapBufferImplementation
  gc->swapBuffersImplementation();
}
