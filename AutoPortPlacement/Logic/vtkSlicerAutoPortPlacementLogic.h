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

// .NAME vtkSlicerAutoPortPlacementLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerAutoPortPlacementLogic_h
#define __vtkSlicerAutoPortPlacementLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes

// VTK includes
#include <vtkCylinderSource.h>
#include <vtkSphereSource.h>
#include <vtkMRMLLinearTransformNode.h>

// STD includes
#include <cstdlib>

#include "vtkSlicerAutoPortPlacementModuleLogicExport.h"

// Forward declarations
class DavinciKinematics;


/// \ingroup Slicer_QtModules_ExtensionTemplate
class VTK_SLICER_AUTOPORTPLACEMENT_MODULE_LOGIC_EXPORT vtkSlicerAutoPortPlacementLogic :
  public vtkSlicerModuleLogic
{
public:

  static vtkSlicerAutoPortPlacementLogic *New();
  vtkTypeMacro(vtkSlicerAutoPortPlacementLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  // For setting the rotation values of each joint in the robot's arm
  void SetPassiveLeftJoint(unsigned jointIdx, double value);
  void SetPassiveRightJoint(unsigned jointIdx, double value);

  // Retrieve the current rotation values of each joint in the robot's
  // arm
  double GetPassiveLeftJoint(unsigned jointIdx) const;
  double GetPassiveRightJoint(unsigned jointIdx) const;

  // Get joint limits for each joint in the robot's arm (arms are
  // symmetric)
  double GetPassiveJointMin(unsigned idx) const;
  double GetPassiveJointMax(unsigned idx) const;

  // Render the current configuration of the robot as a set of
  // cylinders and spheres
  void RenderRobot();

  // Given:
  //
  // 1) a vtkMRMLMarkupsFiducialNode representing a list of task frames;
  //
  // 2) a vtkMRMLMarkupsFiducialNode representing the two endpoints of
  // a line of potential port positions;
  //
  // 3) a vtkMRMLMarkupsROINode/vtkMRMLAnnotationROINode representing a bounding box of
  // potential positions of the robot's base;
  //
  // Find a position for the robot base and configurations for the
  // robot arms to access the given task frames through port positions
  // lying on the potential port position curve. This method sets the
  // robot's base and arm configurations to this found plan.
  void FindFeasiblePlan(vtkMRMLNode* taskFramesNode,
                        vtkMRMLNode* portCurvePointsNode,
                        vtkMRMLNode* robotBaseNode);

  // Sets robot joint angles back to their default values.
  void ResetJointsToDefault();

protected:
  vtkSlicerAutoPortPlacementLogic();
  virtual ~vtkSlicerAutoPortPlacementLogic();

  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes();
  virtual void UpdateFromMRMLScene();
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);
private:

  vtkSlicerAutoPortPlacementLogic(const vtkSlicerAutoPortPlacementLogic&); // Not implemented
  void operator=(const vtkSlicerAutoPortPlacementLogic&);               // Not implemented

  // Computes the robot arms' positions based on their joint angles.
  DavinciKinematics* Kinematics;

  /// For rendering the collision primitives of the davinci
  vtkSmartPointer<vtkCylinderSource> CylinderSource;
  vtkSmartPointer<vtkSphereSource> SphereSource;

  // Current position of the robot base
  double RobotBaseX;
  double RobotBaseY;
  double RobotBaseZ;

  // Configurations of the robot arms, separated by their passive and
  // active configurations. Note that the passive configurations are
  // fixed during the procedure, and the active configuration is what
  // moves the surgical tools inside the patient's body.
  std::vector<double> LeftPassiveConfig;
  std::vector<double> RightPassiveConfig;
  std::vector<double> LeftActiveConfig;
  std::vector<double> RightActiveConfig;

  bool IsRobotInitialized;

  std::vector<vtkSmartPointer<vtkMRMLLinearTransformNode> > RobotTransformNodes;

  void InitRobot();
  void UpdateRobot();
};

#endif
