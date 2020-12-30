/*
 * oculustouchmanipulator.h
 *
 *  Created on: Oct 30, 2017
 *      Author: Björn Blissing
 */

#ifndef _OSG_OCULUSTOUCHMANIPULATOR_H_
#define _OSG_OCULUSTOUCHMANIPULATOR_H_

#include "oculusconfig.h"

#include <osgGA/CameraManipulator>

// Forward declaration
class OculusDevice;

class OSGOCULUS_EXPORT OculusTouchManipulator : public osgGA::CameraManipulator {
 public:
  explicit OculusTouchManipulator(OculusDevice* device) : m_device(device) {}

  void setNode(osg::Node*) override;
  const osg::Node* getNode() const override;
  osg::Node* getNode() override;

  void home(double /*currentTime*/) override;
  void setHomePosition(const osg::Vec3d& eye,
                       const osg::Vec3d& center,
                       const osg::Vec3d& up,
                       bool autoComputeHomePosition = false) override;
  void setTransformation(const osg::Vec3d& eye, const osg::Vec3d& center, const osg::Vec3d& up);

  void setByMatrix(const osg::Matrixd& matrix) override;
  void setByInverseMatrix(const osg::Matrixd& matrix) override;
  osg::Matrixd getMatrix() const override;
  osg::Matrixd getInverseMatrix() const override;

  bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter&) override;

 private:
  void handleTouchInput();

  osg::observer_ptr<OculusDevice> m_device;

  osg::Vec3d m_center;
  osg::Quat m_rotation;
  double m_distance = {1.0};
  double m_minimumDistance = {0.25};

  osg::ref_ptr<osg::Node> _node;
};

#endif /* _OSG_OCULUSTOUCHMANIPULATOR_H_ */
