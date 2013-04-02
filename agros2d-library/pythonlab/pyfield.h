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

#ifndef PYTHONLABFIELD_H
#define PYTHONLABFIELD_H

#include "util.h"
#include "util/global.h"

#include "hermes2d/field.h"
#include "hermes2d/problem.h"

// field
class PyField
{
    public:
        PyField(std::string fieldId);
        ~PyField() {}

        // field id
        inline const std::string& fieldId() { return m_fieldInfo->fieldId().toStdString(); }

        // analysis type
        inline const std::string& getAnalysisType() { return analysisTypeToStringKey(m_fieldInfo->analysisType()).toStdString(); }
        void setAnalysisType(const std::string &analysisType);

        // number of refinements
        inline int getNumberOfRefinements() { return m_fieldInfo->numberOfRefinements(); }
        void setNumberOfRefinements(int numberOfRefinements);

        // polynomial order
        inline int getPolynomialOrder() { return m_fieldInfo->polynomialOrder(); }
        void setPolynomialOrder(int polynomialOrder);

        // linearity type
        inline const std::string& getLinearityType() { return linearityTypeToStringKey(m_fieldInfo->linearityType()).toStdString(); }
        void setLinearityType(const std::string &linearityType);

        // nonlinear tolerance
        inline double getNonlinearTolerance() { return m_fieldInfo->nonlinearTolerance(); }
        void setNonlinearTolerance(double nonlinearTolerance);

        // nonlinear steps
        inline int getNonlinearSteps() { return m_fieldInfo->nonlinearSteps(); }
        void setNonlinearSteps(int nonlinearSteps);

        // damping coefficient
        inline double getNewtonDampingCoeff() { return m_fieldInfo->newtonDampingCoeff(); }
        void setNewtonDampingCoeff(double dampingCoeff);

        // automatic damping
        inline bool getNewtonAutomaticDamping() { return m_fieldInfo->newtonAutomaticDamping(); }
        void setNewtonAutomaticDamping(bool automaticDamping);

        // automatic damping coefficient
        inline double getNewtonAutomaticDampingCoeff() { return m_fieldInfo->newtonAutomaticDampingCoeff(); }
        void setNewtonAutomaticDampingCoeff(double dampingCoeff);

        // steps to increase damping coeff
        inline int getNewtonDampingNumberToIncrease() { return m_fieldInfo->newtonDampingNumberToIncrease(); }
        void setNewtonDampingNumberToIncrease(int dampingNumberToIncrease);

        // picard anderson acceleration
        inline bool getPicardAndersonAcceleration() { return m_fieldInfo->picardAndersonAcceleration(); }
        void setPicardAndersonAcceleration(bool acceleration);

        // picard anderson beta
        inline double getPicardAndersonBeta() { return m_fieldInfo->picardAndersonBeta(); }
        void setPicardAndersonBeta(double beta);

        // picard anderson number of last vectors
        inline int getPicardAndersonNumberOfLastVectors() { return m_fieldInfo->picardAndersonNumberOfLastVectors(); }
        void setPicardAndersonNumberOfLastVectors(int number);

        // adaptivity type
        inline const std::string& getAdaptivityType() { return adaptivityTypeToStringKey(m_fieldInfo->adaptivityType()).toStdString(); }
        void setAdaptivityType(const std::string &adaptivityType);

        // adaptivity tolerance
        inline double getAdaptivityTolerance() { return m_fieldInfo->adaptivityTolerance(); }
        void setAdaptivityTolerance(double adaptivityTolerance);

        // adaptivity steps
        inline int getAdaptivitySteps() { return m_fieldInfo->adaptivitySteps(); }
        void setAdaptivitySteps(int adaptivitySteps);

        // adaptivity back steps
        inline int getAdaptivityBackSteps() { return m_fieldInfo->adaptivityBackSteps(); }
        void setAdaptivityBackSteps(int adaptivityBackSteps);

        //adaptivity redone each
        inline int getAdaptivityRedoneEach() { return m_fieldInfo->adaptivityRedoneEach(); }
        void setAdaptivityRedoneEach(int adaptivityRedoneEach);

        // initial condition
        inline double getInitialCondition() { return m_fieldInfo->initialCondition(); }
        void setInitialCondition(double initialCondition);

        // time steps skip
        inline int getTimeSkip() { return m_fieldInfo->timeSkip(); }
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
                         const map<std::string, vector<double> > &nonlin_y);
        void modifyMaterial(const std::string &name, const map<std::string, double> &parameters,
                            const map<std::string, std::string> &expressions,
                            const map<std::string, vector<double> > &nonlin_x,
                            const map<std::string, vector<double> > &nonlin_y);
        void removeMaterial(const std::string &name);

        // local values, integrals
        void localValues(double x, double y, int timeStep, int adaptivityStep,
                         const std::string &solutionType, map<std::string, double> &results);
        void surfaceIntegrals(const vector<int> &edges, int timeStep, int adaptivityStep,
                              const std::string &solutionType, map<std::string, double> &results);
        void volumeIntegrals(const vector<int> &labels, int timeStep, int adaptivityStep,
                             const std::string &solutionType, map<std::string, double> &results);

        // mesh info
        void initialMeshInfo(map<std::string, int> &info);
        void solutionMeshInfo(int timeStep, int adaptivityStep, const std::string &solutionType, map<std::string, int> &info);

        // adaptivity info
        void adaptivityInfo(int timeStep, const std::string &solutionType, vector<double> &error, vector<int> &dofs);

private:
    FieldInfo *m_fieldInfo;

    SolutionMode getSolutionMode(QString solutionType);
    int getTimeStep(int timeStep, SolutionMode solutionMode);
    int getAdaptivityStep(int adaptivityStep, int timeStep, SolutionMode solutionMode);
};

#endif // PYTHONLABFIELD_H
