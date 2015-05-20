/*
* viewerexamplesdk.cpp
*
*  Created on: Oct 01, 2014
*      Author: Bjorn Blissing
*/

#include <osgDB/ReadFile>
#include <osgGA/TrackballManipulator>
#include <osgViewer/Viewer>

#include "oculusdevicesdk.h"
#include "oculuseventhandlersdk.h"

int main(int argc, char** argv)
{
#if _WIN32
	// Initialize the Oculus Rendering Shim. Must be done before any OpenGL calls
	// Only for Direct mode
	ovr_InitializeRenderingShim();
#endif

	// use an ArgumentParser object to manage the program arguments.
	osg::ArgumentParser arguments(&argc, argv);
	// read the scene from the list of file specified command line arguments.
	osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);

	// if not loaded assume no arguments passed in, try use default cow model instead.
	if (!loadedModel) loadedModel = osgDB::readNodeFile("cow.osgt");

	// Still no loaded model, then exit
	if (!loadedModel) {
		osg::notify(osg::ALWAYS) << "No model could be loaded and didn't find cow.osgt, terminating.." << std::endl;
		return 0;
	}

	// Create Trackball manipulator
	osg::ref_ptr<osgGA::CameraManipulator> cameraManipulator = new osgGA::TrackballManipulator;
	const osg::BoundingSphere& bs = loadedModel->getBound();

	if (bs.valid()) {
		// Adjust view to object view
		osg::Vec3 modelCenter = bs.center();
		osg::Vec3 eyePos = bs.center() + osg::Vec3(0, bs.radius(), 0);
		cameraManipulator->setHomePosition(eyePos, modelCenter, osg::Vec3(0, 0, 1));
	}

	// Open the HMD
	float nearClip = 0.01f;
	float farClip = 10000.0f;
	float pixelsPerDisplayPixel = 1.0;
	float worldUnitsPerMetre = 1.0f;
	osg::ref_ptr<OculusDeviceSDK> oculusDevice = new OculusDeviceSDK(nearClip, farClip, worldUnitsPerMetre, pixelsPerDisplayPixel);

	// Get the suggested context traits
	osg::ref_ptr<osg::GraphicsContext::Traits> traits = oculusDevice->graphicsContextTraits();

	// Create a graphic context based on our desired traits
	osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits);
	if (!gc) {
		osg::notify(osg::NOTICE) << "Error, GraphicsWindow has not been created successfully" << std::endl;
		return 1;
	}

	if (gc.valid()) {
		gc->setClearColor(osg::Vec4(0.2f, 0.2f, 0.4f, 1.0f));
		gc->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Make sure that Oculus SDK handles SwapCallbacks
		gc->setSwapCallback(new OculusSwapCallbackSDK());
	}

	osgViewer::Viewer viewer(arguments);
	viewer.getCamera()->setGraphicsContext(gc);
	viewer.getCamera()->setViewport(0, 0, traits->width, traits->height);

	// Force single threaded to make sure that no other thread can use the GL context
	viewer.setThreadingModel(osgViewer::Viewer::SingleThreaded);

	// Create textures during viewer realization
	viewer.setRealizeOperation(new OculusRealizeOperation(oculusDevice, &viewer));

	// Disable automatic computation of near and far plane
	viewer.getCamera()->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
	viewer.setCameraManipulator(cameraManipulator);
	viewer.realize();
	
	// Add nodes to root node
	osg::ref_ptr<osg::Group> root = new osg::Group;
	root->addChild(oculusDevice->healthWarning()->getGraph());
	// Start timer
	oculusDevice->getHealthAndSafetyDisplayState();

	root->addChild(loadedModel);
	viewer.setSceneData(root);

	// Add statistics handler
	viewer.addEventHandler(new osgViewer::StatsHandler);

	// Add Oculus Keyboard Handler to only one view
	viewer.addEventHandler(new OculusEventHandlerSDK(oculusDevice));

	// Add Oculus Health and Safety event handler
	viewer.addEventHandler(new OculusWarningEventHandlerSDK(oculusDevice, oculusDevice->healthWarning()));
	
	// Start Viewer
	while (!viewer.done()) {
		oculusDevice->beginFrame();
		viewer.frame();
		oculusDevice->endFrame();
	}

	return 0;
}

