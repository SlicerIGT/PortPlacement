#include <davinci-kinematics/davinci.h>
#include <cmath>
#include <iostream>

#include <itkDOMNodeXMLReader.h>

#include <stdexcept>

#include <vtkMath.h>

namespace
{
  const double passiveLowerBounds[] = {0.0,
                                       -(4.0/3.0)*vtkMath::Pi(),
                                       -(4.0/3.0)*vtkMath::Pi(),
                                       -(4.0/3.0)*vtkMath::Pi(),
                                       -(4.0/3.0)*vtkMath::Pi(),
                                       -(4.0/3.0)*vtkMath::Pi()};

  const double passiveUpperBounds[] = {1.0,
                                       (4.0/3.0)*vtkMath::Pi(),
                                       (4.0/3.0)*vtkMath::Pi(),
                                       (4.0/3.0)*vtkMath::Pi(),
                                       (4.0/3.0)*vtkMath::Pi(),
                                       (4.0/3.0)*vtkMath::Pi()};

  const double activeLowerBounds[] = {-(2.0/3.0)*vtkMath::Pi(),
                                      -(2.0/3.0)*vtkMath::Pi(),
                                      -2.0*vtkMath::Pi(),
                                      0.0,
                                      -vtkMath::Pi(),
                                      -vtkMath::Pi()};

  // For now I'm doing symmetric bounds, and making up a bound for the
  // last joint. Accoring to Azimian's implementation, q[1]'s limit is
  // actually pi/4, which is strange and should be checked up on.
  const double  activeUpperBounds[] = {(2.0/3.0)*vtkMath::Pi(),
                                       (2.0/3.0)*vtkMath::Pi(),
                                       2.0*vtkMath::Pi(),
                                       0.2,
                                       vtkMath::Pi(),
                                       vtkMath::Pi()};

  // Generate 4x4 transform matrix corresponding to a rotation of
  // theta about the x-axis.
  Eigen::Matrix4d xRotation(double theta)
  {
    Eigen::Matrix4d R = Eigen::Matrix4d::Identity();
    double c = cos(theta);
    double s = sin(theta);

    R(1,1) = c;
    R(2,1) = s;

    R(1,2) = -s;
    R(2,2) = c;

    return R;
  }

  // Generate 4x4 transform matrix corresponding to a rotation of
  // theta about the y-axis.
  Eigen::Matrix4d yRotation(double theta)
  {
    Eigen::Matrix4d R = Eigen::Matrix4d::Identity();
    double c = cos(theta);
    double s = sin(theta);

    R(0,0) = c;
    R(2,0) = -s;

    R(0,2) = s;
    R(2,2) = c;

    return R;
  }

  // Generate 4x4 transform matrix corresponding to a rotation of
  // theta about the z-axis.
  Eigen::Matrix4d zRotation(double theta)
  {
    Eigen::Matrix4d R = Eigen::Matrix4d::Identity();
    double c = cos(theta);
    double s = sin(theta);
    R(0,0) = c;
    R(1,0) = s;

    R(0,1) = -s;
    R(1,1) = c;

    return R;
  }

  // Given a transformation matrix on SE(3), generate the inverse
  // transformation.
  Eigen::Matrix4d getInverseTransform(const Eigen::Matrix4d& T)
  {
    Eigen::Matrix3d R = T.topLeftCorner<3,3>();
    Eigen::Vector3d p = T.topRightCorner<3,1>();
    Eigen::Matrix4d T_inv;
    T_inv.topLeftCorner<3,3>() = R.transpose();
    T_inv.topRightCorner<3,1>() = -R.transpose()*p;

    T_inv(3,0) = T_inv(3,1) = T_inv(3,2) = 0.0;
    T_inv(3,3) = 1.0;

    return T_inv;
  }
}

DavinciKinematics::DavinciKinematics(const std::string& inputFilename)
{

  itk::DOMNodeXMLReader::Pointer reader = itk::DOMNodeXMLReader::New();
  reader->SetFileName( inputFilename.c_str() );

  try
    {
    // read a DOM object from an XML file
    reader->Update();
    }
  catch ( const itk::ExceptionObject& eo )
    {
    eo.Print( std::cerr );
    }
  catch ( ... )
    {
    std::cerr << "Unknown exception caught!" << std::endl;
    }


  itk::DOMNode::Pointer dom = reader->GetOutput();

  itk::DOMNode::Pointer davinciParametersIntracorporeal = dom->Find("intracorporeal");

  if ( (itk::DOMNode*)davinciParametersIntracorporeal != 0 )
    {
    itk::DOMNode::Pointer wristLengthDOM = davinciParametersIntracorporeal->GetChild("wrist_length");
    std::string wristLengthString = wristLengthDOM->GetTextChild()->GetText();
    double wristLength = atof(wristLengthString.c_str());
    std::cout << "WristLength=" << wristLength << std::endl;
    params_.wristLength = wristLength;

    itk::DOMNode::Pointer gripperLengthDOM = davinciParametersIntracorporeal->GetChild("gripper_length");
    std::string gripperLengthString = gripperLengthDOM->GetTextChild()->GetText();
    double gripperLength = atof(gripperLengthString.c_str());
    std::cout << "GripperLength=" << gripperLength << std::endl;
    params_.gripperLength = gripperLength;
    }
  else
    {
    std::cerr <<"Could not find intracorporeal parameters" << std::endl;
    }


  itk::DOMNode::Pointer davinciParametersExtracorporeal = dom->Find("extracorporeal");

  if ( (itk::DOMNode*)davinciParametersIntracorporeal != 0 )
    {

    itk::DOMNode::Pointer l1DOM = davinciParametersExtracorporeal->GetChild("l1");
    double l1=0.0;

    if ( (itk::DOMNode*)l1DOM != 0 )
      {
      std::string l1String = l1DOM->GetTextChild()->GetText();
      l1= atof(l1String.c_str());
      std::cout << "l1=" << l1 << std::endl;
      }
    else
      {
      std::cerr << "l1 not specified" << std::endl;
      }
    params_.el1 = l1;

    itk::DOMNode::Pointer l2DOM = davinciParametersExtracorporeal->GetChild("l2");
    double l2=0.0;
    if ( (itk::DOMNode*)l2DOM != 0 )
      {
      std::string l2String = l2DOM->GetTextChild()->GetText();
      l2= atof(l2String.c_str());
      std::cout << "l2=" << l2 << std::endl;
      }
    else
      {
      std::cerr << "l2 not specified" << std::endl;
      }
    params_.el2 = l2;

    itk::DOMNode::Pointer l3DOM = davinciParametersExtracorporeal->GetChild("l3");
    double l3=0.0;

    if ( (itk::DOMNode*)l3DOM != 0 )
      {
      std::string l3String = l3DOM->GetTextChild()->GetText();
      l3= atof(l3String.c_str());
      std::cout << "l3=" << l3 << std::endl;
      }
    else
      {
      std::cerr << "l3 is not specified" << std::endl;
      }
    params_.el3 = l3;


    itk::DOMNode::Pointer l4DOM = davinciParametersExtracorporeal->GetChild("l4");
    double l4=0.0;

    if ( (itk::DOMNode*)l4DOM != 0 )
      {
      std::string l4String = l4DOM->GetTextChild()->GetText();
      l4= atof(l4String.c_str());
      std::cout << "l4=" << l4 << std::endl;
      }
    else
      {
      std::cerr << "l4 is not specified" << std::endl;
      }
    params_.el4 = l4;

    itk::DOMNode::Pointer l5DOM = davinciParametersExtracorporeal->GetChild("l5");
    double l5=0.0;

    if ( (itk::DOMNode*)l5DOM != 0 )
      {
      std::string l5String = l5DOM->GetTextChild()->GetText();
      l5= atof(l5String.c_str());
      std::cout << "l5=" << l5 << std::endl;
      }
    else
      {
      std::cerr << "l5 is not specified" << std::endl;
      }
    params_.el5 = l5;

    double r1=0.0;
    itk::DOMNode::Pointer r1DOM = davinciParametersExtracorporeal->GetChild("r1");

    if ( (itk::DOMNode*)r1DOM != 0 )
      {
      std::string r1String = r1DOM->GetTextChild()->GetText();
      r1= atof(r1String.c_str());
      std::cout << "r1=" << r1 << std::endl;
      }
    else
      {
      std::cerr << "radius1 not specified" << std::endl;
      }
    params_.er1 = r1;


    double r2=0.0;
    itk::DOMNode::Pointer r2DOM = davinciParametersExtracorporeal->GetChild("r2");

    if ( (itk::DOMNode*)r2DOM != 0 )
      {
      std::string r2String = r2DOM->GetTextChild()->GetText();
      r2= atof(r2String.c_str());
      std::cout << "r2=" << r2 << std::endl;
      }
    else
      {
      std::cerr << "radius2 not specified" << std::endl;
      }
    params_.er2 = r2;

    }
  else
    {
    std::cerr << "Couldn't find extracorporeal parameter in the xml file" << std::endl;
    }
}

// \TODO reduce number of intermediate matrices for optimization
Eigen::Matrix4d DavinciKinematics::intraFK(const Eigen::Matrix4d& portFrame,
                                           const std::vector<double>& q) const
{
  assert(q.size() == 6);

  Eigen::Vector3d e1; e1(0) = 1.0; e1(1) = 0.0; e1(2) = 0.0;
  Eigen::Vector3d e2; e2(0) = 0.0; e2(1) = 1.0; e2(2) = 0.0;
  Eigen::Vector3d e3; e3(0) = 0.0; e3(1) = 0.0; e3(2) = 1.0;

  Eigen::Matrix4d T = portFrame;
  T *= zRotation(q[0]);

  Eigen::Matrix4d T_1_2 = Eigen::Matrix4d::Zero();
  T_1_2.block<3,1>(0,0) = e1;
  T_1_2.block<3,1>(0,1) = -e3;
  T_1_2.block<3,1>(0,2) = e2;
  T_1_2(3,3) = 1.0;
  T_1_2 *= zRotation(q[1]);
  T = T*T_1_2;

  Eigen::Matrix4d T_2_3 = Eigen::Matrix4d::Zero();
  T_2_3.block<3,1>(0,0) = -e2;
  T_2_3.block<3,1>(0,1) = e3;
  T_2_3.block<3,1>(0,2) = -e1;
  T_2_3(3,3) = 1.0;
  T_2_3 *= zRotation(q[2]);
  T = T*T_2_3;

  Eigen::Matrix4d T_3_4 = Eigen::Matrix4d::Zero();
  T_3_4.block<3,1>(0,0) = e1;
  T_3_4.block<3,1>(0,1) = e2;
  T_3_4.block<3,1>(0,2) = e3;
  T_3_4(2,3) = q[3];
  T_3_4(3,3) = 1.0;
  T = T*T_3_4;

  Eigen::Matrix4d T_4_5 = Eigen::Matrix4d::Zero();
  T_4_5.block<3,1>(0,0) = e3;
  T_4_5.block<3,1>(0,1) = e1;
  T_4_5.block<3,1>(0,2) = e2;
  T_4_5(3,3) = 1.0;
  T_4_5 *= zRotation(q[4]);
  T = T*T_4_5;

  Eigen::Matrix4d T_5_6 = Eigen::Matrix4d::Zero();
  T_5_6.block<3,1>(0,0) = e1;
  T_5_6.block<3,1>(0,1) = -e3;
  T_5_6.block<3,1>(0,2) = e2;
  T_5_6(0,3) = params_.wristLength;
  T_5_6(3,3) = 1.0;
  T_5_6 *= zRotation(q[5]);
  T = T*T_5_6;

  // For gripper. Bring it in later when we have IK working to correspond to this
  // Eigen::Matrix4d T_6_e = Eigen::Matrix4d::Zero();
  // T_6_e.block<3,1>(0,0) = e3;
  // T_6_e.block<3,1>(0,1) = e2;
  // T_6_e.block<3,1>(0,2) = e1;
  // T_6_e(0,3) = gripperLength_;
  // T_6_e(3,3) = 1.0;
  // T = T*T_6_e;

  return T;
}

// Direct, obscure implementation of the Davinci's forward
// kinematics. Transcribed from a similarly obscure MATLAB
// implementation.
Eigen::Matrix4d DavinciKinematics::passiveFK(const Eigen::Matrix4d& baseFrame,
                                             const std::vector<double>& q) const
{
  double c2 = cos(q[1]);
  double s2 = sin(q[1]);
  double c23 = cos(q[1] + q[2]);
  double s23 = sin(q[1] + q[2]);
  double c234 = cos(q[1] + q[2] + q[3]);
  double s234 = sin(q[1] + q[2] + q[3]);
  double c5m6 = cos(q[4] - q[5]);
  double s5m6 = sin(q[4] - q[5]);
  double c5p6 = cos(q[4] + q[5]);
  double s5p6 = sin(q[4] + q[5]);
  double c5 = cos(q[4]);
  double s5 = sin(q[4]);
  double c6 = cos(q[5]);
  double s6 = sin(q[5]);

  Eigen::Matrix4d T;
  T <<
    -s234*s5,
    0.5*s234*c5p6 - c234*s6 + 0.5*c5m6*s234,
    0.5*s234*s5p6 + c234*c6 - 0.5*s5m6*s234,
    (52.0/125.0)*c234 + (17.0/40.0)*c23 + 0.419*c2 + 0.5*0.483*s234*s5p6 + 0.483*c234*c6 - 0.051*s234*s5 - 0.5*0.483*s5m6*s234 + 0.189,

    c234*s5,
    -0.5*c234*c5p6 - s234*s6 - 0.5*c5m6*c234,
    s234*c6 - 0.5*c234*s5p6 + 0.5*s5m6*c234,
    (52.0/125.0)*s234 + (17.0/40.0)*s23 + 0.419*s2 - 0.5*0.483*c234*s5p6 + 0.051*c234*s5 + 0.483*s234*c6 + 0.5*0.483*s5m6*c234,

    c5, c6*s5, s5*s6, q[0] + 0.051*c5 + 0.483*s5*s6 + (2.0 / 125.0),

    0, 0, 0, 1;

  return baseFrame*T;
}

// TODO factor in length of gripper tool by just adjusting initial
// pose before doing anything
void DavinciKinematics::intraIK(const Eigen::Matrix4d& portFrame,
                                const Eigen::Matrix4d& worldPose,
                                std::vector<double>* q) const
{
  assert(q->size() == 6);

  // Express input pose in terms of arm's base frame
  Eigen::Matrix4d pose = getInverseTransform(portFrame) * worldPose;

  Eigen::Matrix3d R = pose.topLeftCorner<3,3>();
  Eigen::Vector3d p = pose.topRightCorner<3,1>();
  Eigen::Vector3d positionTrans = -R.transpose()*p;
  double u = positionTrans(0);
  double v = positionTrans(1);
  double w = positionTrans(2);

  (*q)[5] = atan2(v, -u);
  double c5 = cos((*q)[5]);
  double s5 = sin((*q)[5]);
  (*q)[4] = atan2(w, -params_.wristLength - u*c5 + v*s5);
  double c4 = cos((*q)[4]);
  double s4 = sin((*q)[4]);
  (*q)[3] = (-params_.wristLength - u*c5 + v*s5)*c4 + w*s4;

  Eigen::Matrix3d m;
  m(0,0) = c5*s4;  m(0,1) = -s5;  m(0,2) = c4*c5;
  m(1,0) = -s4*s5; m(1,1) = -c5;  m(1,2) = -c4*s5;
  m(2,0) = c4;     m(2,1) = 0.0;  m(2,2) = -s4;
  Eigen::Matrix3d Rbar = R*m;

  (*q)[1] = atan2(Rbar(2,2), sqrt(Rbar(0,2)*Rbar(0,2) + Rbar(1,2)*Rbar(1,2)));
  double c1 = cos((*q)[1]);
  (*q)[2] = atan2(-Rbar(2,1) / c1, Rbar(2,0) / c1);
  (*q)[0] = atan2(-Rbar(1,2) / c1, -Rbar(0,2) / c1);
}

// \TODO allow for specification of q0 evaluation if computed
// beforehand
Eigen::Matrix<double, 3, 6>
DavinciKinematics::passiveJacobian(const Eigen::Matrix4d& baseFrame,
                                   const std::vector<double>& q_in,
                                   double stepSize) const
{
  std::vector<double> q(q_in);

  Eigen::Matrix<double, 3, 6> jacobian;

  // Using forward difference
  Eigen::Vector3d p0 = this->passiveFK(baseFrame, q).topRightCorner<3,1>();

  for (std::size_t i = 0; i < q.size(); ++i)
    {
    double q0 = q[i];
    q[i] += stepSize;
    Eigen::Vector3d p1 = this->passiveFK(baseFrame, q).topRightCorner<3,1>();
    jacobian.col(i) = (p1 - p0) / stepSize;
    q[i] = q0;
    }

  return jacobian;
}

// Iteration step size calculation came from:
// http://math.ucsd.edu.libproxy.lib.unc.edu/~sbuss/ResearchWeb/ikmethods/iksurvey.pdf
//
// TODO maybe try DLS from same paper
bool DavinciKinematics::passiveIK(const Eigen::Matrix4d& baseFrame,
                                  const Eigen::Vector3d& worldPosition,
                                  std::vector<double>* qOut,
                                  double stepSize) const
{
  std::vector<double> q(6, 0.0);
  Eigen::Matrix<double,6,1> q_vec;

  Eigen::Vector3d goalVec =
    worldPosition - this->passiveFK(baseFrame, q).topRightCorner<3,1>();
  double error = goalVec.squaredNorm();
  unsigned iterations = 0;
  unsigned MAX_ITERATIONS = 1000;
  while (error > 0.001*0.001 && iterations < MAX_ITERATIONS) // TODO do tolerance better
    {
    Eigen::Matrix<double, 3, 6> jacobian = this->passiveJacobian(baseFrame,
                                                                 q, stepSize);
    for (unsigned i = 0; i < 6; ++i)
      q_vec(i) = q[i];

    Eigen::Vector3d JJTe = jacobian*jacobian.transpose()*goalVec;
    double step = goalVec.dot(JJTe) / JJTe.squaredNorm();
    q_vec += step*jacobian.transpose()*goalVec;

    for (unsigned i = 0; i < 6; ++i)
      q[i] = q_vec(i);

    goalVec = worldPosition - this->passiveFK(baseFrame, q).topRightCorner<3,1>();
    error = goalVec.squaredNorm();
    }

  qOut->swap(q);

  if (iterations >= MAX_ITERATIONS)
    {
    std::cout << "Warning: passive IK didn't converge!" << std::endl;
    return false;
    }
  else
    return true;
}

void DavinciKinematics::getExtraCylispheres(const Eigen::Matrix4d& portFrame,
                                            const std::vector<double>& q,
                                            std::vector<Collisions::Cylisphere>* cylispheres) const
{
  double c1 = cos(q[0]);
  double c2 = cos(q[1]);
  double s1 = sin(q[0]);
  double s2 = sin(q[1]);

  Eigen::Vector3d p;
  p << -c1*c2, -c2*s1, s2;

  // instrument cylisphere
  Collisions::Cylisphere c_i;
  c_i.p1.setZero();
  c_i.p2 = -params_.el1*p;
  c_i.r = params_.er1;

  // instrument holder cylisphere
  Collisions::Cylisphere c_h;
  Eigen::Vector3d p2;
  p2 << c1*s2, s1*s2, c2;
  c_h.p1 = c_i.p2 - params_.el2*p2;
  c_h.p2 = c_h.p1 - params_.el3*p;
  c_h.r = params_.er2;

  // parallelogram cylisphere
  Collisions::Cylisphere c_p;
  c_p.p1 = c_h.p1 - params_.el4*p;
  c_p.p2 = c_p.p1;
  c_p.p2(2) -= params_.el5;
  c_p.r = params_.er2;

  cylispheres->push_back(c_i);
  cylispheres->push_back(c_h);
  cylispheres->push_back(c_p);

  // transform cylispheres according to port frame
  Eigen::Matrix3d R = portFrame.topLeftCorner<3,3>();
  Eigen::Vector3d T = portFrame.topRightCorner<3,1>();
  for (std::size_t i = cylispheres->size()-3; i < cylispheres->size(); ++i)
    {
    (*cylispheres)[i].p1 = R*(*cylispheres)[i].p1 + T;
    (*cylispheres)[i].p2 = R*(*cylispheres)[i].p2 + T;
    }
}

// This terribly obscure and magic-number-filled implementation was
// transcribed from a MATLAB implementation of the same. Please
// forgive me
void DavinciKinematics::getPassivePrimitives(const Eigen::Matrix4d& baseFrame,
                                             const std::vector<double>& q,
                                             std::vector<Collisions::Cylisphere>* cylispheres,
                                             Collisions::Sphere* sphere) const
{
  double c2 = cos(q[1]);
  double s2 = sin(q[1]);
  double c3 = cos(q[2]);
  double s3 = sin(q[2]);
  double c234 = cos(q[1] + q[2] + q[3]);
  double s234 = sin(q[1] + q[2] + q[3]);
  double c5 = cos(q[4]);
  double s5 = sin(q[4]);
  double r = 0.07;
  double l1=116e-3;
  double l2=129e-3;
  double l3=163e-3;
  double l4=146e-3;
  double l5=78e-3;
  double l6=338e-3;
  double l7=520e-3;
  double r2=140e-3;

  Collisions::Cylisphere C1;
  C1.p1 << 0.189, 0.0, q[0];
  C1.p2 << 0.419*c2 + 0.189, 0.419*s2, q[0];
  C1.r = r;

  Collisions::Cylisphere C2;
  C2.p1 = C1.p1;
  C2.p2 = C1.p1;
  C2.p2(2) += l1;
  C2.r = r;

  Collisions::Cylisphere C3;
  C3.p1 = C1.p2;
  C3.p1(2) += 0.5*0.29;
  C3.p2 = C3.p1;
  C3.p2(0) += (17.0/40.0)*(c2*c3 - s2*s3);
  C3.p2(1) += (17.0/40.0)*(c2*s3 + c3*s2);
  C3.r = r;

  Collisions::Cylisphere C4;
  C4.p1 = C1.p2;
  C4.p2 = C3.p1;
  C4.p2(2) += l1;
  C4.r = r;

  Collisions::Cylisphere C5;
  C5.p1 = C3.p2;
  C5.p1(2) += l1;
  C5.p2 = C3.p2;
  C5.p2(2) -= l2;
  C5.r = r;

  Collisions::Cylisphere C6;
  C6.p1 = C5.p2;
  C6.p2 = C5.p2;
  C6.p2(0) += -l3*c234;
  C6.p2(1) += -l3*s234;
  C6.r = r;

  Collisions::Cylisphere C7;
  C7.p1 = C5.p2;
  C7.p1(0) += l4*s234*s5 + l5*c234;
  C7.p1(1) += -l4*c234*s5 +  l5*s234;
  C7.p1(2) += -l4*c5;
  C7.p2 = C7.p1;
  C7.p2(0) += l6*c234;
  C7.p2(1) += l6*s234;
  C7.r = 0.5*r;

  // sorry mother
  Eigen::Matrix4d rcmFrame = this->passiveFK(Eigen::Matrix4d::Identity(), q);
  Collisions::Sphere s;
  s.p = rcmFrame.topRightCorner<3,1>() - l7*rcmFrame.block<3,1>(0,2);
  s.r = r2;

  cylispheres->push_back(C1);
  cylispheres->push_back(C2);
  cylispheres->push_back(C3);
  cylispheres->push_back(C4);
  cylispheres->push_back(C5);
  cylispheres->push_back(C6);
  cylispheres->push_back(C7);
  *sphere = s;

  Eigen::Matrix3d R = baseFrame.topLeftCorner<3,3>();
  Eigen::Vector3d T = baseFrame.topRightCorner<3,1>();
  for (std::size_t i = cylispheres->size()-7; i < cylispheres->size(); ++i)
    {
    (*cylispheres)[i].p1 = R*(*cylispheres)[i].p1 + T;
    (*cylispheres)[i].p2 = R*(*cylispheres)[i].p2 + T;
    }
  sphere->p = R*sphere->p + T;
}

DavinciParameters DavinciKinematics::getParams() const
{
  return params_;
}

// \TODO only construct one triangle of covariance matrix (symmetric
// obviously)
void DavinciKinematics::unscentedIK(const Eigen::Matrix4d& portFrame,
                                    const Eigen::Matrix4d& meanTargetPose,
                                    const Eigen::Vector3d& positionVariance,
                                    const Eigen::Vector3d& orientVariance,
                                    std::vector<double>* mean_qOut,
                                    std::vector<double>* covariance_qOut) const
{
  Eigen::Matrix4d targetPose = meanTargetPose;
  std::vector<std::vector<double> > sigmaPoints;
  std::vector<double> q(6);

  // Perturb translation elements
  for (unsigned i = 0; i < 3; ++i)
    {
    double delta = sqrt(2*positionVariance(i));

    targetPose(i,3) += delta;
    this->intraIK(portFrame, targetPose, &q);
    sigmaPoints.push_back(q);

    targetPose(i,3) -= 2*delta;
    this->intraIK(portFrame, targetPose, &q);
    sigmaPoints.push_back(q);
    }

  // Perturb orientation elements
  double delta = sqrt(2*orientVariance(0));
  Eigen::Matrix4d R = xRotation(delta);
  targetPose.noalias() = R*meanTargetPose;
  this->intraIK(portFrame, targetPose, &q);
  sigmaPoints.push_back(q);

  targetPose.noalias() = R.transpose()*meanTargetPose;
  this->intraIK(portFrame, targetPose, &q);
  sigmaPoints.push_back(q);

  delta = sqrt(2*orientVariance(1));
  R = yRotation(delta);
  targetPose.noalias() = R*meanTargetPose;
  this->intraIK(portFrame, targetPose, &q);
  sigmaPoints.push_back(q);

  targetPose.noalias() = R.transpose()*meanTargetPose;
  this->intraIK(portFrame, targetPose, &q);
  sigmaPoints.push_back(q);

  delta = sqrt(2*orientVariance(2));
  R = zRotation(delta);
  targetPose.noalias() = R*meanTargetPose;
  this->intraIK(portFrame, targetPose, &q);
  sigmaPoints.push_back(q);

  targetPose.noalias() = R.transpose()*meanTargetPose;
  this->intraIK(portFrame, targetPose, &q);
  sigmaPoints.push_back(q);
  // end orientation perturbations

  // get moments of transformed sigma points
  std::vector<double> mean_q(6, 0.0);
  for (unsigned i = 0; i < 12; ++i)
    {
    for (unsigned j = 0; j < 6; ++j)
      mean_q[j] += sigmaPoints[i][j];
    }
  for (unsigned i = 0; i < 6; ++i)
    mean_q[i] /= 12.0;

  std::vector<double> covariance_q(6, 0.0);
  for (unsigned i = 0; i < 6; ++i)
    {
    for (unsigned k = 0; k < 12; ++k)
      covariance_q[i] += (sigmaPoints[k][i] - mean_q[i])*(sigmaPoints[k][i] - mean_q[i]);
    covariance_q[i] /= (12 - 1);
    }

  // populate out-parameters
  mean_qOut->swap(mean_q);
  covariance_qOut->swap(covariance_q);
}

double DavinciKinematics::fullClearances(const Eigen::Matrix4d& baseFrameL,
                                         const Eigen::Matrix4d& baseFrameR,
                                         const std::vector<double>& qL,
                                         const std::vector<double>& qR,
                                         const Eigen::Matrix4d& targetPose,
                                         std::vector<double>* dists) const
{
  std::vector<Collisions::Cylisphere> cylL, cylR;
  std::vector<Collisions::Sphere> sL(1), sR(1);
  this->getPassivePrimitives(baseFrameL, qL, &cylL, &sL[0]);
  this->getPassivePrimitives(baseFrameR, qR, &cylR, &sR[0]);

  std::vector<double> qaL(6);
  std::vector<double> qaR(6);
  Eigen::Matrix4d pL = this->passiveFK(baseFrameL, qL);
  Eigen::Matrix4d pR = this->passiveFK(baseFrameR, qR);
  this->intraIK(pL, targetPose, &qaL);
  this->intraIK(pR, targetPose, &qaR);

  std::vector<Collisions::Cylisphere> cylAL, cylAR;
  this->getExtraCylispheres(pL, qaL, &cylAL);
  this->getExtraCylispheres(pR, qaR, &cylAR);

  for (unsigned i = 0; i < cylAL.size(); ++i)
    {
    // don't need to add actives for left arm since we don't need to
    // check active-active collisions twice
    cylR.push_back(cylAR[i]);
    }

  std::vector<Collisions::Sphere> empty;
  double d1 = Collisions::distances(cylAL, empty, cylR, sR, dists);
  double d2 = Collisions::distances(cylAR, empty, cylL, sL, dists);

  return std::min(d1,d2);
}

unsigned DavinciKinematics::numActiveClearances() const
{
  return 57; // TODO figure out how to do this programmatically
}

// \TODO for efficiency compute unscented IK and unscented clearance
// at the same time
void DavinciKinematics::unscentedClearance(const Eigen::Matrix4d& baseFrameL,
                                           const Eigen::Matrix4d& baseFrameR,
                                           const std::vector<double>& qpL,
                                           const std::vector<double>& qpR,
                                           const Eigen::Matrix4d& meanTargetPose,
                                           const Eigen::Vector3d& positionVariance,
                                           const Eigen::Vector3d& orientVariance,
                                           std::vector<double>* mean_clearanceOut,
                                           std::vector<double>* cov_clearanceOut) const
{
  Eigen::Matrix4d targetPose = meanTargetPose;
  std::vector< std::vector<double> > sigmaPoints;
  std::vector<double> c;

  // Perturb translation elements
  for (unsigned i = 0; i < 3; ++i)
    {
    double delta = sqrt(2*positionVariance(i));
    targetPose(i,3) += delta;
    c.clear();
    this->fullClearances(baseFrameL, baseFrameR, qpL, qpR, targetPose, &c);
    sigmaPoints.push_back(c);

    targetPose(i,3) -= 2*delta;
    c.clear();
    this->fullClearances(baseFrameL, baseFrameR, qpL, qpR, targetPose, &c);
    sigmaPoints.push_back(c);
    }

  // Perturb orientation elements
  double delta = sqrt(2*orientVariance(0));
  Eigen::Matrix4d R = xRotation(delta);
  targetPose.noalias() = R*meanTargetPose;
  c.clear();
  this->fullClearances(baseFrameL, baseFrameR, qpL, qpR, targetPose, &c);
  sigmaPoints.push_back(c);

  targetPose.noalias() = R.transpose()*meanTargetPose;
  c.clear();
  this->fullClearances(baseFrameL, baseFrameR, qpL, qpR, targetPose, &c);
  sigmaPoints.push_back(c);

  delta = sqrt(2*orientVariance(1));
  R = yRotation(delta);
  targetPose.noalias() = R*meanTargetPose;
  c.clear();
  this->fullClearances(baseFrameL, baseFrameR, qpL, qpR, targetPose, &c);
  sigmaPoints.push_back(c);

  targetPose.noalias() = R.transpose()*meanTargetPose;
  c.clear();
  this->fullClearances(baseFrameL, baseFrameR, qpL, qpR, targetPose, &c);
  sigmaPoints.push_back(c);

  delta = sqrt(2*orientVariance(2));
  R = zRotation(delta);
  targetPose.noalias() = R*meanTargetPose;
  c.clear();
  this->fullClearances(baseFrameL, baseFrameR, qpL, qpR, targetPose, &c);
  sigmaPoints.push_back(c);

  targetPose.noalias() = R.transpose()*meanTargetPose;
  c.clear();
  this->fullClearances(baseFrameL, baseFrameR, qpL, qpR, targetPose, &c);
  sigmaPoints.push_back(c);
  // end orientation perturbations

  // get moments of transformed sigma points
  std::vector<double> mean_clearance(this->numActiveClearances(), 0.0);
  for (unsigned i = 0; i < 12; ++i)
    {
    for (unsigned j = 0; j < this->numActiveClearances(); ++j)
      mean_clearance[j] += sigmaPoints[i][j];
    }
  for (unsigned i = 0; i < this->numActiveClearances(); ++i)
    mean_clearance[i] /= 12.0;

  std::vector<double> cov_clearance(this->numActiveClearances(), 0.0);
  for (unsigned i = 0; i < this->numActiveClearances(); ++i)
    {
    for (unsigned k = 0; k < 12; ++k)
      cov_clearance[i] += (sigmaPoints[k][i] - mean_clearance[i])*(sigmaPoints[k][i] - mean_clearance[i]);
    cov_clearance[i] /= (12 - 1);
    }

  // populate out-parameters
  mean_clearanceOut->swap(mean_clearance);
  cov_clearanceOut->swap(cov_clearance);
}

double DavinciKinematics::getPassiveJointMin(unsigned idx) const
{
  return passiveLowerBounds[idx];
}

double DavinciKinematics::getPassiveJointMax(unsigned idx) const
{
  return passiveUpperBounds[idx];
}

double DavinciKinematics::getActiveJointMin(unsigned idx) const
{
  return activeLowerBounds[idx];
}

double DavinciKinematics::getActiveJointMax(unsigned idx) const
{
  return activeUpperBounds[idx];
}

void DavinciKinematics::getDefaultPassiveConfig(std::vector<double>* q) const
{
  (*q)[0] = 0.5;
  std::fill(q->begin()+1, q->end(), 0.0);
}

void DavinciKinematics::getDefaultActiveConfig(std::vector<double>* q) const
{
  std::fill(q->begin(), q->end(), 0.0);
}
