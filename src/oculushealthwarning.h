/*
 * oculushealthwarning.h
 *
 *  Created on: Feb 10, 2015
 *      Author: Bjorn Blissing
 */
#ifndef _OSG_OCULUSWARNING_H_
#define _OSG_OCULUSWARNING_H_

#include <osg/MatrixTransform>
#include <osg/Image>
#include <osgViewer/ViewerEventHandlers>

// Forward declarations
class OculusDevice;
class OculusDeviceSDK;


class OculusHealthAndSafetyWarning : public osg::Referenced {
public:
	OculusHealthAndSafetyWarning() : m_scale(3.0), m_distance(4.0), m_transform(0) {};
	void updatePosition(osg::Matrix cameraMatrix, osg::Vec3 position, osg::Quat orientation);
	osg::ref_ptr<osg::Group> getGraph();
	void dismissWarning();
protected:
	~OculusHealthAndSafetyWarning() {};
	
	osg::Image* buildImageFromByteStream();
	const double m_scale;
	const double m_distance;
	osg::ref_ptr<osg::MatrixTransform> m_transform;
	
};


class OculusWarningEventHandler : public osgGA::GUIEventHandler
{
public:
	OculusWarningEventHandler(osg::observer_ptr<OculusDevice> device, osg::observer_ptr<OculusHealthAndSafetyWarning> warning) : m_device(device), m_warning(warning) {}
	virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&);
protected:
	osg::observer_ptr<OculusDevice> m_device;
	osg::observer_ptr<OculusHealthAndSafetyWarning> m_warning;
};

class OculusWarningEventHandlerSDK : public osgGA::GUIEventHandler
{
public:
	OculusWarningEventHandlerSDK(osg::observer_ptr<OculusDeviceSDK> device, osg::observer_ptr<OculusHealthAndSafetyWarning> warning) : m_device(device), m_warning(warning) {}
	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter&);
protected:
	osg::observer_ptr<OculusDeviceSDK> m_device;
	osg::observer_ptr<OculusHealthAndSafetyWarning> m_warning;
};

#endif /* _OSG_OCULUSWARNING_H_ */
