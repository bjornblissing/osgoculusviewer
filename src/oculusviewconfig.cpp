/*
 * oculusviewconfig.cpp
 *
 *  Created on: Sept 26, 2013
 *      Author: Bjorn Blissing & Jan Ciger
 */
#include <osg/io_utils>
#include <osg/Texture2D>
#include <osg/PolygonMode>
#include <osg/Program>
#include <osg/Shader>

#include <osgDB/ReadFile>
#include <osgDB/FileUtils>

#include <osgViewer/View>

#include "oculusviewconfig.h"
#include "oculusdevice.h"

osg::Camera* OculusViewConfig::createRTTCamera(osg::Texture* texture, osg::GraphicsContext* gc) const
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

osg::Camera* OculusViewConfig::createHUDCamera(double left, double right, double bottom, double top, osg::GraphicsContext* gc) const
{
	osg::ref_ptr<osg::Camera> camera = new osg::Camera;
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

osg::Geode* OculusViewConfig::createHUDQuad( float width, float height, float scale ) const
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

void OculusViewConfig::applyShaderParameters(osg::StateSet* stateSet, osg::Program* program, 
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

void OculusViewConfig::configure(osgViewer::View& view) const
{
	m_device->setNearClip(m_nearClip);
	m_device->setFarClip(m_farClip);
	m_device->setSensorPredictionEnabled(m_useSensorPrediction);
	m_device->setSensorPredictionDelta(m_predictionDelta);

	if (m_useCustomScaleFactor) {
		m_device->setCustomScaleFactor(m_customScaleFactor);
	}

	// Create screen with match the Oculus Rift resolution
	osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();

	if (!wsi) {
		osg::notify(osg::NOTICE)<<"Error, no WindowSystemInterface available, cannot create windows."<<std::endl;
		return;
	}

	// Get the screen identifiers set in environment variable DISPLAY
	osg::GraphicsContext::ScreenIdentifier si;
	si.readDISPLAY();
	
	// If displayNum has not been set, reset it to 0.
	if (si.displayNum < 0) si.displayNum = 0;

	// If screenNum has not been set, reset it to 0.
	if (si.screenNum < 0) si.screenNum = 0;

	unsigned int width, height;
	wsi->getScreenResolution(si, width, height);

	osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
	traits->hostName = si.hostName;
	traits->screenNum = si.screenNum;
	traits->displayNum = si.displayNum;
	traits->windowDecoration = false;
	traits->x = 0;
	traits->y = 0;
	traits->width = m_device->hScreenResolution();
	traits->height = m_device->vScreenResolution();
	traits->doubleBuffer = true;
	traits->sharedContext = 0;
	traits->vsync = true; // VSync should always be enabled for Oculus Rift applications

	// Create a graphic context based on our desired traits
	osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits);
	if (!gc) {
		osg::notify(osg::NOTICE) << "Error, GraphicsWindow has not been created successfully" << std::endl;
		return;
	}

	osg::ref_ptr<osg::Camera> camera = view.getCamera();
	camera->setName("Main");
	// Disable scene rendering for main camera
	camera->setCullMask(~m_sceneNodeMask);
	camera->setGraphicsContext(gc);
	// Use full view port
	camera->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));
	// Disable automatic computation of near and far plane on main camera, will propagate to slave cameras
	camera->setComputeNearFarMode( osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR );
	const int textureWidth  = m_device->scaleFactor() * m_device->hScreenResolution()/2;
	const int textureHeight = m_device->scaleFactor() * m_device->vScreenResolution();
	// master projection matrix
	camera->setProjectionMatrix(m_device->projectionCenterMatrix());
	// Create textures for RTT cameras
	osg::ref_ptr<osg::Texture2D> textureLeft = new osg::Texture2D;
	textureLeft->setTextureSize( textureWidth, textureHeight );
	textureLeft->setInternalFormat( GL_RGBA );
	osg::ref_ptr<osg::Texture2D> textureRight = new osg::Texture2D;
	textureRight->setTextureSize( textureWidth, textureHeight );
	textureRight->setInternalFormat( GL_RGBA );
	// Create RTT cameras and attach textures
	osg::ref_ptr<osg::Camera> cameraRTTLeft = createRTTCamera(textureLeft, gc);
	osg::ref_ptr<osg::Camera> cameraRTTRight = createRTTCamera(textureRight, gc);
	cameraRTTLeft->setName("LeftRTT");
	cameraRTTRight->setName("RightRTT");
	cameraRTTLeft->setCullMask(m_sceneNodeMask);
	cameraRTTRight->setCullMask(m_sceneNodeMask);
	// Create HUD cameras for left eye
	osg::ref_ptr<osg::Camera> cameraHUDLeft = createHUDCamera(0.0, 1.0, 0.0, 1.0, gc);
	cameraHUDLeft->setName("LeftHUD");
	cameraHUDLeft->setViewport(new osg::Viewport(0, 0, 
		m_device->hScreenResolution() / 2.0f, m_device->vScreenResolution()));
	// Create HUD cameras for right eye
	osg::ref_ptr<osg::Camera> cameraHUDRight = createHUDCamera(0.0, 1.0, 0.0, 1.0, gc);
	cameraHUDRight->setName("RightHUD");
	cameraHUDRight->setViewport(new osg::Viewport(m_device->hScreenResolution() / 2.0f, 0,
										 m_device->hScreenResolution() / 2.0f, m_device->vScreenResolution()));
	// Create quads for each camera
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
	
	// Add RTT cameras as slaves, specifying offsets for the projection
	view.addSlave(cameraRTTLeft, 
		m_device->projectionOffsetMatrix(OculusDevice::LEFT_EYE), 
		m_device->viewMatrix(OculusDevice::LEFT_EYE), 
		true);
	view.addSlave(cameraRTTRight, 
		m_device->projectionOffsetMatrix(OculusDevice::RIGHT_EYE), 
		m_device->viewMatrix(OculusDevice::RIGHT_EYE), 
		true);

	// Add HUD cameras as slaves
	view.addSlave(cameraHUDLeft, false);
	view.addSlave(cameraHUDRight, false);

	view.setName("Oculus");
	// Connect main camera to node callback that get HMD orientation
	if (m_useOrientations) {
		camera->setDataVariance(osg::Object::DYNAMIC);
		camera->setUpdateCallback(new OculusViewConfigOrientationCallback(cameraRTTLeft, cameraRTTRight, m_device));
	}
}

void OculusViewConfigOrientationCallback::operator() (osg::Node* node, osg::NodeVisitor* nv)
{
	osg::Camera* mainCamera = static_cast<osg::Camera*>(node);
	osg::View* view = mainCamera->getView();

	if (view) {
		osg::Quat orient = m_device.get()->getOrientation();
		// Nasty hack to update the view offset for each of the slave cameras
		// There doesn't seem to be an accessor for this, fortunately the offsets are public
		view->findSlaveForCamera(m_cameraRTTLeft.get())->_viewOffset.setRotate(orient);
		view->findSlaveForCamera(m_cameraRTTRight.get())->_viewOffset.setRotate(orient);
	}

	traverse(node, nv);
}
