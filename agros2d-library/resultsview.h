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

#ifndef LOCALVALUE_H
#define LOCALVALUE_H

#include "util.h"
#include "scenemarkerselectdialog.h"
#include "scenebasicselectdialog.h"

#include <QtWebKit>
#include <QtWebKitWidgets>

class ValueLineEdit;
class SceneMaterial;
class Solution;
class PostDeal;
class ProblemComputation;

class ResultsView : public QDockWidget
{
    Q_OBJECT

public slots:
    void doPostprocessorModeGroupChanged(SceneModePostprocessor sceneModePostprocessor);
    void doShowResults();

    void showEmpty();
    void showNotSolved();
    void showPoint(const Point &point);
    void showPoint();
    void showVolumeIntegral();
    void showSurfaceIntegral();

public:
    ResultsView(QWidget *parent = 0);

    QPushButton *btnSelectMarker;

private:
    Point m_point;

    SceneModePostprocessor m_sceneModePostprocessor;
    QString m_cascadeStyleSheet;

    QAction *actPoint;
    QWebView *webView;

    QSharedPointer<ProblemComputation> m_computation;

    void createActions();

private slots:
    void connectComputation(QSharedPointer<ProblemComputation> computation);
};

class LocalPointValueDialog : public QDialog
{
    Q_OBJECT
public:
    LocalPointValueDialog(Point point, ProblemComputation *computation, QWidget *parent = 0);

    Point point();

private:
    QDialogButtonBox *buttonBox;

    ValueLineEdit *txtPointX;
    ValueLineEdit *txtPointY;

private slots:
    void evaluated(bool isError);
};

#endif // LOCALVALUE_H
