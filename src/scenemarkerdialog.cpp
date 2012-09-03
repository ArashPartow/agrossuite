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

#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/field.h"
#include "datatable.h"
#include "scene.h"
#include "scenebasic.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "gui.h"

SceneBoundary::SceneBoundary(FieldInfo *fieldInfo, QString name, QString type,
                             QMap<QString, Value> values)
    : Boundary(fieldInfo, name, type, values)
{

}

int SceneBoundary::showDialog(QWidget *parent)
{
    SceneBoundaryDialog *dialog = new SceneBoundaryDialog(this, parent);
    if (dialog)
        return dialog->exec();
    else
        QMessageBox::information(QApplication::activeWindow(), QObject::tr(""), QObject::tr("Boundary dialog doesn't exists."));
}

QVariant SceneBoundary::variant()
{
    QVariant v;
    v.setValue(this);
    return v;
}

SceneBoundaryNone::SceneBoundaryNone() : SceneBoundary(NULL, "none")
{

}

// *************************************************************************************************************************************

SceneMaterial::SceneMaterial(FieldInfo *fieldInfo, QString name,
                             QMap<QString, Value> values) : Material(fieldInfo, name, values)
{

}

int SceneMaterial::showDialog(QWidget *parent)
{
    SceneMaterialDialog *dialog = new SceneMaterialDialog(this, parent);
    if (dialog)
        return dialog->exec();
    else
        QMessageBox::information(QApplication::activeWindow(), QObject::tr(""), QObject::tr("Material dialog doesn't exists."));

}

QVariant SceneMaterial::variant()
{
    QVariant v;
    v.setValue(this);
    return v;
}

SceneMaterialNone::SceneMaterialNone() : SceneMaterial(NULL, "none")
{

}

// *************************************************************************************************************************************


SceneFieldWidget::SceneFieldWidget(Module::DialogUI *ui, QWidget *parent)
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
    equationImage = new QLabel();
    equationImage->setMinimumHeight(fontMetrics().height()*2.2);
    equationImage->setMinimumWidth(300);

    QHBoxLayout *layoutEquation = new QHBoxLayout();
    layoutEquation->addWidget(equationImage);
    layoutEquation->addStretch();

    QGroupBox *grpEquation = new QGroupBox(tr("Equation"));
    grpEquation->setLayout(layoutEquation);

    layout->addWidget(grpEquation);

    QMapIterator<QString, QList<Module::DialogRow> > i(ui->groups());
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
                labels.append(new QLabel(QString("%1 (%2):").
                                         arg(row.shortnameHtml()).
                                         arg(row.unitHtml())));
                labels.at(labels.count() - 1)->setToolTip(row.name());
                labels.at(labels.count() - 1)->setMinimumWidth(100);

                // text edit
                values.append(textEdit);
                values.at(values.count() - 1)->setValue(Value(QString::number(row.defaultValue())));

                conditions.append(row.condition());

                int index = layoutGroup->rowCount();
                layoutGroup->addWidget(labels.at(labels.count() - 1), index, 0);
                layoutGroup->addWidget(values.at(values.count() - 1), index, 1);
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

    refresh();
}

// *************************************************************************************************************************************

SceneFieldWidgetMaterial::SceneFieldWidgetMaterial(Module::DialogUI *ui, SceneMaterial *material, QWidget *parent)
    : SceneFieldWidget(ui, parent), material(material)
{
}

ValueLineEdit *SceneFieldWidgetMaterial::addValueEditWidget(const Module::DialogRow &row)
{
    foreach (Module::MaterialTypeVariable *variable, material->fieldInfo()->module()->materialTypeVariables())
    {
        if (variable->id() == row.id())
        {
            ValueLineEdit *edit = new ValueLineEdit(this,
                                                    (variable->isTimeDep() && material->fieldInfo()->analysisType() == AnalysisType_Transient),
                                                    (variable->isNonlinear() && material->fieldInfo()->linearityType() != LinearityType_Linear));
            if (variable->isNonlinear())
            {
                edit->setLabelX(row.shortnameDependenceHtml());
                edit->setLabelY(row.shortnameHtml());
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
        values.at(j)->setValue(material->value(ids.at(j)));

    // conditions
    for (int j = 0; j < conditions.count(); j++)
        values.at(j)->setCondition(conditions.at(j));
}

bool SceneFieldWidgetMaterial::save()
{
    for (int j = 0; j < ids.count(); j++)
        if (values.at(j)->evaluate())
            material->setValue(ids.at(j), values.at(j)->value());
        else
            return false;

    return true;
}

void SceneFieldWidgetMaterial::refresh()
{
    // read equation
    QString fileName = QString(":/equations/%1/%1_%2.png")
            .arg(material->fieldInfo()->fieldId())
            .arg(analysisTypeToStringKey(material->fieldInfo()->analysisType()));

    readPixmap(equationImage, fileName);
}

// *************************************************************************************************************************************

SceneFieldWidgetBoundary::SceneFieldWidgetBoundary(Module::DialogUI *ui, SceneBoundary *boundary, QWidget *parent)
    : SceneFieldWidget(ui, parent), boundary(boundary)
{
}

ValueLineEdit *SceneFieldWidgetBoundary::addValueEditWidget(const Module::DialogRow &row)
{
    foreach (Module::BoundaryType *boundaryType, boundary->fieldInfo()->module()->boundaryTypes())
        foreach (Module::BoundaryTypeVariable *variable, boundaryType->variables())
        {
            if (variable->id() == row.id())
            {
                ValueLineEdit *edit = new ValueLineEdit(this,
                                                        (variable->isTimeDep() && boundary->fieldInfo()->analysisType() == AnalysisType_Transient),
                                                        false);
                return edit;
            }
        }

    return NULL;
}

void SceneFieldWidgetBoundary::addCustomWidget(QVBoxLayout *layout)
{
    comboBox = new QComboBox(this);
    boundary->fieldInfo()->module()->fillComboBoxBoundaryCondition(comboBox);
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
    setMinimumSize(sizeHint());

    // disable variables
    for (int i = 0; i < ids.count(); i++)
        values.at(i)->setEnabled(false);

    Module::BoundaryType *boundary_type = boundary->fieldInfo()->module()->boundaryType(comboBox->itemData(index).toString());
    foreach (Module::BoundaryTypeVariable *variable, boundary_type->variables())
    {
        int i = ids.indexOf(variable->id());

        if (i >= 0)
            values.at(i)->setEnabled(true);
    }

    // read equation
    QString fileName = QString(":/equations/%1/%2.png")
            .arg(boundary->fieldInfo()->fieldId())
            .arg(comboBox->itemData(index).toString());

    readPixmap(equationImage, fileName);

    // sizehint
    // setMinimumSize(sizeHint());
}

void SceneFieldWidgetBoundary::load()
{
    // type
    comboBox->setCurrentIndex(comboBox->findData(boundary->type()));

    // variables
    for (int i = 0; i < ids.count(); i++)
        values.at(i)->setValue(boundary->value(ids.at(i)));

    // conditions
    for (int j = 0; j < conditions.count(); j++)
        values.at(j)->setCondition(conditions.at(j));
}

bool SceneFieldWidgetBoundary::save()
{
    // type
    boundary->setType(comboBox->itemData(comboBox->currentIndex()).toString());

    // variables
    for (int i = 0; i < ids.count(); i++)
        if (values.at(i)->evaluate())
            boundary->setValue(ids.at(i), values.at(i)->value());
        else
            return false;

    return true;
}

// *************************************************************************************************************************************

SceneBoundaryDialog::SceneBoundaryDialog(SceneBoundary *boundary, QWidget *parent)
    : QDialog(parent), boundary(boundary)
{
    setWindowIcon(icon("scene-edgemarker"));
    setWindowTitle(tr("Boundary condition - %1").arg(boundary->fieldInfo()->name()));

    layout = new QGridLayout();
    txtName = new QLineEdit(this);

    createDialog();

    load();
    setSize();
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
    fieldWidget = new SceneFieldWidgetBoundary(boundary->fieldInfo()->module()->boundaryUI(), boundary, this);
    fieldWidget->createContent();
    fieldWidget->setMinimumSize(sizeHint());

    layout->addWidget(fieldWidget, 10, 0, 1, 3);
}

void SceneBoundaryDialog::load()
{
    txtName->setText(boundary->name());

    // load variables
    fieldWidget->load();
}

bool SceneBoundaryDialog::save()
{
    // find name duplicities
    foreach (SceneBoundary *boundary, Util::scene()->boundaries->items())
    {
        if (boundary->name() == txtName->text())
        {
            if (boundary == boundary)
                continue;

            QMessageBox::warning(this, tr("Boundary marker"), tr("Boundary marker name already exists."));
            return false;
        }
    }
    boundary->setName(txtName->text());

    // save variables
    if (!fieldWidget->save())
        return false;

    return true;
}

void SceneBoundaryDialog::setSize()
{
    setMinimumSize(sizeHint());
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
    : QDialog(parent), material(material)
{
    setWindowIcon(icon("scene-labelmarker"));
    setWindowTitle(tr("Material - %1").arg(material->fieldInfo()->name()));

    layout = new QGridLayout();
    txtName = new QLineEdit(this);

    createDialog();

    load();
    setSize();
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
    fieldWidget = new SceneFieldWidgetMaterial(material->fieldInfo()->module()->materialUI(), material, this);
    fieldWidget->createContent();
    fieldWidget->setMinimumSize(sizeHint());

    layout->addWidget(fieldWidget, 10, 0, 1, 3);
}

void SceneMaterialDialog::load()
{
    txtName->setText(material->name());

    // load variables
    fieldWidget->load();
}

bool SceneMaterialDialog::save()
{
    // find name duplicities
    foreach (SceneMaterial *material, Util::scene()->materials->items())
    {
        if (material->name() == txtName->text())
        {
            if (material == material)
                continue;

            QMessageBox::warning(this, tr("Material marker"), tr("Material marker name already exists."));
            return false;
        }
    }
    material->setName(txtName->text());

    // save variables
    if (!fieldWidget->save())
        return false;

    return true;
}


void SceneMaterialDialog::setSize()
{
    setMinimumSize(sizeHint());
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
