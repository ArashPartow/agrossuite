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

// deal.ii
#include <deal.II/grid/tria.h>
#include <deal.II/dofs/dof_handler.h>

#include <deal.II/grid/grid_generator.h>
#include <deal.II/grid/grid_in.h>
#include <deal.II/grid/grid_reordering.h>
#include <deal.II/grid/grid_tools.h>
#include <deal.II/grid/tria_accessor.h>
#include <deal.II/grid/tria_iterator.h>
#include <deal.II/dofs/dof_accessor.h>

#include <deal.II/fe/fe_q.h>
#include <deal.II/dofs/dof_tools.h>

#include <deal.II/fe/fe_values.h>
#include <deal.II/base/quadrature_lib.h>

#include <deal.II/base/function.h>
#include <deal.II/numerics/vector_tools.h>
#include <deal.II/numerics/matrix_tools.h>

#include <deal.II/lac/vector.h>
#include <deal.II/lac/full_matrix.h>
#include <deal.II/lac/sparse_matrix.h>
#include <deal.II/lac/compressed_sparsity_pattern.h>
#include <deal.II/lac/solver_cg.h>
#include <deal.II/lac/sparse_direct.h>
#include <deal.II/lac/precondition.h>

#include <deal.II/grid/grid_refinement.h>
#include <deal.II/numerics/error_estimator.h>

#include <deal.II/numerics/fe_field_function.h>
#include <deal.II/numerics/data_out.h>

#include <deal.II/base/timer.h>

#include <boost/config.hpp>
#include <boost/archive/tmpdir.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

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

#include "solver/module.h"

#include "solver/field.h"
#include "solver/problem.h"
#include "solver/problem_config.h"
#include "mesh/agros_manifold.h"

MeshGenerator::MeshGenerator() : QObject()
{
}

MeshGenerator::~MeshGenerator()
{
}

void MeshGenerator::elementsSharingNode(MeshElement* e, Point* node, QList<MeshElement*>& elements)
{
    if (!elements.contains(e))
    {
        elements.append(e);
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                if (e->neigh[i] != -1 && &this->nodeList[this->elementList[e->neigh[i]].node[j]] == node)
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

    if (!is_triangle)
        throw AgrosException("Shifting nodes using get_determinant works only for triangles.");

    double x[3], y[3];
    for (int i = 0; i < 3; i++)
    {
        x[i] = this->nodeList[element->node[i]].x;
        y[i] = this->nodeList[element->node[i]].y;
    }

    double determinant = x[0] * (y[1] - y[2]) - x[1] * (y[0] - y[2]) + x[2] * (y[0] - y[1]);

    // std::cout << "E[" << x[0] << "," << y[0] << "],[" << x[1] << "," << y[1] << "],[" << x[2] << "," << y[2] << "]" << ",   D=" << determinant << std::endl;

    return (determinant > 0);
}

// Used in moveNode - this actually moves the node (if it has not been moved before).
void MeshGenerator::performActualNodeMove(Point* node, QList<Point*>& already_moved_nodes, const double x_displacement, const double y_displacement, const double multiplier)
{
    if (already_moved_nodes.contains(node))
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
    if (multiplier < .01)
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
        if (new_determinant != old_determinant)
        {
            for (int j = 0; j < 3; j++)
            {
                if (!already_moved_nodes.contains(&this->nodeList[determinants[i].first->node[j]]))
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
        if (edge.neighElem[0] == -1 || edge.neighElem[1] == -1)
        {
            // assert(edge.marker >= 0); // markers changed to marker - 1, check...
            if (edge.marker != -1)
            {
                // curve
                if (Agros2D::scene()->edges->at(edge.marker)->angle() > 0.0 &&
                    Agros2D::scene()->edges->at(edge.marker)->isCurvilinear())
                {
                    // Nodes.
                    Point* node[2] = { &nodeList[edge.node[0]], &nodeList[edge.node[1]] };
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
        if (edge.neighElem[0] == -1 || edge.neighElem[1] == -1)
            continue;

        // assert(edge.marker >= 0); // markers changed to marker - 1, check...
        if (edge.marker != -1)
        {
            // curve
            if (Agros2D::scene()->edges->at(edge.marker)->angle() > 0.0 &&
                Agros2D::scene()->edges->at(edge.marker)->isCurvilinear())
            {
                // Nodes.
                Point* node[2] = { &nodeList[edge.node[0]], &nodeList[edge.node[1]] };
                // Center
                Point center = Agros2D::scene()->edges->at(edge.marker)->center();
                // Radius
                double radius = Agros2D::scene()->edges->at(edge.marker)->radius();

                // Handle the nodes recursively using moveNode()
                for (int inode = 0; inode < 2; inode++)
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

void MeshGenerator::writeTodealii()
{
        //std::cout << fieldInfo->name().toStdString() << std::endl;

        dealii::Triangulation<2> *triangulation = new dealii::Triangulation<2>(dealii::Triangulation<2>::maximum_smoothing, false);

        // vertices
        std::vector<dealii::Point<2> > vertices;
        for (int vertex_i = 0; vertex_i < nodeList.count(); vertex_i++)
        {
            vertices.push_back(dealii::Point<2>(
                nodeList[vertex_i].x,
                nodeList[vertex_i].y));
        }

        QList<QList<QPair<int, int> > > edges_between_elements;

        // Curves //
        // we have to do this here, because the manifold Ids we create here are used for elements and edges in the following.
        std::map<dealii::types::manifold_id, AgrosManifoldSurface<2>*> surfManifolds;
        std::map<dealii::types::manifold_id, AgrosManifoldVolume<2>*> volManifolds;

        // it is important that we know what is the largest edge marker, so that the manifold_ids for elements (volumetric) do not coincide with the surface ones.
        int maxEdgeMarker = edgeList.count() + 1;

        for (int edge_i = 0; edge_i < edgeList.count(); edge_i++)
        {
            MeshEdge edge = edgeList[edge_i];
            if (edge.marker != -1)
            {
                SceneEdge* sceneEdge = Agros2D::scene()->edges->at(edge.marker);
                if (sceneEdge->angle() > 0.0 && sceneEdge->isCurvilinear())
                {
                    dealii::types::manifold_id edgeManifoldId = edge_i + 1;
                    dealii::types::manifold_id elementManifoldId[2] = { maxEdgeMarker + edge.neighElem[0], maxEdgeMarker + edge.neighElem[1] };

                    if (surfManifolds.find(edgeManifoldId) == surfManifolds.end())
                        surfManifolds.insert(std::pair<dealii::types::manifold_id, AgrosManifoldSurface<2>*>(edgeManifoldId, new AgrosManifoldSurface<2>(edgeManifoldId, sceneEdge->center(), sceneEdge->radius())));

                    for (int neighElem_i = 0; neighElem_i < 2; neighElem_i++)
                    {
                        if (edge.neighElem[neighElem_i] != -1)
                        {
                            if (volManifolds.find(elementManifoldId[neighElem_i]) == volManifolds.end())
                                volManifolds.insert(std::pair<dealii::types::manifold_id, AgrosManifoldVolume<2>*>(elementManifoldId[neighElem_i], new AgrosManifoldVolume<2>(elementManifoldId[neighElem_i], surfManifolds.find(edgeManifoldId)->second)));
                            else
                                volManifolds.find(elementManifoldId[neighElem_i])->second->push_surfManifold(surfManifolds.find(edgeManifoldId)->second);
                        }
                    }
                }
            }
        }

        // elements
        std::vector<dealii::CellData<2> > cells;
        for (int element_i = 0; element_i < elementList.count(); element_i++)
        {
            MeshElement element = elementList[element_i];
            if (element.isUsed && (!Agros2D::scene()->labels->at(element.marker)->isHole()))
            {
                dealii::types::manifold_id elementManifoldId = maxEdgeMarker + element_i;
                if (element.isTriangle())
                {
                    assert("triangle");
                }
                else
                {
                    dealii::CellData<2> cell;
                    cell.vertices[0] = element.node[0];
                    cell.vertices[1] = element.node[1];
                    cell.vertices[2] = element.node[2];
                    cell.vertices[3] = element.node[3];
                    cell.material_id = element.marker + 1;
                    if (volManifolds.find(elementManifoldId) == volManifolds.end())
                        cell.manifold_id = 0;
                    else
                        cell.manifold_id = elementManifoldId;

                    cells.push_back(cell);
                }
            }            

            edges_between_elements.push_back(QList<QPair<int, int> > ());
        }


        // boundary markers
        dealii::SubCellData subcelldata;
        for (int edge_i = 0; edge_i < edgeList.count(); edge_i++)
        {
            if (edgeList[edge_i].marker == -1)
                continue;
            std::cout << " neigh elements " << edgeList[edge_i].neighElem[0] << ", " << edgeList[edge_i].neighElem[1] << std::endl;

            dealii::CellData<1> cell_data;
            cell_data.vertices[0] = edgeList[edge_i].node[0];
            cell_data.vertices[1] = edgeList[edge_i].node[1];

            if(edgeList[edge_i].neighElem[1] != -1)
            {
                edges_between_elements[edgeList[edge_i].neighElem[0]].push_back(QPair<int, int>(edgeList[edge_i].neighElem[1], edgeList[edge_i].marker + 1));
                edges_between_elements[edgeList[edge_i].neighElem[1]].push_back(QPair<int, int>(edgeList[edge_i].neighElem[0], edgeList[edge_i].marker + 1));

                // do not push the boundary line
                continue;
                //cell_data.boundary_id = dealii::numbers::internal_face_boundary_id;
            }
            else
            {
                cell_data.boundary_id = edgeList[edge_i].marker + 1;
                std::cout << "marker: " << edgeList[edge_i].marker + 1 << std::endl;
            }
            // todo: co je hranice?
            // todo: kde to deal potrebuje? Kdyz si okrajove podminky resim sam...
//            if (Agros2D::scene()->edges->at(edgeList[edge_i].marker)->marker(fieldInfo) == SceneBoundaryContainer::getNone(fieldInfo))
//                continue;

//            if (Agros2D::scene()->edges->at(edgeList[edge_i].marker)->marker(Agros2D::problem()->fieldInfo("current"))== SceneBoundaryContainer::getNone(Agros2D::problem()->fieldInfo("current")))
//                continue;


            //cell_data.boundary_id = dealii::numbers::internal_face_boundary_id;
            // todo: (Pavel Kus) I do not know how exactly this works, whether internal_face_boundary_id is determined apriori or not
            // todo: but it seems to be potentially dangerous, when there would be many boundaries
            //assert(cell_data.boundary_id != dealii::numbers::internal_face_boundary_id);

            if (surfManifolds.find(edge_i + 1) == surfManifolds.end())
                cell_data.manifold_id = 0;
            else
                cell_data.manifold_id = edge_i + 1;

            subcelldata.boundary_lines.push_back(cell_data);
        }

        dealii::GridTools::delete_unused_vertices(vertices, cells, subcelldata);
        dealii::GridReordering<2>::invert_all_cells_of_negative_grid(vertices, cells);
        dealii::GridReordering<2>::reorder_cells(cells);
        triangulation->create_triangulation_compatibility(vertices, cells, subcelldata);

        // Fix of dealII automatic marking of sub-objects with the same manifoldIds (quads -> lines).
        for (dealii::Triangulation<2>::face_iterator line = triangulation->begin_face(); line != triangulation->end_face(); ++line) {
            if (line->manifold_id() >= maxEdgeMarker)
                line->set_manifold_id(0);
        }

        for (std::map<dealii::types::manifold_id, AgrosManifoldVolume<2>*>::iterator iterator = volManifolds.begin(); iterator != volManifolds.end(); iterator++) {
            triangulation->set_manifold(iterator->first, *iterator->second);
        }

        for (std::map<dealii::types::manifold_id, AgrosManifoldSurface<2>*>::iterator iterator = surfManifolds.begin(); iterator != surfManifolds.end(); iterator++) {
            triangulation->set_manifold(iterator->first, *iterator->second);
        }

        m_triangulation = triangulation;
        std::cout << "triangulation created " << std::endl;

        dealii::Triangulation<2>::cell_iterator cell = triangulation->begin();
        dealii::Triangulation<2>::cell_iterator end_cell = triangulation->end();

        int cell_idx = 0;
        for(; cell != end_cell; ++cell)
        {
            // todo: probably active is not neccessary
            if(cell->active())
            {
                for(int neigh_i = 0; neigh_i < dealii::GeometryInfo<2>::faces_per_cell; neigh_i++)
                {
                    if(cell->face(neigh_i)->boundary_indicator() == dealii::numbers::internal_face_boundary_id)
                    {
                        cell->face(neigh_i)->set_user_index(0);
                    }
                    else
                    {
                        cell->face(neigh_i)->set_user_index((int)cell->face(neigh_i)->boundary_indicator());
                        //std::cout << "cell cell_idx: " << cell_idx << ", face  " << neigh_i << " set to " << (int) cell->face(neigh_i)->boundary_indicator() << " -> value " << cell->face(neigh_i)->user_index() << std::endl;
                    }

                    int neighbor_cell_idx = cell->neighbor_index(neigh_i);
                    if(neighbor_cell_idx != -1)
                    {
                        assert(cell->face(neigh_i)->user_index() == 0);
                        QPair<int, int> neighbor_edge_pair;
                        foreach(neighbor_edge_pair, edges_between_elements[cell_idx])
                        {
                            if(neighbor_edge_pair.first == neighbor_cell_idx)
                            {
                                cell->face(neigh_i)->set_user_index(neighbor_edge_pair.second);
                                //std::cout << "cell cell_idx: " << cell_idx << ", face adj to " << neighbor_cell_idx << " set to " << neighbor_edge_pair.second << " -> value " << cell->face(neigh_i)->user_index() << std::endl;
                                //dealii::TriaAccessor<1,2,2> line = cell->line(neigh_i);
                                //cell->neighbor()
                            }
                        }
                    }
                }
                cell_idx++;
            }
        }

        // save to disk
        QString fnMesh = QString("%1/%2_initial.msh").arg(cacheProblemDir()).arg("mesh"/*fieldInfo->fieldId()*/);
        std::ofstream ofsMesh(fnMesh.toStdString());
        boost::archive::binary_oarchive sbMesh(ofsMesh);
        triangulation->save(sbMesh, 0);
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
