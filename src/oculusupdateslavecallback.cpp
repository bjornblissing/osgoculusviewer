/*
 * oculusupdateslavecallback.cpp
 *
 *  Created on: Jul 07, 2015
 *      Author: Björn Blissing
 */

#include "oculusupdateslavecallback.h"

void OculusUpdateSlaveCallback::updateSlave(osg::View& view, osg::View::Slave& slave)
{
	if (m_cameraType == LEFT_CAMERA)
	{
		m_device->waitToBeginFrame(m_swapCallback->frameIndex());
		m_device->beginFrame(m_swapCallback->frameIndex());
		m_device->updatePose(m_swapCallback->frameIndex());
	}

	// Get the view and projection matrix for the view
	osg::Matrix viewMatrix = m_device->viewMatrix(m_cameraType == LEFT_CAMERA ? OculusDevice::Eye::LEFT : OculusDevice::Eye::RIGHT);
	osg::Matrix projectionMatrix = m_device->projectionMatrix(m_cameraType == LEFT_CAMERA ? OculusDevice::Eye::LEFT : OculusDevice::Eye::RIGHT);

	// Get the HMD position and orientation in world space relative tracking origin
	osg::Vec3 position = m_device->position(m_cameraType == LEFT_CAMERA ? OculusDevice::Eye::LEFT : OculusDevice::Eye::RIGHT);
	osg::Quat orientation = m_device->orientation(m_cameraType == LEFT_CAMERA ? OculusDevice::Eye::LEFT : OculusDevice::Eye::RIGHT);

	// invert orientation (conjugate of Quaternion) and position to apply to the view matrix as offset
	viewMatrix.preMultRotate(orientation.conj());
	viewMatrix.preMultTranslate(-position);

	slave._camera.get()->setViewMatrix(view.getCamera()->getViewMatrix()*viewMatrix);
	slave._camera.get()->setProjectionMatrix(projectionMatrix);

	slave.updateSlaveImplementation(view);
}
