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

#include "swig_geometry.h"
#include "solver/problem.h"
#include "solver/problem_config.h"
#include "scenemarker.h"
#include "scene.h"

int SwigGeometry::nodesCount() const
{
    return Agros::problem()->scene()->nodes->count();
}

int SwigGeometry::edgesCount() const
{
    return Agros::problem()->scene()->faces->count();
}

int SwigGeometry::labelsCount() const
{
    return Agros::problem()->scene()->labels->count();
}

int SwigGeometry::addNode(const double x, const double y)
{
    return addNode(PointValue(Value(Agros::problem(), x),
                              Value(Agros::problem(), y)));
}

int SwigGeometry::addNode(std::string x, std::string y)
{
    return addNode(PointValue(Value(Agros::problem(), QString::fromStdString(x)),
                              Value(Agros::problem(), QString::fromStdString(y))));
}

int SwigGeometry::addNode(PointValue point)
{
    if (Agros::problem()->config()->coordinateType() == CoordinateType_Axisymmetric && point.numberX() < 0.0)
        throw out_of_range(QObject::tr("Radial component must be greater then or equal to zero.").toStdString());

    foreach (SceneNode *node, Agros::problem()->scene()->nodes->items())
    {
        if (node->point() == point.point())
            throw logic_error(QObject::tr("Node already exist.").toStdString());
    }

    SceneNode *node = Agros::problem()->scene()->addNode(new SceneNode(Agros::problem()->scene(), point));
    return Agros::problem()->scene()->nodes->items().indexOf(node);
}

int SwigGeometry::addEdge(double x1, double y1, double x2, double y2,
                          const map<string, string> &boundaries,
                          double angle, int segments, int curvilinear)
{
    qInfo() << "addEdge" << boundaries.size();

    PointValue pointStart = PointValue(Value(Agros::problem(), x1),
                                       Value(Agros::problem(), y1));
    PointValue pointEnd = PointValue(Value(Agros::problem(), x2),
                                     Value(Agros::problem(), y2));
    Value valueAngle = Value(Agros::problem(), angle);

    qInfo() << "addEdge" << boundaries.size() << " ok";

    return addEdge(pointStart, pointEnd, valueAngle, segments, curvilinear, map<std::string, int>(), map<string, string>());
}

int SwigGeometry::addEdge(std::string x1, std::string y1, std::string x2, std::string y2, std::string angle, int segments, int curvilinear,
                          const map<std::string, int> &refinements, const map<std::string, std::string> &boundaries)
{
    PointValue pointStart = PointValue(Value(Agros::problem(), QString::fromStdString(x1)),
                                       Value(Agros::problem(), QString::fromStdString(y1)));
    PointValue pointEnd = PointValue(Value(Agros::problem(), QString::fromStdString(x2)),
                                     Value(Agros::problem(), QString::fromStdString(y2)));
    Value valueAngle = Value(Agros::problem(), QString::fromStdString(angle));

    return addEdge(pointStart, pointEnd, valueAngle, segments, curvilinear, refinements, boundaries);
}

int SwigGeometry::addEdge(PointValue pointStart, PointValue pointEnd, Value valueAngle, int segments, int curvilinear,
                          const map<std::string, int> &refinements, const map<std::string, std::string> &boundaries)
{    
    if (Agros::problem()->config()->coordinateType() == CoordinateType_Axisymmetric &&
            (pointStart.numberX() < 0.0 || pointEnd.numberX() < 0.0))
        throw out_of_range(QObject::tr("Radial component must be greater then or equal to zero.").toStdString());

    testAngle(valueAngle.number());
    testSegments(segments);

    foreach (SceneFace *edge, Agros::problem()->scene()->faces->items())
    {
        if (almostEqualRelAndAbs(edge->nodeStart()->point().x, pointStart.numberX(), POINT_ABS_ZERO, POINT_REL_ZERO) &&
                almostEqualRelAndAbs(edge->nodeEnd()->point().x, pointEnd.numberX(), POINT_ABS_ZERO, POINT_REL_ZERO) &&
                almostEqualRelAndAbs(edge->nodeStart()->point().y, pointStart.numberY(), POINT_ABS_ZERO, POINT_REL_ZERO) &&
                almostEqualRelAndAbs(edge->nodeEnd()->point().y, pointEnd.numberY(), POINT_ABS_ZERO, POINT_REL_ZERO))
            throw logic_error(QObject::tr("Edge already exist.").toStdString());
    }

    SceneNode *nodeStart = new SceneNode(Agros::problem()->scene(), pointStart);
    nodeStart = Agros::problem()->scene()->addNode(nodeStart);
    SceneNode *nodeEnd = new SceneNode(Agros::problem()->scene(), pointEnd);
    nodeEnd = Agros::problem()->scene()->addNode(nodeEnd);
    SceneFace *edge = new SceneFace(Agros::problem()->scene(), nodeStart, nodeEnd, valueAngle, segments, curvilinear);

    try
    {
        setBoundaries(edge, boundaries);
    }
    catch (std::exception& e)
    {
        delete edge;
        throw;
    }

    Agros::problem()->scene()->addFace(edge);

    return Agros::problem()->scene()->faces->items().indexOf(edge);
}

int SwigGeometry::addEdgeByNodes(int nodeStartIndex, int nodeEndIndex, std::string angle, int segments, int curvilinear,
                                 const map<std::string, int> &refinements, const map<std::string, std::string> &boundaries)
{
    Value valueAngle = Value(Agros::problem(), QString::fromStdString(angle));

    if (Agros::problem()->scene()->nodes->isEmpty())
        throw out_of_range(QObject::tr("Geometry does not contain nodes.").toStdString());

    if (nodeStartIndex == nodeEndIndex)
        throw logic_error(QObject::tr("Start node index is the same as index of end node.").toStdString());

    if (nodeStartIndex > (Agros::problem()->scene()->nodes->length() - 1) || nodeStartIndex < 0)
        throw out_of_range(QObject::tr("Node with index '%1' does not exist.").arg(nodeStartIndex).toStdString());
    if (nodeEndIndex > (Agros::problem()->scene()->nodes->length() - 1) || nodeEndIndex < 0)
        throw out_of_range(QObject::tr("Node with index '%1' does not exist.").arg(nodeEndIndex).toStdString());

    testAngle(valueAngle.number());
    testSegments(segments);

    foreach (SceneFace *edge, Agros::problem()->scene()->faces->items())
    {
        if (Agros::problem()->scene()->nodes->items().indexOf(edge->nodeStart()) == nodeStartIndex &&
                Agros::problem()->scene()->nodes->items().indexOf(edge->nodeEnd()) == nodeEndIndex)
            throw logic_error(QObject::tr("Edge already exist.").toStdString());
    }

    SceneFace *edge = new SceneFace(Agros::problem()->scene(),
                                    Agros::problem()->scene()->nodes->at(nodeStartIndex),
                                    Agros::problem()->scene()->nodes->at(nodeEndIndex),
                                    valueAngle, segments, curvilinear);

    try
    {
        setBoundaries(edge, boundaries);
    }
    catch (std::exception& e)
    {
        delete edge;
        throw;
    }

    Agros::problem()->scene()->addFace(edge);

    return Agros::problem()->scene()->faces->items().indexOf(edge);
}

void SwigGeometry::modifyEdge(int index, std::string angle, int segments, int isCurvilinear, const map<std::string, int> &refinements, const map<std::string, std::string> &boundaries)
{
    Value valueAngle = Value(Agros::problem(), QString::fromStdString(angle));

    if (Agros::problem()->scene()->faces->isEmpty())
        throw out_of_range(QObject::tr("No edges are defined.").toStdString());

    if (index < 0 || index >= Agros::problem()->scene()->faces->length())
        throw out_of_range(QObject::tr("Edge index must be between 0 and '%1'.").arg(Agros::problem()->scene()->faces->length()-1).toStdString());

    testAngle(valueAngle.number());
    testSegments(segments);

    SceneFace *edge = Agros::problem()->scene()->faces->items().at(index);

    edge->setAngleValue(valueAngle);
    edge->setSegments(segments);
    edge->setCurvilinear(isCurvilinear);

    setBoundaries(edge, boundaries);

    Agros::problem()->scene()->invalidate();
}

void SwigGeometry::testAngle(double angle) const
{
    if (angle < 0.0 || angle > 90.0)
        throw out_of_range(QObject::tr("Angle '%1' is out of range.").arg(angle).toStdString());
}

void SwigGeometry::testSegments(int segments) const
{
    if (segments < 3 || segments > 20)
        throw out_of_range(QObject::tr("Segments '%1' is out of range.").arg(segments).toStdString());
}

void SwigGeometry::setBoundaries(SceneFace *edge, const map<std::string, std::string> &boundaries)
{
    for (map<std::string, std::string>::const_iterator i = boundaries.begin(); i != boundaries.end(); ++i)
    {
        QString field = QString::fromStdString((*i).first);
        QString marker = QString::fromStdString((*i).second);

        if (!Agros::problem()->hasField(field))
            throw invalid_argument(QObject::tr("Invalid field id '%1'.").arg(field).toStdString());

        bool assigned = false;
        foreach (SceneBoundary *boundary, Agros::problem()->scene()->boundaries->filter(Agros::problem()->fieldInfo(field)).items())
        {
            if (boundary->name() == marker)
            {
                assigned = true;
                edge->addMarker(boundary);
                break;
            }
        }

        if (!assigned)
            throw invalid_argument(QObject::tr("Boundary condition '%1' doesn't exists.").arg(marker).toStdString());
    }
}

int SwigGeometry::addLabel(double x, double y, const map<std::string, std::string> &materials, double area)
{
    PointValue point = PointValue(Value(Agros::problem(), x),
                                  Value(Agros::problem(), y));

    return -1;
    return addLabel(point, map<std::string, int>(), area, map<std::string, int>(), materials);
}

int SwigGeometry::addLabel(PointValue point, const map<std::string, int> &refinements, double area,
                           const map<std::string, int> &orders, const map<std::string, std::string> &materials)
{
    if (Agros::problem()->config()->coordinateType() == CoordinateType_Axisymmetric && point.numberX() < 0.0)
        throw out_of_range(QObject::tr("Radial component must be greater then or equal to zero.").toStdString());

    if (area < 0.0)
        throw out_of_range(QObject::tr("Area must be positive.").toStdString());

    foreach (SceneLabel *label, Agros::problem()->scene()->labels->items())
    {
        if (label->point() == point.point())
            throw logic_error(QObject::tr("Label already exist.").toStdString());
    }

    SceneLabel *label = new SceneLabel(Agros::problem()->scene(), point, area);

    try
    {
        setMaterials(label, materials);
        setRefinements(label, refinements);
        setPolynomialOrders(label, orders);
    }
    catch (std::exception& e)
    {
        delete label;
        throw;
    }

    Agros::problem()->scene()->addLabel(label);

    return Agros::problem()->scene()->labels->items().indexOf(label);
}

void SwigGeometry::modifyLabel(int index, double area, const map<std::string, int> &refinements,
                               const map<std::string, int> &orders, const map<std::string, std::string> &materials)
{
    if (Agros::problem()->scene()->labels->isEmpty())
        throw out_of_range(QObject::tr("No labels are defined.").toStdString());

    if (index < 0 || index >= Agros::problem()->scene()->labels->length())
        throw out_of_range(QObject::tr("Label index must be between 0 and '%1'.").arg(Agros::problem()->scene()->labels->length()-1).toStdString());

    if (area < 0.0)
        throw out_of_range(QObject::tr("Area must be positive.").toStdString());

    SceneLabel *label = Agros::problem()->scene()->labels->at(index);

    label->setArea(area);
    setMaterials(label, materials);
    setRefinements(label, refinements);
    setPolynomialOrders(label, orders);

    Agros::problem()->scene()->invalidate();
}

void SwigGeometry::setMaterials(SceneLabel *label, const map<std::string, std::string> &materials)
{
    for( map<std::string, std::string>::const_iterator i = materials.begin(); i != materials.end(); ++i)
    {
        QString field = QString::fromStdString((*i).first);
        QString marker = QString::fromStdString((*i).second);

        if (!Agros::problem()->hasField(field))
            throw invalid_argument(QObject::tr("Invalid field id '%1'.").arg(field).toStdString());

        if (marker != "none")
        {

            bool assigned = false;
            foreach (SceneMaterial *material, Agros::problem()->scene()->materials->filter(Agros::problem()->fieldInfo(field)).items())
            {
                if ((material->fieldId() == field) && (material->name() == marker))
                {
                    assigned = true;
                    label->addMarker(material);
                    break;
                }
            }

            if (!assigned)
                throw invalid_argument(QObject::tr("Material '%1' doesn't exists.").arg(marker).toStdString());
        }
    }
}

void SwigGeometry::setRefinements(SceneLabel *label, const map<std::string, int> &refinements)
{
    for (map<std::string, int>::const_iterator i = refinements.begin(); i != refinements.end(); ++i)
    {
        QString field = QString::fromStdString((*i).first);
        int refinement = (*i).second;

        if (!Agros::problem()->hasField(field))
            throw invalid_argument(QObject::tr("Invalid field id '%1'.").arg(field).toStdString());

        if ((refinement < 0) || (refinement > 10))
            throw out_of_range(QObject::tr("Number of refinements '%1' is out of range (0 - 10).").arg(refinement).toStdString());

        Agros::problem()->fieldInfo(field)->setLabelRefinement(label, refinement);
    }
}

void SwigGeometry::setPolynomialOrders(SceneLabel *label, const map<std::string, int> &orders)
{
    for (map<std::string, int>::const_iterator i = orders.begin(); i != orders.end(); ++i)
    {
        QString field = QString::fromStdString((*i).first);
        int order = (*i).second;

        if (!Agros::problem()->hasField(field))
            throw invalid_argument(QObject::tr("Invalid field id '%1'.").arg(field).toStdString());

        if ((order < 1) || (order > 10))
            throw out_of_range(QObject::tr("Polynomial order '%1' is out of range (1 - 10).").arg(order).toStdString());

        Agros::problem()->fieldInfo(field)->setLabelPolynomialOrder(label, order);
    }
}

void SwigGeometry::removeNodes(const vector<int> &nodes)
{    
    vector<int> tmp = nodes;
    if (tmp.empty())
    {
        // add all nodes
        for (int i = 0; i < Agros::problem()->scene()->nodes->count(); i++)
            tmp.push_back(i);
    }

    if (!tmp.empty())
    {
        for (vector<int>::const_iterator it = tmp.begin(); it != tmp.end(); ++it)
        {
            if ((*it >= 0) && (*it < Agros::problem()->scene()->nodes->length()))
            {
                SceneNode *node = Agros::problem()->scene()->nodes->at(*it);
                QList<SceneFace *> connectedEdges = node->connectedEdges();
                foreach (SceneFace *edge, connectedEdges)
                    Agros::problem()->scene()->faces->remove(edge);

                Agros::problem()->scene()->nodes->remove(node);
            }
            else
                throw out_of_range(QObject::tr("Node index must be between 0 and '%1'.").arg(Agros::problem()->scene()->nodes->length()-1).toStdString());
        }

        Agros::problem()->scene()->invalidate();
    }
}

void SwigGeometry::removeEdges(const vector<int> &edges)
{
    vector<int> tmp = edges;
    if (tmp.empty())
    {
        // add all nodes
        for (int i = 0; i < Agros::problem()->scene()->faces->count(); i++)
            tmp.push_back(i);
    }

    if (!tmp.empty())
    {
        for (vector<int>::const_iterator it = tmp.begin(); it != tmp.end(); ++it)
        {
            if ((*it >= 0) && (*it < Agros::problem()->scene()->faces->length()))
            {
                SceneFace *face = Agros::problem()->scene()->faces->at(*it);
                Agros::problem()->scene()->faces->remove(face);
            }
            else
                throw out_of_range(QObject::tr("Edge index must be between 0 and '%1'.").arg(Agros::problem()->scene()->faces->length()-1).toStdString());
        }

        Agros::problem()->scene()->invalidate();
    }
}

void SwigGeometry::removeLabels(const vector<int> &labels)
{
    vector<int> tmp = labels;
    if (tmp.empty())
    {
        // add all nodes
        for (int i = 0; i < Agros::problem()->scene()->labels->count(); i++)
            tmp.push_back(i);
    }

    if (!tmp.empty())
    {
        for (vector<int>::const_iterator it = tmp.begin(); it != tmp.end(); ++it)
        {
            if ((*it >= 0) && (*it < Agros::problem()->scene()->labels->length()))
            {
                SceneLabel *label = Agros::problem()->scene()->labels->at(*it);
                Agros::problem()->scene()->labels->remove(label);
            }
            else
                throw out_of_range(QObject::tr("Label index must be between 0 and '%1'.").arg(Agros::problem()->scene()->labels->length()-1).toStdString());
        }

        Agros::problem()->scene()->invalidate();
    }
}

void SwigGeometry::selectNodes(const vector<int> &nodes)
{
    Agros::problem()->scene()->selectNone();

    if (!nodes.empty())
    {
        for (vector<int>::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
        {
            if ((*it >= 0) && (*it < Agros::problem()->scene()->nodes->length()))
                Agros::problem()->scene()->nodes->at(*it)->setSelected(true);
            else
                throw out_of_range(QObject::tr("Node index must be between 0 and '%1'.").arg(Agros::problem()->scene()->nodes->length()-1).toStdString());
        }
    }
    else
    {
        Agros::problem()->scene()->selectAll(SceneGeometryMode_OperateOnNodes);
    }
}

void SwigGeometry::selectEdges(const vector<int> &edges)
{
    Agros::problem()->scene()->selectNone();

    if (!edges.empty())
    {
        for (vector<int>::const_iterator it = edges.begin(); it != edges.end(); ++it)
        {
            if ((*it >= 0) && (*it < Agros::problem()->scene()->faces->length()))
                Agros::problem()->scene()->faces->at(*it)->setSelected(true);
            else
                throw out_of_range(QObject::tr("Edge index must be between 0 and '%1'.").arg(Agros::problem()->scene()->faces->length()-1).toStdString());
        }
    }
    else
    {
        Agros::problem()->scene()->selectAll(SceneGeometryMode_OperateOnEdges);
    }
}

void SwigGeometry::selectLabels(const vector<int> &labels)
{
    Agros::problem()->scene()->selectNone();

    if (!labels.empty())
    {
        for (vector<int>::const_iterator it = labels.begin(); it != labels.end(); ++it)
        {
            if ((*it >= 0) && (*it < Agros::problem()->scene()->labels->length()))
                Agros::problem()->scene()->labels->at(*it)->setSelected(true);
            else
                throw out_of_range(QObject::tr("Label index must be between 0 and '%1'.").arg(Agros::problem()->scene()->labels->length()-1).toStdString());
        }
    }
    else
    {
        Agros::problem()->scene()->selectAll(SceneGeometryMode_OperateOnLabels);
    }
}

void SwigGeometry::selectNodeByPoint(double x, double y)
{
    Agros::problem()->scene()->selectNone();

    SceneNode *node = SceneNode::findClosestNode(Agros::problem()->scene(), Point(x, y));
    if (node)
        node->setSelected(true);
    else
        throw logic_error(QObject::tr("There are no nodes around the point [%1, %2].").arg(x).arg(y).toStdString());
}

void SwigGeometry::selectEdgeByPoint(double x, double y)
{
    Agros::problem()->scene()->selectNone();

    SceneFace *edge = SceneFace::findClosestFace(Agros::problem()->scene(), Point(x, y));
    if (edge)
        edge->setSelected(true);
    else
        throw logic_error(QObject::tr("There are no edges around the point [%1, %2].").arg(x).arg(y).toStdString());
}

void SwigGeometry::selectLabelByPoint(double x, double y)
{
    Agros::problem()->scene()->selectNone();

    SceneLabel *label = SceneLabel::findClosestLabel(Agros::problem()->scene(), Point(x, y));
    if (label)
        label->setSelected(true);
    else
        throw logic_error(QObject::tr("There are no labels around the point [%1, %2].").arg(x).arg(y).toStdString());
}

void SwigGeometry::selectNone()
{
    Agros::problem()->scene()->selectNone();
}

void SwigGeometry::removeSelection()
{
    // Agros::problem()->scene()->deleteSelected();
}

void SwigGeometry::exportVTK(const std::string &fileName) const
{
    Agros::problem()->scene()->exportVTKGeometry(QString::fromStdString(fileName));
}

void SwigGeometry::exportSVG(const std::string &fileName) const
{
    QString geometry = generateSvgGeometry(Agros::problem()->scene()->faces->items());
    writeStringContent(QString::fromStdString(fileName), geometry);
}

std::string SwigGeometry::exportSVG() const
{
    return generateSvgGeometry(Agros::problem()->scene()->faces->items()).toStdString();
}
