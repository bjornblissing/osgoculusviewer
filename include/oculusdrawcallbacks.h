/*
 * oculusdrawcallbacks.h
 *
 *  Created on: Dec 12, 2020
 *      Author: Bjorn Blissing
 */

#ifndef _OSG_OCULUSDRAWCALLBACKS_H_
#define _OSG_OCULUSDRAWCALLBACKS_H_

#include "oculusconfig.h"

#include <osg/Camera>

// Forward declaration
class OculusTextureBuffer;
class OculusDevice;

class OSGOCULUS_EXPORT OculusInitialDrawCallback : public osg::Camera::DrawCallback {
 public:
  virtual void operator()(osg::RenderInfo& renderInfo) const override;
};

class OSGOCULUS_EXPORT OculusPreDrawCallback : public osg::Camera::DrawCallback {
 public:
  OculusPreDrawCallback(osg::Camera* camera, OculusTextureBuffer* textureBuffer) :
      m_camera(camera),
      m_textureBuffer(textureBuffer) {}

  void operator()(osg::RenderInfo& renderInfo) const override;

 private:
  osg::observer_ptr<osg::Camera> m_camera;
  osg::observer_ptr<OculusTextureBuffer> m_textureBuffer;
};

class OSGOCULUS_EXPORT OculusPostDrawCallback : public osg::Camera::DrawCallback {
 public:
  OculusPostDrawCallback(osg::Camera* camera,
                         OculusTextureBuffer* textureBuffer,
                         const OculusDevice* device,
                         bool blit = false);

  void operator()(osg::RenderInfo& renderInfo) const override;

 private:
  osg::observer_ptr<osg::Camera> m_camera;
  osg::observer_ptr<OculusTextureBuffer> m_textureBuffer;
  const OculusDevice* m_device;
  bool m_blit;
};

#endif /* _OSG_OCULUSDRAWCALLBACKS_H_ */
