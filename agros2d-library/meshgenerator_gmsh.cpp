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

#include "meshgenerator_gmsh.h"

#include "util/global.h"
#include "util/loops.h"

#include "scene.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"

#include "sceneview_common.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "logview.h"

#include "hermes2d/module.h"

#include "hermes2d/field.h"
#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"

#include "gmsh/Gmsh.h"
#include "gmsh/Context.h"
#include "gmsh/GModel.h"
#include "gmsh/GVertex.h"
#include "gmsh/GEdge.h"
#include "gmsh/GEdgeLoop.h"
#include "gmsh/GmshDefines.h"
#include "gmsh/MElement.h"

MeshGeneratorGMSH::MeshGeneratorGMSH() : MeshGenerator()
{
}

bool MeshGeneratorGMSH::mesh()
{
    m_isError = !prepare();

    // create gmsh files
    // if (writeToGmshInternal()) // internal
    if (writeToGmshMeshFile()) // file
    {
        // convert gmsh mesh to hermes mesh
        // if (!readFromGmshInternal()) // internal
        if (!readFromGmshMeshFile()) // file
        {
            m_isError = true;
        }
    }
    else
    {
        m_isError = true;
    }

    //  remove gmsh temp files
    // QFile::remove(tempProblemFileName() + ".geo");
    // QFile::remove(tempProblemFileName() + ".msh");

    return !m_isError;
}

bool MeshGeneratorGMSH::writeToGmshInternal()
{
    // basic check
    if (Agros2D::scene()->nodes->length() < 3)
    {
        Agros2D::log()->printError(tr("Mesh generator"), tr("Invalid number of nodes (%1 < 3)").arg(Agros2D::scene()->nodes->length()));
        return false;
    }
    if (Agros2D::scene()->edges->length() < 3)
    {
        Agros2D::log()->printError(tr("Mesh generator"), tr("Invalid number of edges (%1 < 3)").arg(Agros2D::scene()->edges->length()));
        return false;
    }

    // Initialization.
    GmshInitialize();
    // Options may be set this way.
    // Output information messages generated by the Gmsh library.
    GmshSetOption("General", "Terminal", 0.);
    // Be verbose (output debug messages).
    GmshSetOption("General", "Verbosity", 0.0);
    // Create GModel (the Gmsh library core) instance.

    GmshSetOption("Mesh", "Algorithm", (unsigned int) 8);
    GmshSetOption("Mesh", "SubdivisionAlgorithm", (unsigned int) 1);

    m = new GModel;
    m->setFactory("Gmsh");

    // mesh size
    RectPoint rect = Agros2D::scene()->boundingBox();
    double mesh_size = qMin(rect.width(), rect.height()) / 6.0;

    // nodes
    QList<GVertex *> vertices;
    for (int i = 0; i<Agros2D::scene()->nodes->length(); i++)
    {
        vertices.append(m->addVertex(Agros2D::scene()->nodes->at(i)->point().x,
                                     Agros2D::scene()->nodes->at(i)->point().y,
                                     0.0,
                                     mesh_size));
    }

    // edges
    QList<GEdge *> edges;
    edgesMap.clear();
    for (int i = 0; i<Agros2D::scene()->edges->length(); i++)
    {
        GEdge *edge = NULL;

        if (Agros2D::scene()->edges->at(i)->angle() == 0)
        {
            // straight line
            edge = m->addLine(vertices[Agros2D::scene()->nodes->items().indexOf(Agros2D::scene()->edges->at(i)->nodeStart())],
                    vertices[Agros2D::scene()->nodes->items().indexOf(Agros2D::scene()->edges->at(i)->nodeEnd())]);
        }
        else
        {
            // arc
            Point center = Agros2D::scene()->edges->at(i)->center();

            vertices.append(m->addVertex(center.x, center.y, 0.0, mesh_size));

            edge = m->addCircleArcCenter(vertices[Agros2D::scene()->nodes->items().indexOf(Agros2D::scene()->edges->at(i)->nodeStart())],
                    vertices.last(),
                    vertices[Agros2D::scene()->nodes->items().indexOf(Agros2D::scene()->edges->at(i)->nodeEnd())]);
        }

        edgesMap[Agros2D::scene()->edges->at(i)] = edge;
        edges.append(edge);
    }

    try
    {
        Agros2D::scene()->loopsInfo()->processLoops();
    }
    catch (AgrosMeshException& ame)
    {
        Agros2D::log()->printError(tr("Mesh generator"), ame.toString());
        std::cout << "Missing Label";
        return false;
    }

    // loops
    QList<std::vector<GEdge *> > edgesLoop;
    QList<bool> edgesIsReversed;
    for(int i = 0; i < Agros2D::scene()->loopsInfo()->loops().size(); i++)
    {
        edgesLoop.append(std::vector<GEdge *>());
        edgesIsReversed.append(true);
        qDebug() << "loop" << i;

        if (!Agros2D::scene()->loopsInfo()->outsideLoops().contains(i))
        {
            for(int j = 0; j < Agros2D::scene()->loopsInfo()->loops().at(i).size(); j++)
            {
                GEdge *edge = edges[Agros2D::scene()->loopsInfo()->loops().at(i)[j].edge];

                edgesIsReversed.last() = edgesIsReversed.last() && Agros2D::scene()->loopsInfo()->loops().at(i)[j].reverse;
                edgesLoop.last().push_back(edge);

                qDebug() << "edge" << Agros2D::scene()->loopsInfo()->loops().at(i)[j].edge << " reverse " << Agros2D::scene()->loopsInfo()->loops().at(i)[j].reverse;
            }
        }
    }

    // faces
    qDebug() << "edges loop size" << edgesLoop.size();
    for (int i = 0; i < Agros2D::scene()->labels->count(); i++)
    {
        SceneLabel* label = Agros2D::scene()->labels->at(i);
        if (!label->isHole())
        {
            qDebug() << "label PRE" << i;
            bool isReversed = true;
            std::vector<std::vector<GEdge *> > loops;
            for (int j = 0; j < Agros2D::scene()->loopsInfo()->labelLoops()[label].count(); j++)
            {
                qDebug() << Agros2D::scene()->loopsInfo()->labelLoops()[label][j];
                isReversed = isReversed && edgesIsReversed[Agros2D::scene()->loopsInfo()->labelLoops()[label][j]];
                loops.push_back(edgesLoop[Agros2D::scene()->loopsInfo()->labelLoops()[label][j]]);
            }
            GFace *face = m->addPlanarFace(loops);
            if (isReversed)
                face->meshAttributes.reverseMesh = true;
            face->setMeshingAlgo(ALGO_2D_AUTO);

            facesMap[label] = face;
        }
    }

    // create mesh
    m->mesh(2);


    //    // quad mesh
    //    if (Agros2D::problem()->config()->meshType() == MeshType_GMSH_Quad ||
    //            Agros2D::problem()->config()->meshType() == MeshType_GMSH_QuadDelaunay_Experimental)
    //    {
    //        outLoops.append(QString("Recombine Surface {"));
    //        for(int i = 0; i <  surfaces.count(); i++)
    //        {
    //            outLoops.append(QString("%1").arg(surfaces.at(i)));
    //            if(i < surfaces.count() - 1)
    //                outLoops.append(",");
    //        }
    //        outLoops.append(QString("};\n"));
    //    }

    //    // Mesh.Algorithm - 1=MeshAdapt, 2=Automatic, 5=Delaunay, 6=Frontal, 7=bamg, 8=delquad
    //    QString outCommands;
    //    if (Agros2D::problem()->config()->meshType() == MeshType_GMSH_Triangle)
    //    {
    //        outCommands.append(QString("Mesh.Algorithm = 2;\n"));
    //    }
    //    else if (Agros2D::problem()->config()->meshType() == MeshType_GMSH_Quad)
    //    {
    //        outCommands.append(QString("Mesh.Algorithm = 2;\n"));
    //        outCommands.append(QString("Mesh.SubdivisionAlgorithm = 1;\n"));
    //    }
    //    else if (Agros2D::problem()->config()->meshType() == MeshType_GMSH_QuadDelaunay_Experimental)
    //    {
    //        outCommands.append(QString("Mesh.Algorithm = 8;\n"));
    //        outCommands.append(QString("Mesh.SubdivisionAlgorithm = 1;\n"));
    //    }


    m->writeGEO((tempProblemFileName() + ".geo").toStdString());
    m->writeMSH((tempProblemFileName() + ".msh").toStdString());

    return true;
}

bool MeshGeneratorGMSH::readFromGmshInternal()
{
    nodeList.clear();
    edgeList.clear();
    elementList.clear();

    m->indexMeshVertices(true, 0, false);

    // nodes
    int numNodes = 0;
    std::vector<GEntity*> entities;
    m->getEntities(entities);
    for(unsigned int i = 0; i < entities.size(); i++)
    {
        for(unsigned int j = 0; j < entities[i]->mesh_vertices.size(); j++)
        {
            MVertex *mv = entities[i]->mesh_vertices[j];
            nodeList.append(Point(mv->x(), mv->y()));
            numNodes++;
        }
    }

    QMap<SceneEdge *, GEdge *>::const_iterator iEdge = edgesMap.constBegin();
    while (iEdge != edgesMap.constEnd())
    {
        GEdge *edge = iEdge.value();

        for (int i = 0; i < edge->getNumMeshElements(); ++i)
        {
            MElement *me = edge->getMeshElement(i);

            // edge
            if (me->getType() == TYPE_LIN)
            {
                edgeList.append(MeshEdge(me->getVertex(0)->getIndex() - 1,
                                         me->getVertex(1)->getIndex() - 1,
                                         Agros2D::scene()->edges->items().indexOf(iEdge.key())));
            }
        }

        ++iEdge;
    }

    QMap<SceneLabel *, GFace *>::const_iterator iFace = facesMap.constBegin();
    while (iFace != facesMap.constEnd())
    {
        GFace *face = iFace.value();

        for (int i = 0; i < face->getNumMeshElements(); ++i)
        {
            MElement *me = face->getMeshElement(i);

            // triangle
            if (me->getType() == TYPE_TRI)
            {
                elementList.append(MeshElement(me->getVertex(0)->getIndex() - 1,
                                               me->getVertex(1)->getIndex() - 1,
                                               me->getVertex(2)->getIndex() - 1,
                                               Agros2D::scene()->labels->items().indexOf(iFace.key())));
            }

            // quad
            if (me->getType() == TYPE_QUA)
            {
                elementList.append(MeshElement(me->getVertex(0)->getIndex() - 1,
                                               me->getVertex(1)->getIndex() - 1,
                                               me->getVertex(2)->getIndex() - 1,
                                               me->getVertex(3)->getIndex() - 1,
                                               Agros2D::scene()->labels->items().indexOf(iFace.key())));
            }
        }

        ++iFace;
    }

    // Finalization.
    delete m;
    edgesMap.clear();
    facesMap.clear();

    GmshFinalize();

    writeToHermes();

    nodeList.clear();
    edgeList.clear();
    elementList.clear();

    return true;
}

// deprecated but functional
bool MeshGeneratorGMSH::writeToGmshMeshFile()
{
    // basic check
    if (Agros2D::scene()->nodes->length() < 3)
    {
        Agros2D::log()->printError(tr("Mesh generator"), tr("Invalid number of nodes (%1 < 3)").arg(Agros2D::scene()->nodes->length()));
        return false;
    }
    if (Agros2D::scene()->edges->length() < 3)
    {
        Agros2D::log()->printError(tr("Mesh generator"), tr("Invalid number of edges (%1 < 3)").arg(Agros2D::scene()->edges->length()));
        return false;
    }

    QFile file(tempProblemFileName() + ".geo");

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        Agros2D::log()->printError(tr("Mesh generator"), tr("Could not create GMSH geometry file (%1)").arg(file.errorString()));
        return false;
    }
    QTextStream out(&file);

    // mesh size
    RectPoint rect = Agros2D::scene()->boundingBox();
    // out << QString("mesh_size = %1;\n").arg(qMin(rect.width(), rect.height()) / 6.0);
    out << QString("mesh_size = 1e22;\n");

    // nodes
    QString outNodes;
    int nodesCount = 0;
    for (int i = 0; i<Agros2D::scene()->nodes->length(); i++)
    {
        outNodes += QString("Point(%1) = {%2, %3, 0, mesh_size};\n").
                arg(i).
                arg(Agros2D::scene()->nodes->at(i)->point().x, 0, 'f', 10).
                arg(Agros2D::scene()->nodes->at(i)->point().y, 0, 'f', 10);
        nodesCount++;
    }

    // edges
    QString outEdges;
    int edgesCount = 0;
    for (int i = 0; i<Agros2D::scene()->edges->length(); i++)
    {
        if (Agros2D::scene()->edges->at(i)->angle() == 0)
        {
            // line .. increase edge index to count from 1
            outEdges += QString("Line(%1) = {%2, %3};\n").
                    arg(edgesCount+1).
                    arg(Agros2D::scene()->nodes->items().indexOf(Agros2D::scene()->edges->at(i)->nodeStart())).
                    arg(Agros2D::scene()->nodes->items().indexOf(Agros2D::scene()->edges->at(i)->nodeEnd()));
            edgesCount++;
        }
        else
        {
            // arc
            // add pseudo nodes
            Point center = Agros2D::scene()->edges->at(i)->center();
            outNodes += QString("Point(%1) = {%2, %3, 0};\n").
                    arg(nodesCount).
                    arg(center.x, 0, 'f', 10).
                    arg(center.y, 0, 'f', 10);
            nodesCount++;

            outEdges += QString("Circle(%1) = {%2, %3, %4};\n").
                    arg(edgesCount+1).
                    arg(Agros2D::scene()->nodes->items().indexOf(Agros2D::scene()->edges->at(i)->nodeStart())).
                    arg(nodesCount - 1).
                    arg(Agros2D::scene()->nodes->items().indexOf(Agros2D::scene()->edges->at(i)->nodeEnd()));

            edgesCount++;
        }
    }

    try
    {
        Agros2D::scene()->loopsInfo()->processLoops();
    }
    catch (AgrosMeshException& ame)
    {
        Agros2D::log()->printError(tr("Mesh generator"), ame.toString());
        std::cout << "Missing Label";
        return false;
    }

    QString outLoops;
    for(int i = 0; i < Agros2D::scene()->loopsInfo()->loops().size(); i++)
    {
        if (!Agros2D::scene()->loopsInfo()->outsideLoops().contains(i))
        {
            outLoops.append(QString("Line Loop(%1) = {").arg(i+1));
            for(int j = 0; j < Agros2D::scene()->loopsInfo()->loops().at(i).size(); j++)
            {
                if (Agros2D::scene()->loopsInfo()->loops().at(i)[j].reverse)
                    outLoops.append("-");
                outLoops.append(QString("%1").arg(Agros2D::scene()->loopsInfo()->loops().at(i)[j].edge + 1));
                if (j < Agros2D::scene()->loopsInfo()->loops().at(i).size() - 1)
                    outLoops.append(",");
            }
            outLoops.append(QString("};\n"));
        }
    }
    outLoops.append("\n");

    QList<int> surfaces;
    int surfaceCount = 0;
    for (int i = 0; i < Agros2D::scene()->labels->count(); i++)
    {
        surfaceCount++;
        SceneLabel* label = Agros2D::scene()->labels->at(i);
        if(!label->isHole())
        {
            surfaces.push_back(surfaceCount);
            outLoops.append(QString("Plane Surface(%1) = {").arg(surfaceCount));
            for (int j = 0; j < Agros2D::scene()->loopsInfo()->labelLoops()[label].count(); j++)
            {
                outLoops.append(QString("%1").arg(Agros2D::scene()->loopsInfo()->labelLoops()[label][j]+1));
                if (j < Agros2D::scene()->loopsInfo()->labelLoops()[label].count() - 1)
                    outLoops.append(",");
            }
            outLoops.append(QString("};\n"));
        }
    }

    //    outLoops.append(QString("Physical Surface(1) = {"));
    //    for(int i = 0; i < surfaceCount; i++)
    //    {
    //        outLoops.append(QString("%1").arg(i+1));
    //        if(i < surfaceCount - 1)
    //            outLoops.append(",");
    //    }
    //    outLoops.append(QString("};\n"));

    // quad mesh
    if (Agros2D::problem()->config()->meshType() == MeshType_GMSH_Quad ||
            Agros2D::problem()->config()->meshType() == MeshType_GMSH_QuadDelaunay_Experimental)
    {
        outLoops.append(QString("Recombine Surface {"));
        for(int i = 0; i <  surfaces.count(); i++)
        {
            outLoops.append(QString("%1").arg(surfaces.at(i)));
            if(i < surfaces.count() - 1)
                outLoops.append(",");
        }
        outLoops.append(QString("};\n"));
    }
    //    QString outLoops;
    //    outLoops.append(QString("Line Loop(1) = {0, 1, 2, 3};\n"));
    //    outLoops.append(QString("Plane Surface(1) = {1};\n"));
    //    outLoops.append(QString("Line Loop(2) = {4, 5, 6, -1};\n"));
    //    outLoops.append(QString("Plane Surface(2) = {2};\n"));
    //    outLoops.append("\n");

    //    // quad mesh
    //    if (Agros2D::problem()->config()->meshType() == MeshType_GMSH_Quad)
    //        outLoops.append(QString("Recombine Surface {1, 2};\n"));

    // Mesh.Algorithm - 1=MeshAdapt, 2=Automatic, 5=Delaunay, 6=Frontal, 7=bamg, 8=delquad
    QString outCommands;
    outCommands.append(QString("Mesh.CharacteristicLengthFromCurvature = 1;\n"));
    outCommands.append(QString("Mesh.CharacteristicLengthFactor = 1;\n"));
    if (Agros2D::problem()->config()->meshType() == MeshType_GMSH_Triangle)
    {
        outCommands.append(QString("Mesh.Algorithm = 2;\n"));
    }
    else if (Agros2D::problem()->config()->meshType() == MeshType_GMSH_Quad)
    {
        outCommands.append(QString("Mesh.Algorithm = 2;\n"));
        outCommands.append(QString("Mesh.SubdivisionAlgorithm = 1;\n"));
    }
    else if (Agros2D::problem()->config()->meshType() == MeshType_GMSH_QuadDelaunay_Experimental)
    {
        outCommands.append(QString("Mesh.Algorithm = 8;\n"));
        outCommands.append(QString("Mesh.SubdivisionAlgorithm = 1;\n"));
    }

    outNodes.insert(0, QString("\n// nodes\n"));
    out << outNodes;
    outEdges.insert(0, QString("\n// edges\n"));
    out << outEdges;
    outLoops.insert(0, QString("\n// loops\n"));
    out << outLoops;
    outCommands.insert(0, QString("\n// commands\n"));
    out << outCommands;

    file.waitForBytesWritten(0);
    file.close();

    // mesh
    // Initialization.
    GmshInitialize();

    // Options may be set this way.
    // Output information messages generated by the Gmsh library.
    GmshSetOption("General", "Terminal", 0.0); // 1.0
    // Be verbose (output debug messages).
    GmshSetOption("General", "Verbosity", 0.0); // 99.0
    // Create GModel (the Gmsh library core) instance.

    m = new GModel;
    m->setFactory("Gmsh");

    // read mesh
    QTime time;
    time.start();
    m->readGEO((tempProblemFileName() + ".geo").toStdString());
    qDebug() << "readGEO" << time.elapsed();
    // create mesh
    time.start();
    m->mesh(2);
    qDebug() << "mesh" << time.elapsed();
    // write mesh
    time.start();
    m->writeMSH((tempProblemFileName() + ".msh").toStdString());
    qDebug() << "writeMSH" << time.elapsed();

    // Finalization.
    delete m;
    GmshFinalize();

    return true;
}

// deprecated but functional
bool MeshGeneratorGMSH::readFromGmshMeshFile()
{
    nodeList.clear();
    edgeList.clear();
    elementList.clear();

    int k;

    QFile fileGMSH(tempProblemFileName() + ".msh");
    if (!fileGMSH.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        Agros2D::log()->printError(tr("Mesh generator"), tr("Could not read GMSH mesh file"));
        return false;
    }
    QTextStream inGMSH(&fileGMSH);

    // nodes
    inGMSH.readLine();
    inGMSH.readLine();
    inGMSH.readLine();
    inGMSH.readLine();
    sscanf(inGMSH.readLine().toLatin1().data(), "%i", &k);
    for (int i = 0; i < k; i++)
    {
        int n;
        double x, y, z;

        sscanf(inGMSH.readLine().toLatin1().data(), "%i %lf %lf %lf", &n, &x, &y, &z);
        nodeList.append(Point(x, y));
    }

    // elements
    inGMSH.readLine();
    inGMSH.readLine();
    sscanf(inGMSH.readLine().toLatin1().data(), "%i", &k);
    QSet<int> labelMarkersCheck;
    for (int i = 0; i < k; i++)
    {
        int quad[4];
        int n, type, phys, part, marker;

        if (sscanf(inGMSH.readLine().toLatin1().data(), "%i %i %i %i %i %i %i %i %i",
                   &n, &type, &phys, &part, &marker, &quad[0], &quad[1], &quad[2], &quad[3]))
        {
            // edge
            if (type == 1)
                edgeList.append(MeshEdge(quad[0] - 1, quad[1] - 1, marker - 1)); // marker conversion from gmsh, where it starts from 1
            // triangle
            if (type == 2)
                elementList.append(MeshElement(quad[0] - 1, quad[1] - 1, quad[2] - 1, marker - 1)); // marker conversion from gmsh, where it starts from 1
            // quad
            if (type == 3)
                elementList.append(MeshElement(quad[0] - 1, quad[1] - 1, quad[2] - 1, quad[3] - 1, marker - 1)); // marker conversion from gmsh, where it starts from 1
        }
        /*

        if (marker == 0)
        {
            Agros2D::log()->printError(tr("Mesh generator"), tr("Some areas have no label marker"));
            return false;
        }
        */
        labelMarkersCheck.insert(marker - 1);
    }

    fileGMSH.close();

    writeToHermes();

    nodeList.clear();
    edgeList.clear();
    elementList.clear();

    return true;
}
