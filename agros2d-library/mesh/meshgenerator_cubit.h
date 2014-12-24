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

#ifndef MESHGENERATOR_CUBIT_H
#define MESHGENERATOR_CUBIT_H

#include "util.h"
#include "meshgenerator.h"

class MeshGeneratorCubitExternal : public MeshGenerator
{
    Q_OBJECT

private slots:
    void meshCubitError(QProcess::ProcessError error);
    void meshCubitCreated(int exitCode);
    bool writeToCubit();
    bool readLSDynaMeshFormat();

public:
    MeshGeneratorCubitExternal();

    virtual bool mesh();
};

#endif //MESHGENERATOR_CUBIT_H