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

#ifndef PYTHONLABPARTICLETRACING_H
#define PYTHONLABPARTICLETRACING_H

#include "swig_problem.h"

#include "util/global.h"
#include "solver/problem.h"
#include "solver/problem_config.h"

class SwigParticleTracing
{
public:
    SwigParticleTracing() {}
    ~SwigParticleTracing() {}
    void setComputation(SwigComputation *computation);

    // initial position
    void getInitialPosition(vector<double> &position) const;
    void setInitialPosition(const vector<double> &position);

    // initial velocity
    void getInitialVelocity(vector<double> &velocity) const;
    void setInitialVelocity(const vector<double> &velocity);

    // number of particles
    inline int getNumberOfParticles() const
    {
        return m_computation->setting()->value(PostprocessorSetting::ParticleNumberOfParticles).toInt();
    }
    void setNumberOfParticles(int particles);

    // starting radius
    inline double getStartingRadius() const
    {
        return m_computation->setting()->value(PostprocessorSetting::ParticleStartingRadius).toDouble();
    }
    void setStartingRadius(double radius);

    // particle mass
    inline double getParticleMass() const
    {
        return m_computation->setting()->value(PostprocessorSetting::ParticleMass).toDouble();
    }
    void setParticleMass(double mass);

    // particle charge
    inline double getParticleCharge() const
    {
        return m_computation->setting()->value(PostprocessorSetting::ParticleConstant).toDouble();
    }
    void setParticleCharge(double charge);

    // drag force density
    inline double getDragForceDensity() const
    {
        return m_computation->setting()->value(PostprocessorSetting::ParticleDragDensity).toDouble();
    }
    void setDragForceDensity(double density);

    // drag force reference area
    inline double getDragForceReferenceArea() const
    {
        return m_computation->setting()->value(PostprocessorSetting::ParticleDragReferenceArea).toDouble();
    }
    void setDragForceReferenceArea(double area);

    // drag force coefficient
    inline double getDragForceCoefficient() const
    {
        return m_computation->setting()->value(PostprocessorSetting::ParticleDragCoefficient).toDouble();
    }
    void setDragForceCoefficient(double coeff);

    // custom force
    void getCustomForce(vector<double> &force) const;
    void setCustomForce(const vector<double> &force);

    inline bool getElectrostaticInteraction() const
    {
        return m_computation->setting()->value(PostprocessorSetting::ParticleP2PElectricForce).toBool();
    }
    void setElectrostaticInteraction(bool interaction)
    {
        m_computation->setting()->setValue(PostprocessorSetting::ParticleP2PElectricForce, interaction);
    }
    inline bool getMagneticInteraction() const
    {
        return m_computation->setting()->value(PostprocessorSetting::ParticleP2PMagneticForce).toBool();
    }
    void setMagneticInteraction(bool interaction)
    {
        m_computation->setting()->setValue(PostprocessorSetting::ParticleP2PMagneticForce, interaction);
    }

    // butcher table
    std::string getButcherTableType() const
    {
        return butcherTableTypeToStringKey((ButcherTableType) m_computation->setting()->value(PostprocessorSetting::ParticleButcherTableType).toInt()).toStdString();
    }
    void setButcherTableType(const std::string &tableType);

    // relativistic correction
    inline bool getIncludeRelativisticCorrection() const
    {
        return m_computation->setting()->value(PostprocessorSetting::ParticleIncludeRelativisticCorrection).toBool();
    }
    void setIncludeRelativisticCorrection(bool include)
    {
        m_computation->setting()->setValue(PostprocessorSetting::ParticleIncludeRelativisticCorrection, include);
    }

    // get maximum relative error
    inline double getMaximumRelativeError() const
    {
        return m_computation->setting()->value(PostprocessorSetting::ParticleMaximumRelativeError).toDouble();
    }
    void setMaximumRelativeError(double error);

    // maximum number of steps
    inline int getMaximumNumberOfSteps() const
    {
        return m_computation->setting()->value(PostprocessorSetting::ParticleMaximumNumberOfSteps).toInt();
    }
    void setMaximumNumberOfSteps(int steps);

    // minimum step
    inline int getMaximumStep() const
    {
        return m_computation->setting()->value(PostprocessorSetting::ParticleMaximumStep).toInt();
    }
    void setMaximumStep(int step);

    // reflection on different material
    inline bool getReflectOnDifferentMaterial() const
    {
        return m_computation->setting()->value(PostprocessorSetting::ParticleReflectOnDifferentMaterial).toBool();
    }
    void setReflectOnDifferentMaterial(bool reflect)
    {
        m_computation->setting()->setValue(PostprocessorSetting::ParticleReflectOnDifferentMaterial, reflect);
    }

    // reflection on boundary
    inline bool getReflectOnBoundary() const
    {
        return m_computation->setting()->value(PostprocessorSetting::ParticleReflectOnBoundary).toBool();
    }
    void setReflectOnBoundary(bool reflect)
    {
        m_computation->setting()->setValue(PostprocessorSetting::ParticleReflectOnBoundary, reflect);
    }

    // coefficient of restitution
    inline double getCoefficientOfRestitution() const
    {
        return m_computation->setting()->value(PostprocessorSetting::ParticleCoefficientOfRestitution).toDouble();
    }
    void setCoefficientOfRestitution(double coeff);

    // collor by velocity
    inline bool getColorByVelocity() const
    {
        return m_computation->setting()->value(PostprocessorSetting::ParticleColorByVelocity).toBool();
    }
    void setColorByVelocity(bool show)
    {
        m_computation->setting()->setValue(PostprocessorSetting::ParticleColorByVelocity, show);
    }

    // show points
    inline bool getShowPoints() const
    {
        return m_computation->setting()->value(PostprocessorSetting::ParticleShowPoints).toBool();
    }
    void setShowPoints(bool show)
    {
        m_computation->setting()->setValue(PostprocessorSetting::ParticleShowPoints, show);
    }

    // blended faces
    inline bool getShowBlendedFaces() const
    {
        return m_computation->setting()->value(PostprocessorSetting::ParticleShowBlendedFaces).toBool();
    }
    void setShowBlendedFaces(bool show)
    {
        m_computation->setting()->setValue(PostprocessorSetting::ParticleShowBlendedFaces, show);
    }

    // multiple show particles
    inline int getNumShowParticlesAxi() const
    {
        return m_computation->setting()->value(PostprocessorSetting::ParticleNumShowParticlesAxi).toInt();
    }
    void setNumShowParticlesAxi(int particles);

    // solve
    void solve(const vector<vector<double> > &initialPositionsVector, const vector<vector<double> > &initialVelocitiesVector,
               const vector<double> &particleChargesVector, const vector<double> &particleMassesVector);

    void positions(vector<vector<double> > &x, vector<vector<double> > &y, vector<vector<double> > &z) const;
    void velocities(vector<vector<double> > &vx, vector<vector<double> > &vy, vector<vector<double> > &vz) const;
    void times(vector<vector<double> > &t) const;

private:
    QSharedPointer<Computation> m_computation;

    // position, velocity and time
    QList<QList<Point3> > m_positions;
    QList<QList<Point3> > m_velocities;
    QList<QList<double> > m_times;
};

#endif // PYTHONLABPARTICLETRACING_H