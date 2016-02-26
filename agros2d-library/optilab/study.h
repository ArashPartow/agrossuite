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
class QCustomPlot;
class QCPGraph;

class AGROS_LIBRARY_API LogOptimizationDialog : public QDialog
{
    Q_OBJECT
public:
    LogOptimizationDialog(Study *study);
    ~LogOptimizationDialog();

protected:
    virtual void closeEvent(QCloseEvent *e);
    virtual void reject();

private:
    Study *m_study;

    LogWidget *m_logWidget;

    QPushButton *btnClose;
    QPushButton *btnAbort;

    QCustomPlot *m_chart;
    QProgressBar *m_progress;
    QCPGraph *m_objectiveGraph;

    void createControls();

private slots:
    void updateChart();
    void updateParameters(QList<Parameter> parameters, const Computation *computation);

    void solved();
    void aborted();

    void tryClose();
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
    enum Type
    {
        NLopt_tol_rel,
        NLopt_tol_abs,
        NLopt_n_iterations,
        NLopt_algorithm,

        BayesOpt_n_init_samples,
        BayesOpt_n_iterations,
        BayesOpt_n_iter_relearn,
        BayesOpt_init_method
    };

    Study(QList<ComputationSet> computations = QList<ComputationSet>());
    virtual ~Study();

    virtual StudyType type() = 0;

    void clear();
    virtual void solve() = 0;

    virtual void load(QJsonObject &object);
    virtual void save(QJsonObject &object);

    inline QString name() { return m_name; }
    inline void setName(const QString &name) { m_name = name; }

    void addParameter(Parameter parameter) { m_parameters.append(parameter); }
    void removeParameter(const QString &name);
    QList<Parameter> &parameters() { return m_parameters; }

    void addFunctional(Functional functional) { m_functionals.append(functional); }
    void removeFunctional(const QString &name);
    QList<Functional> &functionals() { return m_functionals; }
    bool evaluateFunctionals(QSharedPointer<Computation> computation);

    QList<QSharedPointer<Computation> > &computations(int index = -1);
    QList<ComputationSet> &computationSets() { return m_computationSets; }
    void addComputationSet(const QString &name = "") { m_computationSets.append(ComputationSet(QList<QSharedPointer<Computation> >(), name)); }
    void addComputation(QSharedPointer<Computation> computation, bool newComputationSet = false);

    virtual void fillTreeView(QTreeWidget *trvComputations);
    QVariant variant();

    bool isSolving() const { return m_isSolving; }
    bool isAborted() const { return m_abort; }

    // config
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
    void updateChart();
    void updateParameters(QList<Parameter> parameters, const Computation *computation);

    void solved();

protected:
    QString m_name;

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

    virtual void setDefaultValues() {}
    virtual void setStringKeys() {}
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

class StudyDialog : public QDialog
{
    Q_OBJECT

public:
    StudyDialog(Study *study, QWidget *parent = 0);

    int showDialog();

    static StudyDialog *factory(Study *study, QWidget *parent = 0);

protected:
    Study *m_study;

    void createControls();
    virtual QWidget *createStudyControls() { return new QWidget(this); }
    QWidget *createParameters();
    QWidget *createFunctionals();

    QTreeWidget *trvParameterWidget;
    QPushButton *btnParameterAdd;
    QPushButton *btnParameterEdit;
    QPushButton *btnParameterRemove;
    void readParameters();

    QTreeWidget *trvFunctionalWidget;
    QPushButton *btnFunctionalAdd;
    QPushButton *btnFunctionalEdit;
    QPushButton *btnFunctionalRemove;
    void readFunctionals();

    virtual void load() = 0;
    virtual void save() = 0;

private slots:
    void doAccept();

    void doParameterItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void doParameterItemDoubleClicked(QTreeWidgetItem *item, int role);
    void doParameterRemove(bool checked);

    void doFunctionalItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void doFunctionalItemDoubleClicked(QTreeWidgetItem *item, int role);
    void doFunctionalRemove(bool checked);
};

#endif // STUDY_H
