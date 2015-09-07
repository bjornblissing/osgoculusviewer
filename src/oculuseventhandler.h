/*
 * oculuseventhandler.h
 *
 *  Created on: Jun 24, 2014
 *      Author: Bjorn Blissing
 */

#ifndef _OSG_OCULUSEVENTHANDLER_H_
#define _OSG_OCULUSEVENTHANDLER_H_

#include <osgViewer/ViewerEventHandlers>

// Forward declaration
class OculusDevice;


class OculusEventHandler : public osgGA::GUIEventHandler
{
public:
	explicit OculusEventHandler(osg::ref_ptr<OculusDevice> device) : m_oculusDevice(device), m_usePositionalTracking(true) {}
	virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&);
protected:
	osg::ref_ptr<OculusDevice> m_oculusDevice;
	bool m_usePositionalTracking;

};

#endif /* _OSG_OCULUSEVENTHANDLER_H_ */
