/*
 * HMDCamera.h
 *
 *  Created on: Jun 30, 2013
 *      Author: Jan Ciger
 */

#ifndef _OSG_HMDCAMERA_H_
#define _OSG_HMDCAMERA_H_

#include <osg/Group>

#include "oculusdevice.h"

namespace osgViewer {
	class View;
	class NodeVisitor;
}

class HMDCamera: public osg::Group {
	public:
		HMDCamera(osgViewer::View* view, osg::ref_ptr<OculusDevice> dev);
		virtual void traverse(osg::NodeVisitor& nv);
		void setChromaticAberrationCorrection(bool correctionEnabled) { m_useChromaticAberrationCorrection = correctionEnabled; }
		void setSceneNodeMask(osg::Node::NodeMask nodeMask) { m_sceneNodeMask = nodeMask; }
	protected:
		~HMDCamera();
		virtual void configure();

		osg::Camera* createRTTCamera(osg::Texture* tex, osg::GraphicsContext* gc) const;
		osg::Camera* createHUDCamera(double left, double right, double bottom, double top, osg::GraphicsContext* gc) const;
		osg::Geode*  createHUDQuad(float width, float height, float scale = 1.0f) const;
		void applyShaderParameters(osg::StateSet* stateSet, osg::Program* program, 
			osg::Texture2D* texture, OculusDevice::EyeSide eye) const;
		bool m_configured;
		bool m_useChromaticAberrationCorrection;
		osg::observer_ptr<osgViewer::View> m_view;
		osg::observer_ptr<osg::Camera> m_cameraRTTLeft, m_cameraRTTRight;
		osg::observer_ptr<OculusDevice> m_device;
		osg::Node::NodeMask m_sceneNodeMask;
};

#endif /* _OSG_HMDCAMERA_H_ */
