// This plugin is part of Agros2D.
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

#include "global.h"

#include "util/constants.h"

#include "util/util.h"
#include "logview.h"
#include "scene.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"

#include "solver/module.h"

#include "solver/problem.h"
#include "solver/coupling.h"
#include "solver/solutionstore.h"
#include "solver/plugin_interface.h"

#include "optilab/study.h"

#include "util/system_utils.h"

#include "boost/archive/archive_exception.hpp"

bool isPluginDir(const QString &path)
{
    QDir dir(path);

    QStringList filters;
    filters << "libagros_plugin_*.so" << "agros_plugin_*.dll";
    QStringList list = dir.entryList(filters);

    return (list.size() > 0);
}

QStringList pluginList()
{
    QString pluginPath = "";

    if (isPluginDir(datadir() + "/libs/"))
        pluginPath = datadir() + "/libs/";
    else if (isPluginDir(QCoreApplication::applicationDirPath() + "/../lib/"))
        pluginPath = QCoreApplication::applicationDirPath() + "/../lib/";

    if (pluginPath.isEmpty())
    {
        throw AgrosPluginException(QObject::tr("Could not load find plugins in directory."));
        assert(0);
    }

    QDir dir(pluginPath);

    QStringList filters;
    filters << "libagros_plugin_*.so" << "agros_plugin_*.dll";

    QStringList list;
    foreach (QString entry, dir.entryList(filters))
        list.append(QString("%1/%2").arg(pluginPath).arg(entry));

    return list;
}

void initSingleton()
{
    setlocale(LC_NUMERIC, "C");

    char *argv[] = {(char *) QString("%1/agros_python").arg(getenv("PWD")).toStdString().c_str(), NULL};
    int argc = sizeof(argv) / sizeof(char*) - 1;

    QCoreApplication::setApplicationVersion(versionString());
    QCoreApplication::setOrganizationName("agros");
    QCoreApplication::setOrganizationDomain("agros");
    QCoreApplication::setApplicationName("Agros Suite");

    // std::string codec
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    // force number format
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

    // init singleton
    Agros::createSingleton(QSharedPointer<Log>(new LogStdOut()));
}

void clearAgros2DCache()
{
    QFileInfoList listCache = QFileInfo(cacheProblemDir()).absoluteDir().entryInfoList();
    QFileInfoList listTemp = QFileInfo(tempProblemDir()).absoluteDir().entryInfoList();

    QFileInfoList list;
    list << listCache << listTemp;
    for (int i = 0; i < list.size(); ++i)
    {
        QFileInfo fileInfo = list.at(i);
        if (fileInfo.fileName() == "." || fileInfo.fileName() == ".." || fileInfo.fileName() == QString::number(QCoreApplication::applicationPid()))
            continue;

        if (fileInfo.isDir())
        {
            // process doesn't exists
            if (!SystemUtils::isProcessRunning(fileInfo.fileName().toInt()))
                removeDirectory(fileInfo.absoluteFilePath());
        }
    }

}

static QSharedPointer<Agros> m_singleton;

Agros::Agros(QSharedPointer<Log> log) : m_log(log)
{
    // qInfo() << "Agros::Agros(QSharedPointer<Log> log)";
    clearAgros2DCache();
    // qInfo() << "Agros::Agros(QSharedPointer<Log> log) - clearAgros2DCache";

    // preprocessor
    m_problem = new Problem();
    // qInfo() << "Agros::Agros(QSharedPointer<Log> log) - m_problem = new Problem();";

    initLists();
    // qInfo() << "Agros::Agros(QSharedPointer<Log> log) - initLists";

    m_configComputer = new Config();
    // qInfo() << "Agros::Agros(QSharedPointer<Log> log) - m_configComputer = new Config();";

    // plugins
    // read plugins
#ifdef AGROS_BUILD_STATIC
    foreach (QObject *obj, QPluginLoader::staticInstances())
    {
        PluginInterface *plugin = qobject_cast<PluginInterface *>(obj);
        m_plugins[plugin->fieldId()] = plugin;
    }
#else
    foreach (QString pluginPath, pluginList())
    {
        // load new plugin
        QPluginLoader *loader = new QPluginLoader(pluginPath);

        if (!loader)
        {
            throw AgrosPluginException(QObject::tr("Could not find 'agros2d_plugin_%1'").arg(pluginPath));
        }

        if (!loader->load())
        {
            QString error = loader->errorString();
            delete loader;
            throw AgrosPluginException(QObject::tr("Could not load 'agros2d_plugin_%1' (%2)").arg(pluginPath).arg(error));
        }

        assert(loader->instance());
        PluginInterface *plugin = qobject_cast<PluginInterface *>(loader->instance());
        m_plugins[plugin->fieldId()] = plugin;

        delete loader;
    }
#endif
}



void Agros::clear()
{    
    delete m_singleton.data()->m_problem;
    m_singleton.data()->m_computations.clear();

    delete m_singleton.data()->m_configComputer;

    // remove temp and cache plugins
    removeDirectory(cacheProblemDir());
    removeDirectory(tempProblemDir());
}

void Agros::addComputation(const QString &problemDir, QSharedPointer<Computation> comp)
{
    Agros::singleton()->m_computations[problemDir] = comp;
}

void Agros::clearComputations()
{
    foreach (QSharedPointer<Computation> computation, Agros::singleton()->m_computations)
    {
        // clear solutions
        computation->clearFieldsAndConfig();
        // remove computation from studies
        Agros::singleton()->problem()->studies()->removeComputation(computation);
        // remove from list
        Agros::singleton()->m_computations.remove(computation->problemDir());
    }

    Agros::singleton()->m_computations.clear();
}

void Agros::createSingleton(QSharedPointer<Log> log)
{
    m_singleton = QSharedPointer<Agros>(new Agros(log));
}

Agros *Agros::singleton()
{
    return m_singleton.data();
}

PluginInterface *Agros::loadPlugin(const QString &pluginName)
{
    if (Agros::singleton()->m_plugins.contains(pluginName))
        return Agros::singleton()->m_plugins[pluginName];

    assert(0);
}

// create script from model
QString createPythonFromModel()
{
    QString str;

    // import modules
    str += "import agros\n\n";

    // model
    str += "# problem\n";
    str += QString("problem = agros.problem(clear = True)\n");
    str += QString("problem.coordinate_type = \"%1\"\n").arg(coordinateTypeToStringKey(Agros::problem()->config()->coordinateType()));
    str += QString("problem.mesh_type = \"%1\"\n").arg(meshTypeToStringKey(Agros::problem()->config()->meshType()));

    // parameters
    QMap<QString, ProblemParameter> parameters = Agros::problem()->config()->parameters()->items();
    foreach (QString key, parameters.keys())
        str += QString("problem.parameters[\"%1\"] = %2\n").arg(key).arg(parameters[key].value());
    if (parameters.count() > 0)
        str += "\n";

    if (Agros::problem()->isHarmonic())
        str += QString("problem.frequency = %1\n").
                arg(Agros::problem()->config()->value(ProblemConfig::Frequency).value<Value>().toString());

    if (Agros::problem()->isTransient())
    {
        str += QString("problem.time_step_method = \"%1\"\n"
                       "problem.time_method_order = %2\n"
                       "problem.time_total = %3\n").
                arg(timeStepMethodToStringKey(static_cast<TimeStepMethod>(Agros::problem()->config()->value(ProblemConfig::TimeMethod).toInt()))).
                arg(Agros::problem()->config()->value(ProblemConfig::TimeOrder).toInt()).
                arg(Agros::problem()->config()->value(ProblemConfig::TimeTotal).toDouble());

        if ((static_cast<TimeStepMethod>(Agros::problem()->config()->value(ProblemConfig::TimeMethod).toInt())) == TimeStepMethod_BDFTolerance)
        {
            str += QString("problem.time_method_tolerance = %1\n").
                    arg(Agros::problem()->config()->value(ProblemConfig::TimeMethodTolerance).toDouble());
        }
        else
        {
            str += QString("problem.time_steps = %1\n").
                    arg(Agros::problem()->config()->value(ProblemConfig::TimeConstantTimeSteps).toInt());
        }
        if ((static_cast<TimeStepMethod>(Agros::problem()->config()->value(ProblemConfig::TimeMethod).toInt())) != TimeStepMethod_Fixed &&
                (Agros::problem()->config()->value(ProblemConfig::TimeInitialStepSize).toDouble() > 0.0))
            str += QString("problem.time_initial_time_step = %1\n").
                    arg(Agros::problem()->config()->value(ProblemConfig::TimeInitialStepSize).toDouble());
    }

    // fields
    str += "# fields\n";
    foreach (FieldInfo *fieldInfo, Agros::problem()->fieldInfos())
    {
        str += QString("# %1\n").arg(fieldInfo->fieldId());
        str += QString("%1 = problem.field(\"%2\")\n").
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
            str += QString("%1.matrix_solver_parameters[\"dealii_method\"] = \"%2\"\n").
                    arg(fieldInfo->fieldId()).
                    arg(iterLinearSolverDealIIMethodToStringKey(static_cast<IterSolverDealII>(fieldInfo->value(FieldInfo::LinearSolverIterDealIIMethod).toInt())));
            str += QString("%1.matrix_solver_parameters[\"dealii_preconditioner\"] = \"%2\"\n").
                    arg(fieldInfo->fieldId()).
                    arg(iterLinearSolverDealIIPreconditionerToStringKey(static_cast<PreconditionerDealII>(fieldInfo->value(FieldInfo::LinearSolverIterDealIIPreconditioner).toInt())));
            str += QString("%1.matrix_solver_parameters[\"dealii_tolerance\"] = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->value(FieldInfo::LinearSolverIterToleranceAbsolute).toDouble());
            str += QString("%1.matrix_solver_parameters[\"dealii_iterations\"] = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->value(FieldInfo::LinearSolverIterIters).toInt());
        }
        if (fieldInfo->matrixSolver() == SOLVER_EXTERNAL)
        {
            str += QString("%1.matrix_solver_parameters[\"external_solver\"] = \"%2\"\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->value(FieldInfo::LinearSolverExternalName).toString());
            str += QString("%1.matrix_solver_parameters[\"external_environment\"] = \"%2\"\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->value(FieldInfo::LinearSolverExternalCommandEnvironment).toString());
            str += QString("%1.matrix_solver_parameters[\"external_parameters\"] = \"%2\"\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->value(FieldInfo::LinearSolverExternalCommandParameters).toString());
        }

        if (Agros::problem()->isTransient())
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
                    arg(adaptivityEstimatorToStringKey(static_cast<AdaptivityEstimator>(fieldInfo->value(FieldInfo::AdaptivityEstimator).toInt())));

            str += QString("%1.adaptivity_parameters['strategy'] = \"%2\"\n").
                    arg(fieldInfo->fieldId()).
                    arg(adaptivityStrategyToStringKey(static_cast<AdaptivityStrategy>(fieldInfo->value(FieldInfo::AdaptivityStrategy).toInt())));

            if (fieldInfo->adaptivityType() == AdaptivityMethod_HP)
            {
                str += QString("%1.adaptivity_parameters['strategy_hp'] = \"%2\"\n").
                        arg(fieldInfo->fieldId()).
                        arg(adaptivityStrategyHPToStringKey(static_cast<AdaptivityStrategyHP>(fieldInfo->value(FieldInfo::AdaptivityStrategyHP).toInt())));

            }

            if (((static_cast<AdaptivityStrategy>(fieldInfo->value(FieldInfo::AdaptivityStrategy).toInt())) == AdaptivityStrategy_FixedFractionOfCells) ||
                    ((static_cast<AdaptivityStrategy>(fieldInfo->value(FieldInfo::AdaptivityStrategy).toInt())) == AdaptivityStrategy_FixedFractionOfTotalError))
            {
                str += QString("%1.adaptivity_parameters['fine_percentage'] = %2\n").
                        arg(fieldInfo->fieldId()).
                        arg(fieldInfo->value(FieldInfo::AdaptivityFinePercentage).toInt());

                str += QString("%1.adaptivity_parameters['coarse_percentage'] = %2\n").
                        arg(fieldInfo->fieldId()).
                        arg(fieldInfo->value(FieldInfo::AdaptivityCoarsePercentage).toInt());
            }

            if (Agros::problem()->isTransient())
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
                    arg(dampingTypeToStringKey(static_cast<DampingType>(fieldInfo->value(FieldInfo::NonlinearDampingType).toInt())));

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

        str += "# boundaries\n";
        foreach (SceneBoundary *boundary, Agros::problem()->scene()->boundaries->filter(fieldInfo).items())
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
                    if (value->isNumber())
                    {
                        variables += QString("\"%1\" : %2, ").
                                arg(variable.id()).
                                arg(value->toString());
                    }
                    else
                    {
                        variables += QString("\"%1\" : \"%2\", ").
                                arg(variable.id()).
                                arg(value->toString());
                    }
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

        str += "# materials\n";
        foreach (SceneMaterial *material, Agros::problem()->scene()->materials->filter(fieldInfo).items())
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
                    if (value->isNumber())
                    {
                        variables += QString("\"%1\" : %2, ").
                                arg(variable.id()).
                                arg(value->toString());
                    }
                    else
                    {
                        variables += QString("\"%1\" : \"%2\", ").
                                arg(variable.id()).
                                arg(value->toString());
                    }
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
    str += "geometry = problem.geometry()\n";

    // edges
    if (Agros::problem()->scene()->faces->count() > 0)
    {
        foreach (SceneFace *edge, Agros::problem()->scene()->faces->items())
        {
            Value startPointX = edge->nodeStart()->pointValue().x();
            Value startPointY = edge->nodeStart()->pointValue().y();
            Value endPointX = edge->nodeEnd()->pointValue().x();
            Value endPointY = edge->nodeEnd()->pointValue().y();

            str += QString("geometry.add_edge(%1, %2, %3, %4").
                    arg(startPointX.isNumber() ? QString::number(startPointX.number()) : QString("\"%1\"").arg(startPointX.toString())).
                    arg(startPointY.isNumber() ? QString::number(startPointY.number()) : QString("\"%1\"").arg(startPointY.toString())).
                    arg(endPointX.isNumber() ? QString::number(endPointX.number()) : QString("\"%1\"").arg(endPointX.toString())).
                    arg(endPointY.isNumber() ? QString::number(endPointY.number()) : QString("\"%1\"").arg(endPointY.toString()));

            if (edge->angle() > 0.0)
            {
                if (edge->angleValue().isNumber())
                    str += ", angle = " + QString::number(edge->angle());
                else
                    str += QString("%1").arg(edge->angleValue().toString());

                if (edge->segments() > 4)
                    str += ", segments = " + QString::number(edge->segments());
            }

            // boundaries
            if (Agros::problem()->fieldInfos().count() > 0)
            {
                int boundariesCount = 0;
                QString boundaries = ", boundaries = {";
                foreach (FieldInfo *fieldInfo, Agros::problem()->fieldInfos())
                {
                    SceneBoundary *marker = edge->marker(fieldInfo);

                    if (marker != Agros::problem()->scene()->boundaries->getNone(fieldInfo))
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
    if (Agros::problem()->scene()->labels->count() > 0)
    {
        foreach (SceneLabel *label, Agros::problem()->scene()->labels->items())
        {
            Value pointX = label->pointValue().x();
            Value pointY = label->pointValue().y();
            str += QString("geometry.add_label(%1, %2").
                    arg(pointX.isNumber() ? QString::number(pointX.number()) : QString("\"%1\"").arg(pointX.toString())).
                    arg(pointY.isNumber() ? QString::number(pointY.number()) : QString("\"%1\"").arg(pointY.toString()));

            if (label->area() > 0.0)
                str += ", area = " + QString::number(label->area());

            // refinements
            if (Agros::problem()->fieldInfos().count() > 0)
            {
                int refinementsCount = 0;
                QString refinements = ", refinements = {";
                foreach (FieldInfo *fieldInfo, Agros::problem()->fieldInfos())
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
            if (Agros::problem()->fieldInfos().count() > 0)
            {
                int ordersCount = 0;
                QString orders = ", orders = {";
                foreach (FieldInfo *fieldInfo, Agros::problem()->fieldInfos())
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
            if (Agros::problem()->fieldInfos().count() > 0)
            {
                QString materials = ", materials = {";
                foreach (FieldInfo *fieldInfo, Agros::problem()->fieldInfos())
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

        str += "\n";
    }

    if (Agros::problem()->recipes()->items().count() > 0)
    {
        str += "# recipes \n";
        foreach (ResultRecipe *recipe, Agros::problem()->recipes()->items())
        {
            if (LocalValueRecipe *localRecipe = dynamic_cast<LocalValueRecipe *>(recipe))
            {
                str += QString("%1.add_recipe_local_value(\"%2\", \"%3\", \"%4\", %5, %6, %7, %8)\n").
                        arg(localRecipe->fieldId()).
                        arg(localRecipe->name()).
                        arg(localRecipe->variable()).
                        arg(physicFieldVariableCompToStringKey(localRecipe->variableComponent())).
                        arg(localRecipe->point().x).
                        arg(localRecipe->point().y).
                        arg(localRecipe->timeStep()).
                        arg(localRecipe->adaptivityStep());
            }
            else if (SurfaceIntegralRecipe *surfaceRecipe = dynamic_cast<SurfaceIntegralRecipe *>(recipe))
            {
                QString edges;
                for (int i = 0; i < surfaceRecipe->edges().count(); i++)
                    if (i < surfaceRecipe->edges().count() - 1)
                        edges += QString("%1, ").arg(surfaceRecipe->edges()[i]);
                    else
                        edges += QString("%1").arg(surfaceRecipe->edges()[i]);

                str += QString("%1.add_recipe_surface_integral(\"%2\", \"%3\", [%4], %5, %6)\n").
                        arg(surfaceRecipe->fieldId()).
                        arg(surfaceRecipe->name()).
                        arg(surfaceRecipe->variable()).
                        arg(edges).
                        arg(surfaceRecipe->timeStep()).
                        arg(surfaceRecipe->adaptivityStep());
            }
            else if (VolumeIntegralRecipe *volumeRecipe = dynamic_cast<VolumeIntegralRecipe *>(recipe))
            {
                QString labels;
                for (int i = 0; i < volumeRecipe->labels().count(); i++)
                    if (i < volumeRecipe->labels().count() - 1)
                        labels += QString("%1, ").arg(volumeRecipe->labels()[i]);
                    else
                        labels += QString("%1").arg(volumeRecipe->labels()[i]);

                str += QString("%1.add_recipe_volume_integral(\"%2\", \"%3\", [%4], %5, %6)\n").
                        arg(volumeRecipe->fieldId()).
                        arg(volumeRecipe->name()).
                        arg(volumeRecipe->variable()).
                        arg(labels).
                        arg(volumeRecipe->timeStep()).
                        arg(volumeRecipe->adaptivityStep());
            }
        }

        str += "\n";
    }

    if (Agros::problem()->studies()->items().count() > 0)
    {
        str += "# studies\n";
        foreach (Study *study, Agros::problem()->studies()->items())
        {
            str += QString("study_%1 = problem.add_study(\"%1\")\n").arg(studyTypeToStringKey(study->type()));

            // parameters
            foreach (Parameter parameter, study->parameters())
            {
                str += QString("study_%1.add_parameter(\"%2\", %3, %4)\n").
                        arg(studyTypeToStringKey(study->type())).
                        arg(parameter.name()).
                        arg(parameter.lowerBound()).
                        arg(parameter.upperBound());
            }

            // functionals
            foreach (Functional functional, study->functionals())
                str += QString("study_%1.add_functional(\"%2\", \"%3\", %4)\n").
                        arg(studyTypeToStringKey(study->type())).
                        arg(functional.name()).
                        arg(functional.expression()).
                        arg(functional.weight());

            // settings
            str += QString("study_%1.clear_solution = %2\n").
                    arg(studyTypeToStringKey(study->type())).
                    arg(study->value(Study::General_ClearSolution).toBool() ? "True" : "False");
            str += QString("study_%1.solve_problem = %2\n").
                    arg(studyTypeToStringKey(study->type())).
                    arg(study->value(Study::General_SolveProblem).toBool() ? "True" : "False");

            foreach (Study::Type type, study->keys().keys())
            {
                if (study->keys()[type].toLower().startsWith(studyTypeToStringKey(study->type()).toLower()))
                {
                    // only keys for selected study
                    QString key = study->keys()[type].mid(studyTypeToStringKey(study->type()).count() + 1, -1);

                    QString value;
                    if (study->value(type).type() == QVariant::String)
                        value = QString("\"%1\"").arg(study->value(type).toString());
                    else if (study->value(type).type() == QVariant::Bool)
                        value = (study->value(type).toBool()) ? "True" : "False";
                    else
                        value = study->value(type).toString();

                    str += QString("study_%1.settings[\"%2\"] = %3\n").
                            arg(studyTypeToStringKey(study->type())).
                            arg(key).
                            arg(value);
                }
            }

            str += "\n";
        }
    }

    return str;
}
