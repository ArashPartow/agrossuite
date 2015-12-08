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

#include <deal.II/numerics/data_out.h>
#include <deal.II/numerics/data_postprocessor.h>
#include <deal.II/grid/filtered_iterator.h>
#include <deal.II/hp/dof_handler.h>

#include "sceneview_post.h"

#include "util/global.h"

#include "sceneview_common2d.h"
#include "sceneview_data.h"
#include "scene.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "scenemarkerselectdialog.h"
#include "scenebasicselectdialog.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"

#include "logview.h"

#include "solver/plugin_interface.h"
#include "solver/module.h"
#include "solver/field.h"
#include "solver/problem.h"
#include "solver/problem_config.h"
#include "solver/solutiontypes.h"
#include "solver/solutionstore.h"

#include "pythonlab/pythonengine.h"

PostDataOut::PostDataOut(FieldInfo *fieldInfo, Computation *parentProblem) : dealii::DataOut<2, dealii::hp::DoFHandler<2> >(),
    m_problem(parentProblem), m_fieldInfo(fieldInfo)
{
}

void PostDataOut::compute_nodes(QList<PostTriangle> &values, bool deform)
{
    values.clear();

    // min and max value
    m_min =  numeric_limits<double>::max();
    m_max = -numeric_limits<double>::max();
    double minDeform =  numeric_limits<double>::max();
    double maxDeform = -numeric_limits<double>::max();

    for (std::vector<dealii::DataOutBase::Patch<2> >::const_iterator patch = patches.begin(); patch != patches.end(); ++patch)
    {
        for (unsigned int i = 0; i < (patch->n_subdivisions + 1) * (patch->n_subdivisions + 1); i++)
        {
            double value = patch->data(0, i);

            m_min = std::min(m_min, value);
            m_max = std::max(m_max, value);

            if (deform)
            {
                double value = sqrt(patch->data(1, i)*patch->data(1, i) + patch->data(2, i)*patch->data(2, i));

                minDeform = std::min(m_min, value);
                maxDeform = std::max(m_max, value);
            }
        }
    }

    double dmult = 0.0;
    if (deform)
    {
        RectPoint rect = m_problem->scene()->boundingBox();
        dmult = qMax(rect.width(), rect.height()) / maxDeform / 15.0;
    }

    // compute values in patches
    dealii::Point<2> node0, node1, node2, node3;

    // loop over all patches
    for (std::vector<dealii::DataOutBase::Patch<2> >::const_iterator patch = patches.begin(); patch != patches.end(); ++patch)
    {
        const unsigned int n_subdivisions = patch->n_subdivisions;
        const unsigned int n = n_subdivisions + 1;
        unsigned int d1 = 1;
        unsigned int d2 = n;

        for (unsigned int i2=0; i2<n-1; ++i2)
        {
            for (unsigned int i1=0; i1<n-1; ++i1)
            {
                // compute coordinates for this patch point
                compute_node(node0, &*patch, i1, i2, 0, n_subdivisions);
                compute_node(node1, &*patch, i1, i2+1, 0, n_subdivisions);
                compute_node(node2, &*patch, i1+1, i2, 0, n_subdivisions);
                compute_node(node3, &*patch, i1+1, i2+1, 0, n_subdivisions);

                // compute values
                double value0 = patch->data(0, i1*d1 + i2*d2);
                double value1 = patch->data(0, i1*d1 + (i2+1)*d2);
                double value2 = patch->data(0, (i1+1)*d1 + i2*d2);
                double value3 = patch->data(0, (i1+1)*d1 + (i2+1)*d2);

                if (deform)
                {
                    // node(0) ... value
                    // node(1) ... disp x
                    // node(2) ... disp y
                    node0 += dmult * dealii::Point<2>(patch->data(1, i1*d1 + i2*d2), patch->data(2, i1*d1 + i2*d2));
                    node1 += dmult * dealii::Point<2>(patch->data(1, i1*d1 + (i2+1)*d2), patch->data(2, i1*d1 + (i2+1)*d2));
                    node2 += dmult * dealii::Point<2>(patch->data(1, (i1+1)*d1 + i2*d2), patch->data(2, (i1+1)*d1 + i2*d2));
                    node3 += dmult * dealii::Point<2>(patch->data(1, (i1+1)*d1 + (i2+1)*d2), patch->data(2, (i1+1)*d1 + (i2+1)*d2));
                }

                // create triangles
                values.append(PostTriangle(node0, node1, node2, value0, value1, value2));
                values.append(PostTriangle(node1, node3, node2, value1, value3, value2));
            }
        }
    }
}

void PostDataOut::compute_node(dealii::Point<2> &node, const dealii::DataOutBase::Patch<2> *patch,
                               const unsigned int xstep, const unsigned int ystep, const unsigned int zstep,
                               const unsigned int n_subdivisions)
{
    if (patch->points_are_available)
    {
        unsigned int point_no = 0;
        point_no += (n_subdivisions+1)*ystep;
        for (unsigned int d=0; d<2; ++d)
            node[d]=patch->data(patch->data.size(0)-2+d, point_no);
    }
    else
    {
        // perform a dim-linear interpolation
        const double stepsize=1./n_subdivisions, xfrac=xstep*stepsize;

        node = (patch->vertices[1] * xfrac) + (patch->vertices[0] * (1-xfrac));
        const double yfrac=ystep*stepsize;

        node*= 1-yfrac;
        node += ((patch->vertices[3] * xfrac) + (patch->vertices[2] * (1-xfrac))) * yfrac;
    }
}


dealii::DataOut<2>::cell_iterator PostDataOut::first_cell()
{
    DataOut<2>::cell_iterator cell = this->dofs->begin_active();
    while (cell != this->dofs->end())
    {
        if (!m_problem->scene()->labels->at(cell->material_id() - 1)->marker(m_fieldInfo)->isNone())
            break;
        else
            cell++;
    }

    return cell;
}

dealii::DataOut<2>::cell_iterator PostDataOut::next_cell(const DataOut<2>::cell_iterator &old_cell)
{
    // return dealii::DataOut<2, dealii::hp::DoFHandler<2> >::next_cell(old_cell);

    DataOut<2>::cell_iterator cell = dealii::DataOut<2, dealii::hp::DoFHandler<2> >::next_cell(old_cell);
    while (cell != this->dofs->end())
    {
        if (!m_problem->scene()->labels->at(cell->material_id() - 1)->marker(m_fieldInfo)->isNone())
            break;
        else
            cell++;
    }

    return cell;
}

// ************************************************************************************************************************

PostDeal::PostDeal(Computation *parentProblem) :
    m_computation(parentProblem),
    m_activeViewField(nullptr),
    m_activeTimeStep(NOT_FOUND_SO_FAR),
    m_activeAdaptivityStep(NOT_FOUND_SO_FAR),
    m_isProcessed(false)
{
    connect(m_computation->scene(), SIGNAL(cleared()), this, SLOT(clear()));
    // connect(m_problem, SIGNAL(fieldsChanged()), this, SLOT(clear()));
}

PostDeal::~PostDeal()
{
    clear();
}

void PostDeal::processRangeContour()
{
    if (m_computation->isSolved() && m_activeViewField && (m_computation->setting()->value(ProblemSetting::View_ShowContourView).toBool()))
    {
        Agros2D::log()->printMessage(tr("Post View"), tr("Contour view (%1)").arg(m_computation->setting()->value(ProblemSetting::View_ContourVariable).toString()));

        QString variableName = m_computation->setting()->value(ProblemSetting::View_ContourVariable).toString();
        Module::LocalVariable variable = m_activeViewField->localVariable(m_computation->config()->coordinateType(), variableName);

        m_contourValues.clear();

        std::shared_ptr<PostDataOut> data_out;

        if (variable.isScalar())
            data_out = viewScalarFilter(m_activeViewField->localVariable(m_computation->config()->coordinateType(),
                                                                         m_computation->setting()->value(ProblemSetting::View_ContourVariable).toString()),
                                        PhysicFieldVariableComp_Scalar);

        else
            data_out = viewScalarFilter(m_activeViewField->localVariable(m_computation->config()->coordinateType(),
                                                                         m_computation->setting()->value(ProblemSetting::View_ContourVariable).toString()),
                                        PhysicFieldVariableComp_Magnitude);

        data_out->compute_nodes(m_contourValues, (m_activeViewField->hasDeformableShape() && m_computation->setting()->value(ProblemSetting::View_DeformContour).toBool()));
    }
}

void PostDeal::processRangeScalar()
{
    if (m_computation->setting()->value(ProblemSetting::View_ScalarRangeAuto).toBool())
    {
        m_computation->setting()->setValue(ProblemSetting::View_ScalarRangeMin, 0.0);
        m_computation->setting()->setValue(ProblemSetting::View_ScalarRangeMax, 0.0);
    }

    if ((m_computation->isSolved()) && (m_activeViewField)
            && ((m_computation->setting()->value(ProblemSetting::View_ShowScalarView).toBool())
                || (((SceneViewPost3DMode) m_computation->setting()->value(ProblemSetting::View_ScalarView3DMode).toInt()) == SceneViewPost3DMode_ScalarView3D)))
    {
        Agros2D::log()->printMessage(tr("Post View"), tr("Scalar view (%1)").arg(m_computation->setting()->value(ProblemSetting::View_ScalarVariable).toString()));

        std::shared_ptr<PostDataOut> data_out = viewScalarFilter(m_activeViewField->localVariable(m_computation->config()->coordinateType(),
                                                                                                  m_computation->setting()->value(ProblemSetting::View_ScalarVariable).toString()),
                                                                 (PhysicFieldVariableComp) m_computation->setting()->value(ProblemSetting::View_ScalarVariableComp).toInt());
        data_out->compute_nodes(m_scalarValues, (m_activeViewField->hasDeformableShape() && m_computation->setting()->value(ProblemSetting::View_DeformContour).toBool()));

        if (m_computation->setting()->value(ProblemSetting::View_ScalarRangeAuto).toBool())
        {
            m_computation->setting()->setValue(ProblemSetting::View_ScalarRangeMin, data_out->min());
            m_computation->setting()->setValue(ProblemSetting::View_ScalarRangeMax, data_out->max());
        }
    }
}

void PostDeal::processRangeVector()
{
    if ((m_computation->isSolved()) && (m_activeViewField) && (m_computation->setting()->value(ProblemSetting::View_ShowVectorView).toBool()))
    {
        bool contains = false;
        foreach (Module::LocalVariable variable, m_activeViewField->viewVectorVariables(m_computation->config()->coordinateType()))
        {
            if (variable.id() == m_computation->setting()->value(ProblemSetting::View_VectorVariable).toString())
            {
                contains = true;
                break;
            }
        }

        Agros2D::log()->printMessage(tr("Post View"), tr("Vector view (%1)").arg(m_computation->setting()->value(ProblemSetting::View_VectorVariable).toString()));

        std::shared_ptr<PostDataOut> data_outX = viewScalarFilter(m_activeViewField->localVariable(m_computation->config()->coordinateType(),
                                                                                                   m_computation->setting()->value(ProblemSetting::View_VectorVariable).toString()),
                                                                  PhysicFieldVariableComp_X);

        std::shared_ptr<PostDataOut> data_outY = viewScalarFilter(m_activeViewField->localVariable(m_computation->config()->coordinateType(),
                                                                                                   m_computation->setting()->value(ProblemSetting::View_VectorVariable).toString()),
                                                                  PhysicFieldVariableComp_Y);

        data_outX->compute_nodes(m_vectorXValues);
        data_outY->compute_nodes(m_vectorYValues);
    }
}

void PostDeal::clearView()
{
    m_isProcessed = false;

    m_contourValues.clear();
    m_scalarValues.clear();
    m_vectorXValues.clear();
    m_vectorYValues.clear();
}

void PostDeal::refresh()
{
    m_computation->setIsPostprocessingRunning();
    clearView();

    if (m_computation->isSolved())
    {
        problemSolved();
        processSolved();
    }

    m_isProcessed = true;
    emit processed();
    m_computation->setIsPostprocessingRunning(false);
}

void PostDeal::clear()
{
    clearView();

    m_activeViewField = NULL;
    m_activeTimeStep = NOT_FOUND_SO_FAR;
    m_activeAdaptivityStep = NOT_FOUND_SO_FAR;
}

void PostDeal::problemMeshed()
{
    if (!m_activeViewField)
    {
        setActiveViewField(m_computation->fieldInfos().begin().value());
    }
}

void PostDeal::problemSolved()
{
    if (!m_activeViewField)
        setActiveViewField(m_computation->fieldInfos().begin().value());

    // time step
    int lastTimeStep = m_computation->solutionStore()->lastTimeStep(m_activeViewField);
    if (m_activeTimeStep == NOT_FOUND_SO_FAR || activeTimeStep() > lastTimeStep)
        setActiveTimeStep(lastTimeStep);

    // adaptive step
    int lastAdaptivityStep = m_computation->solutionStore()->lastAdaptiveStep(m_activeViewField, m_activeTimeStep);
    if (m_activeAdaptivityStep == NOT_FOUND_SO_FAR || activeAdaptivityStep() > lastAdaptivityStep)
        setActiveAdaptivityStep(lastAdaptivityStep);
}

void PostDeal::processSolved()
{
    // update time functions
    if (m_computation->isTransient())
        Module::updateTimeFunctions(m_computation, m_computation->timeStepToTotalTime(activeTimeStep()));

    FieldSolutionID fsid(activeViewField()->fieldId(), activeTimeStep(), activeAdaptivityStep());
    if (m_computation->solutionStore()->contains(fsid))
    {
        // add icon to progress
        Agros2D::log()->addIcon(icon("scene-post2d"), tr("Postprocessor"));

        processRangeContour();
        processRangeScalar();
        processRangeVector();
    }
}


std::shared_ptr<PostDataOut> PostDeal::viewScalarFilter(Module::LocalVariable physicFieldVariable,
                                                        PhysicFieldVariableComp physicFieldVariableComp)
{    
    // QTime time;
    // time.start();

    // update time functions
    if (m_computation->isTransient())
        Module::updateTimeFunctions(m_computation, m_computation->timeStepToTotalTime(activeTimeStep()));

    std::shared_ptr<dealii::DataPostprocessorScalar<2> > post = activeViewField()->plugin()->filter(m_computation,
                                                                                                    activeViewField(),
                                                                                                    activeTimeStep(),
                                                                                                    activeAdaptivityStep(),
                                                                                                    physicFieldVariable.id(),
                                                                                                    physicFieldVariableComp);

    // This effectively deallocates the previous pointer.
    this->m_post = post;

    MultiArray ma = activeMultiSolutionArray();

    std::shared_ptr<PostDataOut> data_out = std::shared_ptr<PostDataOut>(new PostDataOut(activeViewField(), m_computation));
    data_out->attach_dof_handler(ma.doFHandler());
    data_out->add_data_vector(ma.solution(), *post);
    // deform shape
    if (m_activeViewField->hasDeformableShape())
    {
        std::vector<std::string> solution_names;
        solution_names.push_back ("x_displacement");
        solution_names.push_back ("y_displacement");

        data_out->add_data_vector(ma.solution(), solution_names);
    }
    data_out->build_patches(2);

    // qDebug() << "process - build patches (" << time.elapsed() << "ms )";

    return data_out;
}

void PostDeal::setActiveViewField(FieldInfo* fieldInfo)
{
    // previous active field
    FieldInfo* previousActiveViewField = m_activeViewField;

    // set new field
    m_activeViewField = fieldInfo;

    // check for different field
    if (previousActiveViewField != fieldInfo)
    {
        setActiveTimeStep(NOT_FOUND_SO_FAR);
        setActiveAdaptivityStep(NOT_FOUND_SO_FAR);

        // set default variables
        Module::LocalVariable scalarVariable = m_activeViewField->defaultViewScalarVariable(m_computation->config()->coordinateType());
        Module::LocalVariable vectorVariable = m_activeViewField->defaultViewVectorVariable(m_computation->config()->coordinateType());

        QString scalarVariableDefault = scalarVariable.id();
        PhysicFieldVariableComp scalarVariableCompDefault = scalarVariable.isScalar() ? PhysicFieldVariableComp_Scalar : PhysicFieldVariableComp_Magnitude;
        QString contourVariableDefault = scalarVariable.id();
        QString vectorVariableDefault = vectorVariable.id();

        foreach (Module::LocalVariable local, m_activeViewField->viewScalarVariables(m_computation->config()->coordinateType()))
        {
            if (m_computation->setting()->value(ProblemSetting::View_ScalarVariable).toString() == local.id())
            {
                scalarVariableDefault = m_computation->setting()->value(ProblemSetting::View_ScalarVariable).toString();
                scalarVariableCompDefault = (PhysicFieldVariableComp) m_computation->setting()->value(ProblemSetting::View_ScalarVariableComp).toInt();
            }
            if (m_computation->setting()->value(ProblemSetting::View_ContourVariable).toString() == local.id())
            {
                contourVariableDefault = m_computation->setting()->value(ProblemSetting::View_ContourVariable).toString();
            }
        }
        foreach (Module::LocalVariable local, m_activeViewField->viewScalarVariables(m_computation->config()->coordinateType()))
        {
            if (m_computation->setting()->value(ProblemSetting::View_VectorVariable).toString() == local.id())
            {
                vectorVariableDefault = m_computation->setting()->value(ProblemSetting::View_VectorVariable).toString();
            }
        }

        m_computation->setting()->setValue(ProblemSetting::View_ScalarVariable, scalarVariableDefault);
        m_computation->setting()->setValue(ProblemSetting::View_ScalarVariableComp, scalarVariableCompDefault);
        m_computation->setting()->setValue(ProblemSetting::View_ContourVariable, contourVariableDefault);
        m_computation->setting()->setValue(ProblemSetting::View_VectorVariable, vectorVariableDefault);

        // order component
        m_computation->setting()->setValue(ProblemSetting::View_OrderComponent, 1);
    }
}

void PostDeal::setActiveTimeStep(int ts)
{
    m_activeTimeStep = ts;
}

void PostDeal::setActiveAdaptivityStep(int as)
{
    m_activeAdaptivityStep = as;
}

MultiArray PostDeal::activeMultiSolutionArray()
{
    FieldSolutionID fsid(activeViewField()->fieldId(), activeTimeStep(), activeAdaptivityStep());
    if (m_computation->solutionStore()->contains(fsid))
        return m_computation->solutionStore()->multiArray(fsid);
    else
        assert(0);
}

// ************************************************************************************************

SceneViewPostInterface::SceneViewPostInterface(QWidget *parent)
    : SceneViewCommon(parent),
      m_textureScalar(0)
{
}

void SceneViewPostInterface::initializeGL()
{
    clearGLLists();

    SceneViewCommon::initializeGL();
}

const double* SceneViewPostInterface::paletteColor(double x) const
{
    switch ((PaletteType) m_computation->setting()->value(ProblemSetting::View_PaletteType).toInt())
    {
    case Palette_Inferno:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= PALETTEENTRIES;
        int n = (int) x;
        return paletteDataInferno[n];
    }
        break;
    case Palette_Parula:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= PALETTEENTRIES;
        int n = (int) x;
        return paletteDataParula[n];
    }
        break;
    case Palette_Jet:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= PALETTEENTRIES;
        int n = (int) x;
        return paletteDataJet[n];
    }
        break;
    case Palette_HSV:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= PALETTEENTRIES;
        int n = (int) x;
        return paletteDataHSV[n];
    }
        break;
    case Palette_BWAsc:
    {
        static double color[3];
        color[0] = color[1] = color[2] = x;
        return color;
    }
        break;
    case Palette_BWDesc:
    {
        static double color[3];
        color[0] = color[1] = color[2] = 1.0 - x;
        return color;
    }
        break;
    case Palette_Viridis:
    default:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= PALETTEENTRIES;
        int n = (int) x;
        return paletteDataViridis[n];
    }
        break;
    }
}

const double* SceneViewPostInterface::paletteColorOrder(int n) const
{
    switch ((PaletteType) m_computation->setting()->value(ProblemSetting::View_OrderPaletteOrderType).toInt())
    {
    case Palette_Jet:
        return paletteOrderJet[n];
    case Palette_Parula:
        return paletteOrderParula[n];
    case Palette_Inferno:
        return paletteOrderInferno[n];
    case Palette_Viridis:
        return paletteOrderViridis[n];
    case Palette_HSV:
        return paletteOrderHSV[n];
    case Palette_BWAsc:
        return paletteOrderBWAsc[n];
    case Palette_BWDesc:
        return paletteOrderBWDesc[n];
    default:
        qWarning() << QString("Undefined: %1.").arg(m_computation->setting()->value(ProblemSetting::View_OrderPaletteOrderType).toInt());
        return NULL;
    }
}

void SceneViewPostInterface::paletteCreate()
{
    int paletteSteps = m_computation->setting()->value(ProblemSetting::View_PaletteFilter).toBool()
            ? 100 : m_computation->setting()->value(ProblemSetting::View_PaletteSteps).toInt();

    unsigned char palette[256][3];
    for (int i = 0; i < paletteSteps; i++)
    {
        const double* color = paletteColor((double) i / paletteSteps);
        palette[i][0] = (unsigned char) (color[0] * 255);
        palette[i][1] = (unsigned char) (color[1] * 255);
        palette[i][2] = (unsigned char) (color[2] * 255);
    }
    for (int i = paletteSteps; i < 256; i++)
        memcpy(palette[i], palette[paletteSteps-1], 3);

    makeCurrent();
    if (glIsTexture(m_textureScalar))
        glDeleteTextures(1, &m_textureScalar);
    glGenTextures(1, &m_textureScalar);

    glBindTexture(GL_TEXTURE_1D, m_textureScalar);
    glTexParameteri(GL_TEXTURE_1D, GL_GENERATE_MIPMAP, GL_TRUE);
    if (m_computation->setting()->value(ProblemSetting::View_PaletteFilter).toBool())
    {
#ifdef Q_WS_WIN
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#else
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
#endif
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
#ifdef Q_WS_WIN
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#else
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
#endif
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    glTexImage1D(GL_TEXTURE_1D, 0, 3, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, palette);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);

    // adjust palette
    if (m_computation->setting()->value(ProblemSetting::View_PaletteFilter).toBool())
    {
        m_texScale = (double) (paletteSteps-1) / 256.0;
        m_texShift = 0.5 / 256.0;
    }
    else
    {
        m_texScale = (double) paletteSteps / 256.0;
        m_texShift = 0.0;
    }
}

void SceneViewPostInterface::paintScalarFieldColorBar(double min, double max)
{
    if (!m_computation->isSolved() || !m_computation->setting()->value(ProblemSetting::View_ShowScalarColorBar).toBool()) return;

    loadProjectionViewPort();

    glScaled(2.0 / width(), 2.0 / height(), 1.0);
    glTranslated(-width() / 2.0, -height() / 2.0, 0.0);

    // dimensions
    int textWidth = (m_charDataPost[GLYPH_M].x1 - m_charDataPost[GLYPH_M].x0)
            * (QString::number(-1.0, 'e', m_computation->setting()->value(ProblemSetting::View_ScalarDecimalPlace).toInt()).length() + 1);
    int textHeight = 2 * (m_charDataPost[GLYPH_M].y1 - m_charDataPost[GLYPH_M].y0);
    Point scaleSize = Point(45.0 + textWidth, 20*textHeight); // height() - 20.0
    Point scaleBorder = Point(10.0, (Agros2D::configComputer()->value(Config::Config_ShowRulers).toBool()) ? 1.8 * textHeight : 10.0);
    double scaleLeft = (width() - (45.0 + textWidth));
    int numTicks = 11;

    // blended rectangle
    drawBlend(Point(scaleLeft, scaleBorder.y), Point(scaleLeft + scaleSize.x - scaleBorder.x, scaleBorder.y + scaleSize.y),
              0.91, 0.91, 0.91);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // palette border
    glColor3d(0.0, 0.0, 0.0);
    glBegin(GL_QUADS);
    glVertex2d(scaleLeft + 30.0, scaleBorder.y + scaleSize.y - 50.0);
    glVertex2d(scaleLeft + 10.0, scaleBorder.y + scaleSize.y - 50.0);
    glVertex2d(scaleLeft + 10.0, scaleBorder.y + 10.0);
    glVertex2d(scaleLeft + 30.0, scaleBorder.y + 10.0);
    glEnd();

    glDisable(GL_POLYGON_OFFSET_FILL);

    // palette
    glEnable(GL_TEXTURE_1D);
    glBindTexture(GL_TEXTURE_1D, m_textureScalar);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    glBegin(GL_QUADS);
    if (fabs(min - max) > EPS_ZERO)
        glTexCoord1d(m_texScale + m_texShift);
    else
        glTexCoord1d(m_texShift);
    glVertex2d(scaleLeft + 28.0, scaleBorder.y + scaleSize.y - 52.0);
    glVertex2d(scaleLeft + 12.0, scaleBorder.y + scaleSize.y - 52.0);
    glTexCoord1d(m_texShift);
    glVertex2d(scaleLeft + 12.0, scaleBorder.y + 12.0);
    glVertex2d(scaleLeft + 28.0, scaleBorder.y + 12.0);
    glEnd();

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glDisable(GL_TEXTURE_1D);

    // ticks
    glLineWidth(1.0);
    glBegin(GL_LINES);
    for (int i = 1; i < numTicks; i++)
    {
        double tickY = (scaleSize.y - 60.0) / (numTicks - 1.0);

        glVertex2d(scaleLeft + 10.0, scaleBorder.y + scaleSize.y - 49.0 - i*tickY);
        glVertex2d(scaleLeft + 15.0, scaleBorder.y + scaleSize.y - 49.0 - i*tickY);
        glVertex2d(scaleLeft + 25.0, scaleBorder.y + scaleSize.y - 49.0 - i*tickY);
        glVertex2d(scaleLeft + 30.0, scaleBorder.y + scaleSize.y - 49.0 - i*tickY);
    }
    glEnd();

    // line
    glLineWidth(1.0);
    glBegin(GL_LINES);
    glVertex2d(scaleLeft + 5.0, scaleBorder.y + scaleSize.y - 31.0);
    glVertex2d(scaleLeft + scaleSize.x - 15.0, scaleBorder.y + scaleSize.y - 31.0);
    glEnd();

    // labels
    for (int i = 1; i < numTicks+1; i++)
    {
        double value = 0.0;
        if (!m_computation->setting()->value(ProblemSetting::View_ScalarRangeLog).toBool())
            value = min + (double) (i-1) / (numTicks-1) * (max - min);
        else
            value = min + (double) pow((double) m_computation->setting()->value(ProblemSetting::View_ScalarRangeBase).toInt(),
                                       ((i-1) / (numTicks-1)))/m_computation->setting()->value(ProblemSetting::View_ScalarRangeBase).toInt() * (max - min);

        if (fabs(value) < EPS_ZERO) value = 0.0;
        double tickY = (scaleSize.y - 60.0) / (numTicks - 1.0);

        printPostAt(scaleLeft + 33.0 + ((value >= 0.0) ? (m_charDataPost[GLYPH_M].x1 - m_charDataPost[GLYPH_M].x0) : 0.0),
                    scaleBorder.y + 10.0 + (i-1)*tickY - textHeight / 4.0,
                    QString::number(value, 'e', m_computation->setting()->value(ProblemSetting::View_ScalarDecimalPlace).toInt()));
    }

    // variable
    Module::LocalVariable localVariable = m_computation->postDeal()->activeViewField()->localVariable(m_computation->config()->coordinateType(),
                                                                                                      m_computation->setting()->value(ProblemSetting::View_ScalarVariable).toString());
    QString str = QString("%1 (%2)").
            arg(m_computation->setting()->value(ProblemSetting::View_ScalarVariable).toString().isEmpty() ? "" : localVariable.shortname()).
            arg(m_computation->setting()->value(ProblemSetting::View_ScalarVariable).toString().isEmpty() ? "" : localVariable.unit());

    printPostAt(scaleLeft + scaleSize.x / 2.0 - (m_charDataPost[GLYPH_M].x1 - m_charDataPost[GLYPH_M].x0) * str.count() / 2.0,
                scaleBorder.y + scaleSize.y - 20.0,
                str);
}
