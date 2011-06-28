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

#ifndef SURFACEINTEGRALVIEW_H
#define SURFACEINTEGRALVIEW_H

#include "util.h"

struct Element;

class SceneMaterial;

namespace Hermes
{
    namespace Module
    {
        struct Integral;
    }
}

class SurfaceIntegralValue
{
protected:
    Element *e;

    double *x;
    double *y;
    double *value1, *value2;
    double *dudx1, *dudy1, *dudx2, *dudy2;
    double3 *pt;
    double3 *tan;

    bool boundary;

    SceneMaterial *material;

    void calculate();
    virtual void initSolutions() = 0;
    virtual void prepareParser(SceneMaterial *material) = 0;

public:
    // variables
    std::map<Hermes::Module::Integral *, double> values;

    // parser
    Hermes::vector<mu::Parser *> parser;

    SurfaceIntegralValue();

    virtual QStringList variables() = 0;
};

class SurfaceIntegralValueView : public QDockWidget
{
    Q_OBJECT

public slots:
    void doShowSurfaceIntegral();

public:
    SurfaceIntegralValueView(QWidget *parent = 0);
    ~SurfaceIntegralValueView();

private slots:
    void doCopyValue();
    void doContextMenu(const QPoint &pos);

private:
    QTreeWidget *trvWidget;

    QAction *actCopy;
    QMenu *mnuInfo;

    void createActions();
    void createMenu();
};

#endif // SURFACEINTEGRALVIEW_H
