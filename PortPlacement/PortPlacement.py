import os
import unittest
from __main__ import vtk, qt, ctk, slicer
import string
#
# PortPlacement
#

class PortPlacement:
  def __init__(self, parent):
    parent.title = "Port Placement"
    parent.categories = ["IGT"]
    parent.dependencies = []
    parent.contributors = ["Andinet Enquobahrie (Kitware), Luis G. Torres (UNC)"]
    parent.helpText = string.Template("""
    The PortPlacement module assists in the planning of surgical port placement in a laparoscopic procedure. Users can specify ports using fiducial markers and the module will automatically visualize simulated surgical tools at the port locations. Users can freely pivot the simulated surgical tools about the ports, as well as automatically orient all surgical tools toward a specified surgical target. Simulated tools are represented by cylinders whose lengths and radii can be varied from tool to tool.

    See the <a href=\"$a/Documentation/Nightly/Extensions/PortPlacement\">module documentation</a> for more details.
    """).substitute({ 'a':parent.slicerWikiUrl})
    parent.acknowledgementText = """
    This work was supported by NSF GRFP Grant No. DGE-1144081 and partially supported by NIH/NIBIB Grant No. 1R43EB014074-01. Anatomical atlas volume in module and extension icons courtesy of the Surgical Planning Laboratory (<a href=\"http://www.na-mic.org/publications/item/view/1266\">Link</a>)
"""
    self.parent = parent

    # Add this test to the SelfTest module's list for discovery when the module
    # is created.  Since this module may be discovered before SelfTests itself,
    # create the list if it doesn't already exist.
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}
    slicer.selfTests['PortPlacement'] = self.runTest

  def runTest(self):
    tester = PortPlacement()
    tester.runTest()

#
# PortPlacementWidget
#

class PortPlacementWidget:
  def __init__(self, parent = None):
    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout(qt.QVBoxLayout())
      self.parent.setMRMLScene(slicer.mrmlScene)
    else:
      self.parent = parent
    self.layout = self.parent.layout()
    if not parent:
      self.setup()
      self.parent.show()

  def setup(self):
    # Instantiate and connect widgets ...

    # #
    # # Reload and Test area
    # #
    # reloadCollapsibleButton = ctk.ctkCollapsibleButton()
    # reloadCollapsibleButton.text = "Reload && Test"
    # self.layout.addWidget(reloadCollapsibleButton)
    # reloadFormLayout = qt.QFormLayout(reloadCollapsibleButton)

    # # reload button
    # # (use this during development, but remove it when delivering
    # #  your module to users)
    # self.reloadButton = qt.QPushButton("Reload")
    # self.reloadButton.toolTip = "Reload this module."
    # self.reloadButton.name = "PortPlacement Reload"
    # reloadFormLayout.addWidget(self.reloadButton)
    # self.reloadButton.connect('clicked()', self.onReload)

    # # reload and test button
    # # (use this during development, but remove it when delivering
    # #  your module to users)
    # self.reloadAndTestButton = qt.QPushButton("Reload and Test")
    # self.reloadAndTestButton.toolTip = "Reload this module and then run the self tests."
    # reloadFormLayout.addWidget(self.reloadAndTestButton)
    # self.reloadAndTestButton.connect('clicked()', self.onReloadAndTest)

    #
    # Ports Area
    #
    portsCollapsibleButton = ctk.ctkCollapsibleButton()
    portsCollapsibleButton.text = "Surgical Ports"
    self.layout.addWidget(portsCollapsibleButton)
    portsFormLayout = qt.QFormLayout(portsCollapsibleButton)

    #
    # port fiducial list selector
    #
    self.portListSelector = slicer.qMRMLNodeComboBox()
    self.portListSelector.nodeTypes = ["vtkMRMLMarkupsFiducialNode"]
    self.portListSelector.addEnabled = False
    self.portListSelector.removeEnabled = False
    self.portListSelector.noneEnabled = True
    self.portListSelector.setMRMLScene(slicer.mrmlScene)
    self.portListSelector.setToolTip("Add surgical ports from a markups node.")
    portsFormLayout.addRow("Markups node of surgical ports", self.portListSelector)

    #
    # Add Port List button
    #
    self.addPortListButton = qt.QPushButton("Set Port Markups Node")
    self.addPortListButton.enabled = False
    portsFormLayout.addRow(self.addPortListButton)

    #
    # Port table
    #
    self.portsTable = qt.QTableView()
    self.portsTableModel = qt.QStandardItemModel()
    self.portsTable.setModel(self.portsTableModel)
    portsFormLayout.addRow(self.portsTable)

    #
    # Remove Port button
    #
    self.removePortButton = qt.QPushButton("Remove Selected Port")
    self.removePortButton.enabled = False
    portsFormLayout.addRow(self.removePortButton)

    #
    # Target area
    #
    targetCollapsibleButton = ctk.ctkCollapsibleButton()
    targetCollapsibleButton.text = "Surgical Target"
    self.layout.addWidget(targetCollapsibleButton)
    targetFormLayout = qt.QFormLayout(targetCollapsibleButton)

    #
    # target selector
    #
    self.targetSelector = slicer.qMRMLNodeComboBox()
    self.targetSelector.nodeTypes = ["vtkMRMLMarkupsFiducialNode"]
    self.targetSelector.addEnabled = False
    self.targetSelector.removeEnabled = False
    self.targetSelector.noneEnabled = True
    self.targetSelector.setMRMLScene(slicer.mrmlScene)
    self.targetSelector.setToolTip("Pick the surgical target that the tools should face.")
    targetFormLayout.addRow("Surgical Target Fiducial", self.targetSelector)

    #
    # Retarget button
    #
    self.retargetButton = qt.QPushButton("Aim Tools at Target")
    self.retargetButton.toolTip = "Reset tool orientations to face target fiducial"
    self.retargetButton.enabled = False
    targetFormLayout.addRow(self.retargetButton)

    #
    # Port Tool Display Options
    #
    toolsCollapsibleButton = ctk.ctkCollapsibleButton()
    toolsCollapsibleButton.text = "Port Tool Display Options"
    self.layout.addWidget(toolsCollapsibleButton)
    toolsFormLayout = qt.QFormLayout(toolsCollapsibleButton)

    #
    # Transform sliders
    #
    self.transformSliders = slicer.qMRMLTransformSliders()
    self.transformSliders.TypeOfTransform = slicer.qMRMLTransformSliders.ROTATION
    self.transformSliders.CoordinateReference = slicer.qMRMLTransformSliders.LOCAL
    self.transformSliders.Title = 'Tool Orientation'
    self.transformSliders.minMaxVisible = False
    toolsFormLayout.addRow(self.transformSliders)

    #
    # radius spin box
    #
    self.radiusSpinBox = qt.QDoubleSpinBox()
    self.radiusSpinBox.setMinimum(0.0)
    self.radiusSpinBox.setMaximum(10.0)
    self.radiusSpinBox.setValue(2.0)
    toolsFormLayout.addRow("Tool radius", self.radiusSpinBox)

    #
    # length spin box
    #
    self.lengthSpinBox = qt.QDoubleSpinBox()
    self.lengthSpinBox.setMinimum(0.0)
    self.lengthSpinBox.setMaximum(250.0)
    self.lengthSpinBox.setValue(150.0)
    toolsFormLayout.addRow("Tool length", self.lengthSpinBox)

    # connections
    self.portListSelector.connect('currentNodeChanged(bool)', self.onPortListSelectorChanged)
    self.targetSelector.connect('currentNodeChanged(bool)', self.onTargetSelectorChanged)
    self.addPortListButton.connect('clicked(bool)', self.onAddPortListButton)
    self.removePortButton.connect('clicked(bool)', self.onRemovePortButton)
    self.retargetButton.connect('clicked(bool)', self.onRetargetButton)
    self.radiusSpinBox.connect('valueChanged(double)', self.onToolShapeChanged)
    self.lengthSpinBox.connect('valueChanged(double)', self.onToolShapeChanged)

    # Add vertical spacer
    self.layout.addStretch(1)

    # Add observer to scene for removal of fiducial nodes
    self.sceneObserverTag = slicer.mrmlScene.AddObserver(slicer.mrmlScene.NodeRemovedEvent,
                                                         self.onNodeRemoved)

    # instantiate port placement module logic
    self.logic = PortPlacementLogic()

    self.currentMarkupsNode = None
    self.onMarkupAddedTag = None

  def __del__(self):
    slicer.mrmlScene.RemoveObserver(self.sceneObserverTag)
    if not (self.currentMarkupsNode is None):
      self.currentMarkupsNode.RemoveObserver(self.onMarkupAddedTag)

  def onPortListSelectorChanged(self, valid):
    self.addPortListButton.enabled = valid

  def onTargetSelectorChanged(self, valid):
    self.retargetButton.enabled = valid

  def onAddPortListButton(self):
    self.logic.setMarkupsNode(self.portListSelector.currentNode(),
                              self.radiusSpinBox.value,
                              self.lengthSpinBox.value)
    if not (self.currentMarkupsNode is None):
      self.currentMarkupsNode.RemoveObserver(self.onMarkupAddedTag)

    self.currentMarkupsNode = self.portListSelector.currentNode()
    self.onMarkupAddedTag = self.currentMarkupsNode.AddObserver(slicer.vtkMRMLMarkupsNode.PointAddedEvent, self.onMarkupAdded)

    self.updateTable()

  def onRemovePortButton(self):
    # Problem: currentIndex() can be valid even when there is no
    # visible selection. This will have to do for now while I figure
    # out how to making Python bindings to QList<QModelIndex> (ugh)
    index = self.portsTable.selectionModel().currentIndex
    self.logic.markupsNode.RemoveMarkup(index.row())
    self.updateTable()

  def onTableItemChanged(self,item):
    # Get markup index corresponding to the table item
    idx = self.itemPortIdxMap[item]

    # If the item is checkable, it's a visibility column. If not, it's
    # the name column
    if item.isCheckable():
      if item.checkState() == 2: # checked enum
        self.logic.makeToolVisible(idx)
      else:
        self.logic.makeToolInvisible(idx)

    else: # name column
      self.logic.markupsNode.SetNthFiducialLabel(idx, item.text())

  def onMarkupAdded(self, node, event):
    self.logic.onMarkupAdded(self.radiusSpinBox.value, self.lengthSpinBox.value)
    self.updateTable()

  def onNodeRemoved(self, scene, event):
    self.logic.onNodeRemoved()
    if not (slicer.mrmlScene.IsNodePresent(self.currentMarkupsNode)):
      self.currentMarkupsNode = None
    self.updateTable()

  def onRetargetButton(self):
    self.logic.retargetTools(self.targetSelector.currentNode())

  def onToolShapeChanged(self):
      toolIdx = self.portsTable.selectionModel().currentIndex.row()
      if toolIdx < self.portsTableModel.rowCount:
        self.logic.setToolShape(toolIdx, self.radiusSpinBox.value, self.lengthSpinBox.value)

  def onCurrentToolChanged(self, newIndex, prevIndex):
    self.removePortButton.enabled = True
    self.transformSliders.setMRMLTransformNode(self.logic.getToolTransform(newIndex.row()))
    self.radiusSpinBox.setValue(self.logic.getToolRadius(newIndex.row()))
    self.lengthSpinBox.setValue(self.logic.getToolLength(newIndex.row()))

  def updateTable(self):
    self.portsTableModel = qt.QStandardItemModel()
    self.portsTable.setModel(self.portsTableModel)
    self.transformSliders.reset()
    self.removePortButton.enabled = False
    self.transformSliders.setMRMLTransformNode(None)

    node = self.logic.markupsNode

    if node is None:
      return

    self.itemPortIdxMap = {}
    for i in range(node.GetNumberOfFiducials()):
      item = qt.QStandardItem()
      item.setText(self.logic.getPortName(i))
      self.portsTableModel.setItem(i, 0, item)
      self.itemPortIdxMap[item] = i

      item = qt.QStandardItem()
      item.setText('Visible')
      item.setCheckable(True)
      if self.logic.isToolVisible(i):
        checkState = 2 # checked enum
      else:
        checkState = 0 # unchecked enum
      item.setCheckState(checkState)
      self.portsTableModel.setItem(i, 1, item)
      self.itemPortIdxMap[item] = i
    self.portsTableModel.setHeaderData(0, 1, "Port Fiducial Name")
    self.portsTableModel.setHeaderData(1, 1, " ")
    self.portsTable.setColumnWidth(0, 15*len("Port Fiducial Name"))

    self.portsTableModel.itemChanged.connect(self.onTableItemChanged)
    self.portsTable.selectionModel().currentRowChanged.connect(self.onCurrentToolChanged)

  def onReload(self,moduleName="PortPlacement"):
    """Generic reload method for any scripted module.
    ModuleWizard will subsitute correct default moduleName.
    """
    import imp, sys, os, slicer

    widgetName = moduleName + "Widget"

    # reload the source code
    # - set source file path
    # - load the module to the global space
    filePath = eval('slicer.modules.%s.path' % moduleName.lower())
    p = os.path.dirname(filePath)
    if not sys.path.__contains__(p):
      sys.path.insert(0,p)
    fp = open(filePath, "r")
    globals()[moduleName] = imp.load_module(
        moduleName, fp, filePath, ('.py', 'r', imp.PY_SOURCE))
    fp.close()

    # rebuild the widget
    # - find and hide the existing widget
    # - create a new widget in the existing parent
    parent = slicer.util.findChildren(name='%s Reload' % moduleName)[0].parent().parent()
    for child in parent.children():
      try:
        child.hide()
      except AttributeError:
        pass
    # Remove spacer items
    item = parent.layout().itemAt(0)
    while item:
      parent.layout().removeItem(item)
      item = parent.layout().itemAt(0)
    # create new widget inside existing parent
    globals()[widgetName.lower()] = eval(
        'globals()["%s"].%s(parent)' % (moduleName, widgetName))
    globals()[widgetName.lower()].setup()

  def onReloadAndTest(self,moduleName="PortPlacement"):
    try:
      self.onReload()
      evalString = 'globals()["%s"].%sTest()' % (moduleName, moduleName)
      tester = eval(evalString)
      tester.runTest()
    except Exception as e:
      import traceback
      traceback.print_exc()
      qt.QMessageBox.warning(slicer.util.mainWindow(),
          "Reload and Test", 'Exception!\n\n' + str(e) + "\n\nSee Python Console for Stack Trace")


#
# PortPlacementLogic
#

class PortPlacementLogic:
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget
  """

  class Tool:

    def __init__(self, markupID, radius, length, position):
      # We use the markupID to check whether this tool's markup has
      # been removed in the onMarkupRemoved event
      self.markupID = markupID

      # Create a vtkCylinderSource for rendering the tool
      self.toolSrc = vtk.vtkCylinderSource()
      self.toolSrc.SetRadius(radius)
      self.toolSrc.SetHeight(length)

      # Create tool model using cylinder source
      self.modelNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLModelNode', slicer.mrmlScene.GenerateUniqueName("Tool"))
      #self.modelNode.HideFromEditorsOn()
      self.modelNode.SetSelectable(False) # Prevent picking on the tool model (endless loop until tip reaches camera)
      self.modelNode.SetPolyDataConnection(self.toolSrc.GetOutputPort())
      self.modelNode.CreateDefaultDisplayNodes()

      # Create a DisplayNode for this node (so that it can control its
      # own visibility) and have modelNode observe it
      self.modelDisplay = self.modelNode.GetDisplayNode()
      self.modelDisplay.SetColor(0,1,1) # cyan

      # Create a transform for our tool model that will scale it to
      # the proper radius, length, and position. Leave the orientation
      # at the default.
      self.transformNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLLinearTransformNode', slicer.mrmlScene.GenerateUniqueName("Transform"))
      #self.transformNode.HideFromEditorsOn()
      self.updatePosition(position)

      # apply the new transform to our tool
      self.modelNode.SetAndObserveTransformNodeID(self.transformNode.GetID())

      # Set tool to be visible
      self.visible = True

    def updateShape(self, radius, length):
      self.toolSrc.SetRadius(radius)
      self.toolSrc.SetHeight(length)
      self.toolSrc.Update()

    def updatePosition(self, p):
      mat = vtk.vtkMatrix4x4()
      self.transformNode.GetMatrixTransformToWorld(mat)
      currPos = [mat.GetElement(j, 3) for j in [0,1,2]]
      trans = [x - y for (x,y) in zip(p, currPos)]

      t = vtk.vtkTransform()
      t.Translate(trans)

      self.transformNode.ApplyTransformMatrix(t.GetMatrix())

    def makeVisible(self):
      self.modelDisplay.VisibilityOn()
      self.visible = True

    def makeInvisible(self):
      self.modelDisplay.VisibilityOff()
      self.visible = False

    def __del__(self):
      slicer.mrmlScene.RemoveNode(self.modelNode)

      # This line causes a "node already removed" error. Weird...
      # slicer.mrmlScene.RemoveNode(self.modelDisplay)

      slicer.mrmlScene.RemoveNode(self.transformNode)

    # End Tool


  def __init__(self):
    self.markupsNode = None
    self.toolList = []

  def __del__(self):
    if not (self.markupsNode is None):
      self.markupsNode.RemoveObserver(self.nodeObserverTag)

  # CAREFUL: the order of these operations is important
  def clearMarkupsNode(self):
    # If we were observing another node, remove first remove ourself
    # as an observer
    if not (self.markupsNode is None):
      self.markupsNode.RemoveObserver(self.pointModifiedObserverTag)
      self.markupsNode.RemoveObserver(self.markupRemovedObserverTag)

    self.markupsNode = None

    # Clear the current tool list
    self.toolList = []

  def setMarkupsNode(self, node, radius, length):
    self.clearMarkupsNode()

    self.markupsNode = node
    if node:
      self.pointModifiedObserverTag = node.AddObserver(slicer.vtkMRMLMarkupsNode.PointModifiedEvent, self.onMarkupModified)
      self.markupRemovedObserverTag = node.AddObserver(slicer.vtkMRMLMarkupsNode.PointRemovedEvent, self.onMarkupRemoved)

      # Add tools associated with the markups in markupsNode
      for i in range(node.GetNumberOfFiducials()):
        p = [0,0,0]
        node.GetNthFiducialPosition(i, p)
        self.toolList.append(self.Tool(node.GetNthMarkupID(i), radius, length, p))

  def setToolShape(self, idx, radius, length):
    self.toolList[idx].updateShape(radius, length)

  def isToolVisible(self, idx):
    return self.toolList[idx].visible

  def makeToolVisible(self, idx):
    self.toolList[idx].makeVisible()

  def makeToolInvisible(self, idx):
    self.toolList[idx].makeInvisible()

  def getToolTransform(self, idx):
    return self.toolList[idx].transformNode

  def getToolRadius(self, idx):
    return self.toolList[idx].toolSrc.GetRadius()

  def getToolLength(self, idx):
    return self.toolList[idx].toolSrc.GetHeight()

  def getPortName(self, idx):
    return self.markupsNode.GetNthFiducialLabel(idx)

  def onMarkupAdded(self, radius, length):
    newMarkupIdx = self.markupsNode.GetNumberOfFiducials() - 1
    p = [0,0,0]
    self.markupsNode.GetNthFiducialPosition(newMarkupIdx, p)
    self.toolList.append(self.Tool(self.markupsNode.GetNthMarkupID(newMarkupIdx), radius, length, p))

  # If the node we're observing has been modified, just iterate
  # through and update all the tools since for now we don't know how
  # to distinguish which markup got modified.
  def onMarkupModified(self, node, event):
    for i in range(node.GetNumberOfFiducials()):
      p = [0,0,0]
      node.GetNthFiducialPosition(i, p)
      self.toolList[i].updatePosition(p)

  # If the markupsNode this logic is associated with was deleted,
  # clear the tools
  def onNodeRemoved(self):
    if not self.markupsNode:
      return

    if not slicer.mrmlScene.IsNodePresent(self.markupsNode):
      self.clearMarkupsNode()

  # If a markup has been removed from the markupsNode we're associated
  # with, remove the corresponding tool
  def onMarkupRemoved(self, node, event):
    if self.markupsNode is None:
      return

    markupsSet = set()
    for i in range(self.markupsNode.GetNumberOfFiducials()):
      markupsSet.add(self.markupsNode.GetNthMarkupID(i))

    prevLen = len(self.toolList)
    for i in range(len(self.toolList)):
      if not (self.toolList[i].markupID in markupsSet):
        del self.toolList[i]
        break
    if prevLen == len(self.toolList):
      print("One of our nodes wasn't removed...")

  # Target all tools toward the targetFid
  def retargetTools(self, targetFid):
    if (self.markupsNode is None or
       targetFid.GetNumberOfFiducials() < 1 or
       targetFid == self.markupsNode):
      return

    import numpy
    import numpy.linalg
    import math
    # Get target coordinates
    targetLoc = [0,0,0]
    targetFid.GetNthFiducialPosition(0, targetLoc)
    targetLoc = numpy.array(targetLoc)

    # Iterate through port tool map and retarget their associated tools
    for i in range(self.markupsNode.GetNumberOfFiducials()):
      portLocList = [0,0,0]
      self.markupsNode.GetNthFiducialPosition(i, portLocList)
      portLoc = numpy.array(portLocList)

      # Assume tools get drawn aligned with the global y-axis. We
      # want to transform the tool to be oriented along the port
      # toward the target point. We begin by finding the axis to
      # rotate the tool by, which is the cross product of the
      # global y and the target vector
      targetVec = targetLoc - portLoc

      if numpy.linalg.norm(targetVec) <= 0.0000001:
        continue

      targetVec = targetVec / numpy.linalg.norm(targetVec)

      mat = vtk.vtkMatrix4x4()
      self.toolList[i].transformNode.GetMatrixTransformToWorld(mat)
      currentVec = numpy.array([mat.GetElement(j, 1) for j in [0,1,2]])

      rotAxis = numpy.cross(currentVec, targetVec)
      normRotAxis = numpy.linalg.norm(rotAxis)
      if normRotAxis <= 0.000001:
        continue

      # get rotation angle and normalize rotation axis
      rotAxis = rotAxis / numpy.linalg.norm(rotAxis)
      angle = math.acos(numpy.dot(currentVec, targetVec)) * 180. / math.pi

      # generate our transform
      t = vtk.vtkTransform()
      trans = [mat.GetElement(j, 3) for j in [0,1,2]]
      negTrans = [-x for x in trans]

      t.Translate(trans)
      t.RotateWXYZ(angle, rotAxis.tolist())
      t.Translate(negTrans)

      self.toolList[i].transformNode.ApplyTransformMatrix(t.GetMatrix())


class PortPlacementTest(unittest.TestCase):
  """
  This is the test case for your scripted module.
  """

  def delayDisplay(self,message,msec=1000):
    """This utility method displays a small dialog and waits.
    This does two things: 1) it lets the event loop catch up
    to the state of the test so that rendering and widget updates
    have all taken place before the test continues and 2) it
    shows the user/developer/tester the state of the test
    so that we'll know when it breaks.
    """
    print(message)
    self.info = qt.QDialog()
    self.infoLayout = qt.QVBoxLayout()
    self.info.setLayout(self.infoLayout)
    self.label = qt.QLabel(message,self.info)
    self.infoLayout.addWidget(self.label)
    qt.QTimer.singleShot(msec, self.info.close)
    self.info.exec_()

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    slicer.mrmlScene.Clear(0)

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_PortPlacement1()

  def test_PortPlacement1(self):
    import numpy
    import numpy.linalg
    import random

    self.delayDisplay("Starting test...")

    m = slicer.util.mainWindow()
    m.moduleSelector().selectModule('PortPlacement')
    self.widget = slicer.modules.PortPlacementWidget

    logic = self.widget.logic
    markupsLogic = slicer.modules.markups.logic()

    # check init
    self.assertTrue(self.widget.portsTableModel.rowCount() == 0)
    self.assertTrue(not self.widget.addPortListButton.enabled)

    # add a list of ports
    nodeID = markupsLogic.AddNewFiducialNode()
    fidNode = slicer.mrmlScene.GetNodeByID(nodeID)
    numPorts = 4
    for i in range(numPorts):
      fidNode.AddFiducial(*[random.uniform(-100.,100.) for j in range(3)])
    self.widget.portListSelector.setCurrentNode(fidNode)
    self.assertTrue(self.widget.addPortListButton.enabled)
    self.widget.onAddPortListButton()

    # check tool transforms
    self.assertTrue(len(logic.toolList) == numPorts)
    for i in range(numPorts):
      p = [0,0,0]
      fidNode.GetNthFiducialPosition(i, p)
      tool_mat = vtk.vtkMatrix4x4()
      logic.toolList[i].transformNode.GetMatrixTransformToWorld(tool_mat)
      tool_pos = [tool_mat.GetElement(j, 3) for j in [0,1,2]]
      diff = numpy.array(p) - numpy.array(tool_pos)
      self.assertTrue(numpy.dot(diff,diff) < 1e-10)


    # check table
    self.assertTrue(self.widget.portsTableModel.rowCount() == numPorts)
    for i in range(numPorts):
      self.assertTrue(self.widget.portsTableModel.item(i).text() == fidNode.GetNthMarkupLabel(i))

    # check removal
    self.assertTrue(not self.widget.removePortButton.enabled)
    index = self.widget.portsTableModel.index(0,0)
    self.widget.portsTable.selectionModel().setCurrentIndex(index, qt.QItemSelectionModel.Current)
    self.assertTrue(self.widget.removePortButton.enabled)
    self.widget.onRemovePortButton()
    self.assertTrue(self.widget.portsTableModel.rowCount() == numPorts - 1)
    self.assertTrue(len(logic.toolList) == numPorts - 1)
    self.assertTrue(fidNode.GetNumberOfFiducials() == numPorts - 1)

    # check add
    fidNode.AddFiducial(*[random.uniform(-100.,100.) for i in range(3)])
    self.assertTrue(len(logic.toolList) == numPorts)
    self.assertTrue(self.widget.portsTableModel.rowCount() == numPorts)

    # check retarget
    self.assertTrue(not self.widget.retargetButton.enabled)
    targetNodeID = markupsLogic.AddNewFiducialNode()
    targetNode = slicer.mrmlScene.GetNodeByID(targetNodeID)
    targetNode.AddFiducial(*[random.uniform(-100.,100.) for i in range(3)])
    self.widget.targetSelector.setCurrentNode(targetNode)
    self.assertTrue(self.widget.retargetButton.enabled)
    self.widget.onRetargetButton()

    target_p = [0,0,0]
    targetNode.GetNthFiducialPosition(0, target_p)
    targetWorld = target_p

    # check retargeting by verifying that tools' positions are
    # unchanged and that their y-axes are oriented toward point
    for i in range(numPorts):
      p = [0,0,0]
      fidNode.GetNthFiducialPosition(i, p)
      tool_mat = vtk.vtkMatrix4x4()
      logic.toolList[i].transformNode.GetMatrixTransformToWorld(tool_mat)
      tool_pos = [tool_mat.GetElement(j, 3) for j in [0,1,2]]
      diff = numpy.array(p) - numpy.array(tool_pos)
      self.assertTrue(numpy.dot(diff,diff) < 1e-10)

      targetLocal = [0,0,0]
      logic.toolList[i].modelNode.TransformPointFromWorld(targetWorld, targetLocal)

      targetLocal = numpy.array(targetLocal)[0:3]
      targetLocal = targetLocal / numpy.linalg.norm(targetLocal)

      # target local should be the unit y-axis (e2)
      yAxis = numpy.array([0.,1.,0.])
      diff = yAxis - targetLocal

      self.assertTrue(numpy.dot(diff,diff) < 1e-10)

    # test fidNode deletion
    slicer.mrmlScene.RemoveNode(fidNode)
    self.assertTrue(len(logic.toolList) == 0)
    self.assertTrue(self.widget.portsTableModel.rowCount() == 0)

    self.delayDisplay("Test passed!")
