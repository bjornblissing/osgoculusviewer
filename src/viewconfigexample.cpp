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

#include <osg/Camera>


// Update callback for the camera which "mirrors" the view inside the oculus,
// matching its tracked position
class MirrorViewCallback : public osg::NodeCallback {
public:
   MirrorViewCallback(osg::Camera* mirrorCamera, OculusDevice* oculus) :
      mMirrorCamera(mirrorCamera),
      mOculusDevice(oculus)
   {}

   virtual void operator() (osg::Node* node, osg::NodeVisitor* nv)
   {
      osg::View* view = mMirrorCamera->getView();

      if (view) {
         
         osg::Vec3 position = mOculusDevice.get()->position();
         osg::Quat orientation = mOculusDevice.get()->orientation();
         osg::Matrix viewMx;
         
         viewMx.preMultRotate(orientation);
         viewMx.preMultTranslate(position);
         
         // Nasty hack to update the view offset for each of the slave cameras
         // There doesn't seem to be an accessor for this, fortunately the offsets are public
         view->findSlaveForCamera(mMirrorCamera.get())->_viewOffset = viewMx;         
      }

      traverse(node, nv);
   }

protected:
   osg::observer_ptr<osg::Camera> mMirrorCamera;
   osg::observer_ptr<OculusDevice> mOculusDevice;
};





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


   
   if (arguments.find("--mirror") != -1)
   {
      // read the screen id where to create the mirror window, 0 by default
      int screenId = 0;
      arguments.read("--mirror", screenId);

      // create a window with an aspect ratio consistent with the main camera,
      // based on its projection matrix
      float fovy, ar, znear, zfar;
      viewer.getCamera()->getProjectionMatrix().getPerspective(fovy, ar, znear, zfar);
      int mirrorWidth = 800;
      int mirrorHeight = (float)mirrorWidth / ar;

      // render the view on a mirror window as well
      osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
      traits->screenNum = screenId;
      traits->x = 100;
      traits->y = 100;
      traits->width = mirrorWidth;
      traits->height = mirrorHeight;
      traits->windowDecoration = true;
      traits->doubleBuffer = true;
      traits->sharedContext = 0;

      osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
      if (!gc.valid())
      {
         OSG_ALWAYS << "[Mirror View] Could NOT create the mirror GraphicsWindow successfully on screen number: " << screenId << std::endl;
      }

      osg::Camera* mirrorCam = new osg::Camera;
      mirrorCam->setGraphicsContext(gc.get());
      mirrorCam->setViewport(new osg::Viewport(0, 0, mirrorWidth, mirrorHeight));
      mirrorCam->setCullMask(0xffffffff);

      // add as slave
      viewer.addSlave(mirrorCam);
      // update callback to match the oculus tracked camera, without any eye offset
      mirrorCam->setUpdateCallback(new MirrorViewCallback(mirrorCam, oculusViewConfig->getOculusDevice()));
   }


	// Add loaded model to viewer
	viewer.setSceneData(loadedModel);
	// Start Viewer
	return viewer.run();
}
