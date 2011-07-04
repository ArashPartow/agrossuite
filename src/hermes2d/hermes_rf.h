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

#ifndef RF_H
#define RF_H

#include "util.h"
#include "module.h"

struct ModuleRF : public Hermes::Module::ModuleAgros
{
    Q_OBJECT
public:
    ModuleRF(ProblemType problemType, AnalysisType analysisType) : Hermes::Module::ModuleAgros(problemType, analysisType) {}

    inline int number_of_solution() const { return 2; }
    bool has_nonlinearity() const { return false; }

    LocalPointValue *local_point_value(const Point &point);
    SurfaceIntegralValue *surface_integral_value();
    VolumeIntegralValue *volume_integral_value();

    ViewScalarFilter *view_scalar_filter(Hermes::Module::LocalVariable *physicFieldVariable,
                                         PhysicFieldVariableComp physicFieldVariableComp);

    Hermes::vector<SolutionArray *> solve(ProgressItemSolve *progressItemSolve);

    // rewrite
    void readBoundaryFromDomElement(QDomElement *element);
    void writeBoundaryToDomElement(QDomElement *element, SceneBoundary *marker);
    void readMaterialFromDomElement(QDomElement *element);
    void writeMaterialToDomElement(QDomElement *element, SceneMaterial *marker);

    SceneBoundary *newBoundary();
    SceneBoundary *newBoundary(PyObject *self, PyObject *args);
    SceneBoundary *modifyBoundary(PyObject *self, PyObject *args);
    SceneMaterial *newMaterial();
    SceneMaterial *newMaterial(PyObject *self, PyObject *args);
    SceneMaterial *modifyMaterial(PyObject *self, PyObject *args);
};

// *******************************************************************************************

class ParserRF : public Parser
{
public:
    double pepsr;
    double pmur;
    double pgamma;
    double pjer;
    double pjei;

    void setParserVariables(SceneMaterial *material);
};

class LocalPointValueRF : public LocalPointValue
{
public:
    LocalPointValueRF(const Point &point);
};

class SurfaceIntegralValueRF : public SurfaceIntegralValue
{
public:
    SurfaceIntegralValueRF();
};

class VolumeIntegralValueRF : public VolumeIntegralValue
{
public:
    VolumeIntegralValueRF();
};

class ViewScalarFilterRF : public ViewScalarFilter
{
public:
    ViewScalarFilterRF(Hermes::vector<MeshFunction *> sln,
                                   std::string expression);
};

// *******************************************************************************************

class SceneBoundaryRF : public SceneBoundary
{
public:
    Value value_real;
    Value value_imag;
    Mode mode;
    Value power;
    Value phase;

    SceneBoundaryRF(const QString &name, PhysicFieldBC type, Value value_real, Value value_imag);
    SceneBoundaryRF(const QString &name, PhysicFieldBC type, Mode mode, Value power, Value phase);
    SceneBoundaryRF(const QString &name, PhysicFieldBC type);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class SceneMaterialRF : public SceneMaterial
{
public:
    Value permittivity;
    Value permeability;
    Value conductivity;
    Value current_density_real;
    Value current_density_imag;

    SceneMaterialRF(const QString &name, Value permittivity, Value permeability, Value conductivity, Value current_density_real, Value current_density_imag);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class SceneBoundaryRFDialog : public SceneBoundaryDialog
{
    Q_OBJECT

public:
    SceneBoundaryRFDialog(SceneBoundaryRF *edgeRFMarker, QWidget *parent);

protected:
    void createContent();

    void load();
    bool save();

private:
    QLabel *lblValueUnitReal;
    QLabel *lblValueUnitImag;
    QComboBox *cmbType;
    QComboBox *cmbMode;
    ValueLineEdit *txtValueReal;
    ValueLineEdit *txtValueImag;

private slots:
    void doTypeChanged(int index);
};

class SceneMaterialRFDialog : public SceneMaterialDialog
{
    Q_OBJECT

public:
    SceneMaterialRFDialog(QWidget *parent, SceneMaterialRF *labelRFMarker);

protected:
    void createContent();

    void load();
    bool save();

private:
    ValueLineEdit *txtPermittivity;
    ValueLineEdit *txtPermeability;
    ValueLineEdit *txtConductivity;
    ValueLineEdit *txtCurrentDensityReal;
    ValueLineEdit *txtCurrentDensityImag;

};

#endif // RF_H
