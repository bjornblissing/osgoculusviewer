/*
 * viewerexample.cpp
 *
 *  Created on: Jul 03, 2013
 *      Author: Bjorn Blissing
 */

#include <osgDB/ReadFile>
#include <osgGA/TrackballManipulator>

#include "oculusviewer.h"
#include "oculuseventhandler.h"


int main( int argc, char** argv )
{
	// use an ArgumentParser object to manage the program arguments.
	osg::ArgumentParser arguments(&argc,argv);
	// read the scene from the list of file specified command line arguments.
	osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);

	// if not loaded assume no arguments passed in, try use default cow model instead.
	if (!loadedModel) loadedModel = osgDB::readNodeFile("cow.osgt");

	// Still no loaded model, then exit
	if (!loadedModel) return 0;

	// Create Trackball manipulator
	osg::ref_ptr<osgGA::CameraManipulator> cameraManipulator = new osgGA::TrackballManipulator;
	const osg::BoundingSphere& bs = loadedModel->getBound();

	if (bs.valid()) {
		// Adjust view to object view
		cameraManipulator->setHomePosition(osg::Vec3(0, bs.radius()*1.5, 0), osg::Vec3(0, 0, 0), osg::Vec3(0, 0, 1));
	}

	// Open the HMD
	float nearClip = 0.01f;
	float farClip = 10000.0f;
	float pixelsPerDisplayPixel = 1.0;
	bool useTimewarp = true;
	osg::ref_ptr<OculusDevice> oculusDevice = new OculusDevice(nearClip, farClip, pixelsPerDisplayPixel, useTimewarp);

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
	}

	osgViewer::Viewer viewer(arguments);
	viewer.getCamera()->setGraphicsContext(gc);
	viewer.getCamera()->setViewport(0, 0, traits->width, traits->height);

	// Disable automatic computation of near and far plane
	viewer.getCamera()->setComputeNearFarMode( osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR );
	viewer.setCameraManipulator(cameraManipulator);
	viewer.realize();
	
	// Subtract at least one bit of the node mask to disable rendering for main camera
	osg::Node::NodeMask sceneNodeMask = loadedModel->getNodeMask() & ~0x1;
	loadedModel->setNodeMask(sceneNodeMask);

	osg::ref_ptr<OculusViewer> oculusViewer = new OculusViewer(&viewer, oculusDevice);
	oculusViewer->setSceneNodeMask(sceneNodeMask);
	oculusViewer->addChild(loadedModel);
	viewer.setSceneData(oculusViewer);
	// Add statistics handler
	viewer.addEventHandler(new osgViewer::StatsHandler);
	// Add Oculus Keyboard Handler to only one view
	viewer.addEventHandler(new OculusEventHandler(oculusDevice));
	// Start Viewer
	viewer.run();
	return 0;
}
