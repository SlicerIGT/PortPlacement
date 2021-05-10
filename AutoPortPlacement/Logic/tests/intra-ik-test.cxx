#include <davinci-kinematics/davinci.h>
#include <iostream>

int main(int argc, char* argv[])
{
  DavinciKinematics kinematics(argv[1]);

  Eigen::Matrix4d baseFrame = Eigen::Matrix4d::Identity();
  baseFrame(0,0) = baseFrame(1,1) = -1.0;

  double q_p[] = {0.744029, -0.259442, -0.896604, -1.16588, 0.0253959, 0.821945};
  std::vector<double> q_p_vec(&q_p[0], &q_p[6]);
  // q_p[0] = 0.5;
  // Eigen::Vector3d portPosition;
  // portPosition << 0.0, 0.8, 0.82;
  // kinematics.passiveIK(baseFrame, portPosition, &q_p);

  Eigen::Matrix4d portFrame = kinematics.passiveFK(baseFrame, q_p_vec);
  std::cout << portFrame << std::endl;

  Eigen::Vector3d e1; e1(0) = 1.0; e1(1) = 0.0; e1(2) = 0.0;
  Eigen::Vector3d e2; e2(0) = 0.0; e2(1) = 1.0; e2(2) = 0.0;
  Eigen::Vector3d e3; e3(0) = 0.0; e3(1) = 0.0; e3(2) = 1.0;

  Eigen::Matrix4d worldPose = Eigen::Matrix4d::Identity();
  worldPose.block<3,1>(0,0) = -e3;
  worldPose.block<3,1>(0,1) = e2;
  worldPose.block<3,1>(0,2) = e1;
  worldPose(0,3) = 0.0;
  worldPose(1,3) = 0.8;
  worldPose(2,3) = 0.7;

  std::vector<double> q(6);
  kinematics.intraIK(portFrame,
                     worldPose,
                     &q);

  for (std::size_t i = 0; i < q.size(); ++i)
    std::cout << q[i] << std::endl;

  Eigen::Matrix4d fkPose = kinematics.intraFK(portFrame,
                                              q);
  std::cout << fkPose << std::endl;

  if (fabs(fkPose(0,0) - worldPose(0,0)) <= 0.0000001 &&
      fabs(fkPose(1,1) - worldPose(1,1)) <= 0.0000001 &&
      fabs(fkPose(0,3) - worldPose(0,3)) <= 0.0000001)
    return 0;
  else
    return 1;
}
