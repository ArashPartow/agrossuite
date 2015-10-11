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

#include "postprocessorview_mesh.h"
#include "postprocessorview.h"

#include "util/global.h"

#include "gui/lineeditdouble.h"
#include "gui/groupbox.h"
#include "gui/common.h"
#include "gui/physicalfield.h"

#include "scene.h"
#include "scenemarker.h"
#include "sceneview_geometry.h"
#include "sceneview_mesh.h"
#include "sceneview_post2d.h"
#include "sceneview_post3d.h"
#include "pythonlab/pythonengine_agros.h"

#include "solver/module.h"

#include "solver/field.h"
#include "solver/problem.h"
#include "solver/problem_config.h"
#include "solver/solutionstore.h"

#include "util/constants.h"

PostprocessorSceneMeshWidget::PostprocessorSceneMeshWidget(PostprocessorWidget *postprocessorWidget, SceneViewMesh *sceneMesh)
    : PostprocessorSceneWidget(postprocessorWidget), m_sceneMesh(sceneMesh)
{
    setWindowIcon(icon("scene-properties"));
    setObjectName("PostprocessorMeshWidget");

    createControls();

    connect(postprocessorWidget->fieldWidget(), SIGNAL(fieldChanged()), this, SLOT(doField()));
}

void PostprocessorSceneMeshWidget::load()
{
    // show
    chkShowInitialMeshView->setChecked(Agros2D::computation()->setting()->value(ProblemSetting::View_ShowInitialMeshView).toBool());
    chkShowSolutionMeshView->setChecked(Agros2D::computation()->setting()->value(ProblemSetting::View_ShowSolutionMeshView).toBool());
    chkShowOrderView->setChecked(Agros2D::computation()->setting()->value(ProblemSetting::View_ShowOrderView).toBool());
    txtOrderComponent->setValue(Agros2D::computation()->setting()->value(ProblemSetting::View_OrderComponent).toInt());

    // order view
    chkShowOrderColorbar->setChecked(Agros2D::computation()->setting()->value(ProblemSetting::View_ShowOrderColorBar).toBool());
    cmbOrderPaletteOrder->setCurrentIndex(cmbOrderPaletteOrder->findData((PaletteOrderType) Agros2D::computation()->setting()->value(ProblemSetting::View_OrderPaletteOrderType).toInt()));
    chkOrderLabel->setChecked(Agros2D::computation()->setting()->value(ProblemSetting::View_ShowOrderLabel).toBool());
}

void PostprocessorSceneMeshWidget::save()
{
    Agros2D::computation()->setting()->setValue(ProblemSetting::View_ShowInitialMeshView, chkShowInitialMeshView->isChecked());
    Agros2D::computation()->setting()->setValue(ProblemSetting::View_ShowSolutionMeshView, chkShowSolutionMeshView->isChecked());
    Agros2D::computation()->setting()->setValue(ProblemSetting::View_ShowOrderView, chkShowOrderView->isChecked());
    Agros2D::computation()->setting()->setValue(ProblemSetting::View_OrderComponent, txtOrderComponent->value());

    // order view
    Agros2D::computation()->setting()->setValue(ProblemSetting::View_ShowOrderColorBar, chkShowOrderColorbar->isChecked());
    Agros2D::computation()->setting()->setValue(ProblemSetting::View_OrderPaletteOrderType, (PaletteOrderType) cmbOrderPaletteOrder->itemData(cmbOrderPaletteOrder->currentIndex()).toInt());
    Agros2D::computation()->setting()->setValue(ProblemSetting::View_ShowOrderLabel, chkOrderLabel->isChecked());
}

void PostprocessorSceneMeshWidget::createControls()
{
    // layout mesh
    chkShowInitialMeshView = new QCheckBox(tr("Initial mesh"));
    chkShowSolutionMeshView = new QCheckBox(tr("Solution mesh"));
    chkShowOrderView = new QCheckBox(tr("Polynomial order"));
    connect(chkShowOrderView, SIGNAL(clicked()), this, SLOT(refresh()));

    txtOrderComponent = new QSpinBox(this);
    txtOrderComponent->setMinimum(1);

    QGridLayout *gridLayoutMesh = new QGridLayout();
    gridLayoutMesh->addWidget(chkShowInitialMeshView, 0, 0, 1, 2);
    gridLayoutMesh->addWidget(chkShowSolutionMeshView, 1, 0, 1, 2);
    gridLayoutMesh->addWidget(chkShowOrderView, 2, 0, 1, 2);
    gridLayoutMesh->addWidget(new QLabel(tr("Component:")), 3, 0);
    gridLayoutMesh->addWidget(txtOrderComponent, 3, 1);

    QGroupBox *grpShowMesh = new QGroupBox(tr("Mesh"));
    grpShowMesh->setLayout(gridLayoutMesh);

    // layout order
    cmbOrderPaletteOrder = new QComboBox();
    cmbOrderPaletteOrder->addItem(tr("Agros"), PaletteOrder_Agros);
    cmbOrderPaletteOrder->addItem(tr("Jet"), PaletteOrder_Jet);
    cmbOrderPaletteOrder->addItem(tr("Copper"), PaletteOrder_Copper);
    cmbOrderPaletteOrder->addItem(tr("Hot"), PaletteOrder_Hot);
    cmbOrderPaletteOrder->addItem(tr("Cool"), PaletteOrder_Cool);
    cmbOrderPaletteOrder->addItem(tr("Bone"), PaletteOrder_Bone);
    cmbOrderPaletteOrder->addItem(tr("Pink"), PaletteOrder_Pink);
    cmbOrderPaletteOrder->addItem(tr("Spring"), PaletteOrder_Spring);
    cmbOrderPaletteOrder->addItem(tr("Summer"), PaletteOrder_Summer);
    cmbOrderPaletteOrder->addItem(tr("Autumn"), PaletteOrder_Autumn);
    cmbOrderPaletteOrder->addItem(tr("Winter"), PaletteOrder_Winter);
    cmbOrderPaletteOrder->addItem(tr("HSV"), PaletteOrder_HSV);
    cmbOrderPaletteOrder->addItem(tr("B/W ascending"), PaletteOrder_BWAsc);
    cmbOrderPaletteOrder->addItem(tr("B/W descending"), PaletteOrder_BWDesc);

    chkShowOrderColorbar = new QCheckBox(tr("Show colorbar"), this);
    chkOrderLabel = new QCheckBox(tr("Show labels"), this);

    QGridLayout *gridLayoutOrder = new QGridLayout();
    gridLayoutOrder->setColumnStretch(1, 1);
    gridLayoutOrder->addWidget(new QLabel(tr("Palette:")), 0, 0);
    gridLayoutOrder->addWidget(cmbOrderPaletteOrder, 0, 1);
    gridLayoutOrder->addWidget(chkShowOrderColorbar, 1, 0, 1, 2);
    gridLayoutOrder->addWidget(chkOrderLabel, 2, 0, 1, 2);

    QGroupBox *grpShowOrder = new QGroupBox(tr("Polynomial order"));
    grpShowOrder->setLayout(gridLayoutOrder);

    QVBoxLayout *widgetsLayout = new QVBoxLayout();
    widgetsLayout->addWidget(grpShowMesh);
    widgetsLayout->addWidget(grpShowOrder);

    QWidget *widget = new QWidget(this);
    widget->setLayout(widgetsLayout);

    QScrollArea *widgetArea = new QScrollArea();
    widgetArea->setFrameShape(QFrame::NoFrame);
    widgetArea->setWidgetResizable(true);
    widgetArea->setWidget(widget);

    QVBoxLayout *layoutMain = new QVBoxLayout();
    layoutMain->addWidget(widgetArea);
    layoutMain->addStretch(1);

    setLayout(layoutMain);
}

void PostprocessorSceneMeshWidget::doField()
{
    txtOrderComponent->setMaximum(m_postprocessorWidget->fieldWidget()->selectedField()->numberOfSolutions());

    load();
}

void PostprocessorSceneMeshWidget::refresh()
{
    // mesh and order
    chkShowInitialMeshView->setEnabled(Agros2D::computation()->isMeshed());
    chkShowSolutionMeshView->setEnabled(Agros2D::computation()->isSolved());
    chkShowOrderView->setEnabled(Agros2D::computation()->isSolved());
    txtOrderComponent->setEnabled(Agros2D::computation()->isSolved() && (chkShowOrderView->isChecked() || chkShowSolutionMeshView->isChecked()));
}

void PostprocessorSceneMeshWidget::updateControls()
{
    if (Agros2D::computation()->isMeshed())
    {
        if (Agros2D::computation()->isSolved())
        {
            load();
        }
    }

    refresh();
}
