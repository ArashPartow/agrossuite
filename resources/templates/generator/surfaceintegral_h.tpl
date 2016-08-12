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

#ifndef {{ID}}_SURFACEINTEGRAL_H
#define {{ID}}_SURFACEINTEGRAL_H

#include <QObject>

#include "util/util.h"

#include "solver/plugin_interface.h"

class FieldInfo;

class {{CLASS}}SurfaceIntegral : public IntegralValue
{
public:
    {{CLASS}}SurfaceIntegral(Computation *computation,
                             const FieldInfo *fieldInfo,
                             int timeStep,
                             int adaptivityStep);

    void calculate();

    virtual void localAssembleSystem(const typename dealii::hp::DoFHandler<2>::active_cell_iterator &cell_int,
                                     IntegralScratchData &scratch_data,
                                     IntegralCopyData &copy_data);
    virtual void copyLocalToGlobal(const IntegralCopyData &copy_data);
};

#endif // {{ID}}_SURFACEINTEGRAL_H
