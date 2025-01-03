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

#ifndef VALUETIMEDIALOG_H
#define VALUETIMEDIALOG_H

#include "util/util.h"
#include "util/point.h"
#include "gui/other.h"
#include "gui/chart.h"
#include "value.h"

#include "datatable.h"

class FieldInfo;
class ValueLineEdit;

class ValueTimeDialog: public QDialog
{
    Q_OBJECT

public:
    ValueTimeDialog(QWidget *parent = 0);
    ~ValueTimeDialog();

    Value value() const { return Value(m_problem, txtLineEdit->text()); }
    void setValue(Value value);

private:
    ChartView *chartView;
    QValueAxis *axisX;
    QValueAxis *axisFunction;
    QLineSeries *valueSeries;

    QPushButton *btnOk;
    QPushButton *btnClose;
    QPushButton *btnPlot;

    QLabel *lblInfoError;
    QLineEdit *txtLineEdit;
    ValueLineEdit *txtTimeTotal;
    QComboBox *cmbPresets;

    ProblemBase *m_problem;

    void createControls();

private slots:
    void doAccept();
    void doReject();

    void checkExpression();
    void plotFunction();
    void presetsChanged(int index);
};

#endif // VALUETIMEDIALOG_H
