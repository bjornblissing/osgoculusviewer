/*
 * oculusupdateslavecallback.h
 *
 *  Created on: Jul 07, 2015
 *      Author: Björn Blissing
 */

#ifndef _OSG_OCULUSUPDATESLAVECALLBACK_H_
#define _OSG_OCULUSUPDATESLAVECALLBACK_H_

#include <osgViewer/View>

// Forward declaration
class OculusDevice;
class OculusSwapCallback;

struct OculusUpdateSlaveCallback : public osg::View::Slave::UpdateSlaveCallback {
  enum CameraType { LEFT_CAMERA, RIGHT_CAMERA };

  OculusUpdateSlaveCallback(CameraType cameraType,
                            OculusDevice* device,
                            OculusSwapCallback* swapCallback) :
      m_cameraType(cameraType),
      m_device(device),
      m_swapCallback(swapCallback) {}

  void updateSlave(osg::View& view, osg::View::Slave& slave) override;

 private:
  CameraType m_cameraType;
  osg::observer_ptr<OculusDevice> m_device;
  osg::observer_ptr<OculusSwapCallback> m_swapCallback;
};

#endif  // _OSG_OCULUSUPDATESLAVECALLBACK_H_
