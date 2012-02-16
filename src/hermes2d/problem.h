#ifndef PROBLEM_H
#define PROBLEM_H

#include "util.h"
#include "solver.h"

template <typename Scalar>
class WeakFormAgros;

template <typename Scalar>
class SolutionArrayList;

template <typename Scalar>
class SolutionArray;

class FieldInfo;
class CouplingInfo;

class ProgressDialog;
class ProgressItemMesh;
class ProgressItemSolve;
class ProgressItemSolveAdaptiveStep;
class ProgressItemProcessView;

class Problem;

class Field
{
public:
    Field(FieldInfo* fieldInfo);
    bool solveInitVariables();
    FieldInfo* fieldInfo() { return m_fieldInfo; }

    // mesh
    void setMeshInitial(Hermes::Hermes2D::Mesh *meshInitial);

public:
//private:
    FieldInfo* m_fieldInfo;

};

/// represents one or more fields, that are hard-coupled -> produce only 1 weak form
class Block
{
public:
    Block(QList<FieldInfo*> fieldInfos, QList<CouplingInfo*> couplings, ProgressItemSolve* progressItemSolve, Problem* parent);

    bool solveInit(Hermes::Hermes2D::Solution<double>* sourceSolution = NULL);
    void solve();

    int numSolutions() const;
    int offset(Field* field) const;

    LinearityType linearityType() const;

    // minimal nonlinear tolerance of individual fields
    double nonlinearTolerance() const;

    //maximal nonlinear steps of individual fields
    int nonlinearSteps() const;

//    Field* couplingSourceField(Coupling* coupling) const;
//    Field* couplingTargetField(Coupling* coupling) const;

    Field* field(FieldInfo* fieldInfo) const;

public:
//private:
    Problem* m_parentProblem;

    WeakFormAgros<double> *m_wf;
    SolutionArrayList<double> *m_solutionList;

    QList<Field*> m_fields;
    QList<CouplingInfo*> m_couplings;
    ProgressItemSolve* m_progressItemSolve;
};

/// intented as central for solution process
/// shielded from gui and QT
/// holds data describing individual fields, means of coupling and solutions
class Problem : public QObject
{
    Q_OBJECT

signals:
    void timeStepChanged(bool showViewProgress = true);
    void meshed();
    void solved();

    void processedRangeContour();
    void processedRangeScalar();
    void processedRangeVector();


public:
    Problem();
    ~Problem() {}
    void createStructure();

    void mesh();

    // solve
    void solve(SolverMode solverMode);

    void postprocess();

    // should store all solutionArrays that have been calculated
    SolutionArray<double> solution(FieldInfo* fieldInfo, int component, int timeStep = 0, int adaptivityStep = 0);
    void setSolution(FieldInfo* fieldInfo, int timeStep, int adaptivityStep, QList<SolutionArray<double> > solution);

    // progress dialog
    ProgressDialog* progressDialog();

    inline Hermes::Hermes2D::Mesh *meshInitial() { return m_meshInitial; }
    inline void setMeshInitial(Hermes::Hermes2D::Mesh* mesh) {m_meshInitial = mesh; }


    // time TODO zatim tady, ale asi presunout
    void setTimeStep(int timeStep, bool showViewProgress = true) { assert(0); }
    inline int timeStep() const { return m_timeStep; }
    int timeStepCount() const { return 0; }
    double time() const { return 0; }

    bool isSolved() const {  return m_isSolved; }
    bool isMeshed()  const {  return m_meshInitial; }
    bool isSolving() const { return m_isSolving; }

    inline int timeElapsed() const { return m_timeElapsed; }
    double adaptiveError() const { return 0; }
    int adaptiveSteps() const { return 0; }
    inline void setTimeElapsed(int timeElapsed) { m_timeElapsed = timeElapsed; }

public:
//private:
    QList<Block*> m_blocks;

    QList<CouplingInfo*> m_couplings;

    // progress dialog
    ProgressDialog *m_progressDialog;
    ProgressItemMesh *m_progressItemMesh;
    ProgressItemSolve *m_progressItemSolve;
    ProgressItemSolveAdaptiveStep *m_progressItemSolveAdaptiveStep;
    ProgressItemProcessView *m_progressItemProcessView;

    int m_timeElapsed;
    bool m_isSolving;
    int m_timeStep;
    bool m_isSolved;

    //TODO move to Field
    Hermes::Hermes2D::Mesh *m_meshInitial; // linearizer only for mesh (on empty solution)


};

#endif // PROBLEM_H
