#ifndef PORT_PLACEMENT_AUTO_PORT_PLACEMENT_LOGIC_OPTIM_OPTIM_H
#define PORT_PLACEMENT_AUTO_PORT_PLACEMENT_LOGIC_OPTIM_OPTIM_H

#include <davinci-kinematics/davinci.h>
#include <itkeigen/Eigen/Dense>
#include <itkeigen/Eigen/StdVector>

namespace Optim
{
  typedef std::vector<Eigen::Matrix4d,
    Eigen::aligned_allocator<Eigen::Matrix4d> > Matrix4dVec;

  // Orientations of task frames are such that +x is the tip tangent,
  // +z is the last joint's axis
  bool findFeasiblePlan(const DavinciKinematics& kin,
                        const Eigen::Matrix3d& baseOrientationL,
                        const Eigen::Matrix3d& baseOrientationR,
                        const Eigen::Vector3d& baseBoxMin,
                        const Eigen::Vector3d& baseBoxMax,
                        const Matrix4dVec& taskFrames,
                        const Eigen::Vector3d& portCurvePoint1,
                        const Eigen::Vector3d& portCurvePoint2,
                        std::vector<double>* qL,
                        std::vector<double>* qR,
                        Eigen::Vector3d* basePosition);
};

#endif
