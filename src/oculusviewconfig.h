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
			m_useSensorPrediction(true),
			m_useCustomScaleFactor(false),
			m_customScaleFactor(1.25f),
			m_nearClip(0.3f),
			m_farClip(5000.0f),
			m_predictionDelta(0.03f),
			m_sceneNodeMask(0x1),
			m_device(0) {
			m_device = new OculusDevice;
		}
		void setEnableOrientationsFromHMD(bool useOrientations) { m_useOrientations = useOrientations; }
		void setEnableChromaticAberrationCorrection(bool correctionEnabled) { m_useChromaticAberrationCorrection = correctionEnabled; }
		void setNearClip(float nearClip) { m_nearClip = nearClip; }
		void setFarClip(float farclip) { m_farClip = farclip; }
		void setSensorPredictionEnabled(bool prediction);
		void setSensorPredictionDelta(float delta) { m_predictionDelta = delta; }
		void resetSensorOrientation() { if (m_device) m_device->resetSensorOrientation(); }
		void setUseDefaultScaleFactor(bool useDefault) { m_useCustomScaleFactor = !useDefault; }
		void setCustomScaleFactor(const float& customScaleFactor) { m_useCustomScaleFactor = true; m_customScaleFactor = customScaleFactor; }
		void setSceneNodeMask(osg::Node::NodeMask nodeMask) { m_sceneNodeMask = nodeMask; }
		virtual void configure(osgViewer::View& view) const;
	protected:
		~OculusViewConfig() {};

		osg::Camera* createRTTCamera(osg::Texture* tex, osg::GraphicsContext* gc) const;
		osg::Camera* createHUDCamera(double left, double right, double bottom, double top, osg::GraphicsContext* gc) const;
		osg::Geode*  createHUDQuad( float width, float height, float scale = 1.0f ) const;
		void applyShaderParameters(osg::StateSet* stateSet, osg::Program* program, 
			osg::Texture2D* texture, OculusDevice::EyeSide eye) const;

		bool m_configured;
		bool m_useOrientations;
		bool m_useChromaticAberrationCorrection;
		bool m_useSensorPrediction;
		bool m_useCustomScaleFactor;
		float m_customScaleFactor;
		float m_nearClip;
		float m_farClip;
		float m_predictionDelta;
		osg::Node::NodeMask m_sceneNodeMask;

		osg::ref_ptr<OculusDevice> m_device;
};

class OculusViewConfigOrientationCallback :  public osg::NodeCallback {
	public:
		OculusViewConfigOrientationCallback(osg::ref_ptr<osg::Camera> rttLeft, osg::ref_ptr<osg::Camera> rttRight, osg::observer_ptr<OculusDevice> device) : m_cameraRTTLeft(rttLeft), m_cameraRTTRight(rttRight), m_device(device) {};
		virtual void operator() (osg::Node* node, osg::NodeVisitor* nv);
	protected:
		osg::observer_ptr<osg::Camera> m_cameraRTTLeft, m_cameraRTTRight;
		osg::observer_ptr<OculusDevice> m_device;
};

#endif /* _OSG_OCULUSVIEWCONFIG_H_ */