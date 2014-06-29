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

#include "meshgenerator.h"

#include "util/global.h"
#include "util/conf.h"

#include "scene.h"
#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "scenemarker.h"
#include "logview.h"

#include "hermes2d/module.h"

#include "hermes2d/field.h"
#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"

MeshGenerator::MeshGenerator() : QObject()
{
    Hermes::HermesCommonApi.set_integral_param_value(Hermes::checkMeshesOnLoad, false);
}

MeshGenerator::~MeshGenerator()
{
}

using namespace Hermes::Hermes2D;

void MeshGenerator::elementsSharingNode(MeshElement* e, Point* node, QList<MeshElement*>& elements)
{
    if(!elements.contains(e))
    {
        elements.append(e);
        for(int i = 0; i < 3; i++)
        {
            for(int j = 0; j < 3; j++)
            {
                if(e->neigh[i] != -1 && &this->nodeList[this->elementList[e->neigh[i]].node[j]] == node)
                {
                    elementsSharingNode(&this->elementList[e->neigh[i]], node, elements);
                    break;
                }
            }
        }
    }
}

bool MeshGenerator::getDeterminant(MeshElement* element)
{

    bool is_triangle = element->node[3] == -1;

    if(!is_triangle)
        throw Hermes::Exceptions::Exception("Shifting nodes using get_determinant works only for triangles.");

    double x[3], y[3];
    for(int i = 0; i < 3; i++)
    {
        x[i] = this->nodeList[element->node[i]].x;
        y[i] = this->nodeList[element->node[i]].y;
    }

    double determinant = x[0]*( y[1] - y[2] ) - x[1]*( y[0] - y[2] ) + x[2]*( y[0] - y[1] );

    // std::cout << "E[" << x[0] << "," << y[0] << "],[" << x[1] << "," << y[1] << "],[" << x[2] << "," << y[2] << "]" << ",   D=" << determinant << std::endl;

    return (determinant > 0);
}

// Used in moveNode - this actually moves the node (if it has not been moved before).
void MeshGenerator::performActualNodeMove(Point* node, QList<Point*>& already_moved_nodes, const double x_displacement, const double y_displacement, const double multiplier)
{
    if(already_moved_nodes.contains(node))
        return;
    else
    {
        double x = node->x;
        double y = node->y;

        node->x += multiplier * x_displacement;
        node->y += multiplier * y_displacement;

        std::cout << "Shift: multiplier=" << multiplier << ", x=" << x << ", y=" << y << ", shifted to: x=" << node->x << ", y=" << node->y << "\n";

        already_moved_nodes.append(node);
    }
}

void MeshGenerator::moveNode(MeshElement* element, Point* node, QList<Point*>& already_moved_nodes, const double x_displacement, const double y_displacement, const double multiplier, const QList<std::pair<MeshElement*, bool> >& determinants)
{
    // We have to stop updating somewhere.
    if(multiplier < .01)
        return;

    /* For debugging purposes
    for (int i = 0; i < determinants.count(); i++)
    {
        double x[3], y[3];
        for(int j = 0; j < 3; j++)
        {
            x[j] = this->nodeList[determinants[i].first->node[j]].x;
            y[j] = this->nodeList[determinants[i].first->node[j]].y;
        }
        std::cout << "Old - E[" << x[0] << "," << y[0] << "],[" << x[1] << "," << y[1] << "],[" << x[2] << "," << y[2] << "]" << ",   D=" << determinants[i].second << std::endl;
    }

    */

    performActualNodeMove(node, already_moved_nodes, x_displacement, y_displacement, multiplier);

    for (int i = 0; i < determinants.count(); i++)
    {
        bool new_determinant = getDeterminant(determinants[i].first);
        bool old_determinant = (determinants[i].second > 0);
        // If we broke the element orientation, we need to recursively continue fixing the elements (vertices)
        if(new_determinant != old_determinant)
        {
            for(int j = 0; j < 3; j++)
            {
                if(!already_moved_nodes.contains(&this->nodeList[determinants[i].first->node[j]]))
                {
                    // Prepare the data structures to pass
                    QList<MeshElement*> elements_to_pass;
                    QList<std::pair<MeshElement*, bool> > determinants_to_pass;
                    // Elements sharing the vertex that broke something.
                    elementsSharingNode(determinants[i].first, &this->nodeList[determinants[i].first->node[j]], elements_to_pass);
                    // Calculate the determinants now - to check if this fix won't break something else.
                    for (int ifound_elems = 0; ifound_elems < elements_to_pass.count(); ifound_elems++)
                        determinants_to_pass.append(std::pair<MeshElement*, bool>(elements_to_pass[ifound_elems], getDeterminant(elements_to_pass[ifound_elems])));

                    // We have to stop updating somewhere - this reduces the multiplier multiplying the displacement of vertices.
                    // Very important is the right constant.
                    double new_multiplier = multiplier * 0.9;
                    this->moveNode(determinants[i].first, &this->nodeList[determinants[i].first->node[j]], already_moved_nodes, x_displacement, y_displacement, new_multiplier, determinants_to_pass);
                }
            }
        }
    }
}

void MeshGenerator::moveNodesOnCurvedEdges()
{
    // First move the boundary elements - they are easy, they can not distort any element that lies between the boundary and themselves.
    for (int i = 0; i < edgeList.count(); i++)
    {
        MeshEdge edge = edgeList[i];

        if (edge.marker == -1)
            continue;

        if (!(Agros2D::scene()->edges->at(edge.marker)->angle() > 0.0 && Agros2D::scene()->edges->at(edge.marker)->isCurvilinear()))
            continue;

        // Only boundary now.
        if(edge.neighElem[0] == -1 || edge.neighElem[1] == -1)
        {
            // assert(edge.marker >= 0); // markers changed to marker - 1, check...
            if (edge.marker != -1)
            {
                // curve
                if (Agros2D::scene()->edges->at(edge.marker)->angle() > 0.0 &&
                    Agros2D::scene()->edges->at(edge.marker)->isCurvilinear())
                {
                    // Nodes.
                    Point* node[2] = { &nodeList[edge.node[0]], &nodeList[edge.node[1]]};
                    // Center
                    Point center = Agros2D::scene()->edges->at(edge.marker)->center();
                    // Radius
                    double radius = Agros2D::scene()->edges->at(edge.marker)->radius();

                    // First node handling
                    double pointAngle1 = atan2(center.y - node[0]->y, center.x - node[0]->x) - M_PI;
                    node[0]->x = center.x + radius * cos(pointAngle1);
                    node[0]->y = center.y + radius * sin(pointAngle1);

                    // Second node handling
                    double pointAngle2 = atan2(center.y - node[1]->y, center.x - node[1]->x) - M_PI;
                    node[1]->x = center.x + radius * cos(pointAngle2);
                    node[1]->y = center.y + radius * sin(pointAngle2);
                }
            }
        }
    }

    // Now move the problematic ones
    for (int i = 0; i < edgeList.count(); i++)
    {
        MeshEdge edge = edgeList[i];

        if (edge.marker == -1)
            continue;

        if (!(Agros2D::scene()->edges->at(edge.marker)->angle() > 0.0 && Agros2D::scene()->edges->at(edge.marker)->isCurvilinear()))
            continue;

        // Boundary has been taken care of.
        if(edge.neighElem[0] == -1 || edge.neighElem[1] == -1)
            continue;

        // assert(edge.marker >= 0); // markers changed to marker - 1, check...
        if (edge.marker != -1)
        {
            // curve
            if (Agros2D::scene()->edges->at(edge.marker)->angle() > 0.0 &&
                Agros2D::scene()->edges->at(edge.marker)->isCurvilinear())
            {
                // Nodes.
                Point* node[2] = { &nodeList[edge.node[0]], &nodeList[edge.node[1]]};
                // Center
                Point center = Agros2D::scene()->edges->at(edge.marker)->center();
                // Radius
                double radius = Agros2D::scene()->edges->at(edge.marker)->radius();

                // Handle the nodes recursively using moveNode()
                for(int inode = 0; inode < 2; inode++)
                {
                    double pointAngle = atan2(center.y - node[inode]->y, center.x - node[inode]->x) - M_PI;
                    double x_displacement = center.x + radius * cos(pointAngle) - node[inode]->x;
                    double y_displacement = center.y + radius * sin(pointAngle) - node[inode]->y;

                    // Initialization for the one node algorithm
                    QList<Point*> already_moved_nodes;
                    QList<MeshElement*> elements_to_pass;
                    QList<std::pair<MeshElement*, bool> > determinants_to_pass;

                    // Find elements sharing this node
                    elementsSharingNode(&this->elementList[edge.neighElem[0]], node[inode], elements_to_pass);

                    // Calculate determinants for them
                    for (int ifound_elems = 0; ifound_elems < elements_to_pass.count(); ifound_elems++)
                        determinants_to_pass.append(std::pair<MeshElement*, bool>(elements_to_pass[ifound_elems], getDeterminant(elements_to_pass[ifound_elems])));

                    // Start the algorithm
                    moveNode(&this->elementList[edge.neighElem[0]], node[inode], already_moved_nodes, x_displacement, y_displacement, 1.0, determinants_to_pass);
                }
            }
        }
    }
}

void MeshGenerator::writeTemporaryGlobalMeshToHermes(Hermes::Hermes2D::MeshSharedPtr global_mesh)
{
    // Vertices //
    int vertices_count = nodeList.count();

    // Initialize mesh.
    int size = HashTable::H2D_DEFAULT_HASH_SIZE;
    while (size < 8 * vertices_count)
        size *= 2;
    global_mesh->init(size);

    // Create top-level vertex nodes.
    for (int vertex_i = 0; vertex_i < vertices_count; vertex_i++)
    {
        Node* node = global_mesh->add_node();
        assert(node->id == vertex_i);
        node->ref = TOP_LEVEL_REF;
        node->type = HERMES_TYPE_VERTEX;
        node->bnd = 0;
        node->p1 = node->p2 = -1;
        node->next_hash = nullptr;
        node->x = nodeList[vertex_i].x;
        node->y = nodeList[vertex_i].y;
    }
    global_mesh->ntopvert = vertices_count;

    // Elements //
    int element_count = 0;
    for (int element_i = 0; element_i < elementList.count(); element_i++)
        if(elementList[element_i].isUsed)
            element_count++;

    global_mesh->nbase = global_mesh->nactive = global_mesh->ninitial = element_count;

    for (int element_i = 0; element_i < elementList.count(); element_i++)
    {
        if(!elementList[element_i].isUsed)
            continue;
        // Trim whitespaces.
        int internal_marker =
            global_mesh->element_markers_conversion.insert_marker(QString::number(elementList[element_i].marker).toStdString());

        if (elementList[element_i].isTriangle())
            global_mesh->create_triangle(internal_marker, global_mesh->get_node(elementList[element_i].node[0]), global_mesh->get_node(elementList[element_i].node[1]), global_mesh->get_node(elementList[element_i].node[2]), nullptr);
        else
            global_mesh->create_quad(internal_marker, global_mesh->get_node(elementList[element_i].node[0]), global_mesh->get_node(elementList[element_i].node[1]), global_mesh->get_node(elementList[element_i].node[2]), global_mesh->get_node(elementList[element_i].node[3]), nullptr);
    }

    // Boundaries //
    int edges_count = edgeList.count();

    Node* en;
    for (int edge_i = 0; edge_i < edges_count; edge_i++)
    {
        if(edgeList[edge_i].marker == -1)
            continue;
        int v1 = edgeList[edge_i].node[0];
        int v2 = edgeList[edge_i].node[1];

        en = global_mesh->peek_edge_node(v1, v2);
        if (en == nullptr)
            throw Hermes::Exceptions::MeshLoadFailureException("Boundary data #%d: edge %d-%d does not exist.", edge_i, v1, v2);

        int marker = global_mesh->boundary_markers_conversion.insert_marker(QString::number(edgeList[edge_i].marker).toStdString());

        en->marker = marker;
    }

    Node* node;
    for_all_edge_nodes(node, global_mesh)
    {
        if (node->ref < 2)
        {
            global_mesh->get_node(node->p1)->bnd = 1;
            global_mesh->get_node(node->p2)->bnd = 1;
            node->bnd = 1;
        }
    }

    // Curves //
    // Just Arcs //
    for (int edge_i = 0; edge_i < edgeList.count(); edge_i++)
    {
        if (edgeList[edge_i].marker != -1)
        {
            // curve
            if (Agros2D::scene()->edges->at(edgeList[edge_i].marker)->angle() > 0.0 &&
                Agros2D::scene()->edges->at(edgeList[edge_i].marker)->isCurvilinear())
            {
                // load the control points, knot vector, etc.
                Node* en;
                int p1 = edgeList[edge_i].node[0], p2 = edgeList[edge_i].node[1];

                int segments = Agros2D::scene()->edges->at(edgeList[edge_i].marker)->segments();

                // subdivision angle and chord
                double theta = deg2rad(Agros2D::scene()->edges->at(edgeList[edge_i].marker)->angle()) / double(segments);
                double chord = 2 * Agros2D::scene()->edges->at(edgeList[edge_i].marker)->radius() * sin(theta / 2.0);

                // length of short chord
                double chordShort = (nodeList[edgeList[edge_i].node[1]] - nodeList[edgeList[edge_i].node[0]]).magnitude();

                // direction
                Point center = Agros2D::scene()->edges->at(edgeList[edge_i].marker)->center();
                int direction = (((nodeList[edgeList[edge_i].node[0]].x - center.x)*(nodeList[edgeList[edge_i].node[1]].y - center.y) -
                    (nodeList[edgeList[edge_i].node[0]].y - center.y)*(nodeList[edgeList[edge_i].node[1]].x - center.x)) > 0) ? 1 : -1;

                double angle = direction * theta * chordShort / chord;

                Hermes::Hermes2D::Curve* curve = MeshUtil::load_arc(global_mesh, edge_i, &en, p1, p2, rad2deg(angle));

                // assign the arc to the elements sharing the edge node
                MeshUtil::assign_curve(en, curve, p1, p2);
            }
        }
    }

    // update refmap coeffs of curvilinear elements
    Element* e;
    for_all_used_elements(e, global_mesh)
    {
        if (e->cm != nullptr)
        {
            if (e->area < 0)
                throw AgrosMeshException(tr("Mesh is corrupted (some areas have a negative area)"));

            e->cm->update_refmap_coeffs(e);
        }
    }
}

void MeshGenerator::fillNeighborStructures()
{
    QList<QSet<int> > vertexElements;
    vertexElements.reserve(nodeList.count());
    for (int i = 0; i < nodeList.count(); i++)
        vertexElements.push_back(QSet<int>());

    for (int i = 0; i < elementList.count(); i++)
        if (elementList[i].isUsed)
            for (int elemNode = 0; elemNode < (elementList[i].isTriangle() ? 3 : 4); elemNode++)
                vertexElements[elementList[i].node[elemNode]].insert(i);

    for (int i = 0; i < edgeList.count(); i++)
    {
        if (edgeList[i].isUsed && edgeList[i].marker != -1)
        {
            QSet<int> neighbours = vertexElements[edgeList[i].node[0]];
            neighbours.intersect(vertexElements[edgeList[i].node[1]]);
            assert((neighbours.size() > 0) && (neighbours.size() <= 2));
            edgeList[i].neighElem[0] = neighbours.values()[0];
            if (neighbours.size() == 2)
                edgeList[i].neighElem[1] = neighbours.values()[1];
        }
    }
}

void MeshGenerator::getDataCountsForSingleSubdomain(FieldInfo* fieldInfo, int& element_number_count, int& boundary_edge_number_count, int& inner_edge_number_count)
{
    for (int i = 0; i < elementList.count(); i++)
        if (elementList[i].isUsed && (Agros2D::scene()->labels->at(elementList[i].marker)->marker(fieldInfo) != SceneMaterialContainer::getNone(fieldInfo)))
            element_number_count++;

    QList<int> unassignedEdges;
    for (int i = 0; i < edgeList.count(); i++)
    {
        if (edgeList[i].isUsed && edgeList[i].marker != -1)
        {
            int numNeighWithField = 0;
            for (int neigh_i = 0; neigh_i < 2; neigh_i++)
            {
                int neigh = edgeList[i].neighElem[neigh_i];
                if (neigh != -1)
                {
                    if (Agros2D::scene()->labels->at(elementList[neigh].marker)->marker(fieldInfo)
                        != SceneMaterialContainer::getNone(fieldInfo))
                        numNeighWithField++;
                }
            }

            // edge has boundary condition prescribed for this field
            bool hasFieldBoundaryCondition = (Agros2D::scene()->edges->at(edgeList[i].marker)->hasMarker(fieldInfo)
                && (Agros2D::scene()->edges->at(edgeList[i].marker)->marker(fieldInfo) != SceneBoundaryContainer::getNone(fieldInfo)));

            if (numNeighWithField == 1)
            {
                // edge is on "boundary" of the field, should have boundary condition prescribed

                if (!hasFieldBoundaryCondition)
                    if (!unassignedEdges.contains(edgeList[i].marker))
                        unassignedEdges.append(edgeList[i].marker);

                boundary_edge_number_count++;
            }
            else if (numNeighWithField == 2)
            {
                // todo: we could enforce not to have boundary conditions prescribed inside:
                // assert(!hasFieldBoundaryCondition);
                inner_edge_number_count;;
            }
        }
    }
}

void MeshGenerator::writeToHermes()
{
    this->m_meshes.clear();
    foreach(FieldInfo* fieldInfo, Agros2D::problem()->fieldInfos())
    {
        Hermes::Hermes2D::MeshSharedPtr mesh(new Hermes::Hermes2D::Mesh());
        m_meshes.push_back(mesh);
    }

    try
    {
        // this->fillNeighborStructures();
        // this->moveNodesOnCurvedEdges();

        MeshSharedPtr global_mesh(new Mesh);

        this->writeTemporaryGlobalMeshToHermes(global_mesh);

        int subdomains_count;
        if (Agros2D::problem()->fieldInfos().isEmpty())
            subdomains_count = 1;
        else
            subdomains_count = Agros2D::problem()->fieldInfos().size();

        Element* e;
        for (int subdomains_i = 0; subdomains_i < subdomains_count; subdomains_i++)
        {
            for (int element_i = 0; element_i < elementList.count(); element_i++)
            {
                if(elementList[element_i].isUsed)
                    m_meshes[subdomains_i]->element_markers_conversion.insert_marker(QString::number(elementList[element_i].marker).toStdString());
            }

            for (int edge_i = 0; edge_i < edgeList.count(); edge_i++)
            {
                m_meshes[subdomains_i]->boundary_markers_conversion.insert_marker(QString::number(edgeList[edge_i].marker).toStdString());
            }
        }

        Hermes::vector<FieldInfo*> fieldInfoVector;
        foreach(FieldInfo* fieldInfo, Agros2D::problem()->fieldInfos())
        {
            fieldInfoVector.push_back(fieldInfo);
        }

        for (int subdomains_i = 0; subdomains_i < subdomains_count; subdomains_i++)
        {
            FieldInfo* fieldInfo = fieldInfoVector[subdomains_i];
            int vertex_number_count = global_mesh->get_num_vertex_nodes();
            int element_number_count = 0;
            int boundary_edge_number_count = 0;
            int inner_edge_number_count = 0;

            this->getDataCountsForSingleSubdomain(fieldInfo, element_number_count, boundary_edge_number_count, inner_edge_number_count);

            // copy the whole mesh if the subdomain is the whole mesh.
            if (element_number_count == 0 || element_number_count == global_mesh->get_num_elements())
            {
                m_meshes[subdomains_i]->copy(global_mesh);
            }
            else
            {
                // Initialize mesh.
                int size = HashTable::H2D_DEFAULT_HASH_SIZE;
                while (size < 8 * vertex_number_count)
                    size *= 2;
                m_meshes[subdomains_i]->init(size);

                // Create top-level vertex nodes.
                for (int vertex_i = 0; vertex_i < vertex_number_count; vertex_i++)
                {
                    Node* node = m_meshes[subdomains_i]->add_node();
                    assert(node->id == vertex_i);
                    node->ref = TOP_LEVEL_REF;
                    node->type = HERMES_TYPE_VERTEX;
                    node->bnd = 0;
                    node->p1 = node->p2 = -1;
                    node->next_hash = nullptr;
                    node->x = nodeList[vertex_i].x;
                    node->y = nodeList[vertex_i].y;
                }
                m_meshes[subdomains_i]->ntopvert = vertex_number_count;

                for (int element_i = 0; element_i < elementList.count(); element_i++)
                {
                    if (elementList[element_i].isUsed && (Agros2D::scene()->labels->at(elementList[element_i].marker)->marker(fieldInfo) != SceneMaterialContainer::getNone(fieldInfo)))
                    {
                        int internal_marker = m_meshes[subdomains_i]->element_markers_conversion.get_internal_marker(QString::number(elementList[element_i].marker).toStdString()).marker;
                        if (elementList[element_i].isTriangle())
                            m_meshes[subdomains_i]->create_triangle(internal_marker, m_meshes[subdomains_i]->get_node(elementList[element_i].node[0]), m_meshes[subdomains_i]->get_node(elementList[element_i].node[1]), m_meshes[subdomains_i]->get_node(elementList[element_i].node[2]), nullptr, element_i);
                        else
                            m_meshes[subdomains_i]->create_quad(internal_marker, m_meshes[subdomains_i]->get_node(elementList[element_i].node[0]), m_meshes[subdomains_i]->get_node(elementList[element_i].node[1]), m_meshes[subdomains_i]->get_node(elementList[element_i].node[2]), m_meshes[subdomains_i]->get_node(elementList[element_i].node[3]), nullptr, element_i);
                    }
                    else
                    {
                        m_meshes[subdomains_i]->elements.skip_slot()->cm = nullptr;
                    }
                }

                m_meshes[subdomains_i]->nbase = elementList.count();
                m_meshes[subdomains_i]->nactive = m_meshes[subdomains_i]->ninitial = element_number_count;

                QList<int> unassignedEdges;
                for (int edge_i = 0; edge_i < edgeList.count(); edge_i++)
                {
                    if (edgeList[edge_i].isUsed && edgeList[edge_i].marker != -1)
                    {
                        int numNeighWithField = 0;
                        for (int neigh_i = 0; neigh_i < 2; neigh_i++)
                        {
                            int neigh = edgeList[edge_i].neighElem[neigh_i];
                            if (neigh != -1)
                            {
                                if (Agros2D::scene()->labels->at(elementList[neigh].marker)->marker(fieldInfo)
                                    != SceneMaterialContainer::getNone(fieldInfo))
                                    numNeighWithField++;
                            }
                        }

                        if (numNeighWithField == 1)
                        {

                            // edge has boundary condition prescribed for this field
                            bool hasFieldBoundaryCondition = (Agros2D::scene()->edges->at(edgeList[edge_i].marker)->hasMarker(fieldInfo)
                                && (Agros2D::scene()->edges->at(edgeList[edge_i].marker)->marker(fieldInfo) != SceneBoundaryContainer::getNone(fieldInfo)));

                            // edge is on "boundary" of the field, should have boundary condition prescribed
                            if (!hasFieldBoundaryCondition)
                                if (!unassignedEdges.contains(edgeList[edge_i].marker))
                                    unassignedEdges.append(edgeList[edge_i].marker);

                            Node* en = m_meshes[subdomains_i]->peek_edge_node(edgeList[edge_i].node[0], edgeList[edge_i].node[1]);
                            int marker = m_meshes[subdomains_i]->boundary_markers_conversion.insert_marker(QString::number(edgeList[edge_i].marker).toStdString());
                            en->marker = marker;
                        }

                        else if (numNeighWithField == 2)
                        {
                            // todo: we could enforce not to have boundary conditions prescribed inside:
                            // assert(!hasFieldBoundaryCondition);

                            Node* en = m_meshes[subdomains_i]->peek_edge_node(edgeList[edge_i].node[0], edgeList[edge_i].node[1]);
                            int marker = m_meshes[subdomains_i]->boundary_markers_conversion.insert_marker(QString::number(edgeList[edge_i].marker).toStdString());
                            en->marker = marker;
                        }
                    }
                }

                // not assigned boundary
                if (unassignedEdges.count() > 0)
                {
                    QString list;
                    foreach(int index, unassignedEdges)
                        list += QString::number(index) + ", ";

                    Agros2D::log()->printError(tr("Mesh generator"), tr("Boundary condition for %1 is not assigned on following edges: %2").arg(fieldInfo->name()).arg(list.left(list.count() - 2)));
                }

                Node* node;
                for_all_edge_nodes(node, m_meshes[subdomains_i])
                {
                    if (node->ref < 2)
                    {
                        m_meshes[subdomains_i]->get_node(node->p1)->bnd = 1;
                        m_meshes[subdomains_i]->get_node(node->p2)->bnd = 1;
                        node->bnd = 1;
                    }
                }

                // Curves //
                // Just Arcs //
                for (int edge_i = 0; edge_i < edgeList.count(); edge_i++)
                {
                    if (false)//edgeList[edge_i].marker != -1)
                    {
                        // curve
                        if (Agros2D::scene()->edges->at(edgeList[edge_i].marker)->angle() > 0.0 &&
                            Agros2D::scene()->edges->at(edgeList[edge_i].marker)->isCurvilinear())
                        {
                            // load the control points, knot vector, etc.
                            Node* en;
                            int p1 = edgeList[edge_i].node[0], p2 = edgeList[edge_i].node[1];

                            int segments = Agros2D::scene()->edges->at(edgeList[edge_i].marker)->segments();

                            // subdivision angle and chord
                            double theta = deg2rad(Agros2D::scene()->edges->at(edgeList[edge_i].marker)->angle()) / double(segments);
                            double chord = 2 * Agros2D::scene()->edges->at(edgeList[edge_i].marker)->radius() * sin(theta / 2.0);

                            // length of short chord
                            double chordShort = (nodeList[edgeList[edge_i].node[1]] - nodeList[edgeList[edge_i].node[0]]).magnitude();

                            // direction
                            Point center = Agros2D::scene()->edges->at(edgeList[edge_i].marker)->center();
                            int direction = (((nodeList[edgeList[edge_i].node[0]].x - center.x)*(nodeList[edgeList[edge_i].node[1]].y - center.y) -
                                (nodeList[edgeList[edge_i].node[0]].y - center.y)*(nodeList[edgeList[edge_i].node[1]].x - center.x)) > 0) ? 1 : -1;

                            double angle = direction * theta * chordShort / chord;

                            Hermes::Hermes2D::Curve* curve = MeshUtil::load_arc(m_meshes[subdomains_i], edge_i, &en, p1, p2, rad2deg(angle), true);

                            // assign the arc to the elements sharing the edge node
                            if (curve)
                                MeshUtil::assign_curve(en, curve, p1, p2);
                        }
                    }
                }

                // update refmap coeffs of curvilinear elements
                Element* e;
                for_all_used_elements(e, m_meshes[subdomains_i])
                {
                    if (e->cm != nullptr)
                        e->cm->update_refmap_coeffs(e);
                }
            }

            m_meshes[subdomains_i]->seq = g_mesh_seq++;

            if (Hermes::HermesCommonApi.get_integral_param_value(Hermes::checkMeshesOnLoad))
                m_meshes[subdomains_i]->initial_single_check();
        }

        // save mesh file
        Hermes::Hermes2D::MeshReaderH2DXML meshloader;
        meshloader.set_validation(false);
        try
        {
            QString fn = QString("%1/initial.msh").arg(cacheProblemDir());
            meshloader.save(compatibleFilename(fn).toStdString().c_str(), m_meshes);
        }
        catch (Hermes::Exceptions::MeshLoadFailureException& e)
        {
            qDebug() << e.what();
            throw;
        }
    }
    catch (Hermes::Exceptions::Exception& e)
    {
        Agros2D::log()->printError(tr("Mesh generator"), tr("Failed: %1").arg(e.what()));
    }
    catch (std::exception& e)
    {
        Agros2D::log()->printError(tr("Mesh generator"), tr("Failed: %1").arg(e.what()));
    }
}

bool MeshGenerator::prepare()
{
    try
    {
        Agros2D::scene()->loopsInfo()->processLoops();
    }
    catch (AgrosMeshException& ame)
    {
        Agros2D::log()->printError(tr("Mesh generator"), ame.toString());
        return false;
    }

    QFile::remove(tempProblemFileName() + ".msh");

    return true;
}
