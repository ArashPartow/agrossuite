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

#include "{{ID}}_volumeintegral.h"
// #include "{{ID}}_extfunction.h"
#include "{{ID}}_interface.h"

#include "util.h"
#include "util/global.h"
#include "util/constants.h"

#include "solver/problem.h"
#include "solver/problem_config.h"
#include "solver/field.h"
#include "solver/solutionstore.h"

#include "solver/plugin_interface.h"

#include <deal.II/grid/tria.h>
#include <deal.II/dofs/dof_handler.h>

#include <deal.II/fe/fe_q.h>
#include <deal.II/dofs/dof_tools.h>

#include <deal.II/fe/fe_values.h>
#include <deal.II/base/quadrature_lib.h>

#include <deal.II/grid/grid_tools.h>
#include <deal.II/fe/mapping_q1.h>
#include <deal.II/numerics/fe_field_function.h>

#include <deal.II/numerics/vector_tools.h>

/*
class {{CLASS}}VolumetricIntegralEggShellCalculator : public PostProcessing::VolumetricIntegralCalculator<double>
{
public:
    {{CLASS}}VolumetricIntegralEggShellCalculator(const FieldInfo *fieldInfo, MeshFunctionSharedPtr<double> source_function, int number_of_integrals)
        : PostProcessing::VolumetricIntegralCalculator<double>(source_function, number_of_integrals), m_fieldInfo(fieldInfo)
    {
        {{#SPECIAL_FUNCTION_SOURCE}}
        {{SPECIAL_FUNCTION_NAME}} = QSharedPointer<{{SPECIAL_EXT_FUNCTION_FULL_NAME}}>(new {{SPECIAL_EXT_FUNCTION_FULL_NAME}}(m_fieldInfo, 0));{{/SPECIAL_FUNCTION_SOURCE}}
    }

    {{CLASS}}VolumetricIntegralEggShellCalculator(const FieldInfo *fieldInfo, std::vector<MeshFunctionSharedPtr<double> > source_functions, int number_of_integrals)
        : PostProcessing::VolumetricIntegralCalculator<double>(source_functions, number_of_integrals), m_fieldInfo(fieldInfo)
    {
        {{#SPECIAL_FUNCTION_SOURCE}}
        {{SPECIAL_FUNCTION_NAME}} = QSharedPointer<{{SPECIAL_EXT_FUNCTION_FULL_NAME}}>(new {{SPECIAL_EXT_FUNCTION_FULL_NAME}}(m_fieldInfo, 0));{{/SPECIAL_FUNCTION_SOURCE}}
    }

    virtual void integral(int n, double* wt, Func<double> **fns, GeomVol<double> *e, double* result)
    {
        SceneLabel *label = Agros2D::scene()->labels->at(atoi(m_fieldInfo->initialMesh()->get_element_markers_conversion().get_user_marker(e->elem_marker).marker.c_str()));
        SceneMaterial *material = label->marker(m_fieldInfo);

        double *x = e->x;
        double *y = e->y;

        {{#VARIABLE_MATERIAL}}const Value *material_{{MATERIAL_VARIABLE}} = material->valueNakedPtr(QLatin1String("{{MATERIAL_VARIABLE}}"));
        {{/VARIABLE_MATERIAL}}
        // {{#SPECIAL_FUNCTION_SOURCE}}
        // QSharedPointer<{{SPECIAL_EXT_FUNCTION_FULL_NAME}}> {{SPECIAL_FUNCTION_NAME}};
        // if (m_fieldInfo->functionUsedInAnalysis("{{SPECIAL_FUNCTION_ID}}"))
        //     {{SPECIAL_FUNCTION_NAME}} = QSharedPointer<{{SPECIAL_EXT_FUNCTION_FULL_NAME}}>(new {{SPECIAL_EXT_FUNCTION_FULL_NAME}}(m_fieldInfo, 0));
        // {{/SPECIAL_FUNCTION_SOURCE}}

        double **value = new double*[source_functions.size()];
        double **dudx = new double*[source_functions.size()];
        double **dudy = new double*[source_functions.size()];

        for (int i = 0; i < source_functions.size(); i++)
        {
            value[i] = fns[i]->val;
            dudx[i] = fns[i]->dx;
            dudy[i] = fns[i]->dy;
        }

        // expressions
        {{#VARIABLE_SOURCE_EGGSHELL}}
        if ((m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}}) && (Agros2D::problem()->config()->coordinateType() == {{COORDINATE_TYPE}}))
        {
            for (int i = 0; i < n; i++)
                result[{{POSITION}}] += wt[i] * ({{EXPRESSION}});
        }
        {{/VARIABLE_SOURCE_EGGSHELL}}

        delete [] value;
        delete [] dudx;
        delete [] dudy;
    }

private:
    // field info
    const FieldInfo *m_fieldInfo;

    {{#SPECIAL_FUNCTION_SOURCE}}
    QSharedPointer<{{SPECIAL_EXT_FUNCTION_FULL_NAME}}> {{SPECIAL_FUNCTION_NAME}};{{/SPECIAL_FUNCTION_SOURCE}}
};

class {{CLASS}}VolumetricIntegralCalculator : public PostProcessing::VolumetricIntegralCalculator<double>
{
public:
    {{CLASS}}VolumetricIntegralCalculator(const FieldInfo *fieldInfo, MeshFunctionSharedPtr<double> source_function, int number_of_integrals)
        : PostProcessing::VolumetricIntegralCalculator<double>(source_function, number_of_integrals), m_fieldInfo(fieldInfo)
    {
        {{#SPECIAL_FUNCTION_SOURCE}}
        {{SPECIAL_FUNCTION_NAME}} = QSharedPointer<{{SPECIAL_EXT_FUNCTION_FULL_NAME}}>(new {{SPECIAL_EXT_FUNCTION_FULL_NAME}}(m_fieldInfo, 0));{{/SPECIAL_FUNCTION_SOURCE}}
    }

    {{CLASS}}VolumetricIntegralCalculator(const FieldInfo *fieldInfo, std::vector<MeshFunctionSharedPtr<double> > source_functions, int number_of_integrals)
        : PostProcessing::VolumetricIntegralCalculator<double>(source_functions, number_of_integrals), m_fieldInfo(fieldInfo)
    {
        {{#SPECIAL_FUNCTION_SOURCE}}
        {{SPECIAL_FUNCTION_NAME}} = QSharedPointer<{{SPECIAL_EXT_FUNCTION_FULL_NAME}}>(new {{SPECIAL_EXT_FUNCTION_FULL_NAME}}(m_fieldInfo, 0));{{/SPECIAL_FUNCTION_SOURCE}}
    }

    virtual void integral(int n, double* wt, Func<double> **fns, GeomVol<double> *e, double* result)
    {
        SceneLabel *label = Agros2D::scene()->labels->at(atoi(m_fieldInfo->initialMesh()->get_element_markers_conversion().get_user_marker(e->elem_marker).marker.c_str()));
        SceneMaterial *material = label->marker(m_fieldInfo);

        double *x = e->x;
        double *y = e->y;
        int elementMarker = e->elem_marker;

        {{#VARIABLE_MATERIAL}}const Value *material_{{MATERIAL_VARIABLE}} = material->valueNakedPtr(QLatin1String("{{MATERIAL_VARIABLE}}"));
        {{/VARIABLE_MATERIAL}}
        // {{#SPECIAL_FUNCTION_SOURCE}}
        // QSharedPointer<{{SPECIAL_EXT_FUNCTION_FULL_NAME}}> {{SPECIAL_FUNCTION_NAME}};
        // if (isInCalculation_{{SPECIAL_FUNCTION_ID}})
        //     {{SPECIAL_FUNCTION_NAME}} = QSharedPointer<{{SPECIAL_EXT_FUNCTION_FULL_NAME}}>(new {{SPECIAL_EXT_FUNCTION_FULL_NAME}}(m_fieldInfo, 0));
        // {{/SPECIAL_FUNCTION_SOURCE}}

        double **value = new double*[source_functions.size()];
        double **dudx = new double*[source_functions.size()];
        double **dudy = new double*[source_functions.size()];

        for (int i = 0; i < source_functions.size(); i++)
        {
            value[i] = fns[i]->val;
            dudx[i] = fns[i]->dx;
            dudy[i] = fns[i]->dy;
        }

        // expressions
        {{#VARIABLE_SOURCE}}
        if ((m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}}) && (Agros2D::problem()->config()->coordinateType() == {{COORDINATE_TYPE}}))
        {
            for (int i = 0; i < n; i++)
                result[{{POSITION}}] += wt[i] * ({{EXPRESSION}});
        }
        {{/VARIABLE_SOURCE}}

        delete [] value;
        delete [] dudx;
        delete [] dudy;
    }

private:
    // field info
    const FieldInfo *m_fieldInfo;

    {{#SPECIAL_FUNCTION_SOURCE}}
    QSharedPointer<{{SPECIAL_EXT_FUNCTION_FULL_NAME}}> {{SPECIAL_FUNCTION_NAME}};{{/SPECIAL_FUNCTION_SOURCE}}
};

// find edges around the selected labels
QList<int> surroundings;
for (int iEdge = 0; iEdge < Agros2D::scene()->edges->count(); iEdge++)
{
    SceneEdge *edge = Agros2D::scene()->edges->at(iEdge);

    if ((edge->leftLabelIdx() != MARKER_IDX_NOT_EXISTING) && (edge->rightLabelIdx() == MARKER_IDX_NOT_EXISTING))
    {
        if (edge->leftLabel()->isSelected())
            surroundings.append(iEdge);
    }
    else if ((edge->leftLabelIdx() == MARKER_IDX_NOT_EXISTING) && (edge->rightLabelIdx() != MARKER_IDX_NOT_EXISTING))
    {
        if (edge->rightLabel()->isSelected())
            surroundings.append(iEdge);
    }
    else if ((edge->leftLabelIdx() != MARKER_IDX_NOT_EXISTING) && (edge->rightLabelIdx() != MARKER_IDX_NOT_EXISTING))
    {
        if ((edge->leftLabel()->isSelected() && !edge->rightLabel()->isSelected()) ||
                (!edge->leftLabel()->isSelected() && edge->rightLabel()->isSelected()))
            surroundings.append(iEdge);
    }
}
qDebug() << "surroundings";
foreach (int idx, surroundings)
    qDebug() << idx;
qDebug() << "------------";

*/

{{CLASS}}VolumeIntegral::{{CLASS}}VolumeIntegral(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep)
    : IntegralValue(fieldInfo, timeStep, adaptivityStep)
{
    calculate();
}

void {{CLASS}}VolumeIntegral::calculate()
{
    m_values.clear();

    if (Agros2D::problem()->isSolved())
    {
        FieldSolutionID fsid(m_fieldInfo, m_timeStep, m_adaptivityStep);
        // check existence
        if (!Agros2D::solutionStore()->contains(fsid))
            return;

        MultiArray ma = Agros2D::solutionStore()->multiArray(fsid);

        // update time functions
        if (!Agros2D::problem()->isSolving() && m_fieldInfo->analysisType() == AnalysisType_Transient)
        {
            Module::updateTimeFunctions(Agros2D::problem()->timeStepToTotalTime(m_timeStep));
        }

        // Gauss quadrature
        dealii::hp::QCollection<2> quadrature_formulas;
        for (unsigned int degree = m_fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt(); degree <= DEALII_MAX_ORDER; degree++)
            quadrature_formulas.push_back(dealii::QGauss<2>(degree + 1));

        dealii::hp::FEValues<2> hp_fe_values(ma.doFHandler()->get_fe(), quadrature_formulas, dealii::update_values | dealii::update_gradients | dealii::update_quadrature_points | dealii::update_JxW_values);

        for (int iLabel = 0; iLabel < Agros2D::scene()->labels->count(); iLabel++)
        {
            SceneLabel *label = Agros2D::scene()->labels->at(iLabel);
            if (!label->isSelected())
                continue;

            SceneMaterial *material = label->marker(m_fieldInfo);

            {{#VARIABLE_MATERIAL}}const Value *material_{{MATERIAL_VARIABLE}} = material->valueNakedPtr(QLatin1String("{{MATERIAL_VARIABLE}}"));
            {{/VARIABLE_MATERIAL}}

            // Then start the loop over all cells, and select those cells which are close enough to the evaluation point:
            dealii::hp::DoFHandler<2>::active_cell_iterator cell_int = ma.doFHandler()->begin_active(), endc_int = ma.doFHandler()->end();
            for (; cell_int != endc_int; ++cell_int)
            {
                // volume integration
                if (cell_int->material_id() - 1 == iLabel)
                {
                    hp_fe_values.reinit(cell_int);

                    const dealii::FEValues<2> &fe_values = hp_fe_values.get_present_fe_values();
                    const unsigned int n_q_points = fe_values.n_quadrature_points;

                    std::vector<dealii::Vector<double> > solution_values(n_q_points, dealii::Vector<double>(m_fieldInfo->numberOfSolutions()));
                    std::vector<std::vector<dealii::Tensor<1,2> > >  solution_grads(n_q_points, std::vector<dealii::Tensor<1,2> >(m_fieldInfo->numberOfSolutions()));

                    fe_values.get_function_values(ma.solution(), solution_values);
                    fe_values.get_function_gradients(ma.solution(), solution_grads);

                    // expressions
                    {{#VARIABLE_SOURCE}}
                    if ((m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}}) && (Agros2D::problem()->config()->coordinateType() == {{COORDINATE_TYPE}}))
                    {
                        for (unsigned int k = 0; k < n_q_points; ++k)
                        {
                            const dealii::Point<2> p = fe_values.quadrature_point(k);

                            m_values[QLatin1String("{{VARIABLE}}")] += fe_values.JxW(k) * ({{EXPRESSION}});
                        }
                    }
                    {{/VARIABLE_SOURCE}}
                }
            }
        }

        //        if ({{INTEGRAL_COUNT_EGGSHELL}} > 0)
        //        {
        //            std::vector<std::string> markersInverted;
        //            for (int i = 0; i < Agros2D::scene()->labels->count(); i++)
        //            {
        //                SceneLabel *label = Agros2D::scene()->labels->at(i);
        //                if (!label->isSelected())
        //                    markersInverted.push_back(QString::number(i).toStdString());
        //            }

        //            if (markers.size() > 0 && markersInverted.size() > 0)
        //            {
        //                MeshSharedPtr eggShellMesh = EggShell::get_egg_shell(ma.solutions().at(0)->get_mesh(), markers, 3);
        //                if(eggShellMesh->get_num_active_elements() == 0)
        //                  return;
        //                MeshFunctionSharedPtr<double> eggShell(new ExactSolutionEggShell(eggShellMesh, 3));

        //                std::vector<MeshFunctionSharedPtr<double> > slns;
        //                for (int i = 0; i < ma.solutions().size(); i++)
        //                    slns.push_back(ma.solutions().at(i));
        //                slns.push_back(eggShell);

        //                {{CLASS}}VolumetricIntegralEggShellCalculator calcEggShell(m_fieldInfo, slns, {{INTEGRAL_COUNT_EGGSHELL}});
        //                double *valuesEggShell = calcEggShell.calculate(markersInverted);

        //                {{#VARIABLE_SOURCE_EGGSHELL}}
        //                if ((m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}}) && (Agros2D::problem()->config()->coordinateType() == {{COORDINATE_TYPE}}))
        //                    m_values[QLatin1String("{{VARIABLE}}")] = valuesEggShell[{{POSITION}}];
        //                {{/VARIABLE_SOURCE_EGGSHELL}}

        //                ::free(valuesEggShell);
        //            }
        //        }
    }
}
