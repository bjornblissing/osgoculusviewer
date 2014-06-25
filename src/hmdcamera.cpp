/*
 * HMDCamera.cpp
 *
 *  Created on: Jun 30, 2013
 *      Author: Jan Ciger
 */

#include <osg/io_utils>
#include <osg/Texture2D>
#include <osg/PolygonMode>
#include <osg/Program>
#include <osg/Shader>

#include <osgDB/ReadFile>
#include <osgDB/FileUtils>

#include <osgViewer/View>

#include "hmdcamera.h"
#include "oculusdevice.h"


HMDCamera::HMDCamera(osgViewer::View* view, osg::ref_ptr<OculusDevice> dev) : osg::Group(),
	m_configured(false),
	m_useChromaticAberrationCorrection(false),
	m_view(view),
	m_device(dev)
{
}


HMDCamera::~HMDCamera()
{
}

void HMDCamera::traverse(osg::NodeVisitor& nv)
{
	if (!m_configured) {
		configure();
	}

	// Get orientation from oculus sensor
	osg::Quat orient = m_device->getOrientation();
	// Nasty hack to update the view offset for each of the slave cameras
	// There doesn't seem to be an accessor for this, fortunately the offsets are public
	m_view->findSlaveForCamera(m_cameraRTTLeft.get())->_viewOffset.setRotate(orient);
	m_view->findSlaveForCamera(m_cameraRTTRight.get())->_viewOffset.setRotate(orient);
	osg::Group::traverse(nv);
}

osg::Camera* HMDCamera::createRTTCamera(osg::Texture* texture, osg::GraphicsContext* gc) const
{
	osg::ref_ptr<osg::Camera> camera = new osg::Camera;
	camera->setClearColor(osg::Vec4(0.2f, 0.2f, 0.4f, 1.0f));
	camera->setClearMask( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );
	camera->setDrawBuffer(GL_FRONT);
	camera->setReadBuffer(GL_FRONT);
	camera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT );
	camera->setRenderOrder(osg::Camera::PRE_RENDER);
	camera->setAllowEventFocus(false);
	camera->setGraphicsContext(gc);
	camera->setReferenceFrame(osg::Camera::RELATIVE_RF);

	if ( texture ) {
		texture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
		texture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
		camera->setViewport(0, 0, texture->getTextureWidth(), texture->getTextureHeight());
		camera->attach(osg::Camera::COLOR_BUFFER, texture, 0, 0, false, 4, 4);
	}

	return camera.release();
}

osg::Camera* HMDCamera::createHUDCamera(double left, double right, double bottom, double top, osg::GraphicsContext* gc) const
{
	osg::ref_ptr<osg::Camera> camera = new osg::Camera;
	camera->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
	camera->setClearColor(osg::Vec4(0.2f, 0.2f, 0.4f, 1.0f));
	camera->setClearMask( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	camera->setRenderOrder(osg::Camera::POST_RENDER);
	camera->setAllowEventFocus(false);
	camera->setGraphicsContext(gc);
	camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	camera->setProjectionMatrix(osg::Matrix::ortho2D(left, right, bottom, top));
	camera->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
	return camera.release();
}

osg::Geode* HMDCamera::createHUDQuad( float width, float height, float scale ) const
{
	osg::Geometry* geom = osg::createTexturedQuadGeometry(osg::Vec3(),
		osg::Vec3(width, 0.0f, 0.0f),
		osg::Vec3(0.0f,  height, 0.0f),
		0.0f, 0.0f, width*scale, height*scale );
	osg::ref_ptr<osg::Geode> quad = new osg::Geode;
	quad->addDrawable( geom );
	int values = osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED;
	quad->getOrCreateStateSet()->setAttribute(new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL), values );
	quad->getOrCreateStateSet()->setMode( GL_LIGHTING, values );
	return quad.release();
}

void HMDCamera::applyShaderParameters(osg::StateSet* stateSet, osg::Program* program, 
	osg::Texture2D* texture, OculusDevice::EyeSide eye) const {
		stateSet->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);
		stateSet->setAttributeAndModes( program, osg::StateAttribute::ON );
		stateSet->addUniform( new osg::Uniform("WarpTexture", 0) );
		stateSet->addUniform( new osg::Uniform("LensCenter", m_device->lensCenter(eye)));
		stateSet->addUniform( new osg::Uniform("ScreenCenter", m_device->screenCenter()));
		stateSet->addUniform( new osg::Uniform("Scale", m_device->scale()));
		stateSet->addUniform( new osg::Uniform("ScaleIn", m_device->scaleIn()));
		stateSet->addUniform( new osg::Uniform("HmdWarpParam", m_device->warpParameters()));
		stateSet->addUniform( new osg::Uniform("ChromAbParam", m_device->chromAbParameters()));
}

void HMDCamera::configure()
{
	const int textureWidth  = m_device->scaleFactor() * m_device->hScreenResolution()/2;
	const int textureHeight = m_device->scaleFactor() * m_device->vScreenResolution();
	
	// master projection matrix
	m_view->getCamera()->setProjectionMatrix(m_device->projectionCenterMatrix());
	m_view->setName("Oculus");
	osg::ref_ptr<osg::Camera> mainCamera = m_view->getCamera();
	mainCamera->setName("Main");
	// Disable scene rendering for main camera
	mainCamera->setCullMask(~m_sceneNodeMask);

	osg::ref_ptr<osg::Texture2D> textureLeft = new osg::Texture2D;
	textureLeft->setTextureSize( textureWidth, textureHeight );
	textureLeft->setInternalFormat( GL_RGBA );
	osg::ref_ptr<osg::Texture2D> textureRight = new osg::Texture2D;
	textureRight->setTextureSize( textureWidth, textureHeight );
	textureRight->setInternalFormat( GL_RGBA );
	
	osg::ref_ptr<osg::GraphicsContext> gc = mainCamera->getGraphicsContext();
	// Create render to texture cameras
	m_cameraRTTLeft = createRTTCamera(textureLeft, gc.get());
	m_cameraRTTRight = createRTTCamera(textureRight, gc.get());
	m_cameraRTTLeft->setName("LeftRTT");
	m_cameraRTTRight->setName("RightRTT");
	m_cameraRTTLeft->setCullMask(m_sceneNodeMask);
	m_cameraRTTRight->setCullMask(m_sceneNodeMask);

	
	// Create HUD cameras for each eye
	osg::ref_ptr<osg::Camera> cameraHUDLeft = createHUDCamera(0.0, 1.0, 0.0, 1.0, gc.get());
	cameraHUDLeft->setName("LeftHUD");
	cameraHUDLeft->setViewport(new osg::Viewport(0, 0, m_device->hScreenResolution() / 2.0f, m_device->vScreenResolution()));
	osg::ref_ptr<osg::Camera> cameraHUDRight = createHUDCamera(0.0, 1.0, 0.0, 1.0, gc.get());
	cameraHUDRight->setName("RightHUD");
	cameraHUDRight->setViewport(new osg::Viewport(m_device->hScreenResolution() / 2.0f, 0,
										 m_device->hScreenResolution() / 2.0f, m_device->vScreenResolution()));
	// Create quads on each camera
	osg::ref_ptr<osg::Geode> leftQuad = createHUDQuad(1.0f, 1.0f);
	cameraHUDLeft->addChild(leftQuad);
	osg::ref_ptr<osg::Geode> rightQuad = createHUDQuad(1.0f, 1.0f);
	cameraHUDRight->addChild(rightQuad);
	
	// Set up shaders from the Oculus SDK documentation
	osg::ref_ptr<osg::Program> program = new osg::Program;
	osg::ref_ptr<osg::Shader> vertexShader = new osg::Shader(osg::Shader::VERTEX);
	vertexShader->loadShaderSourceFromFile(osgDB::findDataFile("warp.vert"));
	osg::ref_ptr<osg::Shader> fragmentShader = new osg::Shader(osg::Shader::FRAGMENT);

	// Fragment shader with or without correction for chromatic aberration
	if (m_useChromaticAberrationCorrection) {
		fragmentShader->loadShaderSourceFromFile(osgDB::findDataFile("warpWithChromeAb.frag"));
	} else {
		fragmentShader->loadShaderSourceFromFile(osgDB::findDataFile("warpWithoutChromeAb.frag"));
	}

	program->addShader(vertexShader);
	program->addShader(fragmentShader);

	// Attach shaders to each HUD
	osg::StateSet* leftEyeStateSet = leftQuad->getOrCreateStateSet();
	osg::StateSet* rightEyeStateSet = rightQuad->getOrCreateStateSet();
	applyShaderParameters(leftEyeStateSet, program.get(), textureLeft.get(), OculusDevice::LEFT_EYE);
	applyShaderParameters(rightEyeStateSet, program.get(), textureRight.get(), OculusDevice::RIGHT_EYE);

	// Add cameras as slaves, specifying offsets for the projection
	m_view->addSlave(m_cameraRTTLeft.get(), m_device->projectionOffsetMatrix(OculusDevice::LEFT_EYE), 
		m_device->viewMatrix(OculusDevice::LEFT_EYE), 
		true);
	m_view->addSlave(m_cameraRTTRight.get(), m_device->projectionOffsetMatrix(OculusDevice::RIGHT_EYE), 
		m_device->viewMatrix(OculusDevice::RIGHT_EYE), 
		true);
	m_view->addSlave(cameraHUDLeft, false);
	m_view->addSlave(cameraHUDRight, false);
	m_configured = true;
}
