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

#ifndef UTIL_ENUMS_H
#define UTIL_ENUMS_H

#include "QtCore"

#include "util.h"
#include "util/table.h"

#undef signals
#include <deal.II/base/time_stepping.h>
#define signals public

enum CoordinateType
{
    CoordinateType_Undefined = -1,
    CoordinateType_Planar = 0,
    CoordinateType_Axisymmetric = 1
};

enum AnalysisType
{
    AnalysisType_Undefined = -1,
    AnalysisType_SteadyState = 1,
    AnalysisType_Harmonic = 3
};

enum AdaptivityMethod
{
    AdaptivityMethod_Undefined = 1000,
    AdaptivityMethod_None = 3,
    AdaptivityMethod_H = 1,
    AdaptivityMethod_P = 2,
    AdaptivityMethod_HP = 0
};

enum AdaptivityEstimator
{
    AdaptivityEstimator_Undefined = -1,
    AdaptivityEstimator_Kelly = 0,
    AdaptivityEstimator_Gradient = 1
};

enum LinearityType
{
    LinearityType_Undefined = -1,
    LinearityType_Linear = 0,
    LinearityType_Picard = 1,
    LinearityType_Newton = 2
};

enum DampingType
{
    DampingType_Undefined = -1,
    DampingType_Automatic = 0,
    DampingType_Fixed = 1,
    DampingType_Off = 2
};

enum CouplingType
{
    CouplingType_Undefined = -1,
    CouplingType_None = 0,
    CouplingType_Weak = 1,
    CouplingType_Hard = 2
};

enum MeshType
{
    MeshType_Undefined = -1,
    MeshType_Triangle = 0,
    // MeshType_Triangle_QuadFineDivision = 1,
    // MeshType_Triangle_QuadRoughDivision = 2,
    // MeshType_Triangle_QuadJoin = 3,
    // MeshType_GMSH_Triangle = 4,
    // MeshType_GMSH_Quad = 5,
    // MeshType_GMSH_QuadDelaunay_Experimental = 6,
    // MeshType_NETGEN_Triangle = 10,
    // MeshType_NETGEN_QuadDominated = 11
    MeshType_CUBIT = 12
};

enum PhysicFieldVariableComp
{
    PhysicFieldVariableComp_Undefined = -1,
    PhysicFieldVariableComp_Scalar = 0,
    PhysicFieldVariableComp_Magnitude = 1,
    PhysicFieldVariableComp_X = 2,
    PhysicFieldVariableComp_Y = 3
};

enum WeakFormKind
{
    WeakForm_MatVol = 0,
    WeakForm_MatSurf = 1,
    WeakForm_VecVol = 2,
    WeakForm_VecSurf = 3,
    WeakForm_ExactSol = 4
};

enum WeakFormVariant
{
    WeakFormVariant_Normal = 0,
    WeakFormVariant_Residual = 1,
    WeakFormVariant_TimeResidual = 2
};

enum SceneGeometryMode
{
    SceneGeometryMode_OperateOnNodes = 0,
    SceneGeometryMode_OperateOnEdges = 1,
    SceneGeometryMode_OperateOnLabels = 2
};

enum MouseSceneMode
{
    MouseSceneMode_Nothing = 0,
    MouseSceneMode_Pan = 1,
    MouseSceneMode_Rotate = 2,
    MouseSceneMode_Move = 3,
    MouseSceneMode_Add = 4
};

enum SceneModePostprocessor
{
    SceneModePostprocessor_Empty = 0,
    SceneModePostprocessor_LocalValue = 1,
    SceneModePostprocessor_SurfaceIntegral = 2,
    SceneModePostprocessor_VolumeIntegral = 3
};

enum PaletteType
{
    Palette_Agros2D = 0,
    Palette_Jet = 1,
    Palette_Copper = 2,
    Palette_Hot = 3,
    Palette_Cool = 4,
    Palette_Bone = 5,
    Palette_Pink = 6,
    Palette_Spring = 7,
    Palette_Summer = 8,
    Palette_Autumn = 9,
    Palette_Winter = 10,
    Palette_HSV = 11,
    Palette_BWAsc = 12,
    Palette_BWDesc = 13
};

enum PaletteOrderType
{
    PaletteOrder_Agros = 0,
    PaletteOrder_Jet = 1,
    PaletteOrder_Copper = 2,
    PaletteOrder_Hot = 3,
    PaletteOrder_Cool = 4,
    PaletteOrder_Bone = 5,
    PaletteOrder_Pink = 6,
    PaletteOrder_Spring = 7,
    PaletteOrder_Summer = 8,
    PaletteOrder_Autumn = 9,
    PaletteOrder_Winter = 10,
    PaletteOrder_HSV = 11,
    PaletteOrder_BWAsc = 12,
    PaletteOrder_BWDesc = 13
};


enum ChartAxisType
{
    ChartAxis_X = 0,
    ChartAxis_Y = 1,
    ChartAxis_Length = 2
};

enum SceneViewPost3DMode
{
    SceneViewPost3DMode_None = -1,
    SceneViewPost3DMode_ScalarView3D = 0,
    SceneViewPost3DMode_ScalarView3DSolid = 1,
    SceneViewPost3DMode_Model = 2
};

enum SceneTransformMode
{
    SceneTransformMode_Translate = 0,
    SceneTransformMode_Rotate = 1,
    SceneTransformMode_Scale = 2
};

enum SolutionMode
{
    SolutionMode_Undefined = -1,
    SolutionMode_Normal = 0,
    SolutionMode_Reference = 1,
    SolutionMode_Finer = 100 // used to choose reference if exists, normal otherwise
};

enum VectorType
{
    VectorType_Arrow = 0,
    VectorType_Cone = 1
};

enum VectorCenter
{
    VectorCenter_Tail = 0,
    VectorCenter_Head = 1,
    VectorCenter_Center = 2
};

enum DataTableType
{
    DataTableType_Undefined = -1,
    DataTableType_CubicSpline = 0,
    DataTableType_PiecewiseLinear = 1,
    DataTableType_Constant = 2
};

enum SpecialFunctionType
{
    SpecialFunctionType_Constant = 0,
    SpecialFunctionType_Function1D = 1
};

enum TimeStepMethodType
{
    TimeStepMethodType_Implicit,
    TimeStepMethodType_Explicit,
    TimeStepMethodType_EmbeddedExplicit
};

enum MatrixSolverType
{
    SOLVER_UMFPACK = 0,
    SOLVER_MUMPS = 4,
    SOLVER_EXTERNAL = 8,
    SOLVER_DEALII,
    SOLVER_EMPTY = 100
};

enum DirectMatrixSolverType
{
    DIRECT_SOLVER_UMFPACK = 0,
    DIRECT_SOLVER_MUMPS = 4,
    DIRECT_SOLVER_SUPERLU = 5,
    DIRECT_SOLVER_AMESOS = 6,
    // Solver external is here, because direct solvers are used in projections.
    DIRECT_SOLVER_EXTERNAL = 8
};

enum IterSolverType
{
    IterSolverType_CG = 0,
    IterSolverType_GMRES = 1,
    IterSolverType_BiCGStab = 2,
    IterSolverType_Richardson = 3,
    IterSolverType_MinRes = 4,
    IterSolverType_GMRS = 5,
    IterSolverType_Relaxation = 6
};

enum PreconditionerType
{
    PreconditionerType_Identity = 0,
    PreconditionerType_Richardson = 1,
    PreconditionerType_UseMatrix = 2,
    PreconditionerType_Relaxation = 3,
    PreconditionerType_Jacobi = 4,
    PreconditionerType_SOR = 5,
    PreconditionerType_SSOR = 6,
    PreconditionerType_PSOR = 7,
    PreconditionerType_LACSolver = 8,
    PreconditionerType_Chebyshev = 9
};

enum MatrixExportFormat
{
    /// \brief Plain ascii file
    /// lines contains row column and value
    EXPORT_FORMAT_PLAIN_ASCII = 1,
    /// Binary MATio format
    EXPORT_FORMAT_MATLAB_MATIO = 4,
    /// \brief Matrix Market which can be read by pysparse library
    EXPORT_FORMAT_MATRIX_MARKET = 3
};

enum NormType
{
    NormType_L2_NORM,
    NormType_H1_NORM,
    NormType_H1_SEMINORM,
    NormType_HCURL_NORM,
    NormType_HDIV_NORM,
    NormType_UNSET_NORM
};

enum SymFlag
{
    SymFlag_ANTISYM = -1,
    SymFlag_NONSYM = 0,
    SymFlag_SYM = 1
};

// keys
AGROS_LIBRARY_API void initLists();

// coordinate type
AGROS_LIBRARY_API QString coordinateTypeString(CoordinateType coordinateType);
AGROS_LIBRARY_API QStringList coordinateTypeStringKeys();
AGROS_LIBRARY_API QString coordinateTypeToStringKey(CoordinateType coordinateType);
AGROS_LIBRARY_API CoordinateType coordinateTypeFromStringKey(const QString &coordinateType);

// analysis type
AGROS_LIBRARY_API QString analysisTypeString(AnalysisType analysisType);
AGROS_LIBRARY_API QStringList analysisTypeStringKeys();
AGROS_LIBRARY_API QString analysisTypeToStringKey(AnalysisType analysisType);
AGROS_LIBRARY_API AnalysisType analysisTypeFromStringKey(const QString &analysisType);

// coupling type
AGROS_LIBRARY_API QString couplingTypeString(CouplingType couplingType);
AGROS_LIBRARY_API QStringList couplingTypeStringKeys();
AGROS_LIBRARY_API QString couplingTypeToStringKey(CouplingType couplingType);
AGROS_LIBRARY_API CouplingType couplingTypeFromStringKey(const QString &couplingType);

// weakform type
AGROS_LIBRARY_API QString weakFormString(WeakFormKind weakForm);
AGROS_LIBRARY_API QStringList weakFormStringKeys();
AGROS_LIBRARY_API QString weakFormToStringKey(WeakFormKind weakForm);
AGROS_LIBRARY_API WeakFormKind weakFormFromStringKey(const QString &weakForm);

// weakform variant
AGROS_LIBRARY_API QString weakFormVariantString(WeakFormVariant weakFormVariant);
AGROS_LIBRARY_API QStringList weakFormVariantStringKeys();
AGROS_LIBRARY_API QString weakFormVariantToStringKey(WeakFormVariant weakFormVariant);
AGROS_LIBRARY_API WeakFormVariant weakFormVariantFromStringKey(const QString &weakFormVariant);

// mesh type
AGROS_LIBRARY_API QString meshTypeString(MeshType meshType);
AGROS_LIBRARY_API QStringList meshTypeStringKeys();
AGROS_LIBRARY_API QString meshTypeToStringKey(MeshType meshType);
AGROS_LIBRARY_API MeshType meshTypeFromStringKey(const QString &meshType);

// physic field variable component
AGROS_LIBRARY_API QString physicFieldVariableCompString(PhysicFieldVariableComp physicFieldVariableComp);
AGROS_LIBRARY_API QStringList physicFieldVariableCompTypeStringKeys();
AGROS_LIBRARY_API QString physicFieldVariableCompToStringKey(PhysicFieldVariableComp physicFieldVariableComp);
AGROS_LIBRARY_API PhysicFieldVariableComp physicFieldVariableCompFromStringKey(const QString &physicFieldVariableComp);

// adaptivity type
AGROS_LIBRARY_API QString adaptivityTypeString(AdaptivityMethod adaptivityType);
AGROS_LIBRARY_API QStringList adaptivityTypeStringKeys();
AGROS_LIBRARY_API QString adaptivityTypeToStringKey(AdaptivityMethod adaptivityType);
AGROS_LIBRARY_API AdaptivityMethod adaptivityTypeFromStringKey(const QString &adaptivityType);

// adaptivity estimator
AGROS_LIBRARY_API QString adaptivityEstimatorString(AdaptivityEstimator adaptivityEstimator);
AGROS_LIBRARY_API QStringList adaptivityEstimatorStringKeys();
AGROS_LIBRARY_API QString adaptivityEstimatorToStringKey(AdaptivityEstimator adaptivityEstimator);
AGROS_LIBRARY_API AdaptivityEstimator adaptivityEstimatorFromStringKey(const QString &adaptivityEstimator);

// adaptivity norm type
AGROS_LIBRARY_API QString errorNormString(NormType projNormType);
AGROS_LIBRARY_API QStringList adaptivityNormTypeStringKeys();
AGROS_LIBRARY_API QString adaptivityNormTypeToStringKey(NormType adaptivityNormType);
AGROS_LIBRARY_API NormType adaptivityNormTypeFromStringKey(const QString &adaptivityNormType);

// time step method
AGROS_LIBRARY_API QString timeStepMethodString(dealii::TimeStepping::runge_kutta_method timeStepMethod);
AGROS_LIBRARY_API QStringList timeStepMethodStringKeys();
AGROS_LIBRARY_API QString timeStepMethodToStringKey(dealii::TimeStepping::runge_kutta_method timeStepMethod);
AGROS_LIBRARY_API dealii::TimeStepping::runge_kutta_method timeStepMethodFromStringKey(const QString &timeStepMethod);
AGROS_LIBRARY_API TimeStepMethodType timeStepMethodType(dealii::TimeStepping::runge_kutta_method timeStepMethod);

// solution mode
AGROS_LIBRARY_API QString solutionTypeString(SolutionMode solutionMode);
AGROS_LIBRARY_API QStringList solutionTypeStringKeys();
AGROS_LIBRARY_API QString solutionTypeToStringKey(SolutionMode solutionType);
AGROS_LIBRARY_API SolutionMode solutionTypeFromStringKey(const QString &solutionType);

// matrix solver type
AGROS_LIBRARY_API bool isMatrixSolverIterative(MatrixSolverType type);
AGROS_LIBRARY_API QString matrixSolverTypeString(MatrixSolverType matrixSolverType);
AGROS_LIBRARY_API QStringList matrixSolverTypeStringKeys();
AGROS_LIBRARY_API QString matrixSolverTypeToStringKey(MatrixSolverType matrixSolverType);
AGROS_LIBRARY_API MatrixSolverType matrixSolverTypeFromStringKey(const QString &matrixSolverType);

// matrix dump format
AGROS_LIBRARY_API QString dumpFormatString(MatrixExportFormat format);
AGROS_LIBRARY_API QStringList dumpFormatStringKeys();
AGROS_LIBRARY_API QString dumpFormatToStringKey(MatrixExportFormat format);
AGROS_LIBRARY_API MatrixExportFormat dumpFormatFromStringKey(const QString &format);

// linearity type
AGROS_LIBRARY_API QString linearityTypeString(LinearityType linearityType);
AGROS_LIBRARY_API QStringList linearityTypeStringKeys();
AGROS_LIBRARY_API QString linearityTypeToStringKey(LinearityType linearityType);
AGROS_LIBRARY_API LinearityType linearityTypeFromStringKey(const QString &linearityType);

// damping type
AGROS_LIBRARY_API QString dampingTypeString(DampingType dampingType);
AGROS_LIBRARY_API QStringList dampingTypeStringKeys();
AGROS_LIBRARY_API QString dampingTypeToStringKey(DampingType dampingType);
AGROS_LIBRARY_API DampingType dampingTypeFromStringKey(const QString &dampingType);

// scene view 3d mode
AGROS_LIBRARY_API QStringList sceneViewPost3DModeStringKeys();
AGROS_LIBRARY_API QString sceneViewPost3DModeToStringKey(SceneViewPost3DMode sceneViewPost3DMode);
AGROS_LIBRARY_API SceneViewPost3DMode sceneViewPost3DModeFromStringKey(const QString &sceneViewPost3DMode);

// palette type
AGROS_LIBRARY_API QStringList paletteTypeStringKeys();
AGROS_LIBRARY_API QString paletteTypeToStringKey(PaletteType paletteType);
AGROS_LIBRARY_API PaletteType paletteTypeFromStringKey(const QString &paletteType);
AGROS_LIBRARY_API QString paletteTypeString(PaletteType paletteType);

// palette order
AGROS_LIBRARY_API QStringList paletteOrderTypeStringKeys();
AGROS_LIBRARY_API QString paletteOrderTypeToStringKey(PaletteOrderType paletteType);
AGROS_LIBRARY_API PaletteOrderType paletteOrderTypeFromStringKey(const QString &paletteType);

// vector type
AGROS_LIBRARY_API QString vectorTypeString(VectorType vectorType);
AGROS_LIBRARY_API QStringList vectorTypeStringKeys();
AGROS_LIBRARY_API QString vectorTypeToStringKey(VectorType vectorType);
AGROS_LIBRARY_API VectorType vectorTypeFromStringKey(const QString &vectorType);

// vector center
AGROS_LIBRARY_API QString vectorCenterString(VectorCenter vectorCenter);
AGROS_LIBRARY_API QStringList vectorCenterStringKeys();
AGROS_LIBRARY_API QString vectorCenterToStringKey(VectorCenter vectorCenter);
AGROS_LIBRARY_API VectorCenter vectorCenterFromStringKey(const QString &vectorCenter);

// data table type
AGROS_LIBRARY_API QString dataTableTypeString(DataTableType dataTableType);
AGROS_LIBRARY_API QStringList dataTableTypeStringKeys();
AGROS_LIBRARY_API QString dataTableTypeToStringKey(DataTableType dataTableType);
AGROS_LIBRARY_API DataTableType dataTableTypeFromStringKey(const QString &dataTableType);

// special function type
AGROS_LIBRARY_API QString specialFunctionTypeString(SpecialFunctionType specialFunctionType);
AGROS_LIBRARY_API QStringList specialFunctionTypeStringKeys();
AGROS_LIBRARY_API QString specialFunctionTypeToStringKey(SpecialFunctionType specialFunctionType);
AGROS_LIBRARY_API SpecialFunctionType specialFunctionTypeFromStringKey(const QString &specialFunctionType);

// butcher table type
AGROS_LIBRARY_API QString butcherTableTypeString(ButcherTableType tableType);
AGROS_LIBRARY_API QStringList butcherTableTypeStringKeys();
AGROS_LIBRARY_API QString butcherTableTypeToStringKey(ButcherTableType tableType);
AGROS_LIBRARY_API ButcherTableType butcherTableTypeFromStringKey(const QString &tableType);

// iterative solver - method
AGROS_LIBRARY_API QString iterLinearSolverMethodString(IterSolverType type);
AGROS_LIBRARY_API QStringList iterLinearSolverMethodStringKeys();
AGROS_LIBRARY_API QString iterLinearSolverMethodToStringKey(IterSolverType type);
AGROS_LIBRARY_API IterSolverType iterLinearSolverMethodFromStringKey(const QString &type);

// iterative solver - preconditioner
AGROS_LIBRARY_API QString iterLinearSolverPreconditionerTypeString(PreconditionerType type);
AGROS_LIBRARY_API QStringList iterLinearSolverPreconditionerTypeStringKeys();
AGROS_LIBRARY_API QString iterLinearSolverPreconditionerTypeToStringKey(PreconditionerType type);
AGROS_LIBRARY_API PreconditionerType iterLinearSolverPreconditionerTypeFromStringKey(const QString &type);

#endif // UTIL_ENUMS_H
