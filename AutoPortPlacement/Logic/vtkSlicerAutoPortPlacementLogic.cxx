/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// AutoPortPlacement Logic includes
#include "vtkSlicerAutoPortPlacementLogic.h"

// Optimization includes
#include <optim/optim.h>

// Kinematics includes
#include <davinci-kinematics/davinci.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkMRMLAnnotationROINode.h>

// VTK includes
#include <vtkNew.h>
#include <vtkTransform.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerAutoPortPlacementLogic);

namespace
{

  // Given a cylisphere returned by the DavinciKinematics object,
  // returns a vtkMRMLLinearTransformNode that will transform the
  // vtkCylinderSource's output to correspond to the input cylisphere.
  void cylisphereToTransformNode(const Collisions::Cylisphere& c,
                                 vtkSmartPointer<vtkMRMLLinearTransformNode> tNode)
  {
    Eigen::Vector3d p1 = c.p1*1000;
    Eigen::Vector3d p2 = c.p2*1000;

    double cylinderRad = c.r*1000;
    Eigen::Vector3d cylinderVec = p2 - p1;
    double cylinderHeight = cylinderVec.norm();

    // Rotate
    cylinderVec.normalize();
    Eigen::Vector3d e2; e2(0) = 0.0; e2(1) = 1.0; e2(2) = 0.0;
    Eigen::Vector3d rotationAxis = e2.cross(cylinderVec);
    double angle = acos(e2.dot(cylinderVec));
    rotationAxis.normalize();

    vtkSmartPointer<vtkMatrix4x4> m = vtkSmartPointer<vtkMatrix4x4>::New();
    tNode->GetMatrixTransformToWorld(m.GetPointer());
    vtkSmartPointer<vtkTransform> t = vtkSmartPointer<vtkTransform>::New();
    t->SetMatrix(m);
    t->Inverse();
    tNode->ApplyTransformMatrix(t->GetMatrix());

    t->Identity();
    t->Translate(p1(0), p1(1), p1(2));
    t->RotateWXYZ(vtkMath::DegreesFromRadians(angle),
                  rotationAxis(0), rotationAxis(1), rotationAxis(2));
    t->Translate(0.0, 0.5*cylinderHeight, 0.0);
    t->Scale(cylinderRad, cylinderHeight, cylinderRad);

    tNode->ApplyTransformMatrix(t->GetMatrix());
  }

  // Given a sphere returned by DavinciKinematics, generates a
  // vtkMRMLLinearTransformNode that transforms the vtkSphereSource's
  // output to correspond to the input sphere.
  void sphereToTransformNode(const Collisions::Sphere& s,
                             vtkSmartPointer<vtkMRMLLinearTransformNode> tNode)
  {
    vtkSmartPointer<vtkMatrix4x4> m = vtkSmartPointer<vtkMatrix4x4>::New();
    tNode->GetMatrixTransformToWorld(m.GetPointer());
    vtkSmartPointer<vtkTransform> t = vtkSmartPointer<vtkTransform>::New();
    t->SetMatrix(m);
    t->Inverse();
    tNode->ApplyTransformMatrix(t->GetMatrix());

    t->Identity();
    Eigen::Vector3d p = 1000*s.p;
    double r = 1000*s.r;
    t->Translate(p(0), p(1), p(2));
    t->Scale(r, r, r);

    tNode->ApplyTransformMatrix(t->GetMatrix());
  }
}

//----------------------------------------------------------------------------
vtkSlicerAutoPortPlacementLogic::vtkSlicerAutoPortPlacementLogic() :
  RobotBaseX(0.0),
  RobotBaseY(-0.5),
  RobotBaseZ(-0.5),
  LeftPassiveConfig(6),
  RightPassiveConfig(6),
  LeftActiveConfig(6),
  RightActiveConfig(6),
  IsRobotInitialized(false),
  Kinematics(nullptr)
{

  this->CylinderSource = vtkSmartPointer<vtkCylinderSource>::New();
  this->CylinderSource->SetHeight(1.0);
  this->CylinderSource->SetCenter(0.0,0.0,0.0);
  this->CylinderSource->SetRadius(1.0);

  this->SphereSource = vtkSmartPointer<vtkSphereSource>::New();
  this->SphereSource->SetRadius(1.0);
}

//----------------------------------------------------------------------------
vtkSlicerAutoPortPlacementLogic::~vtkSlicerAutoPortPlacementLogic()
{
  delete this->Kinematics;
}

//----------------------------------------------------------------------------
void vtkSlicerAutoPortPlacementLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkSlicerAutoPortPlacementLogic::RenderRobot()
{
  if (this->IsRobotInitialized)
    this->UpdateRobot();
  else
    this->InitRobot();
}

//----------------------------------------------------------------------------
// Initializes the models and transforms for visualizing the Davinci robot.
void vtkSlicerAutoPortPlacementLogic::InitRobot()
{
  if (!this->GetMRMLScene())
    {
    return;
    }

  std::vector<Collisions::Cylisphere> cylispheres;
  std::vector<Collisions::Sphere> spheres(2);

  // Make cylisphere for robot's stand
  Collisions::Cylisphere standCyl;
  standCyl.p1 = Eigen::Vector3d(this->RobotBaseX, this->RobotBaseY, this->RobotBaseZ);
  standCyl.p2 = Eigen::Vector3d(this->RobotBaseX, this->RobotBaseY, this->RobotBaseZ + 1.0);
  standCyl.r = 0.07; // Just a number that looks right.
  cylispheres.push_back(standCyl);

  Eigen::Matrix4d baseFrameL = Eigen::Matrix4d::Identity();
  baseFrameL(0,0) = -1.0;
  baseFrameL(1,1) = -1.0;
  baseFrameL(0,3) = this->RobotBaseX;
  baseFrameL(1,3) = this->RobotBaseY;
  baseFrameL(2,3) = this->RobotBaseZ;
  this->Kinematics->getPassivePrimitives(baseFrameL,
                                         this->LeftPassiveConfig,
                                         &cylispheres,
                                         &(spheres[0]));
  Eigen::Matrix4d baseFrameR = Eigen::Matrix4d::Identity();
  baseFrameR(0,3) = this->RobotBaseX;
  baseFrameR(1,3) = this->RobotBaseY;
  baseFrameR(2,3) = this->RobotBaseZ;
  this->Kinematics->getPassivePrimitives(baseFrameR,
                                         this->RightPassiveConfig,
                                         &cylispheres,
                                         &(spheres[1]));
  this->Kinematics->getExtraCylispheres(this->Kinematics->passiveFK(baseFrameL,
                                                                    this->LeftPassiveConfig),
                                        this->LeftActiveConfig,
                                        &cylispheres);
  this->Kinematics->getExtraCylispheres(this->Kinematics->passiveFK(baseFrameR,
                                                                    this->RightPassiveConfig),
                                        this->RightActiveConfig,
                                        &cylispheres);

  for (unsigned i = 0; i < cylispheres.size(); ++i)
    {
    vtkSmartPointer<vtkMRMLModelNode> modelNode = vtkSmartPointer<vtkMRMLModelNode>::New();
    modelNode->SetName(this->GetMRMLScene()->GenerateUniqueName("Link").c_str());

    vtkSmartPointer<vtkPolyData> polyData = this->CylinderSource->GetOutput();
    modelNode->SetAndObservePolyData(polyData);

    this->GetMRMLScene()->AddNode(modelNode);

    vtkSmartPointer<vtkMRMLModelDisplayNode> displayNode =
      vtkSmartPointer<vtkMRMLModelDisplayNode>::New();
    displayNode->SetColor(0.0,1.0,1.0); // cyan
    this->GetMRMLScene()->AddNode(displayNode);
    modelNode->SetAndObserveDisplayNodeID(displayNode->GetID());

    vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode =
      vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    cylisphereToTransformNode(cylispheres[i],
                              transformNode);
    this->GetMRMLScene()->AddNode(transformNode);
    modelNode->SetAndObserveTransformNodeID(transformNode->GetID());

    this->RobotTransformNodes.push_back(transformNode);
    }

  for (unsigned i = 0; i < spheres.size(); ++i)
    {
    vtkSmartPointer<vtkMRMLModelNode> modelNode = vtkSmartPointer<vtkMRMLModelNode>::New();
    modelNode->SetName(this->GetMRMLScene()->GenerateUniqueName("Link").c_str());
    vtkSmartPointer<vtkPolyData> polyData = this->SphereSource->GetOutput();
    modelNode->SetAndObservePolyData(polyData);

    this->GetMRMLScene()->AddNode(modelNode);

    vtkSmartPointer<vtkMRMLModelDisplayNode> displayNode =
      vtkSmartPointer<vtkMRMLModelDisplayNode>::New();
    displayNode->SetColor(0.0,1.0,1.0); // cyan
    this->GetMRMLScene()->AddNode(displayNode);
    modelNode->SetAndObserveDisplayNodeID(displayNode->GetID());

    vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode =
      vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    sphereToTransformNode(spheres[i], transformNode);
    this->GetMRMLScene()->AddNode(transformNode);
    modelNode->SetAndObserveTransformNodeID(transformNode->GetID());

    this->RobotTransformNodes.push_back(transformNode);
    }

  this->IsRobotInitialized = true;
}

//---------------------------------------------------------------------------
// Takes the robot's current robot base position and passive/active
// left/right arm configurations and updates the robot visualization
// correspondingly.
void vtkSlicerAutoPortPlacementLogic::UpdateRobot()
{
  std::vector<Collisions::Cylisphere> cylispheres;
  std::vector<Collisions::Sphere> spheres(2);

  // Make cylisphere for robot's stand
  Collisions::Cylisphere standCyl;
  standCyl.p1 = Eigen::Vector3d(this->RobotBaseX, this->RobotBaseY, this->RobotBaseZ);
  standCyl.p2 = Eigen::Vector3d(this->RobotBaseX, this->RobotBaseY, this->RobotBaseZ + 1.0);
  standCyl.r = 0.07; // ugh
  cylispheres.push_back(standCyl);

  Eigen::Matrix4d baseFrameL = Eigen::Matrix4d::Identity();
  baseFrameL(0,0) = -1.0;
  baseFrameL(1,1) = -1.0;
  baseFrameL(0,3) = this->RobotBaseX;
  baseFrameL(1,3) = this->RobotBaseY;
  baseFrameL(2,3) = this->RobotBaseZ;
  this->Kinematics->getPassivePrimitives(baseFrameL,
                                         this->LeftPassiveConfig,
                                         &cylispheres,
                                         &(spheres[0]));
  Eigen::Matrix4d baseFrameR = Eigen::Matrix4d::Identity();
  baseFrameR(0,3) = this->RobotBaseX;
  baseFrameR(1,3) = this->RobotBaseY;
  baseFrameR(2,3) = this->RobotBaseZ;
  this->Kinematics->getPassivePrimitives(baseFrameR,
                                         this->RightPassiveConfig,
                                         &cylispheres,
                                         &(spheres[1]));
  this->Kinematics->getExtraCylispheres(this->Kinematics->passiveFK(baseFrameL,
                                                                    this->LeftPassiveConfig),
                                        this->LeftActiveConfig,
                                        &cylispheres);
  this->Kinematics->getExtraCylispheres(this->Kinematics->passiveFK(baseFrameR,
                                                                    this->RightPassiveConfig),
                                        this->RightActiveConfig,
                                        &cylispheres);

  for (unsigned i = 0; i < cylispheres.size(); ++i)
    {
    cylisphereToTransformNode(cylispheres[i],
                              this->RobotTransformNodes[i]);
    }
  for (unsigned i = 0; i < spheres.size(); ++i)
    {
    sphereToTransformNode(spheres[i],
                          this->RobotTransformNodes[cylispheres.size()+i]);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerAutoPortPlacementLogic::
FindFeasiblePlan(vtkMRMLNode* taskFramesNode,
                 vtkMRMLNode* portCurveNode,
                 vtkMRMLNode* robotBaseNode)
{
  vtkMRMLMarkupsFiducialNode* taskFramesFiducial =
    vtkMRMLMarkupsFiducialNode::SafeDownCast(taskFramesNode);
  vtkMRMLMarkupsFiducialNode* portCurvePointsFiducial =
    vtkMRMLMarkupsFiducialNode::SafeDownCast(portCurveNode);
  vtkMRMLAnnotationROINode* robotBaseROI =
    vtkMRMLAnnotationROINode::SafeDownCast(robotBaseNode);

  // Retrieve our task frames
  if (taskFramesFiducial->GetNumberOfFiducials() < 1)
    {
    vtkErrorMacro("FindFeasiblePlan: task frames node must have at least 1 fiducial!");
    return;
    }
  Optim::Matrix4dVec taskFrames;
  for (int tIdx = 0; tIdx < taskFramesFiducial->GetNumberOfFiducials(); ++tIdx)
    {
    Eigen::Matrix4d taskFrame = Eigen::Matrix4d::Identity();

    // Get position part
    double pos[3];
    taskFramesFiducial->GetNthFiducialPosition(tIdx, pos);
    for (unsigned i = 0; i < 3; ++i)
      taskFrame(i,3) = pos[i] / 1000;

    // Get orientation part
    double quat[4];
    taskFramesFiducial->GetNthControlPointOrientation(tIdx, quat);
    double rotMat[3][3];
    vtkMath::QuaternionToMatrix3x3(quat, rotMat);
    for (unsigned i = 0; i < 3; ++i)
      for (unsigned j = 0; j < 3; ++j)
        taskFrame(i,j) = rotMat[i][j];

    taskFrames.push_back(taskFrame);
    }

  // Get our port curve endpoints from portCurvePointsFiducial
  if (portCurvePointsFiducial->GetNumberOfFiducials() < 2)
    {
    vtkErrorMacro("FindFeasiblePlan: Port curve points node must have at least 2 fiducials!");
    return;
    }
  Eigen::Vector3d portCurvePoints[2];
  for (unsigned pIdx = 0; pIdx < 2; ++pIdx)
    {
    double pos[3];
    portCurvePointsFiducial->GetNthFiducialPosition(pIdx, pos);
    for (unsigned i = 0; i < 3; ++i)
      portCurvePoints[pIdx](i) = pos[i] / 1000;

    std::cout << "Port curve pt:" << std::endl;
    std::cout << portCurvePoints[pIdx] << std::endl;
    }

  // Set up the robot's base
  //
  // For now use a default base orientation, with L arm rotated 180
  // deg.
  Eigen::Matrix3d baseOrientationL = Eigen::Matrix3d::Identity();
  baseOrientationL(0,0) = -1.0;
  baseOrientationL(1,1) = -1.0;
  Eigen::Matrix3d baseOrientationR = Eigen::Matrix3d::Identity();
  double radii[3];
  double center[3];
  robotBaseROI->GetRadiusXYZ(radii);
  robotBaseROI->GetXYZ(center);
  Eigen::Vector3d baseBoxMin, baseBoxMax;
  for (unsigned i = 0; i < 3; ++i)
    {
    baseBoxMin(i) = (center[i] - radii[i]) / 1000.0; // mm -> m
    baseBoxMax(i) = (center[i] + radii[i]) / 1000.0; // mm -> m
    }

  // Find a feasible plan!
  std::vector<double> qL(6);
  std::vector<double> qR(6);
  Eigen::Vector3d basePosition;
  Optim::findFeasiblePlan(*this->Kinematics, baseOrientationL, baseOrientationR,
                          baseBoxMin, baseBoxMax, taskFrames,
                          portCurvePoints[0], portCurvePoints[1], &qL, &qR, &basePosition);

  // Set robot to new plan
  this->RobotBaseX = basePosition(0);
  this->RobotBaseY = basePosition(1);
  this->RobotBaseZ = basePosition(2);
  for (unsigned i = 0; i < 6; ++i)
    {
    this->SetPassiveLeftJoint(i, qL[i]);
    this->SetPassiveRightJoint(i, qR[i]);
    }

  this->UpdateRobot();
}

//---------------------------------------------------------------------------
void vtkSlicerAutoPortPlacementLogic::SetPassiveLeftJoint(unsigned jointIdx, double value)
{
  this->LeftPassiveConfig[jointIdx] = value;
}

//---------------------------------------------------------------------------
void vtkSlicerAutoPortPlacementLogic::SetPassiveRightJoint(unsigned jointIdx, double value)
{
  this->RightPassiveConfig[jointIdx] = value;
}

//---------------------------------------------------------------------------
double vtkSlicerAutoPortPlacementLogic::GetPassiveLeftJoint(unsigned jointIdx) const
{
  return this->LeftPassiveConfig[jointIdx];
}

//---------------------------------------------------------------------------
double vtkSlicerAutoPortPlacementLogic::GetPassiveRightJoint(unsigned jointIdx) const
{
  return this->RightPassiveConfig[jointIdx];
}

//---------------------------------------------------------------------------
double vtkSlicerAutoPortPlacementLogic::GetPassiveJointMin(unsigned idx) const
{
  return this->Kinematics->getPassiveJointMin(idx);
}

//---------------------------------------------------------------------------
double vtkSlicerAutoPortPlacementLogic::GetPassiveJointMax(unsigned idx) const
{
  return this->Kinematics->getPassiveJointMax(idx);
}

//---------------------------------------------------------------------------
void vtkSlicerAutoPortPlacementLogic::ResetJointsToDefault()
{
  this->Kinematics->getDefaultPassiveConfig(&(this->LeftPassiveConfig));
  this->Kinematics->getDefaultPassiveConfig(&(this->RightPassiveConfig));
  this->Kinematics->getDefaultActiveConfig(&(this->LeftActiveConfig));
  this->Kinematics->getDefaultActiveConfig(&(this->RightActiveConfig));
}

//---------------------------------------------------------------------------
void vtkSlicerAutoPortPlacementLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
  
  if (!this->Kinematics)
  {
    std::string moduleShareDirectory = this->GetModuleShareDirectory() + "/davinci-parameters.xml";
    this->Kinematics = new DavinciKinematics(moduleShareDirectory);
    this->Kinematics->getDefaultPassiveConfig(&(this->LeftPassiveConfig));
    this->Kinematics->getDefaultPassiveConfig(&(this->RightPassiveConfig));
    this->Kinematics->getDefaultActiveConfig(&(this->LeftActiveConfig));
    this->Kinematics->getDefaultActiveConfig(&(this->RightActiveConfig));
  }

}

//-----------------------------------------------------------------------------
void vtkSlicerAutoPortPlacementLogic::RegisterNodes()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerAutoPortPlacementLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerAutoPortPlacementLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
void vtkSlicerAutoPortPlacementLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}
