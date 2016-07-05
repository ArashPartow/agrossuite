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
#include <deal.II/base/quadrature_lib.h>
#include <deal.II/base/multithread_info.h>

#include <deal.II/base/function.h>
#include <deal.II/numerics/vector_tools.h>
#include <deal.II/numerics/matrix_tools.h>
#include <deal.II/numerics/error_estimator.h>
#include <deal.II/numerics/error_estimator.h>
#include <deal.II/numerics/fe_field_function.h>
#include <deal.II/numerics/data_out.h>
#include <deal.II/numerics/solution_transfer.h>

#include <deal.II/lac/vector.h>
#include <deal.II/lac/full_matrix.h>
#include <deal.II/lac/sparse_matrix.h>
#include <deal.II/lac/compressed_sparsity_pattern.h>
#include <deal.II/lac/solver_cg.h>
#include <deal.II/lac/solver_bicgstab.h>
#include <deal.II/lac/solver_gmres.h>
#include <deal.II/lac/solver_richardson.h>
#include <deal.II/lac/solver_minres.h>
#include <deal.II/lac/solver_qmrs.h>
#include <deal.II/lac/solver_relaxation.h>
#include <deal.II/lac/precondition.h>

#include <streambuf>
#include <sstream>

#include "solver.h"
#include "linear_solver.h"
#include "linear_solver_external.h"
#include "estimators.h"

#include "util.h"
#include "util/global.h"
#include "util/constants.h"

#include "field.h"
#include "problem.h"
#include "solver/problem_config.h"
//#include "module.h"
#include "coupling.h"
#include "scene.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "scenemarker.h"
#include "solutionstore.h"
#include "plugin_interface.h"
#include "logview.h"
#include "plugin_interface.h"
#include "weak_form.h"
#include "bdf2.h"

#include <functional>
#include <typeinfo>

SolverLinearSolver::SolverLinearSolver(const FieldInfo *fieldInfo)
    : m_fieldInfo(fieldInfo)
{

}

void SolverLinearSolver::solveUMFPACK(dealii::SparseMatrix<double> &system,
                                      dealii::Vector<double> &rhs,
                                      dealii::Vector<double> &sln,
                                      bool reuseDecomposition)
{
    Agros2D::log()->printDebug(QObject::tr("Solver"),
                               QObject::tr("Direct solver - UMFPACK"));

    if (!reuseDecomposition)
        direct_solver.initialize(system);
    else
        qDebug() << "LU decomposition has been reused";

    direct_solver.vmult(sln, rhs);
}

void SolverLinearSolver::solveExternal(dealii::SparseMatrix<double> &system,
                                       dealii::Vector<double> &rhs,
                                       dealii::Vector<double> &sln)
{
    // read command parameters
    QFile f(QString("%1/libs/%2").arg(datadir()).arg(m_fieldInfo->value(FieldInfo::LinearSolverExternalName).toString()));
    if (!f.open(QFile::ReadOnly | QFile::Text))
        throw AgrosSolverException(QObject::tr("Cannot not open external command file."));
    QTextStream in(&f);
    // command at second line
    QStringList commandContent = in.readAll().split("\n");

    // command at second line
    QString name = commandContent.at(0);
    QString command = commandContent.at(1);

    Agros2D::log()->printDebug(QObject::tr("Solver"),
                               QObject::tr("External solver - %1").arg(name));

    AgrosExternalSolver ext(&system, &rhs);
    ext.setCommand(command);
    ext.setCommandEnvironment(m_fieldInfo->value(FieldInfo::LinearSolverExternalCommandEnvironment).toString());
    ext.setCommandParameters(m_fieldInfo->value(FieldInfo::LinearSolverExternalCommandParameters).toString());
    ext.solve();

    sln = ext.solution();
}

void SolverLinearSolver::solvedealii(dealii::SparseMatrix<double> &system,
                                     dealii::Vector<double> &rhs,
                                     dealii::Vector<double> &sln)
{
    Agros2D::log()->printDebug(QObject::tr("Solver"),
                               QObject::tr("Iterative solver: deal.II (%1, %2)")
                               .arg(iterLinearSolverDealIIMethodString((IterSolverDealII) m_fieldInfo->value(FieldInfo::LinearSolverIterDealIIMethod).toInt()))
                               .arg(iterLinearSolverDealIIPreconditionerString((PreconditionerDealII) m_fieldInfo->value(FieldInfo::LinearSolverIterDealIIPreconditioner).toInt())));

    // preconditioner
    dealii::PreconditionSSOR<> preconditioner;

    switch ((PreconditionerDealII) m_fieldInfo->value(FieldInfo::LinearSolverIterDealIIPreconditioner).toInt())
    {
    case PreconditionerDealII_SSOR:
    {
        // TODO:
        preconditioner.initialize(system, 1.2);
    }
        break;
    default:
        Agros2D::log()->printError(QObject::tr("Solver"), QObject::tr("Preconditioner '%1' is not supported.").arg(m_fieldInfo->matrixSolver()));
        return;
    }

    // solver control
    dealii::SolverControl solver_control(m_fieldInfo->value(FieldInfo::LinearSolverIterIters).toInt(),
                                         m_fieldInfo->value(FieldInfo::LinearSolverIterToleranceAbsolute).toDouble() * rhs.l2_norm());

    switch ((IterSolverDealII) m_fieldInfo->value(FieldInfo::LinearSolverIterDealIIMethod).toInt())
    {
    case IterSolverDealII_CG:
    {
        dealii::SolverCG<> solver(solver_control);
        solver.solve(system, sln, rhs, preconditioner);
    }
        break;
    case IterSolverDealII_BiCGStab:
    {
        dealii::SolverBicgstab<> solver(solver_control);
        solver.solve(system, sln, rhs, preconditioner);
    }
        break;
    case IterSolverDealII_GMRES:
    {
        dealii::SolverGMRES<> solver(solver_control);
        solver.solve(system, sln, rhs, preconditioner);
    }
        break;
    default:
        Agros2D::log()->printError(QObject::tr("Solver"), QObject::tr("Solver method (deal.II) '%1' is not supported.").arg(m_fieldInfo->matrixSolver()));
        return;
    }
}
