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

#include "{{ID}}_filter.h"

#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/field.h"
#include "hermes2d/block.h"
#include "hermes2d/problem.h"
#include "hermes2d/solutionstore.h"

#include "util.h"
#include "util/global.h"

#include "scene.h"
#include "scenelabel.h"
#include "logview.h"

#include "hermes2d/plugin_interface.h"


{{CLASS}}ViewScalarFilter::{{CLASS}}ViewScalarFilter(FieldInfo *fieldInfo,
                                           Hermes::vector<Hermes::Hermes2D::MeshFunction<double> *> sln,
                                           const QString &variable,
                                           PhysicFieldVariableComp physicFieldVariableComp)
    : Hermes::Hermes2D::Filter<double>(sln), m_fieldInfo(fieldInfo), m_variable(variable), m_physicFieldVariableComp(physicFieldVariableComp)
{

}

{{CLASS}}ViewScalarFilter::~{{CLASS}}ViewScalarFilter()
{

}

Hermes::Hermes2D::Func<double> *{{CLASS}}ViewScalarFilter::get_pt_value(double x, double y)
{
    return NULL;
}

void {{CLASS}}ViewScalarFilter::precalculate(int order, int mask)
{
    bool isLinear = (m_fieldInfo->linearityType() == LinearityType_Linear);

    Hermes::Hermes2D::Quad2D* quad = this->quads[Hermes::Hermes2D::Function<double>::cur_quad];
    int np = quad->get_num_points(order, this->get_active_element()->get_mode());
    Hermes::Hermes2D::Function<double>::Node* node = this->new_node(Hermes::Hermes2D::H2D_FN_DEFAULT, np);

    double **value = new double*[m_fieldInfo->module()->numberOfSolutions()];
    double **dudx = new double*[m_fieldInfo->module()->numberOfSolutions()];
    double **dudy = new double*[m_fieldInfo->module()->numberOfSolutions()];

    for (int k = 0; k < this->num; k++)
    {
        this->sln[k]->set_quad_order(order, Hermes::Hermes2D::H2D_FN_DEFAULT);
        dudx[k] = this->sln[k]->get_dx_values();
        dudy[k] = this->sln[k]->get_dy_values();
        value[k] = this->sln[k]->get_fn_values();
    }

    this->update_refmap();

    double *x = this->refmap->get_phys_x(order);
    double *y = this->refmap->get_phys_y(order);
    Hermes::Hermes2D::Element *e = this->refmap->get_active_element();

    // set material
    SceneMaterial *material = Agros2D::scene()->labels->at(atoi(m_fieldInfo->initialMesh().data()->get_element_markers_conversion().
                                                             get_user_marker(e->marker).marker.c_str()))->marker(m_fieldInfo);

    {{#VARIABLE_SOURCE}}
    if ((m_fieldInfo->module()->coordinateType() == {{COORDINATE_TYPE}})
            && (m_fieldInfo->module()->analysisType() == {{ANALYSIS_TYPE}})
            && (m_physicFieldVariableComp == {{PHYSICFIELDVARIABLECOMP_TYPE}})
            && (m_variable == "{{VARIABLE}}"))
        for (int i = 0; i < np; i++)
            node->values[0][0][i] = {{EXPRESSION}};
    {{/VARIABLE_SOURCE}}

    delete [] value;
    delete [] dudx;
    delete [] dudy;

    if(this->nodes->present(order))
    {
      assert(this->nodes->get(order) == this->cur_node);
      ::free(this->nodes->get(order));
    }
    this->nodes->add(node, order);
    this->cur_node = node;
}

{{CLASS}}ViewScalarFilter* {{CLASS}}ViewScalarFilter::clone() const
{
    Hermes::vector<Hermes::Hermes2D::MeshFunction<double> *> slns;

    for (int i = 0; i < this->num; i++)
        slns.push_back(this->sln[i]->clone());

    return new {{CLASS}}ViewScalarFilter(m_fieldInfo, slns, m_variable, m_physicFieldVariableComp);
}

