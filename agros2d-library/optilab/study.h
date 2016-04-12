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

#ifndef STUDY_H
#define STUDY_H

#include <QWidget>

#include "util/enums.h"
#include "logview.h"
#include "parameter.h"
#include "functional.h"
#include "solver/problem.h"
#include "solver/problem_result.h"

class Computation;
class Study;

class SolutionUncertainty
{
public:
    SolutionUncertainty() : lowerBound(0.0), upperBound(0.0), uncertainty(0.0) {}

    double lowerBound;
    double upperBound;
    double uncertainty;
};

class ComputationParameterCompare
{
public:
    ComputationParameterCompare(const QString &parameterName) : m_parameterName(parameterName) {}

    inline bool operator() (QSharedPointer<Computation> i, QSharedPointer<Computation> j)
    {
        return (i->results()->items()[m_parameterName] > j->results()->items()[m_parameterName]);
    }

protected:
    QString m_parameterName;
};

class ComputationSet
{
public:
    ComputationSet(QList<QSharedPointer<Computation> > set = QList<QSharedPointer<Computation> >(), const QString &name = "");
    virtual ~ComputationSet();

    virtual void load(QJsonObject &object);
    virtual void save(QJsonObject &object);

    inline QString name() { return m_name; }
    inline void setName(const QString &name) { m_name = name; }

    inline void addComputation(QSharedPointer<Computation> computation) { m_computations.append(computation); }
    inline void removeComputation(QSharedPointer<Computation> computation) { m_computations.removeAll(computation); }
    QList<QSharedPointer<Computation> > computations() const { return m_computations; }
    QList<QSharedPointer<Computation> > &computations() { return m_computations; }

    void sort(const QString &parameterName);

protected:
    QString m_name;
    QList<QSharedPointer<Computation> > m_computations;
};

class Study : public QObject
{
    Q_OBJECT

public:
    enum ResultType
    {
        ResultType_Parameter,
        ResultType_Functional,
        ResultType_Recipe
    };

    inline QString resultTypeToStringKey(ResultType type)
    {
        if (type == ResultType::ResultType_Parameter) return "parameter";
        else if (type == ResultType::ResultType_Functional) return "functional";
        else if (type == ResultType::ResultType_Recipe) return "recipe";
        else assert(0);
    }

    inline ResultType resultTypeFromStringKey(const QString &type)
    {
        if (type == "parameter") return ResultType::ResultType_Parameter;
        else if (type == "functional") return ResultType::ResultType_Functional;
        else if (type == "recipe") return ResultType::ResultType_Recipe;
        else assert(0);
    }

    enum Type
    {
        General_ClearSolution,
        General_SolveProblem,

        NLopt_xtol_rel,
        NLopt_xtol_abs,
        NLopt_ftol_rel,
        NLopt_ftol_abs,
        NLopt_n_iterations,
        NLopt_algorithm,

        Sweep_num_samples,
        Sweep_init_method,

        BayesOpt_n_init_samples,
        BayesOpt_n_iterations,
        BayesOpt_n_iter_relearn,
        BayesOpt_init_method,
        BayesOpt_surr_name,
        BayesOpt_surr_noise,
        BayesOpt_l_type,
        BayesOpt_sc_type,

        NSGA2_popsize,
        NSGA2_ngen,
        NSGA2_pcross,
        NSGA2_pmut,
        NSGA2_eta_c,
        NSGA2_eta_m,
        NSGA2_crowdobj,

        NSGA3_popsize,
        NSGA3_ngen,
        NSGA3_pcross,
        NSGA3_pmut,
        NSGA3_eta_c,
        NSGA3_eta_m,
        NSGA3_use_surrogate,

        View_ChartHorizontal,
        View_ChartVertical,
        View_ChartLogHorizontal,
        View_ChartLogVertical,
        View_ChartShowTrend,
        View_ChartShowAverageValue
    };

    Study(QList<ComputationSet> computations = QList<ComputationSet>());
    virtual ~Study();

    virtual StudyType type() = 0;

    void clear();
    virtual void solve() = 0;

    virtual void load(QJsonObject &object);
    virtual void save(QJsonObject &object);

    void addParameter(Parameter parameter) { m_parameters.append(parameter); }
    void removeParameter(const QString &name);
    Parameter &parameter(const QString &name);
    QList<Parameter> &parameters() { return m_parameters; }

    void addFunctional(Functional functional) { m_functionals.append(functional); }
    void removeFunctional(const QString &name);
    Functional &functional(const QString &name);
    QList<Functional> &functionals() { return m_functionals; }
    bool evaluateFunctionals(QSharedPointer<Computation> computation);
    void evaluateStep(QSharedPointer<Computation> computation, SolutionUncertainty solutionUncertainty = SolutionUncertainty());
    double evaluateSingleGoal(QSharedPointer<Computation> computation);
    QList<double> evaluateMultiGoal(QSharedPointer<Computation> computation);

    QList<QSharedPointer<Computation> > &computations(int index = -1);
    QList<ComputationSet> computationSets() const { return m_computationSets; }
    int computationsCount() const;
    QList<ComputationSet> &computationSets() { return m_computationSets; }
    void addComputationSet(const QString &name = "") { m_computationSets.append(ComputationSet(QList<QSharedPointer<Computation> >(), name.isEmpty() ? tr("Set %1").arg(m_computationSets.count() + 1) : name)); }
    void addComputation(QSharedPointer<Computation> computation, bool newComputationSet = false);

    virtual int estimatedNumberOfSteps() const { return 0; }

    QVariant variant();

    bool isSolving() const { return m_isSolving; }
    bool isAborted() const { return m_abort; }

    // postprocessor
    QSharedPointer<Computation> findExtreme(ResultType type, const QString &key, bool minimum);

    // config
    QMap<Type, QString> keys() const { return m_settingKey; }
    inline QVariant value(Type type) const { return m_setting[type]; }
    inline void setValue(Type type, int value) {  m_setting[type] = value; }
    inline void setValue(Type type, double value) {  m_setting[type] = value;}
    inline void setValue(Type type, bool value) {  m_setting[type] = value;}
    inline void setValue(Type type, const std::string &value) { setValue(type, QString::fromStdString(value)); }
    inline void setValue(Type type, const QString &value) { m_setting[type] = value;}

    static Study *factory(StudyType type);

public slots:
    void doAbortSolve();

signals:
    void updateChart(QList<double> values, double totalValue, SolutionUncertainty solutionUncertainty);
    void updateParameters(QList<Parameter> parameters, const Computation *computation);

    void solved();

protected:
    QList<Parameter> m_parameters;
    QList<Functional> m_functionals;
    QList<ComputationSet> m_computationSets;

    bool m_isSolving;
    bool m_abort;

    inline QVariant defaultValue(Type type) {  return m_settingDefault[type]; }

    QMap<Type, QVariant> m_setting;
    QMap<Type, QVariant> m_settingDefault;
    QMap<Type, QString> m_settingKey;

    inline QString typeToStringKey(Type type) { return m_settingKey[type]; }
    inline Type stringKeyToType(const QString &key) { return m_settingKey.key(key); }
    inline QStringList stringKeys() { return m_settingKey.values(); }

    virtual void setDefaultValues();
    virtual void setStringKeys();

    friend class PyStudy;
    friend class PyStudyBayesOpt;
};

class Studies : public QObject
{
    Q_OBJECT

public:
    Studies(QObject *parent = 0);
    ~Studies() {}

    void clear();

    inline QList<Study *> &items() { return m_studies; }
    void addStudy(Study *study);
    void removeStudy(Study *study);

    void removeComputation(QSharedPointer<Computation> computation);

    Study * operator[] (int idx) { return m_studies[idx]; }
    const Study * operator[] (int idx) const { return m_studies[idx]; }

signals:
    void invalidated();

private:
    QList<Study *> m_studies;
};

#endif // STUDY_H
