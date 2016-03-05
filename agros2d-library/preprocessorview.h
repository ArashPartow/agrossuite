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

#ifndef PREPROCESSORVIEW_H
#define PREPROCESSORVIEW_H

#include "util.h"
#include "util/enums.h"

#include <QWebView>

class SceneViewPreprocessor;
class FieldInfo;

class PreprocessorWidget : public QWidget
{
    Q_OBJECT

public slots:    
    void refresh();
    void loadTooltip(SceneGeometryMode sceneMode);

    void doProperties();
    void doDelete();
    void doNewParameter();
    void doNewField();
    void doNewStudy();
    void doNewRecipeLocalValue();
    void doNewRecipeSurfaceIntegral();
    void doNewRecipeVolumeIntegral();
    void doNewRecipe(ResultRecipeType type);

public:
    PreprocessorWidget(SceneViewPreprocessor *sceneView, QWidget *parent = 0);
    ~PreprocessorWidget();

    QAction *actUndo;
    QAction *actRedo;

    QToolBar *toolBar;

protected:
    void keyPressEvent(QKeyEvent *event);

private:    
    enum Type
    {
        Undefined = 0,
        GeometryParameter,
        GeometryNode,
        GeometryEdge,
        GeometryLabel,
        Material,
        Boundary,
        OptilabStudy,
        OptilabParameter,
        OptilabFunctional,
        OptilabRecipe,
        ProblemProperties,
        FieldProperties
    };

    SceneViewPreprocessor *m_sceneViewPreprocessor;

    QTextEdit *txtViewNodes;
    QTextEdit *txtViewEdges;
    QTextEdit *txtViewLabels;
    QSplitter *splitter;

    QTreeWidget *trvWidget;

    QAction *actProperties;
    QAction *actDelete;
    QAction *actNewParameter;
    QAction *actNewField;
    QAction *actNewStudy;
    QAction *actNewRecipeLocalValue;
    QAction *actNewRecipeSurfaceIntegral;
    QAction *actNewRecipeVolumeIntegral;
    
    QMenu *mnuPreprocessor;

    QLineEdit *txtGridStep;
    QCheckBox *chkSnapToGrid;

    void createActions();
    void createControls();
    void createMenu();

    QString problemPropertiesToString();
    QString fieldPropertiesToString(FieldInfo *fieldInfo);
    void problemProperties(QTreeWidgetItem *item);
    void fieldProperties(FieldInfo *fieldInfo, QTreeWidgetItem *item);
    QTreeWidgetItem *propertiesItem(QTreeWidgetItem *item, const QString &key, const QString &value,
                                    PreprocessorWidget::Type type = PreprocessorWidget::Undefined,
                                    const QString &data = "");

private slots:
    void doContextMenu(const QPoint &pos);
    void doItemDoubleClicked(QTreeWidgetItem *item, int role);
    void doItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void doApply();
};

#endif // PREPROCESSORVIEW_H
