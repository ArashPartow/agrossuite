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

#ifndef PLUGIN_INTERFACE_H
#define PLUGIN_INTERFACE_H

#include <QtPlugin>

#include "util.h"
#include "hermes2d.h"

#include "scene.h"
#include "scenebasic.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"

#include "hermes2d/module.h"
#include "hermes2d/marker.h"

#include "../../resources_source/classes/coupling_xml.h"

struct PointValue
{
    PointValue()
    {
        this->scalar = 0.0;
        this->vector = Point();
        this->material = NULL;
    }

    PointValue(double scalar, Point vector, Material *material)
    {
        this->scalar = scalar;
        this->vector = vector;
        this->material = material;
    }

    double scalar;
    Point vector;
    Material *material;
};

class LocalValue
{
public:
    LocalValue(FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType, const Point &point)
        : m_fieldInfo(fieldInfo), m_timeStep(timeStep), m_adaptivityStep(adaptivityStep), m_solutionType(solutionType), m_point(point) {}

    // point
    inline Point point() { return m_point; }

    // variables
    QMap<QString, PointValue> values() const { return m_values; }

    virtual void calculate() = 0;

protected:
    // point
    Point m_point;
    // field info
    FieldInfo *m_fieldInfo;
    int m_timeStep;
    int m_adaptivityStep;
    SolutionMode m_solutionType;

    // variables
    QMap<QString, PointValue> m_values;
};

class IntegralValue
{
public:
    IntegralValue(FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType)
        : m_fieldInfo(fieldInfo), m_timeStep(timeStep), m_adaptivityStep(adaptivityStep), m_solutionType(solutionType) {}

    // variables
    inline QMap<QString, double> values() const { return m_values; }

protected:
    // field info
    FieldInfo *m_fieldInfo;
    int m_timeStep;
    int m_adaptivityStep;
    SolutionMode m_solutionType;

    // variables
    QMap<QString, double> m_values;
};

const int OFFSET_NON_DEF = -100;

class FormAgrosInterface
{
public:
    FormAgrosInterface(int offsetI, int offsetJ) : m_markerSource(NULL), m_markerTarget(NULL), m_table(NULL), m_offsetI(offsetI), m_offsetJ(offsetJ), m_markerVolumeCalculated(false) {}

    // source or single marker
    virtual inline void setMarkerSource(Marker *marker) { m_markerSource = marker; m_markerVolumeCalculated = false;}
    inline Marker *markerSource() { assert(m_markerSource); return m_markerSource; }

    // target marker
    virtual inline void setMarkerTarget(Marker *marker) { m_markerTarget = marker; }
    inline Marker *markerTarget() { assert(m_markerTarget); return m_markerTarget; }

    // time discretisation table
    inline void setTimeDiscretisationTable(BDF2Table** table) { m_table = table; }

    // volume (area) of the marekr
    double markerVolume() const;
    virtual void calculateMarkerVolume() const {assert(0);}

protected:
    // source or single marker
    Marker *m_markerSource;
    // target marker
    Marker *m_markerTarget;
    // time discretisation table
    BDF2Table **m_table;

    // the offset of position in the stiffness matrix for the case of hard coupling; could be done some other way
    // for example, generated form ...something(heat_matrix_linear, etc)...._1_3 could have variables holding 1 and 3 (the original position,
    // before the schift) offsetI and offsetJ than could be obtained as different of form->i (j), the real position
    // and the original position
    int m_offsetI;
    int m_offsetJ;

    mutable bool m_markerVolumeCalculated;
    mutable double m_markerVolume;
};

// weakforms
template<typename Scalar>
class MatrixFormVolAgros : public Hermes::Hermes2D::MatrixFormVol<Scalar>, public FormAgrosInterface
{
public:
    MatrixFormVolAgros(unsigned int i, unsigned int j, int offsetI, int offsetJ)
        : Hermes::Hermes2D::MatrixFormVol<Scalar>(i, j), FormAgrosInterface(offsetI, offsetJ) {}
    virtual void calculateMarkerVolume() const
    {
        QMutex mutex;
        mutex.lock();
        {
            if(!m_markerVolumeCalculated)
            {
                MeshSharedPtr initialMesh = this->m_markerSource->fieldInfo()->initialMesh();
                //Hermes::Hermes2D::Mesh::ElementMarkersConversion& markersConversion = initialMesh->get_element_markers_conversion();
                //qDebug() << markersConversion.size();
                m_markerVolume = initialMesh->get_marker_area(initialMesh->get_element_markers_conversion().get_internal_marker(this->getAreas().at(0)).marker);
                m_markerVolumeCalculated = true;
            }
        }
        mutex.unlock();
    }
};

template<typename Scalar>
class VectorFormVolAgros : public Hermes::Hermes2D::VectorFormVol<Scalar>, public FormAgrosInterface
{
public:
    VectorFormVolAgros(unsigned int i, int offsetI, int offsetJ, int *offsetTimeExt)
        : Hermes::Hermes2D::VectorFormVol<Scalar>(i), FormAgrosInterface(offsetI, offsetJ), m_offsetTimeExt(offsetTimeExt) {}
    virtual void calculateMarkerVolume() const
    {
        QMutex mutex;
        mutex.lock();
        {
            if(!m_markerVolumeCalculated)
            {
                MeshSharedPtr initialMesh = this->m_markerSource->fieldInfo()->initialMesh();
                m_markerVolume = initialMesh->get_marker_area(initialMesh->get_element_markers_conversion().get_internal_marker(this->getAreas().at(0)).marker);
                m_markerVolumeCalculated = true;
            }
        }
        mutex.unlock();
    }

protected:
    int *m_offsetTimeExt;
};

template<typename Scalar>
class MatrixFormSurfAgros : public Hermes::Hermes2D::MatrixFormSurf<Scalar>, public FormAgrosInterface
{
public:
    MatrixFormSurfAgros(unsigned int i, unsigned int j, int offsetI, int offsetJ)
        : Hermes::Hermes2D::MatrixFormSurf<Scalar>(i, j), FormAgrosInterface(offsetI, offsetJ) {}
};

template<typename Scalar>
class VectorFormSurfAgros : public Hermes::Hermes2D::VectorFormSurf<Scalar>, public FormAgrosInterface
{
public:
    VectorFormSurfAgros(unsigned int i, int offsetI, int offsetJ)
        : Hermes::Hermes2D::VectorFormSurf<Scalar>(i), FormAgrosInterface(offsetI, offsetJ) {}
};

template<typename Scalar>
class ExactSolutionScalarAgros : public Hermes::Hermes2D::ExactSolutionScalar<Scalar>, public FormAgrosInterface
{
public:
    ExactSolutionScalarAgros(MeshSharedPtr mesh)
        : Hermes::Hermes2D::ExactSolutionScalar<Scalar>(mesh), FormAgrosInterface(OFFSET_NON_DEF, OFFSET_NON_DEF) {}
};


// plugin interface
class AGROS_LIBRARY_API PluginInterface
{
public:
    inline PluginInterface() : m_module(NULL), m_coupling(NULL) {}
    virtual ~PluginInterface() {}

    virtual QString fieldId() = 0;

    inline XMLModule::module *module() const { assert(m_module); return m_module; }
    inline XMLCoupling::coupling *coupling() const { assert(m_coupling); return m_coupling; }

    // weak forms
    virtual MatrixFormVolAgros<double> *matrixFormVol(const ProblemID problemId, FormInfo *form, int offsetI, int offsetJ, Material *material) = 0;
    virtual VectorFormVolAgros<double> *vectorFormVol(const ProblemID problemId, FormInfo *form, int offsetI, int offsetJ, Material *material, int *offsetTimeExt) = 0;
    virtual MatrixFormSurfAgros<double> *matrixFormSurf(const ProblemID problemId, FormInfo *form, int offsetI, int offsetJ, Boundary *boundary) = 0;
    virtual VectorFormSurfAgros<double> *vectorFormSurf(const ProblemID problemId, FormInfo *form, int offsetI, int offsetJ, Boundary *boundary) = 0;

    virtual ExactSolutionScalarAgros<double> *exactSolution(const ProblemID problemId, FormInfo *form, MeshSharedPtr mesh) = 0;

    // postprocessor
    // filter
    virtual MeshFunctionSharedPtr<double> filter(FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType,
                                                 Hermes::vector<MeshFunctionSharedPtr<double> > sln,
                                                 const QString &variable,
                                                 PhysicFieldVariableComp physicFieldVariableComp) = 0;

    // local values
    virtual LocalValue *localValue(FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType, const Point &point) = 0;
    // surface integrals
    virtual IntegralValue *surfaceIntegral(FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType) = 0;
    // volume integrals
    virtual IntegralValue *volumeIntegral(FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType) = 0;
    // force calculation
    virtual Point3 force(FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType,
                         Hermes::Hermes2D::Element *element, SceneMaterial *material, const Point3 &point, const Point3 &velocity) = 0;
    virtual bool hasForce(FieldInfo *fieldInfo) = 0;

    // localization
    virtual QString localeName(const QString &name) = 0;
    // description
    virtual QString localeDescription() = 0;

protected:
    XMLModule::module *m_module;
    XMLCoupling::coupling *m_coupling;
};

const int IMPLIICT_APPROX_COUNT = 500;

template <typename Scalar>
class SpecialFunction
{
public:
    SpecialFunction() : m_interpolation(0), m_bound_low(0), m_bound_hi(0), m_count(IMPLIICT_APPROX_COUNT){}
    ~SpecialFunction();
    Scalar operator()(double h) const;
    Hermes::Ord operator()(Hermes::Ord h) const { return Hermes::Ord(10); }
    void setBounds(double bound_low, double bound_hi, bool extrapolate_low, bool extrapolate_hi);
    void createInterpolation();
    virtual Scalar value(double h) = 0;
    virtual Scalar extrapolation_low() = 0;
    virtual Scalar extrapolation_hi() = 0;
    void setVariant(QString variant) { m_variant = variant; }
protected:
    QSharedPointer<PiecewiseLinear> m_interpolation;
    double m_bound_low;
    double m_bound_hi;
    bool m_extrapolation_low_present;
    double m_extrapolation_low;
    bool m_extrapolation_hi_present;
    double m_extrapolation_hi;
    double m_count;
    QString m_variant;
};


QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(PluginInterface, "agros2d.PluginInterface/1.0")
QT_END_NAMESPACE

#endif // PLUGIN_INTERFACE_H
