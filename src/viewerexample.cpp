/*
 * viewerexample.cpp
 *
 *  Created on: Jul 03, 2013
 *      Author: Bjorn Blissing
 */

#include <osgDB/ReadFile>
#include <osgGA/TrackballManipulator>
#include <osgViewer/Viewer>
#include <osgUtil/GLObjectsVisitor>

#include "oculusviewer.h"
#include "oculuseventhandler.h"

int main( int argc, char** argv )
{
	// use an ArgumentParser object to manage the program arguments.
	osg::ArgumentParser arguments(&argc, argv);
	// read the scene from the list of file specified command line arguments.
	osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);

	// if not loaded assume no arguments passed in, try use default cow model instead.
	if (!loadedModel) { loadedModel = osgDB::readNodeFile("cow.osgt"); }

	// Still no loaded model, then exit
	if (!loadedModel)
	{
		osg::notify(osg::ALWAYS) << "No model could be loaded and didn't find cow.osgt, terminating.." << std::endl;
		return 0;
	}

	// Create Trackball manipulator
	osg::ref_ptr<osgGA::CameraManipulator> cameraManipulator = new osgGA::TrackballManipulator;
	const osg::BoundingSphere& bs = loadedModel->getBound();

	if (bs.valid())
	{
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
	int samples = 4;
	osg::ref_ptr<OculusDevice> oculusDevice = new OculusDevice(nearClip, farClip, pixelsPerDisplayPixel, worldUnitsPerMetre, samples);

	// Exit if we do not have a valid HMD present
	if (!oculusDevice->hmdPresent())
	{
		osg::notify(osg::FATAL) << "Error: No valid HMD present!" << std::endl;
		return 1;
	}

	// Get the suggested context traits
	osg::ref_ptr<osg::GraphicsContext::Traits> traits = oculusDevice->graphicsContextTraits();
	traits->windowName = "OsgOculusViewerExample";

	// Create a graphic context based on our desired traits
	osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits);

	if (!gc)
	{
		osg::notify(osg::NOTICE) << "Error, GraphicsWindow has not been created successfully" << std::endl;
		return 1;
	}

	if (gc.valid())
	{
		gc->setClearColor(osg::Vec4(0.2f, 0.2f, 0.4f, 1.0f));
		gc->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	osgViewer::Viewer viewer(arguments);
	// Force single threaded to make sure that no other thread can use the GL context
	viewer.setThreadingModel(osgViewer::Viewer::SingleThreaded);
	viewer.getCamera()->setGraphicsContext(gc);
	viewer.getCamera()->setViewport(0, 0, traits->width, traits->height);

	// Disable automatic computation of near and far plane
	viewer.getCamera()->setComputeNearFarMode( osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR );
	viewer.setCameraManipulator(cameraManipulator);

	// Things to do when viewer is realized
	osg::ref_ptr<OculusRealizeOperation> oculusRealizeOperation = new OculusRealizeOperation(oculusDevice);
	viewer.setRealizeOperation(oculusRealizeOperation.get());

	osg::ref_ptr<OculusViewer> oculusViewer = new OculusViewer(&viewer, oculusDevice, oculusRealizeOperation);
	oculusViewer->addChild(loadedModel);
	viewer.setSceneData(oculusViewer);
	// Add statistics handler
	viewer.addEventHandler(new osgViewer::StatsHandler);

	viewer.addEventHandler(new OculusEventHandler(oculusDevice));

	viewer.run();

	return 0;
}
