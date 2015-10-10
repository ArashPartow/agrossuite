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

#include "physicalfield.h"

#include "util/global.h"
#include "gui/common.h"

#include "solver/problem.h"
#include "solver/solutionstore.h"
#include "solver/field.h"

PhysicalFieldWidget::PhysicalFieldWidget(QWidget *parent) : QWidget(parent)
{
    cmbFieldInfo = new QComboBox();
    connect(cmbFieldInfo, SIGNAL(currentIndexChanged(int)), this, SLOT(doFieldInfo(int)));

    QGridLayout *layoutField = new QGridLayout();
    layoutField->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutField->setColumnStretch(1, 1);
    layoutField->addWidget(new QLabel(tr("Physical field:")), 0, 0);
    layoutField->addWidget(cmbFieldInfo, 0, 1);

    QGroupBox *grpField = new QGroupBox(tr("Postprocessor settings"), this);
    grpField->setLayout(layoutField);

    // transient
    lblTimeStep = new QLabel(tr("Time step:"));
    cmbTimeStep = new QComboBox(this);
    connect(cmbTimeStep, SIGNAL(currentIndexChanged(int)), this, SLOT(doTimeStep(int)));

    QGridLayout *layoutTransient = new QGridLayout();
    layoutTransient->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutTransient->setColumnStretch(1, 1);
    layoutTransient->addWidget(lblTimeStep, 0, 0);
    layoutTransient->addWidget(cmbTimeStep, 0, 1);

    grpTime = new QGroupBox(tr("Transient analysis"), this);
    grpTime->setVisible(false);
    grpTime->setLayout(layoutTransient);

    // adaptivity
    lblAdaptivityStep = new QLabel(tr("Adaptivity step:"));
    cmbAdaptivityStep = new QComboBox(this);

    QGridLayout *layoutAdaptivity = new QGridLayout();
    layoutAdaptivity->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutAdaptivity->setColumnStretch(1, 1);
    layoutAdaptivity->addWidget(lblAdaptivityStep, 0, 0);
    layoutAdaptivity->addWidget(cmbAdaptivityStep, 0, 1);

    grpAdaptivity = new QGroupBox(tr("Space adaptivity"), this);
    grpAdaptivity->setVisible(false);
    grpAdaptivity->setLayout(layoutAdaptivity);

    QVBoxLayout *layoutMain = new QVBoxLayout();
    layoutMain->setContentsMargins(0, 0, 0, 0);
    layoutMain->addWidget(grpField);
    layoutMain->addWidget(grpTime);
    layoutMain->addWidget(grpAdaptivity);

    setLayout(layoutMain);

    // reconnect computation slots
    connect(Agros2D::singleton(), SIGNAL(reconnectSlots()), this, SLOT(reconnectActions()));
}

PhysicalFieldWidget::~PhysicalFieldWidget()
{
}

void PhysicalFieldWidget::reconnectActions()
{
    connect(Agros2D::computation(), SIGNAL(meshed()), this, SLOT(updateControls()));
    connect(Agros2D::computation(), SIGNAL(solved()), this, SLOT(updateControls()));
}

FieldInfo* PhysicalFieldWidget::selectedField()
{
    if (Agros2D::computation()->hasField(cmbFieldInfo->itemData(cmbFieldInfo->currentIndex()).toString()))
        return Agros2D::computation()->fieldInfo(cmbFieldInfo->itemData(cmbFieldInfo->currentIndex()).toString());
    else
        return NULL;
}

void PhysicalFieldWidget::selectField(const FieldInfo* fieldInfo)
{
    if (cmbFieldInfo->findData(fieldInfo->fieldId()) != -1)
    {
        cmbFieldInfo->setCurrentIndex(cmbFieldInfo->findData(fieldInfo->fieldId()));

        if (Agros2D::computation()->isSolved())
        {
            fillComboBoxTimeStep(fieldInfo, cmbTimeStep);
            doTimeStep(-1);
        }
    }
}

int PhysicalFieldWidget::selectedTimeStep()
{
    if (cmbTimeStep->currentIndex() == -1)
        return 0;
    else
        return cmbTimeStep->itemData(cmbTimeStep->currentIndex()).toInt();
}

void PhysicalFieldWidget::selectTimeStep(int timeStep)
{
    if (cmbTimeStep->findData(timeStep) != -1)
        cmbTimeStep->setCurrentIndex(cmbTimeStep->findData(timeStep));
}

int PhysicalFieldWidget::selectedAdaptivityStep()
{
    if (cmbAdaptivityStep->currentIndex() == -1)
        return 0;
    else
        return cmbAdaptivityStep->itemData(cmbAdaptivityStep->currentIndex()).toInt();
}

void PhysicalFieldWidget::selectAdaptivityStep(int adaptivityStep)
{
    if (cmbAdaptivityStep->findData(adaptivityStep) != -1)
        cmbAdaptivityStep->setCurrentIndex(cmbAdaptivityStep->findData(adaptivityStep));
}

void PhysicalFieldWidget::updateControls()
{
    if (Agros2D::computation() && Agros2D::computation()->isMeshed())
    {
        fillComboBoxFieldInfo(cmbFieldInfo);
        doFieldInfo(cmbFieldInfo->currentIndex());
    }
    else
    {
        cmbFieldInfo->clear();
        doFieldInfo(cmbFieldInfo->currentIndex());
    }
}

void PhysicalFieldWidget::doFieldInfo(int index)
{
    QString fieldName = cmbFieldInfo->itemData(cmbFieldInfo->currentIndex()).toString();
    if (Agros2D::computation() && Agros2D::computation()->hasField(fieldName))
    {
        FieldInfo *fieldInfo = Agros2D::computation()->fieldInfo(fieldName);
        if (Agros2D::computation()->isSolved())
        {
            fillComboBoxTimeStep(fieldInfo, cmbTimeStep);
        }
        else
        {
            cmbTimeStep->clear();
        }
        doTimeStep(-1);

        if ((m_currentFieldName != fieldName) || (m_currentAnalysisType != fieldInfo->analysisType()))
            emit fieldChanged();

        // set current field name
        m_currentFieldName = fieldName;
        m_currentAnalysisType = fieldInfo->analysisType();
    }
    else
    {
        cmbTimeStep->clear();
        doTimeStep(-1);
    }
}

void PhysicalFieldWidget::doTimeStep(int index)
{
    if (Agros2D::computation() && Agros2D::computation()->isSolved())
    {
        fillComboBoxAdaptivityStep(selectedField(), selectedTimeStep(), cmbAdaptivityStep);
        if ((cmbAdaptivityStep->currentIndex() >= cmbAdaptivityStep->count()) || (cmbAdaptivityStep->currentIndex() < 0))
        {
            cmbAdaptivityStep->setCurrentIndex(cmbAdaptivityStep->count() - 1);
        }
    }
    else
    {
        cmbAdaptivityStep->clear();
    }

    grpTime->setVisible(cmbTimeStep->count() >= 1);
    grpAdaptivity->setVisible(cmbAdaptivityStep->count() > 1);
}

