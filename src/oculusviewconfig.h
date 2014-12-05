/*
 * oculusviewconfig.h
 *
 *  Created on: Sep 26, 2013
 *      Author: Bjorn Blissing & Jan Cigar
 */

#ifndef _OSG_OCULUSVIEWCONFIG_H_
#define _OSG_OCULUSVIEWCONFIG_H_ 1

#include <osgViewer/View>
#include "oculusdevice.h"

class OculusViewConfig : public osgViewer::ViewConfig {
	public:
		OculusViewConfig(float nearClip=0.01f, float farClip=10000.0f, float pixelsPerDisplayPixel=1.0f, bool useTimewarp=true) : osgViewer::ViewConfig(),
			m_configured(false),
			m_sceneNodeMask(0x1),
			m_device(0) {
			m_device = new OculusDevice(nearClip, farClip, pixelsPerDisplayPixel, useTimewarp);
		}
		void setSceneNodeMask(osg::Node::NodeMask nodeMask) { m_sceneNodeMask = nodeMask; }
		virtual void configure(osgViewer::View& view) const;
		OculusDevice* oculusDevice() const { return m_device.get(); }
	protected:
		~OculusViewConfig() {};

		bool m_configured;
		osg::Node::NodeMask m_sceneNodeMask;

		osg::ref_ptr<OculusDevice> m_device;
};

class OculusViewConfigOrientationCallback :  public osg::NodeCallback {
	public:
		OculusViewConfigOrientationCallback(osg::ref_ptr<osg::Camera> rttLeft, 
			osg::ref_ptr<osg::Camera> rttRight, 
			osg::ref_ptr<OculusDevice> device,
			osg::ref_ptr<OculusSwapCallback> swapCallback) : m_cameraRTTLeft(rttLeft), m_cameraRTTRight(rttRight), m_device(device), m_swapCallback(swapCallback) {};
		virtual void operator() (osg::Node* node, osg::NodeVisitor* nv);
	protected:
		osg::observer_ptr<osg::Camera> m_cameraRTTLeft, m_cameraRTTRight;
		osg::observer_ptr<OculusDevice> m_device;
		osg::observer_ptr<OculusSwapCallback> m_swapCallback;
};

#endif /* _OSG_OCULUSVIEWCONFIG_H_ */