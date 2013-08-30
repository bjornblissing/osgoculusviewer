/*
* main.cpp
*
* Created on: Jul 03, 2013
* Author: Bjorn Blissing
*/

#include "oculusdevice.h"

#include <osg/Texture2D>
#include <osg/Program>
#include <osg/StateSet>
#include <osg/Camera>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/PolygonMode>

#include <osg/MatrixTransform>

#include <osgViewer/CompositeViewer>
#include <osgViewer/Renderer>
#include <osgViewer/ViewerEventHandlers>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgGA/TrackballManipulator>

osg::Geode* createScreenQuad( float width, float height, float scale=1.0f )
{
	osg::Geometry* geom = osg::createTexturedQuadGeometry(osg::Vec3(),
						  osg::Vec3(width,0.0f,0.0f),
						  osg::Vec3(0.0f,height,0.0f),
						  0.0f,
						  0.0f,
						  width*scale,
						  height*scale);
	osg::ref_ptr<osg::Geode> quad = new osg::Geode;
	quad->addDrawable( geom );
	int values = osg::StateAttribute::OFF|osg::StateAttribute::PROTECTED;
	quad->getOrCreateStateSet()->setAttribute(new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL), values );
	quad->getOrCreateStateSet()->setMode( GL_LIGHTING, values );
	return quad.release();
}

osg::Camera* createRTTCamera( osg::Camera::BufferComponent buffer, osg::Texture* tex )
{
	osg::ref_ptr<osg::Camera> camera = new osg::Camera;
	camera->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
	camera->setClearColor( osg::Vec4(0.2f, 0.2f, 0.4f, 1.0f) );
	camera->setClearMask( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );
	camera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT );
	camera->setRenderOrder( osg::Camera::PRE_RENDER);

	if ( tex ) {
		tex->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR );
		tex->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );
		camera->setViewport( 0, 0, tex->getTextureWidth(), tex->getTextureHeight() );
		camera->attach( buffer, tex, 0, 0, false, 4, 4);
	}

	return camera.release();
}

osg::Camera* createHUDCamera( double left, double right, double bottom, double top )
{
	osg::ref_ptr<osg::Camera> camera = new osg::Camera;
	camera->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
	camera->setClearMask( GL_DEPTH_BUFFER_BIT );
	camera->setRenderOrder( osg::Camera::POST_RENDER);
	camera->setAllowEventFocus( false );
	camera->setProjectionMatrix( osg::Matrix::ortho2D(left, right, bottom, top) );
	camera->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
	camera->setViewMatrix(osg::Matrix::identity());
	return camera.release();
}

int main( int argc, char** argv )
{
	OculusDevice* oculusDevice = new OculusDevice;
	// use an ArgumentParser object to manage the program arguments.
	osg::ArgumentParser arguments(&argc,argv);
	// read the scene from the list of file specified command line arguments.
	osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);

	// if not loaded assume no arguments passed in, try use default cow model instead.
	if (!loadedModel) loadedModel = osgDB::readNodeFile("cow.osgt");

	// Still no loaded model, then exit
	if (!loadedModel) return 0;

	// Set the scaleing of the texture, using the recommend 25% from the SDK
	oculusDevice->setCustomScaleFactor(1.25f);
	// Calculate the texture size
	const int textureWidth = oculusDevice->scaleFactor() * oculusDevice->hScreenResolution()/2;
	const int textureHeight = oculusDevice->scaleFactor() * oculusDevice->vScreenResolution();
	// Setup textures for the RTT cameras
	osg::ref_ptr<osg::Texture2D> leftEyeTex2D = new osg::Texture2D;
	leftEyeTex2D->setTextureSize( textureWidth, textureHeight );
	leftEyeTex2D->setInternalFormat( GL_RGBA );
	osg::ref_ptr<osg::Texture2D> rightEyeTex2D = new osg::Texture2D;
	rightEyeTex2D->setTextureSize( textureWidth, textureHeight );
	rightEyeTex2D->setInternalFormat( GL_RGBA );
	// Initialize RTT cameras for each eye
	osg::ref_ptr<osg::Camera> leftEyeRTTCamera = createRTTCamera(osg::Camera::COLOR_BUFFER, leftEyeTex2D);
	leftEyeRTTCamera->setComputeNearFarMode( osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR );
	leftEyeRTTCamera->addChild( loadedModel );
	osg::ref_ptr<osg::Camera> rightEyeRTTCamera = createRTTCamera(osg::Camera::COLOR_BUFFER, rightEyeTex2D);
	rightEyeRTTCamera->setComputeNearFarMode( osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR );
	rightEyeRTTCamera->addChild( loadedModel );
	// Create HUD cameras for each eye
	osg::ref_ptr<osg::Camera> leftEyeHUDCamera = createHUDCamera(0.0, 1.0, 0.0, 1.0);
	osg::ref_ptr<osg::Camera> rightEyeHUDCamera = createHUDCamera(0.0, 1.0, 0.0, 1.0);
	// Create quads on each camera
	osg::ref_ptr<osg::Geode> leftQuad = createScreenQuad(1.0f, 1.0f);
	leftEyeHUDCamera->addChild( leftQuad );
	osg::ref_ptr<osg::Geode> rightQuad = createScreenQuad(1.0f, 1.0f);
	rightEyeHUDCamera->addChild( rightQuad );
	// Set up shaders from the Oculus SDK documentation
	osg::ref_ptr<osg::Program> program = new osg::Program;
	osg::ref_ptr<osg::Shader> vertexShader = new osg::Shader(osg::Shader::VERTEX);
	vertexShader->loadShaderSourceFromFile(osgDB::findDataFile("warp.vert"));
	osg::ref_ptr<osg::Shader> fragmentShader = new osg::Shader(osg::Shader::FRAGMENT);
	// Fragment shader with or without correction for chromatic aberration
	bool useChromaticAberrationCorrection = true;

	if (useChromaticAberrationCorrection) {
		fragmentShader->loadShaderSourceFromFile(osgDB::findDataFile("warpWithChromeAb.frag"));
	} else {
		fragmentShader->loadShaderSourceFromFile(osgDB::findDataFile("warpWithoutChromeAb.frag"));
	}

	program->addShader( vertexShader );
	program->addShader( fragmentShader );
	// Configure state sets for both eyes
	osg::StateSet* leftEyeStateSet = leftQuad->getOrCreateStateSet();
	leftEyeStateSet->setTextureAttributeAndModes(0, leftEyeTex2D, osg::StateAttribute::ON);
	leftEyeStateSet->setAttributeAndModes( program, osg::StateAttribute::ON );
	leftEyeStateSet->addUniform( new osg::Uniform("WarpTexture", 0) );
	leftEyeStateSet->addUniform( new osg::Uniform("LensCenter", oculusDevice->lensCenter(OculusDevice::LEFT_EYE)));
	leftEyeStateSet->addUniform( new osg::Uniform("ScreenCenter", oculusDevice->screenCenter()));
	leftEyeStateSet->addUniform( new osg::Uniform("Scale", oculusDevice->scale()));
	leftEyeStateSet->addUniform( new osg::Uniform("ScaleIn", oculusDevice->scaleIn()));
	leftEyeStateSet->addUniform( new osg::Uniform("HmdWarpParam", oculusDevice->warpParameters()));
	leftEyeStateSet->addUniform( new osg::Uniform("ChromAbParam", oculusDevice->chromAbParameters()));
	osg::StateSet* rightEyeStateSet = rightQuad->getOrCreateStateSet();
	rightEyeStateSet->setTextureAttributeAndModes(0, rightEyeTex2D, osg::StateAttribute::ON);
	rightEyeStateSet->setAttributeAndModes( program, osg::StateAttribute::ON );
	rightEyeStateSet->addUniform( new osg::Uniform("WarpTexture", 0) );
	rightEyeStateSet->addUniform( new osg::Uniform("LensCenter", oculusDevice->lensCenter(OculusDevice::RIGHT_EYE)));
	rightEyeStateSet->addUniform( new osg::Uniform("ScreenCenter", oculusDevice->screenCenter()));
	rightEyeStateSet->addUniform( new osg::Uniform("Scale", oculusDevice->scale()));
	rightEyeStateSet->addUniform( new osg::Uniform("ScaleIn", oculusDevice->scaleIn()));
	rightEyeStateSet->addUniform( new osg::Uniform("HmdWarpParam", oculusDevice->warpParameters()));
	rightEyeStateSet->addUniform( new osg::Uniform("ChromAbParam", oculusDevice->chromAbParameters()));
	// Add cameras to groups
	osg::ref_ptr<osg::Group> leftRoot = new osg::Group;
	osg::ref_ptr<osg::Group> rightRoot = new osg::Group;
	leftRoot->addChild(leftEyeRTTCamera);
	leftRoot->addChild(leftEyeHUDCamera);
	rightRoot->addChild(rightEyeRTTCamera);
	rightRoot->addChild(rightEyeHUDCamera);
	osgViewer::CompositeViewer viewer(arguments);
	osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();

	if (!wsi) {
		osg::notify(osg::NOTICE)<<"Error, no WindowSystemInterface available, cannot create windows."<<std::endl;
		return 1;
	}

	unsigned int width, height;
	wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(0), width, height);
	osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
	traits->windowDecoration = false;
	traits->x = 0;
	traits->y = 0;
	traits->width = oculusDevice->hScreenResolution();
	traits->height = oculusDevice->vScreenResolution();
	traits->doubleBuffer = true;
	traits->sharedContext = 0;
	traits->sampleBuffers = true;
	traits->samples = 4;
	traits->vsync = true;
	osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits);

	if (gc.valid()) {
		gc->setClearColor(osg::Vec4(0.2f, 0.2f, 0.4f, 1.0f));
		gc->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	// Create Trackball manipulator
	osg::ref_ptr<osgGA::CameraManipulator> cameraManipulator = new osgGA::TrackballManipulator;
	const osg::BoundingSphere& bs = loadedModel->getBound();

	if (bs.valid()) {
		// Adjust view to object view
		cameraManipulator->setHomePosition(osg::Vec3(0, bs.radius()*1.5, 0), osg::Vec3(0, 0, 0), osg::Vec3(0, 0, 1));
	}

	// Create views and attach camera groups to them
	osg::ref_ptr<osgViewer::View> leftView = new osgViewer::View;
	leftView->setName("LeftEyeView");
	viewer.addView(leftView);
	leftView->setSceneData( leftRoot );
	leftView->getCamera()->setName("LeftEyeCamera");
	leftView->getCamera()->setViewport(new osg::Viewport(0, 0, oculusDevice->hScreenResolution() / 2, oculusDevice->vScreenResolution()));
	leftView->getCamera()->setGraphicsContext(gc);
	leftView->addEventHandler( new osgViewer::StatsHandler );
	leftView->setCameraManipulator(cameraManipulator);
	osg::ref_ptr<osgViewer::View>  rightView = new osgViewer::View;
	rightView->setName("RightEyeView");
	viewer.addView(rightView);
	rightView->setSceneData( rightRoot );
	rightView->getCamera()->setName("RightEyeCamera");
	rightView->getCamera()->setViewport(new osg::Viewport(oculusDevice->hScreenResolution() / 2, 0, oculusDevice->hScreenResolution() / 2, oculusDevice->vScreenResolution()));
	rightView->getCamera()->setGraphicsContext(gc);
	rightView->setCameraManipulator(cameraManipulator);

	// Realize viewer
	if (!viewer.isRealized()) {
		viewer.realize();
	}

	// Create matrix for camera position and orientation (from HMD)
	osg::Matrix cameraManipulatorViewMatrix;
	osg::Matrix orientationMatrix;
	// Get the view matrix for each eye for later use
	osg::Matrix leftEyeViewMatrix = oculusDevice->viewMatrix(OculusDevice::LEFT_EYE);
	osg::Matrix rightEyeViewMatrix = oculusDevice->viewMatrix(OculusDevice::RIGHT_EYE);
	// Set the projection matrix for each eye
	leftEyeRTTCamera->setProjectionMatrix(oculusDevice->projectionMatrix(OculusDevice::LEFT_EYE));
	rightEyeRTTCamera->setProjectionMatrix(oculusDevice->projectionMatrix(OculusDevice::RIGHT_EYE));

	// Start Viewer
	while (!viewer.done()) {
		// Get orientation from oculus sensor
		orientationMatrix.makeRotate(oculusDevice->getOrientation());
		// Get camera matrix from manipulator
		cameraManipulatorViewMatrix = cameraManipulator->getInverseMatrix();
		leftEyeRTTCamera->setViewMatrix(leftEyeViewMatrix*cameraManipulatorViewMatrix*orientationMatrix);
		rightEyeRTTCamera->setViewMatrix(rightEyeViewMatrix*cameraManipulatorViewMatrix*orientationMatrix);
		viewer.frame();
	}

	return 0;
}