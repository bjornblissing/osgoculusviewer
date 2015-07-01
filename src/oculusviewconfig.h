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
#include "oculushealthwarning.h"


class OculusViewConfig : public osgViewer::ViewConfig {
	public:
		OculusViewConfig(float nearClip = 0.01f, float farClip = 10000.0f, bool useTimewarp = true, float pixelsPerDisplayPixel = 1.0f, float worldUnitsPerMetre = 1.0f) : osgViewer::ViewConfig(),
			m_configured(false),
			m_device(0),
			m_warning(0) {
			m_device = new OculusDevice(nearClip, farClip, useTimewarp, pixelsPerDisplayPixel, worldUnitsPerMetre);
			m_warning = new OculusHealthAndSafetyWarning();
		}
		virtual void configure(osgViewer::View& view) const;
		OculusDevice* oculusDevice() const { return m_device.get(); }
		OculusHealthAndSafetyWarning* warning() const { return m_warning.get(); }
	protected:
		~OculusViewConfig() {};

		bool m_configured;

		osg::ref_ptr<OculusDevice> m_device;
		osg::ref_ptr<OculusHealthAndSafetyWarning> m_warning;
};


class OculusViewConfigOrientationCallback :  public osg::NodeCallback {
	public:
		OculusViewConfigOrientationCallback(osg::observer_ptr<osg::Camera> rttLeft, 
			osg::observer_ptr<osg::Camera> rttRight, 
			osg::observer_ptr<OculusDevice> device,
			osg::observer_ptr<OculusSwapCallback> swapCallback,
			osg::observer_ptr<OculusHealthAndSafetyWarning> warning) : m_cameraRTTLeft(rttLeft), m_cameraRTTRight(rttRight), m_device(device), m_swapCallback(swapCallback), m_warning(warning) {};
		virtual void operator() (osg::Node* node, osg::NodeVisitor* nv);
	protected:
		osg::observer_ptr<osg::Camera> m_cameraRTTLeft, m_cameraRTTRight;
		osg::observer_ptr<OculusDevice> m_device;
		osg::observer_ptr<OculusSwapCallback> m_swapCallback;
		osg::observer_ptr<OculusHealthAndSafetyWarning> m_warning;
		
};

#endif /* _OSG_OCULUSVIEWCONFIG_H_ */