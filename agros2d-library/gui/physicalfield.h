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

#ifndef GUI_PHYSICALFIELD_H
#define GUI_PHYSICALFIELD_H

#include "util.h"
#include "solver/plugin_interface.h"

class PhysicalFieldWidget : public QWidget
{
    Q_OBJECT

signals:
    void fieldChanged();

public slots:
    void updateControls();
    void connectComputation(QSharedPointer<Computation> computation);

public:
    PhysicalFieldWidget(QWidget *parent = 0);
    ~PhysicalFieldWidget();

    FieldInfo *selectedField();
    void selectField(const FieldInfo *fieldInfo);
    int selectedTimeStep();
    void selectTimeStep(int timeStep);
    int selectedAdaptivityStep();
    void selectAdaptivityStep(int adaptivityStep);

private:
    // computations
    QGroupBox *grpComputation;
    QComboBox *cmbComputation;

    // field
    QComboBox *cmbFieldInfo;

    // transient
    QGroupBox *grpTime;
    QLabel *lblTimeStep;
    QComboBox *cmbTimeStep;

    // adaptivity
    QGroupBox *grpAdaptivity;
    QLabel *lblAdaptivityStep;
    QComboBox *cmbAdaptivityStep;

    QString m_currentFieldName;
    AnalysisType m_currentAnalysisType;

    QSharedPointer<Computation> m_computation;

    // stored values
    QString m_lastFieldId;
    int m_lastTimeStep;
    int m_lastAdaptiveStep;

public slots:
    void doComputation(int index = -1);
    void doFieldInfo(int index = -1);
    void doTimeStep(int index = -1);
};

#endif // GUI_PHYSICALFIELD_H
