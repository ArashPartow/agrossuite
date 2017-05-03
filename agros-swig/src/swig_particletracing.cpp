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

#include "swig_particletracing.h"

#include "particle/particle_tracing.h"
#include "particle/particle_forces.h"

void SwigParticleTracing::setComputation(SwigComputation *computation)
{
    m_computation = computation->computation();
}

void SwigParticleTracing::solve(const vector<vector<double> > &initialPositionsVector, const vector<vector<double> > &initialVelocitiesVector,
                              const vector<double> &particleChargesVector, const vector<double> &particleMassesVector)
{
    if (!m_computation->isSolved())
        throw invalid_argument(QObject::tr("Problem is not solved.").toStdString());

    int numberOfParticles = m_computation->setting()->value(PostprocessorSetting::ParticleNumberOfParticles).toInt();

    // initial position
    QList<Point3> initialPositions;
    if (initialPositions.empty())
    {
        Point3 initialPosition(m_computation->setting()->value(PostprocessorSetting::ParticleStartX).toDouble(),
                               m_computation->setting()->value(PostprocessorSetting::ParticleStartY).toDouble(), 0.0);

        for (int i = 0; i < numberOfParticles; i++)
            initialPositions.append(initialPosition);
    }
    else
    {
        if (numberOfParticles != initialPositions.size())
            throw invalid_argument(QObject::tr("Number of initial positions is not equal to number of particles.").toStdString());

        for (int i = 0; i < initialPositions.size(); i++)
            initialPositions.append(Point3(initialPositionsVector.at(i).at(0), initialPositionsVector.at(i).at(1), 0.0));
    }

    // initial velocity
    QList<Point3> initialVelocities;
    if (initialVelocities.empty())
    {
        Point3 initialVelocity(m_computation->setting()->value(PostprocessorSetting::ParticleStartVelocityX).toDouble(),
                               m_computation->setting()->value(PostprocessorSetting::ParticleStartVelocityY).toDouble(), 0.0);

        for (int i = 0; i < numberOfParticles; i++)
            initialVelocities.append(initialVelocity);
    }
    else
    {
        if (numberOfParticles != initialVelocities.size())
            throw invalid_argument(QObject::tr("Number of initial velocities is not equal to number of particles.").toStdString());

        for (int i = 0; i < initialVelocities.size(); i++)
            initialVelocities.append(Point3(initialVelocitiesVector.at(i).at(0), initialVelocitiesVector.at(i).at(1), 0.0));
    }

    // particle charges
    QList<double> particleCharges;
    if (particleCharges.empty())
    {
        for (int i = 0; i < numberOfParticles; i++)
            particleCharges.append(m_computation->setting()->value(PostprocessorSetting::ParticleConstant).toDouble());
    }
    else
    {
        if (numberOfParticles != particleCharges.size())
            throw invalid_argument(QObject::tr("Number of particle charges is not equal to number of particles.").toStdString());

        particleCharges = QList<double>::fromVector(QVector<double>::fromStdVector(particleChargesVector));
    }

    // particle masses
    QList<double> particleMasses;
    if (particleMasses.empty())
    {
        for (int i = 0; i < numberOfParticles; i++)
            particleMasses.append(m_computation->setting()->value(PostprocessorSetting::ParticleMass).toDouble());
    }
    else
    {
        if (numberOfParticles != particleMasses.size())
            throw invalid_argument(QObject::tr("Number of partical masses is not equal to number of particles.").toStdString());

        particleMasses = QList<double>::fromVector(QVector<double>::fromStdVector(particleMassesVector));
    }

    ParticleTracing particleTracing(m_computation.data(), particleMasses);
    ParticleTracingForceCustom forceCustom(&particleTracing);
    ParticleTracingForceDrag forceDrag(&particleTracing);
    ParticleTracingForceField forceField(&particleTracing, particleCharges);
    ParticleTracingForceFieldP2P forceFieldP2P(&particleTracing, particleCharges, particleMasses);

    particleTracing.computeTrajectoryParticles(initialPositions, initialVelocities);

    m_positions = particleTracing.positions();
    m_velocities = particleTracing.velocities();
    m_times = particleTracing.times();
}

void SwigParticleTracing::positions(vector<vector<double> > &x,
                                  vector<vector<double> > &y,
                                  vector<vector<double> > &z) const
{
    for (int i = 0; i < m_positions.length(); i++)
    {
        vector<double> xi, yi, zi;
        for (int j = 0; j < m_positions.at(i).length(); j++)
        {
            xi.push_back(m_positions.at(i).at(j).x);
            yi.push_back(m_positions.at(i).at(j).y);
            zi.push_back(m_positions.at(i).at(j).z);
        }

        x.push_back(xi);
        y.push_back(yi);
        z.push_back(zi);
    }
}

void SwigParticleTracing::velocities(vector<vector<double> > &vx,
                                   vector<vector<double> > &vy,
                                   vector<vector<double> > &vz) const
{
    for (int i = 0; i < m_velocities.length(); i++)
    {
        vector<double> vxi, vyi, vzi;
        for (int j = 0; j < m_velocities.at(i).length(); j++)
        {
            vxi.push_back(m_velocities.at(i).at(j).x);
            vyi.push_back(m_velocities.at(i).at(j).y);
            vzi.push_back(m_velocities.at(i).at(j).z);
        }

        vx.push_back(vxi);
        vy.push_back(vyi);
        vz.push_back(vzi);
    }
}

void SwigParticleTracing::times(vector<vector<double> > &t) const
{
    if (m_times.isEmpty())
        throw logic_error(QObject::tr("Trajectories of particles are not solved.").toStdString());

    for (int i = 0; i < m_times.length(); i++)
        t.push_back(m_times.at(i).toVector().toStdVector());
}

void SwigParticleTracing::getInitialPosition(vector<double> &position) const
{
    position.push_back(m_computation->setting()->value(PostprocessorSetting::ParticleStartX).toDouble());
    position.push_back(m_computation->setting()->value(PostprocessorSetting::ParticleStartY).toDouble());
}

void SwigParticleTracing::setInitialPosition(const vector<double> &position)
{
    RectPoint rect = m_computation->scene()->boundingBox();

    double x = position[0];
    double y = position[1];

    if (x < rect.start.x || x > rect.end.x)
        throw out_of_range(QObject::tr("The x coordinate is out of range.").toStdString());
    if (y < rect.start.y || y > rect.end.y)
        throw out_of_range(QObject::tr("The y coordinate is out of range.").toStdString());

    m_computation->setting()->setValue(PostprocessorSetting::ParticleStartX, x);
    m_computation->setting()->setValue(PostprocessorSetting::ParticleStartY, y);
}

void SwigParticleTracing::getInitialVelocity(vector<double> &velocity) const
{
    velocity.push_back(m_computation->setting()->value(PostprocessorSetting::ParticleStartVelocityX).toDouble());
    velocity.push_back(m_computation->setting()->value(PostprocessorSetting::ParticleStartVelocityY).toDouble());
}

void SwigParticleTracing::setInitialVelocity(const vector<double> &velocity)
{
    m_computation->setting()->setValue(PostprocessorSetting::ParticleStartVelocityX, velocity[0]);
    m_computation->setting()->setValue(PostprocessorSetting::ParticleStartVelocityY, velocity[1]);
}

void SwigParticleTracing::setNumberOfParticles(int particles)
{
    if (particles < 1 || particles > 200)
        throw out_of_range(QObject::tr("Number of particles is out of range (1 - 200).").toStdString());

    m_computation->setting()->setValue(PostprocessorSetting::ParticleNumberOfParticles, particles);
}

void SwigParticleTracing::setStartingRadius(double radius)
{
    if (radius < 0.0)
        throw out_of_range(QObject::tr("Particles dispersion cannot be negative.").toStdString());

    m_computation->setting()->setValue(PostprocessorSetting::ParticleStartingRadius, radius);
}

void SwigParticleTracing::setParticleMass(double mass)
{
    if (mass < 0.0)
        throw out_of_range(QObject::tr("Particle mass cannot be negative.").toStdString());

    m_computation->setting()->setValue(PostprocessorSetting::ParticleMass, mass);
}

void SwigParticleTracing::setParticleCharge(double charge)
{
    m_computation->setting()->setValue(PostprocessorSetting::ParticleConstant, charge);
}

void SwigParticleTracing::setDragForceDensity(double density)
{
    if (density < 0.0)
        throw out_of_range(QObject::tr("Drag force density cannot be negative.").toStdString());

    m_computation->setting()->setValue(PostprocessorSetting::ParticleDragDensity, density);
}

void SwigParticleTracing::setDragForceReferenceArea(double area)
{
    if (area < 0.0)
        throw out_of_range(QObject::tr("Drag force area cannot be negative.").toStdString());

    m_computation->setting()->setValue(PostprocessorSetting::ParticleDragReferenceArea, area);
}

void SwigParticleTracing::setDragForceCoefficient(double coeff)
{
    if (coeff < 0.0)
        throw out_of_range(QObject::tr("Drag force coefficient cannot be negative.").toStdString());

    m_computation->setting()->setValue(PostprocessorSetting::ParticleDragCoefficient, coeff);
}

void SwigParticleTracing::setCustomForce(const vector<double> &force)
{
    if (force.empty())
        throw invalid_argument(QObject::tr("Value of force is not defined.").toStdString());

    m_computation->setting()->setValue(PostprocessorSetting::ParticleCustomForceX, force[0]);
    m_computation->setting()->setValue(PostprocessorSetting::ParticleCustomForceY, force[1]);
    m_computation->setting()->setValue(PostprocessorSetting::ParticleCustomForceZ, force[2]);
}

void SwigParticleTracing::getCustomForce(vector<double> &force) const
{
    force.push_back(m_computation->setting()->value(PostprocessorSetting::ParticleCustomForceX).toDouble());
    force.push_back(m_computation->setting()->value(PostprocessorSetting::ParticleCustomForceY).toDouble());
    force.push_back(m_computation->setting()->value(PostprocessorSetting::ParticleCustomForceZ).toDouble());
}

void SwigParticleTracing::setButcherTableType(const std::string &tableType)
{
    if (!butcherTableTypeStringKeys().contains(QString::fromStdString(tableType)))
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(butcherTableTypeStringKeys())).toStdString());

    m_computation->setting()->setValue(PostprocessorSetting::ParticleButcherTableType, butcherTableTypeFromStringKey(QString::fromStdString(tableType)));
}

void SwigParticleTracing::setMaximumRelativeError(double error)
{
    if (error < 0.0)
        throw out_of_range(QObject::tr("Maximum relative error cannot be negative.").toStdString());

    m_computation->setting()->setValue(PostprocessorSetting::ParticleMaximumRelativeError, error);
}

void SwigParticleTracing::setMaximumNumberOfSteps(int steps)
{
    if (steps < 10 || steps > 1e5)
        throw out_of_range(QObject::tr("Maximum number of steps is out of range (10 - 1e5).").toStdString());

    m_computation->setting()->setValue(PostprocessorSetting::ParticleMaximumNumberOfSteps, steps);
}

void SwigParticleTracing::setMaximumStep(int step)
{
    if (step < 0.0)
        throw out_of_range(QObject::tr("Maximum step cannot be negative.").toStdString());

    m_computation->setting()->setValue(PostprocessorSetting::ParticleMaximumStep, step);
}

void SwigParticleTracing::setCoefficientOfRestitution(double coeff)
{
    if (coeff < 0.0 || coeff > 1.0)
        throw out_of_range(QObject::tr("Coefficient of restitution must be between 0 (collide inelastically) and 1 (collide elastically).").toStdString());

    m_computation->setting()->setValue(PostprocessorSetting::ParticleCoefficientOfRestitution, coeff);
}

void SwigParticleTracing::setNumShowParticlesAxi(int particles)
{
    if (particles < 1 || particles > 500)
        throw out_of_range(QObject::tr("Number of multiple show particles is out of range (1 - 500).").toStdString());

    m_computation->setting()->setValue(PostprocessorSetting::ParticleNumShowParticlesAxi, particles);
}
