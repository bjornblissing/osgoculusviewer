/*
 * oculusswapcallback.h
 *
 *  Created on: Dec 13, 2020
 *      Author: Bjorn Blissing
 */

#ifndef _OSG_OCULUSSWAPCALLBACK_H_
#define _OSG_OCULUSSWAPCALLBACK_H_

#include <osg/GraphicsContext>

// Forward declaration
class OculusDevice;

class OculusSwapCallback : public osg::GraphicsContext::SwapCallback {
 public:
  explicit OculusSwapCallback(OculusDevice* device) : m_device(device) {}
  void swapBuffersImplementation(osg::GraphicsContext* gc) override;
  long long frameIndex() const {
    return m_frameIndex;
  }

 private:
  osg::observer_ptr<OculusDevice> m_device;
  long long m_frameIndex = {0};
};

#endif /* _OSG_OCULUSSWAPCALLBACK_H_ */