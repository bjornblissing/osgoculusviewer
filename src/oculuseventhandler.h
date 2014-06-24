/*
 * oculuseventhandler.h
 *
 *  Created on: Jun 24, 2014
 *      Author: Bjorn Blissing
 */

#ifndef _OSG_OCULUSEVENTHANDLER_H_
#define _OSG_OCULUSEVENTHANDLER_H_

#include <osgViewer/ViewerEventHandlers>

#include "oculusviewconfig.h"

class OculusEventHandler : public osgGA::GUIEventHandler
{
public:
	OculusEventHandler(osg::ref_ptr<OculusViewConfig> viewConfig) : m_oculusViewConfig(viewConfig) {}
	virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&);
protected:
	osg::ref_ptr<OculusViewConfig> m_oculusViewConfig;

};

#endif /* _OSG_OCULUSEVENTHANDLER_H_ */
