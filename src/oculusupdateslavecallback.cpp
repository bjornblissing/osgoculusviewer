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

	osg::Matrix viewOffset = (m_cameraType == LEFT_CAMERA) ? m_device->viewMatrixLeft() : m_device->viewMatrixRight();

	// invert orientation and position to apply to the view matrix as offset
	orientation[3] *= -1.f;
	viewOffset.preMultRotate(orientation);
	viewOffset.preMultTranslate(-position);

	slave._viewOffset = viewOffset;

	slave.updateSlaveImplementation(view);
}
