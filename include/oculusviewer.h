/*
 * oculusviewer.h
 *
 *  Created on: Jun 30, 2013
 *      Author: Jan Ciger & Björn Blissing
 */

#ifndef _OSG_OCULUSVIEWER_H_
#define _OSG_OCULUSVIEWER_H_

#include "oculusconfig.h"

#include <osg/Group>

// Forward declaration
namespace osgViewer {
class Viewer;
}

class OculusDevice;
class OculusRealizeOperation;

class OSGOCULUS_EXPORT OculusViewer : public osg::Group {
 public:
  OculusViewer(osgViewer::Viewer* viewer,
               OculusDevice* dev,
               OculusRealizeOperation* realizeOperation) :
      osg::Group(),
      m_viewer(viewer),
      m_device(dev),
      m_realizeOperation(realizeOperation) {}
  void traverse(osg::NodeVisitor& nv) override;

 private:
  void configure();

  bool m_configured = {false};

  osg::observer_ptr<osgViewer::Viewer> m_viewer;
  osg::observer_ptr<OculusDevice> m_device;
  osg::observer_ptr<OculusRealizeOperation> m_realizeOperation;
};

#endif /* _OSG_OCULUSVIEWER_H_ */
