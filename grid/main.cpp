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

#include "../3rdparty/tclap/CmdLine.h"
#include "browser.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    setlocale (LC_NUMERIC, "C");

    a.setWindowIcon(icon("condor"));
    a.setApplicationVersion(versionString());
    a.setOrganizationName("agros");
    a.setOrganizationDomain("agros");
    a.setApplicationName("Grid Browser");

#ifdef Q_WS_MAC
    // don't show icons in menu
    a.setAttribute(Qt::AA_DontShowIconsInMenus, true);
#endif

    // std::string codec
#if QT_VERSION < 0x050000
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
#endif
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    // force number format
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

    BrowserWindow w(argc, argv);
    w.show();

    return a.exec();

    /*
    try
    {
        // command line info
        TCLAP::CmdLine cmd("Solver MUMPS", ' ');

        TCLAP::ValueArg<std::string> solverArg("o", "solver", "Solver", true, "", "string");
        TCLAP::ValueArg<std::string> matrixArg("m", "matrix", "Matrix", true, "", "string");
        TCLAP::ValueArg<std::string> rhsArg("r", "rhs", "RHS", true, "", "string");
        TCLAP::ValueArg<std::string> solutionArg("s", "solution", "Solution", true, "", "string");
        TCLAP::ValueArg<std::string> initialArg("i", "initial", "Initial vector", false, "", "string");

        cmd.add(solverArg);
        cmd.add(matrixArg);
        cmd.add(rhsArg);
        cmd.add(solutionArg);
        cmd.add(initialArg);

        // parse the argv array.
        cmd.parse(argc, argv);

        CSCMatrix<double> *matrix = NULL;
        SimpleVector<double> *rhs = new SimpleVector<double>();
        LinearMatrixSolver<double> *solver;

        
        //if (solverArg.getValue() == "umfpack")
        //{
        //    matrix = new CSCMatrix<double>();
        //    solver = new UMFPackLinearMatrixSolver<double>(matrix, rhs);
        //}
        //else
        
        if (solverArg.getValue() == "mumps")
        {
            matrix = new MumpsMatrix<double>();
            solver = new MumpsSolver<double>(static_cast<MumpsMatrix<double> *>(matrix), rhs);
        }

        matrix->import_from_file(matrixArg.getValue().c_str(), "matrix", EXPORT_FORMAT_BSON);
        rhs->import_from_file(rhsArg.getValue().c_str(), "rhs", EXPORT_FORMAT_BSON);

        // solve
        // solver->set_verbose_output(true);
        solver->solve();

        // sln vector
        SimpleVector<double> *solution = new SimpleVector<double>(rhs->get_size());
        solution->alloc(rhs->get_size());
        solution->set_vector(solver->get_sln_vector());
        solution->export_to_file(solutionArg.getValue(), "sln", EXPORT_FORMAT_BSON);

        delete matrix;
        delete rhs;
        delete solution;
    }
    catch (TCLAP::ArgException &e)
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        return 1;
    }
    */

    return 0;
}

