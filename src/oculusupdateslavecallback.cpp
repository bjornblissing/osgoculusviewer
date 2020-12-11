/*
 * oculusupdateslavecallback.cpp
 *
 *  Created on: Jul 07, 2015
 *      Author: Björn Blissing
 */

#include "oculusupdateslavecallback.h"

void OculusUpdateSlaveCallback::updateSlave(osg::View& view, osg::View::Slave& slave) {
  // We need to call these functions for the first camera, which currently is the left camera
  if (m_cameraType == LEFT_CAMERA) {
    m_device->waitToBeginFrame(m_swapCallback->frameIndex());
    m_device->beginFrame(m_swapCallback->frameIndex());
    m_device->updatePose(m_swapCallback->frameIndex());
  }

  // Get the view and projection matrix for the view
  osg::Matrix viewMatrix = m_device->viewMatrix(
    m_cameraType == LEFT_CAMERA ? OculusDevice::Eye::LEFT : OculusDevice::Eye::RIGHT);
  osg::Matrix projectionMatrix = m_device->projectionMatrix(
    m_cameraType == LEFT_CAMERA ? OculusDevice::Eye::LEFT : OculusDevice::Eye::RIGHT);

  slave._camera.get()->setViewMatrix(view.getCamera()->getViewMatrix() * viewMatrix);
  slave._camera.get()->setProjectionMatrix(projectionMatrix);
  m_device->updateTimewarpProjection(m_cameraType == LEFT_CAMERA ? OculusDevice::Eye::LEFT :
                                                                   OculusDevice::Eye::RIGHT);

  slave.updateSlaveImplementation(view);
}
