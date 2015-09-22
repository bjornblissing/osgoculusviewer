/*
 * oculuseventhandler.cpp
 *
 *  Created on: Jun 24, 2014
 *      Author: Bjorn Blissing
 */

#include "oculuseventhandler.h"
#include "oculusdevice.h"

bool OculusEventHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& ad)
{
	switch (ea.getEventType())
	{
		case (osgGA::GUIEventAdapter::KEYUP):
		{
			switch (ea.getKey())
			{
				case osgGA::GUIEventAdapter::KEY_R:
					m_oculusDevice->resetSensorOrientation();
					return osgGA::GUIEventHandler::handle(ea, ad);
					break;

				case osgGA::GUIEventAdapter::KEY_0:
					m_oculusDevice->setPerfHudMode(0);
					return osgGA::GUIEventHandler::handle(ea, ad);
					break;

				case osgGA::GUIEventAdapter::KEY_1:
					m_oculusDevice->setPerfHudMode(1);
					return osgGA::GUIEventHandler::handle(ea, ad);
					break;

				case osgGA::GUIEventAdapter::KEY_2:
					m_oculusDevice->setPerfHudMode(2);
					return osgGA::GUIEventHandler::handle(ea, ad);
					break;

				case osgGA::GUIEventAdapter::KEY_3:
					m_oculusDevice->setPerfHudMode(3);
					return osgGA::GUIEventHandler::handle(ea, ad);
					break;

				case osgGA::GUIEventAdapter::KEY_4:
					m_oculusDevice->setPerfHudMode(4);
					return osgGA::GUIEventHandler::handle(ea, ad);
					break;

				case osgGA::GUIEventAdapter::KEY_X:
					m_usePositionalTracking = !m_usePositionalTracking;
					m_oculusDevice->setPositionalTrackingState(m_usePositionalTracking);
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
