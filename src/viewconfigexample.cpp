/*
 * viewconfigexample.cpp
 *
 *  Created on: Jul 03, 2013
 *      Author: Bjorn Blissing
 */

#include <osgDB/ReadFile>
#include <osgGA/TrackballManipulator>

#include "oculuseventhandler.h"
#include "oculusviewconfig.h"


int main( int argc, char** argv )
{
	// use an ArgumentParser object to manage the program arguments.
	osg::ArgumentParser arguments(&argc,argv);
	// read the scene from the list of file specified command line arguments.
	osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);

	// if not loaded assume no arguments passed in, try use default cow model instead.
	if (!loadedModel) loadedModel = osgDB::readNodeFile("cow.osgt");

	// Still no loaded model, then exit
	if (!loadedModel) {
		osg::notify(osg::ALWAYS) << "No model could be loaded and didn't find cow.osgt, terminating.." << std::endl;
 		return 0;
	}

	// Add nodes to root node
	osg::ref_ptr<osg::Group> root = new osg::Group;
	root->addChild(loadedModel);
		
	// Subtract at least one bit of the node mask to disable rendering for main camera
	osg::Node::NodeMask sceneNodeMask = loadedModel->getNodeMask() & ~0x1;
	loadedModel->setNodeMask(sceneNodeMask);

	// Create Trackball manipulator
	osg::ref_ptr<osgGA::CameraManipulator> cameraManipulator = new osgGA::TrackballManipulator;
	const osg::BoundingSphere& bs = loadedModel->getBound();

	if (bs.valid()) {
		// Adjust view to object view
		osg::Vec3 modelCenter = bs.center();
		osg::Vec3 eyePos = bs.center() + osg::Vec3(0, bs.radius(), 0);
		cameraManipulator->setHomePosition(eyePos, modelCenter, osg::Vec3(0, 0, 1));
	}

	// Create Oculus View Config
	float nearClip = 0.01f;
	float farClip = 10000.0f;
	float pixelsPerDisplayPixel = 1.0f;
	bool useTimewarp = true;
	float worldUnitsPerMetre = 1.0f;
	osg::ref_ptr<OculusViewConfig> oculusViewConfig = new OculusViewConfig(nearClip, farClip, useTimewarp, pixelsPerDisplayPixel, worldUnitsPerMetre);
	// Add health and safety warning
	root->addChild(oculusViewConfig->warning()->getGraph());
	// Set the node mask used for scene
	oculusViewConfig->setSceneNodeMask(sceneNodeMask);
	// Create viewer
	osgViewer::Viewer viewer(arguments);
	// Add statistics handler
	viewer.addEventHandler(new osgViewer::StatsHandler);
	// Add camera manipulator
	viewer.setCameraManipulator(cameraManipulator);
	// Apply view config
	viewer.apply(oculusViewConfig);
	// Add loaded model to viewer
	viewer.setSceneData(root);
	// Start Viewer
	return viewer.run();
}
