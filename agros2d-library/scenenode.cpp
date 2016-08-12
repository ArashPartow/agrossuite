// This file is part of Agros.
//
// Agros is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros.  If not, see <http://www.gnu.org/licenses/>.
//
//
// University of West Bohemia, Pilsen, Czech Republic
// Email: info@agros2d.org, home page: http://agros2d.org/

#include "scenenode.h"

#include "util/util.h"
#include "util/global.h"

#include "scene.h"
#include "sceneedge.h"
#include "scenemarker.h"
#include "solver/problem.h"
#include "solver/problem_config.h"

SceneNode::SceneNode(Scene *scene, const Point &point) : SceneBasic(scene),
    m_point(scene->parentProblem(), point)
{
}

SceneNode::SceneNode(Scene *scene, const PointValue &pointValue) : SceneBasic(scene),
    m_point(pointValue)
{
}

void SceneNode::setPointValue(const PointValue &point)
{
    m_point = point;

    // refresh cache
    foreach (SceneFace *edge, connectedEdges())
        edge->computeCenterAndRadius();
}

double SceneNode::distance(const Point &point) const
{
    return (this->point() - point).magnitude();
}

/*
int SceneNode::showDialog(QWidget *parent, bool isNew)
{
    SceneNodeDialog *dialog = new SceneNodeDialog(this, parent, isNew);
    return dialog->exec();
}

SceneNodeCommandRemove* SceneNode::getRemoveCommand()
{
    return new SceneNodeCommandRemove(this->pointValue());
}
*/
SceneNode *SceneNode::findClosestNode(Scene *scene, const Point &point)
{
    SceneNode *nodeClosest = NULL;

    double distance = numeric_limits<double>::max();
    foreach (SceneNode *node, scene->nodes->items())
    {
        double nodeDistance = node->distance(point);
        if (node->distance(point) < distance)
        {
            distance = nodeDistance;
            nodeClosest = node;
        }
    }

    return nodeClosest;
}

bool SceneNode::isConnected() const
{
    foreach (SceneFace *edge, m_scene->faces->items())
        if (edge->nodeStart() == this || edge->nodeEnd() == this)
            return true;

    return false;
}

bool SceneNode::isEndNode() const
{
    int connections = 0;
    foreach (SceneFace *edge, m_scene->faces->items())
        if (edge->nodeStart() == this || edge->nodeEnd() == this)
            connections++;

    return (connections == 1);
}

QList<SceneFace *> SceneNode::connectedEdges() const
{
    QList<SceneFace *> edges;
    edges.reserve(m_scene->faces->count());

    foreach (SceneFace *edge, m_scene->faces->items())
        if (edge->nodeStart() == this || edge->nodeEnd() == this)
            edges.append(edge);

    return edges;
}

int SceneNode::numberOfConnectedEdges() const
{
    return m_scene->numberOfConnectedNodeEdges().value(const_cast<SceneNode *>(this));
}

bool SceneNode::hasLyingEdges() const
{
    return (lyingEdges().length() > 0);
}

QList<SceneFace *> SceneNode::lyingEdges() const
{
    return m_scene->lyingEdgeNodes().keys(const_cast<SceneNode *>(this));
}

bool SceneNode::isOutsideArea() const
{
    return (m_scene->parentProblem()->config()->coordinateType() == CoordinateType_Axisymmetric) &&
            (this->point().x < - EPS_ZERO);
}

bool SceneNode::isError()
{
    return (isOutsideArea() || numberOfConnectedEdges() <= 1 || hasLyingEdges());
}

// *************************************************************************************************************************************

SceneNode* SceneNodeContainer::get(SceneNode *node) const
{
    foreach (SceneNode *nodeCheck, m_data)
    {
        if (nodeCheck->point() == node->point())
        {
            return nodeCheck;
        }
    }

    return NULL;
}

SceneNode* SceneNodeContainer::get(const Point &point) const
{
    foreach (SceneNode *nodeCheck, m_data)
    {
        if (nodeCheck->point() == point)
            return nodeCheck;
    }

    return NULL;
}

bool SceneNodeContainer::remove(SceneNode *item)
{
    // remove all edges connected to this node
    item->scene()->faces->removeConnectedToNode(item);

    return SceneBasicContainer<SceneNode>::remove(item);
}

RectPoint SceneNodeContainer::boundingBox() const
{
    Point min( numeric_limits<double>::max(),  numeric_limits<double>::max());
    Point max(-numeric_limits<double>::max(), -numeric_limits<double>::max());

    foreach (SceneNode *node, m_data)
    {
        min.x = qMin(min.x, node->point().x);
        max.x = qMax(max.x, node->point().x);
        min.y = qMin(min.y, node->point().y);
        max.y = qMax(max.y, node->point().y);
    }

    return RectPoint(min, max);
}

SceneNodeContainer SceneNodeContainer::selected()
{
    SceneNodeContainer list;
    foreach (SceneNode* item, this->m_data)
    {
        if (item->isSelected())
            list.m_data.push_back(item);
    }

    return list;
}

SceneNodeContainer SceneNodeContainer::highlighted()
{
    SceneNodeContainer list;
    foreach (SceneNode* item, this->m_data)
    {
        if (item->isHighlighted())
            list.m_data.push_back(item);
    }

    return list;
}

// *************************************************************************************************************************************

/*

SceneNodeDialog::SceneNodeDialog(SceneNode *node, QWidget *parent, bool isNew) : SceneBasicDialog(parent, isNew)
{
    m_object = node;

    setWindowIcon(icon("scene-node"));
    setWindowTitle(tr("Node"));

    createControls();

    load();

    setMinimumSize(sizeHint());
    // setMaximumSize(sizeHint());
}

SceneNodeDialog::~SceneNodeDialog()
{
    delete txtPointX;
    delete txtPointY;
}

QLayout* SceneNodeDialog::createContent()
{
    txtPointX = new ValueLineEdit();
    txtPointY = new ValueLineEdit();
    connect(txtPointX, SIGNAL(editingFinished()), this, SLOT(doEditingFinished()));
    connect(txtPointY, SIGNAL(editingFinished()), this, SLOT(doEditingFinished()));
    connect(txtPointX, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtPointY, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    lblDistance = new QLabel();
    lblAngle = new QLabel();

    // coordinates must be greater then or equal to 0 (axisymmetric case)
    if (m_object->scene()->parentProblem()->config()->coordinateType() == CoordinateType_Axisymmetric)
        txtPointX->setMinimum(0.0);

    QFormLayout *layout = new QFormLayout();
    layout->addRow(m_object->scene()->parentProblem()->config()->labelX() + " (m):", txtPointX);
    layout->addRow(m_object->scene()->parentProblem()->config()->labelY() + " (m):", txtPointY);
    layout->addRow(tr("Distance:"), lblDistance);
    layout->addRow(tr("Angle:"), lblAngle);

    return layout;
}

bool SceneNodeDialog::load()
{
    SceneNode *sceneNode = dynamic_cast<SceneNode *>(m_object);

    txtPointX->setValue(sceneNode->pointValue().x());
    txtPointY->setValue(sceneNode->pointValue().y());

    doEditingFinished();

    return true;
}

bool SceneNodeDialog::save()
{
    if (!txtPointX->evaluate(false)) return false;
    if (!txtPointY->evaluate(false)) return false;

    SceneNode *sceneNode = dynamic_cast<SceneNode *>(m_object);

    PointValue point(txtPointX->value(), txtPointY->value());

    // check if node doesn't exists
    if (m_object->scene()->getNode(point.point()) && ((sceneNode->point() != point.point()) || m_isNew))
    {
        QMessageBox::warning(this, tr("Node"), tr("Node already exists."));
        return false;
    }

    if (!m_isNew)
    {
        if (sceneNode->point() != point.point())
        {
            m_object->scene()->undoStack()->push(new SceneNodeCommandEdit(sceneNode->pointValue(), point));
        }
    }

    sceneNode->setPointValue(point);

    return true;
}

void SceneNodeDialog::doEditingFinished()
{
    lblDistance->setText(QString("%1 m").arg(sqrt(txtPointX->number()*txtPointX->number() + txtPointY->number()*txtPointY->number())));
    lblAngle->setText(QString("%1 deg.").arg(
                          (sqrt(txtPointX->number()*txtPointX->number() + txtPointY->number()*txtPointY->number()) > EPS_ZERO)
                          ? atan2(txtPointY->number(), txtPointX->number()) / M_PI * 180.0 : 0.0));
}

// undo framework *******************************************************************************************************************

SceneNodeCommandAdd::SceneNodeCommandAdd(const PointValue &point, QUndoCommand *parent)
    : QUndoCommand(parent), m_point(point)
{
}

void SceneNodeCommandAdd::undo()
{
    SceneNode *node = Agros2D::problem()->scene()->getNode(m_point.point());
    if (node)
    {
        Agros2D::problem()->scene()->nodes->remove(node);
        Agros2D::problem()->scene()->invalidate();
    }
}

void SceneNodeCommandAdd::redo()
{
    Agros2D::problem()->scene()->addNode(new SceneNode(Agros2D::problem()->scene(), m_point));
    Agros2D::problem()->scene()->invalidate();
}

SceneNodeCommandRemove::SceneNodeCommandRemove(const PointValue &point, QUndoCommand *parent)
    : QUndoCommand(parent), m_point(point)
{    
}

void SceneNodeCommandRemove::undo()
{
    Agros2D::problem()->scene()->addNode(new SceneNode(Agros2D::problem()->scene(), m_point));
    Agros2D::problem()->scene()->invalidate();
}

void SceneNodeCommandRemove::redo()
{
    SceneNode *node = Agros2D::problem()->scene()->getNode(m_point.point());
    if (node)
    {
        Agros2D::problem()->scene()->nodes->remove(node);
        Agros2D::problem()->scene()->invalidate();
    }
}

SceneNodeCommandEdit::SceneNodeCommandEdit(const PointValue &point, const PointValue &pointNew, QUndoCommand *parent)
    : QUndoCommand(parent), m_point(point), m_pointNew(pointNew)
{    
}

void SceneNodeCommandEdit::undo()
{
    SceneNode *node = Agros2D::problem()->scene()->getNode(m_pointNew.point());
    if (node)
    {
        node->setPointValue(m_point);
        Agros2D::problem()->scene()->invalidate();
    }
}

void SceneNodeCommandEdit::redo()
{
    SceneNode *node = Agros2D::problem()->scene()->getNode(m_point.point());
    if (node)
    {
        node->setPointValue(m_pointNew);
        Agros2D::problem()->scene()->invalidate();
    }
}

SceneNodeCommandMoveMulti::SceneNodeCommandMoveMulti(QList<PointValue> points, QList<PointValue> pointsNew, QUndoCommand *parent)
    : QUndoCommand(parent), m_points(points), m_pointsNew(pointsNew)
{   
}

void SceneNodeCommandMoveMulti::moveAll(QList<PointValue> moveFrom, QList<PointValue> moveTo)
{
    assert(moveFrom.size() == moveTo.size());
    QList<SceneNode*> nodes;
    for (int i = 0; i < moveFrom.size(); i++)
    {
        Point point = moveFrom[i].point();
        SceneNode *node = Agros2D::problem()->scene()->getNode(point);
        nodes.push_back(node);
    }

    for (int i = 0; i < moveFrom.size(); i++)
    {
        ProblemBase *problem = moveTo[i].x().problem();
        Point point = moveTo[i].point();
        SceneNode *node = nodes[i];
        if (node)
        {
            node->setPointValue(PointValue(problem, point));
        }
    }
}

void SceneNodeCommandMoveMulti::undo()
{
    Agros2D::problem()->scene()->stopInvalidating(true);

    moveAll(m_pointsNew, m_points);

    Agros2D::problem()->scene()->stopInvalidating(false);
    Agros2D::problem()->scene()->invalidate();
}

void SceneNodeCommandMoveMulti::redo()
{
    Agros2D::problem()->scene()->stopInvalidating(true);

    moveAll(m_points, m_pointsNew);

    Agros2D::problem()->scene()->stopInvalidating(false);
    Agros2D::problem()->scene()->invalidate();
}

SceneNodeCommandAddMulti::SceneNodeCommandAddMulti(QList<PointValue> points, QUndoCommand *parent)
    : QUndoCommand(parent), m_points(points)
{
}

void SceneNodeCommandAddMulti::undo()
{
    Agros2D::problem()->scene()->stopInvalidating(true);
    foreach(PointValue point, m_points)
    {
        SceneNode *node = Agros2D::problem()->scene()->getNode(point.point());
        if (node)
        {
            Agros2D::problem()->scene()->nodes->remove(node);
        }
    }

    Agros2D::problem()->scene()->stopInvalidating(false);
    Agros2D::problem()->scene()->invalidate();
}

void SceneNodeCommandAddMulti::redo()
{
    Agros2D::problem()->scene()->stopInvalidating(true);

    foreach(PointValue point, m_points)
    {
        Agros2D::problem()->scene()->addNode(new SceneNode(Agros2D::problem()->scene(), point));
    }

    Agros2D::problem()->scene()->stopInvalidating(false);
    Agros2D::problem()->scene()->invalidate();
}

SceneNodeCommandRemoveMulti::SceneNodeCommandRemoveMulti(QList<PointValue> points, QUndoCommand *parent)
    : QUndoCommand(parent), m_nodePoints(points)
{    
}

void SceneNodeCommandRemoveMulti::undo()
{
    Agros2D::problem()->scene()->stopInvalidating(true);

    // new nodes
    foreach(PointValue point, m_nodePoints)
    {
        Agros2D::problem()->scene()->addNode(new SceneNode(Agros2D::problem()->scene(), point));
    }

    // new edges
    for (int i = 0; i < m_edgePointStart.count(); i++)
    {
        SceneNode *nodeStart = Agros2D::problem()->scene()->getNode(m_edgePointStart[i]);
        SceneNode *nodeEnd = Agros2D::problem()->scene()->getNode(m_edgePointEnd[i]);
        assert(nodeStart && nodeEnd);
        SceneFace *edge = new SceneFace(Agros2D::problem()->scene(), nodeStart, nodeEnd, m_edgeAngle[i]);

        edge->addMarkersFromStrings(m_edgeMarkers[i]);

        // add edge to the list
        Agros2D::problem()->scene()->addFace(edge);
    }

    Agros2D::problem()->scene()->stopInvalidating(false);
    Agros2D::problem()->scene()->invalidate();
}

void SceneNodeCommandRemoveMulti::redo()
{
    m_edgePointStart.clear();
    m_edgePointEnd.clear();
    m_edgeAngle.clear();
    m_edgeMarkers.clear();

    Agros2D::problem()->scene()->stopInvalidating(true);

    foreach (PointValue point, m_nodePoints)
    {
        SceneNode *node = Agros2D::problem()->scene()->getNode(point.point());
        if (node)
        {
            QList<SceneFace *> connectedEdges = node->connectedEdges();
            foreach (SceneFace *edge, connectedEdges)
            {
                m_edgePointStart.append(edge->nodeStart()->point());
                m_edgePointEnd.append(edge->nodeEnd()->point());
                m_edgeMarkers.append(edge->markersKeys());
                m_edgeAngle.append(edge->angleValue());

                Agros2D::problem()->scene()->faces->remove(edge);
            }

            Agros2D::problem()->scene()->nodes->remove(node);
        }
    }

    Agros2D::problem()->scene()->stopInvalidating(false);
    Agros2D::problem()->scene()->invalidate();
}

*/
