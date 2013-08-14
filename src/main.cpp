/*
 * main.cpp
 *
 *  Created on: Jul 03, 2013
 *      Author: Bjorn Blissing
 */

#include <iostream>
#include <osg/io_utils>

#include <osg/Texture2D>
#include <osg/Program>
#include <osg/StateSet>
#include <osg/Camera>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/PolygonMode>
#include <osg/MatrixTransform>

#include <osgDB/ReadFile>
#include <osgGA/TrackballManipulator>
#include <osgViewer/Viewer>

#include "oculusdevice.h"
#include "HMDCamera.h"

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
	OculusDevice* oculusDevice = new OculusDevice;
	// Set the scaling of the texture, using the recommend 25% from the SDK
	oculusDevice->setScaleFactor(1.25f);
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

	osgViewer::Viewer viewer(arguments);
	viewer.getCamera()->setGraphicsContext(gc);
	viewer.getCamera()->setViewport(0, 0, traits->width, traits->height);
	viewer.setCameraManipulator(cameraManipulator);
	viewer.realize();
	osg::ref_ptr<HMDCamera> hmd_camera = new HMDCamera(&viewer, oculusDevice);
	hmd_camera->setChromaticAberrationCorrection(true);
	hmd_camera->addChild(loadedModel);
	viewer.setSceneData(hmd_camera);
	// Create matrix for camera position and orientation (from HMD)
	osg::Matrix cameraManipulatorViewMatrix;
	osg::Matrix orientationMatrix;
	// Start Viewer
	viewer.run();
	return 0;
}
