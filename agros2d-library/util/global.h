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

#ifndef GLOBAL_H
#define GLOBAL_H

#include "util.h"
#include "util/enums.h"

class Problem;
class Computation;
class Config;
class SolutionStore;
class Log;
class PluginInterface;

/*
class AGROS_LIBRARY_API AgrosApplication : public QApplication
{
public:
    AgrosApplication(int& argc, char ** argv);
    ~AgrosApplication();

    virtual bool notify(QObject *receiver, QEvent *event);
};
*/

class AGROS_LIBRARY_API Agros2D : public QObject
{
    Q_OBJECT

public:
    Agros2D(const Agros2D &);
    Agros2D & operator = (const Agros2D &);
    Agros2D();

    static void createSingleton();
    static Agros2D* singleton();

    static inline Config *configComputer() { return Agros2D::singleton()->m_configComputer; }

    static inline Problem *problem() { return Agros2D::singleton()->m_problem; }

    static inline QMap<QString, QSharedPointer<Computation> > computations() { return Agros2D::singleton()->m_computations; }
    static void addComputation(const QString &problemDir, QSharedPointer<Computation> comp);
    static void clearComputations();

    static inline Log *log() { return Agros2D::singleton()->m_log; }

    static PluginInterface *loadPlugin(const QString &pluginName);

    static void clear();

private:    
    // computer config
    Config *m_configComputer;
    // problem
    Problem *m_problem;
    // computations
    QMap<QString, QSharedPointer<Computation> > m_computations;
    // log and memory monitor
    Log *m_log;
};

#endif /* GLOBAL_H */
