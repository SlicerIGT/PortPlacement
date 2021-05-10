#include <davinci-kinematics/davinci.h>
#include <iostream>

int main(int argc, char* argv[])
{
  std::vector<double> q(6, 0.0);
  q[0] = 0.5;

  DavinciKinematics kinematics(argv[1]);
  std::cout << kinematics.passiveJacobian(Eigen::Matrix4d::Identity(), q, 0.0001) << std::endl;

  return 0;
}
