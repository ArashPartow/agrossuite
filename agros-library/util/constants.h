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

#ifndef UTIL_CONSTANTS_H
#define UTIL_CONSTANTS_H

#include <QtCore>
#include <QCoreApplication>

const double SPEEDOFLIGHT = 299792458;

const int DEALII_MAX_ORDER = 10;

// constants
const QString IMAGEROOT = QString("..%1resources_source%1images").arg(QDir::separator());
const QString MODULEROOT = QString("%1resources_source%1modules").arg(QDir::separator());
const QString COUPLINGROOT = QString("%1resources_source%1couplings").arg(QDir::separator());
const QString TEMPLATEROOT = QString("%1resources%1templates").arg(QDir::separator());

// discrete saving
const bool SAVEMATRIXANDRHS = false;
const QString DISCRETEDIRECTORY = "discrete";

const bool EXPERIMENTALFEATURES = false;

const int PALETTEENTRIES = 256;
const double VECTORSSCALEMIN = 0.1;
const double VECTORSSCALEMAX = 5.0;
const int SCALARDECIMALPLACE = 4;
const double CONTOURSWIDTH = 1.0;
const int VECTORSCOUNTMIN = 1;
const int VECTORSCOUNTMAX = 500;
const int CONTOURSCOUNTMIN = 1;
const int CONTOURSCOUNTMAX = 100;
const int PALETTESTEPSMIN = 3;
const int PALETTESTEPSMAX = 50;

// cache size
const int CACHE_SIZE = 10;

// solver cache
const bool USER_SOLVER_CACHE = false;

const int NOT_FOUND_SO_FAR = -999;

const int GLYPH_M = 77;

const double COLORBACKGROUND[3] = { 255 / 255.0, 255 / 255.0, 255 / 255.0 };
const double COLORGRID[3] = { 220 / 255.0, 220 / 255.0, 220 / 255.0 };
const double COLORCROSS[3] = { 150 / 255.0, 150 / 255.0, 150 / 255.0 };
const double COLORNODE[3] = { 150 / 255.0, 0 / 255.0, 0 / 255.0 };
const double COLOREDGE[3] = { 0 / 255.0, 0 / 255.0, 150 / 255.0 };
const double COLORLABEL[3] = { 0 / 255.0, 150 / 255.0, 0 / 255.0 };
const double COLORSELECTED[3] = { 0 / 255.0, 0 / 255.0, 0 / 255.0 };
const double COLORHIGHLIGHTED[3] = { 250 / 255.0, 150 / 255.0, 0 / 255.0 };
const double COLORCROSSED[3] = { 255 / 255.0, 0 / 255.0, 0 / 255.0 };
const double COLORNOTCONNECTED[3] = { 255 / 255.0, 0 / 255.0, 0 / 255.0 };

const double COLORCONTOURS[3] = { 10 / 255.0, 10 / 255.0, 10 / 255.0 };
const double COLORVECTORS[3] = { 0 / 255.0, 0 / 255.0, 0 / 255.0 };
const double COLORMESH[3] = { 150 / 255.0, 70 / 255.0, 0 / 255.0 };

const int LABELSIZE = 6;
const int EDGEWIDTH = 2;
const int NODESIZE = 6;

#endif // UTIL_CONSTANTS_H

