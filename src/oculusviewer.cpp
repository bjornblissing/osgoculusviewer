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

	// Create RTT cameras and attach textures
	osg::Camera* cameraRTTLeft = m_device->createRTTCamera(OculusDevice::Eye::LEFT, osg::Camera::ABSOLUTE_RF, clearColor, gc.get());
	osg::Camera* cameraRTTRight = m_device->createRTTCamera(OculusDevice::Eye::RIGHT, osg::Camera::ABSOLUTE_RF, clearColor, gc.get()); 
	cameraRTTLeft->setName("LeftRTT");
	cameraRTTRight->setName("RightRTT");

	// Add RTT cameras as slaves, specifying offsets for the projection
	m_view->addSlave(cameraRTTLeft,
					 m_device->projectionMatrix(OculusDevice::Eye::LEFT),
					 m_device->viewMatrix(OculusDevice::Eye::LEFT),
					 true);
	m_view->getSlave(0)._updateSlaveCallback = new OculusUpdateSlaveCallback(OculusUpdateSlaveCallback::LEFT_CAMERA, m_device.get(), swapCallback.get());

	m_view->addSlave(cameraRTTRight,
					m_device->projectionMatrix(OculusDevice::Eye::RIGHT),
					m_device->viewMatrix(OculusDevice::Eye::RIGHT),
					 true);
	m_view->getSlave(1)._updateSlaveCallback = new OculusUpdateSlaveCallback(OculusUpdateSlaveCallback::RIGHT_CAMERA, m_device.get(), swapCallback.get());

	// Use sky light instead of headlight to avoid light changes when head movements
	m_view->setLightingMode(osg::View::SKY_LIGHT);

	// Disable rendering of main camera since its being overwritten by the swap texture anyway
	camera->setGraphicsContext(nullptr);

	m_configured = true;
}
