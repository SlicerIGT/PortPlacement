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

// Qt includes
#include <QDebug>

// Slicer includes
#include "qSlicerAutoPortPlacementModuleWidget.h"
#include "ui_qSlicerAutoPortPlacementModuleWidget.h"

// Logic includes
#include "vtkSlicerAutoPortPlacementLogic.h"

// VTK includes
#include <vtkTransform.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerAutoPortPlacementModuleWidgetPrivate: public Ui_qSlicerAutoPortPlacementModuleWidget
{
public:
  qSlicerAutoPortPlacementModuleWidgetPrivate();
  int CurrentLeftPassiveIdx;
  int CurrentRightPassiveIdx;
};

//-----------------------------------------------------------------------------
// qSlicerAutoPortPlacementModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerAutoPortPlacementModuleWidgetPrivate::qSlicerAutoPortPlacementModuleWidgetPrivate() :
  CurrentLeftPassiveIdx(0),
  CurrentRightPassiveIdx(0)
{
}

//-----------------------------------------------------------------------------
// qSlicerAutoPortPlacementModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerAutoPortPlacementModuleWidget::qSlicerAutoPortPlacementModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerAutoPortPlacementModuleWidgetPrivate )
{
}

//-----------------------------------------------------------------------------
qSlicerAutoPortPlacementModuleWidget::~qSlicerAutoPortPlacementModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerAutoPortPlacementModuleWidget::RefreshConfigSliders()
{
  Q_D(qSlicerAutoPortPlacementModuleWidget);

  vtkSlicerAutoPortPlacementLogic *portLogic =
    vtkSlicerAutoPortPlacementLogic::SafeDownCast(this->logic());

  d->LeftPassiveSlider->setMinimum(portLogic->GetPassiveJointMin(d->CurrentLeftPassiveIdx));
  d->LeftPassiveSlider->setMaximum(portLogic->GetPassiveJointMax(d->CurrentLeftPassiveIdx));
  d->RightPassiveSlider->setMinimum(portLogic->GetPassiveJointMin(d->CurrentRightPassiveIdx));
  d->RightPassiveSlider->setMaximum(portLogic->GetPassiveJointMax(d->CurrentRightPassiveIdx));

  d->LeftPassiveSlider->setValue(portLogic->GetPassiveLeftJoint(d->CurrentLeftPassiveIdx));
  d->RightPassiveSlider->setValue(portLogic->GetPassiveRightJoint(d->CurrentRightPassiveIdx));
}

//-----------------------------------------------------------------------------
void qSlicerAutoPortPlacementModuleWidget::setup()
{
  Q_D(qSlicerAutoPortPlacementModuleWidget);
  d->setupUi(this);

  this->Superclass::setup();

  d->LeftPassiveSlider->setSingleStep(0.001);
  d->RightPassiveSlider->setSingleStep(0.001);

  d->LeftPassiveConfigCombo->setCurrentIndex(d->CurrentLeftPassiveIdx);
  d->RightPassiveConfigCombo->setCurrentIndex(d->CurrentRightPassiveIdx);

  this->RefreshConfigSliders();
}

//-----------------------------------------------------------------------------
void qSlicerAutoPortPlacementModuleWidget::onLeftPassiveComboChanged(int idx)
{
  Q_D(qSlicerAutoPortPlacementModuleWidget);

  vtkSlicerAutoPortPlacementLogic *portLogic =
    vtkSlicerAutoPortPlacementLogic::SafeDownCast(this->logic());

  d->CurrentLeftPassiveIdx = idx;

  d->LeftPassiveSlider->setMinimum(portLogic->GetPassiveJointMin(idx));
  d->LeftPassiveSlider->setMaximum(portLogic->GetPassiveJointMax(idx));

  d->LeftPassiveSlider->setValue(portLogic->GetPassiveLeftJoint(idx));
}

//-----------------------------------------------------------------------------
void qSlicerAutoPortPlacementModuleWidget::onRightPassiveComboChanged(int idx)
{
  Q_D(qSlicerAutoPortPlacementModuleWidget);

  vtkSlicerAutoPortPlacementLogic *portLogic =
    vtkSlicerAutoPortPlacementLogic::SafeDownCast(this->logic());

  d->CurrentRightPassiveIdx = idx;

  d->RightPassiveSlider->setMinimum(portLogic->GetPassiveJointMin(idx));
  d->RightPassiveSlider->setMaximum(portLogic->GetPassiveJointMax(idx));

  d->RightPassiveSlider->setValue(portLogic->GetPassiveRightJoint(idx));
}

//-----------------------------------------------------------------------------
void qSlicerAutoPortPlacementModuleWidget::onLeftPassiveSliderChanged(double value)
{
  Q_D(qSlicerAutoPortPlacementModuleWidget);

  vtkSlicerAutoPortPlacementLogic *portLogic =
    vtkSlicerAutoPortPlacementLogic::SafeDownCast(this->logic());

  portLogic->SetPassiveLeftJoint(d->CurrentLeftPassiveIdx, value);

  portLogic->RenderRobot();
}

//-----------------------------------------------------------------------------
void qSlicerAutoPortPlacementModuleWidget::onRightPassiveSliderChanged(double value)
{
  Q_D(qSlicerAutoPortPlacementModuleWidget);

  vtkSlicerAutoPortPlacementLogic *portLogic =
    vtkSlicerAutoPortPlacementLogic::SafeDownCast(this->logic());

  portLogic->SetPassiveRightJoint(d->CurrentRightPassiveIdx, value);

  portLogic->RenderRobot();
}

//-----------------------------------------------------------------------------
void qSlicerAutoPortPlacementModuleWidget::onSpecComboBoxChanged()
{
  Q_D(qSlicerAutoPortPlacementModuleWidget);

  bool enabled =
    d->taskFrameComboBox->currentNode() &&
    d->portCurveComboBox->currentNode() &&
    d->robotBaseComboBox->currentNode();

  d->findSurgicalPlanButton->setEnabled(enabled);
}

//-----------------------------------------------------------------------------
void qSlicerAutoPortPlacementModuleWidget::onFindSurgicalPlanButtonPressed()
{
  Q_D(qSlicerAutoPortPlacementModuleWidget);

  vtkSlicerAutoPortPlacementLogic *portLogic =
    vtkSlicerAutoPortPlacementLogic::SafeDownCast(this->logic());

  portLogic->FindFeasiblePlan(d->taskFrameComboBox->currentNode(),
                              d->portCurveComboBox->currentNode(),
                              d->robotBaseComboBox->currentNode());
}

//-----------------------------------------------------------------------------
void qSlicerAutoPortPlacementModuleWidget::onResetConfigButtonPressed()
{
  vtkSlicerAutoPortPlacementLogic *portLogic =
    vtkSlicerAutoPortPlacementLogic::SafeDownCast(this->logic());

  portLogic->ResetJointsToDefault();

  this->RefreshConfigSliders();

  portLogic->RenderRobot();
}
