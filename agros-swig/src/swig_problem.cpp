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

#include "swig_problem.h"
#include "swig_field.h"
#include "swig_geometry.h"

#include "solver/problem_result.h"
#include "solver/coupling.h"
#include "solver/solutionstore.h"
#include "solver/plugin_interface.h"


void SwigProblemBase::getParameters(std::vector<std::string> &keys) const
{
    foreach (QString key, m_problem->config()->parameters()->items().keys())
        keys.push_back(key.toStdString());
}

double SwigProblemBase::getParameter(const string &key) const
{
    if (m_problem->config()->parameters()->items().contains(QString::fromStdString(key)))
    {
        return m_problem->config()->parameters()->number(QString::fromStdString(key));
    }
    else
    {
        QString str;
        foreach (QString key, m_problem->config()->parameters()->items().keys())
            str += key + ", ";
        if (str.length() > 0)
            str = str.left(str.length() - 2);

        throw logic_error(QObject::tr("Invalid argument. Valid keys: %1").arg(str).toStdString());
    }
}

void SwigProblemBase::setParameter(const string &key, double value)
{
    m_problem->config()->parameters()->set(QString::fromStdString(key), value);
}

std::string SwigProblemBase::getCouplingType(const std::string &sourceField, const std::string &targetField) const
{
    QString source = QString::fromStdString(sourceField);
    QString target = QString::fromStdString(targetField);

    checkExistingFields(source, target);

    if (m_problem->hasCoupling(source, target))
    {
        CouplingInfo *couplingInfo = m_problem->couplingInfo(source, target);
        return couplingTypeToStringKey(couplingInfo->couplingType()).toStdString();
    }
    else
        throw logic_error(QObject::tr("Coupling '%1' + '%2' doesn't exists.").arg(source).arg(target).toStdString());
}

void SwigProblemBase::setCouplingType(const std::string &sourceField, const std::string &targetField, const std::string &type)
{
    QString source = QString::fromStdString(sourceField);
    QString target = QString::fromStdString(targetField);

    checkExistingFields(source, target);

    if (m_problem->hasCoupling(source, target))
    {
        CouplingInfo *couplingInfo = m_problem->couplingInfo(source, target);
        if (couplingTypeStringKeys().contains(QString::fromStdString(type)))
            couplingInfo->setCouplingType(couplingTypeFromStringKey(QString::fromStdString(type)));
        else
            throw invalid_argument(QObject::tr("Invalid coupling type key. Valid keys: %1").arg(stringListToString(couplingTypeStringKeys())).toStdString());
    }
    else
        throw logic_error(QObject::tr("Coupling '%1' + '%2' doesn't exists.").arg(source).arg(target).toStdString());
}

void SwigProblemBase::checkExistingFields(const QString &sourceField, const QString &targetField) const
{
    if (m_problem->fieldInfos().isEmpty())
        throw logic_error(QObject::tr("No fields are defined.").toStdString());

    if (!m_problem->fieldInfos().contains(sourceField))
        throw logic_error(QObject::tr("Source field '%1' is not defined.").arg(sourceField).toStdString());

    if (!m_problem->fieldInfos().contains(targetField))
        throw logic_error(QObject::tr("Target field '%1' is not defined.").arg(targetField).toStdString());
}

// *****************************************************************************************************

SwigProblem::SwigProblem(bool clearProblem) : SwigProblemBase()
{
    m_problem = QSharedPointer<Problem>(Agros::problem());

    if (clearProblem)
        clear();
}

SwigProblem::~SwigProblem()
{
    // qDebug() << "PyProblem::~PyProblem() - m_problem " << m_problem.isNull();
    // qDebug() << "PyProblem::~PyProblem() - m_problemBase " << m_problemBase.isNull();
}

void SwigProblem::clear()
{
    m_problem->clearFieldsAndConfig();
}

SwigField *SwigProblem::field(const std::string &fieldId)
{
    return new SwigField(fieldId);
}

SwigGeometry *SwigProblem::geometry()
{
    return new SwigGeometry();
}

SwigComputation *SwigProblem::computation(bool newComputation)
{
    return new SwigComputation(newComputation);
}

void SwigProblem::setCoordinateType(const std::string &coordinateType)
{
    if (coordinateTypeStringKeys().contains(QString::fromStdString(coordinateType)))
        m_problem->config()->setCoordinateType(coordinateTypeFromStringKey(QString::fromStdString(coordinateType)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(coordinateTypeStringKeys())).toStdString());
}

void SwigProblem::setMeshType(const std::string &meshType)
{
    if (meshTypeStringKeys().contains(QString::fromStdString(meshType)))
        m_problem->config()->setMeshType(meshTypeFromStringKey(QString::fromStdString(meshType)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(meshTypeStringKeys())).toStdString());
}

void SwigProblem::setFrequency(double frequency)
{
    if (frequency > 0.0)
        m_problem->config()->setValue(ProblemConfig::Frequency, Value(m_problem.data(), frequency));
    else
        throw out_of_range(QObject::tr("The frequency must be positive.").toStdString());
}

void SwigProblem::setTimeStepMethod(const std::string &timeStepMethod)
{
    if (timeStepMethodStringKeys().contains(QString::fromStdString(timeStepMethod)))
        m_problem->config()->setValue(ProblemConfig::TimeMethod, (dealii::TimeStepping::runge_kutta_method) timeStepMethodFromStringKey(QString::fromStdString(timeStepMethod)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(timeStepMethodStringKeys())).toStdString());
}

void SwigProblem::setTimeMethodTolerance(double timeMethodTolerance)
{
    if (timeMethodTolerance > 0.0)
        m_problem->config()->setValue(ProblemConfig::TimeMethodTolerance, timeMethodTolerance);
    else
        throw out_of_range(QObject::tr("The time method tolerance must be positive.").toStdString());
}

void SwigProblem::setTimeMethodOrder(int timeMethodOrder)
{
    if (timeMethodOrder >= 1 && timeMethodOrder <= 3)
        m_problem->config()->setValue(ProblemConfig::TimeOrder, timeMethodOrder);
    else
        throw out_of_range(QObject::tr("Number of time method order must be greater than 1.").toStdString());
}

void SwigProblem::setInitialTimeStep(double initialTimeStep)
{
    if (initialTimeStep > 0.0)
        m_problem->config()->setValue(ProblemConfig::TimeInitialStepSize, initialTimeStep);
    else
        throw out_of_range(QObject::tr("Initial time step must be positive.").toStdString());
}

void SwigProblem::setNumConstantTimeSteps(int timeSteps)
{
    if (timeSteps >= 1)
        m_problem->config()->setValue(ProblemConfig::TimeConstantTimeSteps, timeSteps);
    else
        throw out_of_range(QObject::tr("Number of time steps must be greater than 1.").toStdString());
}

void SwigProblem::setTimeTotal(double timeTotal)
{
    if (timeTotal >= 0.0)
        m_problem->config()->setValue(ProblemConfig::TimeTotal, timeTotal);
    else
        throw out_of_range(QObject::tr("The total time must be positive.").toStdString());
}

void SwigProblem::load(const std::string &fn)
{
    QString fileName = QString::fromStdString(fn);
    if (QFile::exists(fileName))
        Agros::problem()->readProblemFromArchive(fileName);
    else
        throw logic_error(QObject::tr("File '%1' does not exists.").arg(fileName).toStdString());
}

void SwigProblem::save(const std::string &fn)
{
    QString fileName = QString::fromStdString(fn);
    Agros::problem()->writeProblemToArchive(fileName, false);
}

std::string SwigProblem::typeOfStudyAtIndex(int index)
{
    if (qSharedPointerDynamicCast<Problem>(m_problem)->studies()->items().count() == 0)
        throw out_of_range(QObject::tr("No valid studies.").toStdString());

    if (index < 0 || index >= qSharedPointerDynamicCast<Problem>(m_problem)->studies()->items().count())
        throw out_of_range(QObject::tr("Out of range. Valid indices (0-%1).").arg(qSharedPointerDynamicCast<Problem>(m_problem)->studies()->items().count() - 1).toStdString());

    return studyTypeToStringKey(qSharedPointerDynamicCast<Problem>(m_problem)->studies()->items().at(index)->type()).toStdString();
}

// ********************************************************************************************

SwigComputation::SwigComputation(bool newComputation) : SwigProblemBase()
{    
    m_problem = Agros::problem()->createComputation(newComputation);
}

SwigComputation::SwigComputation(const string &computation) : SwigProblemBase()
{
    QMap<QString, QSharedPointer<Computation> > computations = Agros::computations();
    QString key = QString::fromStdString(computation);
    if (computations.contains(key))
        m_problem = computations[key];
    else
        throw logic_error(QObject::tr("Computation '%1' does not exists.").arg(key).toStdString());
}

SwigComputation::~SwigComputation()
{
    // qDebug() << "PyComputation::~PyComputation() - m_problemBase " << m_problemBase.isNull();
    // qDebug() << "PyComputation::~PyComputation() - m_problem " << m_problem.isNull();
}

void SwigComputation::clear()
{
    if (!computation()->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    computation()->clearSolution();
}

void SwigComputation::solve()
{
    computation()->scene()->loopsInfo()->processPolygonTriangles(true);
    computation()->solve();

    if (!computation()->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());
}

double SwigComputation::timeElapsed() const
{
    if (!computation()->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    double time = computation()->timeElapsed().hour()*3600 + computation()->timeElapsed().minute()*60 +
            computation()->timeElapsed().second() + computation()->timeElapsed().msec() * 1e-3;
    return time;
}

void SwigComputation::timeStepsLength(vector<double> &steps) const
{
    if (!computation()->isTransient())
        throw logic_error(QObject::tr("Problem is not transient.").toStdString());

    if (!computation()->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    QList<double> timeStepLengths = computation()->timeStepLengths();
    for (int i = 0; i < timeStepLengths.size(); i++)
        steps.push_back(timeStepLengths.at(i));
}

void SwigComputation::timeStepsTimes(vector<double> &times) const
{
    if (!computation()->isTransient())
        throw logic_error(QObject::tr("Problem is not transient.").toStdString());

    if (!computation()->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    QList<double> timeStepTimes = computation()->timeStepTimes();
    for (int i = 0; i < timeStepTimes.size(); i++)
        times.push_back(timeStepTimes.at(i));
}

void SwigComputation::getResults(std::vector<std::string> &keys) const
{
    StringToDoubleMap results = computation()->results()->items();

    foreach (QString key, results.keys())
        keys.push_back(key.toStdString());
}

double SwigComputation::getResult(const std::string &key) const
{
    StringToDoubleMap results = computation()->results()->items();

    if (results.contains(QString::fromStdString(key)))
    {
        return results[QString::fromStdString(key)];
    }
    else
    {
        QString str;
        foreach (QString key, results.keys())
            str += key + ", ";
        if (str.length() > 0)
            str = str.left(str.length() - 2);

        throw logic_error(QObject::tr("Invalid argument. Valid keys: %1").arg(str).toStdString());
    }
}

void SwigComputation::setResult(const string &key, double value)
{
    StringToDoubleMap results = computation()->results()->items();
    results[QString::fromStdString(key)] = value;
}

SwigSolution::~SwigSolution()
{
    // qDebug() << "SwigSolution::~SwigSolution() - m_computation " << m_computation.isNull();
}

void SwigSolution::setComputation(SwigComputation *computation, const std::string &fieldId)
{
    m_computation = computation->computation();

    QString id = QString::fromStdString(fieldId);
    if (m_computation->hasField(id))
        m_fieldInfo = m_computation->fieldInfo(id);
    else
        throw invalid_argument(QObject::tr("Invalid field id. Field %1 is not defined.").arg(id).toStdString());
}

int SwigSolution::getTimeStep(int timeStep) const
{
    if (timeStep == -1)
        timeStep = m_computation->solutionStore()->lastTimeStep(m_fieldInfo);
    else if (timeStep < 0 || timeStep > m_computation->timeStepLengths().length() - 1)
        throw out_of_range(QObject::tr("Time step must be in the range from 0 to %1.").arg(m_computation->timeStepLengths().length() - 1).toStdString());


    return timeStep;
}

int SwigSolution::getAdaptivityStep(int adaptivityStep, int timeStep) const
{
    if (adaptivityStep == -1)
        adaptivityStep = m_computation->solutionStore()->lastAdaptiveStep(m_fieldInfo, timeStep);
    else if (adaptivityStep < 0 || adaptivityStep > m_fieldInfo->value(FieldInfo::AdaptivitySteps).toInt() - 1)
        throw out_of_range(QObject::tr("Adaptivity step is out of range. (0 to %1).").arg(m_fieldInfo->value(FieldInfo::AdaptivitySteps).toInt() - 1).toStdString());

    return adaptivityStep;
}

void SwigSolution::localValues(double x, double y, int timeStep, int adaptivityStep, map<std::string, double> &results) const
{
    map<std::string, double> values;

    if (m_computation->isSolved() || m_computation->isSolving())
    {
        Point point(x, y);

        // set time and adaptivity step if -1 (default parameter - last steps), check steps
        timeStep = getTimeStep(timeStep);
        adaptivityStep = getAdaptivityStep(adaptivityStep, timeStep);

        std::shared_ptr<LocalValue> value = m_fieldInfo->plugin()->localValue(m_computation.data(), m_fieldInfo, timeStep, adaptivityStep, point);
        QMapIterator<QString, LocalPointValue> it(value->values());
        while (it.hasNext())
        {
            it.next();
            Module::LocalVariable variable = m_fieldInfo->localVariable(m_computation->config()->coordinateType(), it.key());

            if (variable.isScalar())
            {
                values[variable.shortname().toStdString()] = it.value().scalar;
            }
            else
            {
                values[variable.shortname().toStdString()] = it.value().vector.magnitude();
                values[variable.shortname().toStdString() + m_computation->config()->labelX().toLower().toStdString()] = it.value().vector.x;
                values[variable.shortname().toStdString() + m_computation->config()->labelY().toLower().toStdString()] = it.value().vector.y;
            }
        }
    }
    else
    {
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());
    }

    results = values;
}

void SwigSolution::surfaceIntegrals(const vector<int> &edges, int timeStep, int adaptivityStep,
                                  map<std::string, double> &results) const
{
    map<std::string, double> values;

    if (m_computation->isSolved() || m_computation->isSolving())
    {
        m_computation->scene()->selectNone();

        if (!edges.empty())
        {
            for (vector<int>::const_iterator it = edges.begin(); it != edges.end(); ++it)
            {
                if ((*it >= 0) && (*it < m_computation->scene()->faces->length()))
                {
                    m_computation->scene()->faces->at(*it)->setSelected(true);
                }
                else
                {
                    throw out_of_range(QObject::tr("Edge index must be between 0 and '%1'.").arg(m_computation->scene()->faces->length()-1).toStdString());
                    results = values;
                    return;
                }
            }
        }
        else
        {
            m_computation->scene()->selectAll(SceneGeometryMode_OperateOnEdges);
        }

        // set time and adaptivity step if -1 (default parameter - last steps), check steps
        timeStep = getTimeStep(timeStep);
        adaptivityStep = getAdaptivityStep(adaptivityStep, timeStep);

        std::shared_ptr<IntegralValue> integral = m_fieldInfo->plugin()->surfaceIntegral(m_computation.data(), m_fieldInfo, timeStep, adaptivityStep);
        QMapIterator<QString, double> it(integral->values());
        while (it.hasNext())
        {
            it.next();
            Module::Integral integral = m_fieldInfo->surfaceIntegral(m_computation->config()->coordinateType(), it.key());
            values[integral.shortname().toStdString()] = it.value();
        }
    }
    else
    {
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());
    }

    results = values;
}

void SwigSolution::volumeIntegrals(const vector<int> &labels, int timeStep, int adaptivityStep,
                                 map<std::string, double> &results) const
{
    map<std::string, double> values;
    if (m_computation->isSolved() || m_computation->isSolving())
    {
        m_computation->scene()->selectNone();

        if (!labels.empty())
        {
            for (vector<int>::const_iterator it = labels.begin(); it != labels.end(); ++it)
            {
                if ((*it >= 0) && (*it < m_computation->scene()->labels->length()))
                {
                    if (m_computation->scene()->labels->at(*it)->marker(m_fieldInfo) != m_computation->scene()->materials->getNone(m_fieldInfo))
                        m_computation->scene()->labels->at(*it)->setSelected(true);
                    else
                        throw out_of_range(QObject::tr("Label with index '%1' is 'none'.").arg(*it).toStdString());
                }
                else
                {
                    throw out_of_range(QObject::tr("Label index must be between 0 and '%1'.").arg(m_computation->scene()->labels->length()-1).toStdString());
                    results = values;
                    return;
                }
            }
        }
        else
        {
            m_computation->scene()->selectAll(SceneGeometryMode_OperateOnLabels);
        }

        // set time and adaptivity step if -1 (default parameter - last steps), check steps
        timeStep = getTimeStep(timeStep);
        adaptivityStep = getAdaptivityStep(adaptivityStep, timeStep);

        std::shared_ptr<IntegralValue> integral = m_fieldInfo->plugin()->volumeIntegral(m_computation.data(), m_fieldInfo, timeStep, adaptivityStep);
        QMapIterator<QString, double> it(integral->values());
        while (it.hasNext())
        {
            it.next();
            Module::Integral integral = m_fieldInfo->volumeIntegral(m_computation->config()->coordinateType(), it.key());
            values[integral.shortname().toStdString()] = it.value();
        }
    }
    else
    {
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());
    }

    results = values;
}

void SwigSolution::initialMeshInfo(map<std::string, int> &info) const
{
    if (!m_computation->isMeshed())
        throw logic_error(QObject::tr("Problem is not meshed.").toStdString());

    // todo: initial mesh the same for all fields
    info["nodes"] = m_computation->initialMesh().n_used_vertices();
    info["elements"] = m_computation->initialMesh().n_active_cells();
}

void SwigSolution::solutionMeshInfo(int timeStep, int adaptivityStep, map<std::string, int> &info) const
{
    if (!m_computation->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    // set time and adaptivity step if -1 (default parameter - last steps), check steps
    timeStep = getTimeStep(timeStep);
    adaptivityStep = getAdaptivityStep(adaptivityStep, timeStep);

    // TODO: (Franta) time and adaptivity step in gui vs. implementation
    MultiArray ma = m_computation->solutionStore()->multiArray(FieldSolutionID(m_fieldInfo->fieldId(), timeStep, adaptivityStep));

    info["nodes"] = ma.doFHandler().get_triangulation().n_used_vertices();
    info["elements"] = ma.doFHandler().get_triangulation().n_active_cells();
    info["dofs"] = ma.doFHandler().n_dofs();
}

void SwigSolution::solverInfo(int timeStep, int adaptivityStep,
                            vector<double> &solutionsChange, vector<double> &residual,
                            vector<double> &dampingCoeff, int &jacobianCalculations) const
{
    if (!m_computation->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    // step if -1 (default parameter - last steps)
    timeStep = getTimeStep(timeStep);
    adaptivityStep = getAdaptivityStep(adaptivityStep, timeStep);

    SolutionStore::SolutionRunTimeDetails runTime = m_computation->solutionStore()->multiSolutionRunTimeDetail(FieldSolutionID(m_fieldInfo->fieldId(), timeStep, adaptivityStep));

    /*
    for (int i = 0; i < runTime.relativeChangeOfSolutions().size(); i++)
        solutionsChange.push_back(runTime.relativeChangeOfSolutions().at(i));

    for (int i = 0; i < runTime.newtonResidual().size(); i++)
        residual.push_back(runTime.newtonResidual().at(i));

    for (int i = 0; i < runTime.nonlinearDamping().size(); i++)
        dampingCoeff.push_back(runTime.nonlinearDamping().at(i));
    jacobianCalculations = runTime.jacobianCalculations();
     */
}

void SwigSolution::adaptivityInfo(int timeStep, vector<double> &error, vector<int> &dofs) const
{
    if (!m_computation->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    if (m_fieldInfo->adaptivityType() == AdaptivityMethod_None)
        throw logic_error(QObject::tr("Solution is not adaptive.").toStdString());

    // set time step if -1 (default parameter - last steps)
    timeStep = getTimeStep(timeStep);

    int adaptivitySteps = m_computation->solutionStore()->lastAdaptiveStep(m_fieldInfo, timeStep) + 1;
    for (int i = 0; i < adaptivitySteps; i++)
    {
        SolutionStore::SolutionRunTimeDetails runTime = m_computation->solutionStore()->multiSolutionRunTimeDetail(FieldSolutionID(m_fieldInfo->fieldId(), timeStep, i));
        /*
        error.push_back(runTime.adaptivityError());
        dofs.push_back(runTime.DOFs());
        */
    }
}

void SwigSolution::solution(int timeStep, int adaptivityStep, vector<double> &sln) const
{
    timeStep = getTimeStep(timeStep);
    adaptivityStep = getAdaptivityStep(adaptivityStep, timeStep);

    FieldSolutionID fsid(m_fieldInfo->fieldId(), timeStep, adaptivityStep);
    if (m_computation->solutionStore()->contains(fsid))
    {
        MultiArray ma = m_computation->solutionStore()->multiArray(fsid);
        sln = std::vector<double>(ma.solution().size());
        for (unsigned int i = 0; i < sln.size(); i++)
            sln[i] = ma.solution()[i];

        return;
    }

    sln = std::vector<double>();
    throw logic_error(QObject::tr("Solution does not exist.").toStdString());
}

void SwigSolution::exportVTK(const std::string &fileName, int timeStep, int adaptivityStep, const std::string &variable, std::string physicFieldVariableComp)
{
    timeStep = getTimeStep(timeStep);
    adaptivityStep = getAdaptivityStep(adaptivityStep, timeStep);

    FieldSolutionID fsid(m_fieldInfo->fieldId(), timeStep, adaptivityStep);
    if (m_computation->solutionStore()->contains(fsid))
    {
        MultiArray ma = m_computation->solutionStore()->multiArray(fsid);

        dealii::DataPostprocessorScalar<2> *post = m_fieldInfo->plugin()->filter(m_computation.data(),
                                                                                                  m_fieldInfo,
                                                                                                  timeStep,
                                                                                                  adaptivityStep,
                                                                                                  QString::fromStdString(variable),
                                                                                                  physicFieldVariableCompFromStringKey(QString::fromStdString(physicFieldVariableComp)));

        std::shared_ptr<PostDataOut> data_out = std::shared_ptr<PostDataOut>(new PostDataOut(m_fieldInfo, m_computation.data()));
        data_out->attach_dof_handler(ma.doFHandler());
        data_out->add_data_vector(ma.solution(), *post);
        data_out->build_patches(2);

        // release post object
        delete post;

        std::ofstream output(fileName);
        data_out->write_vtk(output);
    }
}