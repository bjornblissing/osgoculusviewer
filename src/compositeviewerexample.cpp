/*
* compositeviewerexample.cpp
*
* Created on: Jul 03, 2013
* Author: Bjorn Blissing
*/
#include "oculusdevice.h"
#include "oculuseventhandler.h"
#include "oculushealthwarning.h"

#include <osgViewer/CompositeViewer>

#include <osgDB/ReadFile>
#include <osgGA/TrackballManipulator>


int main( int argc, char** argv )
{
	float nearClip = 0.01f;
	float farClip = 10000.0f;
	float pixelsPerDisplayPixel = 1.0;
	bool useTimewarp = true;
	osg::ref_ptr<OculusDevice> oculusDevice = new OculusDevice(nearClip, farClip, pixelsPerDisplayPixel, useTimewarp);
	// use an ArgumentParser object to manage the program arguments.
	osg::ArgumentParser arguments(&argc,argv);
	// read the scene from the list of file specified command line arguments.
	osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);

	// if not loaded assume no arguments passed in, try use default cow model instead.
	if (!loadedModel) loadedModel = osgDB::readNodeFile("cow.osgt");

	// Still no loaded model, then exit
	if (!loadedModel) return 0;

	// Add nodes to root node
	osg::ref_ptr<osg::Group> root = new osg::Group;
	root->addChild(loadedModel);

	// Add health and safety warning
	osg::ref_ptr<OculusHealthAndSafetyWarning> warning = new OculusHealthAndSafetyWarning(oculusDevice);
	root->addChild(warning.get()->getGraph());

	// Calculate the texture size
	const int textureWidth = oculusDevice->renderTargetWidth()/2;
	const int textureHeight = oculusDevice->renderTargetHeight();
	// Setup textures for the RTT cameras
	osg::ref_ptr<osg::Texture2D> textureLeft = new osg::Texture2D;
	textureLeft->setTextureSize(textureWidth, textureHeight);
	textureLeft->setInternalFormat(GL_RGBA);
	osg::ref_ptr<osg::Texture2D> textureRight = new osg::Texture2D;
	textureRight->setTextureSize(textureWidth, textureHeight);
	textureRight->setInternalFormat(GL_RGBA);
	// Initialize RTT cameras for each eye
	osg::ref_ptr<osg::Camera> leftEyeRTTCamera = oculusDevice->createRTTCamera(textureLeft, OculusDevice::LEFT, osg::Camera::ABSOLUTE_RF);
	leftEyeRTTCamera->setComputeNearFarMode( osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR );
	leftEyeRTTCamera->addChild( root );
	osg::ref_ptr<osg::Camera> rightEyeRTTCamera = oculusDevice->createRTTCamera(textureRight, OculusDevice::RIGHT, osg::Camera::ABSOLUTE_RF);
	rightEyeRTTCamera->setComputeNearFarMode( osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR );
	rightEyeRTTCamera->addChild( root );
	// Create HUD cameras for each eye
	osg::ref_ptr<osg::Camera> leftCameraWarp = oculusDevice->createWarpOrthoCamera(0.0, 1.0, 0.0, 1.0);
	osg::ref_ptr<osg::Camera> rightCameraWarp = oculusDevice->createWarpOrthoCamera(0.0, 1.0, 0.0, 1.0);

	// Create shader program
	osg::ref_ptr<osg::Program> program = oculusDevice->createShaderProgram();

	// Create distortionMesh for each camera
	osg::ref_ptr<osg::Geode> leftDistortionMesh = oculusDevice->distortionMesh(OculusDevice::LEFT, program, 0, 0, textureWidth, textureHeight, true);
	leftCameraWarp->addChild(leftDistortionMesh);
	osg::ref_ptr<osg::Geode> rightDistortionMesh = oculusDevice->distortionMesh(OculusDevice::RIGHT, program, 0, 0, textureWidth, textureHeight, true);
	rightCameraWarp->addChild(rightDistortionMesh);

	// Add pre draw camera to handle time warp
	leftCameraWarp->setPreDrawCallback(new WarpCameraPreDrawCallback(oculusDevice));
	rightCameraWarp->setPreDrawCallback(new WarpCameraPreDrawCallback(oculusDevice));

	// Attach shaders to each distortion mesh
	osg::ref_ptr<osg::StateSet> leftEyeStateSet = leftDistortionMesh->getOrCreateStateSet();
	osg::ref_ptr<osg::StateSet> rightEyeStateSet = rightDistortionMesh->getOrCreateStateSet();

	oculusDevice->applyShaderParameters(leftEyeStateSet, program, textureLeft, OculusDevice::LEFT);
	oculusDevice->applyShaderParameters(rightEyeStateSet, program, textureRight, OculusDevice::RIGHT);

	// Create Trackball manipulator
	osg::ref_ptr<osgGA::CameraManipulator> cameraManipulator = new osgGA::TrackballManipulator;
	const osg::BoundingSphere& bs = loadedModel->getBound();

	if (bs.valid()) {
		// Adjust view to object view
		cameraManipulator->setHomePosition(osg::Vec3(0, bs.radius()*1.5, 0), osg::Vec3(0, 0, 0), osg::Vec3(0, 0, 1));
	}

	// Add cameras to groups
	osg::ref_ptr<osg::Group> leftRoot = new osg::Group;
	osg::ref_ptr<osg::Group> rightRoot = new osg::Group;
	leftRoot->addChild(leftEyeRTTCamera);
	leftRoot->addChild(leftCameraWarp);
	rightRoot->addChild(rightEyeRTTCamera);
	rightRoot->addChild(rightCameraWarp);

	osg::ref_ptr<osg::GraphicsContext::Traits> traits = oculusDevice->graphicsContextTraits();
	osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits);

	// Attach to window, needed for direct mode
	oculusDevice->attachToWindow(gc);
	
	// Attach a callback to detect swap
	osg::ref_ptr<OculusSwapCallback> swapCallback = new OculusSwapCallback(oculusDevice);
	gc->setSwapCallback(swapCallback);

	// Create a composite viewer
	osgViewer::CompositeViewer viewer(arguments);

	// Create views and attach camera groups to them
	osg::ref_ptr<osgViewer::View> leftView = new osgViewer::View;
	leftView->setName("LeftEyeView");
	viewer.addView(leftView);
	leftView->setSceneData(leftRoot);
	leftView->getCamera()->setName("LeftEyeCamera");
	leftView->getCamera()->setClearColor(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
	leftView->getCamera()->setViewport(new osg::Viewport(0, 0, oculusDevice->screenResolutionWidth() / 2, oculusDevice->screenResolutionHeight()));
	leftView->getCamera()->setGraphicsContext(gc);
	// Add statistics view to only one view
	leftView->addEventHandler(new osgViewer::StatsHandler);
	// Add Oculus Keyboard Handler to only one view
	leftView->addEventHandler(new OculusEventHandler(oculusDevice));
	// Add Oculus Health and Safety event handler
	leftView->addEventHandler(new OculusWarningEventHandler(warning));
	leftView->setCameraManipulator(cameraManipulator);
	osg::ref_ptr<osgViewer::View>  rightView = new osgViewer::View;
	rightView->setName("RightEyeView");
	viewer.addView(rightView);
	rightView->setSceneData(rightRoot);
	rightView->getCamera()->setClearColor(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
	rightView->getCamera()->setName("RightEyeCamera");
	rightView->getCamera()->setViewport(new osg::Viewport(oculusDevice->screenResolutionWidth() / 2, 0, oculusDevice->screenResolutionWidth() / 2, oculusDevice->screenResolutionHeight()));
	rightView->getCamera()->setGraphicsContext(gc);
	rightView->setCameraManipulator(cameraManipulator);

	// Use sky light instead of headlight to avoid light changes when head movements
	leftView->setLightingMode(osg::View::SKY_LIGHT);
	rightView->setLightingMode(osg::View::SKY_LIGHT);

	// Realize viewer
	if (!viewer.isRealized()) {
		viewer.realize();
	}

	// Create matrix for camera position and orientation & position (from HMD)
	osg::Matrix cameraManipulatorViewMatrix;
	osg::Matrix hmdMatrix;
	// Get the view matrix for each eye for later use
	osg::Matrix leftEyeViewMatrix = oculusDevice->viewMatrixLeft();
	osg::Matrix rightEyeViewMatrix = oculusDevice->viewMatrixRight();
	// Set the projection matrix for each eye
	leftEyeRTTCamera->setProjectionMatrix(oculusDevice->projectionMatrixLeft());
	rightEyeRTTCamera->setProjectionMatrix(oculusDevice->projectionMatrixRight());

	// Start Viewer
	while (!viewer.done()) {
		// Update the pose
		oculusDevice->updatePose(swapCallback->frameIndex());
		
		osg::Vec3 position = oculusDevice->position();
		osg::Quat orientation = oculusDevice->orientation();

		hmdMatrix.makeRotate(orientation);
		hmdMatrix.preMultTranslate(position);

		leftEyeViewMatrix = oculusDevice->viewMatrixLeft();
		rightEyeViewMatrix = oculusDevice->viewMatrixRight();
		// Get camera matrix from manipulator
		cameraManipulatorViewMatrix = cameraManipulator->getInverseMatrix();
		leftEyeRTTCamera->setViewMatrix(cameraManipulatorViewMatrix*hmdMatrix*leftEyeViewMatrix);
		rightEyeRTTCamera->setViewMatrix(cameraManipulatorViewMatrix*hmdMatrix*rightEyeViewMatrix);

		// Handle health and safety warning
		if (warning.valid()) {
			warning.get()->updatePosition(cameraManipulator->getMatrix(), position, orientation);
		}

		viewer.frame();
	}

	return 0;
}