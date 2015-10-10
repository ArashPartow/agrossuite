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

#ifndef POSTPROCESSORVIEW_H
#define POSTPROCESSORVIEW_H

#include "util.h"

class PostDeal;
class SceneViewPreprocessor;
class SceneViewMesh;
class SceneViewPost2D;
class SceneViewPost3D;
class ChartView;
class LineEditDouble;
class CollapsableGroupBoxButton;
class FieldInfo;
class ValueLineEdit;
class PhysicalFieldWidget;

class PostprocessorSceneMeshWidget;

class PostprocessorWidget : public QWidget
{
    Q_OBJECT

public:
    PostprocessorWidget(PostDeal *postDeal,
                        SceneViewMesh *sceneMesh);

    inline PhysicalFieldWidget *fieldWidget() { return m_fieldWidget; }
    inline PostDeal *postDeal() { return m_postDeal; }

    QAction *actSceneModePost;

signals:
    void apply();

public slots:
    void doApply();

private:
    PostDeal *m_postDeal;

    // basic
    PhysicalFieldWidget *m_fieldWidget;

    // toolbar
    QToolBar *toolBar;
    QTabWidget *tabWidget;
    QPushButton *btnOK;

    // mesh and polynomial info
    QLabel *lblMeshInitial;
    QLabel *lblMeshSolution;
    QLabel *lblDOFs;

    //
    PostprocessorSceneMeshWidget *meshWidget;

    void createControls();

    // TMP
    SceneViewMesh *m_sceneMesh;

private slots:
    void reconnectActions();

    void doCalculationFinished();
};


#endif // POSTPROCESSORVIEW_H
