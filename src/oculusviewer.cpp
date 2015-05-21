/*
 * oculusviewer.cpp
 *
 *  Created on: Jun 30, 2013
 *      Author: Jan Ciger & Björn Blissing
 */

#include "oculusviewer.h"

#include <osgViewer/View>


/* Public functions */
void OculusViewer::traverse(osg::NodeVisitor& nv)
{
	// Must be realized before any traversal
	if (m_realizeOperation->realized()) {

		if (!m_configured) {
			configure();
		}

		if (m_configured) {
			m_device.get()->updatePose(m_swapCallback->frameIndex());
			osg::Vec3 position = m_device.get()->position();
			osg::Quat orientation = m_device.get()->orientation();
			osg::Matrix viewOffsetLeft = m_device.get()->viewMatrixLeft();
			osg::Matrix viewOffsetRight = m_device.get()->viewMatrixRight();
			viewOffsetLeft.preMultRotate(orientation);
			viewOffsetRight.preMultRotate(orientation);
			viewOffsetLeft.preMultTranslate(position);
			viewOffsetRight.preMultTranslate(position);
			// Nasty hack to update the view offset for each of the slave cameras
			// There doesn't seem to be an accessor for this, fortunately the offsets are public
			m_view->findSlaveForCamera(m_cameraRTTLeft.get())->_viewOffset = viewOffsetLeft;
			m_view->findSlaveForCamera(m_cameraRTTRight.get())->_viewOffset = viewOffsetRight;
		}
	}

	osg::Group::traverse(nv);
}


/* Protected functions */
void OculusViewer::configure()
{
	osg::ref_ptr<osg::GraphicsContext> gc =  m_view->getCamera()->getGraphicsContext();
	
	// Attach a callback to detect swap
	m_swapCallback = new OculusSwapCallback(m_device);
	gc->setSwapCallback(m_swapCallback);

	osg::ref_ptr<osg::Camera> camera = m_view->getCamera();
	camera->setName("Main");

	// Disable scene rendering for main camera
	//camera->setCullMask(~m_sceneNodeMask);

	// master projection matrix
	camera->setProjectionMatrix(m_device->projectionMatrixCenter());
	// Create RTT cameras and attach textures
	m_cameraRTTLeft = m_device->createRTTCamera(OculusDevice::LEFT, osg::Camera::RELATIVE_RF, gc);
	m_cameraRTTRight = m_device->createRTTCamera(OculusDevice::RIGHT, osg::Camera::RELATIVE_RF, gc);
	m_cameraRTTLeft->setName("LeftRTT");
	m_cameraRTTRight->setName("RightRTT");
	m_cameraRTTLeft->setCullMask(m_sceneNodeMask);
	m_cameraRTTRight->setCullMask(m_sceneNodeMask);

	// Add RTT cameras as slaves, specifying offsets for the projection
	m_view->addSlave(m_cameraRTTLeft.get(),
		m_device->projectionOffsetMatrixLeft(),
		m_device->viewMatrixLeft(),
		true);
	m_view->addSlave(m_cameraRTTRight.get(),
		m_device->projectionOffsetMatrixRight(),
		m_device->viewMatrixRight(),
		true);

	// Use sky light instead of headlight to avoid light changes when head movements
	m_view->setLightingMode(osg::View::SKY_LIGHT);

	m_configured = true;
}
