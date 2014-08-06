/*
 * oculusviewconfig.cpp
 *
 *  Created on: Sept 26, 2013
 *      Author: Bjorn Blissing & Jan Ciger
 */
#include <osg/io_utils>
#include <osg/Texture2D>
#include <osg/PolygonMode>
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

osg::Camera* OculusViewConfig::createWarpOrthoCamera(double left, double right, double bottom, double top, osg::GraphicsContext* gc) const
{
	osg::ref_ptr<osg::Camera> camera = new osg::Camera;
	camera->setClearColor(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
	camera->setClearMask( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	camera->setRenderOrder(osg::Camera::POST_RENDER);
	camera->setAllowEventFocus(false);
	camera->setGraphicsContext(gc);
	camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	camera->setProjectionMatrix(osg::Matrix::ortho2D(left, right, bottom, top));
	camera->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
	return camera.release();
}

void OculusViewConfig::configure(osgViewer::View& view) const
{
	m_device->setNearClip(m_nearClip);
	m_device->setFarClip(m_farClip);

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
	traits->x = m_device->windowPos().x();
	traits->y = m_device->windowPos().y();
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
	
	// Attach a callback to detect swap
	osg::ref_ptr<OculusSwapCallback> swapCallback = new OculusSwapCallback(m_device);
	gc->setSwapCallback(swapCallback);

	osg::ref_ptr<osg::Camera> camera = view.getCamera();
	camera->setName("Main");
	// Disable scene rendering for main camera
	camera->setCullMask(~m_sceneNodeMask);
	camera->setGraphicsContext(gc);
	// Use full view port
	camera->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));
	// Disable automatic computation of near and far plane on main camera, will propagate to slave cameras
	camera->setComputeNearFarMode( osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR );
	const int textureWidth  = m_device->hRenderTargetSize()/2;
	const int textureHeight = m_device->vRenderTargetSize();
	// master projection matrix
	camera->setProjectionMatrix(m_device->projectionMatrixCenter());
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
	
	// Create warp ortho camera
	osg::ref_ptr<osg::Camera> cameraWarp = createWarpOrthoCamera(0.0, 1.0, 0.0, 1.0, gc);
	cameraWarp->setName("WarpOrtho");
	cameraWarp->setViewport(new osg::Viewport(0, 0, m_device->hScreenResolution(), m_device->vScreenResolution()));

	// Set up shaders from the Oculus SDK documentation
	osg::ref_ptr<osg::Program> program = new osg::Program;
	osg::ref_ptr<osg::Shader> vertexShader = new osg::Shader(osg::Shader::VERTEX);
	vertexShader->loadShaderSourceFromFile(osgDB::findDataFile("warp_mesh.vert"));
	osg::ref_ptr<osg::Shader> fragmentShader = new osg::Shader(osg::Shader::FRAGMENT);
	fragmentShader->loadShaderSourceFromFile(osgDB::findDataFile("warp_mesh.frag"));
	program->addShader(vertexShader);
	program->addShader(fragmentShader);

	// Create distortionMesh for each camera
	osg::ref_ptr<osg::Geode> leftDistortionMesh = m_device->distortionMesh(0, program, 0, 0, textureWidth, textureHeight);
	cameraWarp->addChild(leftDistortionMesh);

	osg::ref_ptr<osg::Geode> rightDistortionMesh = m_device->distortionMesh(1, program, textureWidth, 0, textureWidth, textureHeight);
	cameraWarp->addChild(rightDistortionMesh);

	// Add pre draw camera to handle time warp
	cameraWarp->setPreDrawCallback(new WarpCameraPreDrawCallback(m_device));

	// Attach shaders to each distortion mesh
	osg::StateSet* leftEyeStateSet = leftDistortionMesh->getOrCreateStateSet();
	leftEyeStateSet->setTextureAttributeAndModes(0, textureLeft, osg::StateAttribute::ON);
	leftEyeStateSet->setAttributeAndModes(program, osg::StateAttribute::ON);
	leftEyeStateSet->addUniform(new osg::Uniform("Texture", 0));
	leftEyeStateSet->addUniform(new osg::Uniform("EyeToSourceUVScale", m_device->eyeToSourceUVScale(0)));
	leftEyeStateSet->addUniform(new osg::Uniform("EyeToSourceUVOffset", m_device->eyeToSourceUVOffset(0)));

	osg::StateSet* rightEyeStateSet = rightDistortionMesh->getOrCreateStateSet();
	rightEyeStateSet->setTextureAttributeAndModes(0, textureRight, osg::StateAttribute::ON);
	rightEyeStateSet->setAttributeAndModes(program, osg::StateAttribute::ON);
	rightEyeStateSet->addUniform(new osg::Uniform("Texture", 0));
	rightEyeStateSet->addUniform(new osg::Uniform("EyeToSourceUVScale", m_device->eyeToSourceUVScale(1)));
	rightEyeStateSet->addUniform(new osg::Uniform("EyeToSourceUVOffset", m_device->eyeToSourceUVOffset(1)));

	// Add RTT cameras as slaves, specifying offsets for the projection
	view.addSlave(cameraRTTLeft, 
		m_device->projectionOffsetMatrixLeft(),
		m_device->viewMatrixLeft(), 
		true);
	view.addSlave(cameraRTTRight, 
		m_device->projectionOffsetMatrixRight(),
		m_device->viewMatrixRight(),
		true);

	// Add warp camera as slave
	view.addSlave(cameraWarp, false);
	view.setName("Oculus");

	// Connect main camera to node callback that get HMD orientation
	if (m_useOrientations) {
		camera->setDataVariance(osg::Object::DYNAMIC);
		camera->setUpdateCallback(new OculusViewConfigOrientationCallback(cameraRTTLeft, cameraRTTRight, m_device, swapCallback));
	}
}

void WarpCameraPreDrawCallback::operator()(osg::RenderInfo&) const
{
	// Wait till time - warp point to reduce latency.
	m_device->waitTillTime();
}

void  OculusSwapCallback::swapBuffersImplementation(osg::GraphicsContext *gc) {
	// Run the default system swapBufferImplementation
	gc->swapBuffersImplementation();
	// End frame timing when swap buffer is done
	m_device->endFrameTiming();
	// Start a new frame with incremented frame index
	m_device->beginFrameTiming(++m_frameIndex);
}

void OculusViewConfigOrientationCallback::operator() (osg::Node* node, osg::NodeVisitor* nv)
{
	osg::Camera* mainCamera = static_cast<osg::Camera*>(node);
	osg::View* view = mainCamera->getView();

	if (view) {
		osg::Quat orient = m_device.get()->getOrientation(m_swapCallback->frameIndex());
		// Nasty hack to update the view offset for each of the slave cameras
		// There doesn't seem to be an accessor for this, fortunately the offsets are public
		view->findSlaveForCamera(m_cameraRTTLeft.get())->_viewOffset.setRotate(orient);
		view->findSlaveForCamera(m_cameraRTTRight.get())->_viewOffset.setRotate(orient);
	}

	traverse(node, nv);
}
