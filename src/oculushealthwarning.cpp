/*
 * oculushealthwarning.cpp
 *
 *  Created on: Feb 10, 2015
 *      Author: Bjorn Blissing
 */
#include "oculushealthwarning.h"

#include <sstream>

#include <osgDB/ReadFile>
#include <osg/StateAttribute>

typedef unsigned char uint8_t; // Needed to read type uint8_t


// Location of image in byte stream format TGA
#include "healthAndSafety.tga.h"

#include "oculusdevice.h"
#include "oculusdevicesdk.h"


/* Public functions */
void OculusHealthAndSafetyWarning::updatePosition(osg::Matrix cameraMatrix, osg::Vec3 position, osg::Quat orientation) {
	if (m_transform.valid()) {
		osg::Matrix matrix;
		matrix.setTrans(osg::Vec3(0.0, 0.0, -m_distance));
		matrix.postMultRotate(orientation.inverse());
		matrix.postMultTranslate(-position);
		m_transform->setMatrix(matrix*cameraMatrix);
	}
}

osg::ref_ptr<osg::Group> OculusHealthAndSafetyWarning::getGraph() {
	if (!m_transform) {
		m_transform = new osg::MatrixTransform;
		m_transform->setDataVariance(osg::Object::DYNAMIC);

		osg::Image* image = buildImageFromByteStream();
		if (image && image->valid()) {
			double aspectRatio = image->s() / image->t();
			osg::Geode* quadGeode = new osg::Geode;
			osg::Geometry* quadGeometry = osg::createTexturedQuadGeometry(
				osg::Vec3(-(aspectRatio*m_scale) / 2, -m_scale / 2, 1.0),
				osg::Vec3(aspectRatio*m_scale, 0.0, 0.0),
				osg::Vec3(0.0, m_scale, 0.0),
				1.0,
				1.0);
			quadGeode->addDrawable(quadGeometry);
			osg::Texture2D* texture = new osg::Texture2D;
			texture->setImage(image);
			osg::StateSet* stateSet = quadGeode->getOrCreateStateSet();
			stateSet->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);
			m_transform->addChild(quadGeode);
			stateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
			stateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
			stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
			stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
			stateSet->setRenderBinDetails(1, "DepthSortedBin");
		}
	}
	return m_transform->asGroup();
}

void OculusHealthAndSafetyWarning::dismissWarning() {
	if (m_transform.valid()) {
		// Remove all children
		size_t i = m_transform->getNumChildren();
		m_transform->removeChildren(0, i);
		m_transform = 0;
	}
}


/* Protected functions */
osg::Image* OculusHealthAndSafetyWarning::buildImageFromByteStream() {
	std::string imageString((char*)healthAndSafety_tga, sizeof(healthAndSafety_tga));
	std::stringstream imageStream(imageString);
	osgDB::ReaderWriter *rw = osgDB::Registry::instance()->getReaderWriterForExtension("tga");
	if (!rw) {
		std::cerr << "Error: could not find a suitable reader to load the specified data" << std::endl;
		return 0;
	}
	osgDB::ReaderWriter::ReadResult rr = rw->readImage(imageStream);
	if (!rr.validImage()) {
		std::cerr << "Error: Cannot build a valid image from data" << std::endl;
		return 0;
	}
	return rr.takeImage();
}


/* Callbacks */
bool OculusWarningEventHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& ad)
{
	if (ea.getEventType() == osgGA::GUIEventAdapter::KEYUP) {
		if (m_device->tryDismissHealthAndSafetyDisplay()) {
			m_warning->dismissWarning();
		}
	}
	return osgGA::GUIEventHandler::handle(ea, ad);
}

/* Callbacks */
bool OculusWarningEventHandlerSDK::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& ad)
{
	if (ea.getEventType() == osgGA::GUIEventAdapter::KEYUP) {
 		if (m_device->tryDismissHealthAndSafetyDisplay()) {
			m_warning->dismissWarning();
		}	 
	}
	return osgGA::GUIEventHandler::handle(ea, ad);
}