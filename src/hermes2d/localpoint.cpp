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

#include "localpoint.h"
#include "scene.h"
#include "scenebasic.h"
#include "scenelabel.h"
#include "scenemarkerdialog.h"
#include "logview.h"
#include "hermes2d.h"
#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/field.h"
#include "hermes2d/problem.h"
#include "hermes2d/solutionstore.h"

int findElementInMesh(Hermes::Hermes2D::Mesh *mesh, const Point &point)
{
    assert(mesh);

    for (int i = 0, max = mesh->get_max_element_id(); i < max; i++)
    {
        Hermes::Hermes2D::Element *element;
        if ((element = mesh->get_element_fast(i))->used)
        {
            if (element->active)
            {
                bool inElement = false;
                int j;
                int npol = (element->is_triangle()) ? 3 : 4;

                for (int i = 0, j = npol-1; i < npol; j = i++) {
                    if ((((element->vn[i]->y <= point.y) && (point.y < element->vn[j]->y)) ||
                         ((element->vn[j]->y <= point.y) && (point.y < element->vn[i]->y))) &&
                            (point.x < (element->vn[j]->x - element->vn[i]->x) * (point.y - element->vn[i]->y)
                             / (element->vn[j]->y - element->vn[i]->y) + element->vn[i]->x))
                        inElement = !inElement;
                }

                if (inElement)
                    return i;
            }
        }
    }

    return -1;
}

LocalPointValue::LocalPointValue(FieldInfo *fieldInfo, const Point &point) : PostprocessorValue(fieldInfo), point(point)
{
    // parser variables
    parsers.push_back(m_fieldInfo->module()->expressionParser());

    initParser();

    calculate();
}

LocalPointValue::~LocalPointValue()
{
}

void LocalPointValue::initParser()
{
    // parser variables
    parsers.push_back(m_fieldInfo->module()->expressionParser());

    // init material variables
    // parser->initParserMaterialVariables();
}

void LocalPointValue::calculate()
{
    m_values.clear();
    this->point = point;

    // update time functions
    if (m_fieldInfo->analysisType() == AnalysisType_Transient)
    {
        QList<double> timeLevels = Util::solutionStore()->timeLevels(Util::scene()->activeViewField());
        m_fieldInfo->module()->updateTimeFunctions(timeLevels[Util::scene()->activeTimeStep()]);
    }

    if (Util::problem()->isSolved())
    {
        int index = findElementInMesh(Util::problem()->meshInitial(m_fieldInfo), point);
        if (index != -1)
        {
            // find marker
            Hermes::Hermes2D::Element *e = Util::problem()->meshInitial(m_fieldInfo)->get_element_fast(index);
            SceneLabel *label = Util::scene()->labels->at(atoi(Util::problem()->meshInitial(m_fieldInfo)->get_element_markers_conversion().get_user_marker(e->marker).marker.c_str()));
            SceneMaterial *tmpMaterial = label->getMarker(m_fieldInfo);

            // set variables
            double px = point.x;
            double py = point.y;
            parsers[0]->DefineVar(Util::problem()->config()->labelX().toLower().toStdString(), &px);
            parsers[0]->DefineVar(Util::problem()->config()->labelY().toLower().toStdString(), &py);

            double *pvalue = new double[m_fieldInfo->module()->numberOfSolutions()];
            double *pdx = new double[m_fieldInfo->module()->numberOfSolutions()];
            double *pdy = new double[m_fieldInfo->module()->numberOfSolutions()];
            std::vector<Hermes::Hermes2D::Solution<double> *> sln(m_fieldInfo->module()->numberOfSolutions()); //TODO PK <double>

            for (int k = 0; k < m_fieldInfo->module()->numberOfSolutions(); k++)
            {
                FieldSolutionID fsid(m_fieldInfo, Util::scene()->activeTimeStep(), Util::scene()->activeAdaptivityStep(), Util::scene()->activeSolutionType());
                sln[k] = Util::solutionStore()->multiSolution(fsid).component(k).sln.get();

                double value;
                if ((m_fieldInfo->analysisType() == AnalysisType_Transient) && Util::scene()->activeTimeStep() == 0)
                    // const solution at first time step
                    value = m_fieldInfo->initialCondition().number();
                else
                    value = sln[k]->get_pt_value(point.x, point.y, Hermes::Hermes2D::H2D_FN_VAL_0);

                Point derivative;
                derivative.x = sln[k]->get_pt_value(point.x, point.y, Hermes::Hermes2D::H2D_FN_DX_0);
                derivative.y = sln[k]->get_pt_value(point.x, point.y, Hermes::Hermes2D::H2D_FN_DY_0);

                // set variables
                pvalue[k] = value;
                pdx[k] = derivative.x;
                pdy[k] = derivative.y;

                std::stringstream number;
                number << (k+1);

                parsers[0]->DefineVar("value" + number.str(), &pvalue[k]);
                parsers[0]->DefineVar("d" + Util::problem()->config()->labelX().toLower().toStdString() + number.str(), &pdx[k]);
                parsers[0]->DefineVar("d" + Util::problem()->config()->labelY().toLower().toStdString() + number.str(), &pdy[k]);
            }

            // set material variables
            setMaterialToParsers(tmpMaterial);

            // parse expression
            foreach (Module::LocalVariable *variable, m_fieldInfo->module()->localPointVariables())
            {
                try
                {
                    PointValue pointValue;
                    if (variable->isScalar)
                    {
                        parsers[0]->SetExpr(variable->expr.scalar.toStdString());
                        pointValue.scalar = parsers[0]->Eval();
                    }
                    else
                    {
                        parsers[0]->SetExpr(variable->expr.comp_x.toStdString());
                        pointValue.vector.x = parsers[0]->Eval();
                        parsers[0]->SetExpr(variable->expr.comp_y.toStdString());
                        pointValue.vector.y = parsers[0]->Eval();
                    }
                    m_values[variable] = pointValue;

                }
                catch (mu::Parser::exception_type &e)
                {
                    qDebug() << "Local value: " << variable->name << " (" << variable->id << ") " << variable->name << " - " <<
                                QString::fromStdString(parsers[0]->GetExpr()) << " - " << QString::fromStdString(e.GetMsg());
                }
            }

            delete [] pvalue;
            delete [] pdx;
            delete [] pdy;
        }
    }
}
