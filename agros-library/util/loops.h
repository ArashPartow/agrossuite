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

#ifndef UTIL_LOOPS_H
#define UTIL_LOOPS_H

class Scene;
class SceneLabel;
class SceneFace;

#include "util/util.h"

class LoopsInfo : public QObject
{
public:
    LoopsInfo(Scene *scene);
    virtual ~LoopsInfo() {}

    enum Intersection
    {
        Intersection_Uncertain,
        Intersection_Left,
        Intersection_Right,
        Intersection_Both,
        Intersection_No
    };

    // describes conection from the LoopsNode to other nodes
    struct LoopsNodeEdgeData
    {
        LoopsNodeEdgeData();
        LoopsNodeEdgeData(int node, int edge, bool reverse,  double angle) : node(node), edge(edge), reverse(reverse), angle(angle), visited(false) {}
        int node;
        int edge;
        bool reverse;
        double angle; // to order edges going from node (anti)clockwise
        bool visited;
    };

    // describes one node in the graph
    // has information about each edge going from the node
    // those edges are sorted depending on the angle
    struct LoopsNode
    {
        void insertEdge(int endNode, int edgeIdx, bool reverse,  double angle);
        bool hasUnvisited();
        LoopsNodeEdgeData startLoop();
        LoopsNodeEdgeData continueLoop(int previousNode);
        void setVisited(int index) {nodeEdges[index].visited = true;}

        QList<LoopsNodeEdgeData> nodeEdges;
    };

    // describes the graph
    // holds list of LoopsNode, which corresponds to vertices in the geometry
    // each LoopsNode has information about each edge going from it and nodes connected to it by this edge
    struct LoopsGraph
    {
        LoopsGraph(int numNodes);
        void addEdge(int startNode, int endNode, int edgeIdx, double angle);
        void print();

        QList<LoopsNode> nodes;
    };

    struct Triangle
    {
        Triangle(const Point &a, const Point &b, const Point &c) : a(a), b(b), c(c)
        {
        }

        Point a, b, c;
    };

    inline QList<QList<LoopsNodeEdgeData> > loops() const { return m_loops; }
    inline QList<int> outsideLoops() const { return m_outsideLoops; }
    inline QMap<SceneLabel*, QList<int> > labelLoops() const { return m_labelLoops; }

    // polygon triangles
    inline QMap<SceneLabel*, QList<Triangle> > polygonTriangles() const { return m_polygonTriangles; }

    inline bool isProcessPolygonError() { return m_isProcessPolygonError; }

    void clear();
    void processPolygonTriangles(bool force = false);

private:
    Scene *m_scene;

    bool m_isProcessPolygonError;

    // list of individual loops. Each loop is a list of LoopsNodeEdgeData
    QList<QList<LoopsNodeEdgeData> > m_loops;

    // loops cooresponding to indiviual labels. Indices to the m_loops list
    // the firs (0) corresponds to a inmost loop around label
    // the rest are (if present) holes in this label
    QMap<SceneLabel*, QList<int> > m_labelLoops;

    QList<int> m_outsideLoops;

    QMap<SceneLabel*, QList<Triangle> > m_polygonTriangles;

    void processLoops();

    Intersection intersects(Point point, double tangent, SceneFace* edge);
    Intersection intersects(Point point, double tangent, SceneFace* edge, Point& intersection);
    int intersectionsParity(Point point, QList<LoopsNodeEdgeData> loop);
    bool isInsideSeg(double angleSegStart, double angleSegEnd, double angle);

    QList<Triangle> triangulateLabel(const QList<Point> &polyline, const QList<QList<Point> > &holes, const Point& steiner);
    int windingNumber(Point point, QList<LoopsNodeEdgeData> loop);
    bool areSameLoops(QList<LoopsNodeEdgeData> loop1, QList<LoopsNodeEdgeData> loop2);
    bool areEdgeDuplicities(QList<LoopsNodeEdgeData> loop);
    int longerLoop(int idx1, int idx2);
    bool shareEdge(int idx1, int idx2);
    void switchOrientation(int idx);
    void addEdgePoints(QList<Point> *polyline, const SceneFace &edge, bool reverse = false);
};


#endif // UTIL_LOOPS_H

