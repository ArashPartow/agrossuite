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

#include "pyview.h"

#include "sceneview_geometry.h"
#include "sceneview_mesh.h"
#include "sceneview_post2d.h"
#include "sceneview_post3d.h"

#include "solver/module.h"
#include "solver/solutionstore.h"

#include "util/constants.h"
#include "util/global.h"

SceneViewCommon* PyView::currentSceneViewMode()
{
    /*
    if (currentPythonEngineAgros()->sceneViewMesh()->actSceneModeMesh->isChecked())
        return currentPythonEngineAgros()->sceneViewMesh();
    else if (currentPythonEngineAgros()->sceneViewPost2D()->actSceneModePost2D->isChecked())
        return currentPythonEngineAgros()->sceneViewPost2D();
    else if (currentPythonEngineAgros()->sceneViewPost3D()->actSceneModePost3D->isChecked())
        return currentPythonEngineAgros()->sceneViewPost3D();
    else if (currentPythonEngineAgros()->sceneViewParticleTracing()->actSceneModeParticleTracing->isChecked())
        return currentPythonEngineAgros()->sceneViewParticleTracing();
    */

    assert(0);
    return nullptr;
}

void PyView::saveImageToFile(const std::string &file, int width, int height)
{
    if (!silentMode())
        currentSceneViewMode()->saveImageToFile(QString::fromStdString(file), width, height);
}

void PyView::zoomBestFit()
{
    if (!silentMode())
        currentSceneViewMode()->doZoomBestFit();
}

void PyView::zoomIn()
{
    if (!silentMode())
        currentSceneViewMode()->doZoomIn();
}

void PyView::zoomOut()
{
    if (!silentMode())
        currentSceneViewMode()->doZoomOut();
}

void PyView::zoomRegion(double x1, double y1, double x2, double y2)
{
    if (!silentMode())
        currentSceneViewMode()->doZoomRegion(Point(x1, y1), Point(x2, y2));
}

// ************************************************************************************

void PyViewConfig::setFontFamily(Config::Type type, const std::string &family)
{
    if (silentMode())
        return;

    QStringList filter;
    filter << "*.ttf";
    QStringList list = QDir(datadir() + "/resources/fonts").entryList(filter);

    foreach (QString fileName, list)
        list.replaceInStrings(fileName, QFileInfo(fileName).baseName());

    foreach (QString fileName, list)
    {
        if (fileName.toStdString() == family)
        {
            Agros2D::configComputer()->setValue(type, QString::fromStdString(family));
            return;
        }

    }

    throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(list)).toStdString());
}

// ************************************************************************************

void PyViewMeshAndPost::setActiveTimeStep(int timeStep)
{
    if (!Agros2D::computation()->isSolved())
        throw invalid_argument(QObject::tr("Problem is not solved.").toStdString());

    if (timeStep < 0 || timeStep > Agros2D::computation()->timeLastStep())
        throw out_of_range(QObject::tr("Time step must be in the range from 0 to %1.").arg(Agros2D::computation()->timeLastStep()).toStdString());

    if (silentMode())
        return;

    FieldInfo *fieldInfo = Agros2D::computation()->postDeal()->activeViewField();
    int adaptivityStep = Agros2D::computation()->solutionStore()->lastAdaptiveStep(fieldInfo, timeStep);

    Agros2D::computation()->postDeal()->setActiveTimeStep(timeStep);
    Agros2D::computation()->postDeal()->setActiveAdaptivityStep(adaptivityStep);
    Agros2D::computation()->postDeal()->refresh();
}

void PyViewMeshAndPost::setActiveAdaptivityStep(int adaptivityStep)
{
    if (!Agros2D::computation()->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    int last_step = Agros2D::computation()->solutionStore()->lastAdaptiveStep(Agros2D::computation()->postDeal()->activeViewField(),
                                                               Agros2D::computation()->postDeal()->activeTimeStep());

    if (adaptivityStep < 1 || adaptivityStep > last_step + 1)
        throw out_of_range(QObject::tr("Adaptivity step for active field (%1) must be in the range from 0 to %2.")
                           .arg(Agros2D::computation()->postDeal()->activeViewField()->fieldId())
                           .arg(last_step + 1).toStdString());

    if (!silentMode())
    {
        Agros2D::computation()->postDeal()->setActiveAdaptivityStep(adaptivityStep - 1);
        Agros2D::computation()->postDeal()->refresh();
    }
}

// ************************************************************************************

void PyViewMesh::checkExistingMesh()
{
    if (!Agros2D::computation()->isMeshed() && !currentPythonEngineAgros()->isScriptRunning())
        throw logic_error(QObject::tr("Problem is not meshed.").toStdString());
}

void PyViewMesh::setProblemSetting(ProblemSetting::Type type, bool value)
{
    checkExistingMesh();

    if (!silentMode())
        Agros2D::computation()->setting()->setValue(type, value);
}

void PyViewMesh::activate()
{
    checkExistingMesh();

    if (!silentMode())
    {
        // currentPythonEngineAgros()->sceneViewMesh()->actSceneModeMesh->trigger();
        Agros2D::computation()->postDeal()->refresh();
    }
}

void PyViewMesh::refresh()
{
    checkExistingMesh();

    if (!silentMode())
        Agros2D::computation()->postDeal()->refresh();
}

void PyViewMesh::setField(const std::string &fieldId)
{
    checkExistingMesh();

    if (!Agros2D::computation()->hasField(QString::fromStdString(fieldId)))
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(Agros2D::computation()->fieldInfos().keys())).toStdString());

    if (silentMode())
        return;

    FieldInfo *fieldInfo = Agros2D::computation()->fieldInfo(QString::fromStdString(fieldId));

    Agros2D::computation()->postDeal()->setActiveViewField(fieldInfo);
    Agros2D::computation()->postDeal()->refresh();
}

void PyViewMesh::setOrderViewPalette(const std::string &palette)
{
    checkExistingMesh();

    if (!paletteOrderTypeStringKeys().contains(QString::fromStdString(palette)))
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(paletteOrderTypeStringKeys())).toStdString());

    if (!silentMode())
        Agros2D::computation()->setting()->setValue(ProblemSetting::View_OrderPaletteOrderType, paletteOrderTypeFromStringKey(QString::fromStdString(palette)));
}

void PyViewMesh::setComponent(int component)
{
    checkExistingMesh();

    if (component < 1 && component > Agros2D::computation()->postDeal()->activeViewField()->numberOfSolutions())
        throw out_of_range(QObject::tr("Component must be in the range from 1 to %1.").arg(Agros2D::computation()->postDeal()->activeViewField()->numberOfSolutions()).toStdString());

    if (!silentMode())
        Agros2D::computation()->setting()->setValue(ProblemSetting::View_OrderComponent, component);
}

// ************************************************************************************

void PyViewPost::checkExistingSolution()
{
    if (!Agros2D::computation()->isSolved() && !currentPythonEngineAgros()->isScriptRunning())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());
}

void PyViewPost::setProblemSetting(ProblemSetting::Type type, bool value)
{
    checkExistingSolution();

    if (!silentMode())
        Agros2D::computation()->setting()->setValue(type, value);
}

void PyViewPost::setField(const std::string &fieldId)
{
    checkExistingSolution();

    if (!Agros2D::computation()->hasField(QString::fromStdString(fieldId)))
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(Agros2D::computation()->fieldInfos().keys())).toStdString());

    if (silentMode())
        return;

    FieldInfo *fieldInfo = Agros2D::computation()->fieldInfo(QString::fromStdString(fieldId));
    int timeStep = Agros2D::computation()->postDeal()->activeTimeStep();
    // int timeStep = Agros2D::solutionStore()->lastTimeStep(fieldInfo);

    // set last adaptivity step (keeping of previous step can be misleading)
    int adaptivityStep = Agros2D::computation()->solutionStore()->lastAdaptiveStep(fieldInfo, timeStep);

    Agros2D::computation()->postDeal()->setActiveViewField(fieldInfo);
    Agros2D::computation()->postDeal()->setActiveTimeStep(timeStep);
    Agros2D::computation()->postDeal()->setActiveAdaptivityStep(adaptivityStep);
    Agros2D::computation()->postDeal()->refresh();
}

void PyViewPost::setScalarViewVariable(const std::string &var)
{
    checkExistingSolution();

    QStringList list;
    foreach (Module::LocalVariable variable, Agros2D::computation()->postDeal()->activeViewField()->viewScalarVariables(Agros2D::computation()->config()->coordinateType()))
    {
        list.append(variable.id());
        if (variable.id() == QString::fromStdString(var))
        {
            if (!silentMode())
            {
                Agros2D::computation()->setting()->setValue(ProblemSetting::View_ScalarVariable, QString::fromStdString(var));
                return;
            }
        }
    }

    throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(list)).toStdString());
}

void PyViewPost::setScalarViewVariableComp(const std::string &component)
{
    checkExistingSolution();

    if (!physicFieldVariableCompTypeStringKeys().contains(QString::fromStdString(component)))
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(physicFieldVariableCompTypeStringKeys())).toStdString());

    if (!silentMode())
        Agros2D::computation()->setting()->setValue(ProblemSetting::View_ScalarVariableComp, physicFieldVariableCompFromStringKey(QString::fromStdString(component)));
}

void PyViewPost::setScalarViewPalette(const std::string &palette)
{
    checkExistingSolution();

    if (!paletteTypeStringKeys().contains(QString::fromStdString(palette)))
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(paletteTypeStringKeys())).toStdString());

    if (!silentMode())
        Agros2D::computation()->setting()->setValue(ProblemSetting::View_PaletteType, paletteTypeFromStringKey(QString::fromStdString(palette)));
}

// ************************************************************************************

void PyViewPost2D::activate()
{
    checkExistingSolution();

    if (!silentMode())
    {
        // currentPythonEngineAgros()->sceneViewPost2D()->actSceneModePost2D->trigger();
        Agros2D::computation()->postDeal()->refresh();
    }
}

void PyViewPost2D::refresh()
{
    checkExistingSolution();

    if (!silentMode())
        Agros2D::computation()->postDeal()->refresh();
}

void PyViewPost2D::setContourVariable(const std::string &var)
{
    checkExistingSolution();

    QStringList list;
    foreach (Module::LocalVariable variable, Agros2D::computation()->postDeal()->activeViewField()->viewScalarVariables(Agros2D::computation()->config()->coordinateType()))
    {
        if (variable.isScalar())
        {
            list.append(variable.id());

            if (variable.id() == QString::fromStdString(var))
            {
                if (!silentMode())
                {
                    Agros2D::computation()->setting()->setValue(ProblemSetting::View_ContourVariable, QString::fromStdString(var));
                    return;
                }
            }
        }
    }

    throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(list)).toStdString());
}

void PyViewPost2D::exportScalarVTK(const std::string &fileName)
{
    checkExistingSolution();

    currentPythonEngineAgros()->sceneViewPost2D()->exportVTKScalarView(QString::fromStdString(fileName));
}

void PyViewPost2D::exportContourVTK(const std::string &fileName)
{
    checkExistingSolution();

    currentPythonEngineAgros()->sceneViewPost2D()->exportVTKContourView(QString::fromStdString(fileName));
}

void PyViewPost2D::setVectorVariable(const std::string &var)
{
    checkExistingSolution();

    QStringList list;
    foreach (Module::LocalVariable variable, Agros2D::computation()->postDeal()->activeViewField()->viewVectorVariables(Agros2D::computation()->config()->coordinateType()))
    {
        list.append(variable.id());
        if (variable.id() == QString::fromStdString(var))
        {
            if (!silentMode())
            {
                Agros2D::computation()->setting()->setValue(ProblemSetting::View_VectorVariable, QString::fromStdString(var));
                return;
            }
        }
    }

    throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(list)).toStdString());
}

void PyViewPost2D::setVectorType(const std::string &type)
{
    checkExistingSolution();

    if (!vectorTypeStringKeys().contains(QString::fromStdString(type)))
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(vectorTypeStringKeys())).toStdString());

    if (!silentMode())
        Agros2D::computation()->setting()->setValue(ProblemSetting::View_VectorType, vectorTypeFromStringKey(QString::fromStdString(type)));
}

void PyViewPost2D::setVectorCenter(const std::string &center)
{
    checkExistingSolution();

    if (!vectorCenterStringKeys().contains(QString::fromStdString(center)))
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(vectorCenterStringKeys())).toStdString());

    if (!silentMode())
        Agros2D::computation()->setting()->setValue(ProblemSetting::View_VectorCenter, vectorCenterFromStringKey(QString::fromStdString(center)));
}

// ************************************************************************************

void PyViewPost3D::activate()
{
    checkExistingSolution();

    if (!silentMode())
    {
        currentPythonEngineAgros()->sceneViewPost3D()->actSceneModePost3D->trigger();
        Agros2D::computation()->postDeal()->refresh();
    }
}

void PyViewPost3D::refresh()
{
    checkExistingSolution();

    if (!silentMode())
        Agros2D::computation()->postDeal()->refresh();
}

void PyViewPost3D::setPost3DMode(const std::string &mode)
{
    if (!sceneViewPost3DModeStringKeys().contains(QString::fromStdString(mode)))
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(sceneViewPost3DModeStringKeys())).toStdString());

    if (!silentMode())
        Agros2D::computation()->setting()->setValue(ProblemSetting::View_ScalarView3DMode, sceneViewPost3DModeFromStringKey(QString::fromStdString(mode)));
}

// ************************************************************************************

void PyViewParticleTracing::activate()
{
    if (!Agros2D::computation()->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    if (!silentMode())
    {
        currentPythonEngineAgros()->sceneViewParticleTracing()->actSceneModeParticleTracing->trigger();
        Agros2D::computation()->postDeal()->refresh();
    }
}

void PyViewParticleTracing::refresh()
{
    if (!Agros2D::computation()->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    if (!silentMode())
        Agros2D::computation()->postDeal()->refresh();
}
