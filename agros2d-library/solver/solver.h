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

#ifndef SOLVER_H
#define SOLVER_H

#include "util.h"
#include "util/global.h"
#include "solutiontypes.h"

#undef signals
#include <deal.II/grid/tria.h>
#include <deal.II/dofs/dof_handler.h>

#include <deal.II/hp/dof_handler.h>
#include <deal.II/hp/fe_collection.h>
#include <deal.II/hp/q_collection.h>
#include <deal.II/hp/fe_values.h>

#include <deal.II/fe/fe_q.h>
#include <deal.II/fe/fe_system.h>
#include <deal.II/dofs/dof_tools.h>
#include <deal.II/lac/solver.h>
#include <deal.II/lac/precondition.h>
#include <deal.II/lac/vector.h>
#include <deal.II/lac/full_matrix.h>
#include <deal.II/lac/sparse_matrix.h>
#include <deal.II/lac/sparse_direct.h>

#include <deal.II/base/time_stepping.h>

#define signals public

struct DoubleCellIterator
{
    dealii::hp::DoFHandler<2>::active_cell_iterator cell_first, cell_second;

    DoubleCellIterator(const dealii::hp::DoFHandler<2>::active_cell_iterator cell_heat, const dealii::hp::DoFHandler<2>::active_cell_iterator cell_elasticity):
        cell_first(cell_heat), cell_second(cell_elasticity)
    {
    }

    DoubleCellIterator(const DoubleCellIterator &dci) :
        cell_first(dci.cell_first), cell_second(dci.cell_second)
    {
    }

    DoubleCellIterator& operator=(const DoubleCellIterator& ehi)
    {
        cell_first = ehi.cell_first;
        cell_second = ehi.cell_second;

        return *this;
    }

    DoubleCellIterator& operator++()
    {
        ++cell_first;
        ++cell_second;
        return *this;
    }

    DoubleCellIterator operator++(int)
    {
        DoubleCellIterator tmp(*this); // copy
        operator++(); // pre-increment
        return tmp;   // return old value
    }


};


inline bool operator==(const DoubleCellIterator& a, const DoubleCellIterator& b)
{
    if((a.cell_second == b.cell_second) || (a.cell_first == b.cell_first))
    {
        assert((a.cell_second == b.cell_second) && (a.cell_first == b.cell_first));
        return true;
    }
    return false;
}

inline bool operator!=(const DoubleCellIterator& a, const DoubleCellIterator& b)
{
    return ! (a==b);
}





class FieldInfo;
class SceneBoundary;
class BDF2Table;

class AGROS_LIBRARY_API SolverDeal
{
public:    
    SolverDeal(const FieldInfo *fieldInfo);
    virtual ~SolverDeal();

    inline dealii::Vector<double> *solution() const { return m_solution; }
    inline dealii::Triangulation<2> *triangulation() const { return m_triangulation; }
    inline dealii::hp::DoFHandler<2> *doFHandler() const { return m_doFHandler; }
    dealii::hp::FECollection<2> *feCollection() const { return m_feCollection; }

    inline dealii::hp::QCollection<2> quadrature_formulas() const { return m_quadrature_formulas; }
    inline dealii::hp::QCollection<2-1> face_quadrature_formulas() const { return m_face_quadrature_formulas; }

    // if use_dirichlet_lift == false, zero dirichlet boundary condition is used
    // this is used for later iterations of the Newton method
    // this function, however, has to be called to ensure zero dirichlet boundary
    virtual void setup(bool useDirichletLift);

    // todo: this does not work due to a problem in Deal.
    // todo: after a refinement, user_data associated with faces are lost
    // todo: might be worked around using initialUnrefinedMesh
    void propagateBoundaryMarkers();

    virtual void assembleSystem() = 0;
    virtual void assembleDirichlet(bool useDirichletLift) = 0;

    // problem
    void solve();

    void setCouplingSource(QString fieldID, dealii::Vector<double> * sourceVector) { m_coupling_sources[fieldID] = sourceVector; }

    // transient - Runge Kutta - future step!
    void assembleMassMatrix();

    // Hand made Euler methods
    double transientBDF(const double time, const double time_step);
    double transientCrankNicolson(const double time, const double time_step);

    inline void set_time(const double new_time) { m_time = new_time; }
    inline double get_time() const { return m_time; }

    static dealii::hp::FECollection<2> *createFECollection(const FieldInfo *fieldInfo);

protected:   
    struct AssemblyScratchData
    {
        AssemblyScratchData(const dealii::hp::FECollection<2> &feCollection,
                             dealii::hp::QCollection<2> quadratureFormulas,
                             dealii::hp::QCollection<2-1> faceQuadratureFormulas);
        AssemblyScratchData(const AssemblyScratchData &scratch_data);

        dealii::hp::FEValues<2> hp_fe_values;
        dealii::hp::FEFaceValues<2> hp_fe_face_values;
    };

    struct AssemblyCopyData
    {
        AssemblyCopyData(const dealii::hp::FECollection<2> &feCollection,
                         dealii::hp::QCollection<2> quadratureFormulas,
                         dealii::hp::QCollection<2-1> faceQuadratureFormulas,
                         const FieldInfo *fieldInfo);

        bool isAssembled;

        dealii::FullMatrix<double> cell_matrix;
        dealii::FullMatrix<double> cell_mass_matrix; // transient
        dealii::Vector<double> cell_rhs;

        std::vector<dealii::types::global_dof_index> local_dof_indices;

        /*
        // volume value and grad cache
        std::vector<dealii::Vector<double> > shape_value;
        std::vector<std::vector<dealii::Tensor<1,2> > > shape_grad;
        // surface cache
        std::vector<std::vector<dealii::Point<2> > > shape_face_point;
        std::vector<std::vector<dealii::Vector<double> > > shape_face_value;
        std::vector<std::vector<double> > shape_face_JxW;
        // std::vector<std::vector<dealii::Tensor<1,2> > > shape_face_grad;

        // previous values and grads
        std::vector<dealii::Vector<double> > solution_value_previous;
        std::vector<std::vector<dealii::Tensor<1,2> > > solution_grad_previous;
        */
    };

    // local reference
    const FieldInfo *m_fieldInfo;
    const Scene *m_scene;
    const Problem *m_problem;

    dealii::Triangulation<2> *m_triangulation;
    dealii::hp::DoFHandler<2> *m_doFHandler;
    dealii::hp::FECollection<2> *m_feCollection;

    // quadrature cache
    dealii::hp::QCollection<2> m_quadrature_formulas;
    dealii::hp::QCollection<2-1> m_face_quadrature_formulas;

    // current solution
    dealii::Vector<double> *m_solution;
    // previous solution (for nonlinear solver)
    dealii::Vector<double> *m_solution_nonlinear_previous;

    // weak coupling sources
    QMap<QString, dealii::Vector<double> * >m_coupling_sources;

    // hanging nodes and sparsity pattern
    dealii::ConstraintMatrix hanging_node_constraints;
    dealii::SparsityPattern sparsity_pattern;

    // matrix and rhs
    dealii::SparseMatrix<double> system_matrix;
    dealii::Vector<double> system_rhs;

    // transient mass matrix
    double m_time;
    dealii::SparseMatrix<double> mass_matrix;
    dealii::SparseMatrix<double> transient_left_matrix;
    dealii::Vector<double> system_rhs_transient_previous;
    QList<dealii::Vector<double> > solution_transient_previous;
    BDF2Table *m_bdf2Table;

    // we need to be able to keep lu decomposition for Jacobian reuse
    dealii::SparseDirectUMFPACK direct_solver;

    double computeNorm();

    // Newton method
    bool m_assemble_matrix;

    // linear system
    void solveLinearSystem(dealii::SparseMatrix<double> &system, dealii::Vector<double> &rhs, dealii::Vector<double> &sln);
    void solveUMFPACK(dealii::SparseMatrix<double> &system, dealii::Vector<double> &rhs, dealii::Vector<double> &sln);
    void solveExternalUMFPACK(dealii::SparseMatrix<double> &system, dealii::Vector<double> &rhs, dealii::Vector<double> &sln);
    void solvedealii(dealii::SparseMatrix<double> &system, dealii::Vector<double> &rhs, dealii::Vector<double> &sln);

    //  linearity
    void solveProblem();
    void solveProblemNonLinearPicard();
    void solveProblemNonLinearNewton();
    void setupProblemNonLinearNewton();

    // adaptivity
    void solveAdaptivity();
    void estimateAdaptivitySmoothness(dealii::Vector<float> &smoothness_indicators) const;
    void refineGrid(bool refine = true);

    // Runge Kutta methods
    dealii::Vector<double> transientEvaluateMassMatrixExplicitPart(const double time, const dealii::Vector<double> &y) const;
    dealii::Vector<double> transientEvaluateMassMatrixImplicitPart(const double time, const double tau, const dealii::Vector<double> &y);
};

namespace Module {
class ErrorCalculator;
}

class SolverAgros
{
public:
    SolverAgros() : m_jacobianCalculations(0), m_phase(Phase_Undefined) {}

    enum Phase
    {
        Phase_Undefined,
        Phase_Init,
        Phase_Solving,
        Phase_DampingFactorChanged,
        Phase_JacobianReused,
        Phase_Finished
    };

    inline QVector<double> steps() const { return m_steps; }
    inline QVector<double> damping() const { return m_damping; }
    inline QVector<double> residualNorms() const { return m_residualNorms; }
    inline QVector<double> solutionNorms() const { return m_solutionNorms; }
    inline QVector<double> relativeChangeOfSolutions() const { return m_relativeChangeOfSolutions; }
    inline int jacobianCalculations() const { return m_jacobianCalculations; }

    inline Phase phase() const { return m_phase; }

    void clearSteps();

protected:
    Phase m_phase;

    virtual void setError() = 0;

    QVector<double> m_steps;
    QVector<double> m_damping;
    QVector<double> m_residualNorms;
    QVector<double> m_solutionNorms;
    QVector<double> m_relativeChangeOfSolutions;
    int m_jacobianCalculations;
};

class ProblemSolver
{
public:
    ProblemSolver();

    static void init();
    static void clear();
    static void solveProblem();
    static QMap<QString, const SolverDeal *> solvers();

private:
    static QMap<FieldInfo *, SolverDeal *> m_solverDeal;
};

#endif // SOLVER_H
