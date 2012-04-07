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

#ifndef PYTHONLABAGROS_H
#define PYTHONLABAGROS_H

#include "pythonlab/pythonconsole.h"
#include "pythonlab/pythonengine.h"
#include "pythonlab/pythoneditor.h"

#include "util.h"
#include "scene.h"

class Solution;
class SceneViewPreprocessor;
class SceneViewMesh;
class SceneViewPost2D;
class SceneViewPost3D;

class PythonEngineAgros : public PythonEngine
{
    Q_OBJECT
public:
    PythonEngineAgros() : PythonEngine() {}

    inline void setSceneViewGeometry(SceneViewPreprocessor *sceneViewGeometry) { assert(sceneViewGeometry); m_sceneViewGeometry = sceneViewGeometry; }
    inline SceneViewPreprocessor *sceneViewGeometry() { assert(m_sceneViewGeometry); return m_sceneViewGeometry; }
    inline void setSceneViewMesh(SceneViewMesh *sceneViewMesh) { assert(sceneViewMesh); m_sceneViewMesh = sceneViewMesh; }
    inline SceneViewMesh *sceneViewMesh() { assert(m_sceneViewMesh); return m_sceneViewMesh; }
    inline void setSceneViewPost2D(SceneViewPost2D *sceneViewPost2D) { assert(sceneViewPost2D); m_sceneViewPost2D = sceneViewPost2D; }
    inline SceneViewPost2D *sceneViewPost2D() {assert(m_sceneViewPost2D); return m_sceneViewPost2D; }
    inline void setSceneViewPost3D(SceneViewPost3D *sceneViewPost3D) { assert(sceneViewPost3D); m_sceneViewPost3D = sceneViewPost3D; }
    inline SceneViewPost3D *sceneViewPost3D() { assert(m_sceneViewPost3D); return m_sceneViewPost3D; }

protected:
    virtual void addCustomExtensions();
    virtual void runPythonHeader();

private slots:
    void doExecutedScript();

private:
    SceneViewPreprocessor *m_sceneViewGeometry;
    SceneViewMesh *m_sceneViewMesh;
    SceneViewPost2D *m_sceneViewPost2D;
    SceneViewPost3D *m_sceneViewPost3D;
};

class PythonLabAgros : public PythonEditorDialog
{
    Q_OBJECT
public:
    PythonLabAgros(PythonEngine *pythonEngine, QStringList args, QWidget *parent);

private:
    QAction *actCreateFromModel;

private slots:
    void doCreatePythonFromModel();
};

bool scriptIsRunning();

// current python engine agros
PythonEngineAgros *currentPythonEngineAgros();

QString createPythonFromModel();
ScriptResult runPythonScript(const QString &script, const QString &fileName = "");
ExpressionResult runPythonExpression(const QString &expression, bool returnValue = true);

class ScriptEngineRemote : QObject
{
    Q_OBJECT
public:
    ScriptEngineRemote();
    ~ScriptEngineRemote();

private slots:
    void connected();
    void readCommand();
    void disconnected();

    void displayError(QLocalSocket::LocalSocketError socketError);

private:
    QString command;

    QLocalServer *m_server;
    QLocalSocket *m_server_socket;
    QLocalSocket *m_client_socket;
};

// ************************************************************************************

// problem
class PyProblem
{
    public:
        PyProblem(char *coordinateType, char *name, char *meshType, char *matrixSolver, double frequency, double timeStep, double timeTotal);
        ~PyProblem() {}

        // name
        inline const char *getName() { return Util::scene()->problemInfo()->name.toStdString().c_str(); }
        void setName(const char *name) { Util::scene()->problemInfo()->name = QString(name); }

        // coordinate type
        inline const char *getCoordinateType() { return coordinateTypeToStringKey(Util::scene()->problemInfo()->coordinateType).toStdString().c_str(); }
        void setCoordinateType(const char *coordinateType) { Util::scene()->problemInfo()->coordinateType = coordinateTypeFromStringKey(QString(coordinateType)); }

        // mesh type
        inline const char *getMeshType() { return meshTypeToStringKey(Util::scene()->problemInfo()->meshType).toStdString().c_str(); }
        void setMeshType(const char *meshType) { Util::scene()->problemInfo()->meshType = meshTypeFromStringKey(QString(meshType)); }

        // matrix solver
        inline const char *getMatrixSolver() { return matrixSolverTypeToStringKey(Util::scene()->problemInfo()->matrixSolver).toStdString().c_str(); }
        void setMatrixSolver(const char *matrixSolver) { Util::scene()->problemInfo()->matrixSolver = matrixSolverTypeFromStringKey(QString(matrixSolver)); }

        // frequency
        inline const double getFrequency() { return Util::scene()->problemInfo()->frequency; }
        void setFrequency(const double frequency) { Util::scene()->problemInfo()->frequency = frequency; }

        // time step
        inline const double getTimeStep() { return Util::scene()->problemInfo()->timeStep.number(); }
        void setTimeStep(const double timeStep) { Util::scene()->problemInfo()->timeStep = Value(QString::number(timeStep)); }

        // time total
        inline const double getTimeTotal() { return Util::scene()->problemInfo()->timeTotal.number(); }
        void setTimeTotal(const double timeTotal) { Util::scene()->problemInfo()->timeTotal = Value(QString::number(timeTotal)); }

        void solve();
};

// field
class PyField
{
    private:
        FieldInfo *m_fieldInfo;

    public:
        PyField(char *fieldId, char *analysisType, int numberOfRefinements, int polynomialOrder, char *linearityType, double nonlinearTolerance, int nonlinearSteps,
                char *adaptivityType, double adaptivityTolerance, int adaptivitySteps, double initialCondition, char *weakForms);
        ~PyField() {}

        FieldInfo *fieldInfo();

        // field id
        inline const char *getFieldId() { return m_fieldInfo->fieldId().toStdString().c_str(); }

        // analysis type
        inline const char *getAnalysisType() { return analysisTypeToStringKey(Util::scene()->fieldInfo(m_fieldInfo->fieldId())->analysisType()).toStdString().c_str(); }
        void setAnalysisType(const char *analysisType) { Util::scene()->fieldInfo(m_fieldInfo->fieldId())->setAnalysisType(analysisTypeFromStringKey(QString(analysisType))); }

        // number of refinements
        inline const int getNumberOfRefinemens() { return Util::scene()->fieldInfo(m_fieldInfo->fieldId())->numberOfRefinements; }
        void setNumberOfRefinemens(const int numberOfRefinemens) { Util::scene()->fieldInfo(m_fieldInfo->fieldId())->numberOfRefinements = numberOfRefinemens; }

        // polynomial order
        inline const int getPolynomialOrder() { return Util::scene()->fieldInfo(m_fieldInfo->fieldId())->polynomialOrder; }
        void setPolynomialOrder(const int polynomialOrder) { Util::scene()->fieldInfo(m_fieldInfo->fieldId())->polynomialOrder = polynomialOrder; }

        // linearity type
        inline const char *getLinearityType() { return linearityTypeToStringKey(Util::scene()->fieldInfo(m_fieldInfo->fieldId())->linearityType).toStdString().c_str(); }
        void setLinearityType(const char *linearityType) { Util::scene()->fieldInfo(m_fieldInfo->fieldId())->linearityType = linearityTypeFromStringKey(QString(linearityType)); }

        // nonlinear tolerance
        inline const double getNonlinearTolerance() { return Util::scene()->fieldInfo(m_fieldInfo->fieldId())->nonlinearTolerance; }
        void setNonlinearTolerance(const double nonlinearTolerance) { Util::scene()->fieldInfo(m_fieldInfo->fieldId())->nonlinearTolerance = nonlinearTolerance; }

        // nonlinear steps
        inline const int getNonlinearSteps() { return Util::scene()->fieldInfo(m_fieldInfo->fieldId())->nonlinearSteps; }
        void setNonlinearSteps(const int nonlinearSteps) { Util::scene()->fieldInfo(m_fieldInfo->fieldId())->nonlinearSteps = nonlinearSteps; }

        // adaptivity type
        inline const char *getAdaptivityType() { return adaptivityTypeToStringKey(Util::scene()->fieldInfo(m_fieldInfo->fieldId())->adaptivityType).toStdString().c_str(); }
        void setAdaptivityType(const char *adaptivityType) { Util::scene()->fieldInfo(m_fieldInfo->fieldId())->adaptivityType = adaptivityTypeFromStringKey(QString(adaptivityType)); }

        // adaptivity tolerance
        inline const double getAdaptivityTolerance() { return Util::scene()->fieldInfo(m_fieldInfo->fieldId())->adaptivityTolerance; }
        void setAdaptivityTolerance(const double adaptivityTolerance) { Util::scene()->fieldInfo(m_fieldInfo->fieldId())->adaptivityTolerance = adaptivityTolerance; }

        // adaptivity steps
        inline const int getAdaptivitySteps() { return Util::scene()->fieldInfo(m_fieldInfo->fieldId())->adaptivitySteps; }
        void setAdaptivitySteps(const int adaptivitySteps) { Util::scene()->fieldInfo(m_fieldInfo->fieldId())->adaptivitySteps = adaptivitySteps; }

        // initial condition
        inline const double getInitialCondition() { return Util::scene()->fieldInfo(m_fieldInfo->fieldId())->initialCondition.number(); }
        void setInitialCondition(const double initialCondition) { Util::scene()->fieldInfo(m_fieldInfo->fieldId())->initialCondition = Value(QString::number(initialCondition)); }

        // weak forms
        inline const char *getWeakForms() { return weakFormsTypeToStringKey(Util::scene()->fieldInfo(m_fieldInfo->fieldId())->weakFormsType).toStdString().c_str(); }
        void setWeakForms(const char *weakForms) { Util::scene()->fieldInfo(m_fieldInfo->fieldId())->weakFormsType = weakFormsTypeFromStringKey(QString(weakForms)); }

        // boundaries
        void addBoundary(char *name, char *type, map<char*, double> parameters);
        void setBoundary(char *name, char *type, map<char*, double> parameters);
        void removeBoundary(char *name);

        // materials
        void addMaterial(char *name, map<char*, double> parameters);
        void setMaterial(char *name, map<char*, double> parameters);
        void removeMaterial(char *name);

        void solve() { assert(0); qDebug() << "Not now :)"; }

        void localValues(double x, double y, map<char*, double> &results);
        void surfaceIntegrals(vector<int> edges, map<char *, double> &results);
        void volumeIntegrals(vector<int> labels, map<char *, double> &results);
};

// geometry
class PyGeometry
{
    public:
        PyGeometry() {}
        ~PyGeometry() {}

        // elements
        void addNode(double x, double y);
        void addEdge(double x1, double y1, double x2, double y2, double angle, int refinement, map<char*, char*> boundaries);
        void addLabel(double x, double y, double area, int order, map<char*, char*> materials);

        // remove operations
        void removeNode(int index);
        void removeEdge(int index);
        void removeLabel(int index);

        void removeNodePoint(double x, double y);
        void removeEdgePoint(double x1, double y1, double x2, double y2, double angle);
        void removeLabelPoint(double x, double y);

        // select operations
        void selectNodes(vector<int> nodes);
        void selectEdges(vector<int> edges);
        void selectLabels(vector<int> labels);

        void selectNodePoint(double x, double y);
        void selectEdgePoint(double x, double y);
        void selectLabelPoint(double x, double y);

        void selectNone();

        // transform operations
        void moveSelection(double dx, double dy, bool copy);
        void rotateSelection(double x, double y, double angle, bool copy);
        void scaleSelection(double x, double y, double scale, bool copy);
        void removeSelection();

        // mesh
        void mesh();
        char *meshFileName();

        // zoom operations
        void zoomBestFit();
        void zoomIn();
        void zoomOut();
        void zoomRegion(double x1, double y1, double x2, double y2);
};

// functions
char *pyVersion();
void pyQuit();

char *pyInput(char *str);
void pyMessage(char *str);

void pyOpenDocument(char *str);
void pySaveDocument(char *str);
void pyCloseDocument();

void pySaveImage(char *str, int w, int h);

// ************************************************************************************

// cython functions
char *pythonSolutionFileName();

void pythonSolveAdaptiveStep();

void pythonMode(char *str);
void pythonPostprocessorMode(char *str);

void pythonShowScalar(char *type, char *variable, char *component, double rangemin, double rangemax);
void pythonShowGrid(bool show);
void pythonShowGeometry(bool show);
void pythonShowInitialMesh(bool show);
void pythonShowSolutionMesh(bool show);
void pythonShowContours(bool show);
void pythonShowVectors(bool show);

void pythonSetTimeStep(int timestep);
int pythonTimeStepCount();

#endif // PYTHONLABAGROS_H
