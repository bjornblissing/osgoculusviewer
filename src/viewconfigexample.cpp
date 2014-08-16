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
	if (!loadedModel) return 0;

	// Subtract at least one bit of the node mask to disable rendering for main camera
	osg::Node::NodeMask sceneNodeMask = loadedModel->getNodeMask() & ~0x1;
	loadedModel->setNodeMask(sceneNodeMask);
		 
	// Create Oculus View Config
	float nearClip = 0.01f;
	float farClip = 10000.0f;
	bool useTimewarp = true;
	osg::ref_ptr<OculusViewConfig> oculusViewConfig = new OculusViewConfig(nearClip, farClip, useTimewarp);
	// Set the node mask used for scene
	oculusViewConfig->setSceneNodeMask(sceneNodeMask);
	// Create viewer
	osgViewer::Viewer viewer(arguments);
	// Add statistics handler
	viewer.addEventHandler(new osgViewer::StatsHandler);
	// Apply view config
	viewer.apply(oculusViewConfig);
	// Add loaded model to viewer
	viewer.setSceneData(loadedModel);
	// Start Viewer
	return viewer.run();
}
