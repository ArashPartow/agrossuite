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
#include "parameter.h"
#include "functional.h"
#include "solver/problem.h"
#include "solver/problem_result.h"

class Computation;


class ComputationParameterCompare
{
public:
    ComputationParameterCompare(const QString &parameterName) : m_parameterName(parameterName) {}

    inline bool operator() (QSharedPointer<Computation> i, QSharedPointer<Computation> j)
    {
        return (i->results()->results()[m_parameterName] > j->results()->results()[m_parameterName]);
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
    QList<Parameter> &parameters() { return m_parameters; }

    void addFunctional(Functional functional) { m_functionals.append(functional); }
    QList<Functional> &functionals() { return m_functionals; }
    bool evaluateFunctionals(QSharedPointer<Computation> computation);

    QList<QSharedPointer<Computation> > &computations(int index = -1);
    void addComputationSet(const QString &name = "") { m_computationSets.append(ComputationSet(QList<QSharedPointer<Computation> >(), name)); }
    void addComputation(QSharedPointer<Computation> computation, bool newComputationSet = false);

    virtual void fillTreeView(QTreeWidget *trvComputations);
    QVariant variant();

protected:
    QString m_name;

    QList<Parameter> m_parameters;
    QList<Functional> m_functionals;
    QList<ComputationSet> m_computationSets;
};

class Studies : public QObject
{
    Q_OBJECT

public:
    Studies(QObject *parent = 0);
    ~Studies() {}

    void clear();

    bool load(const QString &fileName);
    bool save(const QString &fileName);

    inline QList<Study *> &studies() { return m_studies; }
    void addStudy(Study *study);
    void removeStudy(Study *study);

    Study * operator[] (int idx) { return m_studies[idx]; }
    const Study * operator[] (int idx) const { return m_studies[idx]; }

signals:
    void invalidated();

private:
    QList<Study *> m_studies;
};

#endif // STUDY_H
