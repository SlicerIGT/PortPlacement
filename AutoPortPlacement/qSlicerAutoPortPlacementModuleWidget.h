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

#ifndef __qSlicerAutoPortPlacementModuleWidget_h
#define __qSlicerAutoPortPlacementModuleWidget_h

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerAutoPortPlacementModuleExport.h"

class qSlicerAutoPortPlacementModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_AUTOPORTPLACEMENT_EXPORT qSlicerAutoPortPlacementModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerAutoPortPlacementModuleWidget(QWidget *parent=0);
  ~qSlicerAutoPortPlacementModuleWidget() override;

  // If robot configuration is changed programmatically, have sliders
  // reflect change
  void RefreshConfigSliders();

public slots:
  void onLeftPassiveComboChanged(int idx);
  void onLeftPassiveSliderChanged(double value);
  void onRightPassiveComboChanged(int idx);
  void onRightPassiveSliderChanged(double value);
  void onSpecComboBoxChanged();
  void onResetConfigButtonPressed();

  void onFindSurgicalPlanButtonPressed();

protected:
  QScopedPointer<qSlicerAutoPortPlacementModuleWidgetPrivate> d_ptr;

  void setup() override;

private:
  Q_DECLARE_PRIVATE(qSlicerAutoPortPlacementModuleWidget);
  Q_DISABLE_COPY(qSlicerAutoPortPlacementModuleWidget);
};

#endif
