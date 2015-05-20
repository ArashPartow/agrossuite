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

#ifndef {{CLASS}}_WEAKFORM_H
#define {{CLASS}}_WEAKFORM_H

#include "util.h"
#include "solver/plugin_interface.h"
#include "solver/marker.h"
#include "{{ID}}_interface.h"

// typedef std::tuple<typename dealii::hp::DoFHandler<2>::active_cell_iterator, typename dealii::hp::DoFHandler<2>::active_cell_iterator> IteratorTuple;

class SolverDeal{{CLASS}} : public SolverDealTransient
{
public:
    SolverDeal{{CLASS}}(const FieldInfo *fieldInfo)
        : SolverDealTransient(fieldInfo) {}

    virtual void assembleSystem(const dealii::Vector<double> &solutionNonlinearPrevious,
                                bool assembleMatrix = true,
                                bool assembleRHS = true);
    virtual void assembleDirichlet(bool calculateDirichletLiftValue);

protected:
    // virtual void localAssembleSystem(const dealii::SynchronousIterators<IteratorTuple> &iter,
    virtual void localAssembleSystem(const DoubleCellIterator &iter,
                                AssemblyScratchData &scratch,
                                AssemblyCopyData &copy_data);
    virtual void copyLocalToGlobal(const AssemblyCopyData &copy_data);
};

#endif // {{CLASS}}_INTERFACE_H
