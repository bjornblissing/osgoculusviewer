/*
 * oculusviewer.cpp
 *
 *  Created on: Jun 30, 2013
 *      Author: Jan Ciger & Björn Blissing
 */

#include "oculusviewer.h"
#include "oculusupdateslavecallback.h"

/* Public functions */
void OculusViewer::traverse(osg::NodeVisitor& nv)
{
	// Must be realized before any traversal
	if (m_realizeOperation->realized())
	{
		if (!m_configured)
		{
			configure();
		}
	}

	osg::Group::traverse(nv);
}

/* Protected functions */
void OculusViewer::configure()
{
	osg::ref_ptr<osg::GraphicsContext> gc =  m_view->getCamera()->getGraphicsContext();

	// Attach a callback to detect swap
	osg::ref_ptr<OculusSwapCallback> swapCallback = new OculusSwapCallback(m_device);
	gc->setSwapCallback(swapCallback.get());

	osg::ref_ptr<osg::Camera> camera = m_view->getCamera();
	camera->setName("Main");
	osg::Vec4 clearColor = camera->getClearColor();

	// master projection matrix
	camera->setProjectionMatrix(m_device->projectionMatrixCenter());
	// Create RTT cameras and attach textures
	m_cameraRTTLeft = m_device->createRTTCamera(OculusDevice::LEFT, osg::Camera::RELATIVE_RF, clearColor, gc.get());
	m_cameraRTTRight = m_device->createRTTCamera(OculusDevice::RIGHT, osg::Camera::RELATIVE_RF, clearColor, gc.get());
	m_cameraRTTLeft->setName("LeftRTT");
	m_cameraRTTRight->setName("RightRTT");

	// Add RTT cameras as slaves, specifying offsets for the projection
	m_view->addSlave(m_cameraRTTLeft.get(),
					 m_device->projectionOffsetMatrixLeft(),
					 m_device->viewMatrixLeft(),
					 true);
	m_view->getSlave(0)._updateSlaveCallback = new OculusUpdateSlaveCallback(OculusUpdateSlaveCallback::LEFT_CAMERA, m_device.get(), swapCallback.get());

	m_view->addSlave(m_cameraRTTRight.get(),
					 m_device->projectionOffsetMatrixRight(),
					 m_device->viewMatrixRight(),
					 true);
	m_view->getSlave(1)._updateSlaveCallback = new OculusUpdateSlaveCallback(OculusUpdateSlaveCallback::RIGHT_CAMERA, m_device.get(), swapCallback.get());

	// Use sky light instead of headlight to avoid light changes when head movements
	m_view->setLightingMode(osg::View::SKY_LIGHT);

	// Disable rendering of main camera since its being overwritten by the swap texture anyway
	camera->setGraphicsContext(nullptr);

	m_configured = true;
}
