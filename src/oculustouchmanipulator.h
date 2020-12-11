/*
 * oculustouchmanipulator.h
 *
 *  Created on: Oct 30, 2017
 *      Author: Björn Blissing
 */

#ifndef _OSG_OCULUSTOUCHMANIPULATOR_H_
#define _OSG_OCULUSTOUCHMANIPULATOR_H_

#include "oculusdevice.h"

#include <osgGA/CameraManipulator>

class OculusTouchManipulator : public osgGA::CameraManipulator {
 public:
  explicit OculusTouchManipulator(OculusDevice* device) :
      m_device(device),
      m_distance(1.0),
      m_minimumDistance(0.25){};

  void setNode(osg::Node*);
  const osg::Node* getNode() const;
  osg::Node* getNode();

  void home(double /*currentTime*/);
  void setHomePosition(const osg::Vec3d& eye,
                       const osg::Vec3d& center,
                       const osg::Vec3d& up,
                       bool autoComputeHomePosition = false);
  void setTransformation(const osg::Vec3d& eye, const osg::Vec3d& center, const osg::Vec3d& up);

  void setByMatrix(const osg::Matrixd& matrix);
  void setByInverseMatrix(const osg::Matrixd& matrix);
  osg::Matrixd getMatrix() const;
  osg::Matrixd getInverseMatrix() const;

  bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter&);

 private:
  void handleTouchInput();

  osg::observer_ptr<OculusDevice> m_device;

  osg::Vec3d m_center;
  osg::Quat m_rotation;
  double m_distance;
  double m_minimumDistance;

  osg::ref_ptr<osg::Node> _node;
};

#endif /* _OSG_OCULUSTOUCHMANIPULATOR_H_ */
