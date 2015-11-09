# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.2

# The generator used is:
set(CMAKE_DEPENDS_GENERATOR "Unix Makefiles")

# The top level Makefile was generated from the following files:
set(CMAKE_MAKEFILE_DEPENDS
  "CMakeCache.txt"
  "../CMakeLists.txt"
  "CMakeFiles/3.2.2/CMakeCCompiler.cmake"
  "CMakeFiles/3.2.2/CMakeCXXCompiler.cmake"
  "CMakeFiles/3.2.2/CMakeSystem.cmake"
  "../cmake/FindMKL.cmake"
  "../cmake/FindOpenCL.cmake"
  "../src/CMakeLists.txt"
  "../src/base/CMakeLists.txt"
  "../src/base/backend_manager.hpp"
  "../src/base/base_matrix.hpp"
  "../src/base/base_paralution.hpp"
  "../src/base/base_stencil.hpp"
  "../src/base/base_vector.hpp"
  "../src/base/host/CMakeLists.txt"
  "../src/base/host/host_affinity.hpp"
  "../src/base/host/host_conversion.hpp"
  "../src/base/host/host_io.hpp"
  "../src/base/host/host_matrix_bcsr.hpp"
  "../src/base/host/host_matrix_coo.hpp"
  "../src/base/host/host_matrix_csr.hpp"
  "../src/base/host/host_matrix_dense.hpp"
  "../src/base/host/host_matrix_dia.hpp"
  "../src/base/host/host_matrix_ell.hpp"
  "../src/base/host/host_matrix_hyb.hpp"
  "../src/base/host/host_matrix_mcsr.hpp"
  "../src/base/host/host_stencil_laplace2d.hpp"
  "../src/base/host/host_vector.hpp"
  "../src/base/local_matrix.hpp"
  "../src/base/local_stencil.hpp"
  "../src/base/local_vector.hpp"
  "../src/base/matrix_formats.hpp"
  "../src/base/matrix_formats_ind.hpp"
  "../src/base/ocl/CMakeLists.txt"
  "../src/base/ocl/backend_ocl.hpp"
  "../src/base/ocl/kernels_ocl.hpp"
  "../src/base/ocl/ocl_allocate_free.hpp"
  "../src/base/ocl/ocl_matrix_bcsr.hpp"
  "../src/base/ocl/ocl_matrix_coo.hpp"
  "../src/base/ocl/ocl_matrix_csr.hpp"
  "../src/base/ocl/ocl_matrix_dense.hpp"
  "../src/base/ocl/ocl_matrix_dia.hpp"
  "../src/base/ocl/ocl_matrix_ell.hpp"
  "../src/base/ocl/ocl_matrix_hyb.hpp"
  "../src/base/ocl/ocl_matrix_mcsr.hpp"
  "../src/base/ocl/ocl_vector.hpp"
  "../src/base/operator.hpp"
  "../src/base/stencil_types.hpp"
  "../src/base/vector.hpp"
  "../src/base/version.hpp"
  "../src/paralution.hpp"
  "../src/plug-ins/CMakeLists.txt"
  "../src/plug-ins/OpenFOAM/matrices/lduMatrix/solvers/paralution/paralution_openfoam.H"
  "../src/plug-ins/OpenFOAM/matrices/lduMatrix/solvers/paralution_AMG/paralution_AMG.C"
  "../src/plug-ins/OpenFOAM/matrices/lduMatrix/solvers/paralution_AMG/paralution_AMG.H"
  "../src/plug-ins/OpenFOAM/matrices/lduMatrix/solvers/paralution_PBiCG/paralution_PBiCG.C"
  "../src/plug-ins/OpenFOAM/matrices/lduMatrix/solvers/paralution_PBiCG/paralution_PBiCG.H"
  "../src/plug-ins/OpenFOAM/matrices/lduMatrix/solvers/paralution_PCG/paralution_PCG.C"
  "../src/plug-ins/OpenFOAM/matrices/lduMatrix/solvers/paralution_PCG/paralution_PCG.H"
  "../src/plug-ins/OpenFOAM/matrices/lduMatrix/solvers/paralution_PGMRES/paralution_PGMRES.C"
  "../src/plug-ins/OpenFOAM/matrices/lduMatrix/solvers/paralution_PGMRES/paralution_PGMRES.H"
  "../src/plug-ins/paralution_dealii.hpp"
  "../src/plug-ins/paralution_fortran.cpp"
  "../src/solvers/CMakeLists.txt"
  "../src/solvers/chebyshev.hpp"
  "../src/solvers/deflation/dpcg.hpp"
  "../src/solvers/direct/inversion.hpp"
  "../src/solvers/direct/lu.hpp"
  "../src/solvers/direct/qr.hpp"
  "../src/solvers/eigenvalue/ampe_sira.hpp"
  "../src/solvers/eigenvalue/cg_hn.hpp"
  "../src/solvers/iter_ctrl.hpp"
  "../src/solvers/krylov/bicgstab.hpp"
  "../src/solvers/krylov/cg.hpp"
  "../src/solvers/krylov/cr.hpp"
  "../src/solvers/krylov/fgmres.hpp"
  "../src/solvers/krylov/gmres.hpp"
  "../src/solvers/krylov/idr.hpp"
  "../src/solvers/mixed_precision.hpp"
  "../src/solvers/multigrid/amg.hpp"
  "../src/solvers/multigrid/base_amg.hpp"
  "../src/solvers/multigrid/base_multigrid.hpp"
  "../src/solvers/multigrid/multigrid.hpp"
  "../src/solvers/preconditioners/preconditioner.hpp"
  "../src/solvers/preconditioners/preconditioner_ai.hpp"
  "../src/solvers/preconditioners/preconditioner_as.hpp"
  "../src/solvers/preconditioners/preconditioner_blockprecond.hpp"
  "../src/solvers/preconditioners/preconditioner_multicolored.hpp"
  "../src/solvers/preconditioners/preconditioner_multicolored_gs.hpp"
  "../src/solvers/preconditioners/preconditioner_multicolored_ilu.hpp"
  "../src/solvers/preconditioners/preconditioner_multielimination.hpp"
  "../src/solvers/preconditioners/preconditioner_saddlepoint.hpp"
  "../src/solvers/solver.hpp"
  "../src/utils/CMakeLists.txt"
  "../src/utils/allocate_free.hpp"
  "../src/utils/info.hpp"
  "../src/utils/log.hpp"
  "../src/utils/math_functions.hpp"
  "../src/utils/paralution.ptk.in"
  "../src/utils/time_functions.hpp"
  "/usr/share/cmake-3.2/Modules/CMakeCInformation.cmake"
  "/usr/share/cmake-3.2/Modules/CMakeCXXInformation.cmake"
  "/usr/share/cmake-3.2/Modules/CMakeCommonLanguageInclude.cmake"
  "/usr/share/cmake-3.2/Modules/CMakeGenericSystem.cmake"
  "/usr/share/cmake-3.2/Modules/CMakeParseArguments.cmake"
  "/usr/share/cmake-3.2/Modules/CMakeSystemSpecificInformation.cmake"
  "/usr/share/cmake-3.2/Modules/CMakeSystemSpecificInitialize.cmake"
  "/usr/share/cmake-3.2/Modules/Compiler/GNU-C.cmake"
  "/usr/share/cmake-3.2/Modules/Compiler/GNU-CXX.cmake"
  "/usr/share/cmake-3.2/Modules/Compiler/GNU.cmake"
  "/usr/share/cmake-3.2/Modules/FindCUDA.cmake"
  "/usr/share/cmake-3.2/Modules/FindOpenMP.cmake"
  "/usr/share/cmake-3.2/Modules/FindPackageHandleStandardArgs.cmake"
  "/usr/share/cmake-3.2/Modules/FindPackageMessage.cmake"
  "/usr/share/cmake-3.2/Modules/Platform/Linux-GNU-C.cmake"
  "/usr/share/cmake-3.2/Modules/Platform/Linux-GNU-CXX.cmake"
  "/usr/share/cmake-3.2/Modules/Platform/Linux-GNU.cmake"
  "/usr/share/cmake-3.2/Modules/Platform/Linux.cmake"
  "/usr/share/cmake-3.2/Modules/Platform/UnixPaths.cmake"
  )

# The corresponding makefile is:
set(CMAKE_MAKEFILE_OUTPUTS
  "Makefile"
  "CMakeFiles/cmake.check_cache"
  )

# Byproducts of CMake generate step:
set(CMAKE_MAKEFILE_PRODUCTS
  "paralution.ptk"
  "CMakeFiles/CMakeDirectoryInformation.cmake"
  "src/CMakeFiles/CMakeDirectoryInformation.cmake"
  )

# Dependency information for all targets:
set(CMAKE_DEPEND_INFO_FILES
  "src/CMakeFiles/agros2d_paralution.dir/DependInfo.cmake"
  )
