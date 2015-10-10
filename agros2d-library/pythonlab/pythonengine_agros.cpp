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

#include <Python.h>

#include "../resources_source/python/agros2d.cpp"

#include "pythonengine_agros.h"

#include "logview.h"
#include "solver/plugin_interface.h"
#include "solver/module.h"
#include "util/memory_monitor.h"

#ifdef TBB_FOUND
#include <tbb/tbb.h>
tbb::mutex runPythonHeaderMutex;
#endif

// current python engine agros
AGROS_LIBRARY_API PythonEngineAgros *currentPythonEngineAgros()
{
    return static_cast<PythonEngineAgros *>(currentPythonEngine());
}

void PythonEngineAgros::addCustomExtensions()
{
    PythonEngine::addCustomExtensions();

    // init agros cython extensions
    PyObject *m = PyInit_agros2d();
    PyDict_SetItemString(PyImport_GetModuleDict(), "agros2d", m);
}

void PythonEngineAgros::addCustomFunctions()
{
    addFunctions(readFileContent(datadir() + "/resources/python/functions_agros2d.py"));
}

void PythonEngineAgros::abortScript()
{
    if (Agros2D::computation()->isMeshing() || Agros2D::computation()->isSolving())
        Agros2D::computation()->doAbortSolve();

    PythonEngine::abortScript();
}

void PythonEngineAgros::materialValues(const QString &function, double from, double to,
                                       QVector<double> *keys, QVector<double> *values, int count)
{
    if (function.isEmpty())
        return;

    // function
    bool succesfulRun = runExpression(function);
    if (!succesfulRun)
    {
        ErrorResult result = currentPythonEngineAgros()->parseError();
        qDebug() << "Function: " << result.error();
    }

    // prepare keys
    double step = (to - from) / (count - 1);
    QString keysVector = "[";
    for (int i = 0; i < count; i++)
    {
        double key = from + i * step;
        keys->append(key);

        if (i == 0)
            keysVector += QString("%1").arg(key + EPS_ZERO);
        else if (i == (count - 1))
            keysVector += QString(", %1]").arg(key - EPS_ZERO);
        else
            keysVector += QString(", %1").arg(key);
    }

    // run expression
    runExpression(QString("agros2d_material_values = agros2d_material_eval(%1)").arg(keysVector));

    // extract values
    PyObject *result = PyDict_GetItemString(dict(), "agros2d_material_values");
    if (result)
    {
        Py_INCREF(result);
        for (int i = 0; i < count; i++)
            values->append(PyFloat_AsDouble(PyList_GetItem(result, i)));
        Py_XDECREF(result);
    }

    // remove variables
    runExpression("del agros2d_material; del agros2d_material_values");

    // error during execution
    if (keys->size() != values->size())
    {
        keys->clear();
        values->clear();
    }
}

QStringList PythonEngineAgros::testSuiteScenarios()
{
    QStringList list;

    // run expression
    bool successfulRun = currentPythonEngine()->runExpression(QString("from test_suite.scenario import find_all_scenarios; agros2d_scenarios = find_all_scenarios()"));

    if (successfulRun)
    {
        // extract values
        PyObject *result = PyDict_GetItemString(currentPythonEngine()->dict(), "agros2d_scenarios");
        if (result)
        {
            Py_INCREF(result);
            for (int i = 0; i < PyList_Size(result); i++)
            {
                QString testName = QString::fromWCharArray(PyUnicode_AsUnicode(PyList_GetItem(result, i)));

                list.append(testName);
            }
            Py_XDECREF(result);
        }
    }
    else
    {
        // parse error
        ErrorResult result = currentPythonEngine()->parseError();
        qDebug() << result.error();
        qDebug() << result.tracebackToString();
    }

    // remove variables
    currentPythonEngine()->runExpression("del agros2d_scenarios");

    return list;
}

// *****************************************************************************

// create script from model
QString createPythonFromModel()
{
    QString str;

    // import modules
    str += "import agros2d as a2d\n\n";

    // startup script
    if (!Agros2D::preprocessor()->setting()->value(ProblemSetting::Problem_StartupScript).toString().trimmed().isEmpty())
    {
        str += "# startup script\n";
        str += Agros2D::preprocessor()->setting()->value(ProblemSetting::Problem_StartupScript).toString();
        str += "\n\n";
    }

    // model
    str += "# problem\n";
    str += QString("problem = a2d.problem(clear = True)\n");
    str += QString("problem.coordinate_type = \"%1\"\n").arg(coordinateTypeToStringKey(Agros2D::preprocessor()->config()->coordinateType()));
    str += QString("problem.mesh_type = \"%1\"\n").arg(meshTypeToStringKey(Agros2D::preprocessor()->config()->meshType()));

    if (Agros2D::preprocessor()->isHarmonic())
        str += QString("problem.frequency = %1\n").
                arg(Value::parseValueFromString(Agros2D::preprocessor()->config()->value(ProblemConfig::Frequency).toString()).toString());

    if (Agros2D::preprocessor()->isTransient())
    {
        str += QString("problem.time_step_method = \"%1\"\n"
                       "problem.time_method_order = %2\n"
                       "problem.time_total = %3\n").
                arg(timeStepMethodToStringKey((TimeStepMethod) Agros2D::preprocessor()->config()->value(ProblemConfig::TimeMethod).toInt())).
                arg(Agros2D::preprocessor()->config()->value(ProblemConfig::TimeOrder).toInt()).
                arg(Agros2D::preprocessor()->config()->value(ProblemConfig::TimeTotal).toDouble());

        if (((TimeStepMethod) Agros2D::preprocessor()->config()->value(ProblemConfig::TimeMethod).toInt()) == TimeStepMethod_BDFTolerance)
        {
            str += QString("problem.time_method_tolerance = %1\n").
                    arg(Agros2D::preprocessor()->config()->value(ProblemConfig::TimeMethodTolerance).toDouble());
        }
        else
        {
            str += QString("problem.time_steps = %1\n").
                    arg(Agros2D::preprocessor()->config()->value(ProblemConfig::TimeConstantTimeSteps).toInt());
        }
        if (((TimeStepMethod) Agros2D::preprocessor()->config()->value(ProblemConfig::TimeMethod).toInt()) != TimeStepMethod_Fixed &&
                (Agros2D::preprocessor()->config()->value(ProblemConfig::TimeInitialStepSize).toDouble() > 0.0))
            str += QString("problem.time_initial_time_step = %1\n").
                    arg(Agros2D::preprocessor()->config()->value(ProblemConfig::TimeInitialStepSize).toDouble());
    }

    // fields
    str += "\n# fields\n";
    foreach (FieldInfo *fieldInfo, Agros2D::preprocessor()->fieldInfos())
    {
        str += QString("# %1\n").arg(fieldInfo->fieldId());

        // str += QString("%1 = a2d.field(field_id = \"%2\")\n").
        str += QString("%1 = a2d.field(\"%2\")\n").
                arg(fieldInfo->fieldId()).
                arg(fieldInfo->fieldId());
        str += QString("%1.analysis_type = \"%2\"\n").
                arg(fieldInfo->fieldId()).
                arg(analysisTypeToStringKey(fieldInfo->analysisType()));
        str += QString("%1.matrix_solver = \"%2\"\n").
                arg(fieldInfo->fieldId()).
                arg(matrixSolverTypeToStringKey(fieldInfo->matrixSolver()));

        if (fieldInfo->matrixSolver() == SOLVER_DEALII)
        {
                str += QString("%1.matrix_solver_parameters[\"method_dealii\"] = \"%2\"\n").
                        arg(fieldInfo->fieldId()).
                        arg(iterLinearSolverDealIIMethodToStringKey((IterSolverDealII) fieldInfo->value(FieldInfo::LinearSolverIterDealIIMethod).toInt()));
                str += QString("%1.matrix_solver_parameters[\"preconditioner_dealii\"] = \"%2\"\n").
                        arg(fieldInfo->fieldId()).
                        arg(iterLinearSolverDealIIPreconditionerToStringKey((PreconditionerDealII) fieldInfo->value(FieldInfo::LinearSolverIterDealIIPreconditioner).toInt()));
        }
        if (fieldInfo->matrixSolver() == SOLVER_PARALUTION)
        {
                str += QString("%1.matrix_solver_parameters[\"method_paralution\"] = \"%2\"\n").
                        arg(fieldInfo->fieldId()).
                        arg(iterLinearSolverPARALUTIONMethodToStringKey((IterSolverPARALUTION) fieldInfo->value(FieldInfo::LinearSolverIterPARALUTIONMethod).toInt()));
                str += QString("%1.matrix_solver_parameters[\"preconditioner_paralution\"] = \"%2\"\n").
                        arg(fieldInfo->fieldId()).
                        arg(iterLinearSolverPARALUTIONPreconditionerToStringKey((PreconditionerPARALUTION) fieldInfo->value(FieldInfo::LinearSolverIterPARALUTIONPreconditioner).toInt()));
                str += QString("%1.matrix_solver_parameters[\"double_precision\"] = %2\n").
                        arg(fieldInfo->fieldId()).
                        arg(fieldInfo->value(FieldInfo::LinearSolverIterPARALUTIONDoublePrecision).toBool() ? "True" : "False");

        }
        if (isMatrixSolverIterative(fieldInfo->matrixSolver()))
        {           
            str += QString("%1.matrix_solver_parameters[\"tolerance\"] = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->value(FieldInfo::LinearSolverIterToleranceAbsolute).toDouble());
            str += QString("%1.matrix_solver_parameters[\"iterations\"] = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->value(FieldInfo::LinearSolverIterIters).toInt());
        }

        if (Agros2D::preprocessor()->isTransient())
        {
            if (fieldInfo->analysisType() == analysisTypeFromStringKey("transient"))
            {
                str += QString("%1.transient_initial_condition = %2\n").
                        arg(fieldInfo->fieldId()).
                        arg(fieldInfo->value(FieldInfo::TransientInitialCondition).toDouble());
            }
            else
            {
                str += QString("%1.transient_time_skip = %2\n").
                        arg(fieldInfo->fieldId()).
                        arg(fieldInfo->value(FieldInfo::TransientTimeSkip).toInt());
            }
        }

        str += QString("%1.number_of_refinements = %2\n").
                arg(fieldInfo->fieldId()).
                arg(fieldInfo->value(FieldInfo::SpaceNumberOfRefinements).toInt());

        str += QString("%1.polynomial_order = %2\n").
                arg(fieldInfo->fieldId()).
                arg(fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt());

        str += QString("%1.adaptivity_type = \"%2\"\n").
                arg(fieldInfo->fieldId()).
                arg(adaptivityTypeToStringKey(fieldInfo->adaptivityType()));

        if (fieldInfo->adaptivityType() != AdaptivityMethod_None)
        {
            str += QString("%1.adaptivity_parameters['steps'] = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->value(FieldInfo::AdaptivitySteps).toInt());

            str += QString("%1.adaptivity_parameters['tolerance'] = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->value(FieldInfo::AdaptivityTolerance).toDouble());

            str += QString("%1.adaptivity_parameters['estimator'] = \"%2\"\n").
                    arg(fieldInfo->fieldId()).
                    arg(adaptivityEstimatorToStringKey((AdaptivityEstimator) fieldInfo->value(FieldInfo::AdaptivityEstimator).toInt()));

            str += QString("%1.adaptivity_parameters['strategy'] = \"%2\"\n").
                    arg(fieldInfo->fieldId()).
                    arg(adaptivityStrategyToStringKey((AdaptivityStrategy) fieldInfo->value(FieldInfo::AdaptivityStrategy).toInt()));

            if (fieldInfo->adaptivityType() == AdaptivityMethod_HP)
            {
                str += QString("%1.adaptivity_parameters['strategy_hp'] = \"%2\"\n").
                        arg(fieldInfo->fieldId()).
                        arg(adaptivityStrategyHPToStringKey((AdaptivityStrategyHP) fieldInfo->value(FieldInfo::AdaptivityStrategyHP).toInt()));

            }

            if (((AdaptivityStrategy) fieldInfo->value(FieldInfo::AdaptivityStrategy).toInt() == AdaptivityStrategy_FixedFractionOfCells) ||
                        ((AdaptivityStrategy) fieldInfo->value(FieldInfo::AdaptivityStrategy).toInt() == AdaptivityStrategy_FixedFractionOfTotalError))
            {
                str += QString("%1.adaptivity_parameters['fine_percentage'] = %2\n").
                        arg(fieldInfo->fieldId()).
                        arg(fieldInfo->value(FieldInfo::AdaptivityFinePercentage).toInt());

                str += QString("%1.adaptivity_parameters['coarse_percentage'] = %2\n").
                        arg(fieldInfo->fieldId()).
                        arg(fieldInfo->value(FieldInfo::AdaptivityCoarsePercentage).toInt());
            }

            if (Agros2D::preprocessor()->isTransient())
            {
                str += QString("%1.adaptivity_parameters['transient_back_steps'] = %2\n").
                        arg(fieldInfo->fieldId()).
                        arg(fieldInfo->value(FieldInfo::AdaptivityTransientBackSteps).toInt());

                str += QString("%1.adaptivity_parameters['transient_redone_steps'] = %2\n").
                        arg(fieldInfo->fieldId()).
                        arg(fieldInfo->value(FieldInfo::AdaptivityTransientRedoneEach).toInt());
            }
        }

        str += QString("%1.solver = \"%2\"\n").
                arg(fieldInfo->fieldId()).
                arg(linearityTypeToStringKey(fieldInfo->linearityType()));

        if (fieldInfo->linearityType() != LinearityType_Linear)
        {
            str += QString("%1.solver_parameters['residual'] = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->value(FieldInfo::NonlinearResidualNorm).toDouble());

            str += QString("%1.solver_parameters['relative_change_of_solutions'] = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->value(FieldInfo::NonlinearRelativeChangeOfSolutions).toDouble());

            str += QString("%1.solver_parameters['damping'] = \"%2\"\n").
                    arg(fieldInfo->fieldId()).
                    arg(dampingTypeToStringKey((DampingType)fieldInfo->value(FieldInfo::NonlinearDampingType).toInt()));

            str += QString("%1.solver_parameters['damping_factor'] = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->value(FieldInfo::NonlinearDampingCoeff).toDouble());

            str += QString("%1.solver_parameters['damping_factor_increase_steps'] = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->value(FieldInfo::NonlinearStepsToIncreaseDampingFactor).toInt());

            str += QString("%1.solver_parameters['damping_factor_decrease_ratio'] = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->value(FieldInfo::NonlinearDampingFactorDecreaseRatio).toDouble());
        }

        // newton
        if (fieldInfo->linearityType() == LinearityType_Newton)
        {
            str += QString("%1.solver_parameters['jacobian_reuse'] = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg((fieldInfo->value(FieldInfo::NewtonReuseJacobian).toBool()) ? "True" : "False");

            str += QString("%1.solver_parameters['jacobian_reuse_ratio'] = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->value(FieldInfo::NewtonJacobianReuseRatio).toDouble());

            str += QString("%1.solver_parameters['jacobian_reuse_steps'] = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->value(FieldInfo::NewtonMaxStepsReuseJacobian).toInt());

        }

        // picard
        if (fieldInfo->linearityType() == LinearityType_Picard)
        {
            str += QString("%1.solver_parameters['anderson_acceleration'] = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg((fieldInfo->value(FieldInfo::PicardAndersonAcceleration).toBool()) ? "True" : "False");

            if (fieldInfo->value(FieldInfo::PicardAndersonAcceleration).toBool())
            {
                str += QString("%1.solver_parameters['anderson_beta'] = %2\n").
                        arg(fieldInfo->fieldId()).
                        arg(fieldInfo->value(FieldInfo::PicardAndersonBeta).toDouble());

                str += QString("%1.solver_parameters['anderson_last_vectors'] = %2\n").
                        arg(fieldInfo->fieldId()).
                        arg(fieldInfo->value(FieldInfo::PicardAndersonNumberOfLastVectors).toInt());
            }
        }

        str += "\n";

        str += "\n# boundaries\n";
        foreach (SceneBoundary *boundary, Agros2D::preprocessor()->scene()->boundaries->filter(fieldInfo).items())
        {
            const QMap<uint, QSharedPointer<Value> > values = boundary->values();

            QString variables = "{";

            Module::BoundaryType boundaryType = fieldInfo->boundaryType(boundary->type());
            foreach (Module::BoundaryTypeVariable variable, boundaryType.variables())
            {
                QSharedPointer<Value> value = values[qHash(variable.id())];

                if (value->isTimeDependent() || value->isCoordinateDependent() || (value->hasTable() && (fieldInfo->linearityType() != LinearityType_Linear)))
                {
                    variables += QString("\"%1\" : { \"expression\" : \"%2\" }, ").
                            arg(variable.id()).
                            arg(value->text());
                }
                else
                {
                    variables += QString("\"%1\" : %2, ").
                            arg(variable.id()).
                            arg(value->toString());
                }
            }
            variables = (variables.endsWith(", ") ? variables.left(variables.length() - 2) : variables) + "}";

            str += QString("%1.add_boundary(\"%2\", \"%3\", %4)\n").
                    arg(fieldInfo->fieldId()).
                    arg(boundary->name()).
                    arg(boundary->type()).
                    arg(variables);
        }

        str += "\n";

        str += "\n# materials\n";
        foreach (SceneMaterial *material, Agros2D::preprocessor()->scene()->materials->filter(fieldInfo).items())
        {
            const QMap<uint, QSharedPointer<Value> > values = material->values();

            QString variables = "{";
            foreach (Module::MaterialTypeVariable variable, material->fieldInfo()->materialTypeVariables())
            {
                QSharedPointer<Value> value = values[qHash(variable.id())];

                if (value->hasTable() && !value->isNumber())
                {
                    variables += QString("\"%1\" : { \"expression\" : \"%2\", \"x\" : [%3], \"y\" : [%4] }, ").
                            arg(variable.id()).
                            arg(value->text()).
                            arg(value->table().toStringX()).
                            arg(value->table().toStringY());

                }
                else if (value->hasTable() && value->isNumber())
                {
                    variables += QString("\"%1\" : { \"value\" : %2, \"x\" : [%3], \"y\" : [%4], \"interpolation\" : \"%5\", \"extrapolation\" : \"%6\", \"derivative_at_endpoints\" : \"%7\" }, ").
                            arg(variable.id()).
                            arg(value->number()).
                            arg(value->table().toStringX()).
                            arg(value->table().toStringY()).
                            arg(dataTableTypeToStringKey(value->table().type())).
                            arg(value->table().extrapolateConstant() == true ? "constant" : "linear").
                            arg(value->table().splineFirstDerivatives() == true ? "first" : "second");
                }
                else if (value->isTimeDependent() || value->isCoordinateDependent())
                {
                    variables += QString("\"%1\" : { \"expression\" : \"%2\" }, ").
                            arg(variable.id()).
                            arg(value->text());
                }
                else
                {
                    variables += QString("\"%1\" : %2, ").
                            arg(variable.id()).
                            arg(value->toString());
                }
            }

            variables = (variables.endsWith(", ") ? variables.left(variables.length() - 2) : variables) + "}";

            str += QString("%1.add_material(\"%2\", %3)\n").
                    arg(fieldInfo->fieldId()).
                    arg(material->name()).
                    arg(variables);
        }

        str += "\n";
    }

    // geometry
    str += "# geometry\n";
    str += "geometry = a2d.geometry\n";

    // edges
    if (Agros2D::preprocessor()->scene()->edges->count() > 0)
    {
        //str += "\n# edges\n";
        foreach (SceneEdge *edge, Agros2D::preprocessor()->scene()->edges->items())
        {
            str += QString("geometry.add_edge(%1, %2, %3, %4").
                    arg(edge->nodeStart()->pointValue().x().isNumber() ? QString::number(edge->nodeStart()->point().x) : edge->nodeStart()->pointValue().x().toString()).
                    arg(edge->nodeStart()->pointValue().y().isNumber() ? QString::number(edge->nodeStart()->point().y) : edge->nodeStart()->pointValue().y().toString()).
                    arg(edge->nodeEnd()->pointValue().x().isNumber() ? QString::number(edge->nodeEnd()->point().x) : edge->nodeEnd()->pointValue().x().toString()).
                    arg(edge->nodeEnd()->pointValue().y().isNumber() ? QString::number(edge->nodeEnd()->point().y) : edge->nodeEnd()->pointValue().y().toString());

            if (edge->angle() > 0.0)
            {
                str += ", angle = " + QString::number(edge->angle());
                if (edge->segments() > 4)
                    str += ", segments = " + QString::number(edge->segments());
                if (!edge->isCurvilinear())
                    str += ", curvilinear = False";
            }

            // refinement
            if (Agros2D::preprocessor()->fieldInfos().count() > 0)
            {
                int refinementCount = 0;
                QString refinements = ", refinements = {";
                foreach (FieldInfo *fieldInfo, Agros2D::preprocessor()->fieldInfos())
                {
                    if (fieldInfo->edgeRefinement(edge) > 0)
                    {
                        refinements += QString("\"%1\" : %2, ").
                                arg(fieldInfo->fieldId()).
                                arg(fieldInfo->edgeRefinement(edge));

                        refinementCount++;
                    }
                }
                refinements = (refinements.endsWith(", ") ? refinements.left(refinements.length() - 2) : refinements) + "}";

                if (refinementCount > 0)
                    str += refinements;
            }

            // boundaries
            if (Agros2D::preprocessor()->fieldInfos().count() > 0)
            {
                int boundariesCount = 0;
                QString boundaries = ", boundaries = {";
                foreach (FieldInfo *fieldInfo, Agros2D::preprocessor()->fieldInfos())
                {
                    SceneBoundary *marker = edge->marker(fieldInfo);

                    if (marker != Agros2D::preprocessor()->scene()->boundaries->getNone(fieldInfo))
                    {
                        boundaries += QString("\"%1\" : \"%2\", ").
                                arg(fieldInfo->fieldId()).
                                arg(marker->name());

                        boundariesCount++;
                    }
                }
                boundaries = (boundaries.endsWith(", ") ? boundaries.left(boundaries.length() - 2) : boundaries) + "}";
                if (boundariesCount > 0)
                    str += boundaries;
            }

            str += ")\n";
        }
        str += "\n";
    }

    // labels
    if (Agros2D::preprocessor()->scene()->labels->count() > 0)
    {
        //str += "# labels\n";
        foreach (SceneLabel *label, Agros2D::preprocessor()->scene()->labels->items())
        {
            str += QString("geometry.add_label(%1, %2").
                    arg(label->pointValue().x().isNumber() ? QString::number(label->point().x) : label->pointValue().x().toString()).
                    arg(label->pointValue().y().isNumber() ? QString::number(label->point().y) : label->pointValue().y().toString());

            if (label->area() > 0.0)
                str += ", area = " + QString::number(label->area());

            // refinements
            if (Agros2D::preprocessor()->fieldInfos().count() > 0)
            {
                int refinementsCount = 0;
                QString refinements = ", refinements = {";
                foreach (FieldInfo *fieldInfo, Agros2D::preprocessor()->fieldInfos())
                {
                    if (fieldInfo->labelRefinement(label) > 0)
                    {
                        refinements += QString("\"%1\" : %2, ").
                                arg(fieldInfo->fieldId()).
                                arg(fieldInfo->labelRefinement(label));

                        refinementsCount++;
                    }
                }
                refinements = (refinements.endsWith(", ") ? refinements.left(refinements.length() - 2) : refinements) + "}";
                if (refinementsCount > 0)
                    str += refinements;
            }

            // orders
            if (Agros2D::preprocessor()->fieldInfos().count() > 0)
            {
                int ordersCount = 0;
                QString orders = ", orders = {";
                foreach (FieldInfo *fieldInfo, Agros2D::preprocessor()->fieldInfos())
                {
                    if (fieldInfo->labelPolynomialOrder(label) != fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt())
                    {
                        orders += QString("\"%1\" : %2, ").
                                arg(fieldInfo->fieldId()).
                                arg(fieldInfo->labelPolynomialOrder(label));

                        ordersCount++;
                    }
                }
                orders = (orders.endsWith(", ") ? orders.left(orders.length() - 2) : orders) + "}";
                if (ordersCount > 0)
                    str += orders;
            }

            // materials
            if (Agros2D::preprocessor()->fieldInfos().count() > 0)
            {
                QString materials = ", materials = {";
                foreach (FieldInfo *fieldInfo, Agros2D::preprocessor()->fieldInfos())
                {
                    SceneMaterial *marker = label->marker(fieldInfo);

                    materials += QString("\"%1\" : \"%2\", ").
                            arg(fieldInfo->fieldId()).
                            arg(marker->name());
                }
                materials = (materials.endsWith(", ") ? materials.left(materials.length() - 2) : materials) + "}";
                str += materials;
            }

            str += ")\n";
        }
    }
    str += "a2d.view.zoom_best_fit()";

    return str;
}

// ************************************************************************************

void openFile(const std::string &file, bool openWithSolution)
{
    try
    {
        Agros2D::preprocessor()->readProblemFromFile(QString::fromStdString(file));

        if (openWithSolution)
            Agros2D::computation()->readSolutionFromFile(QString::fromStdString(file));
    }
    catch (AgrosException &e)
    {
        throw logic_error(e.toString().toStdString());
    }
}

void saveFile(const std::string &file, bool saveWithSolution)
{
    try
    {
        Agros2D::preprocessor()->writeProblemToFile(QString::fromStdString(file), false);

        if (saveWithSolution || silentMode())
            Agros2D::computation()->writeSolutionToFile(QString::fromStdString(file));
    }
    catch (AgrosException &e)
    {
        throw logic_error(e.toString().toStdString());
    }
}

int appTime()
{
    return Agros2D::memoryMonitor()->appTime();
}

void memoryUsage(std::vector<int> &time, std::vector<int> &usage)
{
    time = Agros2D::memoryMonitor()->memoryTime().toVector().toStdVector();
    usage = Agros2D::memoryMonitor()->memoryUsage().toVector().toStdVector();
}

char *pyVersion()
{
    return const_cast<char*>(QApplication::applicationVersion().toStdString().c_str());
}

void pyQuit()
{
    // doesn't work without main event loop (run from script)
    // QApplication::exit(0);

    exit(0);
}

char *pyInput(std::string str)
{
    QString text = QInputDialog::getText(QApplication::activeWindow(), QObject::tr("Script input"), QString::fromStdString(str));
    return const_cast<char*>(text.toStdString().c_str());
}

void pyMessage(std::string str)
{
    QMessageBox::information(QApplication::activeWindow(), QObject::tr("Script message"), QString::fromStdString(str));
}

std::string pyDatadir(std::string str)
{
    QString path = QFileInfo(datadir() + "/" + QString::fromStdString(str)).absoluteFilePath();
    return compatibleFilename(path).toStdString();
}

// ************************************************************************************

void PyOptions::setNumberOfThreads(int threads)
{
    // if (threads < 1 || threads > omp_get_max_threads())
    //     throw out_of_range(QObject::tr("Number of threads is out of range (1 - %1).").arg(omp_get_max_threads()).toStdString());

    Agros2D::configComputer()->setValue(Config::Config_NumberOfThreads, threads);
}

void PyOptions::setCacheSize(int size)
{
    if (size < 2 || size > 50)
        throw out_of_range(QObject::tr("Cache size is out of range (2 - 50).").toStdString());

    Agros2D::configComputer()->setValue(Config::Config_CacheSize, size);
}

void PyOptions::setDumpFormat(std::string format)
{
    if (dumpFormatStringKeys().contains(QString::fromStdString(format)))
        Agros2D::configComputer()->setValue(Config::Config_LinearSystemFormat, (MatrixExportFormat) dumpFormatFromStringKey(QString::fromStdString(format)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(dumpFormatStringKeys())).toStdString());
}

