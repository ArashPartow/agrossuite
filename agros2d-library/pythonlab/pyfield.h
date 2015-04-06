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

#ifndef PYTHONLABFIELD_H
#define PYTHONLABFIELD_H

#include "util/global.h"
#include "solver/field.h"

class PyField
{
    public:
        PyField(std::string fieldId);
        ~PyField() {}

        template <typename Type>
        void setParameter(const std::string &parameter, Type value)
        {
            assert(m_fieldInfo->stringKeyToType(QString::fromStdString(parameter)));
            m_fieldInfo->setValue(m_fieldInfo->stringKeyToType(QString::fromStdString(parameter)), value);
        }

        inline int getBoolParameter(const std::string &parameter)
        {
            assert(m_fieldInfo->stringKeyToType(QString::fromStdString(parameter)));
            return m_fieldInfo->value(m_fieldInfo->stringKeyToType(QString::fromStdString(parameter))).toBool();
        }

        inline int getIntParameter(const std::string &parameter)
        {
            assert(m_fieldInfo->stringKeyToType(QString::fromStdString(parameter)));
            return m_fieldInfo->value(m_fieldInfo->stringKeyToType(QString::fromStdString(parameter))).toInt();
        }

        inline double getDoubleParameter(const std::string &parameter)
        {
            assert(m_fieldInfo->stringKeyToType(QString::fromStdString(parameter)));
            return m_fieldInfo->value(m_fieldInfo->stringKeyToType(QString::fromStdString(parameter))).toDouble();
        }

        // field id
        inline std::string fieldId() const { return m_fieldInfo->fieldId().toStdString(); }

        // analysis type
        inline std::string getAnalysisType() const { return analysisTypeToStringKey(m_fieldInfo->analysisType()).toStdString(); }
        void setAnalysisType(const std::string &analysisType);

        // linearity type
        inline std::string getLinearityType() const { return linearityTypeToStringKey(m_fieldInfo->linearityType()).toStdString(); }
        void setLinearityType(const std::string &linearityType);

        // automatic damping
        inline std::string getNonlinearDampingType() const { return dampingTypeToStringKey((DampingType)m_fieldInfo->value(FieldInfo::NonlinearDampingType).toInt()).toStdString(); }
        void setNonlinearDampingType(std::string dampingType);

        // adaptivity type
        inline std::string getAdaptivityType() const { return adaptivityTypeToStringKey(m_fieldInfo->adaptivityType()).toStdString(); }
        void setAdaptivityType(const std::string &adaptivityType);

        // matrix solver
        inline std::string getMatrixSolver() const { return matrixSolverTypeToStringKey(m_fieldInfo->matrixSolver()).toStdString(); }
        void setMatrixSolver(const std::string &matrixSolver);

        // matrix solver - iterative deal.II
        inline std::string getLinearSolverDealIIMethod() const {
             return iterLinearSolverDealIIMethodToStringKey((IterSolverDealII) m_fieldInfo->value(FieldInfo::LinearSolverIterDealIIMethod).toInt()).toStdString();
        }
        void setLinearSolverDealIIMethod(const std::string &linearSolverMethod);

        inline std::string getLinearSolverDealIIPreconditioner() const {
            return iterLinearSolverDealIIPreconditionerToStringKey((PreconditionerDealII) m_fieldInfo->value(FieldInfo::LinearSolverIterDealIIPreconditioner).toInt()).toStdString();
        }
        void setLinearSolverDealIIPreconditioner(const std::string &linearSolverPreconditioner);

        // matrix solver - iterative PARALUTION
        inline std::string getLinearSolverPARALUTIONMethod() const {
             return iterLinearSolverPARALUTIONMethodToStringKey((IterSolverPARALUTION) m_fieldInfo->value(FieldInfo::LinearSolverIterPARALUTIONMethod).toInt()).toStdString();
        }
        void setLinearSolverPARALUTIONMethod(const std::string &linearSolverMethod);

        inline std::string getLinearSolverPARALUTIONPreconditioner() const {
            return iterLinearSolverPARALUTIONPreconditionerToStringKey((PreconditionerPARALUTION) m_fieldInfo->value(FieldInfo::LinearSolverIterPARALUTIONPreconditioner).toInt()).toStdString();
        }
        void setLinearSolverPARALUTIONPreconditioner(const std::string &linearSolverPreconditioner);

        // number of refinements
        inline int getNumberOfRefinements() const { return m_fieldInfo->value(FieldInfo::SpaceNumberOfRefinements).toInt(); }
        void setNumberOfRefinements(int numberOfRefinements);

        // polynomial order
        inline int getPolynomialOrder() const { return m_fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt(); }
        void setPolynomialOrder(int polynomialOrder);

        // adaptivity estimator
        inline std::string getAdaptivityEstimator() const { return adaptivityEstimatorToStringKey((AdaptivityEstimator) m_fieldInfo->value(FieldInfo::AdaptivityEstimator).toInt()).toStdString(); }
        void setAdaptivityEstimator(const std::string &adaptivityEstimator);

        // initial condition
        inline double getInitialCondition() const { return m_fieldInfo->value(FieldInfo::TransientInitialCondition).toDouble(); }
        void setInitialCondition(double initialCondition);

        // time steps skip
        inline int getTimeSkip() const { return m_fieldInfo->value(FieldInfo::TransientTimeSkip).toInt(); }
        void setTimeSkip(double timeSkip);

        // boundaries
        void addBoundary(const std::string &name, const std::string &type,
                         const map<std::string, double> &parameters,
                         const map<std::string, std::string > &expressions);
        void modifyBoundary(const std::string &name, const std::string &type,
                            const map<std::string, double> &parameters,
                            const map<std::string, std::string> &expressions);
        void removeBoundary(const std::string &name);

        // materials
        void addMaterial(const std::string &name, const map<std::string, double> &parameters,
                         const map<std::string, std::string> &expressions,
                         const map<std::string, vector<double> > &nonlin_x,
                         const map<std::string, vector<double> > &nonlin_y,
                         const map<std::string, map<std::string, std::string> > &settings_map);
        void modifyMaterial(const std::string &name, const map<std::string, double> &parameters,
                            const map<std::string, std::string> &expressions,
                            const map<std::string, vector<double> > &nonlin_x,
                            const map<std::string, vector<double> > &nonlin_y,
                            const map<std::string, map<std::string, std::string> > &settings_map);
        void removeMaterial(const std::string &name);

        // local values, integrals
        void localValues(double x, double y, int timeStep, int adaptivityStep,
                         map<std::string, double> &results) const;
        void surfaceIntegrals(const vector<int> &edges, int timeStep, int adaptivityStep,
                              map<std::string, double> &results) const;
        void volumeIntegrals(const vector<int> &labels, int timeStep, int adaptivityStep,
                             map<std::string, double> &results) const;

        // mesh info
        void initialMeshInfo(map<std::string, int> &info) const;
        void solutionMeshInfo(int timeStep, int adaptivityStep, map<std::string, int> &info) const;

        // solver info
        void solverInfo(int timeStep, int adaptivityStep,
                        vector<double> &solutionsChange, vector<double> &residual,
                        vector<double> &dampingCoeff, int &jacobianCalculations) const;

        // adaptivity info
        void adaptivityInfo(int timeStep, vector<double> &error, vector<int> &dofs) const;

        // matrix and RHS
        std::string filenameMatrix(int timeStep, int adaptivityStep) const;
        std::string filenameRHS(int timeStep, int adaptivityStep) const;

private:
    FieldInfo *m_fieldInfo;

    int getTimeStep(int timeStep) const;
    int getAdaptivityStep(int adaptivityStep, int timeStep) const;
};

#endif // PYTHONLABFIELD_H
