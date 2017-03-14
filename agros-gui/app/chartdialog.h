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

#ifndef CHARTDIALOG_H
#define CHARTDIALOG_H

#include "util/util.h"
#include "solver/plugin_interface.h"
#include "gui/other.h"

#include <QSvgWidget>
#include <QSvgRenderer>

class LineEditDouble;
class LocalValue;
class FieldInfo;
class SceneViewPost2D;
class QCustomPlot;
class QCPBars;
class QCPItemTracer;
class QCPItemText;
class ChartLine;
class PostprocessorWidget;

class SceneViewChart : public QWidget
{
    Q_OBJECT

public:
    SceneViewChart(PostprocessorWidget *postprocessorWidget);

    inline QCustomPlot *chart() { return m_chart; }

signals:
    void labelRight(const QString &right);
    void labelCenter(const QString &center);

public slots:
    void refresh();
    void doSaveImage();
    void doExportData();

private slots:
    void chartMouseMoved(QMouseEvent *event);

private:
    PostprocessorWidget *m_postprocessorWidget;
    QCustomPlot *m_chart;

    QVector<double> horizontalAxisValues(ChartLine *chartLine);
    void plotGeometry();
    void plotTime();

    QMap<QString, double> getData(Point point, int timeStep, int adaptivityStep);
    QStringList headers();

    friend class PostprocessorSceneChartWidget;
};

#endif // CHARTDIALOG_H