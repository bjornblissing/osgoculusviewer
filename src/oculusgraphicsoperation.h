/*
 * oculusgraphicsoperation.h
 *
 *  Created on: Dec 12, 2020
 *      Author: Bjorn Blissing
 */

#ifndef _OSG_OCULUSGRAPHICSOPERATION_H_
#define _OSG_OCULUSGRAPHICSOPERATION_H_

#include <osg/GraphicsThread>

// Forward declaration
class OculusDevice;

class OculusRealizeOperation : public osg::GraphicsOperation {
 public:
  explicit OculusRealizeOperation(OculusDevice* device) :
      osg::GraphicsOperation("OculusRealizeOperation", false),
      m_device(device) {}
  void operator()(osg::GraphicsContext* gc) override;
  bool realized() const {
    return m_realized;
  }

 private:
  OpenThreads::Mutex _mutex;
  osg::observer_ptr<OculusDevice> m_device;
  bool m_realized = {false};
};

class OculusCleanUpOperation : public osg::GraphicsOperation {
 public:
  explicit OculusCleanUpOperation(OculusDevice* device) :
      osg::GraphicsOperation("OculusCleanupOperation", false),
      m_device(device) {}
  void operator()(osg::GraphicsContext* gc) override;

 private:
  osg::observer_ptr<OculusDevice> m_device;
};

#endif /* _OSG_OCULUSGRAPHICSOPERATION_H_ */