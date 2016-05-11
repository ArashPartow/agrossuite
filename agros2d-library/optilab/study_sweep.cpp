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

#include "study_sweep.h"

#include "study.h"
#include "parameter.h"

#include "util/global.h"
#include "solver/problem.h"
#include "solver/problem_result.h"
#include "solver/solutionstore.h"
#include "solver/plugin_interface.h"

#include "scene.h"

const QString N_INIT_SAMPLES = "n_init_samples";
const QString N_ITERATIONS = "n_iterations";
const QString N_ITER_RELEARN = "n_iter_relearn";
const QString INIT_METHOD = "init_method";

SweepProblem::SweepProblem(StudySweep *study, bayesopt::Parameters par)
    : ContinuousModel(study->parameters().count(), par), m_study(study)
{
    vectord lowerBound(m_study->parameters().count());
    vectord upperBound(m_study->parameters().count());

    // set bounding box
    for (int i = 0; i < m_study->parameters().count(); i++)
    {
        Parameter parameter = m_study->parameters()[i];

        lowerBound.insert_element(i, parameter.lowerBound());
        upperBound.insert_element(i, parameter.upperBound());
    }

    setBoundingBox(lowerBound, upperBound);
}

double SweepProblem::evaluateSample(const vectord& x)
{
    // computation
    QSharedPointer<Computation> computation = Agros2D::problem()->createComputation(true);

    // set parameters
    for (int i = 0; i < m_study->parameters().count(); i++)
    {
        Parameter parameter = m_study->parameters()[i];
        computation->config()->parameters()->set(parameter.name(), x[i]);
    }

    // evaluate step
    try
    {
        m_study->evaluateStep(computation);
        double value = m_study->evaluateSingleGoal(computation);

        // penalty
        double totalPenalty = 0.0;
        for (int i = 0; i < m_study->parameters().count(); i++)
        {
            Parameter parameter = m_study->parameters()[i];
            if (parameter.penaltyEnabled())
                totalPenalty += parameter.penalty(x[i]);
        }

        if (m_study->value(Study::General_ClearSolution).toBool())
            computation->clearSolution();

        // add computation
        m_study->addComputation(computation);

        return value + totalPenalty;
    }
    catch (AgrosSolverException &e)
    {
        qDebug() << e.toString();

        return numeric_limits<double>::max();
    }
}

StudySweep::StudySweep() : Study()
{
    // init method
    initMethodList.insert(1, "lhs");
    initMethodList.insert(2, "sobol");
}

int StudySweep::estimatedNumberOfSteps() const
{
    return value(Study::Sweep_num_samples).toInt();
}

QString StudySweep::initMethodString(int method) const
{
    if (method == 1)
        return QObject::tr("Latin Hypercube Sampling (LHS)");
    else if (method == 2)
        return QObject::tr("Sobol Sequences");
    else
    {
        std::cerr << "init method '" + QString::number(method).toStdString() + "' is not implemented. initMethodString(int method)" << endl;
        throw;
    }
}

void StudySweep::solve()
{
    m_computationSets.clear();
    m_isSolving = true;

    // parameters
    bayesopt::Parameters par = initialize_parameters_to_default();
    par.n_init_samples = value(Sweep_num_samples).toInt();
    par.n_iterations = 1;
    par.n_iter_relearn = 10000;
    par.init_method = initMethodFromStringKey(value(Sweep_init_method).toString());
    par.noise = 1e-10;
    par.random_seed = 0;
    par.verbose_level = 1;

    SweepProblem SweepProblem(this, par);

    // init BayesOpt problem
    addComputationSet(tr("Sweep"));
    SweepProblem.initializeOptimization();

    m_isSolving = false;

    emit solved();
}

void StudySweep::setDefaultValues()
{
    Study::setDefaultValues();

    m_settingDefault[Sweep_num_samples] = 20;
    m_settingDefault[Sweep_init_method] = initMethodToStringKey(1); // 1-LHS, 2-Sobol
}

void StudySweep::setStringKeys()
{
    Study::setStringKeys();

    m_settingKey[Sweep_num_samples] = "Sweep_num_samples";
    m_settingKey[Sweep_init_method] = "Sweep_init_method";
}

// *****************************************************************************************************

StudySweepDialog::StudySweepDialog(Study *study, QWidget *parent)
    : StudyDialog(study, parent)
{

}

QLayout *StudySweepDialog::createStudyControls()
{
    txtNumSamples = new QSpinBox(this);
    txtNumSamples->setMinimum(1);
    txtNumSamples->setMaximum(10000);

    cmbInitMethod = new QComboBox(this);
    foreach (QString key, study()->initMethodStringKeys())
        cmbInitMethod->addItem(study()->initMethodString(study()->initMethodFromStringKey(key)), key);

    QGridLayout *layoutInitialization = new QGridLayout(this);
    layoutInitialization->addWidget(new QLabel(tr("Number of samples:")), 0, 0);
    layoutInitialization->addWidget(txtNumSamples, 0, 1);
    layoutInitialization->addWidget(new QLabel(tr("Initial strategy:")), 1, 0);
    layoutInitialization->addWidget(cmbInitMethod, 1, 1);

    QGroupBox *grpInitialization = new QGroupBox(tr("Sweep analysis"), this);
    grpInitialization->setLayout(layoutInitialization);

    QVBoxLayout *layoutMain = new QVBoxLayout(this);
    layoutMain->addWidget(grpInitialization);

    return layoutMain;
}

void StudySweepDialog::load()
{
    StudyDialog::load();

    txtNumSamples->setValue(study()->value(Study::Sweep_num_samples).toInt());
    cmbInitMethod->setCurrentIndex(cmbInitMethod->findData(study()->value(Study::Sweep_init_method).toString()));
}

void StudySweepDialog::save()
{
    StudyDialog::save();

    study()->setValue(Study::Sweep_num_samples, txtNumSamples->value());
    study()->setValue(Study::Sweep_init_method, cmbInitMethod->itemData(cmbInitMethod->currentIndex()).toString());
}
