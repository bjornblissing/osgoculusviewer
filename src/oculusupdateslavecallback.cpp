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
		m_device->updatePose(m_swapCallback->frameIndex());
	}

	osg::Vec3 position = m_device->position();
	osg::Quat orientation = m_device->orientation();

	osg::Matrix viewMatrix = (m_cameraType == LEFT_CAMERA) ? m_device->viewMatrixLeft() : m_device->viewMatrixRight();
	osg::Matrix projectionMatrix = (m_cameraType == LEFT_CAMERA) ? m_device->projectionMatrixLeft() : m_device->projectionMatrixRight();

	// invert orientation (conjugate of Quaternion) and position to apply to the view matrix as offset
	viewMatrix.preMultRotate(orientation.conj());
	viewMatrix.preMultTranslate(-position);

	slave._camera.get()->setViewMatrix(view.getCamera()->getViewMatrix()*viewMatrix);
	slave._camera.get()->setProjectionMatrix(projectionMatrix);

	slave.updateSlaveImplementation(view);
}
