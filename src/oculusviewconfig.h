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
		OculusViewConfig() : osgViewer::ViewConfig(),
			m_configured(false),
			m_useChromaticAberrationCorrection(true),
			m_useOrientations(true),
			m_useCustomScaleFactor(false),
			m_useTimeWarp(true),
			m_customScaleFactor(1.0f),
			m_nearClip(0.01f),
			m_farClip(10000.0f),
			m_sceneNodeMask(0x1),
			m_device(0) {
			m_device = new OculusDevice;
		}
		void setEnableOrientationsFromHMD(bool useOrientations) { m_useOrientations = useOrientations; }
		void setEnableChromaticAberrationCorrection(bool correctionEnabled) { m_useChromaticAberrationCorrection = correctionEnabled; }
		void setNearClip(float nearClip) { m_nearClip = nearClip; }
		void setFarClip(float farclip) { m_farClip = farclip; }
		void resetSensorOrientation() { if (m_device) m_device->resetSensorOrientation(); }
		void setUseDefaultScaleFactor(bool useDefault) { m_useCustomScaleFactor = !useDefault; }
		void setCustomScaleFactor(const float& customScaleFactor) { m_useCustomScaleFactor = true; m_customScaleFactor = customScaleFactor; }
		void setSceneNodeMask(osg::Node::NodeMask nodeMask) { m_sceneNodeMask = nodeMask; }
		virtual void configure(osgViewer::View& view) const;
	protected:
		~OculusViewConfig() {};

		bool m_configured;
		bool m_useOrientations;
		bool m_useChromaticAberrationCorrection;
		bool m_useTimeWarp;
		bool m_useCustomScaleFactor;
		float m_customScaleFactor;
		float m_nearClip;
		float m_farClip;
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