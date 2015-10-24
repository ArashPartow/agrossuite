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

#include "postprocessorview_chart.h"
#include "postprocessorview.h"

#include "chartdialog.h"

#include "util/global.h"

#include "gui/lineeditdouble.h"
#include "gui/groupbox.h"
#include "gui/common.h"
#include "gui/physicalfield.h"

#include "scene.h"
#include "scenemarker.h"
#include "sceneview_geometry_chart.h"
#include "pythonlab/pythonengine_agros.h"

#include "solver/module.h"

#include "solver/field.h"
#include "solver/problem.h"
#include "solver/problem_config.h"
#include "solver/solutionstore.h"

#include "util/constants.h"

#include "qcustomplot/qcustomplot.h"

QList<Point> ChartLine::getPoints()
{
    if (numberOfPoints == 0)
        return QList<Point>();

    QList<Point> points;
    points.reserve(numberOfPoints);

    double dx = (end.x - start.x) / (numberOfPoints - 1);
    double dy = (end.y - start.y) / (numberOfPoints - 1);

    for (int i = 0; i < numberOfPoints; i++)
        if (reverse)
            points.insert(0, Point(start.x + i*dx, start.y + i*dy));
        else
            points.append(Point(start.x + i*dx, start.y + i*dy));

    return points;
}

// **************************************************************************************************

PostprocessorSceneChartWidget::PostprocessorSceneChartWidget(PostprocessorWidget *postprocessorWidget, SceneViewChart *sceneChart)
    : PostprocessorSceneWidget(postprocessorWidget), m_sceneChart(sceneChart)
{
    setWindowIcon(icon("chart"));
    setObjectName("PostprocessorChartWidget");

    createControls();
}

void PostprocessorSceneChartWidget::createControls()
{
    // variable
    cmbFieldVariable = new QComboBox();
    connect(cmbFieldVariable, SIGNAL(currentIndexChanged(int)), this, SLOT(doFieldVariable(int)));

    // component
    cmbFieldVariableComp = new QComboBox();

    QFormLayout *layoutVariable = new QFormLayout();
    layoutVariable->addRow(tr("Variable:"), cmbFieldVariable);
    layoutVariable->addRow(tr("Component:"), cmbFieldVariableComp);

    QGroupBox *grpVariable = new QGroupBox(tr("Variable"));
    grpVariable->setLayout(layoutVariable);

    // viewer
    geometryViewer = new SceneViewPreprocessorChart(this);
    geometryViewer->setMinimumHeight(150);
    // geometryViewer->setMaximumHeight(150);

    QVBoxLayout *layoutChart = new QVBoxLayout();
    layoutChart->addWidget(geometryViewer);

    QGroupBox *grpChart = new QGroupBox(tr("Line preview"));
    grpChart->setLayout(layoutChart);

    btnSaveImage = new QPushButton();
    btnSaveImage->setDefault(false);
    btnSaveImage->setEnabled(false);
    btnSaveImage->setText(tr("Save image"));
    connect(btnSaveImage, SIGNAL(clicked()), m_sceneChart, SLOT(doSaveImage()));

    btnExportData = new QPushButton();
    btnExportData->setDefault(false);
    btnExportData->setEnabled(false);
    btnExportData->setText(tr("Export"));
    connect(btnExportData, SIGNAL(clicked()), m_sceneChart, SLOT(doExportData()));

    // geometry
    lblStartX = new QLabel("X:");
    lblStartY = new QLabel("Y:");
    lblEndX = new QLabel("X:");
    lblEndY = new QLabel("Y:");

    txtStartX = new LineEditDouble(0.0);
    txtStartY = new LineEditDouble(0.0);
    connect(txtStartX, SIGNAL(textChanged(QString)), this, SLOT(createChartLine()));
    connect(txtStartY, SIGNAL(textChanged(QString)), this, SLOT(createChartLine()));

    txtEndX = new LineEditDouble(0.0);
    txtEndY = new LineEditDouble(0.0);
    connect(txtEndX, SIGNAL(textChanged(QString)), this, SLOT(createChartLine()));
    connect(txtEndY, SIGNAL(textChanged(QString)), this, SLOT(createChartLine()));

    // start
    QGridLayout *layoutStart = new QGridLayout();
    layoutStart->addWidget(lblStartX, 0, 0);
    layoutStart->addWidget(txtStartX, 0, 1);
    layoutStart->addWidget(lblStartY, 0, 2);
    layoutStart->addWidget(txtStartY, 0, 3);

    QGroupBox *grpStart = new QGroupBox(tr("Start"));
    grpStart->setLayout(layoutStart);

    // end
    QGridLayout *layoutEnd = new QGridLayout();
    layoutEnd->addWidget(lblEndX, 0, 0);
    layoutEnd->addWidget(txtEndX, 0, 1);
    layoutEnd->addWidget(lblEndY, 0, 2);
    layoutEnd->addWidget(txtEndY, 0, 3);

    QGroupBox *grpEnd = new QGroupBox(tr("End"));
    grpEnd->setLayout(layoutEnd);

    // x - axis
    radHorizontalAxisLength = new QRadioButton(tr("Length"));
    radHorizontalAxisLength->setChecked(true);
    radHorizontalAxisX = new QRadioButton("X");
    radHorizontalAxisY = new QRadioButton("Y");

    QButtonGroup *axisGroup = new QButtonGroup();
    axisGroup->addButton(radHorizontalAxisLength);
    axisGroup->addButton(radHorizontalAxisX);
    axisGroup->addButton(radHorizontalAxisY);

    /*
    connect(radAxisLength, SIGNAL(clicked()), this, SLOT(doPlot()));
    connect(radAxisX, SIGNAL(clicked()), this, SLOT(doPlot()));
    connect(radAxisY, SIGNAL(clicked()), this, SLOT(doPlot()));
    */

    // axis
    QHBoxLayout *layoutAxis = new QHBoxLayout();
    layoutAxis->addWidget(radHorizontalAxisLength);
    layoutAxis->addWidget(radHorizontalAxisX);
    layoutAxis->addWidget(radHorizontalAxisY);

    QGroupBox *grpAxis = new QGroupBox(tr("Horizontal axis"));
    grpAxis->setLayout(layoutAxis);

    // axis points and time step
    txtHorizontalAxisPoints = new QSpinBox(this);
    txtHorizontalAxisPoints->setMinimum(2);
    txtHorizontalAxisPoints->setMaximum(500);
    txtHorizontalAxisPoints->setValue(200);
    chkHorizontalAxisReverse = new QCheckBox(tr("Reverse"));
    //connect(chkAxisPointsReverse, SIGNAL(clicked()), this, SLOT(doPlot()));

    // timestep
    QGridLayout *layoutAxisPointsAndTimeStep = new QGridLayout();
    layoutAxisPointsAndTimeStep->setColumnStretch(1, 1);
    layoutAxisPointsAndTimeStep->addWidget(new QLabel(tr("Points:")), 0, 0);
    layoutAxisPointsAndTimeStep->addWidget(txtHorizontalAxisPoints, 0, 1);
    layoutAxisPointsAndTimeStep->addWidget(chkHorizontalAxisReverse, 1, 0, 1, 2);

    QGroupBox *grpAxisPointsAndTimeStep = new QGroupBox(tr("Points and time step"), this);
    grpAxisPointsAndTimeStep->setLayout(layoutAxisPointsAndTimeStep);

    // time
    lblPointX = new QLabel("X:");
    lblPointY = new QLabel("Y:");

    txtTimeX = new LineEditDouble(0.0);
    txtTimeY = new LineEditDouble(0.0);
    connect(txtTimeX, SIGNAL(textChanged(QString)), this, SLOT(createChartLine()));
    connect(txtTimeY, SIGNAL(textChanged(QString)), this, SLOT(createChartLine()));

    QGridLayout *layoutTime = new QGridLayout();
    layoutTime->addWidget(lblPointX, 0, 0);
    layoutTime->addWidget(txtTimeX, 0, 1);
    layoutTime->addWidget(lblPointY, 1, 0);
    layoutTime->addWidget(txtTimeY, 1, 1);

    QGroupBox *grpTime = new QGroupBox(tr("Point"));
    grpTime->setLayout(layoutTime);

    // button bar
    QHBoxLayout *layoutButton = new QHBoxLayout();
    layoutButton->setContentsMargins(2, 0, 0, 0);
    layoutButton->addStretch();
    layoutButton->addWidget(btnSaveImage);
    layoutButton->addWidget(btnExportData);

    QWidget *widButton = new QWidget();
    widButton->setLayout(layoutButton);

    // controls geometry
    widGeometry = new QWidget();
    QVBoxLayout *controlsGeometryLayout = new QVBoxLayout();
    widGeometry->setLayout(controlsGeometryLayout);
    controlsGeometryLayout->addWidget(grpStart);
    controlsGeometryLayout->addWidget(grpEnd);
    controlsGeometryLayout->addWidget(grpAxis);
    controlsGeometryLayout->addWidget(grpAxisPointsAndTimeStep);
    controlsGeometryLayout->addStretch();

    // controls time
    widTime = new QWidget();
    QVBoxLayout *controlsTimeLayout = new QVBoxLayout();
    widTime->setLayout(controlsTimeLayout);
    controlsTimeLayout->addWidget(grpTime);
    controlsTimeLayout->addStretch();

    tbxAnalysisType = new QTabWidget();
    tbxAnalysisType->addTab(widGeometry, icon(""), tr("Geometry"));
    tbxAnalysisType->addTab(widTime, icon(""), tr("Time"));

    // controls
    QVBoxLayout *controlsLayout = new QVBoxLayout();
    // controlsLayout->setContentsMargins(0, 0, 0, 0);
    controlsLayout->addWidget(grpVariable);
    controlsLayout->addWidget(tbxAnalysisType, 1);
    controlsLayout->addWidget(grpChart, 1);
    // controlsLayout->addStretch(1);

    QWidget *widget = new QWidget(this);
    widget->setLayout(controlsLayout);

    QScrollArea *widgetArea = new QScrollArea();
    widgetArea->setFrameShape(QFrame::NoFrame);
    widgetArea->setWidgetResizable(true);
    widgetArea->setWidget(widget);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(2, 2, 2, 3);
    layout->addWidget(widgetArea);
    layout->addWidget(widButton);

    setLayout(layout);
}

void PostprocessorSceneChartWidget::doFieldVariable(int index)
{
    if (!(m_postprocessorWidget->computation() && m_postprocessorWidget->fieldWidget() && m_postprocessorWidget->fieldWidget()->selectedField()))
        return;

    Module::LocalVariable physicFieldVariable = m_postprocessorWidget->fieldWidget()->selectedField()->localVariable(m_postprocessorWidget->computation()->config()->coordinateType(), cmbFieldVariable->itemData(index).toString());

    cmbFieldVariableComp->clear();
    if (physicFieldVariable.isScalar())
    {
        cmbFieldVariableComp->addItem(tr("Scalar"), PhysicFieldVariableComp_Scalar);
    }
    else
    {
        cmbFieldVariableComp->addItem(tr("Magnitude"), PhysicFieldVariableComp_Magnitude);
        cmbFieldVariableComp->addItem(m_postprocessorWidget->computation()->config()->labelX(), PhysicFieldVariableComp_X);
        cmbFieldVariableComp->addItem(m_postprocessorWidget->computation()->config()->labelY(), PhysicFieldVariableComp_Y);
    }

    if (cmbFieldVariableComp->currentIndex() == -1)
        cmbFieldVariableComp->setCurrentIndex(0);
}

void PostprocessorSceneChartWidget::createChartLine()
{
    ChartLine line;

    if (tbxAnalysisType->currentWidget() == widGeometry)
    {
        line = ChartLine(Point(txtStartX->value(), txtStartY->value()),
                         Point(txtEndX->value(), txtEndY->value()),
                         txtHorizontalAxisPoints->value(),
                         chkHorizontalAxisReverse->isChecked());
    }
    if (tbxAnalysisType->currentWidget() == widTime)
    {
        line = ChartLine(Point(txtTimeX->value(), txtTimeY->value()),
                         Point(txtTimeX->value(), txtTimeY->value()),
                         0.0,
                         0);
    }

    geometryViewer->setChartLine(line);
    geometryViewer->doZoomBestFit();
}

void PostprocessorSceneChartWidget::refresh()
{
    if (!(m_postprocessorWidget->computation() && m_postprocessorWidget->fieldWidget() && m_postprocessorWidget->fieldWidget()->selectedField()))
        return;

    fillComboBoxScalarVariable(m_postprocessorWidget->computation()->config()->coordinateType(), m_postprocessorWidget->fieldWidget()->selectedField(), cmbFieldVariable);
    doFieldVariable(cmbFieldVariable->currentIndex());

    // correct labels
    lblStartX->setText(m_postprocessorWidget->computation()->config()->labelX() + ":");
    lblStartY->setText(m_postprocessorWidget->computation()->config()->labelY() + ":");
    lblEndX->setText(m_postprocessorWidget->computation()->config()->labelX() + ":");
    lblEndY->setText(m_postprocessorWidget->computation()->config()->labelY() + ":");
    lblPointX->setText(m_postprocessorWidget->computation()->config()->labelX() + ":");
    lblPointY->setText(m_postprocessorWidget->computation()->config()->labelY() + ":");
    radHorizontalAxisX->setText(m_postprocessorWidget->computation()->config()->labelX());
    radHorizontalAxisY->setText(m_postprocessorWidget->computation()->config()->labelY());

    if (m_postprocessorWidget->computation()->isTransient())
    {
        widTime->setEnabled(true);
    }
    else
    {
        widTime->setEnabled(false);
        widGeometry->setEnabled(true);
        tbxAnalysisType->setCurrentWidget(widGeometry);
    }
}

void PostprocessorSceneChartWidget::load()
{
    if (!(m_postprocessorWidget->computation() && m_postprocessorWidget->fieldWidget() && m_postprocessorWidget->fieldWidget()->selectedField()))
        return;

    txtStartX->setValue(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ChartStartX).toDouble());
    txtStartY->setValue(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ChartStartY).toDouble());
    txtEndX->setValue(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ChartEndX).toDouble());
    txtEndY->setValue(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ChartEndY).toDouble());
    txtTimeX->setValue(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ChartTimeX).toDouble());
    txtTimeY->setValue(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ChartTimeY).toDouble());
    radHorizontalAxisX->setChecked((ChartAxisType) m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ChartHorizontalAxis).toInt() == ChartAxis_X);
    radHorizontalAxisY->setChecked((ChartAxisType) m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ChartHorizontalAxis).toInt() == ChartAxis_Y);
    radHorizontalAxisLength->setChecked((ChartAxisType) m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ChartHorizontalAxis).toInt() == ChartAxis_Length);
    txtHorizontalAxisPoints->setValue(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ChartHorizontalAxisPoints).toInt());
    chkHorizontalAxisReverse->setChecked(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ChartHorizontalAxisReverse).toBool());

    cmbFieldVariable->setCurrentIndex(cmbFieldVariable->findData(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ChartVariable).toString()));
    if (cmbFieldVariable->count() > 0 && cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()) != QVariant::Invalid)
    {
        m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ChartVariable, cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toString());
        doFieldVariable(cmbFieldVariable->currentIndex());

        cmbFieldVariableComp->setCurrentIndex(cmbFieldVariableComp->findData((PhysicFieldVariableComp) m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ChartVariableComp).toInt()));
        m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ChartVariableComp, (PhysicFieldVariableComp) cmbFieldVariableComp->itemData(cmbFieldVariableComp->currentIndex()).toInt());
    }
    if (cmbFieldVariable->currentIndex() == -1 && cmbFieldVariable->count() > 0)
    {
        // set first variable
        cmbFieldVariable->setCurrentIndex(0);
    }

    if ((ChartMode) m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ChartMode).toInt() == ChartMode_Geometry)
        tbxAnalysisType->setCurrentWidget(widGeometry);
    else if ((ChartMode) m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ChartMode).toInt() == ChartMode_Time)
        tbxAnalysisType->setCurrentWidget(widTime);

    btnSaveImage->setEnabled(m_sceneChart->chart()->graph()->data()->size() > 0);
    btnExportData->setEnabled(m_sceneChart->chart()->graph()->data()->size() > 0);
}

void PostprocessorSceneChartWidget::save()
{
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ChartStartX, txtStartX->value());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ChartStartY, txtStartY->value());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ChartEndX, txtEndX->value());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ChartEndY, txtEndY->value());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ChartTimeX, txtTimeX->value());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ChartTimeY, txtTimeY->value());
    if (radHorizontalAxisX->isChecked())
        m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ChartHorizontalAxis, ChartAxis_X);
    else if (radHorizontalAxisY->isChecked())
        m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ChartHorizontalAxis, ChartAxis_Y);
    else if (radHorizontalAxisLength->isChecked())
        m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ChartHorizontalAxis, ChartAxis_Length);
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ChartHorizontalAxisReverse, chkHorizontalAxisReverse->isChecked());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ChartHorizontalAxisPoints, txtHorizontalAxisPoints->value());
    if (tbxAnalysisType->currentWidget() == widGeometry)
        m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ChartMode, ChartMode_Geometry);
    else if (tbxAnalysisType->currentWidget() == widTime)
        m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ChartMode, ChartMode_Time);
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ChartVariable, cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toString());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ChartVariableComp, (PhysicFieldVariableComp) cmbFieldVariableComp->itemData(cmbFieldVariableComp->currentIndex()).toInt());

    createChartLine();
}
