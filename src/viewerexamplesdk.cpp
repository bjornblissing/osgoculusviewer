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
		osg::notify(osg::NOTICE) << "Error, no loaded model and couldn't find cow.osgt" << std::endl;
		return 0;
	}
	// Create Trackball manipulator
	osg::ref_ptr<osgGA::CameraManipulator> cameraManipulator = new osgGA::TrackballManipulator;
	const osg::BoundingSphere& bs = loadedModel->getBound();

	if (bs.valid()) {
		// Adjust view to object view
		cameraManipulator->setHomePosition(osg::Vec3(0, bs.radius()*1.5, 0), osg::Vec3(0, 0, 0), osg::Vec3(0, 0, 1));
	}

	// Open the HMD
	osg::ref_ptr<OculusDeviceSDK> oculusDevice = new OculusDeviceSDK();
	oculusDevice->initialize();

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
		gc->setSwapCallback(new OculusSwapCallback());
	}

	osgViewer::Viewer viewer(arguments);
	viewer.getCamera()->setGraphicsContext(gc);
	viewer.getCamera()->setViewport(0, 0, traits->width, traits->height);

	// Disable automatic computation of near and far plane
	viewer.getCamera()->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
	viewer.setCameraManipulator(cameraManipulator);

	// Add oculus geode to initialize rendering
	osg::ref_ptr<OculusDrawable> oculusDrawable = new OculusDrawable;
	oculusDrawable->setOculusDevice(oculusDevice.get());
	oculusDrawable->setView(&viewer);
	osg::ref_ptr<osg::Geode> oculusGeode = new osg::Geode;
	oculusGeode->addDrawable(oculusDrawable);

	// Create world root and add this to viewer
	osg::ref_ptr<osg::Group> root = new osg::Group;
	root->addChild(loadedModel);
	root->addChild(oculusGeode);
	viewer.setSceneData(root);

	// Realize viewer
	viewer.realize();
	// Run one frame to make sure that textures are created
	viewer.frame();

	// Start Viewer
	while (!viewer.done()) {
		oculusDevice->beginFrame();
		oculusDevice->getEyePose();
		viewer.frame();
		oculusDevice->endFrame();
	}

	return 0;
}

