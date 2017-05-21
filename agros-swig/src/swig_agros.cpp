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

#include "swig_agros.h"

#include "logview.h"
#include "solver/problem.h"
#include "optilab/study.h"
#include "solver/plugin_interface.h"
#include "solver/module.h"
#include "solver/problem_result.h"
#include "scene.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"

#ifdef TBB_FOUND
#include <tbb/tbb.h>
tbb::mutex runPythonHeaderMutex;
#endif

void openFile(const std::string &file, bool openWithSolution)
{
    try
    {
        Agros::problem()->readProblemFromArchive(QString::fromStdString(file));

        // if (openWithSolution)
        //    Agros::computation()->readSolutionFromFile(QString::fromStdString(file));
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
        Agros::problem()->writeProblemToArchive(QString::fromStdString(file), !saveWithSolution);

        // if (saveWithSolution || silentMode())
        //    Agros::computation()->writeSolutionToFile(QString::fromStdString(file));
    }
    catch (AgrosException &e)
    {
        throw logic_error(e.toString().toStdString());
    }
}

void swigInitSingleton()
{
    initSingleton();
}


std::string swigVersion()
{
    return QCoreApplication::applicationVersion().toStdString();
}

std::string swigDatadir(std::string str)
{
    QString path = QFileInfo(datadir() + "/" + QString::fromStdString(str)).absoluteFilePath();
    return compatibleFilename(path).toStdString();
}

// ************************************************************************************

void SwigOptions::setCacheSize(int size)
{
    if (size < 2 || size > 50)
        throw out_of_range(QObject::tr("Cache size is out of range (2 - 50).").toStdString());

    Agros::configComputer()->setValue(Config::Config_CacheSize, size);
}

void SwigOptions::setDumpFormat(std::string format)
{
    if (dumpFormatStringKeys().contains(QString::fromStdString(format)))
        Agros::configComputer()->setValue(Config::Config_LinearSystemFormat, (MatrixExportFormat) dumpFormatFromStringKey(QString::fromStdString(format)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(dumpFormatStringKeys())).toStdString());
}
