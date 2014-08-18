/*
 * oculusviewer.cpp
 *
 *  Created on: Jun 30, 2013
 *      Author: Jan Ciger & Björn Blissing
 */

#include "oculusviewer.h"

#include <osgViewer/View>

void OculusViewer::traverse(osg::NodeVisitor& nv)
{
	if (!m_configured) {
		configure();
	}

	if (m_configured) {
		m_device.get()->updatePose(m_swapCallback->frameIndex());
		osg::Vec3 position = m_device.get()->position();
		osg::Quat orientation = m_device.get()->orientation();
		osg::Matrix viewOffsetLeft = m_device.get()->viewMatrixLeft();
		osg::Matrix viewOffsetRight = m_device.get()->viewMatrixRight();
		viewOffsetLeft.preMultTranslate(position);
		viewOffsetRight.preMultTranslate(position);
		viewOffsetLeft.postMultRotate(orientation);
		viewOffsetRight.postMultRotate(orientation);
		// Nasty hack to update the view offset for each of the slave cameras
		// There doesn't seem to be an accessor for this, fortunately the offsets are public
		m_view->findSlaveForCamera(m_cameraRTTLeft.get())->_viewOffset = viewOffsetLeft;
		m_view->findSlaveForCamera(m_cameraRTTRight.get())->_viewOffset = viewOffsetRight;
	}
	osg::Group::traverse(nv);
}

void OculusViewer::configure()
{
	osg::ref_ptr<osg::GraphicsContext> gc =  m_view->getCamera()->getGraphicsContext();
	
	// Attach a callback to detect swap
	m_swapCallback = new OculusSwapCallback(m_device);
	gc->setSwapCallback(m_swapCallback);

	osg::ref_ptr<osg::Camera> camera = m_view->getCamera();
	camera->setName("Main");

	// Disable scene rendering for main camera
	camera->setCullMask(~m_sceneNodeMask);

	const int textureWidth = m_device->renderTargetWidth()/2;
	const int textureHeight = m_device->renderTargetHeight();
	// master projection matrix
	camera->setProjectionMatrix(m_device->projectionMatrixCenter());
	// Create textures for RTT cameras
	osg::ref_ptr<osg::Texture2D> textureLeft = new osg::Texture2D;
	textureLeft->setTextureSize(textureWidth, textureHeight);
	textureLeft->setInternalFormat(GL_RGBA);
	osg::ref_ptr<osg::Texture2D> textureRight = new osg::Texture2D;
	textureRight->setTextureSize(textureWidth, textureHeight);
	textureRight->setInternalFormat(GL_RGBA);
	// Create RTT cameras and attach textures
	m_cameraRTTLeft = m_device->createRTTCamera(textureLeft, OculusDevice::LEFT, osg::Camera::RELATIVE_RF, gc);
	m_cameraRTTRight = m_device->createRTTCamera(textureRight, OculusDevice::RIGHT, osg::Camera::RELATIVE_RF, gc);
	m_cameraRTTLeft->setName("LeftRTT");
	m_cameraRTTRight->setName("RightRTT");
	m_cameraRTTLeft->setCullMask(m_sceneNodeMask);
	m_cameraRTTRight->setCullMask(m_sceneNodeMask);

	// Create warp ortho camera
	osg::ref_ptr<osg::Camera> cameraWarp = m_device->createWarpOrthoCamera(0.0, 1.0, 0.0, 1.0, gc);
	cameraWarp->setName("WarpOrtho");
	cameraWarp->setViewport(new osg::Viewport(0, 0, m_device->screenResolutionWidth(), m_device->screenResolutionHeight()));

	// Create shader program
	osg::ref_ptr<osg::Program> program = m_device->createShaderProgram();

	// Create distortionMesh for each camera
	osg::ref_ptr<osg::Geode> leftDistortionMesh = m_device->distortionMesh(OculusDevice::LEFT, program, 0, 0, textureWidth, textureHeight);
	cameraWarp->addChild(leftDistortionMesh);

	osg::ref_ptr<osg::Geode> rightDistortionMesh = m_device->distortionMesh(OculusDevice::RIGHT, program, 0, 0, textureWidth, textureHeight);
	cameraWarp->addChild(rightDistortionMesh);

	// Add pre draw camera to handle time warp
	cameraWarp->setPreDrawCallback(new WarpCameraPreDrawCallback(m_device));

	// Attach shaders to each distortion mesh
	osg::StateSet* leftEyeStateSet = leftDistortionMesh->getOrCreateStateSet();
	osg::StateSet* rightEyeStateSet = rightDistortionMesh->getOrCreateStateSet();

	m_device->applyShaderParameters(leftEyeStateSet, program.get(), textureLeft.get(), OculusDevice::LEFT);
	m_device->applyShaderParameters(rightEyeStateSet, program.get(), textureRight.get(), OculusDevice::RIGHT);

	// Add RTT cameras as slaves, specifying offsets for the projection
	m_view->addSlave(m_cameraRTTLeft.get(),
		m_device->projectionOffsetMatrixLeft(),
		m_device->viewMatrixLeft(),
		true);
	m_view->addSlave(m_cameraRTTRight.get(),
		m_device->projectionOffsetMatrixRight(),
		m_device->viewMatrixRight(),
		true);

	// Add warp camera as slave
	m_view->addSlave(cameraWarp, false);
	m_view->setName("Oculus");

	m_configured = true;
}
