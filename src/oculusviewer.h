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
namespace osgViewer
{
	class View;
}


class OculusViewer : public osg::Group
{
public:
	OculusViewer(osgViewer::View* view, osg::ref_ptr<OculusDevice> dev, osg::ref_ptr<OculusRealizeOperation> realizeOperation) : osg::Group(),
		m_configured(false),
		m_view(view),
		m_cameraRTTLeft(nullptr), m_cameraRTTRight(nullptr),
		m_device(dev),
		m_realizeOperation(realizeOperation)
	{};
	virtual void traverse(osg::NodeVisitor& nv);
protected:
	~OculusViewer() {};
	virtual void configure();

	bool m_configured;

	osg::observer_ptr<osgViewer::View> m_view;
	osg::observer_ptr<osg::Camera> m_cameraRTTLeft, m_cameraRTTRight;
	osg::observer_ptr<OculusDevice> m_device;
	osg::observer_ptr<OculusRealizeOperation> m_realizeOperation;
};

#endif /* _OSG_OCULUSVIEWER_H_ */
