/*
 * oculusdrawcallbacks.h
 *
 *  Created on: Dec 12, 2020
 *      Author: Bjorn Blissing
 */

#ifndef _OSG_OCULUSDRAWCALLBACKS_H_
#define _OSG_OCULUSDRAWCALLBACKS_H_

#include <osg/Camera>

// Forward declaration
class OculusTextureBuffer;
class OculusDevice;

class OculusInitialDrawCallback : public osg::Camera::DrawCallback {
 public:
  virtual void operator()(osg::RenderInfo& renderInfo) const override;
};

class OculusPreDrawCallback : public osg::Camera::DrawCallback {
 public:
  OculusPreDrawCallback(osg::Camera* camera, OculusTextureBuffer* textureBuffer) :
      m_camera(camera),
      m_textureBuffer(textureBuffer) {}

  void operator()(osg::RenderInfo& renderInfo) const override;

 private:
  osg::observer_ptr<osg::Camera> m_camera;
  osg::observer_ptr<OculusTextureBuffer> m_textureBuffer;
};

class OculusPostDrawCallback : public osg::Camera::DrawCallback {
 public:
  OculusPostDrawCallback(osg::Camera* camera, OculusTextureBuffer* textureBuffer, bool blit = false, OculusDevice* device = nullptr);

  void operator()(osg::RenderInfo& renderInfo) const override;

 private:
  osg::observer_ptr<osg::Camera> m_camera;
  osg::observer_ptr<OculusTextureBuffer> m_textureBuffer;
  bool m_blit;
  osg::observer_ptr<OculusDevice> m_device;
};

#endif /* _OSG_OCULUSDRAWCALLBACKS_H_ */
