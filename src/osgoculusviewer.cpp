/*
 * main.cpp
 *
 *  Created on: Jul 03, 2013
 *      Author: Bjorn Blissing
 */

#include <osgDB/ReadFile>
#include <osgGA/TrackballManipulator>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include "oculusViewConfig.h"

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

	// Create Oculus View Config
	osg::ref_ptr<OculusViewConfig> oculusViewConfig = new OculusViewConfig;
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
