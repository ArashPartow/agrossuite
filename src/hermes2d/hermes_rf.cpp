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

#include "hermes_rf.h"
#include "scene.h"
#include "gui.h"

struct RFEdge
{
    PhysicFieldBC type;
    TEMode mode;
    double value_real;
    double value_imag;
    double power;
    double phase;
    double height;
    Point start;
    Point end;
    double angle;
};

struct RFLabel
{
    double permittivity;
    double permeability;
    double conductivity;
    double current_density_real;
    double current_density_imag;
};

RFEdge *rfEdge;
RFLabel *rfLabel;

template<typename Real, typename Scalar>
Scalar rf_matrix_form_surf_imag_real(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (!(rfEdge[e->edge_marker].type == PhysicFieldBC_RF_MatchedBoundary ||
          rfEdge[e->edge_marker].type == PhysicFieldBC_RF_Port))
        return 0.0;

    double mu = rfLabel[e->elem_marker].permeability * MU0;
    double eps = rfLabel[e->elem_marker].permittivity * EPS0;
    int mode = 1;

    double beta = 0.0;
    double Z = 0.0;
    double height = rfEdge[e->edge_marker].height;

    if(!rfEdge[e->edge_marker].height == 0)
    {
        beta = sqrt(sqr(2 * M_PI * frequency) * mu * eps - sqr(mode * M_PI / height));

        return beta * int_u_v<Real, Scalar>(n, wt, u, v);
    }
    else
    {
        beta = sqrt(sqr(2 * M_PI * frequency) * mu * eps);
        Z = ((2 * M_PI * frequency) * mu ) / beta;

        return Z * int_u_v<Real, Scalar>(n, wt, u, v);
    }
}

template<typename Real, typename Scalar>
Scalar rf_matrix_form_surf_real_imag(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (!(rfEdge[e->edge_marker].type == PhysicFieldBC_RF_MatchedBoundary ||
          rfEdge[e->edge_marker].type == PhysicFieldBC_RF_Port))
        return 0.0;

    double mu = rfLabel[e->elem_marker].permeability * MU0;
    double eps = rfLabel[e->elem_marker].permittivity * EPS0;
    int mode = 1;

    double beta = 0.0;
    double Z = 0.0;
    double height = rfEdge[e->edge_marker].height;

    if(!rfEdge[e->edge_marker].height == 0)
    {
        beta = sqrt(sqr(2 * M_PI * frequency) * mu * eps - sqr(mode * M_PI / height));

        return beta * int_u_v<Real, Scalar>(n, wt, u, v);
    }
    else
    {
        beta = sqrt(sqr(2 * M_PI * frequency) * mu * eps);
        Z = ((2 * M_PI * frequency) * mu ) / beta;

        return Z * int_u_v<Real, Scalar>(n, wt, u, v);
    }
}

template<typename Real, typename Scalar>
Scalar rf_vector_form_surf_real(int n, double *wt, Func<Real> *u_ext[], Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (rfEdge[e->edge_marker].type != PhysicFieldBC_RF_Port)
        return 0.0;

    //   return 0.0;
    // dodelat clen  + 2*j*beta*E0z
    // kde E0z je podle mode
    // pro mode = 1 => pulvlna sinusovky
    // e->x[i]

    double mu = rfLabel[e->elem_marker].permeability * MU0;
    double eps = rfLabel[e->elem_marker].permittivity * EPS0;
    Scalar E0z = 0.0;
    double beta = 0.0;
    int mode = 0;
    Scalar length = sqrt(sqr(rfEdge[e->edge_marker].end.x - rfEdge[e->edge_marker].start.x) +
                         sqr(rfEdge[e->edge_marker].end.y - rfEdge[e->edge_marker].start.y));

    Scalar result = 0;
    for (int i = 0; i < n; i++)
    {
        Scalar lengthPoint = sqrt(sqr(e->x[i] - rfEdge[e->edge_marker].start.x) +
                                  sqr(e->y[i] - rfEdge[e->edge_marker].start.y));

        switch (rfEdge[e->edge_marker].mode)
        {
        case TEMode_0:
        {
            mode = 1;
            if(!rfEdge[e->edge_marker].height == 0)
                beta = sqrt(sqr(2 * M_PI * frequency) * mu * eps - sqr(mode * M_PI / (rfEdge[e->edge_marker].height)));
            else
                beta = sqr(2 * M_PI * frequency) * mu * eps;
            E0z = rfEdge[e->edge_marker].power * sin((lengthPoint * M_PI) / length);
        }

            break;
        case TEMode_1:
        {
            mode = 1;
            beta = sqrt(sqr(2 * M_PI * frequency) * mu * eps - sqr(mode * M_PI / (rfEdge[e->edge_marker].height)));
            E0z =  rfEdge[e->edge_marker].power * cos((lengthPoint * M_PI) / (rfEdge[e->edge_marker].height));
        }
            break;
        case TEMode_2:
        {
            mode = 1;
            beta = sqrt(sqr(2 * M_PI * frequency) * mu * eps - sqr(mode * M_PI / (rfEdge[e->edge_marker].height)));
            E0z =  0;
        }
            break;
        default:
            break;
        }

        E0z *= sin(rfEdge[e->edge_marker].phase / 180.0 * M_PI);

        if (isPlanar)
            result += wt[i] * E0z * 2 * beta * (v->val[i]);
        else
            result += 2 * M_PI * wt[i] * E0z * 2 * beta * (e->x[i] * v->val[i]);
    }

    return result;
}

template<typename Real, typename Scalar>
Scalar rf_vector_form_surf_imag(int n, double *wt, Func<Real> *u_ext[], Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (rfEdge[e->edge_marker].type != PhysicFieldBC_RF_Port)
        return 0.0;

    //   return 0.0;
    // dodelat clen  + 2*j*beta*E0z
    // kde E0z je podle mode
    // pro mode = 1 => pulvlna sinusovky
    // e->x[i]

    double mu = rfLabel[e->elem_marker].permeability * MU0;
    double eps = rfLabel[e->elem_marker].permittivity * EPS0;
    Scalar E0z = 0.0;
    double beta = 0.0;
    int mode = 0;
    Scalar length = sqrt(sqr(rfEdge[e->edge_marker].end.x - rfEdge[e->edge_marker].start.x) +
                         sqr(rfEdge[e->edge_marker].end.y - rfEdge[e->edge_marker].start.y));

    Scalar result = 0;
    for (int i = 0; i < n; i++)
    {
        Scalar lengthPoint = sqrt(sqr(e->x[i] - rfEdge[e->edge_marker].start.x) +
                                  sqr(e->y[i] - rfEdge[e->edge_marker].start.y));

        switch (rfEdge[e->edge_marker].mode)
        {
        case TEMode_0:
        {
            mode = 1;
            if(!rfEdge[e->edge_marker].height == 0)
                beta = sqrt(sqr(2 * M_PI * frequency) * mu * eps - sqr(mode * M_PI / (rfEdge[e->edge_marker].height)));
            else
                beta = sqr(2 * M_PI * frequency) * mu * eps;
            E0z = rfEdge[e->edge_marker].power * sin((lengthPoint * M_PI) / length);
        }

            break;
        case TEMode_1:
        {
            mode = 1;
            beta = sqrt(sqr(2 * M_PI * frequency) * mu * eps - sqr(mode * M_PI / (rfEdge[e->edge_marker].height)));
            E0z =  rfEdge[e->edge_marker].power * cos((lengthPoint * M_PI) / (rfEdge[e->edge_marker].height));
        }
            break;
        case TEMode_2:
        {
            mode = 1;
            beta = sqrt(sqr(2 * M_PI * frequency) * mu * eps - sqr(mode * M_PI / (rfEdge[e->edge_marker].height)));
            E0z =  0;
        }
            break;
        default:
            break;
        }

        E0z *= - cos(rfEdge[e->edge_marker].phase / 180.0 * M_PI);

        if (isPlanar)
            result += wt[i] * E0z * 2 * beta * (v->val[i]);
        else
            result += 2 * M_PI * wt[i] * E0z * 2 * beta * (e->x[i] * v->val[i]);
    }
    return result;
}

template<typename Real, typename Scalar>
Scalar rf_matrix_form_real_real(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (isPlanar)
        return - int_grad_u_grad_v<Real, Scalar>(n, wt, u, v)
                + sqr(2 * M_PI * frequency) * (rfLabel[e->elem_marker].permeability * MU0) * (rfLabel[e->elem_marker].permittivity * EPS0)
                * int_u_v<Real, Scalar>(n, wt, u, v);
    else
    {
        Scalar result_1 = 0;
        Scalar result_2 = 0;
        for (int i = 0; i < n; i++)
        {
            result_1 += wt[i] * (u->dx[i] * v->val[i]) / e->x[i];
            result_2 += wt[i] * (u->val[i] * v->val[i]) / (e->x[i] * e->x[i]);
        }

        return  - 2 * M_PI * (result_1 + int_grad_u_grad_v<Real, Scalar>(n, wt, u, v) - result_2)
                + sqr(2 * M_PI * frequency) * (rfLabel[e->elem_marker].permeability * MU0) * (rfLabel[e->elem_marker].permittivity * EPS0)
                * int_u_v<Real, Scalar>(n, wt, u, v);
    }
}



template<typename Real, typename Scalar>
Scalar rf_matrix_form_real_imag(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (isPlanar)
        return + 2 * M_PI * frequency * (rfLabel[e->elem_marker].permeability * MU0) * rfLabel[e->elem_marker].conductivity
                * int_u_v<Real, Scalar>(n, wt, u, v);
    else
        //return 0.0;
        return + 2 * M_PI * frequency * (rfLabel[e->elem_marker].permeability * MU0) * rfLabel[e->elem_marker].conductivity
                * int_u_v<Real, Scalar>(n, wt, u, v);
}

template<typename Real, typename Scalar>
Scalar rf_matrix_form_imag_real(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (isPlanar)
        return - 2 * M_PI * frequency * (rfLabel[e->elem_marker].permeability * MU0) * rfLabel[e->elem_marker].conductivity
                * int_u_v<Real, Scalar>(n, wt, u, v);
    else
        //return 0.0;
        return - 2 * M_PI * frequency * (rfLabel[e->elem_marker].permeability * MU0) * rfLabel[e->elem_marker].conductivity
                * int_u_v<Real, Scalar>(n, wt, u, v);
}

template<typename Real, typename Scalar>
Scalar rf_matrix_form_imag_imag(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (isPlanar)
        return - int_grad_u_grad_v<Real, Scalar>(n, wt, u, v)
                + sqr(2 * M_PI * frequency) * (rfLabel[e->elem_marker].permeability * MU0) * (rfLabel[e->elem_marker].permittivity * EPS0)
                * int_u_v<Real, Scalar>(n, wt, u, v);
    else
    {
        Scalar result_1 = 0;
        Scalar result_2 = 0;
        for (int i = 0; i < n; i++)
        {
            result_1 += wt[i] * (u->dx[i] * v->val[i]) / e->x[i];
            result_2 += wt[i] * (u->val[i] * v->val[i]) / (e->x[i] * e->x[i]);
        }

        return - 2 * M_PI * (result_1 + int_grad_u_grad_v<Real, Scalar>(n, wt, u, v) - result_2)
                + sqr(2 * M_PI * frequency) * (rfLabel[e->elem_marker].permeability * MU0) * (rfLabel[e->elem_marker].permittivity * EPS0)
                * int_u_v<Real, Scalar>(n, wt, u, v);
    }
}
/*
template<typename Real, typename Scalar>
Scalar rf_vector_form_real(int n, double *wt, Func<Real> *u_ext[], Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    return - 2 * M_PI * frequency * (rfLabel[e->elem_marker].permeability * MU0) * rfLabel[e->elem_marker].current_density_imag;
}
*/

template<typename Real, typename Scalar>
Scalar rf_vector_form_real(int n, double *wt, Func<Real> *u_ext[], Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    Scalar result = 0 ;
    int u = 0;
    for (int i = 0; i < n; i++)
        result += wt[i] * (rfLabel[e->elem_marker].current_density_imag * v->val[i]);

    return - 2 * M_PI * frequency * (rfLabel[e->elem_marker].permeability * MU0) * result;
}


/*
template<typename Real, typename Scalar>
Scalar rf_vector_form_imag(int n, double *wt, Func<Real> *u_ext[], Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    return 2 * M_PI * frequency * (rfLabel[e->elem_marker].permeability * MU0) * rfLabel[e->elem_marker].current_density_real;
}
*/

template<typename Real, typename Scalar>
Scalar rf_vector_form_imag(int n, double *wt, Func<Real> *u_ext[], Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    Scalar result = 0 ;
    int u = 0;
    for (int i = 0; i < n; i++)
        result += wt[i] * (rfLabel[e->elem_marker].current_density_real * v->val[i]);


    return 2 * M_PI * frequency * (rfLabel[e->elem_marker].permeability * MU0) * result;
}


void callbackRFWeakForm(WeakForm *wf, Hermes::vector<Solution *> slnArray)
{
    if (slnArray.size() == 1)
    {
        wf->add_matrix_form(0, 0, callback(rf_matrix_form_real_real));
        if (analysisType == AnalysisType_Transient)
            wf->add_vector_form(0, callback(rf_vector_form_real), HERMES_ANY, slnArray.at(0));
        else
            wf->add_vector_form(0, callback(rf_vector_form_real));
        wf->add_vector_form_surf(0, callback(rf_vector_form_surf_real));
    }
    else
    {
        wf->add_matrix_form(0, 0, callback(rf_matrix_form_real_real));
        wf->add_matrix_form(0, 1, callback(rf_matrix_form_real_imag));
        wf->add_matrix_form(1, 0, callback(rf_matrix_form_imag_real));
        wf->add_matrix_form(1, 1, callback(rf_matrix_form_imag_imag));
        wf->add_vector_form(0, callback(rf_vector_form_real));
        wf->add_vector_form(1, callback(rf_vector_form_imag));
        wf->add_matrix_form_surf(0, 1, callback(rf_matrix_form_surf_imag_real));
        wf->add_matrix_form_surf(1, 0, callback(rf_matrix_form_surf_real_imag));
        wf->add_vector_form_surf(0, callback(rf_vector_form_surf_real));
        wf->add_vector_form_surf(1, callback(rf_vector_form_surf_imag));
    }
}

// *******************************************************************************************************

int HermesRF::numberOfSolution() const
{
    return (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic) ? 2 : 1;
}

PhysicFieldVariable HermesRF::contourPhysicFieldVariable()
{
    return PhysicFieldVariable_RF_ElectricFieldReal;
}

PhysicFieldVariable HermesRF::scalarPhysicFieldVariable()
{
    return PhysicFieldVariable_RF_ElectricFieldReal;
}

PhysicFieldVariableComp HermesRF::scalarPhysicFieldVariableComp()
{
    return PhysicFieldVariableComp_Magnitude;
}

PhysicFieldVariable HermesRF::vectorPhysicFieldVariable()
{
    return PhysicFieldVariable_RF_ElectricFieldReal;
}


void HermesRF::readEdgeMarkerFromDomElement(QDomElement *element)
{
    TEMode mode = teModeFromStringKey(element->attribute("mode"));
    PhysicFieldBC type = physicFieldBCFromStringKey(element->attribute("type"));
    switch (type)
    {
    case PhysicFieldBC_None:
        break;
    case PhysicFieldBC_RF_ElectricField:
    case PhysicFieldBC_RF_MagneticField:
        Util::scene()->addEdgeMarker(new SceneEdgeRFMarker(element->attribute("name"),
                                                           type,
                                                           Value(element->attribute("value_real", "0")),
                                                           Value(element->attribute("value_imag", "0"))));
        break;
    case PhysicFieldBC_RF_Port:
        Util::scene()->addEdgeMarker(new SceneEdgeRFMarker(element->attribute("name"),
                                                           type,
                                                           mode,
                                                           Value(element->attribute("power", "0")),
                                                           Value(element->attribute("phase", "0")),
                                                           Value(element->attribute("height", "0"))));
        break;
    case PhysicFieldBC_RF_MatchedBoundary:
        Util::scene()->addEdgeMarker(new SceneEdgeRFMarker(element->attribute("name"),
                                                           type,
                                                           Value(element->attribute("height", "0"))));
        break;
    default:
        std::cerr << tr("Boundary type '%1' doesn't exists.").arg(element->attribute("type")).toStdString() << endl;
        break;
    }
}

void HermesRF::writeEdgeMarkerToDomElement(QDomElement *element, SceneEdgeMarker *marker)
{
    SceneEdgeRFMarker *edgeRFMarker = dynamic_cast<SceneEdgeRFMarker *>(marker);

    element->setAttribute("type", physicFieldBCToStringKey(edgeRFMarker->type));

    switch (edgeRFMarker->type)
    {
    case PhysicFieldBC_RF_ElectricField:
    case PhysicFieldBC_RF_MagneticField:
        element->setAttribute("value_real", edgeRFMarker->value_real.text);
        element->setAttribute("value_imag", edgeRFMarker->value_imag.text);
        break;
    case PhysicFieldBC_RF_Port:
        element->setAttribute("power", edgeRFMarker->power.text);
        element->setAttribute("phase", edgeRFMarker->phase.text);
        element->setAttribute("mode", teModeToStringKey(edgeRFMarker->mode));
        element->setAttribute("height", edgeRFMarker->height.text);
        break;
    case PhysicFieldBC_RF_MatchedBoundary:
        element->setAttribute("height", edgeRFMarker->height.text);
        break;
    default:
        std::cerr << tr("Boundary type '%1' doesn't exists.").arg(element->attribute("type")).toStdString() << endl;
        break;
    }
}

void HermesRF::readLabelMarkerFromDomElement(QDomElement *element)
{
    Util::scene()->addLabelMarker(new SceneLabelRFMarker(element->attribute("name"),
                                                         Value(element->attribute("permittivity", "1")),
                                                         Value(element->attribute("permeability", "1")),
                                                         Value(element->attribute("conductivity", "0")),
                                                         Value(element->attribute("current_density_real", "0")),
                                                         Value(element->attribute("current_density_imag", "0"))));
}

void HermesRF::writeLabelMarkerToDomElement(QDomElement *element, SceneLabelMarker *marker)
{
    SceneLabelRFMarker *labelRFMarker = dynamic_cast<SceneLabelRFMarker *>(marker);

    element->setAttribute("permittivity", labelRFMarker->permittivity.text);
    element->setAttribute("permeability", labelRFMarker->permeability.text);
    element->setAttribute("conductivity", labelRFMarker->conductivity.text);
    element->setAttribute("current_density_real", labelRFMarker->current_density_real.text);
    element->setAttribute("current_density_imag", labelRFMarker->current_density_imag.text);
}

LocalPointValue *HermesRF::localPointValue(const Point &point)
{
    return new LocalPointValueRF(point);
}

QStringList HermesRF::localPointValueHeader()
{
    QStringList headers;
    headers << "X" << "Y" << "E_real" << "E_imag" << "E";
    return QStringList(headers);
}

SurfaceIntegralValue *HermesRF::surfaceIntegralValue()
{
    return new SurfaceIntegralValueRF();
}

QStringList HermesRF::surfaceIntegralValueHeader()
{
    QStringList headers;
    headers << "l" << "S";
    return QStringList(headers);
}

VolumeIntegralValue *HermesRF::volumeIntegralValue()
{
    return new VolumeIntegralValueRF();
}

QStringList HermesRF::volumeIntegralValueHeader()
{
    QStringList headers;
    headers << "V" << "S";
    return QStringList(headers);
}

SceneEdgeMarker *HermesRF::newEdgeMarker()
{
    return new SceneEdgeRFMarker(tr("new boundary"),
                                 PhysicFieldBC_RF_ElectricField,
                                 Value("0"),
                                 Value("0"));
}

SceneEdgeMarker *HermesRF::newEdgeMarker(PyObject *self, PyObject *args)
{
    // FIXME - parse
    double value1, value2, height;
    char *name, *type, *mode;
    if (PyArg_ParseTuple(args, "sssdd|d", &name, &type, &mode, &value1, &value2, &height))
    {
        // check name
        if (Util::scene()->getEdgeMarker(name)) return NULL;

        if (physicFieldBCFromStringKey(type) == PhysicFieldBC_RF_ElectricField ||
                physicFieldBCFromStringKey(type) == PhysicFieldBC_RF_MagneticField)
            return new SceneEdgeRFMarker(name,
                                         physicFieldBCFromStringKey(type),
                                         Value(QString::number(value1)),
                                         Value(QString::number(value2)));
        if (physicFieldBCFromStringKey(type) == PhysicFieldBC_RF_Port)
            return new SceneEdgeRFMarker(name,
                                         physicFieldBCFromStringKey(type),
                                         teModeFromStringKey(mode),
                                         Value(QString::number(value1)),
                                         Value(QString::number(value2)),
                                         Value(QString::number(height)));
        if (physicFieldBCFromStringKey(type) == (PhysicFieldBC_RF_MatchedBoundary))
            return new SceneEdgeRFMarker(name,
                                         physicFieldBCFromStringKey(type),
                                         Value(QString::number(height)));

    }

    return NULL;
}

SceneEdgeMarker *HermesRF::modifyEdgeMarker(PyObject *self, PyObject *args)
{
    // FIXME - parse
    double value1, value2, height;
    char *name, *type, *mode;
    if (PyArg_ParseTuple(args, "sssdd|d", &name, &type, &mode, &value1, &value2, &height))
    {
        if (SceneEdgeRFMarker *marker = dynamic_cast<SceneEdgeRFMarker *>(Util::scene()->getEdgeMarker(name)))
        {
            if (physicFieldBCFromStringKey(type))
            {
                marker->type = physicFieldBCFromStringKey(type);
                return marker;
            }
            else
            {
                PyErr_SetString(PyExc_RuntimeError, QObject::tr("Boundary type '%1' is not supported.").arg(type).toStdString().c_str());
                return NULL;
            }

            if (physicFieldBCFromStringKey(type) == PhysicFieldBC_RF_ElectricField ||
                    physicFieldBCFromStringKey(type) == PhysicFieldBC_RF_MagneticField ||
                    physicFieldBCFromStringKey(type) == PhysicFieldBC_RF_Port)
            {
                marker->value_real = Value(QString::number(value1));
                marker->value_imag = Value(QString::number(value2));
                marker->mode = teModeFromStringKey(mode);
            }
            if (physicFieldBCFromStringKey(type) == PhysicFieldBC_RF_MatchedBoundary)
            {
                marker->height = Value(QString::number(height));

            }
        }
        else
        {
            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Boundary marker with name '%1' doesn't exists.").arg(name).toStdString().c_str());
            return NULL;
        }
    }

    return NULL;
}

SceneLabelMarker *HermesRF::newLabelMarker()
{
    return new SceneLabelRFMarker(tr("new material"),
                                  Value("1"),
                                  Value("1"),
                                  Value("0"),
                                  Value("0"),
                                  Value("0"));
}

SceneLabelMarker *HermesRF::newLabelMarker(PyObject *self, PyObject *args)
{
    double permittivity, permeability, conductivity, current_density_real, current_density_imag;
    char *name;
    if (PyArg_ParseTuple(args, "sddddd", &name, &permittivity, &permeability, &conductivity, &current_density_real, &current_density_imag))
    {
        // check name
        if (Util::scene()->getLabelMarker(name)) return NULL;

        return new SceneLabelRFMarker(name,
                                      Value(QString::number(permittivity)),
                                      Value(QString::number(permeability)),
                                      Value(QString::number(conductivity)),
                                      Value(QString::number(current_density_real)),
                                      Value(QString::number(current_density_imag)));
    }

    return NULL;
}

SceneLabelMarker *HermesRF::modifyLabelMarker(PyObject *self, PyObject *args)
{
    double permittivity, permeability, conductivity, current_density_real, current_density_imag;
    char *name;
    if (PyArg_ParseTuple(args, "sddddd", &name, &permittivity, &permeability, &conductivity, &current_density_real, &current_density_imag))
    {
        if (SceneLabelRFMarker *marker = dynamic_cast<SceneLabelRFMarker *>(Util::scene()->getLabelMarker(name)))
        {
            marker->permittivity = Value(QString::number(permittivity));
            marker->permeability = Value(QString::number(permeability));
            marker->conductivity = Value(QString::number(conductivity));
            marker->current_density_real = Value(QString::number(current_density_real));
            marker->current_density_imag = Value(QString::number(current_density_imag));
            return marker;
        }
        else
        {
            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Label marker with name '%1' doesn't exists.").arg(name).toStdString().c_str());
            return NULL;
        }
    }

    return NULL;
}

void HermesRF::fillComboBoxScalarVariable(QComboBox *cmbFieldVariable)
{
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_ElectricField), PhysicFieldVariable_RF_ElectricField);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_ElectricFieldReal), PhysicFieldVariable_RF_ElectricFieldReal);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_ElectricFieldImag), PhysicFieldVariable_RF_ElectricFieldImag);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_MagneticField), PhysicFieldVariable_RF_MagneticField);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_MagneticFieldRealX), PhysicFieldVariable_RF_MagneticFieldRealX);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_MagneticFieldImagX), PhysicFieldVariable_RF_MagneticFieldImagX);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_MagneticFieldRealY), PhysicFieldVariable_RF_MagneticFieldRealY);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_MagneticFieldImagY), PhysicFieldVariable_RF_MagneticFieldImagY);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_MagneticFluxDensity), PhysicFieldVariable_RF_MagneticFluxDensity);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_MagneticFluxDensityRealX), PhysicFieldVariable_RF_MagneticFluxDensityRealX);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_MagneticFluxDensityImagX), PhysicFieldVariable_RF_MagneticFluxDensityImagX);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_MagneticFluxDensityRealY), PhysicFieldVariable_RF_MagneticFluxDensityRealY);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_MagneticFluxDensityImagY), PhysicFieldVariable_RF_MagneticFluxDensityImagY);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_PoyntingVector), PhysicFieldVariable_RF_PoyntingVector);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_PoyntingVectorReal), PhysicFieldVariable_RF_PoyntingVectorReal);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_PoyntingVectorImag), PhysicFieldVariable_RF_PoyntingVectorImag);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_PowerLosses), PhysicFieldVariable_RF_PowerLosses);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_Permittivity), PhysicFieldVariable_RF_Permittivity);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_Permeability), PhysicFieldVariable_RF_Permeability);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_Conductivity), PhysicFieldVariable_RF_Conductivity);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_CurrentDensityReal), PhysicFieldVariable_RF_CurrentDensityReal);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_CurrentDensityImag), PhysicFieldVariable_RF_CurrentDensityImag);
}

void HermesRF::fillComboBoxVectorVariable(QComboBox *cmbFieldVariable)
{
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_ElectricFieldReal), PhysicFieldVariable_RF_ElectricFieldReal);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_RF_MagneticFieldRealX), PhysicFieldVariable_RF_MagneticFieldRealX);
}


void HermesRF::showLocalValue(QTreeWidget *trvWidget, LocalPointValue *localPointValue)
{
    LocalPointValueRF *localPointValueRF = dynamic_cast<LocalPointValueRF *>(localPointValue);

    // rf field
    QTreeWidgetItem *rfNode = new QTreeWidgetItem(trvWidget);
    rfNode->setText(0, tr("RF field"));
    rfNode->setExpanded(true);

        // material
        addTreeWidgetItemValue(rfNode, tr("Permittivity:"), QString("%1").arg(localPointValueRF->permittivity, 0, 'f', 2), "");
        addTreeWidgetItemValue(rfNode, tr("Permeability:"), QString("%1").arg(localPointValueRF->permeability, 0, 'f', 2), "");
        addTreeWidgetItemValue(rfNode, tr("Conductivity:"), QString("%1").arg(localPointValueRF->conductivity, 0, 'f', 2), "");

        // Electric Field
        QTreeWidgetItem *itemElectricField = new QTreeWidgetItem(rfNode);
        itemElectricField->setText(0, tr("Electric field"));
        itemElectricField->setExpanded(true);

        addTreeWidgetItemValue(itemElectricField, tr("real:"), QString("%1").arg(localPointValueRF->electric_field_real, 0, 'e', 3), "V/m");
        addTreeWidgetItemValue(itemElectricField, tr("imag:"), QString("%1").arg(localPointValueRF->electric_field_imag, 0, 'e', 3), "V/m");
        addTreeWidgetItemValue(itemElectricField, tr("magnitude:"), QString("%1").arg(sqrt(sqr(localPointValueRF->electric_field_real) + sqr(localPointValueRF->electric_field_imag)), 0, 'e', 3), "V/m");

        // Magnetic Field
        QTreeWidgetItem *itemMagneticField = new QTreeWidgetItem(rfNode);
        itemMagneticField->setText(0, tr("Magnetic field"));
        itemMagneticField->setExpanded(true);

        addTreeWidgetItemValue(itemMagneticField, tr("real X:"), QString("%1").arg(localPointValueRF->magnetic_field_realX, 0, 'e', 3), "A/m");
        addTreeWidgetItemValue(itemMagneticField, tr("imag X:"), QString("%1").arg(localPointValueRF->magnetic_field_imagX, 0, 'e', 3), "A/m");
        addTreeWidgetItemValue(itemMagneticField, tr("real Y:"), QString("%1").arg(localPointValueRF->magnetic_field_realY, 0, 'e', 3), "A/m");
        addTreeWidgetItemValue(itemMagneticField, tr("imag Y:"), QString("%1").arg(localPointValueRF->magnetic_field_imagY, 0, 'e', 3), "A/m");
        addTreeWidgetItemValue(itemMagneticField, tr("magnitude:"), QString("%1").arg(sqrt(sqr(localPointValueRF->magnetic_field_realX) + sqr(localPointValueRF->magnetic_field_imagX) + sqr(localPointValueRF->magnetic_field_realY) + sqr(localPointValueRF->magnetic_field_imagY)), 0, 'e', 3), "A/m");

        // Magnetic Flux Density
        QTreeWidgetItem *itemFluxDensity = new QTreeWidgetItem(rfNode);
        itemFluxDensity->setText(0, tr("Magnetic flux density"));
        itemFluxDensity->setExpanded(true);

        addTreeWidgetItemValue(itemFluxDensity, tr("real X:"), QString("%1").arg(localPointValueRF->flux_density_realX, 0, 'e', 3), "T");
        addTreeWidgetItemValue(itemFluxDensity, tr("imag X:"), QString("%1").arg(localPointValueRF->flux_density_imagX, 0, 'e', 3), "T");
        addTreeWidgetItemValue(itemFluxDensity, tr("real Y:"), QString("%1").arg(localPointValueRF->flux_density_realY, 0, 'e', 3), "T");
        addTreeWidgetItemValue(itemFluxDensity, tr("imag Y:"), QString("%1").arg(localPointValueRF->flux_density_imagY, 0, 'e', 3), "T");
        addTreeWidgetItemValue(itemFluxDensity, tr("magnitude:"), QString("%1").arg(sqrt(sqr(localPointValueRF->flux_density_realX) + sqr(localPointValueRF->flux_density_imagX) + sqr(localPointValueRF->flux_density_realY) + sqr(localPointValueRF->flux_density_imagY)), 0, 'e', 3), "T");

        // Poynting vector
        QTreeWidgetItem *itemPoyntingVector = new QTreeWidgetItem(rfNode);
        itemPoyntingVector->setText(0, tr("Poynting vector"));
        itemPoyntingVector->setExpanded(true);

        addTreeWidgetItemValue(itemPoyntingVector, tr("real:"), QString("%1").arg(localPointValueRF->poynting_vector_real, 0, 'e', 3), "W/m2");
        addTreeWidgetItemValue(itemPoyntingVector, tr("imag:"), QString("%1").arg(localPointValueRF->poynting_vector_imag, 0, 'e', 3), "W/m2");
        addTreeWidgetItemValue(itemPoyntingVector, tr("magnitude:"), QString("%1").arg(sqrt(sqr(localPointValueRF->poynting_vector_real) + sqr(localPointValueRF->poynting_vector_imag)), 0, 'e', 3), "W/m2");

        // Current Density
        QTreeWidgetItem *itemCurrentDensity = new QTreeWidgetItem(rfNode);
        itemCurrentDensity->setText(0, tr("Current density"));
        itemCurrentDensity->setExpanded(true);

        addTreeWidgetItemValue(itemCurrentDensity, tr("real:"), QString("%1").arg(localPointValueRF->current_density_real, 0, 'f', 2), "A/m2");
        addTreeWidgetItemValue(itemCurrentDensity, tr("imag:"), QString("%1").arg(localPointValueRF->current_density_imag, 0, 'f', 2), "A/m2");
        addTreeWidgetItemValue(itemCurrentDensity, tr("magnitude:"), QString("%1").arg(sqrt(sqr(localPointValueRF->current_density_real) + sqr(localPointValueRF->current_density_imag)), 0, 'f', 2), "A/m2");
}

void HermesRF::showSurfaceIntegralValue(QTreeWidget *trvWidget, SurfaceIntegralValue *surfaceIntegralValue)
{
    SurfaceIntegralValueRF *surfaceIntegralValueRF = dynamic_cast<SurfaceIntegralValueRF *>(surfaceIntegralValue);

    QTreeWidgetItem *magneticNode = new QTreeWidgetItem(trvWidget);
    magneticNode->setText(0, tr("RF field"));
    magneticNode->setExpanded(true);
}

void HermesRF::showVolumeIntegralValue(QTreeWidget *trvWidget, VolumeIntegralValue *volumeIntegralValue)
{
    VolumeIntegralValueRF *volumeIntegralValueRF = dynamic_cast<VolumeIntegralValueRF *>(volumeIntegralValue);

    // harmonic
    QTreeWidgetItem *magneticNode = new QTreeWidgetItem(trvWidget);
    magneticNode->setText(0, tr("RF field"));
    magneticNode->setExpanded(true);
}

ViewScalarFilter *HermesRF::viewScalarFilter(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
{
    Solution *sln1 = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep() * Util::scene()->problemInfo()->hermes()->numberOfSolution());

    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
    {
        Solution *sln2 = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep() * Util::scene()->problemInfo()->hermes()->numberOfSolution() + 1);
        return new ViewScalarFilterRF(Hermes::vector<MeshFunction *>(sln1, sln2),
                                      physicFieldVariable,
                                      physicFieldVariableComp);
    }
}

QList<SolutionArray *> HermesRF::solve(ProgressItemSolve *progressItemSolve)
{
    // edge markers
    BCTypes bcTypesReal, bcTypesImag;
    BCValues bcValuesReal, bcValuesImag;

    rfEdge = new RFEdge[Util::scene()->edges.count()+1];
    rfEdge[0].type = PhysicFieldBC_None;
    rfEdge[0].value_real = 0.0;
    rfEdge[0].value_imag = 0.0;
    rfEdge[0].power = 0.0;
    rfEdge[0].phase = 0.0;
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        if (Util::scene()->edgeMarkers.indexOf(Util::scene()->edges[i]->marker) == 0)
        {
            rfEdge[i+1].type = PhysicFieldBC_None;
            rfEdge[i+1].value_real = 0.0;
            rfEdge[i+1].value_imag = 0.0;
        }
        else
        {
            SceneEdgeRFMarker *edgeRFMarker = dynamic_cast<SceneEdgeRFMarker *>(Util::scene()->edges[i]->marker);

            // evaluate script
            if (!edgeRFMarker->value_real.evaluate()) return QList<SolutionArray *>();
            if (!edgeRFMarker->value_imag.evaluate()) return QList<SolutionArray *>();

            rfEdge[i+1].type = edgeRFMarker->type;
            rfEdge[i+1].mode = edgeRFMarker->mode;
            rfEdge[i+1].value_real = edgeRFMarker->value_real.number;
            rfEdge[i+1].value_imag = edgeRFMarker->value_imag.number;
            rfEdge[i+1].power = edgeRFMarker->power.number;
            rfEdge[i+1].phase = edgeRFMarker->phase.number;
            rfEdge[i+1].height = edgeRFMarker->height.number;
            rfEdge[i+1].start = Util::scene()->edges[i]->nodeStart->point;
            rfEdge[i+1].end = Util::scene()->edges[i]->nodeEnd->point;
            rfEdge[i+1].angle = Util::scene()->edges[i]->angle;

            switch (edgeRFMarker->type)
            {
            case PhysicFieldBC_None:
                bcTypesReal.add_bc_none(i+1);
                bcTypesImag.add_bc_none(i+1);
                break;
            case PhysicFieldBC_RF_ElectricField:
                bcTypesReal.add_bc_dirichlet(i+1);
                bcTypesImag.add_bc_dirichlet(i+1);
                bcValuesReal.add_const(i+1, edgeRFMarker->value_real.number);
                bcValuesImag.add_const(i+1, edgeRFMarker->value_imag.number);
                break;
            case PhysicFieldBC_RF_MagneticField:
                bcTypesReal.add_bc_neumann(i+1);
                bcTypesImag.add_bc_neumann(i+1);
                break;
            case PhysicFieldBC_RF_MatchedBoundary:
                bcTypesReal.add_bc_newton(i+1);
                bcTypesImag.add_bc_newton(i+1);
                break;
            case PhysicFieldBC_RF_Port:
                bcTypesReal.add_bc_newton(i+1);
                bcTypesImag.add_bc_newton(i+1);
                break;
            }
        }
    }

    // label markers
    rfLabel = new RFLabel[Util::scene()->labels.count()];
    for (int i = 0; i<Util::scene()->labels.count(); i++)
    {
        if (Util::scene()->labelMarkers.indexOf(Util::scene()->labels[i]->marker) == 0)
        {
        }
        else
        {
            SceneLabelRFMarker *labelRFMarker = dynamic_cast<SceneLabelRFMarker *>(Util::scene()->labels[i]->marker);

            // evaluate script
            if (!labelRFMarker->permittivity.evaluate()) return QList<SolutionArray *>();
            if (!labelRFMarker->permeability.evaluate()) return QList<SolutionArray *>();
            if (!labelRFMarker->conductivity.evaluate()) return QList<SolutionArray *>();
            if (!labelRFMarker->current_density_real.evaluate()) return QList<SolutionArray *>();
            if (!labelRFMarker->current_density_imag.evaluate()) return QList<SolutionArray *>();

            rfLabel[i].permittivity = labelRFMarker->permittivity.number;
            rfLabel[i].permeability = labelRFMarker->permeability.number;
            rfLabel[i].conductivity = labelRFMarker->conductivity.number;
            rfLabel[i].current_density_real = labelRFMarker->current_density_real.number;
            rfLabel[i].current_density_imag = labelRFMarker->current_density_imag.number;
        }
    }

    QList<SolutionArray *> solutionArrayList = solveSolutioArray(progressItemSolve,
                                                                 Hermes::vector<BCTypes *>(&bcTypesReal, &bcTypesImag),
                                                                 Hermes::vector<BCValues *>(&bcValuesReal, &bcValuesImag),
                                                                 callbackRFWeakForm);

    delete [] rfEdge;
    delete [] rfLabel;

    return solutionArrayList;
}

// ****************************************************************************************************************

LocalPointValueRF::LocalPointValueRF(const Point &point) : LocalPointValue(point)
{
    permittivity = 0;
    permeability = 0;
    conductivity = 0;
    current_density_real = 0;
    current_density_imag = 0;

    electric_field_real = 0;
    electric_field_imag = 0;
    magnetic_field_realX = 0;
    magnetic_field_imagX = 0;
    magnetic_field_realY = 0;
    magnetic_field_imagY = 0;
    flux_density_realX = 0;
    flux_density_imagX = 0;
    flux_density_realY = 0;
    flux_density_imagY = 0;
    poynting_vector_real = 0;
    poynting_vector_imag = 0;

    if (Util::scene()->sceneSolution()->isSolved())
    {
        // value real
        PointValue valueReal = PointValue(value, derivative, labelMarker);

        SceneLabelRFMarker *marker = dynamic_cast<SceneLabelRFMarker *>(valueReal.marker);
        // solution
        if (marker != NULL)
        {
            if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
            {
                Solution *sln2 = Util::scene()->sceneSolution()->sln(1);

                double w = 2 * M_PI * frequency;
                double mu = marker->permeability.number * MU0;
                // value imag
                PointValue valueImag = pointValue(sln2, point);
                // derivative
                Point derReal = valueReal.derivative;
                Point derImag = valueImag.derivative;

                // Electric Field
                electric_field_real = valueReal.value;
                electric_field_imag = valueImag.value;

                // Magnetic Field
                magnetic_field_realX = -(1/(w*mu))*derImag.y;
                magnetic_field_imagX = (1/(w*mu))*derReal.y;
                magnetic_field_realY = (1/(w*mu))*derImag.x;
                magnetic_field_imagY = -(1/(w*mu))*derReal.x;

                // Magnetic Flux Density
                flux_density_realX = -(1/w)*derImag.y;
                flux_density_imagX = (1/w)*derReal.y;
                flux_density_realY = (1/w)*derImag.x;
                flux_density_imagY = -(1/w)*derReal.x;

                // Poynting vector
                poynting_vector_real = valueReal.value * sqrt(sqr(-(1/(w*mu))*derImag.y) + sqr((1/(w*mu))*derImag.x));
                poynting_vector_imag = valueImag.value * sqrt(sqr((1/(w*mu))*derReal.y) + sqr(-(1/(w*mu))*derReal.x));

                // material + current density
                permittivity = marker->permittivity.number;
                permeability = marker->permeability.number;
                conductivity = marker->conductivity.number;
                current_density_real = marker->current_density_real.number;
                current_density_imag = marker->current_density_imag.number;
            }
        }
    }
}

double LocalPointValueRF::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
{
    switch (physicFieldVariable)
    {
    case PhysicFieldVariable_RF_ElectricField:
    {
        return sqrt(sqr(electric_field_real) + sqr(electric_field_imag));
    }
        break;
    case PhysicFieldVariable_RF_ElectricFieldReal:
    {
        return electric_field_real;
    }
        break;
    case PhysicFieldVariable_RF_ElectricFieldImag:
    {
        return electric_field_imag;
    }
        break;
    case PhysicFieldVariable_RF_MagneticField:
    {
        return sqrt(sqr(magnetic_field_realX) + sqr(magnetic_field_imagX) + sqr(magnetic_field_realY) + sqr(magnetic_field_imagY));
    }
        break;
    case PhysicFieldVariable_RF_MagneticFieldRealX:
    {
        return magnetic_field_realX;
    }
        break;
    case PhysicFieldVariable_RF_MagneticFieldImagX:
    {
        return magnetic_field_imagX;
    }
        break;
    case PhysicFieldVariable_RF_MagneticFieldRealY:
    {
        return magnetic_field_realY;
    }
        break;
    case PhysicFieldVariable_RF_MagneticFieldImagY:
    {
        return magnetic_field_imagY;
    }
            break;
    case PhysicFieldVariable_RF_MagneticFluxDensity:
    {
        return sqrt(sqr(flux_density_realX) + sqr(flux_density_imagX) + sqr(flux_density_realY) + sqr(flux_density_imagY));
    }
        break;
    case PhysicFieldVariable_RF_MagneticFluxDensityRealX:
    {
        return flux_density_realX;
    }
        break;
    case PhysicFieldVariable_RF_MagneticFluxDensityImagX:
    {
        return flux_density_imagX;
    }
        break;
    case PhysicFieldVariable_RF_MagneticFluxDensityRealY:
    {
        return flux_density_realY;
    }
        break;
    case PhysicFieldVariable_RF_MagneticFluxDensityImagY:
    {
        return flux_density_imagY;
    }
        break;
    case PhysicFieldVariable_RF_PoyntingVector:
    {
        return sqrt(sqr(poynting_vector_real) + sqr(poynting_vector_imag));
    }
        break;
    case PhysicFieldVariable_RF_PoyntingVectorReal:
    {
        return poynting_vector_real;
    }
        break;
    case PhysicFieldVariable_RF_PoyntingVectorImag:
    {
        return poynting_vector_imag;
    }
        break;
    case PhysicFieldVariable_RF_Permittivity:
    {
        return permittivity;
    }
        break;
    case PhysicFieldVariable_RF_Permeability:
    {
        return permeability;
    }
        break;
    case PhysicFieldVariable_RF_Conductivity:
    {
        return conductivity;
    }
        break;
    case PhysicFieldVariable_RF_CurrentDensityReal:
    {
        return current_density_real;
    }
        break;
    case PhysicFieldVariable_RF_CurrentDensityImag:
    {
        return current_density_imag;
    }
        break;
    default:
        cerr << "Physical field variable '" + physicFieldVariableString(physicFieldVariable).toStdString() + "' is not implemented. LocalPointValueRF::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)" << endl;
        throw;
        break;
    }
}

QStringList LocalPointValueRF::variables()
{
    QStringList row;
    row <<  QString("%1").arg(point.x, 0, 'e', 5) <<
           QString("%1").arg(point.y, 0, 'e', 5) <<
           QString("%1").arg(electric_field_real, 0, 'e', 5) <<
           QString("%1").arg(electric_field_imag, 0, 'e', 5) <<
           QString("%1").arg(sqrt(sqr(electric_field_real) + sqr(electric_field_imag)), 0, 'e', 5) <<
           QString("%1").arg(magnetic_field_realX, 0, 'e', 5) <<
           QString("%1").arg(magnetic_field_imagX, 0, 'e', 5) <<
           QString("%1").arg(magnetic_field_realY, 0, 'e', 5) <<
           QString("%1").arg(magnetic_field_imagY, 0, 'e', 5) <<
           QString("%1").arg(sqrt(sqr(magnetic_field_realX) + sqr(magnetic_field_imagX) + sqr(magnetic_field_realY) + sqr(magnetic_field_imagY)), 0, 'e', 5) <<
           QString("%1").arg(flux_density_realX, 0, 'e', 5) <<
           QString("%1").arg(flux_density_imagX, 0, 'e', 5) <<
           QString("%1").arg(flux_density_realY, 0, 'e', 5) <<
           QString("%1").arg(flux_density_imagY, 0, 'e', 5) <<
           QString("%1").arg(sqrt(sqr(flux_density_realX) + sqr(flux_density_imagX) + sqr(flux_density_realY) + sqr(flux_density_imagY)), 0, 'e', 5) <<
           QString("%1").arg(poynting_vector_real, 0, 'e', 5) <<
           QString("%1").arg(poynting_vector_imag, 0, 'e', 5) <<
           QString("%1").arg(sqrt(sqr(poynting_vector_real) + sqr(poynting_vector_imag)), 0, 'e', 5) <<
           QString("%1").arg(permittivity, 0, 'e', 5) <<
           QString("%1").arg(permeability, 0, 'f', 3) <<
           QString("%1").arg(conductivity, 0, 'e', 5) <<
           QString("%1").arg(current_density_real, 0, 'e', 5) <<
           QString("%1").arg(current_density_imag, 0, 'e', 5);

    return QStringList(row);
}

// ****************************************************************************************************************

SurfaceIntegralValueRF::SurfaceIntegralValueRF() : SurfaceIntegralValue()
{
    calculate();
}

void SurfaceIntegralValueRF::calculateVariables(int i)
{
    SceneLabelRFMarker *marker = dynamic_cast<SceneLabelRFMarker *>(Util::scene()->labels[e->marker]->marker);
}

QStringList SurfaceIntegralValueRF::variables()
{
    QStringList row;
    row <<  QString("%1").arg(length, 0, 'e', 5) <<
           QString("%1").arg(surface, 0, 'e', 5);
    return QStringList(row);
}


// ****************************************************************************************************************

VolumeIntegralValueRF::VolumeIntegralValueRF() : VolumeIntegralValue()
{
    calculate();
}

void VolumeIntegralValueRF::calculateVariables(int i)
{
    SceneLabelRFMarker *marker = dynamic_cast<SceneLabelRFMarker *>(Util::scene()->labels[e->marker]->marker);
}

void VolumeIntegralValueRF::initSolutions()
{
    sln1 = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep() * Util::scene()->problemInfo()->hermes()->numberOfSolution());
    sln2 = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep() * Util::scene()->problemInfo()->hermes()->numberOfSolution() + 1);
}

QStringList VolumeIntegralValueRF::variables()
{
    QStringList row;
    row <<  QString("%1").arg(volume, 0, 'e', 5) <<
           QString("%1").arg(crossSection, 0, 'e', 5);
    return QStringList(row);
}

// *************************************************************************************************************************************

void ViewScalarFilterRF::calculateVariable(int i)
{
    switch (m_physicFieldVariable)
    {
    case PhysicFieldVariable_RF_ElectricField:
    {
        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        {
            node->values[0][0][i] = sqrt(sqr(value1[i]) + sqr(value2[i]));
        }
        else
        {
            node->values[0][0][i] = sqrt(sqr(value1[i]) + sqr(value2[i])) * x[i];
        }
    }
        break;
    case PhysicFieldVariable_RF_ElectricFieldReal:
    {
        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        {
            node->values[0][0][i] = value1[i];
        }
        else
        {
            node->values[0][0][i] = - value1[i] * x[i];
        }
    }
        break;
    case PhysicFieldVariable_RF_ElectricFieldImag:
    {
        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        {
            node->values[0][0][i] = value2[i];
        }
        else
        {
            node->values[0][0][i] = - value2[i] * x[i];
        }
    }
        break;
    case PhysicFieldVariable_RF_MagneticField:
        {
            SceneLabelRFMarker *marker = dynamic_cast<SceneLabelRFMarker *>(labelMarker);

            double mu = marker->permeability.number * MU0;
            double w = 2 * M_PI * frequency;

            if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
            {
                node->values[0][0][i] = sqrt(sqr(-(1/(w*mu)) * dudy2[i]) + sqr((1/(w*mu)) * dudy1[i]) + sqr((1/(w*mu)) * dudx2[i]) + sqr(-(1/(w*mu)) * dudx1[i]));
            }
            else
            {
                node->values[0][0][i] = sqrt(sqr(-(1/(w*mu)) * dudy2[i]) + sqr((1/(w*mu)) * dudy1[i]) + sqr((1/(w*mu)) * dudx2[i]) + sqr(-(1/(w*mu)) * dudx1[i])) * x[i];
            }
        }
        break;
    case PhysicFieldVariable_RF_MagneticFieldRealX:
    {
            SceneLabelRFMarker *marker = dynamic_cast<SceneLabelRFMarker *>(labelMarker);

            double mu = marker->permeability.number * MU0;
            double w = 2 * M_PI * frequency;

            if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
            {
                node->values[0][0][i] = -(1/(w*mu)) * dudy2[i];
            }
            else
            {
                node->values[0][0][i] = -(1/(w*mu)) * dudy2[i] * x[i];
            }
        }

        break;
    case PhysicFieldVariable_RF_MagneticFieldImagX:
        {
            SceneLabelRFMarker *marker = dynamic_cast<SceneLabelRFMarker *>(labelMarker);

            double mu = marker->permeability.number * MU0;
            double w = 2 * M_PI * frequency;

            if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
            {
                node->values[0][0][i] = (1/(w*mu)) * dudy1[i];
            }
            else
            {
                node->values[0][0][i] = (1/(w*mu)) * dudy1[i] * x[i];
            }
        }
        break;
    case PhysicFieldVariable_RF_MagneticFieldRealY:
        {
            SceneLabelRFMarker *marker = dynamic_cast<SceneLabelRFMarker *>(labelMarker);

            double mu = marker->permeability.number * MU0;
            double w = 2 * M_PI * frequency;

            if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
            {
                node->values[0][0][i] = (1/(w*mu)) * dudx2[i];
            }
            else
            {
                node->values[0][0][i] = (1/(w*mu)) * dudx2[i] * x[i];
            }
        }
        break;
    case PhysicFieldVariable_RF_MagneticFieldImagY:
        {
            SceneLabelRFMarker *marker = dynamic_cast<SceneLabelRFMarker *>(labelMarker);

            double mu = marker->permeability.number * MU0;
            double w = 2 * M_PI * frequency;

            if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
            {
                node->values[0][0][i] = -(1/(w*mu)) * dudx1[i];
            }
            else
            {
                node->values[0][0][i] = -(1/(w*mu)) * dudx1[i] * x[i];
            }
        }
        break;
    case PhysicFieldVariable_RF_MagneticFluxDensity:
        {
            double w = 2 * M_PI * frequency;

            if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
            {
                node->values[0][0][i] = sqrt(sqr(-(1/(w)) * dudy2[i]) + sqr((1/(w)) * dudy1[i]) + sqr((1/(w)) * dudx2[i]) + sqr(-(1/(w)) * dudx1[i]));
            }
            else
            {
                node->values[0][0][i] = sqrt(sqr(-(1/(w)) * dudy2[i]) + sqr((1/(w)) * dudy1[i]) + sqr((1/(w)) * dudx2[i]) + sqr(-(1/(w)) * dudx1[i]));
            }
        }
        break;
    case PhysicFieldVariable_RF_MagneticFluxDensityRealX:
        {
            double w = 2 * M_PI * frequency;

            if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
            {
                node->values[0][0][i] = -(1/(w)) * dudy2[i];
            }
            else
            {
                node->values[0][0][i] = -(1/(w)) * dudy2[i] * x[i];
            }
        }
        break;
    case PhysicFieldVariable_RF_MagneticFluxDensityImagX:
        {
            double w = 2 * M_PI * frequency;

            if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
            {
                node->values[0][0][i] = (1/(w)) * dudy1[i];
            }
            else
            {
                node->values[0][0][i] = (1/(w)) * dudy1[i] * x[i];
            }
        }
        break;
    case PhysicFieldVariable_RF_MagneticFluxDensityRealY:
        {
            double w = 2 * M_PI * frequency;

            if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
            {
                node->values[0][0][i] = (1/(w)) * dudx2[i];
            }
            else
            {
                node->values[0][0][i] = (1/(w)) * dudx2[i] * x[i];
            }
        }
        break;
    case PhysicFieldVariable_RF_MagneticFluxDensityImagY:
        {
            double w = 2 * M_PI * frequency;

            if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
            {
                node->values[0][0][i] = -(1/(w)) * dudx1[i];
            }
            else
            {
                node->values[0][0][i] = -(1/(w)) * dudx1[i] * x[i];
            }
        }
        break;
    case PhysicFieldVariable_RF_PoyntingVector:
    {
        SceneLabelRFMarker *marker = dynamic_cast<SceneLabelRFMarker *>(labelMarker);

        double mu = marker->permeability.number * MU0;
        double w = 2 * M_PI * frequency;

        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        {
            node->values[0][0][i] = sqrt(sqr(value1[i] * sqrt(sqr(-(1/(w*mu)) * dudy2[i]) + sqr((1/(w*mu)) * dudx2[i]))) + sqr(value2[i] * sqrt(sqr((1/(w*mu)) * dudy1[i]) + sqr(-(1/(w*mu)) * dudx1[i]))));
        }
        else
        {
            node->values[0][0][i] = sqrt(sqr(value1[i]) + sqr(value2[i])) * sqrt(sqr(-(1/(w*mu)) * dudy2[i]) + sqr((1/(w*mu)) * dudy1[i]) + sqr((1/(w*mu)) * dudx2[i]) + sqr(-(1/(w*mu)) * dudx1[i])) * x[i];
        }
    }
        break;
    case PhysicFieldVariable_RF_PoyntingVectorReal:
    {
        SceneLabelRFMarker *marker = dynamic_cast<SceneLabelRFMarker *>(labelMarker);

        double mu = marker->permeability.number * MU0;
        double w = 2 * M_PI * frequency;

        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        {
            node->values[0][0][i] = value1[i] * sqrt(sqr(-(1/(w*mu)) * dudy2[i]) + sqr((1/(w*mu)) * dudx2[i]));
        }
        else
        {
            node->values[0][0][i] = value1[i] * sqrt(sqr(-(1/(w*mu)) * dudy2[i]) + sqr((1/(w*mu)) * dudx2[i])) * x[i];
        }
    }
        break;
    case PhysicFieldVariable_RF_PoyntingVectorImag:
    {
        SceneLabelRFMarker *marker = dynamic_cast<SceneLabelRFMarker *>(labelMarker);

        double mu = marker->permeability.number * MU0;
        double w = 2 * M_PI * frequency;

        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        {
            node->values[0][0][i] = value2[i] * sqrt(sqr((1/(w*mu)) * dudy1[i]) + sqr(-(1/(w*mu)) * dudx1[i]));
        }
        else
        {
            node->values[0][0][i] = value2[i] * sqrt(sqr((1/(w*mu)) * dudy1[i]) + sqr(-(1/(w*mu)) * dudx1[i])) * x[i];
        }
    }
        break;
    case PhysicFieldVariable_RF_PowerLosses:
    {
        SceneLabelRFMarker *marker = dynamic_cast<SceneLabelRFMarker *>(labelMarker);
        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        {
            node->values[0][0][i] = (sqr(value1[i]) + sqr(value2[i])) * (marker->conductivity.number);
        }
        else
        {
            node->values[0][0][i] = (sqr(value1[i]) + sqr(value2[i])) * (marker->conductivity.number) * x[i];
        }
    }
        break;
    case PhysicFieldVariable_RF_Permeability:
    {
        SceneLabelRFMarker *marker = dynamic_cast<SceneLabelRFMarker *>(labelMarker);
        node->values[0][0][i] = marker->permeability.number;
    }
        break;
    case PhysicFieldVariable_RF_Permittivity:
    {
        SceneLabelRFMarker *marker = dynamic_cast<SceneLabelRFMarker *>(labelMarker);
        node->values[0][0][i] = marker->permittivity.number;
    }
        break;
    case PhysicFieldVariable_RF_Conductivity:
    {
        SceneLabelRFMarker *marker = dynamic_cast<SceneLabelRFMarker *>(labelMarker);
        node->values[0][0][i] = marker->conductivity.number;
    }
        break;
    case PhysicFieldVariable_RF_CurrentDensityReal:
    {
        SceneLabelRFMarker *marker = dynamic_cast<SceneLabelRFMarker *>(labelMarker);
        node->values[0][0][i] = marker->current_density_real.number;
    }
        break;
    case PhysicFieldVariable_RF_CurrentDensityImag:
    {
        SceneLabelRFMarker *marker = dynamic_cast<SceneLabelRFMarker *>(labelMarker);
        node->values[0][0][i] = marker->current_density_imag.number;
    }
        break;
    default:
        cerr << "Physical field variable '" + physicFieldVariableString(m_physicFieldVariable).toStdString() + "' is not implemented. ViewScalarFilterRF::calculateVariable()" << endl;
        throw;
        break;
    }
}

// *************************************************************************************************************************************

SceneEdgeRFMarker::SceneEdgeRFMarker(const QString &name, PhysicFieldBC type, Value value_real, Value value_imag)
    : SceneEdgeMarker(name, type)
{
    this->value_real = value_real;
    this->value_imag = value_imag;
}


SceneEdgeRFMarker::SceneEdgeRFMarker(const QString &name, PhysicFieldBC type, TEMode mode, Value power, Value phase, Value height)
    : SceneEdgeMarker(name, type)
{
    this->power = power;
    this->phase = phase;
    this->mode = mode;
    this->height = height;
}

SceneEdgeRFMarker::SceneEdgeRFMarker(const QString &name, PhysicFieldBC type, Value height) : SceneEdgeMarker(name, type)
{
    this->height = height;
}

QString SceneEdgeRFMarker::script()
{
    if (type == PhysicFieldBC_RF_ElectricField ||
            type == PhysicFieldBC_RF_MagneticField)
        return QString("addboundary(\"%1\", \"%2\", %4, %5)").
                arg(name).
                arg(physicFieldBCToStringKey(type)).
                arg(value_real.text).
                arg(value_imag.text);

    if (type == (PhysicFieldBC_RF_Port))
        return QString("addboundary(\"%1\", \"%2\", \"%3\", %4, %5, %6)").
                arg(name).
                arg(physicFieldBCToStringKey(type)).
                arg(teModeToStringKey(mode)).
                arg(power.text).
                arg(phase.text).
                arg(height.text);

    if (type == PhysicFieldBC_RF_MatchedBoundary)
    {
        return QString("addboundary(\"%1\", \"%2\", %3)").
                arg(name).
                arg(physicFieldBCToStringKey(type)).
                arg(height.text);
    }

}

QMap<QString, QString> SceneEdgeRFMarker::data()
{
    QMap<QString, QString> out;
    switch (type)
    {
    case PhysicFieldBC_RF_ElectricField:
        out["Electric field - real (V/m)"] = value_real.text;
        out["Electric field - imag (V/m)"] = value_imag.text;
        break;
    case PhysicFieldBC_RF_MagneticField:
        out["Magnetic field - real (A/m)"] = value_real.text;
        out["Magnetic field - imag (A/m)"] = value_imag.text;
        break;
    case PhysicFieldBC_RF_Port:
        out["Port - power (W)"] = power.text;
        out["Port - phase (deg.)"] = phase.text;
        break;
    case PhysicFieldBC_RF_MatchedBoundary:
        break;

    }
    return QMap<QString, QString>(out);
}

int SceneEdgeRFMarker::showDialog(QWidget *parent)
{
    DSceneEdgeRFMarker *dialog = new DSceneEdgeRFMarker(this, parent);
    return dialog->exec();
}

// *************************************************************************************************************************************

SceneLabelRFMarker::SceneLabelRFMarker(const QString &name, Value permittivity,Value permeability, Value conductivity, Value current_density_real, Value current_density_imag)
    : SceneLabelMarker(name)
{
    this->permittivity = permittivity;
    this->permeability = permeability;
    this->conductivity = conductivity;
    this->current_density_real = current_density_real;
    this->current_density_imag = current_density_imag;
}

QString SceneLabelRFMarker::script()
{
    return QString("addmaterial(\"%1\", %2, %3, %4)").
            arg(name).
            arg(permittivity.text).
            arg(permeability.text).
            arg(conductivity.text).
            arg(current_density_real.text).
            arg(current_density_imag.text);
}

QMap<QString, QString> SceneLabelRFMarker::data()
{
    QMap<QString, QString> out;
    out["Permittivity (-)"] = permittivity.text;
    out["Permeability (-)"] = permeability.text;
    out["Conductivity (S/m)"] = conductivity.text;
    out["Current density - real (A/m2)"] = current_density_real.text;
    out["Current density - imag (A/m2)"] = current_density_imag.text;
    return QMap<QString, QString>(out);
}

int SceneLabelRFMarker::showDialog(QWidget *parent)
{
    DSceneLabelRFMarker *dialog = new DSceneLabelRFMarker(parent, this);
    return dialog->exec();
}

// *************************************************************************************************************************************

DSceneEdgeRFMarker::DSceneEdgeRFMarker(SceneEdgeRFMarker *edgeRFMarker, QWidget *parent) : DSceneEdgeMarker(parent)
{
    m_edgeMarker = edgeRFMarker;

    createDialog();

    load();
    setSize();
}

void DSceneEdgeRFMarker::createContent()
{
    lblValueUnitReal = new QLabel("");
    lblValueUnitImag = new QLabel("");

    cmbType = new QComboBox(this);
    cmbType->addItem(physicFieldBCString(PhysicFieldBC_RF_ElectricField), PhysicFieldBC_RF_ElectricField);
    cmbType->addItem(physicFieldBCString(PhysicFieldBC_RF_MagneticField), PhysicFieldBC_RF_MagneticField);
    cmbType->addItem(physicFieldBCString(PhysicFieldBC_RF_MatchedBoundary), PhysicFieldBC_RF_MatchedBoundary);
    cmbType->addItem(physicFieldBCString(PhysicFieldBC_RF_Port), PhysicFieldBC_RF_Port);
    connect(cmbType, SIGNAL(currentIndexChanged(int)), this, SLOT(doTypeChanged(int)));

    txtValueReal = new ValueLineEdit(this, true);
    txtValueImag = new ValueLineEdit(this, true);
    txtHeight = new ValueLineEdit(this, true);

    cmbMode = new QComboBox(this);
    cmbMode->addItem(teModeString(TEMode_0), TEMode_0);
    cmbMode->addItem(teModeString(TEMode_1), TEMode_1);
    cmbMode->addItem(teModeString(TEMode_2), TEMode_2);

    // set active marker
    doTypeChanged(cmbType->currentIndex());

    connect(txtValueReal, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtValueImag, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtHeight, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    QHBoxLayout *layoutPhysicField = new QHBoxLayout();
    layoutPhysicField->addWidget(txtValueReal);
    layoutPhysicField->addWidget(lblValueUnitImag, 10, 0);
    layoutPhysicField->addWidget(txtValueImag);

    layout->addWidget(new QLabel(tr("BC type:")), 4, 0);
    layout->addWidget(cmbType, 4, 2);
    layout->addWidget(lblValueUnitReal, 10, 0);
    layout->addLayout(layoutPhysicField, 10, 2);
    layout->addWidget(new QLabel(tr("TE mode:")), 12, 0);
    layout->addWidget(cmbMode, 12, 2);
    layout->addWidget(createLabel(tr("<i>h</i> (m)"),
                                  tr("Height:")), 13, 0);
    layout->addWidget(txtHeight, 13, 2);
}

void DSceneEdgeRFMarker::load()
{
    DSceneEdgeMarker::load();

    SceneEdgeRFMarker *edgeRFMarker = dynamic_cast<SceneEdgeRFMarker *>(m_edgeMarker);

    cmbType->setCurrentIndex(cmbType->findData(edgeRFMarker->type));

    if (edgeRFMarker->type == PhysicFieldBC_RF_Port)
        cmbMode->setCurrentIndex(cmbMode->findData(edgeRFMarker->mode));

    if (edgeRFMarker->type == PhysicFieldBC_RF_ElectricField ||
            edgeRFMarker->type == PhysicFieldBC_RF_MagneticField)
    {
        txtValueReal->setValue(edgeRFMarker->value_real);
        txtValueImag->setValue(edgeRFMarker->value_imag);
    }

    if (edgeRFMarker->type == PhysicFieldBC_RF_Port ||
            edgeRFMarker->type == PhysicFieldBC_RF_MatchedBoundary)
    {
        txtHeight->setValue(edgeRFMarker->height);
    }

    if (edgeRFMarker->type == PhysicFieldBC_RF_Port)
    {
        txtValueReal->setValue(edgeRFMarker->power);
        txtValueImag->setValue(edgeRFMarker->phase);
    }
}

bool DSceneEdgeRFMarker::save() {
    if (!DSceneEdgeMarker::save()) return false;;

    SceneEdgeRFMarker *edgeRFMarker = dynamic_cast<SceneEdgeRFMarker *>(m_edgeMarker);

    edgeRFMarker->type = (PhysicFieldBC) cmbType->itemData(cmbType->currentIndex()).toInt();
    edgeRFMarker->mode = (TEMode) cmbMode->itemData(cmbMode->currentIndex()).toInt();

    if (edgeRFMarker->type == PhysicFieldBC_RF_ElectricField ||
            edgeRFMarker->type == PhysicFieldBC_RF_MagneticField)
    {
        if (txtValueReal->evaluate())
            edgeRFMarker->value_real  = txtValueReal->value();
        else
            return false;

        if (txtValueImag->evaluate())
            edgeRFMarker->value_imag  = txtValueImag->value();
        else
            return false;
    }
    else
    {
        edgeRFMarker->value_real = Value();
        edgeRFMarker->value_imag = Value();
    }

    if (edgeRFMarker->type == PhysicFieldBC_RF_Port ||
            edgeRFMarker->type == PhysicFieldBC_RF_MatchedBoundary)
    {
        if (txtHeight->evaluate())
            edgeRFMarker->height  = txtHeight->value();
        else
            return false;
    }
    else
    {
        edgeRFMarker->height = Value();
    }

    if (edgeRFMarker->type == PhysicFieldBC_RF_Port)
    {
        if (txtValueReal->evaluate())
            edgeRFMarker->power  = txtValueReal->value();
        else
            return false;

        if (txtValueImag->evaluate())
            edgeRFMarker->phase = txtValueImag->value();
        else
            return false;
    }
    else
    {
        edgeRFMarker->power = Value();
        edgeRFMarker->phase = Value();
    }

    return true;
}

void DSceneEdgeRFMarker::doTypeChanged(int index)
{
    txtValueReal->setEnabled(false);
    txtValueImag->setEnabled(false);
    txtHeight->setEnabled(false);
    cmbMode->setEnabled(false);

    switch ((PhysicFieldBC) cmbType->itemData(index).toInt())
    {
    case PhysicFieldBC_RF_ElectricField:
    {
        txtValueReal->setEnabled(true);
            lblValueUnitReal->setText(tr(""));
            lblValueUnitReal->setToolTip(cmbType->itemText(index));
        txtValueImag->setEnabled(true);
            lblValueUnitImag->setText(tr(" + j "));
            lblValueUnitImag->setToolTip(cmbType->itemText(index));
    }
        break;

    case PhysicFieldBC_RF_MagneticField:
    {
        txtValueReal->setEnabled(true);
           lblValueUnitReal->setText(tr(""));
           lblValueUnitReal->setToolTip(cmbType->itemText(index));
        txtValueImag->setEnabled(true);
            lblValueUnitImag->setText(tr(" + j "));
            lblValueUnitImag->setToolTip(cmbType->itemText(index));
    }
        break;

    case PhysicFieldBC_RF_MatchedBoundary:
    {
        txtHeight->setEnabled(true);
        lblValueUnitReal->setText(tr(""));
        lblValueUnitImag->setText(tr(""));
    }
        break;

    case PhysicFieldBC_RF_Port:
    {
        txtValueReal->setEnabled(true);
            lblValueUnitReal->setText(tr("Power (W):"));
            lblValueUnitReal->setToolTip(cmbType->itemText(index));
        txtValueImag->setEnabled(true);
            lblValueUnitImag->setText(tr("Phase (°):"));
            lblValueUnitImag->setToolTip(cmbType->itemText(index));
        txtHeight->setEnabled(true);
        cmbMode->setEnabled(true);       
    }
        break;
    }
}


// *************************************************************************************************************************************

DSceneLabelRFMarker::DSceneLabelRFMarker(QWidget *parent, SceneLabelRFMarker *labelRFMarker) : DSceneLabelMarker(parent)
{
    m_labelMarker = labelRFMarker;

    createDialog();

    load();
    setSize();
}

void DSceneLabelRFMarker::createContent()
{
    txtPermittivity = new ValueLineEdit(this);
    txtPermeability = new ValueLineEdit(this);
    txtConductivity = new ValueLineEdit(this);
    txtCurrentDensityReal = new ValueLineEdit(this);
    txtCurrentDensityImag = new ValueLineEdit(this);

    connect(txtPermeability, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtConductivity, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtCurrentDensityReal, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtCurrentDensityImag, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtPermittivity, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    QHBoxLayout *layoutCurrentDensity = new QHBoxLayout();
    layoutCurrentDensity->addWidget(txtCurrentDensityReal);
    layoutCurrentDensity->addWidget(new QLabel(" + j "));
    layoutCurrentDensity->addWidget(txtCurrentDensityImag);

    layout->addWidget(createLabel(tr("<i>%1</i><sub>r</sub> (-)").arg(QString::fromUtf8("ε")),
                                  tr("Permittivity")), 10, 0);
    layout->addWidget(txtPermittivity, 10, 2);
    layout->addWidget(createLabel(tr("<i>%1</i><sub>r</sub> (-)").arg(QString::fromUtf8("μ")),
                                  tr("Permeability")), 11, 0);
    layout->addWidget(txtPermeability, 11, 2);
    layout->addWidget(createLabel(tr("<i>%1</i> (S/m)").arg(QString::fromUtf8("σ")),
                                  tr("Conductivity")), 12, 0);
    layout->addWidget(txtConductivity, 12, 2);
    layout->addWidget(new QLabel(), 13, 0);
    layout->addWidget(createLabel(tr("<i>J</i><sub>ext</sub> (A/m<sup>2</sup>)"),
                                  tr("Current density")), 13, 0);
    layout->addLayout(layoutCurrentDensity, 13, 2);
}

void DSceneLabelRFMarker::load()
{
    DSceneLabelMarker::load();

    SceneLabelRFMarker *labelRFMarker = dynamic_cast<SceneLabelRFMarker *>(m_labelMarker);

    txtPermittivity->setValue(labelRFMarker->permittivity);
    txtPermeability->setValue(labelRFMarker->permeability);
    txtConductivity->setValue(labelRFMarker->conductivity);
    txtCurrentDensityReal->setValue(labelRFMarker->current_density_real);
    txtCurrentDensityImag->setValue(labelRFMarker->current_density_imag);
}

bool DSceneLabelRFMarker::save() {
    if (!DSceneLabelMarker::save()) return false;;

    SceneLabelRFMarker *labelRFMarker = dynamic_cast<SceneLabelRFMarker *>(m_labelMarker);

    if (txtPermittivity->evaluate())
        labelRFMarker->permittivity  = txtPermittivity->value();
    else
        return false;

    if (txtPermeability->evaluate())
        labelRFMarker->permeability  = txtPermeability->value();
    else
        return false;

    if (txtConductivity->evaluate())
        labelRFMarker->conductivity  = txtConductivity->value();
    else
        return false;

    if (txtCurrentDensityReal->evaluate())
        labelRFMarker->current_density_real  = txtCurrentDensityReal->value();
    else
        return false;

    if (txtCurrentDensityImag->evaluate())
        labelRFMarker->current_density_imag  = txtCurrentDensityImag->value();
    else
        return false;

    return true;
}
