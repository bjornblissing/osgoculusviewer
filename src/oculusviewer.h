/*
 * oculusviewer.h
 *
 *  Created on: Jun 30, 2013
 *      Author: Jan Ciger & Björn Blissing
 */

#ifndef _OSG_OCULUSVIEWER_H_
#define _OSG_OCULUSVIEWER_H_

#include <osg/Group>

#include "oculusdevice.h"

// Forward declaration
namespace osgViewer {
	class View;
}

class OculusViewer : public osg::Group {
	public:
		OculusViewer(osgViewer::View* view, osg::ref_ptr<OculusDevice> dev) : osg::Group(), 
			m_configured(false), 
			m_view(view), 
			m_cameraRTTLeft(0), m_cameraRTTRight(0), 
			m_device(dev), 
			m_swapCallback(0),
			m_sceneNodeMask(0x1) {};
		virtual void traverse(osg::NodeVisitor& nv);
		void setSceneNodeMask(osg::Node::NodeMask nodeMask) { m_sceneNodeMask = nodeMask; }
	protected:
		~OculusViewer() {};
		virtual void configure();

		bool m_configured;

		osg::observer_ptr<osgViewer::View> m_view;
		osg::observer_ptr<osg::Camera> m_cameraRTTLeft, m_cameraRTTRight;
		osg::observer_ptr<OculusDevice> m_device;
		osg::ref_ptr<OculusSwapCallback> m_swapCallback;
		osg::Node::NodeMask m_sceneNodeMask;
};

#endif /* _OSG_OCULUSVIEWER_H_ */
