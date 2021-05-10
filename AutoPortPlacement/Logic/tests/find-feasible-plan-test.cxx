#include <optim/optim.h>

int main(int argc, char* argv[])
{
  DavinciKinematics kinematics(argv[1]);

  // Rotate about +z by 180 degrees
  Eigen::Matrix3d baseOrientationL = Eigen::Matrix3d::Identity();
  baseOrientationL(0,0) = -1.0;
  baseOrientationL(1,1) = -1.0;

  Eigen::Matrix3d baseOrientationR = Eigen::Matrix3d::Identity();

  Eigen::Vector3d baseBoxMin = Eigen::Vector3d::Constant(-0.01);
  Eigen::Vector3d baseBoxMax = Eigen::Vector3d::Constant(0.01);

  // Let's add two task frames for now
  Optim::Matrix4dVec taskFrames;
  Eigen::Matrix4d frame = Eigen::Matrix4d::Identity();

  Eigen::Vector3d e1; e1(0) = 1.0; e1(1) = 0.0; e1(2) = 0.0;
  Eigen::Vector3d e2; e2(0) = 0.0; e2(1) = 1.0; e2(2) = 0.0;
  Eigen::Vector3d e3; e3(0) = 0.0; e3(1) = 0.0; e3(2) = 1.0;

  // tip tangent along -z
  frame.block<3,1>(0,0) = -e3;
  frame.block<3,1>(0,1) = e2;
  frame.block<3,1>(0,2) = e1;

  // centered at (0,0.8.0.7)
  frame(0,3) = 0.0;
  frame(1,3) = 0.8;
  frame(2,3) = 0.7;

  taskFrames.push_back(frame);

  frame(0,3) = 0.0;
  frame(1,3) = 0.81;
  frame(2,3) = 0.71;

  taskFrames.push_back(frame);

  // Set line segment of feasible port locations
  // Want this to be a few centimeters up from the task frame
  Eigen::Vector3d portCurvePoint1 = frame.topRightCorner<3,1>();
  portCurvePoint1(2) += 0.12;
  portCurvePoint1(0) -= 0.2;
  Eigen::Vector3d portCurvePoint2 = frame.topRightCorner<3,1>();
  portCurvePoint2(2) += 0.12;
  portCurvePoint2(0) += 0.2;

  // Give 'er a go!
  std::vector<double> qL(6);
  std::vector<double> qR(6);
  Eigen::Vector3d basePosition;
  Optim::findFeasiblePlan(kinematics, baseOrientationL, baseOrientationR,
                          baseBoxMin, baseBoxMax, taskFrames,
                          portCurvePoint1, portCurvePoint2,
                          &qL, &qR, &basePosition);

  return 0;
}
