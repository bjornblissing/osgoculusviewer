/*
 * oculusmirrortexture.h
 *
 *  Created on: Dec 13, 2020
 *      Author: Bjorn Blissing
 *
 */

#ifndef _OSG_OCULUSMIRRORTEXTURE_H_
#define _OSG_OCULUSMIRRORTEXTURE_H_

#include "oculusconfig.h"

#include <osg/State>

#include <OVR_CAPI_GL.h>

class OSGOCULUS_EXPORT OculusMirrorTexture : public osg::Referenced {
 public:
  OculusMirrorTexture(ovrSession& session, osg::ref_ptr<osg::State> state, int width, int height);
  void destroy(osg::GraphicsContext* gc = 0);
  GLint width() const {
    return m_width;
  }
  GLint height() const {
    return m_height;
  }
  void blitTexture(osg::GraphicsContext* gc) const;

 private:
  ~OculusMirrorTexture() {}
  const ovrSession m_session;
  ovrMirrorTexture m_mirrorTexture = {nullptr};
  GLint m_width;
  GLint m_height;
  GLuint m_mirrorFBO;
};

#endif /* _OSG_OCULUSMIRRORTEXTURE_H_ */
