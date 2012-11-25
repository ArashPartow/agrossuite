// This file is part of Agros2D.
//
// Agros2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros2D.  If not, see <http://www.gnu.org/licenses/>.
//
// hp-FEM group (http://hpfem.org/)
// University of Nevada, Reno (UNR) and University of West Bohemia, Pilsen
// Email: agros2d@googlegroups.com, home page: http://hpfem.org/agros2d/

#ifndef PYTHONLABVIEW_H
#define PYTHONLABVIEW_H

#include "util.h"
#include "scene.h"
#include "hermes2d/field.h"

class Solution;
class SceneViewPreprocessor;
class SceneViewMesh;
class SceneViewPost2D;
class SceneViewPost3D;
class PostHermes;

// view
struct PyViewConfig
{
    // field
    void setField(char *fieldid);
    inline char* getField() const { return const_cast<char*>(Util::scene()->activeViewField()->fieldId().toStdString().c_str()); }

    // time step
    void setActiveTimeStep(int timeStep);
    inline int getActiveTimeStep() const { return Util::scene()->activeTimeStep(); }

    // adaptivity step
    void setActiveAdaptivityStep(int adaptivityStep);
    inline int getActiveAdaptivityStep() const { return Util::scene()->activeAdaptivityStep(); }

    // solution type
    void setActiveSolutionType(char *solutionType);
    inline char* getActiveSolutionType() const { return const_cast<char*>(solutionTypeToStringKey(Util::scene()->activeSolutionType()).toStdString().c_str()); }

    // grid
    void setGridShow(bool show);
    inline bool getGridShow() const { return Util::config()->showGrid; }
    void setGridStep(double step);
    inline double getGridStep() const { return Util::config()->gridStep; }

    // axes
    void setAxesShow(bool show);
    inline bool getAxesShow() const { return Util::config()->showAxes; }

    // rulers
    void setRulersShow(bool show);
    inline bool getRulersShow() const { return Util::config()->showRulers; }

    // todo: (Franta) font, size of nodes and edges and labels, colors
};

// view mesh
struct PyViewMesh
{
    void activate();

    // mesh
    void setInitialMeshViewShow(bool show);
    inline bool getInitialMeshViewShow() const { return Util::config()->showInitialMeshView; }
    void setSolutionMeshViewShow(bool show);
    inline bool getSolutionMeshViewShow() const { return Util::config()->showSolutionMeshView; }

    // polynomial order
    void setOrderViewShow(bool show);
    inline bool getOrderViewShow() const { return Util::config()->showOrderView; }
    void setOrderViewColorBar(bool show);
    inline bool getOrderViewColorBar() const { return Util::config()->showOrderColorBar; }
    void setOrderViewLabel(bool show);
    inline bool getOrderViewLabel() const { return Util::config()->orderLabel; }
    void setOrderViewPalette(char* palette);
    inline char* getOrderViewPalette() const { return const_cast<char*>(paletteOrderTypeToStringKey(Util::config()->orderPaletteOrderType).toStdString().c_str()); }
};

// post2d
struct PyViewPost2D
{
    void activate();

    // scalar view
    void setScalarViewShow(bool show);
    inline bool getScalarViewShow() const { return Util::config()->showScalarView; }
    void setScalarViewVariable(char* var);
    inline char* getScalarViewVariable() const { return const_cast<char*>(Util::config()->scalarVariable.toStdString().c_str()); }
    void setScalarViewVariableComp(char* component);
    inline char* getScalarViewVariableComp() const { return const_cast<char*>(physicFieldVariableCompToStringKey(Util::config()->scalarVariableComp).toStdString().c_str()); }

    void setScalarViewPalette(char* palette);
    inline char* getScalarViewPalette() const { return const_cast<char*>(paletteTypeToStringKey(Util::config()->paletteType).toStdString().c_str()); }
    void setScalarViewPaletteQuality(char* quality);
    inline char* getScalarViewPaletteQuality() const { return const_cast<char*>(paletteQualityToStringKey(Util::config()->linearizerQuality).toStdString().c_str()); }
    void setScalarViewPaletteSteps(int steps);
    inline int getScalarViewPaletteSteps() const { return Util::config()->paletteSteps; }
    void setScalarViewPaletteFilter(bool filter);
    inline bool getScalarViewPaletteFilter() const { return Util::config()->paletteFilter; }

    void setScalarViewRangeLog(bool log);
    inline bool getScalarViewRangeLog() const { return Util::config()->scalarRangeLog; }
    void setScalarViewRangeBase(double base);
    inline double getScalarViewRangeBase() const { return Util::config()->scalarRangeBase; }

    void setScalarViewColorBar(bool show);
    inline bool getScalarViewColorBar() const { return Util::config()->showScalarColorBar; }
    void setScalarViewDecimalPlace(int place);
    inline int getScalarViewDecimalPlace() const { return Util::config()->scalarDecimalPlace; }

    void setScalarViewRangeAuto(bool autoRange);
    inline bool getScalarViewRangeAuto() const { return Util::config()->scalarRangeAuto; }
    void setScalarViewRangeMin(double min);
    inline double getScalarViewRangeMin() const { return Util::config()->scalarRangeMin; }
    void setScalarViewRangeMax(double max);
    inline double getScalarViewRangeMax() const { return Util::config()->scalarRangeMax; }

    // contour
    void setContourShow(bool show);
    inline bool getContourShow() const { return Util::config()->showContourView; }
    void setContourCount(int count);
    inline int getContourCount() const { return Util::config()->contoursCount; }
    void setContourVariable(char* var);
    inline char* getContourVariable() const { return const_cast<char*>(Util::config()->contourVariable.toStdString().c_str()); }

    // vector
    void setVectorShow(bool show);
    inline bool getVectorShow() const { return Util::config()->showVectorView; }
    void setVectorCount(int count);
    inline int getVectorCount() const { return Util::config()->vectorCount; }
    void setVectorScale(double scale);
    inline int getVectorScale() const { return Util::config()->vectorScale; }
    void setVectorVariable(char* var);
    inline char* getVectorVariable() const { return const_cast<char*>(Util::config()->vectorVariable.toStdString().c_str()); }
    void setVectorProportional(bool show);
    inline bool getVectorProportional() const { return Util::config()->vectorProportional; }
    void setVectorColor(bool show);
    inline bool getVectorColor() const { return Util::config()->vectorColor; }
};

// post3d
struct PyViewPost3D
{
    void activate();

    // scalar view
    void setPost3DMode(char* mode);
    inline char* getPost3DMode() const { return const_cast<char*>(sceneViewPost3DModeToStringKey(Util::config()->showPost3D).toStdString().c_str()); }

    // TODO: (Franta) duplicated code
    void setScalarViewVariable(char* var);
    inline char* getScalarViewVariable() const { return const_cast<char*>(Util::config()->scalarVariable.toStdString().c_str()); }
    void setScalarViewVariableComp(char* component);
    inline char* getScalarViewVariableComp() const { return const_cast<char*>(physicFieldVariableCompToStringKey(Util::config()->scalarVariableComp).toStdString().c_str()); }

    void setScalarViewPalette(char* palette);
    inline char* getScalarViewPalette() const { return const_cast<char*>(paletteTypeToStringKey(Util::config()->paletteType).toStdString().c_str()); }
    void setScalarViewPaletteQuality(char* quality);
    inline char* getScalarViewPaletteQuality() const { return const_cast<char*>(paletteQualityToStringKey(Util::config()->linearizerQuality).toStdString().c_str()); }
    void setScalarViewPaletteSteps(int steps);
    inline int getScalarViewPaletteSteps() const { return Util::config()->paletteSteps; }
    void setScalarViewPaletteFilter(bool filter);
    inline bool getScalarViewPaletteFilter() const { return Util::config()->paletteFilter; }

    void setScalarViewRangeLog(bool log);
    inline bool getScalarViewRangeLog() const { return Util::config()->scalarRangeLog; }
    void setScalarViewRangeBase(double base);
    inline double getScalarViewRangeBase() const { return Util::config()->scalarRangeBase; }

    void setScalarViewColorBar(bool show);
    inline bool getScalarViewColorBar() const { return Util::config()->showScalarColorBar; }
    void setScalarViewDecimalPlace(int place);
    inline int getScalarViewDecimalPlace() const { return Util::config()->scalarDecimalPlace; }

    void setScalarViewRangeAuto(bool autoRange);
    inline bool getScalarViewRangeAuto() const { return Util::config()->scalarRangeAuto; }
    void setScalarViewRangeMin(double min);
    inline double getScalarViewRangeMin() const { return Util::config()->scalarRangeMin; }
    void setScalarViewRangeMax(double max);
    inline double getScalarViewRangeMax() const { return Util::config()->scalarRangeMax; }
};

#endif // PYTHONLABVIEW_H
