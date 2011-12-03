// This file is part of Agros2D.
//
// Agros2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros2D.  If not, see <http://www.gnu.org/licenses/>.
//
// hp-FEM group (http://hpfem.org/)
// University of Nevada, Reno (UNR) and University of West Bohemia, Pilsen
// Email: agros2d@googlegroups.com, home page: http://hpfem.org/agros2d/

#include "scene.h"
#include "sceneview.h"

#include "util.h"
#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "scenemarkerdialog.h"
#include "scenefunction.h"
#include "scenesolution.h"

#include "problemdialog.h"
#include "scenetransformdialog.h"
#include "scenemarker.h"
#include "scenemarkerselectdialog.h"
#include "scenebasicselectdialog.h"
#include "progressdialog.h"

#include "scripteditordialog.h"

#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"

void ProblemInfo::clear()
{
    coordinateType = CoordinateType_Planar;
    name = QObject::tr("unnamed");
    date = QDate::currentDate();
    fileName = "";
    scriptStartup = "";
    description = "";

    // matrix solver
    matrixSolver = Hermes::SOLVER_UMFPACK;

    // harmonic
    frequency = 0.0;

    // transient
    timeStep = Value("1.0", false);
    timeTotal = Value("1.0", false);
}

FieldInfo::FieldInfo(ProblemInfo* parent, QString fieldId)
{
    m_module = NULL;
    m_parent = parent;

    if (fieldId.isEmpty())
    {
        // default
        // read default field (Util::config() is not set)
        QSettings settings;
        m_fieldId = settings.value("General/DefaultPhysicField", "electrostatic").toString();

        bool check = false;
        std::map<std::string, std::string> modules = availableModules();
        for (std::map<std::string, std::string>::iterator it = modules.begin(); it != modules.end(); ++it)
            if (m_fieldId.toStdString() == it->first)
            {
                check = true;
                break;
            }
        if (!check)
            m_fieldId = "electrostatic";
    }
    else
    {
        m_fieldId = fieldId;
    }

    clear();
}

FieldInfo::~FieldInfo()
{
    if (m_module) delete m_module;
}

void FieldInfo::clear()
{
    // module object
    setAnalysisType(AnalysisType_SteadyState);

    numberOfRefinements = 1;
    polynomialOrder = 2;
    meshType = MeshType_Triangle;
    adaptivityType = AdaptivityType_None;
    adaptivitySteps = 0;
    adaptivityTolerance = 1.0;

    initialCondition = Value("0.0", false);

    // weakforms
    weakFormsType = WeakFormsType_Compiled;

    // linearity
    linearityType = LinearityType_Linear;
    nonlinearTolerance = 1e-3;
    nonlinearSteps = 10;
}

void FieldInfo::setAnalysisType(AnalysisType analysisType)
{
    m_analysisType = analysisType;

    if (m_module) delete m_module;
    m_module = moduleFactory(m_fieldId.toStdString(),
                             coordinateType(),
                             m_analysisType);
}

DxfFilter::DxfFilter(Scene *scene)
{
    logMessage("DxfFilter::DxfFilter()");

    this->m_scene = scene;
}

void DxfFilter::addLine(const DL_LineData &l)
{
    logMessage("DxfFilter::addLine()");

    // start node
    SceneNode *nodeStart = m_scene->addNode(new SceneNode(Point(l.x1, l.y1)));
    // end node
    SceneNode *nodeEnd = m_scene->addNode(new SceneNode(Point(l.x2, l.y2)));

    // edge
    m_scene->addEdge(new SceneEdge(nodeStart, nodeEnd,  0, 0)); //TODO - do it better
}

void DxfFilter::addArc(const DL_ArcData& a)
{
    logMessage("DxfFilter::addArc()");

    double angle1 = a.angle1;
    double angle2 = a.angle2;

    while (angle1 < 0.0) angle1 += 360.0;
    while (angle1 >= 360.0) angle1 -= 360.0;
    while (angle2 < 0.0) angle2 += 360.0;
    while (angle2 >= 360.0) angle2 -= 360.0;

    // start node
    SceneNode *nodeStart = m_scene->addNode(new SceneNode(Point(a.cx + a.radius*cos(angle1/180*M_PI), a.cy + a.radius*sin(angle1/180*M_PI))));
    // end node
    SceneNode *nodeEnd = m_scene->addNode(new SceneNode(Point(a.cx + a.radius*cos(angle2/180*M_PI), a.cy + a.radius*sin(angle2/180*M_PI))));

    // edge
    m_scene->addEdge(new SceneEdge(nodeStart, nodeEnd, (angle1 < angle2) ? angle2-angle1 : angle2+360.0-angle1, 0)); //TODO - do it better
}

void DxfFilter::addCircle(const DL_CircleData& c)
{
    logMessage("DxfFilter::addCircle()");

    // nodes
    SceneNode *node1 = m_scene->addNode(new SceneNode(Point(c.cx + c.radius, c.cy)));
    SceneNode *node2 = m_scene->addNode(new SceneNode(Point(c.cx, c.cy + c.radius)));
    SceneNode *node3 = m_scene->addNode(new SceneNode(Point(c.cx - c.radius, c.cy)));
    SceneNode *node4 = m_scene->addNode(new SceneNode(Point(c.cx, c.cy - c.radius)));

    // edges
    m_scene->addEdge(new SceneEdge(node1, node2, 90, 0)); // do it better
    m_scene->addEdge(new SceneEdge(node2, node3, 90, 0));
    m_scene->addEdge(new SceneEdge(node3, node4, 90, 0));
    m_scene->addEdge(new SceneEdge(node4, node1, 90, 0));
}

// ************************************************************************************************************************

// initialize pointer
Util *Util::m_singleton = NULL;

Util::Util()
{
    logMessage("Util::Util()");

    m_scene = new Scene();

    // script remote
    m_scriptEngineRemote = new ScriptEngineRemote();

    // completer
    m_completer = new QCompleter();
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setCompletionMode(QCompleter::InlineCompletion);
    m_completer->setModelSorting(QCompleter::UnsortedModel);
    m_completer->setModel(new QStringListModel());

    QSettings settings;
    QStringList list = settings.value("CommandDialog/RecentCommands").value<QStringList>();
    QStringListModel *model = dynamic_cast<QStringListModel *>(m_completer->model());
    model->setStringList(list);

    // config
    m_config = new Config();
    m_config->load();

    initLists();
}

Util::~Util()
{
    logMessage("Util::~Util()");

    delete m_scene;
    delete m_completer;
    delete m_config;
    delete m_scriptEngineRemote;
}

void Util::createSingleton()
{
    m_singleton = new Util();
}

Util *Util::singleton()
{
    return m_singleton;
}

// ************************************************************************************************************************

Scene::Scene()
{
    logMessage("Scene::Scene()");

    createActions();

    m_problemInfo = new ProblemInfo();
    m_undoStack = new QUndoStack(this);
    m_sceneSolution = new SceneSolution<double>(); //TODO PK <double>

    connect(this, SIGNAL(invalidated()), this, SLOT(doInvalidated()));
    connect(m_sceneSolution, SIGNAL(solved()), this, SLOT(doInvalidated()));

    boundaries = new SceneBoundaryContainer();
    materials = new SceneMaterialContainer();

    nodes = new SceneNodeContainer();
    edges = new SceneEdgeContainer();
    labels = new SceneLabelContainer();

    clear();
}

Scene::~Scene()
{
    logMessage("Scene::~Scene()");

    clear();

    delete m_sceneSolution;
    delete m_undoStack;

    // TODO write destructors or use shared_ptrs...
    delete boundaries;
    delete materials;
    delete nodes;
    delete edges;
    delete labels;
}

void Scene::createActions()
{
    logMessage("Scene::createActions()");

    // scene - add items
    actNewNode = new QAction(icon("scene-node"), tr("New &node..."), this);
    actNewNode->setShortcut(tr("Alt+N"));
    actNewNode->setStatusTip(tr("New node"));
    connect(actNewNode, SIGNAL(triggered()), this, SLOT(doNewNode()));

    actNewEdge = new QAction(icon("scene-edge"), tr("New &edge..."), this);
    actNewEdge->setShortcut(tr("Alt+E"));
    actNewEdge->setStatusTip(tr("New edge"));
    connect(actNewEdge, SIGNAL(triggered()), this, SLOT(doNewEdge()));

    actNewLabel = new QAction(icon("scene-label"), tr("New &label..."), this);
    actNewLabel->setShortcut(tr("Alt+L"));
    actNewLabel->setStatusTip(tr("New label"));
    connect(actNewLabel, SIGNAL(triggered()), this, SLOT(doNewLabel()));

    actDeleteSelected = new QAction(icon("edit-delete"), tr("Delete selected objects"), this);
    actDeleteSelected->setStatusTip(tr("Delete selected objects"));
    connect(actDeleteSelected, SIGNAL(triggered()), this, SLOT(doDeleteSelected()));

    actNewBoundary = new QAction(icon("scene-edgemarker"), tr("New &boundary condition..."), this);
    actNewBoundary->setShortcut(tr("Alt+B"));
    actNewBoundary->setStatusTip(tr("New boundary condition"));
    connect(actNewBoundary, SIGNAL(triggered()), this, SLOT(doNewBoundary()));

    actNewMaterial = new QAction(icon("scene-labelmarker"), tr("New &material..."), this);
    actNewMaterial->setShortcut(tr("Alt+M"));
    actNewMaterial->setStatusTip(tr("New material"));
    connect(actNewMaterial, SIGNAL(triggered()), this, SLOT(doNewMaterial()));

    actTransform = new QAction(icon("scene-transform"), tr("&Transform"), this);
    actTransform->setStatusTip(tr("Transform"));
    connect(actTransform, SIGNAL(triggered()), this, SLOT(doTransform()));

    actClearSolution = new QAction(icon(""), tr("Clear solution"), this);
    actClearSolution->setStatusTip(tr("Clear solution"));
    connect(actClearSolution, SIGNAL(triggered()), this, SLOT(doClearSolution()));

    actProblemProperties = new QAction(icon("document-properties"), tr("&Problem properties"), this);
    actProblemProperties->setShortcut(tr("F12"));
    actProblemProperties->setStatusTip(tr("Problem properties"));
    connect(actProblemProperties, SIGNAL(triggered()), this, SLOT(doProblemProperties()));
}

SceneNode *Scene::addNode(SceneNode *node)
{
    logMessage("SceneNode *Scene::addNode()");

    // clear solution
    m_sceneSolution->clear();

    // check if node doesn't exists
    if(SceneNode* existing = nodes->get(node)){
        delete node;
        return existing;
    }

    nodes->add(node);
    if (!scriptIsRunning()) emit invalidated();

    return node;
}

void Scene::removeNode(SceneNode *node)
{
    logMessage("Scene::nodes->remove()");

    // clear solution
    m_sceneSolution->clear();

    // remove all edges connected to this node
    edges->removeConnectedToNode(node);

    nodes->remove(node);
    // delete node;

    emit invalidated();
}

SceneNode *Scene::getNode(const Point &point)
{
    logMessage("SceneNode *Scene::getNode()");

    nodes->get(point);
}


SceneEdge *Scene::addEdge(SceneEdge *edge)
{
    logMessage("SceneEdge *Scene::addEdge");

    // clear solution
    m_sceneSolution->clear();

    // check if edge doesn't exists
    if(SceneEdge* existing = edges->get(edge)){
        delete edge;
        return existing;
    }

    edges->add(edge);
    if (!scriptIsRunning()) emit invalidated();

    return edge;
}

void Scene::removeEdge(SceneEdge *edge)
{
    logMessage("Scene::edges->remove()");

    // clear solution
    m_sceneSolution->clear();

    edges->remove(edge);
    // delete edge;

    emit invalidated();
}

SceneEdge *Scene::getEdge(const Point &pointStart, const Point &pointEnd, double angle)
{
    logMessage("SceneEdge *Scene::getEdge()");

    edges->get(pointStart, pointEnd, angle);
}

SceneLabel *Scene::addLabel(SceneLabel *label)
{
    logMessage("SceneLabel *Scene::addLabel");

    // clear solution
    m_sceneSolution->clear();

    // check if label doesn't exists
    if(SceneLabel* existing = labels->get(label)){
        delete label;
        return existing;
    }

    labels->add(label);
    if (!scriptIsRunning()) emit invalidated();

    return label;
}

void Scene::removeLabel(SceneLabel *label)
{
    logMessage("Scene::label->remove()");

    // clear solution
    m_sceneSolution->clear();

    labels->remove(label);
    // delete label;

    emit invalidated();
}

SceneLabel *Scene::getLabel(const Point &point)
{
    logMessage("SceneLabel *Scene::getLabel()");

    labels->get(point);
}

void Scene::addBoundary(SceneBoundary *boundary)
{
    logMessage("Scene::addBoundary()");

    boundaries->add(boundary);
    if (!scriptIsRunning()) emit invalidated();
}

void Scene::removeBoundary(SceneBoundary *boundary)
{
    logMessage("Scene::removeBoundary()");

    //TODO instead of setting NoneBoundary we now remove... rething
    edges->removeMarkerFromAll(boundary);
    boundaries->remove(boundary);
    // delete boundary;

    emit invalidated();
}

void Scene::setBoundary(SceneBoundary *boundary)
{
    logMessage("setEdgeBoundary()");

    edges->selected().addMarkerToAll(boundary);
    selectNone();
}

SceneBoundary *Scene::getBoundary(const QString &name)
{
    logMessage("SceneBoundary *Scene::getBoundary()");

    return boundaries->get(name);
}

void Scene::addMaterial(SceneMaterial *material)
{
    logMessage("Scene::addMaterial()");

    this->materials->add(material);
    if (!scriptIsRunning()) emit invalidated();
}


SceneMaterial *Scene::getMaterial(const QString &name)
{
    logMessage("Scene::SceneMaterial *Scene::getMaterial()");

    return materials->get(name);
}

void Scene::removeMaterial(SceneMaterial *material)
{
    logMessage("Scene::removeMaterial()");

    //TODO instead of setting NoneMaterial we now remove... rething
    labels->removeMarkerFromAll(material);
    this->materials->remove(material);
    // delete material;

    emit invalidated();
}

void Scene::setMaterial(SceneMaterial *material)
{
    logMessage("Scene::setLabelMaterial()");

    labels->selected().addMarkerToAll(material);
    selectNone();
}


void Scene::clear()
{
    logMessage("Scene::clear()");

    blockSignals(true);

    m_undoStack->clear();

    m_sceneSolution->clear();
    m_problemInfo->clear();

    QMapIterator<QString, FieldInfo *> i(m_fieldInfos);
    while (i.hasNext())
    {
        i.next();
        delete i.value();
    }
    m_fieldInfos.clear();
    //TODO - allow "no field"
    addField(new FieldInfo(m_problemInfo));

    // geometry
    nodes->clear();
    edges->clear();
    labels->clear();

    // markers
    boundaries->clear();
    materials->clear();

    // none edge
    addBoundary(new SceneBoundaryNone());
    // none label
    addMaterial(new SceneMaterialNone());

    blockSignals(false);

    emit fileNameChanged(tr("unnamed"));
    emit invalidated();
}

RectPoint Scene::boundingBox() const
{
    logMessage("RectPoint Scene::boundingBox()");

    if (nodes->isEmpty())
    {
        return RectPoint(Point(-0.5, -0.5), Point(0.5, 0.5));
    }
    else
    {
        return nodes->boundingBox();
    }
}

void Scene::selectNone()
{
    logMessage("Scene::selectNone()");

    nodes->setSelected(false);
    edges->setSelected(false);
    labels->setSelected(false);
}

void Scene::selectAll(SceneMode sceneMode)
{
    logMessage("Scene::selectAll()");

    selectNone();

    switch (sceneMode)
    {
    case SceneMode_OperateOnNodes:
        nodes->setSelected();
        break;
    case SceneMode_OperateOnEdges:
        edges->setSelected();
        break;
    case SceneMode_OperateOnLabels:
        labels->setSelected();
        break;
    }
}

void Scene::deleteSelected()
{
    logMessage("Scene::deleteSelected()");

    m_undoStack->beginMacro(tr("Delete selected"));

    nodes->selected().deleteWithUndo(tr("Remove node"));
    edges->selected().deleteWithUndo(tr("Remove edge"));
    labels->selected().deleteWithUndo(tr("Remove label"));

    m_undoStack->endMacro();

    emit invalidated();
}

int Scene::selectedCount()
{
    logMessage("Scene::selectedCount()");

    return nodes->selected().length() +
            edges->selected().length() +
            labels->selected().length();
}

void Scene::highlightNone()
{
    logMessage("Scene::highlightNone()");

    nodes->setHighlighted(false);
    edges->setHighlighted(false);
    labels->setHighlighted(false);
}

void Scene::transformTranslate(const Point &point, bool copy)
{
    assert(0); //TODO
    //    logMessage("Scene::transformTranslate()");

    //    // clear solution
    //    m_sceneSolution->clear();

    //    m_undoStack->beginMacro(tr("Translation"));

    //    foreach (SceneEdge *edge, edges->items())
    //    {
    //        if (edge->isSelected)
    //        {
    //            edge->nodeStart->isSelected = true;
    //            edge->nodeEnd->isSelected = true;
    //        }
    //    }

    //    foreach (SceneNode *node, nodes->items())
    //    {
    //        if (node->isSelected)
    //        {
    //            Point pointNew = node->point + point;
    //            if (!copy)
    //            {
    //                if (!getNode(pointNew))
    //                {
    //                    m_undoStack->push(new SceneNodeCommandEdit(node->point, pointNew));
    //                    node->point = pointNew;
    //                }
    //            }
    //            else
    //            {
    //                SceneNode *nodeNew = new SceneNode(pointNew);
    //                SceneNode *nodeAdded = addNode(nodeNew);
    //                if (nodeAdded == nodeNew) m_undoStack->push(new SceneNodeCommandAdd(nodeNew->point));
    //            }
    //        }
    //    }

    //    foreach (SceneLabel *label, labels->items())
    //    {
    //        if (label->isSelected)
    //        {
    //            Point pointNew = label->point + point;
    //            if (!copy)
    //            {
    //                if (!getLabel(pointNew))
    //                {
    //                    m_undoStack->push(new SceneLabelCommandEdit(label->point, pointNew));
    //                    label->point = pointNew;
    //                }
    //            }
    //            else
    //            {
    //                SceneLabel *labelNew = new SceneLabel(pointNew, label->area, label->polynomialOrder);
    //                SceneLabel *labelAdded = addLabel(labelNew);
    //                if (labelAdded == labelNew) m_undoStack->push(new SceneLabelCommandAdd(labelNew->point, QString::fromStdString(labelNew->marker->getName()), labelNew->area, label->polynomialOrder));
    //            }
    //        }
    //    }

    //    m_undoStack->endMacro();

    //    emit invalidated();
}

void Scene::transformRotate(const Point &point, double angle, bool copy)
{
    assert(0); //TODO
    //    logMessage("Scene::transformRotate()");

    //    // clear solution
    //    m_sceneSolution->clear();

    //    m_undoStack->beginMacro(tr("Rotation"));

    //    foreach (SceneEdge *edge, edges->items())
    //    {
    //        if (edge->isSelected)
    //        {
    //            edge->nodeStart->isSelected = true;
    //            edge->nodeEnd->isSelected = true;
    //        }
    //    }

    //    foreach (SceneNode *node, nodes->items())
    //        if (node->isSelected)
    //        {
    //            double distanceNode = (node->point - point).magnitude();
    //            double angleNode = (node->point - point).angle()/M_PI*180;

    //            Point pointNew = point + Point(distanceNode * cos((angleNode + angle)/180.0*M_PI), distanceNode * sin((angleNode + angle)/180.0*M_PI));
    //            if (!copy)
    //            {
    //                node->point = pointNew;
    //            }
    //            else
    //            {
    //                SceneNode *nodeNew = new SceneNode(pointNew);
    //                SceneNode *nodeAdded = addNode(nodeNew);
    //                if (nodeAdded == nodeNew) m_undoStack->push(new SceneNodeCommandAdd(nodeNew->point));
    //            }
    //        }

    //    foreach (SceneLabel *label, labels->items())
    //        if (label->isSelected)
    //        {
    //            double distanceNode = (label->point - point).magnitude();
    //            double angleNode = (label->point - point).angle()/M_PI*180;

    //            Point pointNew = point + Point(distanceNode * cos((angleNode + angle)/180.0*M_PI), distanceNode * sin((angleNode + angle)/180.0*M_PI));
    //            if (!copy)
    //            {
    //                label->point = pointNew;
    //            }
    //            else
    //            {
    //                SceneLabel *labelNew = new SceneLabel(pointNew, label->marker, label->area, label->polynomialOrder);
    //                SceneLabel *labelAdded = addLabel(labelNew);
    //                if (labelAdded == labelNew) m_undoStack->push(new SceneLabelCommandAdd(labelNew->point, QString::fromStdString(labelNew->marker->getName()), labelNew->area, labelNew->polynomialOrder));
    //            }
    //        }

    //    m_undoStack->endMacro();

    //    emit invalidated();
}

void Scene::transformScale(const Point &point, double scaleFactor, bool copy)
{
    assert(0); //TODO
    //    logMessage("Scene::transformScale()");

    //    // clear solution
    //    m_sceneSolution->clear();

    //    m_undoStack->beginMacro(tr("Scale"));

    //    foreach (SceneEdge *edge, edges->items())
    //    {
    //        if (edge->isSelected)
    //        {
    //            edge->nodeStart->isSelected = true;
    //            edge->nodeEnd->isSelected = true;
    //        }
    //    }

    //    foreach (SceneNode *node, nodes->items())
    //        if (node->isSelected)
    //        {
    //            Point pointNew = point + (node->point - point) * scaleFactor;
    //            if (!copy)
    //            {
    //                node->point = pointNew;
    //            }
    //            else
    //            {
    //                SceneNode *nodeNew = new SceneNode(pointNew);
    //                SceneNode *nodeAdded = addNode(nodeNew);
    //                if (nodeAdded == nodeNew) m_undoStack->push(new SceneNodeCommandAdd(nodeNew->point));
    //            }
    //        }

    //    foreach (SceneLabel *label, labels->items())
    //        if (label->isSelected)
    //        {
    //            Point pointNew = point + (label->point - point) * scaleFactor;
    //            if (!copy)
    //            {
    //                label->point = pointNew;
    //            }
    //            else
    //            {
    //                SceneLabel *labelNew = new SceneLabel(pointNew, label->marker, label->area, label->polynomialOrder);
    //                SceneLabel *labelAdded = addLabel(labelNew);
    //                if (labelAdded == labelNew) m_undoStack->push(new SceneLabelCommandAdd(labelNew->point, QString::fromStdString(labelNew->marker->getName()), labelNew->area, labelNew->polynomialOrder));
    //            }
    //        }

    //    m_undoStack->endMacro();

    //    emit invalidated();
}

void Scene::doInvalidated()
{
    logMessage("Scene::doInvalidated()");

    actNewEdge->setEnabled((nodes->length() >= 2) && (boundaries->length() >= 1));
    actNewLabel->setEnabled(materials->length() >= 1);
    actClearSolution->setEnabled(m_sceneSolution->isSolved());
}

void Scene::doNewNode(const Point &point)
{
    logMessage("Scene::doNewNode()");

    SceneNode *node = new SceneNode(point);
    if (node->showDialog(QApplication::activeWindow(), true) == QDialog::Accepted)
    {
        SceneNode *nodeAdded = addNode(node);
        if (nodeAdded == node) m_undoStack->push(new SceneNodeCommandAdd(node->point));
    }
    else
        delete node;
}

void Scene::doNewEdge()
{
    logMessage("Scene::doNewEdge()");

    SceneEdge *edge = new SceneEdge(nodes->at(0), nodes->at(1), 0, 0); //TODO - do it better
    if (edge->showDialog(QApplication::activeWindow(), true) == QDialog::Accepted)
    {
        SceneEdge *edgeAdded = addEdge(edge);
        if (edgeAdded == edge)
            m_undoStack->push(new SceneEdgeCommandAdd(edge->nodeStart->point,
                                                      edge->nodeEnd->point,
                                                      "TODO",
                                                      edge->angle,
                                                      edge->refineTowardsEdge));
    }
    else
        delete edge;
}

void Scene::doNewLabel(const Point &point)
{
    logMessage("Scene::doNewLabel()");

    SceneLabel *label = new SceneLabel(point, 0.0, 0); //TODO - do it better
    if (label->showDialog(QApplication::activeWindow(), true) == QDialog::Accepted)
    {
        SceneLabel *labelAdded = addLabel(label);
        if (labelAdded == label) m_undoStack->push(new SceneLabelCommandAdd(label->point,
                                                                            "TODO",
                                                                            label->area,
                                                                            label->polynomialOrder));
    }
    else
        delete label;
}

void Scene::doDeleteSelected()
{
    logMessage("doDeleteSelected()");

    deleteSelected();
}

void Scene::doNewBoundary()
{
    assert(0);
    logMessage("Scene::doNewBoundary()");

    SceneBoundary *marker = Util::scene()->fieldInfo("TODO")->module()->newBoundary();

    if (marker->showDialog(QApplication::activeWindow()) == QDialog::Accepted)
        addBoundary(marker);
    else
        delete marker;
}

void Scene::doNewMaterial()
{
    assert(0);
    logMessage("Scene::doNewMaterial()");

    SceneMaterial *marker = Util::scene()->fieldInfo("TODO")->module()->newMaterial();

    if (marker->showDialog(QApplication::activeWindow()) == QDialog::Accepted)
        addMaterial(marker);
    else
        delete marker;
}

void Scene::doTransform()
{
    logMessage("Scene::doTransform()");

    SceneTransformDialog sceneTransformDialog(QApplication::activeWindow());
    sceneTransformDialog.exec();
}

void Scene::doClearSolution()
{
    logMessage("Scene::doClearSolution()");

    m_sceneSolution->clear();
    emit invalidated();
}

void Scene::doProblemProperties()
{
    logMessage("Scene::doProblemProperties()");

    QString scalar = "";
    QString vector = "";
    PhysicFieldVariableComp scalarComp = PhysicFieldVariableComp_Undefined;

    // previous value
    scalar = QString::fromStdString(sceneView()->sceneViewSettings().scalarPhysicFieldVariable);
    scalarComp = sceneView()->sceneViewSettings().scalarPhysicFieldVariableComp;
    vector = QString::fromStdString(sceneView()->sceneViewSettings().vectorPhysicFieldVariable);

    ProblemDialog problemDialog(m_problemInfo, m_fieldInfos, false, QApplication::activeWindow());
    // if (problemDialog.showDialog() == QDialog::Accepted)

    problemDialog.showDialog();

    //TODO - allow "no field"
    FieldInfo *fieldInfo = Util::scene()->fieldInfos().values().at(0);

    // contour
    sceneView()->sceneViewSettings().contourPhysicFieldVariable = fieldInfo->module()->view_default_scalar_variable->id;

    // scalar view
    // determines whether the selected field exists
    for (Hermes::vector<Hermes::Module::LocalVariable *>::iterator it = fieldInfo->module()->view_scalar_variables.begin();
         it < fieldInfo->module()->view_scalar_variables.end(); ++it )
    {
        Hermes::Module::LocalVariable *variable = ((Hermes::Module::LocalVariable *) *it);
        if (variable->id == scalar.toStdString())
        {
            sceneView()->sceneViewSettings().scalarPhysicFieldVariable = variable->id;
            sceneView()->sceneViewSettings().scalarPhysicFieldVariableComp = scalarComp;
        }
    }
    if (sceneView()->sceneViewSettings().scalarPhysicFieldVariable == "")
    {
        sceneView()->sceneViewSettings().scalarPhysicFieldVariable = fieldInfo->module()->view_default_scalar_variable->id;
        sceneView()->sceneViewSettings().scalarPhysicFieldVariableComp = fieldInfo->module()->view_default_scalar_variable_comp();
        sceneView()->sceneViewSettings().scalarRangeAuto = true;
    }

    // vector view
    for (Hermes::vector<Hermes::Module::LocalVariable *>::iterator it = fieldInfo->module()->view_vector_variables.begin();
         it < fieldInfo->module()->view_vector_variables.end(); ++it )
    {
        Hermes::Module::LocalVariable *variable = ((Hermes::Module::LocalVariable *) *it);
        if (variable->id == vector.toStdString())
            sceneView()->sceneViewSettings().vectorPhysicFieldVariable = variable->id;
    }
    if (sceneView()->sceneViewSettings().vectorPhysicFieldVariable == "")
        sceneView()->sceneViewSettings().vectorPhysicFieldVariable = fieldInfo->module()->view_default_vector_variable->id;

    emit invalidated();
}

void Scene::writeToDxf(const QString &fileName)
{
    logMessage("Scene::writeToDxf()");

    RectPoint box = boundingBox();

    // save current locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    DL_Dxf* dxf = new DL_Dxf();
    DL_Codes::version exportVersion = DL_Codes::AC1015;
    DL_WriterA *dw = dxf->out(fileName.toStdString().c_str(), exportVersion);
    if (dw == NULL) {
        qCritical() << fileName << " could not be opened.";
        return;
    }

    dxf->writeHeader(*dw);
    // int variable:
    dw->dxfString(9, "$INSUNITS");
    dw->dxfInt(70, 4);
    // real (double, float) variable:
    dw->dxfString(9, "$DIMEXE");
    dw->dxfReal(40, 1.25);
    // string variable:
    dw->dxfString(9, "$TEXTSTYLE");
    dw->dxfString(7, "Standard");
    // vector variable:
    dw->dxfString(9, "$LIMMIN");
    dw->dxfReal(10, 0.0);
    dw->dxfReal(20, 0.0);
    dw->sectionEnd();
    dw->sectionTables();
    dxf->writeVPort(*dw);
    dw->tableLineTypes(25);
    dxf->writeLineType(*dw, DL_LineTypeData("BYBLOCK", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("BYLAYER", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("CONTINUOUS", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("ACAD_ISO02W100", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("ACAD_ISO03W100", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("ACAD_ISO04W100", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("ACAD_ISO05W100", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("BORDER", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("BORDER2", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("BORDERX2", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("CENTER", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("CENTER2", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("CENTERX2", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("DASHDOT", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("DASHDOT2", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("DASHDOTX2", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("DASHED", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("DASHED2", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("DASHEDX2", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("DIVIDE", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("DIVIDE2", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("DIVIDEX2", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("DOT", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("DOT2", 0));
    dxf->writeLineType(*dw, DL_LineTypeData("DOTX2", 0));
    dw->tableEnd();

    int numberOfLayers = 1;
    dw->tableLayers(numberOfLayers);

    dxf->writeLayer(*dw,
                    DL_LayerData("main", 0),
                    DL_Attributes(
                        std::string(""),            // leave empty
                        DL_Codes::black,            // default color
                        qMax(box.width(), box.height())/100.0,   // default width
                        "CONTINUOUS"));             // default line style

    dw->tableEnd();
    dxf->writeStyle(*dw);
    dxf->writeView(*dw);
    dxf->writeUcs(*dw);

    dw->tableAppid(1);
    dw->tableAppidEntry(0x12);
    dw->dxfString(2, "ACAD");
    dw->dxfInt(70, 0);
    dw->tableEnd();
    dxf->writeDimStyle(*dw, 1, 1, 1, 1, 1);
    dxf->writeBlockRecord(*dw);
    dw->tableEnd();
    dw->sectionEnd();
    dw->sectionBlocks();

    dw->sectionEnd();
    dw->sectionEntities();

    // edges
    for (int i = 0; i<edges->length(); i++)
    {
        if (fabs(edges->at(i)->angle) < EPS_ZERO)
        {
            // line
            double x1 = edges->at(i)->nodeStart->point.x;
            double y1 = edges->at(i)->nodeStart->point.y;
            double x2 = edges->at(i)->nodeEnd->point.x;
            double y2 = edges->at(i)->nodeEnd->point.y;

            dxf->writeLine(*dw, DL_LineData(x1, y1, 0.0, x2, y2, 0.0), DL_Attributes("main", 256, -1, "BYLAYER"));
        }
        else
        {
            // arc
            double cx = edges->at(i)->center().x;
            double cy = edges->at(i)->center().y;
            double radius = edges->at(i)->radius();
            double angle1 = atan2(cy - edges->at(i)->nodeStart->point.y, cx - edges->at(i)->nodeStart->point.x)/M_PI*180.0 + 180.0;
            double angle2 = atan2(cy - edges->at(i)->nodeEnd->point.y, cx - edges->at(i)->nodeEnd->point.x)/M_PI*180.0 + 180.0;

            while (angle1 < 0.0) angle1 += 360.0;
            while (angle1 >= 360.0) angle1 -= 360.0;
            while (angle2 < 0.0) angle2 += 360.0;
            while (angle2 >= 360.0) angle2 -= 360.0;

            dxf->writeArc(*dw, DL_ArcData(cx, cy, 0.0, radius, angle1, angle2), DL_Attributes("main", 256, -1, "BYLAYER"));
        }
    }

    dw->sectionEnd();
    dxf->writeObjects(*dw);
    dxf->writeObjectsEnd(*dw);
    dw->dxfEOF();
    dw->close();

    delete dw;
    delete dxf;

    // set system locale
    setlocale(LC_NUMERIC, plocale);
}


void Scene::readFromDxf(const QString &fileName)
{
    logMessage("Scene::readFromDxf()");

    // save current locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    blockSignals(true);

    DxfFilter *filter = new DxfFilter(this);
    DL_Dxf* dxf = new DL_Dxf();
    if (!dxf->in(fileName.toStdString(), filter))
    {
        qCritical() << fileName << " could not be opened.";
        return;
    }

    delete dxf;
    delete filter;

    blockSignals(false);

    emit invalidated();

    // set system locale
    setlocale(LC_NUMERIC, plocale);
}

ErrorResult Scene::readFromFile(const QString &fileName)
{
    logMessage("Scene::readFromFile()");

    QSettings settings;
    QFileInfo fileInfo(fileName);
    if (fileInfo.absoluteDir() != tempProblemDir())
        settings.setValue("General/LastProblemDir", fileInfo.absolutePath());

    QDomDocument doc;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return ErrorResult(ErrorResultType_Critical, tr("File '%1' cannot be opened (%2).").
                           arg(fileName).
                           arg(file.errorString()));

    // save current locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    clear();
    //TODO - allow "no field"
    m_fieldInfos.clear();

    m_problemInfo->fileName = fileName;
    emit fileNameChanged(fileInfo.absoluteFilePath());

    blockSignals(true);

    if (!doc.setContent(&file)) {
        file.close();
        return ErrorResult(ErrorResultType_Critical, tr("File '%1' is not valid Agros2D file.").arg(fileName));
    }
    file.close();

    QDomNode n;
    QDomElement element;

    // main document
    QDomElement eleDoc = doc.documentElement();
    QString version = eleDoc.attribute("version");

    // convert document for agros2d version >2.0
    if (version.isEmpty())
    {
        Util::scene()->convertA2DFile(fileName);
        return ErrorResult(ErrorResultType_Information, tr("File '%1' has been converted to new version. The file is created in temp directory. Please, save problem file to the new location.").arg(fileName));
    }

    // problem info
    QDomNode eleProblemInfo = eleDoc.elementsByTagName("problem").at(0);

    // name
    m_problemInfo->name = eleProblemInfo.toElement().attribute("name");
    // date
    m_problemInfo->date = QDate::fromString(eleProblemInfo.toElement().attribute("date", QDate::currentDate().toString(Qt::ISODate)), Qt::ISODate);
    // problem type                                                                                                                                                                                                                             `
    m_problemInfo->coordinateType = coordinateTypeFromStringKey(eleProblemInfo.toElement().attribute("coordinatetype"));

    // harmonic
    m_problemInfo->frequency = eleProblemInfo.toElement().attribute("frequency", "0").toDouble();

    // transient
    m_problemInfo->timeStep.setText(eleProblemInfo.toElement().attribute("timestep", "1"));
    m_problemInfo->timeTotal.setText(eleProblemInfo.toElement().attribute("timetotal", "1"));

    // matrix solver
    m_problemInfo->matrixSolver = matrixSolverTypeFromStringKey(eleProblemInfo.toElement().attribute("matrix_solver",
                                                                                                     matrixSolverTypeToStringKey(Hermes::SOLVER_UMFPACK)));

    // startup script
    QDomNode eleScriptStartup = eleProblemInfo.toElement().elementsByTagName("startupscript").at(0);
    m_problemInfo->scriptStartup = eleScriptStartup.toElement().text();

    // FIX ME - EOL conversion
    QPlainTextEdit textEdit;
    textEdit.setPlainText(m_problemInfo->scriptStartup);
    m_problemInfo->scriptStartup = textEdit.toPlainText();

    // description
    QDomNode eleDescription = eleProblemInfo.toElement().elementsByTagName("description").at(0);
    m_problemInfo->description = eleDescription.toElement().text();

    // geometry ***************************************************************************************************************

    QDomNode eleGeometry = eleDoc.elementsByTagName("geometry").at(0);

    // nodes
    QDomNode eleNodes = eleGeometry.toElement().elementsByTagName("nodes").at(0);
    n = eleNodes.firstChild();
    while(!n.isNull())
    {
        element = n.toElement();

        Point point = Point(element.attribute("x").toDouble(), element.attribute("y").toDouble());

        addNode(new SceneNode(point));
        n = n.nextSibling();
    }

    // edges
    QDomNode eleEdges = eleGeometry.toElement().elementsByTagName("edges").at(0);
    n = eleEdges.firstChild();
    while(!n.isNull())
    {
        element = n.toElement();

        SceneNode *nodeFrom = nodes->at(element.attribute("start").toInt());
        SceneNode *nodeTo = nodes->at(element.attribute("end").toInt());
        double angle = element.attribute("angle", "0").toDouble();
        int refineTowardsEdge = element.attribute("refine_towards", "0").toInt();

        SceneEdge *edge = new SceneEdge(nodeFrom, nodeTo, angle, refineTowardsEdge);

        addEdge(edge);

        n = n.nextSibling();
    }

    // labels
    QDomNode eleLabels = eleGeometry.toElement().elementsByTagName("labels").at(0);
    n = eleLabels.firstChild();
    while(!n.isNull())
    {
        element = n.toElement();
        Point point = Point(element.attribute("x").toDouble(), element.attribute("y").toDouble());
        double area = element.attribute("area", "0").toDouble();
        int polynomialOrder = element.attribute("polynomialorder", "0").toInt();

        SceneLabel *label = new SceneLabel(point, area, polynomialOrder);
        // SceneMaterial *marker = materials->at(element.attribute("marker").toInt());
        // label->addMarker(marker);

        addLabel(label);

        n = n.nextSibling();
    }

    // field ***************************************************************************************************************

    QDomNode eleFields = eleProblemInfo.toElement().elementsByTagName("fields").at(0);
    n = eleFields.firstChild();
    while(!n.isNull())
    {
        QDomNode eleField = n.toElement();

        FieldInfo *field = new FieldInfo(m_problemInfo, eleField.toElement().attribute("fieldid"));

        // analysis type
        field->setAnalysisType(analysisTypeFromStringKey(eleField.toElement().attribute("analysistype",
                                                                                        analysisTypeToStringKey(AnalysisType_SteadyState))));

        // initial condition
        field->initialCondition.setText(eleField.toElement().attribute("initialcondition", "0"));

        // weakforms
        field->weakFormsType = weakFormsTypeFromStringKey(eleField.toElement().attribute("weakforms",
                                                                                         weakFormsTypeToStringKey(WeakFormsType_Compiled)));

        // polynomial order
        field->polynomialOrder = eleField.toElement().attribute("polynomialorder").toInt();

        // mesh
        QDomNode eleFieldMesh = eleField.toElement().elementsByTagName("mesh").at(0);

        // mesh type
        field->meshType = meshTypeFromStringKey(eleFieldMesh.toElement().attribute("meshtype",
                                                                                   meshTypeToStringKey(MeshType_Triangle)));

        // number of refinements
        field->numberOfRefinements = eleFieldMesh.toElement().attribute("numberofrefinements").toInt();

        // adaptivity
        QDomNode eleFieldAdaptivity = eleField.toElement().elementsByTagName("adaptivity").at(0);

        field->adaptivityType = adaptivityTypeFromStringKey(eleFieldAdaptivity.toElement().attribute("adaptivitytype"));
        field->adaptivitySteps = eleFieldAdaptivity.toElement().attribute("adaptivitysteps").toInt();
        field->adaptivityTolerance = eleFieldAdaptivity.toElement().attribute("adaptivitytolerance").toDouble();

        // linearity
        QDomNode eleFieldLinearity = eleField.toElement().elementsByTagName("solver").at(0);

        field->linearityType = linearityTypeFromStringKey(eleFieldLinearity.toElement().attribute("linearity",
                                                                                                  linearityTypeToStringKey(LinearityType_Linear)));
        field->nonlinearSteps = eleFieldLinearity.toElement().attribute("nonlinearsteps", "10").toInt();
        field->nonlinearTolerance = eleFieldLinearity.toElement().attribute("nonlineartolerance", "1e-3").toDouble();

        // boundary conditions
        QDomNode eleBoundaries = eleField.toElement().elementsByTagName("boundaries").at(0);
        n = eleBoundaries.firstChild();
        while(!n.isNull())
        {
            element = n.toElement();
            QString name = element.toElement().attribute("name");
            QString type = element.toElement().attribute("type");

            if (element.toElement().attribute("id") == 0)
            {
                // none marker
                addBoundary(new SceneBoundaryNone());
            }
            else
            {
                // read marker
                SceneBoundary *boundary = new SceneBoundary(field,
                                                            name.toStdString(),
                                                            type.toStdString());

                Hermes::Module::BoundaryType *boundary_type = field->module()->get_boundary_type(type.toStdString());
                for (Hermes::vector<Hermes::Module::BoundaryTypeVariable *>::iterator it = boundary_type->variables.begin();
                     it < boundary_type->variables.end(); ++it)
                {
                    Hermes::Module::BoundaryTypeVariable *variable = ((Hermes::Module::BoundaryTypeVariable *) *it);

                    boundary->setValue(variable->id,
                                       Value(element.toElement().attribute(QString::fromStdString(variable->id), "0")));
                }

                Util::scene()->addBoundary(boundary);

                // add boundary to the edge marker
                QDomNode nodeEdge = element.firstChild();
                while(!nodeEdge.isNull())
                {
                    QDomElement eleEdge = nodeEdge.toElement();

                    int id = eleEdge.toElement().attribute("edge").toInt();
                    edges->at(id)->addMarker(boundary);

                    nodeEdge = nodeEdge.nextSibling();
                }
            }

            n = n.nextSibling();
        }

        // materials
        QDomNode eleMaterials = eleField.toElement().elementsByTagName("materials").at(0);
        n = eleMaterials.firstChild();
        while(!n.isNull())
        {
            element = n.toElement();
            QString name = element.toElement().attribute("name");

            if (element.toElement().attribute("id") == 0)
            {
                // none marker
                addMaterial(new SceneMaterialNone());
            }
            else
            {
                // read marker
                SceneMaterial *material = new SceneMaterial(field,
                                                            name.toStdString());
                Hermes::vector<Hermes::Module::MaterialTypeVariable *> materials = field->module()->material_type_variables;
                for (Hermes::vector<Hermes::Module::MaterialTypeVariable *>::iterator it = materials.begin(); it < materials.end(); ++it)
                {
                    Hermes::Module::MaterialTypeVariable *variable = ((Hermes::Module::MaterialTypeVariable *) *it);

                    material->setValue(variable->id,
                                       Value(element.toElement().attribute(QString::fromStdString(variable->id),
                                                                           QString::number(variable->default_value))));
                }

                // add material
                Util::scene()->addMaterial(material);

                // add material to the label marker
                QDomNode nodeLabel = element.firstChild();
                while(!nodeLabel.isNull())
                {
                    QDomElement eleLabel = nodeLabel.toElement();

                    int id = eleLabel.toElement().attribute("label").toInt();
                    labels->at(id)->addMarker(material);

                    nodeLabel = nodeLabel.nextSibling();
                }
            }

            n = n.nextSibling();
        }

        // add field
        addField(field);

        // next field
        n = n.nextSibling();
    }

    // set system locale
    setlocale(LC_NUMERIC, plocale);

    blockSignals(false);

    // default values
    emit invalidated();
    emit defaultValues();

    // mesh
    /*
    if (eleDoc.elementsByTagName("mesh").count() > 0)
    {
        QDomNode eleMesh = eleDoc.elementsByTagName("mesh").at(0);
        Util::scene()->sceneSolution()->loadMeshInitial(eleMesh.toElement());
    }

    // solutions
    if (eleDoc.elementsByTagName("solutions").count() > 0)
    {
        QDomNode eleSolutions = eleDoc.elementsByTagName("solutions").at(0);
        Util::scene()->sceneSolution()->loadSolution(eleSolutions.toElement());
        emit invalidated();
    }
    */

    // run script
    runPythonScript(m_problemInfo->scriptStartup);

    return ErrorResult();
}

ErrorResult Scene::writeToFile(const QString &fileName)
{
    assert(0); //TODO
    //    logMessage("Scene::writeToFile()");

    //    QSettings settings;

    //    // custom form
    //    /*
    //    if (m_problemInfo->fieldId() == "custom")
    //        if (!QFile::exists(m_problemInfo->fileName.left(m_problemInfo->fileName.size() - 4) + ".xml"))
    //        {
    //            QFile::remove(fileName.left(fileName.size() - 4) + ".xml");
    //            QFile::copy(m_problemInfo->fileName.left(m_problemInfo->fileName.size() - 4) + ".xml",
    //                        fileName.left(fileName.size() - 4) + ".xml");
    //        }
    //    */

    //    if (QFileInfo(tempProblemFileName()).baseName() != QFileInfo(fileName).baseName())
    //    {
    //        QFileInfo fileInfo(fileName);
    //        if (fileInfo.absoluteDir() != tempProblemDir())
    //        {
    //            settings.setValue("General/LastProblemDir", fileInfo.absoluteFilePath());
    //            m_problemInfo->fileName = fileName;
    //        }
    //    }

    //    // save current locale
    //    char *plocale = setlocale (LC_NUMERIC, "");
    //    setlocale (LC_NUMERIC, "C");

    //    QDomDocument doc;

    //    // main document
    //    QDomElement eleDoc = doc.createElement("document");
    //    doc.appendChild(eleDoc);
    //    eleDoc.setAttribute("version", "2.0");

    //    // problems
    //    QDomNode eleProblems = doc.createElement("problems");
    //    eleDoc.appendChild(eleProblems);
    //    // first problem
    //    QDomElement eleProblem = doc.createElement("problem");
    //    eleProblems.appendChild(eleProblem);
    //    // id
    //    eleProblem.setAttribute("id", 0);
    //    // name
    //    eleProblem.setAttribute("name", m_problemInfo->name);
    //    // date
    //    eleProblem.setAttribute("date", m_problemInfo->date.toString(Qt::ISODate));
    //    // problem type
    //    eleProblem.toElement().setAttribute("problemtype", problemTypeToStringKey(m_problemInfo->problemType));
    //    // analysis type
    //    eleProblem.setAttribute("analysistype", analysisTypeToStringKey(m_problemInfo->analysisType));
    //    // type
    //    eleProblem.setAttribute("type", QString::fromStdString(m_problemInfo->fieldId()));
    //    // number of refinements
    //    eleProblem.setAttribute("numberofrefinements", m_problemInfo->numberOfRefinements);
    //    // polynomial order
    //    eleProblem.setAttribute("polynomialorder", m_problemInfo->polynomialOrder);
    //    // mesh type
    //    eleProblem.setAttribute("meshtype", meshTypeToStringKey(m_problemInfo->meshType));
    //    // adaptivity
    //    eleProblem.setAttribute("adaptivitytype", adaptivityTypeToStringKey(m_problemInfo->adaptivityType));
    //    eleProblem.setAttribute("adaptivitysteps", m_problemInfo->adaptivitySteps);
    //    eleProblem.setAttribute("adaptivitytolerance", m_problemInfo->adaptivityTolerance);
    //    eleProblem.setAttribute("maxdofs", Util::config()->maxDofs);
    //    // harmonic magnetic
    //    eleProblem.setAttribute("frequency", m_problemInfo->frequency);
    //    // transient
    //    eleProblem.setAttribute("timestep", m_problemInfo->timeStep.text());
    //    eleProblem.setAttribute("timetotal", m_problemInfo->timeTotal.text());
    //    eleProblem.setAttribute("initialcondition", m_problemInfo->initialCondition.text());
    //    // linearity
    //    eleProblem.setAttribute("linearity", linearityTypeToStringKey(m_problemInfo->linearityType));
    //    eleProblem.setAttribute("nonlinearsteps", m_problemInfo->nonlinearSteps);
    //    eleProblem.setAttribute("nonlineartolerance", m_problemInfo->nonlinearTolerance);

    //    // matrix solver
    //    eleProblem.setAttribute("matrix_solver", matrixSolverTypeToStringKey(m_problemInfo->matrixSolver));

    //    // weakforms
    //    eleProblem.setAttribute("weakforms", weakFormsTypeToStringKey(m_problemInfo->weakFormsType));

    //    // startup script
    //    QDomElement eleScriptStartup = doc.createElement("scriptstartup");
    //    eleScriptStartup.appendChild(doc.createTextNode(m_problemInfo->scriptStartup));
    //    eleProblem.appendChild(eleScriptStartup);

    //    // description
    //    QDomElement eleDescription = doc.createElement("description");
    //    eleDescription.appendChild(doc.createTextNode(m_problemInfo->description));
    //    eleProblem.appendChild(eleDescription);

    //    // geometry
    //    QDomNode eleGeometry = doc.createElement("geometry");
    //    eleDoc.appendChild(eleGeometry);

    //    // geometry ***************************************************************************************************************

    //    // nodes
    //    QDomNode eleNodes = doc.createElement("nodes");
    //    eleGeometry.appendChild(eleNodes);
    //    for (int i = 0; i<nodes.length(); i++)
    //    {
    //        QDomElement eleNode = doc.createElement("node");

    //        eleNode.setAttribute("id", i);
    //        eleNode.setAttribute("x", nodes[i]->point.x);
    //        eleNode.setAttribute("y", nodes[i]->point.y);

    //        eleNodes.appendChild(eleNode);
    //    }

    //    // edges
    //    QDomNode eleEdges = doc.createElement("edges");
    //    eleGeometry.appendChild(eleEdges);
    //    for (int i = 0; i<edges.length(); i++)
    //    {
    //        QDomElement eleEdge = doc.createElement("edge");

    //        eleEdge.setAttribute("id", i);
    //        eleEdge.setAttribute("start", nodes.indexOf(edges[i]->nodeStart));
    //        eleEdge.setAttribute("end", nodes.indexOf(edges[i]->nodeEnd));
    //        eleEdge.setAttribute("angle", edges[i]->angle);
    //        eleEdge.setAttribute("refine_towards", edges[i]->refineTowardsEdge);
    //        eleEdge.setAttribute("marker", boundaries.indexOf(edges[i]->boundary));

    //        eleEdges.appendChild(eleEdge);
    //    }

    //    // labels
    //    QDomNode eleLabels = doc.createElement("labels");
    //    eleGeometry.appendChild(eleLabels);
    //    for (int i = 0; i<labels.length(); i++)
    //    {
    //        QDomElement eleLabel = doc.createElement("label");

    //        eleLabel.setAttribute("id", i);
    //        eleLabel.setAttribute("x", labels[i]->point.x);
    //        eleLabel.setAttribute("y", labels[i]->point.y);
    //        eleLabel.setAttribute("area", labels[i]->area);
    //        eleLabel.setAttribute("polynomialorder", labels[i]->polynomialOrder);
    //        eleLabel.setAttribute("marker", materials.indexOf(labels[i]->material));

    //        eleLabels.appendChild(eleLabel);
    //    }

    //    // markers ***************************************************************************************************************

    //    // edge markers
    //    QDomNode eleBoundaries = doc.createElement("edges");
    //    eleProblem.appendChild(eleBoundaries);
    //    for (int i = 1; i<boundaries.length(); i++)
    //    {
    //        QDomElement eleBoundary = doc.createElement("edge");

    //        eleBoundary.setAttribute("id", i);
    //        eleBoundary.setAttribute("name", QString::fromStdString(boundaries[i]->name));
    //        if (boundaries[i]->getType() == "")
    //            eleBoundary.setAttribute("type", "none");

    //        if (i > 0)
    //        {
    //            // write marker
    //            eleBoundary.setAttribute("type", QString::fromStdString(boundaries[i]->getType()));

    //            for (std::map<std::string, Value>::iterator it = boundaries[i]->getValues().begin(); it != boundaries[i]->getValues().end(); ++it)
    //                eleBoundary.setAttribute(QString::fromStdString(it->first), it->second.toString());
    //        }

    //        eleBoundaries.appendChild(eleBoundary);
    //    }

    //    // label markers
    //    QDomNode eleMaterials = doc.createElement("labels");
    //    eleProblem.appendChild(eleMaterials);
    //    for (int i = 1; i<materials.length(); i++)
    //    {
    //        QDomElement eleMaterial = doc.createElement("label");

    //        eleMaterial.setAttribute("id", i);
    //        eleMaterial.setAttribute("name", QString::fromStdString(materials[i]->name));

    //        if (i > 0)
    //        {
    //            // write marker
    //            for (std::map<std::string, Value>::iterator it = materials[i]->values.begin(); it != materials[i]->values.end(); ++it)
    //                eleMaterial.setAttribute(QString::fromStdString(it->first), it->second.toString());
    //        }

    //        eleMaterials.appendChild(eleMaterial);
    //    }

    //    if (settings.value("Solver/SaveProblemWithSolution", false).value<bool>())
    //    {
    //        // mesh
    //        QDomNode eleMesh = doc.createElement("mesh");
    //        Util::scene()->sceneSolution()->saveMeshInitial(&doc, eleMesh.toElement());
    //        eleDoc.appendChild(eleMesh);

    //        // solution
    //        QDomNode eleSolutions = doc.createElement("solutions");
    //        Util::scene()->sceneSolution()->saveSolution(&doc, eleSolutions.toElement());
    //        eleDoc.appendChild(eleSolutions);
    //    }

    //    // save to file
    //    QFile file(fileName);
    //    if (!file.open(QIODevice::WriteOnly))
    //        return ErrorResult(ErrorResultType_Critical, tr("File '%1' cannot be saved (%2).").
    //                           arg(fileName).
    //                           arg(file.errorString()));

    //    QTextStream out(&file);
    //    doc.save(out, 4);

    //    file.waitForBytesWritten(0);
    //    file.close();

    //    if (QFileInfo(tempProblemFileName()).baseName() != QFileInfo(fileName).baseName())
    //        emit fileNameChanged(QFileInfo(fileName).absoluteFilePath());

    //    emit invalidated();

    //    // set system locale
    //    setlocale(LC_NUMERIC, plocale);

    //    return ErrorResult();
}

void Scene::convertA2DFile(const QString &fileName)
{
    logMessage("Scene::convertA2DFile()");

    QDomDocument convertDocument;
    QDomDocument convertTable;

    // read document
    QFile convertDocumentFile(fileName);
    convertDocumentFile.open(QIODevice::ReadOnly);
    convertDocument.setContent(&convertDocumentFile);
    convertDocumentFile.close();

    QDomElement document = convertDocument.documentElement();
    QDomNode documentProblems = document.toElement().elementsByTagName("problems").at(0);
    QDomNode documentProblem = documentProblems.toElement().elementsByTagName("problem").at(0);

    // read convert table
    QFile convertTableFile(datadir() + "/resources/a2d_convert_table.xml");
    convertTableFile.open(QIODevice::ReadOnly);
    convertTable.setContent(&convertTableFile);
    convertTableFile.close();

    QDomElement table = convertTable.documentElement();
    QDomNode tableProblems = table.toElement().elementsByTagName("problems").at(0);

    // convert edges
    QDomNode documentEdges = documentProblem.toElement().elementsByTagName("edges").at(0);
    QDomNode tableEdges = tableProblems.toElement().elementsByTagName("edges").at(0);

    QDomNode documentEdge = documentEdges.firstChild();
    while(!documentEdge.isNull())
    {
        QDomElement  documentElement = documentEdge.toElement();

        QDomNode tableEdge = tableEdges.firstChild();
        while(!tableEdge.isNull())
        {
            QDomElement tableElement = tableEdge.toElement();

            QString parameter = tableElement.toElement().attribute("parameter");
            if (!parameter.isEmpty())
            {
                QString parameterValue = documentElement.attribute(parameter);
                if (parameterValue == tableElement.attribute("oldvalue"))
                    documentElement.setAttribute(parameter, tableElement.attribute("newvalue"));
            }

            QString value = documentElement.toElement().attribute(tableElement.toElement().attribute("old"));
            if (!value.isEmpty())
            {
                if (documentElement.toElement().attribute("type") == tableElement.toElement().attribute("type"))
                {
                    documentElement.setAttribute(tableElement.toElement().attribute("new"), value);
                    documentElement.removeAttribute(tableElement.toElement().attribute("old"));
                }
            }

            tableEdge = tableEdge.nextSibling();
        }

        documentEdge = documentEdge.nextSibling();
    }

    // convert labels
    QDomNode documentLabels = documentProblem.toElement().elementsByTagName("labels").at(0);
    QDomNode tableLabels = tableProblems.toElement().elementsByTagName("labels").at(0);

    QDomNode documentLabel = documentLabels.firstChild();
    while(!documentLabel.isNull())
    {
        QDomElement  documentElement = documentLabel.toElement();

        QDomNode tableLabel = tableLabels.firstChild();
        while(!tableLabel.isNull())
        {
            QDomElement  tableElement = tableLabel.toElement();

            QString value = documentElement.toElement().attribute(tableElement.toElement().attribute("old"));
            if (!value.isEmpty())
            {
                documentElement.setAttribute(tableElement.toElement().attribute("new"), value);
                documentElement.removeAttribute(tableElement.toElement().attribute("old"));
            }

            tableLabel = tableLabel.nextSibling();
        }

        documentLabel = documentLabel.nextSibling();
    }

    // set document version
    document.setAttribute("version", "2.0");

    // write document
    QFileInfo fileInfo(fileName);
    QString tempFilePath = tempProblemDir() + "/" + fileInfo.baseName() + "." + fileInfo.suffix();
    QFile tempFile(tempFilePath);

    tempFile.open(QIODevice::WriteOnly);
    QTextStream out(&tempFile);
    convertDocument.save(out, 4);
    tempFile.waitForBytesWritten(0);
    tempFile.close();

    // FIXME (call open function from MainWindow)
    ErrorResult result = readFromFile(tempFilePath);
    if (result.isError())
    {
        result.showDialog();
    }
}
