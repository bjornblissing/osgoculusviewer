/*
 * oculustouchmanipulator.cpp
 *
 *  Created on: Oct 30, 2017
 *      Author: Björn Blissing
 */

#include <oculusdevice.h>
#include <oculustouchmanipulator.h>

void OculusTouchManipulator::setNode(osg::Node* node) {
  _node = node;

  // compute home position
  if (getAutoComputeHomePosition())
    computeHomePosition(NULL, true);
}

const osg::Node* OculusTouchManipulator::getNode() const {
  return _node.get();
}

osg::Node* OculusTouchManipulator::getNode() {
  return _node.get();
}

void OculusTouchManipulator::home(double /*currentTime*/) {
  setTransformation(_homeEye, _homeCenter, _homeUp);
}

void OculusTouchManipulator::setHomePosition(const osg::Vec3d& eye,
                                             const osg::Vec3d& center,
                                             const osg::Vec3d& up,
                                             bool autoComputeHomePosition) {
  setAutoComputeHomePosition(autoComputeHomePosition);
  _homeEye = eye;
  _homeCenter = center;
  _homeUp = up;
  setTransformation(eye, center, up);
}

void OculusTouchManipulator::setTransformation(const osg::Vec3d& eye,
                                               const osg::Vec3d& center,
                                               const osg::Vec3d& up) {
  osg::Vec3d lv(center - eye);

  osg::Vec3d f(lv);
  f.normalize();
  osg::Vec3d s(f ^ up);
  s.normalize();
  osg::Vec3d u(s ^ f);
  u.normalize();

  // clang-format off
	osg::Matrixd rotation_matrix(
	  s[0], u[0], -f[0], 0.0f,
		s[1], u[1], -f[1], 0.0f,
		s[2], u[2], -f[2], 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
  // clang-format on

  m_center = center;
  m_distance = lv.length();
  m_rotation = rotation_matrix.getRotate().inverse();
}

void OculusTouchManipulator::setByMatrix(const osg::Matrixd& matrix) {
  m_center = osg::Vec3d(0., 0., -m_distance) * matrix;
  m_rotation = matrix.getRotate();
}

void OculusTouchManipulator::setByInverseMatrix(const osg::Matrixd& matrix) {
  setByMatrix(osg::Matrixd::inverse(matrix));
}

osg::Matrixd OculusTouchManipulator::getMatrix() const {
  return osg::Matrixd::translate(0.0, 0.0, m_distance) * osg::Matrixd::rotate(m_rotation) *
         osg::Matrixd::translate(m_center);
}

osg::Matrixd OculusTouchManipulator::getInverseMatrix() const {
  return osg::Matrixd::translate(-m_center) * osg::Matrixd::rotate(m_rotation.inverse()) *
         osg::Matrixd::translate(0.0, 0.0, -m_distance);
}

bool OculusTouchManipulator::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter&) {
  if (ea.getEventType() != osgGA::GUIEventAdapter::FRAME)
    return false;
  if (m_device->touchControllerAvailable())
    handleTouchInput();
  return true;
}

void OculusTouchManipulator::handleTouchInput() {
  const ovrInputState input = m_device->touchControllerState();

  // Thumbsticks
  osg::Vec2 thumbstickLeft = osg::Vec2(input.Thumbstick[0].x, input.Thumbstick[0].y);

  // Do not respond to small values
  if (thumbstickLeft.length() > 0.1) {
    const float rotationScale = 0.01;
    osg::Quat rotateYaw(-rotationScale * thumbstickLeft.x() / osg::PI_2,
                        m_rotation * osg::Vec3d(0.0, 1.0, 0.0));

    osg::Quat rotatePitch;
    osg::Vec3d cameraRight(m_rotation * osg::Vec3d(1.0, 0.0, 0.0));

    rotatePitch.makeRotate(rotationScale * thumbstickLeft.y() / osg::PI_2, cameraRight);

    m_rotation = m_rotation * rotateYaw * rotatePitch;
  }

  osg::Vec2 thumbstickRight = osg::Vec2(input.Thumbstick[1].x, input.Thumbstick[1].y);

  // Do not respond to small values
  if (thumbstickRight.length() > 0.1) {
    const float distanceScale = 0.1;

    double newDistance = m_distance - thumbstickRight.y() * distanceScale;
    if (newDistance > m_minimumDistance) {
      m_distance = newDistance;
    }
  }

  // Press B and Y button to recenter camera
  if ((input.Buttons & ovrButton_B) && (input.Buttons & ovrButton_Y)) {
    home(0);
  }
}
