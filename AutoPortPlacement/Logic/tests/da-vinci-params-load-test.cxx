#include <davinci-kinematics/davinci.h>
#include <cmath>

int main(int argc, char* argv[])
{
  DavinciKinematics kinematics(argv[1]);

  DavinciParameters p = kinematics.getParams();

  if (p.wristLength == 0.008 &&
      p.gripperLength == 0.01 &&
      p.el1 == 0.126 &&
      p.el2 == 0.04 &&
      p.el3 == 0.445 &&
      p.el4 == 0.043 &&
      p.el5 == 0.52 &&
      p.er1 == 0.005 &&
      p.er2 == 0.05)
    {
    return 0;
    }
  else
    return 1;
}
