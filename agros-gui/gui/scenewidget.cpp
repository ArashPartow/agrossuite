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

#include "scenewidget.h"
#include "scene.h"
#include "scenemarker.h"

#include "app/sceneview_common.h"
#include "app/sceneview_data.h"
#include "app/scenemarkerselectdialog.h"
#include "app/scenebasicselectdialog.h"
#include "app/chartdialog.h"
#include "app/scenemarkerdialog.h"
#include "gui/infowidget.h"
#include "gui/other.h"

#include "optilab/optilab.h"
#include "logwidget.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"

#include "solver/module.h"
#include "solver/problem.h"

SceneViewWidget::SceneViewWidget(SceneViewCommon *widget, QWidget *parent) : QWidget(parent)
{
    createControls(widget);

    labelLeft(widget->labelView());

    connect(widget, SIGNAL(labelCenter(QString)), this, SLOT(labelCenter(QString)));
    connect(widget, SIGNAL(labelRight(QString)), this, SLOT(labelRight(QString)));
}

SceneViewWidget::SceneViewWidget(SceneViewChart *widget, QWidget *parent) : QWidget(parent)
{
    createControls(widget);
    labelLeft(tr("Chart"));

    connect(widget, SIGNAL(labelCenter(QString)), this, SLOT(labelCenter(QString)));
    connect(widget, SIGNAL(labelRight(QString)), this, SLOT(labelRight(QString)));
}

SceneViewWidget::SceneViewWidget(QWidget *widget, QWidget *parent) : QWidget(parent)
{
    createControls(widget);
    labelLeft(tr("Info"));
}

SceneViewWidget::SceneViewWidget(InfoWidget *widget, QWidget *parent) : QWidget(parent)
{
    createControls(widget);
    labelLeft(tr("Info"));
}

SceneViewWidget::SceneViewWidget(LogView *widget, QWidget *parent) : QWidget(parent)
{
    createControls(widget);
    labelLeft(tr("Application log"));
}

SceneViewWidget::~SceneViewWidget()
{
}

void SceneViewWidget::createControls(QWidget *widget)
{
    // label
    sceneViewLabelLeft = new QLabel();
    sceneViewLabelLeft->setMinimumWidth(120);
    sceneViewLabelCenter = new QLabel();
    sceneViewLabelCenter->setMinimumWidth(120);
    sceneViewLabelRight = new QLabel();
    sceneViewLabelRight->setMinimumWidth(150);

    QHBoxLayout *sceneViewLabelLayout = new QHBoxLayout();
    sceneViewLabelLayout->addWidget(sceneViewLabelLeft);
    sceneViewLabelLayout->addStretch(1);
    sceneViewLabelLayout->addWidget(sceneViewLabelCenter);
    sceneViewLabelLayout->addStretch(1);
    sceneViewLabelLayout->addWidget(sceneViewLabelRight);

    // view
    QVBoxLayout *sceneViewLayout = new QVBoxLayout();
    sceneViewLayout->addLayout(sceneViewLabelLayout);
    sceneViewLayout->addWidget(widget);
    sceneViewLayout->setStretch(1, 1);

    setLayout(sceneViewLayout);
}

void SceneViewWidget::labelLeft(const QString &left)
{
    sceneViewLabelLeft->setText(left);
}

void SceneViewWidget::labelCenter(const QString &center)
{
    sceneViewLabelCenter->setText(center);
}

void SceneViewWidget::labelRight(const QString &right)
{
    sceneViewLabelRight->setText(right);
}