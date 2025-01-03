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

#include "scenemarkerdialog.h"

#include "util/global.h"

#include "gui/common.h"
#include "gui/valuelineedit.h"

#include "scenemarker.h"
#include "solver/module.h"

#include "solver/field.h"
#include "datatable.h"
#include "scene.h"
#include "solver/problem.h"
#include "scenebasic.h"
#include "sceneedge.h"
#include "scenelabel.h"

SceneFieldWidget::SceneFieldWidget(Module::DialogUI ui, QWidget *parent)
    : QWidget(parent), ui(ui)
{
}

void SceneFieldWidget::createContent()
{
    // widget layout
    layout = new QVBoxLayout();
    setLayout(layout);

    // add custom widget
    addCustomWidget(layout);

    // equation
    // equationLaTeX = new LaTeXViewer(this);
    equationImage = new QLabel();

    QVBoxLayout *layoutEquation = new QVBoxLayout();
    // layoutEquation->addWidget(equationLaTeX);
    layoutEquation->addWidget(equationImage);
    layoutEquation->addStretch();

    QGroupBox *grpEquation = new QGroupBox(tr("Equation"));
    grpEquation->setLayout(layoutEquation);

    layout->addWidget(grpEquation);

    QMapIterator<QString, QList<Module::DialogRow> > i(ui.groups());
    while (i.hasNext())
    {
        i.next();

        // group layout
        QGridLayout *layoutGroup = new QGridLayout();

        // variables
        QList<Module::DialogRow> variables = i.value();

        foreach (Module::DialogRow row, variables)
        {
            ValueLineEdit *textEdit = addValueEditWidget(row);

            if (textEdit)
            {
                // id
                ids.append(row.id());

                // label
                if (textEdit->isBool())
                {
                    labels.append(new QLabel(row.name()));
                }
                else{
                    labels.append(new QLabel(QString("%1 (%2):").
                                             arg(row.shortnameHtml()).
                                             arg(row.unitHtml())));
                    labels.at(labels.count() - 1)->setToolTip(row.name());
                    labels.at(labels.count() - 1)->setMinimumWidth(100);
                }

                // text edit
                values.append(textEdit);
                textEdit->setValue(Value(nullptr, QString::number(row.defaultValue())));

                conditions.append(row.condition());

                int index = layoutGroup->rowCount();
                if (textEdit->isBool())
                {
                    textEdit->setTitle(row.name());
                    layoutGroup->addWidget(textEdit, index, 0, 1, 2);
                    connect(textEdit, SIGNAL(enableFields(QString, bool)), this, SIGNAL(enableFields(QString, bool)));
                }
                else
                {
                    layoutGroup->addWidget(labels.at(labels.count() - 1), index, 0);
                    layoutGroup->addWidget(textEdit, index, 1);
                    if (dynamic_cast<SceneFieldWidgetMaterial *>(this))
                        connect(this, SIGNAL(enableFields(QString, bool)), textEdit, SLOT(doEnableFields(QString, bool)));
                }
            }
        }

        // widget layout
        if (i.key() == "")
        {
            QGroupBox *grpGroup = new QGroupBox(tr("Parameters"));
            grpGroup->setLayout(layoutGroup);
            layout->addWidget(grpGroup);
        }
        else
        {
            QGroupBox *grpGroup = new QGroupBox(i.key());
            grpGroup->setLayout(layoutGroup);
            layout->addWidget(grpGroup);
        }
    }

    foreach(ValueLineEdit* valueLineEdit, values)
    {
        if(valueLineEdit->isBool())
            valueLineEdit->doCheckBoxStateChanged();
    }

    refresh();
}

// *************************************************************************************************************************************

SceneFieldWidgetMaterial::SceneFieldWidgetMaterial(Module::DialogUI ui, SceneMaterial *material, QWidget *parent)
    : SceneFieldWidget(ui, parent), m_material(material)
{
}

ValueLineEdit *SceneFieldWidgetMaterial::addValueEditWidget(const Module::DialogRow &row)
{
    foreach (Module::MaterialTypeVariable variable, m_material->fieldInfo()->materialTypeVariables())
    {
        if (variable.id() == row.id())
        {
            ValueLineEdit *edit = new ValueLineEdit(this,
                                                    (variable.isTimeDep() && m_material->fieldInfo()->analysisType() == AnalysisType_Transient),
                                                    (variable.isNonlinear() && m_material->fieldInfo()->linearityType() != LinearityType_Linear),
                                                    variable.isBool(), variable.id(), variable.onlyIf(), variable.onlyIfNot(), variable.isSource());
            if (variable.isNonlinear())
            {
                edit->setTitle(row.name());
                edit->setLabelX(row.shortnameDependence());
                edit->setLabelY(row.shortname());
            }
            return edit;
        }
    }

    return NULL;
}

void SceneFieldWidgetMaterial::load()
{
    // variables
    for (int j = 0; j < ids.count(); j++)
        values.at(j)->setValue(*m_material->valueNakedPtr(ids.at(j)));

    // conditions
    for (int i = 0; i < conditions.count(); i++)
        values.at(i)->setCondition(conditions.at(i));
}

bool SceneFieldWidgetMaterial::save()
{
    for (int i = 0; i < ids.count(); i++)
    {
        if (!values.at(i)->isEnabled())
            continue;

        if (values.at(i)->evaluate())
            m_material->setValue(ids.at(i), values.at(i)->value());
        else
            return false;
    }

    return true;
}

void SceneFieldWidgetMaterial::refresh()
{
    // equationLaTeX->setLatex(material->fieldInfo()->equation());
    readEquation();
}

void SceneFieldWidgetMaterial::readEquation()
{
    QPixmap pixmap(QString("%1/resources/images/equations/%2_equation_%3.png").
                   arg(Agros::dataDir()).
                   arg(m_material->fieldInfo()->fieldId()).
                   arg(analysisTypeToStringKey(m_material->fieldInfo()->analysisType())));

    equationImage->setPixmap(pixmap);
    equationImage->setMask(pixmap.mask());
}

// *************************************************************************************************************************************

SceneFieldWidgetBoundary::SceneFieldWidgetBoundary(Module::DialogUI ui, SceneBoundary *boundary, QWidget *parent)
    : SceneFieldWidget(ui, parent), m_boundary(boundary)
{
}

ValueLineEdit *SceneFieldWidgetBoundary::addValueEditWidget(const Module::DialogRow &row)
{
    foreach (Module::BoundaryType boundaryType, m_boundary->fieldInfo()->boundaryTypes())
        foreach (Module::BoundaryTypeVariable variable, boundaryType.variables())
        {
            if (variable.id() == row.id())
            {
                ValueLineEdit *edit = new ValueLineEdit(this,
                                                        (variable.isTimeDep() && m_boundary->fieldInfo()->analysisType() == AnalysisType_Transient),
                                                        false);
                return edit;
            }
        }

    return NULL;
}

void SceneFieldWidgetBoundary::addCustomWidget(QVBoxLayout *layout)
{
    comboBox = new QComboBox(this);
    foreach (Module::BoundaryType marker, m_boundary->fieldInfo()->boundaryTypes())
        comboBox->addItem(marker.name(),
                          marker.id());

    connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(doTypeChanged(int)));

    QFormLayout *layoutForm = new QFormLayout();
    layoutForm->addRow(tr("Type:"), comboBox);

    layout->addLayout(layoutForm);
}

void SceneFieldWidgetBoundary::refresh()
{
    // set active marker
    doTypeChanged(comboBox->currentIndex());
}

void SceneFieldWidgetBoundary::doTypeChanged(int index)
{
    // disable variables
    for (int i = 0; i < ids.count(); i++)
        values.at(i)->setEnabled(false);

    Module::BoundaryType boundaryType = m_boundary->fieldInfo()->boundaryType(comboBox->itemData(index).toString());
    foreach (Module::BoundaryTypeVariable variable, boundaryType.variables())
    {
        int i = ids.indexOf(variable.id());

        if (i >= 0)
            values.at(i)->setEnabled(true);
    }

    // read equation
    // equationLaTeX->setLatex(boundaryType.equation());
    readEquation();
}

void SceneFieldWidgetBoundary::load()
{
    // type
    comboBox->setCurrentIndex(comboBox->findData(m_boundary->type()));

    // variables
    for (int i = 0; i < ids.count(); i++)
        values.at(i)->setValue(*m_boundary->valueNakedPtr(ids.at(i)));

    // conditions
    for (int i = 0; i < conditions.count(); i++)
        values.at(i)->setCondition(conditions.at(i));
}

bool SceneFieldWidgetBoundary::save()
{
    // type
    m_boundary->setType(comboBox->itemData(comboBox->currentIndex()).toString());

    // variables
    for (int i = 0; i < ids.count(); i++)
    {
        if (!values.at(i)->isEnabled())
            continue;

        if (values.at(i)->evaluate())
            m_boundary->setValue(ids.at(i), values.at(i)->value());
        else
            return false;
    }

    return true;
}

void SceneFieldWidgetBoundary::readEquation()
{
    QPixmap pixmap(QString("%1/resources/images/equations/%2_equation_%3_%4.png").
                   arg(Agros::dataDir()).
                   arg(m_boundary->fieldInfo()->fieldId()).
                   arg(analysisTypeToStringKey(m_boundary->fieldInfo()->analysisType())).
                   arg(comboBox->itemData(comboBox->currentIndex()).toString()));

    equationImage->setPixmap(pixmap);
    equationImage->setMask(pixmap.mask());
}

// *************************************************************************************************************************************

SceneBoundaryDialog::SceneBoundaryDialog(SceneBoundary *boundary, QWidget *parent)
    : QDialog(parent), m_boundary(boundary)
{
    setWindowTitle(tr("Boundary condition - %1").arg(boundary->fieldInfo()->name()));
    setWindowIcon(icon("menu_boundary"));

    layout = new QGridLayout();
    txtName = new QLineEdit(this);

    createDialog();

    load();
}

void SceneBoundaryDialog::createDialog()
{
    // dialog buttons
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    layout->addWidget(new QLabel(tr("Name:")), 0, 0);
    layout->addWidget(txtName, 0, 2);

    // content
    createContent();

    layout->addWidget(buttonBox, 100, 0, 1, 3);
    layout->setRowStretch(99, 1);

    txtName->setFocus();

    setLayout(layout);
}

void SceneBoundaryDialog::createContent()
{
    fieldWidget = new SceneFieldWidgetBoundary(m_boundary->fieldInfo()->boundaryUI(), m_boundary, this);
    fieldWidget->createContent();

    layout->addWidget(fieldWidget, 10, 0, 1, 3);
}

void SceneBoundaryDialog::load()
{
    txtName->setText(m_boundary->name());

    // load variables
    fieldWidget->load();
}

bool SceneBoundaryDialog::save()
{
    // find name duplicities
    foreach (SceneBoundary *boundary, m_boundary->scene()->boundaries->filter(m_boundary->fieldInfo()).items())
    {
        if (boundary->name() == txtName->text())
        {
            if (m_boundary == boundary)
                continue;

            QMessageBox::warning(this, tr("Boundary marker"), tr("Boundary condition with the same name already exists."));
            return false;
        }
    }
    m_boundary->setName(txtName->text());

    // save variables
    if (!fieldWidget->save())
        return false;

    return true;
}

void SceneBoundaryDialog::doAccept()
{
    if (save())
        accept();
}

void SceneBoundaryDialog::doReject()
{
    reject();
}

void SceneBoundaryDialog::evaluated(bool isError)
{
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!isError);
}

// *************************************************************************************************************************************

SceneMaterialDialog::SceneMaterialDialog(SceneMaterial *material, QWidget *parent)
    : QDialog(parent), m_material(material)
{
    setWindowTitle(tr("Material - %1").arg(material->fieldInfo()->name()));
    setWindowIcon(icon("menu_material"));

    layout = new QGridLayout();
    txtName = new QLineEdit(this);

    createDialog();

    load();
}

void SceneMaterialDialog::createDialog()
{
    // dialog buttons
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    // name
    layout->addWidget(new QLabel(tr("Name:")), 0, 0, 1, 2);
    layout->addWidget(txtName, 0, 2);

    // content
    createContent();

    layout->addWidget(buttonBox, 100, 0, 1, 3);
    layout->setRowStretch(99, 1);

    txtName->setFocus();

    setLayout(layout);
}

void SceneMaterialDialog::createContent()
{
    fieldWidget = new SceneFieldWidgetMaterial(m_material->fieldInfo()->materialUI(), m_material, this);
    fieldWidget->createContent();

    layout->addWidget(fieldWidget, 10, 0, 1, 3);
}

void SceneMaterialDialog::load()
{
    txtName->setText(m_material->name());

    // load variables
    fieldWidget->load();
}

bool SceneMaterialDialog::save()
{
    // find name duplicities
    foreach (SceneMaterial *material, m_material->scene()->materials->filter(m_material->fieldInfo()).items())
    {
        if (material->name() == txtName->text())
        {
            if (m_material == material)
                continue;

            QMessageBox::warning(this, tr("Material marker"), tr("Material marker with the same name already exists."));
            return false;
        }
    }
    m_material->setName(txtName->text());

    // save variables
    if (!fieldWidget->save())
        return false;

    return true;
}

void SceneMaterialDialog::doAccept()
{
    if (save())
        accept();
}

void SceneMaterialDialog::doReject()
{
    reject();
}

void SceneMaterialDialog::evaluated(bool isError)
{
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!isError);
}
