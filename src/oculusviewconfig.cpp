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

osg::Camera* OculusViewConfig::createRTTCamera(osg::Camera::BufferComponent buffer, osg::Texture* tex, osg::GraphicsContext* gc) const
{
	osg::ref_ptr<osg::Camera> camera = new osg::Camera;
	camera->setClearColor(osg::Vec4(0.2f, 0.2f, 0.4f, 1.0f));
	camera->setClearMask( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );
	camera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT );
	camera->setRenderOrder( osg::Camera::PRE_RENDER );
	camera->setGraphicsContext(gc);
	camera->setReferenceFrame(osg::Camera::RELATIVE_RF);

	if ( tex ) {
		tex->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR );
		tex->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );
		camera->setViewport( 0, 0, tex->getTextureWidth(), tex->getTextureHeight() );
		camera->attach( buffer, tex, 0, 0, false, 4, 4);
	}

	return camera.release();
}

osg::Camera* OculusViewConfig::createHUDCamera(double left, double right, double bottom, double top, osg::GraphicsContext* gc) const
{
	osg::ref_ptr<osg::Camera> camera = new osg::Camera;
	camera->setGraphicsContext(gc);
	camera->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
	camera->setClearColor(osg::Vec4(0.2f, 0.2f, 0.4f, 1.0f));
	camera->setClearMask( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	camera->setRenderOrder( osg::Camera::POST_RENDER );
	camera->setAllowEventFocus( false );
	camera->setProjectionMatrix( osg::Matrix::ortho2D(left, right, bottom, top) );
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

void OculusViewConfig::configure(osgViewer::View& view) const
{
	m_dev->setNearClip(m_nearClip);
	m_dev->setFarClip(m_farClip);
	m_dev->setSensorPredictionEnabled(m_useSensorPrediction);
	m_dev->setSensorPredictionDelta(m_predictionDelta);

	if (m_useCustomScaleFactor) {
		m_dev->setCustomScaleFactor(m_customScaleFactor);
	}

	// Create screen with match the Oculus Rift resolution
	osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();

	if (!wsi) {
		osg::notify(osg::NOTICE)<<"Error, no WindowSystemInterface available, cannot create windows."<<std::endl;
		return;
	}

	unsigned int width, height;
	wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(0), width, height);
	osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
	traits->windowDecoration = false;
	traits->x = 0;
	traits->y = 0;
	traits->width = m_dev->hScreenResolution();
	traits->height = m_dev->vScreenResolution();
	traits->doubleBuffer = true;
	traits->sharedContext = 0;
	traits->sampleBuffers = true;
	traits->samples = 4;
	traits->vsync = true;
	osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits);
	osg::ref_ptr<osg::Camera> camera = view.getCamera();
	camera->setGraphicsContext(gc);
	// Use full viewport
	camera->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));
	// Disable automatic computation of near and far plane on main camera, will propagate to slave cameras
	camera->setComputeNearFarMode( osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR );
	const int textureWidth  = m_dev->scaleFactor() * m_dev->hScreenResolution()/2;
	const int textureHeight = m_dev->scaleFactor() * m_dev->vScreenResolution();
	// master projection matrix
	camera->setProjectionMatrix(m_dev->projectionCenterMatrix());
	osg::ref_ptr<osg::Texture2D> l_tex = new osg::Texture2D;
	l_tex->setTextureSize( textureWidth, textureHeight );
	l_tex->setInternalFormat( GL_RGBA );
	osg::ref_ptr<osg::Texture2D> r_tex = new osg::Texture2D;
	r_tex->setTextureSize( textureWidth, textureHeight );
	r_tex->setInternalFormat( GL_RGBA );
	osg::ref_ptr<osg::Camera> l_rtt = createRTTCamera(osg::Camera::COLOR_BUFFER, l_tex, gc);
	osg::ref_ptr<osg::Camera> r_rtt = createRTTCamera(osg::Camera::COLOR_BUFFER, r_tex, gc);
	// Create HUD cameras for each eye
	osg::ref_ptr<osg::Camera> l_hud = createHUDCamera(0.0, 1.0, 0.0, 1.0, gc);
	l_hud->setViewport(new osg::Viewport(0, 0, m_dev->hScreenResolution() / 2.0f, m_dev->vScreenResolution()));
	osg::ref_ptr<osg::Camera> r_hud = createHUDCamera(0.0, 1.0, 0.0, 1.0, gc);
	r_hud->setViewport(new osg::Viewport(m_dev->hScreenResolution() / 2.0f, 0,
										 m_dev->hScreenResolution() / 2.0f, m_dev->vScreenResolution()));
	// Create quads on each camera
	osg::ref_ptr<osg::Geode> leftQuad = createHUDQuad(1.0f, 1.0f);
	l_hud->addChild(leftQuad);
	osg::ref_ptr<osg::Geode> rightQuad = createHUDQuad(1.0f, 1.0f);
	r_hud->addChild(rightQuad);
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
	// Configure state sets for both eyes
	osg::StateSet* leftEyeStateSet = leftQuad->getOrCreateStateSet();
	leftEyeStateSet->setTextureAttributeAndModes(0, l_tex, osg::StateAttribute::ON);
	leftEyeStateSet->setAttributeAndModes( program, osg::StateAttribute::ON );
	leftEyeStateSet->addUniform( new osg::Uniform("WarpTexture", 0) );
	leftEyeStateSet->addUniform( new osg::Uniform("LensCenter", m_dev->lensCenter(OculusDevice::LEFT_EYE)));
	leftEyeStateSet->addUniform( new osg::Uniform("ScreenCenter", m_dev->screenCenter()));
	leftEyeStateSet->addUniform( new osg::Uniform("Scale", m_dev->scale()));
	leftEyeStateSet->addUniform( new osg::Uniform("ScaleIn", m_dev->scaleIn()));
	leftEyeStateSet->addUniform( new osg::Uniform("HmdWarpParam", m_dev->warpParameters()));
	leftEyeStateSet->addUniform( new osg::Uniform("ChromAbParam", m_dev->chromAbParameters()));
	osg::StateSet* rightEyeStateSet = rightQuad->getOrCreateStateSet();
	rightEyeStateSet->setTextureAttributeAndModes(0, r_tex, osg::StateAttribute::ON);
	rightEyeStateSet->setAttributeAndModes( program, osg::StateAttribute::ON );
	rightEyeStateSet->addUniform( new osg::Uniform("WarpTexture", 0) );
	rightEyeStateSet->addUniform( new osg::Uniform("LensCenter", m_dev->lensCenter(OculusDevice::RIGHT_EYE)));
	rightEyeStateSet->addUniform( new osg::Uniform("ScreenCenter", m_dev->screenCenter()));
	rightEyeStateSet->addUniform( new osg::Uniform("Scale", m_dev->scale()));
	rightEyeStateSet->addUniform( new osg::Uniform("ScaleIn", m_dev->scaleIn()));
	rightEyeStateSet->addUniform( new osg::Uniform("HmdWarpParam", m_dev->warpParameters()));
	rightEyeStateSet->addUniform( new osg::Uniform("ChromAbParam", m_dev->chromAbParameters()));
	// Add cameras as slaves, specifying offsets for the projection
	view.addSlave(l_rtt, m_dev->projectionOffsetMatrix(OculusDevice::LEFT_EYE), m_dev->viewMatrix(OculusDevice::LEFT_EYE), true);
	view.addSlave(r_rtt, m_dev->projectionOffsetMatrix(OculusDevice::RIGHT_EYE), m_dev->viewMatrix(OculusDevice::RIGHT_EYE), true);
	view.addSlave(l_hud, false);
	view.addSlave(r_hud, false);

	// Connect main camera to node callback that get HMD orientation
	if (m_useOrientations) {
		camera->setDataVariance(osg::Object::DYNAMIC);
		camera->setUpdateCallback(new OculusViewConfigOrientationCallback(l_rtt, r_rtt, m_dev));
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
		view->findSlaveForCamera(m_l_rtt.get())->_viewOffset.setRotate(orient);
		view->findSlaveForCamera(m_r_rtt.get())->_viewOffset.setRotate(orient);
	}

	traverse(node, nv);
}
