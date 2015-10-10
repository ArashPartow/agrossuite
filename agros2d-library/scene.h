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

#ifndef SCENE_H
#define SCENE_H

#include "util.h"
#include "util/enums.h"
#include "util/conf.h"

#include "solver/solutiontypes.h"

class Scene;
class SceneNode;
class SceneEdge;
class SceneLabel;
class SceneBoundary;
class SceneMaterial;
struct SceneViewSettings;
class LoopsInfo;

class SceneNodeContainer;
class SceneEdgeContainer;
class SceneLabelContainer;

class SceneBoundaryContainer;
class SceneMaterialContainer;

class ProblemWidget;
class SceneTransformDialog;
class ProgressItemSolve;
class CouplingInfo;

class Problem;
class SolutionStore;
class Log;

class PluginInterface;

QString generateSvgGeometry(QList<SceneEdge *> edges);

class AGROS_LIBRARY_API NewMarkerAction : public QAction
{
    Q_OBJECT

public:
    NewMarkerAction(QIcon icon, QObject* parent, QString field);

public slots:
    void doTriggered();

signals:
    void triggered(QString);

private:
    QString field;
};

class AGROS_LIBRARY_API Scene : public QObject
{
    Q_OBJECT

public slots:
    void doNewNode(const Point &point = Point());
    void doNewEdge();
    void doNewLabel(const Point &point = Point());
    void doDeleteSelected();

    void doNewBoundary();
    void doNewBoundary(QString field);

    void doNewMaterial();
    void doNewMaterial(QString field);

    void doFieldsChanged();

signals:
    void invalidated();
    void cleared();

    void defaultValues();

public:
    Scene(Problem *parentProblem);
    ~Scene();

    // parent problem
    Problem *parentProblem() { return m_problem; }

    // geometry
    SceneNodeContainer* nodes;
    SceneEdgeContainer* edges;
    SceneLabelContainer* labels;

    // boundaries and materials
    SceneBoundaryContainer *boundaries;
    SceneMaterialContainer *materials;

    QAction *actNewNode;
    QAction *actNewEdge;
    QAction *actNewLabel;
    QAction *actDeleteSelected;

    QAction *actNewBoundary;
    QMap<QString, QAction*> actNewBoundaries;

    QAction *actNewMaterial;
    QMap<QString, QAction*> actNewMaterials;

    QAction *actTransform;

    SceneNode *addNode(SceneNode *node);
    SceneNode *getNode(const Point &point);

    SceneEdge *addEdge(SceneEdge *edge);
    SceneEdge *getEdge(const Point &pointStart, const Point &pointEnd, double angle, int segments, bool isCurvilinear);
    SceneEdge *getEdge(const Point &pointStart, const Point &pointEnd);

    SceneLabel *addLabel(SceneLabel *label);
    SceneLabel *getLabel(const Point &point);

    void addBoundary(SceneBoundary *boundary);
    void removeBoundary(SceneBoundary *boundary);
    void setBoundary(SceneBoundary *boundary); // set edge marker to selected edges
    SceneBoundary *getBoundary(FieldInfo *field, const QString &name);

    void addMaterial(SceneMaterial *material);
    void removeMaterial(SceneMaterial *material);
    void setMaterial(SceneMaterial *material); // set label marker to selected labels
    SceneMaterial *getMaterial(FieldInfo *field, const QString &name);

    void clear();

    RectPoint boundingBox() const;

    void selectNone();
    void selectAll(SceneGeometryMode sceneMode);
    int selectedCount();
    void deleteSelected();

    void highlightNone();
    int highlightedCount();

    void transformTranslate(const Point &point, bool copy, bool withMarkers);
    void transformRotate(const Point &point, double angle, bool copy, bool withMarkers);
    void transformScale(const Point &point, double scaleFactor, bool copy, bool withMarkers);

    LoopsInfo *loopsInfo() const { return m_loopsInfo; }
    QMultiMap<SceneEdge *, SceneNode *> lyingEdgeNodes() const { return m_lyingEdgeNodes; }
    QMap<SceneNode *, int> numberOfConnectedNodeEdges() const { return m_numberOfConnectedNodeEdges; }
    QList<SceneEdge *> crossings() const { return m_crossings; }

    void importFromDxf(const QString &fileName);
    void exportToDxf(const QString &fileName);
    void exportVTKGeometry(const QString &fileName);

    void checkNodeConnect(SceneNode *node);
    void checkTwoNodesSameCoordinates();
    void checkGeometryResult();
    void checkGeometryAssignement();

    void addBoundaryAndMaterialMenuItems(QMenu* menu, QWidget* parent);

    inline QUndoStack *undoStack() const { return m_undoStack; }

    void stopInvalidating(bool sI) { m_stopInvalidating = sI;}
    inline void invalidate() { emit invalidated(); }

private:
    Problem *m_problem;

    QUndoStack *m_undoStack;

    LoopsInfo *m_loopsInfo;
    QMultiMap<SceneEdge *, SceneNode *> m_lyingEdgeNodes;
    QMap<SceneNode *, int> m_numberOfConnectedNodeEdges;
    QList<SceneEdge *> m_crossings;

    void createActions();

    Point calculateNewPoint(SceneTransformMode mode, Point originalPoint, Point transformationPoint, double angle, double scaleFactor);

    // false if cannot (obstruct nodes)
    bool moveSelectedNodes(SceneTransformMode mode, Point point, double angle, double scaleFactor, bool copy);
    bool moveSelectedEdges(SceneTransformMode mode, Point point, double angle, double scaleFactor, bool copy, bool withMarkers);
    bool moveSelectedLabels(SceneTransformMode mode, Point point, double angle, double scaleFactor, bool copy, bool withMarkers);

    void transform(QString name, SceneTransformMode mode, const Point &point, double angle, double scaleFactor, bool copy, bool withMarkers);

    // find lying nodes on edges, number of connected edges and crossings
    void findLyingEdgeNodes();
    void findNumberOfConnectedNodeEdges();
    void findCrossings();

    bool m_stopInvalidating;

private slots:
    void doInvalidated();
};

#endif /* SCENE_H */
