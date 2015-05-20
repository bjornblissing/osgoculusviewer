/*
 * oculuseventhandlersdk.cpp
 *
 *  Created on: May 20, 2015
 *      Author: Bjorn Blissing
 */

#ifndef _OSG_OCULUSEVENTHANDLERSDK_H_
#define _OSG_OCULUSEVENTHANDLERSDK_H_

#include <osgViewer/ViewerEventHandlers>

// Forward declaration
class OculusDeviceSDK;


class OculusEventHandlerSDK : public osgGA::GUIEventHandler
{
public:
	explicit OculusEventHandlerSDK(osg::ref_ptr<OculusDeviceSDK> device) : m_oculusDevice(device) {}
	virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&);
protected:
	osg::observer_ptr<OculusDeviceSDK> m_oculusDevice;

};

#endif /* _OSG_OCULUSEVENTHANDLERSDK_H_ */
