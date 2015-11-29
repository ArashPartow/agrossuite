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

{{CLASS}}VolumeIntegral::{{CLASS}}VolumeIntegral(Computation *computation,
                                                 const FieldInfo *fieldInfo,
                                                 int timeStep,
                                                 int adaptivityStep)
    : IntegralValue(computation, fieldInfo, timeStep, adaptivityStep)
{
    calculate();
}

void {{CLASS}}VolumeIntegral::calculate()
{
    m_values.clear();

    if (m_computation->isSolved())
    {
        double frequency = m_computation->config()->value(ProblemConfig::Frequency).value<Value>().number();

        FieldSolutionID fsid(m_fieldInfo->fieldId(), m_timeStep, m_adaptivityStep);
        // check existence
        if (!m_computation->solutionStore()->contains(fsid))
            return;

        MultiArray ma = m_computation->solutionStore()->multiArray(fsid);

        // update time functions
        if (!m_computation->isSolving() && m_fieldInfo->analysisType() == AnalysisType_Transient)
        {
            Module::updateTimeFunctions(m_computation, m_computation->timeStepToTotalTime(m_timeStep));
        }

        // Gauss quadrature - volume
        dealii::hp::QCollection<2> quadrature_formulas;
        for (unsigned int degree = m_fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt(); degree <= DEALII_MAX_ORDER; degree++)
            quadrature_formulas.push_back(dealii::QGauss<2>(degree + 1));

        dealii::hp::FEValues<2> hp_fe_values(ma.doFHandler()->get_fe(), quadrature_formulas, dealii::update_values | dealii::update_gradients | dealii::update_quadrature_points | dealii::update_JxW_values);

        // Gauss quadrature - surface
        dealii::hp::QCollection<2-1> face_quadrature_formulas;
        for (unsigned int degree = m_fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt(); degree <= DEALII_MAX_ORDER; degree++)
            face_quadrature_formulas.push_back(dealii::QGauss<2-1>(degree + 1));

        dealii::hp::FEFaceValues<2> hp_fe_face_values(ma.doFHandler()->get_fe(), face_quadrature_formulas, dealii::update_values | dealii::update_gradients | dealii::update_quadrature_points | dealii::update_normal_vectors | dealii::update_JxW_values);

        // find faces around the selected labels
        QList<int> surroundings;
        if ({{INTEGRAL_COUNT_EGGSHELL}} > 0)
        {
            for (int iFace = 0; iFace < m_computation->scene()->faces->count(); iFace++)
            {
                SceneFace *edge = m_computation->scene()->faces->at(iFace);

                if ((edge->leftLabelIdx() != MARKER_IDX_NOT_EXISTING) && (edge->rightLabelIdx() == MARKER_IDX_NOT_EXISTING))
                {
                    if (edge->leftLabel()->isSelected())
                        surroundings.append(iFace);
                }
                else if ((edge->leftLabelIdx() == MARKER_IDX_NOT_EXISTING) && (edge->rightLabelIdx() != MARKER_IDX_NOT_EXISTING))
                {
                    if (edge->rightLabel()->isSelected())
                        surroundings.append(iFace);
                }
                else if ((edge->leftLabelIdx() != MARKER_IDX_NOT_EXISTING) && (edge->rightLabelIdx() != MARKER_IDX_NOT_EXISTING))
                {
                    if ((edge->leftLabel()->isSelected() && !edge->rightLabel()->isSelected()) ||
                            (!edge->leftLabel()->isSelected() && edge->rightLabel()->isSelected()))
                        surroundings.append(iFace);
                }
            }
        }

        for (int iLabel = 0; iLabel < m_computation->scene()->labels->count(); iLabel++)
        {
            SceneLabel *label = m_computation->scene()->labels->at(iLabel);
            SceneMaterial *material = label->marker(m_fieldInfo);
            if (material->isNone())
                continue;

            {{#VARIABLE_MATERIAL}}const Value *material_{{MATERIAL_VARIABLE}} = material->valueNakedPtr(QLatin1String("{{MATERIAL_VARIABLE}}"));
            {{/VARIABLE_MATERIAL}}

            // Then start the loop over all cells, and select those cells which are close enough to the evaluation point:
            dealii::hp::DoFHandler<2>::active_cell_iterator cell_int = ma.doFHandler()->begin_active(), endc_int = ma.doFHandler()->end();
            for (; cell_int != endc_int; ++cell_int)
            {
                // volume integration
                if (cell_int->material_id() - 1 == iLabel)
                {
                    if (label->isSelected())
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
                        if ((m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}}) && (m_computation->config()->coordinateType() == {{COORDINATE_TYPE}}))
                        {
                            double res = 0.0;
                            for (unsigned int k = 0; k < n_q_points; ++k)
                            {
                                const dealii::Point<2> p = fe_values.quadrature_point(k);

                                res += fe_values.JxW(k) * ({{EXPRESSION}});
                            }
                            m_values[QLatin1String("{{VARIABLE}}")] += res;
                        }
                        {{/VARIABLE_SOURCE}}
                    }

                    // volume to surface (temporary eggshell replacement)
                    if ({{INTEGRAL_COUNT_EGGSHELL}} > 0)
                    {
                        for (int j = 0; j < surroundings.count(); j++)
                        {
                            // surface integration
                            for (unsigned int face = 0; face < dealii::GeometryInfo<2>::faces_per_cell; ++face)
                            {
                                if (cell_int->face(face)->user_index() - 1 == surroundings[j])
                                {
                                    hp_fe_face_values.reinit(cell_int, face);

                                    const dealii::FEFaceValues<2> &fe_values = hp_fe_face_values.get_present_fe_values();
                                    const unsigned int n_face_q_points = fe_values.n_quadrature_points;

                                    std::vector<dealii::Vector<double> > solution_values(n_face_q_points, dealii::Vector<double>(m_fieldInfo->numberOfSolutions()));
                                    std::vector<std::vector<dealii::Tensor<1,2> > >  solution_grads(n_face_q_points, std::vector<dealii::Tensor<1,2> >(m_fieldInfo->numberOfSolutions()));

                                    fe_values.get_function_values(ma.solution(), solution_values);
                                    fe_values.get_function_gradients(ma.solution(), solution_grads);

                                    // expressions
                                    {{#VARIABLE_SOURCE_EGGSHELL}}
                                    if ((m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}}) && (m_computation->config()->coordinateType() == {{COORDINATE_TYPE}}))
                                    {
                                        double res = 0.0;
                                        for (unsigned int k = 0; k < n_face_q_points; ++k)
                                        {
                                            const dealii::Point<2> p = fe_values.quadrature_point(k);
                                            const dealii::Tensor<1,2> normal = fe_values.normal_vector(k);

                                            res += fe_values.JxW(k) * ({{EXPRESSION}});
                                        }
                                        m_values[QLatin1String("{{VARIABLE}}")] += res;
                                    }
                                    {{/VARIABLE_SOURCE_EGGSHELL}}
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
