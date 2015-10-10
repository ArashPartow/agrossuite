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

#include "common.h"

#include "util.h"
#include "util/global.h"

#include "solver/module.h"

#include "solver/solver.h"
#include "solver/field.h"
#include "solver/problem.h"
#include "solver/solutionstore.h"

#include <QNetworkAccessManager>

void readPixmap(QLabel *lblEquation, const QString &name)
{
    QPixmap pixmap;
    pixmap.load(name);
    lblEquation->setPixmap(pixmap);
    lblEquation->setMaximumSize(pixmap.size());
    lblEquation->setMinimumSize(pixmap.size());
}

QLabel *createLabel(const QString &label, const QString &toolTip)
{
    QLabel *lblEquation = new QLabel(label + ":");
    lblEquation->setToolTip(toolTip);
    lblEquation->setMinimumWidth(100);
    return lblEquation;
}

void addTreeWidgetItemValue(QTreeWidgetItem *parent, const QString &name, const QString &text, const QString &unit)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    item->setText(0, name);
    item->setText(1, text);
    item->setTextAlignment(1, Qt::AlignRight);
    item->setText(2, QString("%1 ").arg(unit));
    item->setTextAlignment(2, Qt::AlignLeft);
}

void fillComboBoxFieldInfo(QComboBox *cmbFieldInfo)
{
    // store variable
    QString fieldId = cmbFieldInfo->itemData(cmbFieldInfo->currentIndex()).toString();

    // clear combo
    cmbFieldInfo->blockSignals(true);
    cmbFieldInfo->clear();
    foreach (FieldInfo *fieldInfo, Agros2D::preprocessor()->fieldInfos())
        cmbFieldInfo->addItem(fieldInfo->name(), fieldInfo->fieldId());

    cmbFieldInfo->setCurrentIndex(cmbFieldInfo->findData(fieldId));
    if (cmbFieldInfo->currentIndex() == -1)
        cmbFieldInfo->setCurrentIndex(0);
    cmbFieldInfo->blockSignals(false);
}

void fillComboBoxScalarVariable(FieldInfo *fieldInfo, QComboBox *cmbFieldVariable)
{
    // if (!Agros2D::problem()->isSolved())
    //     return;

    // store variable
    QString physicFieldVariable = cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toString();

    // clear combo
    cmbFieldVariable->blockSignals(true);
    cmbFieldVariable->clear();
    foreach (Module::LocalVariable variable, fieldInfo->viewScalarVariables())
        cmbFieldVariable->addItem(variable.name(),
                                  variable.id());

    cmbFieldVariable->setCurrentIndex(cmbFieldVariable->findData(physicFieldVariable));
    if (cmbFieldVariable->currentIndex() == -1)
        cmbFieldVariable->setCurrentIndex(0);
    cmbFieldVariable->blockSignals(false);
}

void fillComboBoxContourVariable(FieldInfo *fieldInfo, QComboBox *cmbFieldVariable)
{
    // if (!Agros2D::problem()->isSolved())
    //     return;

    // store variable
    QString physicFieldVariable = cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toString();

    // clear combo
    cmbFieldVariable->blockSignals(true);
    cmbFieldVariable->clear();
    foreach (Module::LocalVariable variable, fieldInfo->viewScalarVariables())
        cmbFieldVariable->addItem(variable.name(),
                                  variable.id());


    cmbFieldVariable->setCurrentIndex(cmbFieldVariable->findData(physicFieldVariable));
    if (cmbFieldVariable->currentIndex() == -1)
        cmbFieldVariable->setCurrentIndex(0);
    cmbFieldVariable->blockSignals(false);
}

void fillComboBoxVectorVariable(FieldInfo *fieldInfo, QComboBox *cmbFieldVariable)
{
    // if (!Agros2D::problem()->isSolved())
    //     return;

    // store variable
    QString physicFieldVariable = cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toString();

    // clear combo
    cmbFieldVariable->blockSignals(true);
    cmbFieldVariable->clear();
    foreach (Module::LocalVariable variable, fieldInfo->viewVectorVariables())
        cmbFieldVariable->addItem(variable.name(),
                                  variable.id());

    cmbFieldVariable->setCurrentIndex(cmbFieldVariable->findData(physicFieldVariable));
    if (cmbFieldVariable->currentIndex() == -1)
        cmbFieldVariable->setCurrentIndex(0);
    cmbFieldVariable->blockSignals(false);
}

void fillComboBoxTimeStep(const FieldInfo* fieldInfo, QComboBox *cmbTimeStep)
{
    if (!Agros2D::computation()->isSolved())
        return;

    QList<double> times = Agros2D::computation()->timeStepTimes();

    cmbTimeStep->blockSignals(true);

    // store variable
    int timeStep = cmbTimeStep->currentIndex();
    if (timeStep == -1)
        timeStep = times.count() - 1;

    // clear combo
    cmbTimeStep->clear();

    // qDebug() << fieldInfo->name();
    int selectedIndex = -1;
    for (int step = 0; step < times.length(); step++)
    {        
        bool stepIsAvailable = Agros2D::solutionStore()->contains(FieldSolutionID(fieldInfo, step, Agros2D::solutionStore()->lastAdaptiveStep(fieldInfo, step)));
        if (!stepIsAvailable)
            continue;

        if (step == 0)
            cmbTimeStep->addItem(QObject::tr("Initial step"), 0.0);
        else
            cmbTimeStep->addItem(QObject::tr("%1 s (step: %2)").arg(QString::number(times[step], 'e', 2)).arg(step), step);

        if (step == timeStep)
            selectedIndex = cmbTimeStep->count() - 1;
    }

    if (selectedIndex != -1)
        cmbTimeStep->setCurrentIndex(selectedIndex);
    if (cmbTimeStep->count() > 0 && cmbTimeStep->currentIndex() == -1)
        cmbTimeStep->setCurrentIndex(0);

    cmbTimeStep->blockSignals(false);
}

void fillComboBoxAdaptivityStep(FieldInfo* fieldInfo, int timeStep, QComboBox *cmbAdaptivityStep)
{
    if (!Agros2D::computation()->isSolved())
        return;

    cmbAdaptivityStep->blockSignals(true);

    int lastAdaptiveStep = Agros2D::solutionStore()->lastAdaptiveStep(fieldInfo, timeStep);

    // store variable
    int adaptivityStep = cmbAdaptivityStep->currentIndex();
    if (adaptivityStep == -1)
        adaptivityStep = lastAdaptiveStep;

    // clear combo
    cmbAdaptivityStep->clear();

    for (int step = 0; step <= lastAdaptiveStep; step++)
    {
        cmbAdaptivityStep->addItem(QString::number(step + 1), step);
    }

    cmbAdaptivityStep->setCurrentIndex(adaptivityStep);
    cmbAdaptivityStep->blockSignals(false);
}

void fillComboBoxFonts(QComboBox *cmbFonts)
{
    // read fonts
    cmbFonts->blockSignals(true);

    // store variable
    QString fnt = cmbFonts->itemData(cmbFonts->currentIndex(), Qt::UserRole).toString();

    QDir dir(datadir() + "/resources/fonts");

    QStringList filter;
    filter << "*.ttf";
    QStringList list = dir.entryList(filter);

    cmbFonts->clear();
    foreach (QString filename, list)
    {
        cmbFonts->addItem(QFileInfo(filename).baseName(), QFileInfo(filename).baseName());
    }

    cmbFonts->setCurrentIndex(cmbFonts->findData(fnt, Qt::UserRole));
    if (cmbFonts->currentIndex() == -1 && cmbFonts->children().size() > 0)
        cmbFonts->setCurrentIndex(0);
    cmbFonts->blockSignals(false);
}

// column minimum width
int columnMinimumWidth()
{
    if (QApplication::desktop()->width() == 1024)
        return 70;
    else
        return 110;
}

