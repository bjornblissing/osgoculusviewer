/*
 * oculusviewer.h
 *
 *  Created on: Jun 30, 2013
 *      Author: Jan Ciger
 */

#ifndef _OSG_OCULUSVIEWER_H_
#define _OSG_OCULUSVIEWER_H_

#include <osg/Group>

#include "oculusdevice.h"

namespace osgViewer {
	class View;
	class NodeVisitor;
}

class OculusViewer : public osg::Group {
	public:
		OculusViewer(osgViewer::View* view, osg::ref_ptr<OculusDevice> dev);
		virtual void traverse(osg::NodeVisitor& nv);
		void setSceneNodeMask(osg::Node::NodeMask nodeMask) { m_sceneNodeMask = nodeMask; }
	protected:
		~OculusViewer() {};
		virtual void configure();

		osg::Camera* createRTTCamera(osg::Texture* tex, osg::GraphicsContext* gc, OculusDevice::Eye eye) const;
		osg::Camera* createWarpOrthoCamera(double left, double right, double bottom, double top, osg::GraphicsContext* gc) const;
		void applyShaderParameters(osg::StateSet* stateSet, osg::Program* program, osg::Texture2D* texture, OculusDevice::Eye eye) const;

		bool m_configured;
		bool m_useChromaticAberrationCorrection;
		bool m_useTimeWarp;
		bool m_useCustomScaleFactor;
		float m_customScaleFactor;
		float m_nearClip;
		float m_farClip;

		osg::observer_ptr<osgViewer::View> m_view;
		osg::observer_ptr<osg::Camera> m_cameraRTTLeft, m_cameraRTTRight;
		osg::observer_ptr<OculusDevice> m_device;
		osg::ref_ptr<OculusSwapCallback> m_swapCallback;
		osg::Node::NodeMask m_sceneNodeMask;
};

#endif /* _OSG_OCULUSVIEWER_H_ */
