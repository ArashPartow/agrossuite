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

#include "mpi/mpi.h"

#include <streambuf>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <set>

#define JOB_INIT -1
#define JOB_END -2
#define USE_COMM_WORLD -987654

#include <petscksp.h>
#include <petscmat.h>
#include <petscblaslapack.h>



#include "../../3rdparty/tclap/CmdLine.h"
#include "../util/sparse_io.h"


class LinearSystemPETScArgs : public LinearSystemArgs
{

    // another used args (not listed here): -s, -r, -p, -m, -q
public:
    LinearSystemPETScArgs(const std::string &name, int argc, const char * const *argv)
        : LinearSystemArgs(name, argc, argv),
          solverArg(TCLAP::ValueArg<std::string>("l", "solver", "Solver", false, "", "string")),
          preconditionerArg(TCLAP::ValueArg<std::string>("c", "preconditioner", "Preconditioner", false, "", "string")),
          aggregationTypeArg(TCLAP::ValueArg<std::string>("e", "aggregationType", "AggregationType", false, "", "string")),
          smootherTypeArg(TCLAP::ValueArg<std::string>("o", "smootherType", "SmootherType", false, "", "string")),
          coarseTypeArg(TCLAP::ValueArg<std::string>("z", "coarseType", "CoarseType", false, "", "string")),
          absTolArg(TCLAP::ValueArg<double>("a", "abs_tol", "Absolute tolerance", false, 1e-13, "double")),
          relTolArg(TCLAP::ValueArg<double>("t", "rel_tol", "Relative tolerance", false, 1e-9, "double")),
          maxIterArg(TCLAP::ValueArg<int>("x", "max_iter", "Maximum number of iterations", false, 1000, "int")),
          multigridArg(TCLAP::SwitchArg("g", "multigrid", "Algebraic multigrid", false))
    {
        cmd.add(solverArg);
        cmd.add(preconditionerArg);
        cmd.add(aggregationTypeArg);
        cmd.add(smootherTypeArg);
        cmd.add(coarseTypeArg);
        cmd.add(absTolArg);
        cmd.add(relTolArg);
        cmd.add(maxIterArg);
        cmd.add(multigridArg);
    }

    TCLAP::ValueArg<std::string> solverArg;
    TCLAP::ValueArg<std::string> preconditionerArg;
    TCLAP::ValueArg<std::string> aggregationTypeArg;
    TCLAP::ValueArg<std::string> smootherTypeArg;
    TCLAP::ValueArg<std::string> coarseTypeArg;
    TCLAP::ValueArg<double> absTolArg;
    TCLAP::ValueArg<double> relTolArg;
    TCLAP::ValueArg<int> maxIterArg;
    TCLAP::SwitchArg multigridArg;
};

LinearSystemPETScArgs *createLinearSystem(std::string extSolverName, int argc, char *argv[])
{
    LinearSystemPETScArgs *linearSystem = new LinearSystemPETScArgs(extSolverName, argc, argv);
    linearSystem->readLinearSystem();

    // create empty solution vector (Agros2D)
    linearSystem->system_sln->resize(linearSystem->system_rhs->max_len);
    linearSystem->convertToCOO();

    return linearSystem;
}

// usage:
// LinearSystemTrilinosArgs *linearSystem = nullptr;
// ...
// linearSystem = createLinearSystem("External solver - TRILINOS", argc, argv);
// -----
// get parameters to local value
// double relTol = linearSystem->relTolArg.getValue();
// int maxIter = linearSystem->maxIterArg.getValue();


KSPType solver(std::string solver)
{
    if(solver == "richardson")
        return KSPRICHARDSON;
    else if( solver == "chebyshev")
        return KSPCHEBYSHEV;
    else if( solver == "cg")
        return KSPCG;
    else if (solver == "groppcg")
        return KSPGROPPCG;
    else if (solver == "pipecg")
        return KSPPIPECG;
    else if (solver == "cgne")
        return   KSPCGNE;
    else if (solver == "nash")
        return KSPNASH;
    else if (solver == "stcg")
        return KSPSTCG;
    else if (solver == "gltr")
        return KSPGLTR;
    else if (solver == "gmres")
        return KSPGMRES;
    else if (solver == "fgmres")
        return KSPFGMRES;
    else if (solver == "lgmres")
        return KSPLGMRES;
    else if (solver == "dgmres")
        return KSPLGMRES;
    else if (solver == "pgmres")
        return KSPPGMRES;
    else if (solver == "tcqmr")
        return KSPTCQMR;
    else if (solver == "bcgs")
        return KSPBCGS;
    else if (solver == "ibcgs")
        return KSPIBCGS;
    else if (solver == "fbcgs")
        return KSPFBCGS;
    else if (solver == "fbcgsr")
        return KSPFBCGSR;
    else if (solver == "bcgsl")
        return KSPBCGSL;
    else if (solver == "cgs")
        return KSPCGS;
    else if (solver == "tfqmr")
        return KSPTFQMR;
    else if (solver == "cr")
        return KSPCR;
    else if (solver == "pipecr")
        return KSPPIPECR;
    else if (solver == "lsqr")
        return KSPLSQR;
    else if (solver == "preonly")
        return KSPPREONLY;
    else if (solver == "qcg")
        return KSPQCG;
    else if (solver == "bicg")
        return KSPBICG;
    else if (solver == "minres")
        return KSPMINRES;
    else if (solver == "symmlq")
        return KSPSYMMLQ;
    else if (solver == "lcd")
        return KSPLCD;
    else if (solver == "python")
        return KSPPYTHON;
    else if (solver == "gcr")
        return KSPGCR;
    else
        return KSPCG;
}

PCType preConditioner(std::string preConditioner)
{
    if( preConditioner == "jacobi")
        return PCJACOBI;
    else if (preConditioner == "hypre")
        return PCHYPRE;
    else if (preConditioner == "sor")
        return PCSOR;
    else if (preConditioner == "lu")
        return PCLU;
    else if (preConditioner == "mg")
        return PCMG;
    else if (preConditioner == "shell")
        return PCSHELL;
    else if (preConditioner == "bjacobi")
        return PCBJACOBI;
    else if (preConditioner == "eisenstat")
        return PCEISENSTAT;
    else if (preConditioner == "ilu")
        return PCILU;
    else if (preConditioner == "icc")
        return PCICC;
    else if (preConditioner == "asm")
        return PCASM;
    else if (preConditioner == "gasm")
        return PCGASM;
    else if (preConditioner == "ksp")
        return PCKSP;
    else if (preConditioner == "composite")
        return PCCOMPOSITE;
    else if (preConditioner == "redundant")
        return PCREDUNDANT;
    else if (preConditioner == "spai")
        return PCSPAI;
    else if (preConditioner == "nn")
        return PCNN;
    else if (preConditioner == "cholesky")
        return PCCHOLESKY;
    else if (preConditioner == "pbjacobi")
        return PCPBJACOBI;
    else if (preConditioner == "mat")
        return PCMAT;
    else if (preConditioner == "parms")
        return PCPARMS;
    else if (preConditioner == "fieldsplit")
        return PCFIELDSPLIT;
    else if (preConditioner == "fieldsplit")
        return PCFIELDSPLIT;
    else if (preConditioner == "tfs")
        return PCTFS;
    else if (preConditioner == "ml")
        return PCML;
    else if (preConditioner == "galerkin")
        return PCGALERKIN;
    else if (preConditioner == "exotic")
        return PCEXOTIC;
    else if (preConditioner == "cp")
        return PCCP;
    else if (preConditioner == "bfbt")
        return PCBFBT;
    else if (preConditioner == "lsc")
        return PCLSC;
    else if (preConditioner == "python")
        return PCPYTHON;
    else if (preConditioner == "pfmg")
        return PCPFMG;
    else if (preConditioner == "syspfmg")
        return PCSYSPFMG;
    else if (preConditioner == "redistribute")
        return PCREDISTRIBUTE;
    else if (preConditioner == "svd")
        return PCSVD;
    else if (preConditioner == "gamg")
        return PCGAMG;
    else if (preConditioner == "sacusp")
        return PCSACUSP; /* these four run on NVIDIA GPUs using CUSP */
    else if (preConditioner == "sacusppoly")
        return PCSACUSPPOLY;
    else if (preConditioner == "bicgstabcusp")
        return PCBICGSTABCUSP;
    else if (preConditioner == "ainvcusp")
        return PCAINVCUSP;
    else if (preConditioner == "bddc")
        return PCBDDC;
    else
        return PCBJACOBI;
}

PetscErrorCode assembleRHS(LinearSystemPETScArgs *linearSystem, Vec &b)
{
    int ierr = -1;
    int istart = 0;
    int iend = 0;

    // local assemble
    ierr = VecGetOwnershipRange(b, &istart, &iend); CHKERRQ(ierr);
    PetscInt *vecIdx = new PetscInt[iend - istart];
    PetscScalar *vecVal = new PetscScalar[iend - istart];
    for (int i = 0; i < iend - istart; i++)
    {
        vecIdx[i] = istart + i;
        vecVal[i] = linearSystem->system_rhs->val[istart + i];
    }

    VecSetValues(b, iend - istart, vecIdx, vecVal, INSERT_VALUES);

    ierr = VecAssemblyBegin(b); CHKERRQ(ierr);
    ierr = VecAssemblyEnd(b); CHKERRQ(ierr);

    PetscFree(vecIdx);
    PetscFree(vecVal);
}

void getCSR(LinearSystemPETScArgs *linearSystem, int start, int end,
            PetscInt *csrRowPtr, PetscInt *csrColInd, PetscScalar *csrVal = nullptr)
{
    // loop over the elements of the matrix row by row
    unsigned int index = 0;
    for (unsigned int row = 0; row < end - start; row++)
    {
        std::size_t col_start = linearSystem->system_matrix_pattern->rowstart[row + start];
        std::size_t col_end = linearSystem->system_matrix_pattern->rowstart[row + start + 1];

        csrRowPtr[row] = index;

        for (unsigned int i = col_start; i < col_end; i++)
        {
            csrColInd[index] = linearSystem->system_matrix_pattern->colnums[i];
            if (csrVal)
                csrVal[index] = linearSystem->matA[i];

            index++;
        }
    }
    csrRowPtr[end - start] = index;
}

PetscErrorCode assembleMatrix(LinearSystemPETScArgs *linearSystem, Mat &A)
{
    int ierr = -1;
    int istart = 0;
    int iend = 0;

    PetscInt *csrRowPtr = new PetscInt[linearSystem->n() + 1];
    PetscInt *csrColInd = new PetscInt[linearSystem->nz()];

    getCSR(linearSystem, 0, linearSystem->n(), csrRowPtr, csrColInd);

    // preallocate whole matrix
    ierr = MatMPIAIJSetPreallocationCSR(A, csrRowPtr, csrColInd, PETSC_NULL); CHKERRQ(ierr);
    // ierr = MatSeqAIJSetPreallocationCSR(A, csrRowPtr, csrColInd, PETSC_NULL); CHKERRQ(ierr);

    // local assemble
    ierr = MatGetOwnershipRange(A, &istart, &iend); CHKERRQ(ierr);
    int nzLocal = 0;
    for (unsigned int row = istart; row < iend; row++)
        nzLocal += linearSystem->system_matrix_pattern->rowstart[row + 1] - linearSystem->system_matrix_pattern->rowstart[row];

    PetscInt *csrRowPtrLocal = new PetscInt[iend - istart + 1];
    PetscInt *csrColIndLocal = new PetscInt[nzLocal];
    PetscScalar *csrValLocal = new PetscScalar[nzLocal];

    getCSR(linearSystem, istart, iend, csrRowPtrLocal, csrColIndLocal, csrValLocal);

    ierr = MatCreateMPIAIJWithArrays(PETSC_COMM_WORLD, iend - istart, PETSC_DECIDE,
                                     PETSC_DETERMINE, linearSystem->n(),
                                     csrRowPtrLocal, csrColIndLocal, csrValLocal, &A); CHKERRQ(ierr);
    //  MatView(A, PETSC_VIEWER_STDOUT_SELF);

    ierr = MatAssemblyBegin(A, MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);
    ierr = MatAssemblyEnd(A, MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);

    PetscFree(csrRowPtr);
    PetscFree(csrColInd);

    PetscFree(csrRowPtrLocal);
    PetscFree(csrColIndLocal);
    PetscFree(csrValLocal);
}

int main(int argc, char *argv[])
{
    try
    {
        int status = 0;

        Vec x,b;
        Mat  A;
        PetscMPIInt size;
        PetscErrorCode ierr;
        PetscBool nonzeroguess = PETSC_FALSE;
        KSP ksp;
        PC pc;
        PetscInt n_rows = 0;
        PetscInt n = 0;

        PetscInitialize(&argc,&argv, (char*)0," ");
        ierr = MPI_Comm_size(PETSC_COMM_WORLD, &size); CHKERRQ(ierr);
        int rank;
        ierr = MPI_Comm_rank(PETSC_COMM_WORLD, &rank);
        // std::cout << "rank =  " << rank << std::endl;
        ierr = PetscOptionsGetBool(NULL,"-nonzero_guess", &nonzeroguess,NULL); CHKERRQ(ierr);

        auto timeStart = std::chrono::steady_clock::now();

        LinearSystemPETScArgs *linearSystem = nullptr;
        linearSystem = createLinearSystem("External solver - PETSc", argc, argv);

        n_rows = linearSystem->n();
        n = linearSystem->nz();

        linearSystem->setInfoNumOfProc(size);

        int istart = 0;
        int iend = 0;

        // create vector
        ierr = VecCreateMPI(PETSC_COMM_WORLD, PETSC_DECIDE, n_rows, &x); CHKERRQ(ierr);
        ierr = VecCreateMPI(PETSC_COMM_WORLD, PETSC_DECIDE, n_rows, &b); CHKERRQ(ierr);
        ierr = PetscObjectSetName((PetscObject) x, "Solution"); CHKERRQ(ierr);
        ierr = PetscObjectSetName((PetscObject) b, "RHS"); CHKERRQ(ierr);
        ierr = VecSetFromOptions(x); CHKERRQ(ierr);
        ierr = VecSetFromOptions(b); CHKERRQ(ierr);

        // local assemble
        assembleRHS(linearSystem, b);
        // VecView(b, PETSC_VIEWER_STDOUT_SELF);

        // create matrix
        ierr = MatCreate(PETSC_COMM_WORLD, &A); CHKERRQ(ierr);
        ierr = MatSetType(A, MATMPIAIJ); CHKERRQ(ierr);
        ierr = MatSetSizes(A, PETSC_DECIDE, PETSC_DECIDE, n_rows, n_rows); CHKERRQ(ierr);
        ierr = MatSetFromOptions(A); CHKERRQ(ierr);

        assembleMatrix(linearSystem, A);

        MatInfo matinfo;
        MatGetInfo(A, MAT_LOCAL, &matinfo);
        // std::cout  << "nnz: " << (PetscInt) matinfo.nz_used << ", n: " << (PetscInt) matinfo.block_size << std::endl;
        linearSystem->setInfoTimeReadMatrix(elapsedSeconds(timeStart));

        // Create linear solver context
        ierr = KSPCreate(PETSC_COMM_WORLD, &ksp);CHKERRQ(ierr);
#if (PETSC_VERSION_GT(3,6,0))
        ierr = KSPSetOperators(ksp, A, A); CHKERRQ(ierr);
#else
        ierr = KSPSetOperators(ksp, A, A, DIFFERENT_NONZERO_PATTERN); CHKERRQ(ierr);
#endif
        PetscReal relTol;
        if (linearSystem->relTolArg.isSet())
        {
            relTol = linearSystem->relTolArg.getValue();
        }
        else relTol = PETSC_DEFAULT;

        PetscReal absTol;
        if (linearSystem-> absTolArg.isSet())
        {
            absTol = linearSystem->absTolArg.getValue();
        }
        else absTol = PETSC_DEFAULT;

        PetscInt maxIter;
        if (linearSystem-> maxIterArg.isSet())
        {
            maxIter = linearSystem->maxIterArg.getValue();
        }
        else maxIter = PETSC_DEFAULT;

        ierr = KSPGetPC(ksp, &pc);

        PCSetType(pc, preConditioner(linearSystem->preconditionerArg.getValue()));
        linearSystem->setInfoSolverPreconditionerName(linearSystem->preconditionerArg.getValue());
        linearSystem->setInfoSolverSolverName(linearSystem->solverArg.getValue());

        ierr = KSPSetTolerances(ksp, relTol, absTol, PETSC_DEFAULT, maxIter); CHKERRQ(ierr);
        ierr = KSPSetType(ksp, solver(linearSystem->solverArg.getValue()));
        ierr = KSPSetFromOptions(ksp); CHKERRQ(ierr);

        auto timeSolveStart = std::chrono::steady_clock::now();
        ierr = KSPSolve(ksp, b, x); CHKERRQ(ierr);
        linearSystem->setInfoTimeSolver(elapsedSeconds(timeSolveStart));

        // VecView(x, PETSC_VIEWER_STDOUT_SELF);

        for (int i = 0; i < linearSystem->n(); i++)
        {
            VecGetValues(x, 1, &i, &linearSystem->system_rhs->val[i]);
        }

        if (rank == 0)
        {
            linearSystem->system_sln = linearSystem->system_rhs;
            linearSystem->writeSolution();

            // check solution
            if (linearSystem->hasReferenceSolution())
                status = linearSystem->compareWithReferenceSolution();

            linearSystem->setInfoTimeTotal(elapsedSeconds(timeStart));

            if (linearSystem->verbose() > 0)
            {
                linearSystem->printStatus();

                if (linearSystem->verbose() > 2)
                    linearSystem->exportStatusToFile();
            }
        }

        ierr = VecDestroy(&x); CHKERRQ(ierr);
        ierr = VecDestroy(&b); CHKERRQ(ierr);
        ierr = MatDestroy(&A); CHKERRQ(ierr);
        ierr = KSPDestroy(&ksp); CHKERRQ(ierr);

        ierr = PetscFinalize();

        exit(status);
    }
    catch (TCLAP::ArgException &e)
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        return 1;
    }

    return 1;
}
