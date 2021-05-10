// Logic includes
#include <davinci-kinematics/davinci.h>

#include <iostream>

int main(int argc, char* argv[])
{
  DavinciKinematics kinematics(argv[1]);

  std::vector<double> q(6,0.0);
  q[0] = 0.1;
  q[1] = 0.1;
  q[2] = -0.2;
  q[3] = 0.15;
  q[4] = -0.1;
  Eigen::Matrix4d rcm = kinematics.passiveFK(Eigen::Matrix4d::Identity(),
                                             q);

  std::cout << rcm << std::endl;

  std::vector<double> q2(6,0.0);
  kinematics.passiveIK(Eigen::Matrix4d::Identity(),
                       rcm.topRightCorner<3,1>(),
                       &q2);

  Eigen::Matrix4d rcm2 = kinematics.passiveFK(Eigen::Matrix4d::Identity(),
                                              q2);

  std::cout << rcm2 << std::endl;

  double error = (rcm.topRightCorner<3,1>() - rcm2.topRightCorner<3,1>()).norm();
  std::cout << error << std::endl;

  if (error <= 0.001)
    return 0;
  else
    return 1;
}
