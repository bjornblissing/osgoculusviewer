/*
 * oculuseventhandlersdk.cpp
 *
 *  Created on: May 20, 2015
 *      Author: Bjorn Blissing
 */

#include "oculuseventhandlersdk.h"
#include "oculusdevicesdk.h"

bool OculusEventHandlerSDK::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& ad)
{
	switch(ea.getEventType())
	{
	case(osgGA::GUIEventAdapter::KEYUP):
		{
			switch(ea.getKey())
			{
			case osgGA::GUIEventAdapter::KEY_R:
				m_oculusDevice->resetSensorOrientation();
				return osgGA::GUIEventHandler::handle(ea, ad);
				break;
			case osgGA::GUIEventAdapter::KEY_M:
				m_oculusDevice->toggleMirrorToWindow();
				return osgGA::GUIEventHandler::handle(ea, ad);
				break;
			case osgGA::GUIEventAdapter::KEY_P:
				m_oculusDevice->toggleLowPersistence();
				return osgGA::GUIEventHandler::handle(ea, ad);
				break;
			case osgGA::GUIEventAdapter::KEY_D:
				m_oculusDevice->toggleDynamicPrediction();
				return osgGA::GUIEventHandler::handle(ea, ad);
				break;
			default:
				return osgGA::GUIEventHandler::handle(ea, ad);
			} 
		}
	default:
		return osgGA::GUIEventHandler::handle(ea, ad);
	}
}
