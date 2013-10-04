/*
 * HMDCamera.h
 *
 *  Created on: Jun 30, 2013
 *      Author: Jan Ciger
 */

#ifndef _OSG_HMDCAMERA_H_
#define _OSG_HMDCAMERA_H_

#include <osg/Group>

// Forward declarations
class OculusDevice;
namespace osgViewer {
	class View;
	class NodeVisitor;
}

class HMDCamera: public osg::Group {
	public:
		HMDCamera(osgViewer::View* view, osg::ref_ptr<OculusDevice> dev);
		virtual void traverse(osg::NodeVisitor& nv);
		void setChromaticAberrationCorrection(bool correctionEnabled) { m_chromaticAberrationCorrection = correctionEnabled; }
	protected:
		~HMDCamera();
		virtual void configure();

		osg::Camera* createRTTCamera(osg::Camera::BufferComponent buffer, osg::Texture* tex);
		osg::Camera* createHUDCamera(double left, double right, double bottom, double top);
		osg::Geode*  createHUDQuad( float width, float height, float scale = 1.0f );

		bool m_configured;
		bool m_chromaticAberrationCorrection;
		osg::observer_ptr<osgViewer::View> m_view;
		osg::observer_ptr<osg::Camera> m_l_rtt, m_r_rtt;
		osg::observer_ptr<OculusDevice> m_dev;
};

#endif /* _OSG_HMDCAMERA_H_ */
