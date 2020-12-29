/*
 * oculusdevice.h
 *
 *  Created on: Jul 03, 2013
 *      Author: Bjorn Blissing
 */

#ifndef _OSG_OCULUSDEVICE_H_
#define _OSG_OCULUSDEVICE_H_

#include <osg/GraphicsContext>
#include <osg/State>
#include <osg/Transform>

#include <OVR_CAPI.h>

// Forward declarations
class OculusTextureBuffer;
class OculusMirrorTexture;

class OculusDevice : public osg::Referenced {
 public:
  typedef enum Eye_ { LEFT = 0, RIGHT = 1, COUNT = 2 } Eye;

  typedef enum TrackingOrigin_ { EYE_LEVEL = 0, FLOOR_LEVEL = 1 } TrackingOrigin;

  OculusDevice(float nearClip,
               float farClip,
               const float pixelsPerDisplayPixel,
               const float worldUnitsPerMetre,
               const int samples,
               TrackingOrigin origin,
               const int mirrorTextureWidth,
               bool blitOnPostDraw = false);

  void createRenderBuffers(osg::State* state);
  void init();

  void destroyTextures(osg::GraphicsContext* gc);
  bool hmdPresent() const;

  unsigned int screenResolutionWidth() const {
    return m_hmdDesc.Resolution.w;
  }
  unsigned int screenResolutionHeight() const {
    return m_hmdDesc.Resolution.h;
  }

  const ovrHmdDesc& hmdDescription() const {
    return m_hmdDesc;
  }

  float nearClip() const {
    return m_nearClip;
  }

  float farClip() const {
    return m_farClip;
  }

  void setNearClip(float nearClip) {
    m_nearClip = nearClip;
  }

  void setFarClip(float farClip) {
    m_farClip = farClip;
  }

  bool blitOnPostDraw() const {
    return m_blitOnPostDraw;
  }

  void resetSensorOrientation() const {
    ovr_RecenterTrackingOrigin(m_session);
  }

  void updatePose(long long frameIndex);

  osg::Vec3 position(Eye eye) const;
  osg::Quat orientation(Eye eye) const;

  osg::Matrixf viewMatrix(Eye eye) const;
  osg::Matrixf projectionMatrix(Eye eye) const;
  void updateTimewarpProjection(Eye eye);

  osg::Camera* createRTTCamera(OculusDevice::Eye eye,
                               osg::Transform::ReferenceFrame referenceFrame,
                               const osg::Vec4& clearColor,
                               osg::GraphicsContext* gc = 0) const;

  bool waitToBeginFrame(long long frameIndex = 0);
  bool beginFrame(long long frameIndex = 0);

  bool submitFrame(long long frameIndex = 0);
  void blitMirrorTexture(osg::GraphicsContext* gc) const;

  void setPerfHudMode(int mode);

  bool touchControllerAvailable() const;
  ovrInputState touchControllerState() const {
    return m_controllerState;
  }

  ovrPoseStatef headPose() const {
    return m_headPose;
  }
  ovrPoseStatef handPoseLeft() const {
    return m_handPoses[ovrHand_Left];
  }
  ovrPoseStatef handPoseRigth() const {
    return m_handPoses[ovrHand_Right];
  }

  osg::GraphicsContext::Traits* graphicsContextTraits() const;

 private:
  ~OculusDevice();  // Since we inherit from osg::Referenced we must make destructor private
  OculusDevice(const OculusDevice&) = delete;
  OculusDevice& operator=(const OculusDevice&) = delete;
  OculusDevice(OculusDevice&&) = delete;
  OculusDevice& operator=(OculusDevice&&) = delete;

  void printHMDDebugInfo();

  void getEyeRenderDesc();

  void setTrackingOrigin();

  void setupLayers();

  void trySetProcessAsHighPriority() const;

  ovrSession m_session = {nullptr};
  ovrHmdDesc m_hmdDesc{};
  ovrInputState m_controllerState;
  ovrPoseStatef m_handPoses[2];
  ovrPoseStatef m_headPose;

  const float m_pixelsPerDisplayPixel;
  const float m_worldUnitsPerMetre;

  osg::ref_ptr<OculusTextureBuffer> m_textureBuffer[2] = {nullptr, nullptr};
  osg::ref_ptr<OculusMirrorTexture> m_mirrorTexture = {nullptr};

  unsigned int m_mirrorTextureWidth;

  ovrEyeRenderDesc m_eyeRenderDesc[2];
  ovrVector2f m_UVScaleOffset[2][2];
  double m_sensorSampleTime;
  ovrPosef m_eyeRenderPose[2];
  ovrLayerEyeFovDepth m_layerEyeFovDepth;
  ovrTimewarpProjectionDesc m_posTimewarpProjectionDesc = {};

  osg::Vec3 m_position{};
  osg::Quat m_orientation{};

  float m_nearClip;
  float m_farClip;
  int m_samples = {0};
  bool m_begunFrame = {false};
  bool m_blitOnPostDraw = {false};
  TrackingOrigin m_origin;
};

#endif /* _OSG_OCULUSDEVICE_H_ */
