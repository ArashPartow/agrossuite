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

#include "solutionstore.h"

#include "util/global.h"
#include "util/constants.h"

#include "logview.h"
#include "field.h"
#include "scene.h"
#include "problem.h"
#include "problem_config.h"

#include <deal.II/fe/fe_q.h>

#include <boost/config.hpp>
#include <boost/archive/tmpdir.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include "../../resources_source/classes/structure_xml.h"

void SolutionStore::printDebugCacheStatus()
{
    assert(m_multiSolutionCacheIDOrder.size() == m_multiSolutionDealCache.keys().size());
    qDebug() << "solution store cache status:";
    foreach(FieldSolutionID fsid, m_multiSolutionCacheIDOrder)
    {
        assert(m_multiSolutionDealCache.keys().contains(fsid));
        qDebug() << fsid.toString();
    }
}

SolutionStore::~SolutionStore()
{
    clearAll();
}

QString SolutionStore::baseStoreFileName(FieldSolutionID solutionID) const
{
    QString fn = QString("%1/%2").
            arg(cacheProblemDir()).
            arg(solutionID.toString());

    return fn;
}

void SolutionStore::clearAll()
{
    // fast remove of all files
    foreach (FieldSolutionID sid, m_multiSolutions)
        removeSolution(sid, false);

    // remove runtime
    QString fn = QString("%1/runtime.xml").arg(cacheProblemDir());
    if (QFile::exists(fn))
        QFile::remove(fn);

    assert(m_multiSolutions.isEmpty());
    assert(m_multiSolutionRunTimeDetails.isEmpty());
    assert(m_multiSolutionDealCache.isEmpty());
}

MultiArray SolutionStore::multiArray(FieldSolutionID solutionID)
{
    if(solutionID.solutionMode == SolutionMode_Finer)
    {
        solutionID.solutionMode = SolutionMode_Reference;
        if(!m_multiSolutions.contains(solutionID))
            solutionID.solutionMode = SolutionMode_Normal;
    }

    assert(m_multiSolutions.contains(solutionID));

    if (!m_multiSolutionDealCache.contains(solutionID))
    {
        // load from stream
        QString baseFN = baseStoreFileName(solutionID);

        // triangulation
        dealii::Triangulation<2> *triangulation = new dealii::Triangulation<2>;
        QString fnMesh = QString("%1.msh").arg(baseFN);
        std::ifstream ifsMesh(fnMesh.toStdString());
        boost::archive::binary_iarchive sbiMesh(ifsMesh);
        triangulation->load(sbiMesh, 0);

        dealii::DoFHandler<2> *doFHandler = new dealii::DoFHandler<2>(*triangulation);

        // fe system
        dealii::FESystem<2> *fe = new dealii::FESystem<2>(dealii::FE_Q<2>(solutionID.group->value(FieldInfo::SpacePolynomialOrder).toInt()), solutionID.group->numberOfSolutions());
        doFHandler->distribute_dofs(*fe);

        QString fnDoF = QString("%1.dof").arg(baseFN);
        std::ifstream ifsDoF(fnDoF.toStdString());
        boost::archive::binary_iarchive sbiDoF(ifsDoF);
        doFHandler->load(sbiDoF, 0);

        // solution vector
        dealii::Vector<double> *solution = new dealii::Vector<double>();

        QString fnSol = QString("%1.sol").arg(baseFN);
        std::ifstream ifsSol(fnSol.toStdString());
        boost::archive::binary_iarchive sbiSol(ifsSol);
        solution->load(sbiSol, 0);

        // new multisolution
        MultiArray msa(doFHandler, solution);

        insertMultiSolutionToCache(solutionID, msa);

        return msa;
    }
    else
    {
        return m_multiSolutionDealCache[solutionID];
    }
}

bool SolutionStore::contains(FieldSolutionID solutionID) const
{
    return m_multiSolutions.contains(solutionID);
}

void SolutionStore::addSolution(FieldSolutionID solutionID, MultiArray multiSolution, SolutionRunTimeDetails runTime)
{
    // qDebug() << "saving solution " << solutionID;
    // assert(!m_multiSolutions.contains(solutionID));
    assert(solutionID.timeStep >= 0);
    assert(solutionID.adaptivityStep >= 0);
    // save soloution

    SolutionRunTimeDetails::FileName fileNames;

    QString baseFN = baseStoreFileName(solutionID);

    QString fnMesh = QString("%1.msh").arg(baseFN);
    fileNames.setMeshFileName(QFileInfo(fnMesh).fileName());
    std::ofstream ofsMesh(fnMesh.toStdString());
    boost::archive::binary_oarchive sbMesh(ofsMesh);
    multiSolution.doFHandler()->get_tria().save(sbMesh, 0);

    QString fnDoF = QString("%1.dof").arg(baseFN);
    fileNames.setDoFFileName(QFileInfo(fnDoF).fileName());
    std::ofstream ofsDoF(fnDoF.toStdString());
    boost::archive::binary_oarchive sbDoF(ofsDoF);
    multiSolution.doFHandler()->save(sbDoF, 0);

    QString fnSol = QString("%1.sol").arg(baseFN);
    fileNames.setSolutionFileName(QFileInfo(fnSol).fileName());
    std::ofstream ofsSol(fnSol.toStdString());
    boost::archive::binary_oarchive sbSol(ofsSol);
    multiSolution.solution()->save(sbSol, 0);

    runTime.setFileNames(fileNames);

    // append multisolution
    m_multiSolutions.append(solutionID);

    // append properties
    m_multiSolutionRunTimeDetails.insert(solutionID, runTime);

    // insert to the cache
    insertMultiSolutionToCache(solutionID, multiSolution);

    //printDebugCacheStatus();

    // save run time details to the file
    saveRunTimeDetails();

    // save to the memory info (for debug purposes)
    // m_memoryInfos[solutionID] = tr1::shared_ptr<MemoryInfo>(new MemoryInfo(multiSolution));
}

void SolutionStore::removeSolution(FieldSolutionID solutionID, bool saveRunTime)
{
    assert(m_multiSolutions.contains(solutionID));

    // remove from list
    m_multiSolutions.removeOne(solutionID);
    // remove properties
    m_multiSolutionRunTimeDetails.remove(solutionID);
    // remove from cache
    if (m_multiSolutionDealCache.contains(solutionID))
    {
        // free ma
        m_multiSolutionDealCache[solutionID].clear();
        m_multiSolutionDealCache.remove(solutionID);
        m_multiSolutionCacheIDOrder.removeOne(solutionID);
    }

    // remove old files
    QFileInfo info(Agros2D::problem()->config()->fileName());
    if (info.exists())
    {
        QString fn = baseStoreFileName(solutionID);

        for (int solutionIndex = 0; solutionIndex < solutionID.group->numberOfSolutions(); solutionIndex++)
        {
            QString fnMesh = QString("%1_%2.msh").arg(fn).arg(solutionIndex);
            if (QFile::exists(fnMesh))
                QFile::remove(fnMesh);

            QString fnSpace = QString("%1_%2.spc").arg(fn).arg(solutionIndex);
            if (QFile::exists(fnSpace))
                QFile::remove(fnSpace);

            QString fnSolution = QString("%1_%2.sln").arg(fn).arg(solutionIndex);
            if (QFile::exists(fnSolution))
                QFile::remove(fnSolution);
        }
    }

    // save structure to the file
    if (saveRunTime)
        saveRunTimeDetails();
}

void SolutionStore::removeTimeStep(int timeStep)
{
    foreach (FieldSolutionID sid, m_multiSolutions)
    {
        if (sid.timeStep == timeStep)
            removeSolution(sid);
    }

}

int SolutionStore::lastTimeStep(const FieldInfo *fieldInfo, SolutionMode solutionType) const
{
    int timeStep = NOT_FOUND_SO_FAR;
    foreach (FieldSolutionID sid, m_multiSolutions)
    {
        if((sid.group == fieldInfo) && (sid.solutionMode == solutionType) && (sid.timeStep > timeStep))
            timeStep = sid.timeStep;
    }

    return timeStep;
}

/*
MultiArray<double> SolutionStore::multiSolutionPreviousCalculatedTS(BlockSolutionID solutionID)
{
    MultiArray<double> ma;
    foreach(FieldBlock *field, solutionID.group->fields())
    {
        FieldSolutionID fieldSolutionID = solutionID.fieldSolutionID(field->fieldInfo());
        fieldSolutionID.timeStep = nearestTimeStep(field->fieldInfo(), solutionID.timeStep - 1);

        MultiArray<double> maGroup = multiArray(fieldSolutionID);
        ma.append(maGroup.spaces(), maGroup.solutions());
    }

    return ma;
}
*/

int SolutionStore::nthCalculatedTimeStep(const FieldInfo *fieldInfo, int n) const
{
    int count = 0;
    for(int step = 0; step <= lastTimeStep(fieldInfo, SolutionMode_Normal); step++)
    {
        if(this->contains(FieldSolutionID(fieldInfo, step, 0, SolutionMode_Normal)))
            count++;

        // n is counted from zero
        if(count == n + 1)
            return step;
    }

    assert(0);
}


int SolutionStore::nearestTimeStep(const FieldInfo *fieldInfo, int timeStep) const
{
    int ts = timeStep;
    while (!this->contains(FieldSolutionID(fieldInfo, ts, 0, SolutionMode_Normal)))
    {
        ts--;
        if (ts <= 0)
            return 0;
    }
    return ts;
}

double SolutionStore::lastTime(const FieldInfo *fieldInfo)
{
    int timeStep = lastTimeStep(fieldInfo, SolutionMode_Normal);
    double time = NOT_FOUND_SO_FAR;

    foreach (FieldSolutionID id, m_multiSolutions)
    {
        if ((id.group == fieldInfo) && (id.timeStep == timeStep))
        {
            if (time == NOT_FOUND_SO_FAR)
                time = Agros2D::problem()->timeStepToTotalTime(id.timeStep);
            else
                assert(time == Agros2D::problem()->timeStepToTotalTime(id.timeStep));
        }
    }
    assert(time != NOT_FOUND_SO_FAR);
    return time;
}

int SolutionStore::lastAdaptiveStep(const FieldInfo *fieldInfo, SolutionMode solutionType, int timeStep) const
{
    if (timeStep == -1)
        timeStep = lastTimeStep(fieldInfo, solutionType);

    int adaptiveStep = NOT_FOUND_SO_FAR;
    foreach (FieldSolutionID sid, m_multiSolutions)
    {
        if ((sid.group == fieldInfo) && (sid.solutionMode == solutionType) && (sid.timeStep == timeStep) && (sid.adaptivityStep > adaptiveStep))
            adaptiveStep = sid.adaptivityStep;
    }

    return adaptiveStep;
}

FieldSolutionID SolutionStore::lastTimeAndAdaptiveSolution(const FieldInfo *fieldInfo, SolutionMode solutionType)
{
    FieldSolutionID solutionID;
    if (solutionType == SolutionMode_Finer) {
        FieldSolutionID solutionIDNormal = lastTimeAndAdaptiveSolution(fieldInfo, SolutionMode_Normal);
        FieldSolutionID solutionIDReference = lastTimeAndAdaptiveSolution(fieldInfo, SolutionMode_Reference);
        if((solutionIDNormal.timeStep > solutionIDReference.timeStep) ||
                (solutionIDNormal.adaptivityStep > solutionIDReference.adaptivityStep))
        {
            solutionID = solutionIDNormal;
        }
        else
        {
            solutionID = solutionIDReference;
        }
    }
    else
    {
        solutionID.group = fieldInfo;
        solutionID.adaptivityStep = lastAdaptiveStep(fieldInfo, solutionType);
        solutionID.timeStep = lastTimeStep(fieldInfo, solutionType);
        solutionID.solutionMode = solutionType;
    }

    return solutionID;
}

QList<double> SolutionStore::timeLevels(const FieldInfo *fieldInfo) const
{
    QList<double> list;

    foreach(FieldSolutionID fsid, m_multiSolutions)
    {
        if (fsid.group == fieldInfo)
        {
            double time = Agros2D::problem()->timeStepToTotalTime(fsid.timeStep);
            if (!list.contains(time))
                list.push_back(time);
        }
    }

    return list;
}

int SolutionStore::timeLevelIndex(const FieldInfo *fieldInfo, double time)
{
    int level = -1;
    QList<double> levels = timeLevels(fieldInfo);
    if(levels.isEmpty())
        return 0;

    foreach(double timeLevel, levels)
    {
        if(timeLevel <= time)
            level++;
    }
    assert(level >= 0);
    return level;
}

double SolutionStore::timeLevel(const FieldInfo *fieldInfo, int timeLevelIndex)
{
    QList<double> levels = timeLevels(fieldInfo);
    if (timeLevelIndex >= 0 && timeLevelIndex < levels.count())
        return levels.at(timeLevelIndex);

    assert(0);
    return 0.0;
}

void SolutionStore::insertMultiSolutionToCache(FieldSolutionID solutionID, MultiArray multiSolution)
{
    // save to stream
    std::stringstream fsMesh(std::ios::out | std::ios::in | std::ios::binary);
    boost::archive::binary_oarchive sboMesh(fsMesh);
    multiSolution.doFHandler()->get_tria().save(sboMesh, 0);

    std::stringstream fsDoF(std::ios::out | std::ios::in | std::ios::binary);
    boost::archive::binary_oarchive sboDoF(fsDoF);
    multiSolution.doFHandler()->save(sboDoF, 0);

    // std::stringstream fsSol(std::ios::out | std::ios::in | std::ios::binary);
    // boost::archive::binary_oarchive sboSol(fsSol);
    // multiSolution.solution()->save(sboSol, 0);

    // load from stream
    // triangulation
    //dealii::Triangulation<2> *triangulation = new dealii::Triangulation<2>(multiSolution.doFHandler()->get_tria());
    dealii::Triangulation<2> *triangulation = new dealii::Triangulation<2>();
    boost::archive::binary_iarchive sbiMesh(fsMesh);
    triangulation->load(sbiMesh, 0);

    dealii::DoFHandler<2> *doFHandler = new dealii::DoFHandler<2>(*triangulation);

    // fe system
    dealii::FESystem<2> *fe = new dealii::FESystem<2>(dealii::FE_Q<2>(solutionID.group->value(FieldInfo::SpacePolynomialOrder).toInt()), solutionID.group->numberOfSolutions());
    doFHandler->distribute_dofs(*fe);
    boost::archive::binary_iarchive sbiDoF(fsDoF);
    doFHandler->load(sbiDoF, 0);

    // solution vector
    dealii::Vector<double> *solution = new dealii::Vector<double>(*multiSolution.solution());
    //dealii::Vector<double> *solution = new dealii::Vector<double>();
    //boost::archive::binary_iarchive sbiSol(fsSol);
    //solution->load(sbiSol, 0);

    // new multisolution
    MultiArray multiSolutionCopy(doFHandler, solution);

    if (!m_multiSolutionDealCache.contains(solutionID))
    {
        // flush cache
        if (m_multiSolutionDealCache.count() > Agros2D::configComputer()->value(Config::Config_CacheSize).toInt())
        {
            assert(! m_multiSolutionCacheIDOrder.empty());
            FieldSolutionID idRemove = m_multiSolutionCacheIDOrder[0];
            m_multiSolutionCacheIDOrder.removeFirst();

            // free ma
            m_multiSolutionDealCache[idRemove].clear();
            m_multiSolutionDealCache.remove(idRemove);
        }

        // add solution
        m_multiSolutionDealCache.insert(solutionID, multiSolutionCopy);
        m_multiSolutionCacheIDOrder.append(solutionID);
    }
}

void SolutionStore::loadRunTimeDetails()
{
    QString fn = QString("%1/runtime.xml").arg(cacheProblemDir());

    try
    {
        std::unique_ptr<XMLStructure::structure> structure_xsd = XMLStructure::structure_(compatibleFilename(fn).toStdString(), xml_schema::flags::dont_validate);
        XMLStructure::structure *structure = structure_xsd.get();

        int time_step = 0;
        for (unsigned int i = 0; i < structure->element_data().size(); i++)
        {
            XMLStructure::element_data data = structure->element_data().at(i);

            // check field
            if (!Agros2D::problem()->hasField(QString::fromStdString(data.field_id())))
                throw AgrosException(QObject::tr("Field '%1' info mismatch.").arg(QString::fromStdString(data.field_id())));

            FieldSolutionID solutionID(Agros2D::problem()->fieldInfo(QString::fromStdString(data.field_id())),
                                       data.time_step(),
                                       data.adaptivity_step(),
                                       solutionTypeFromStringKey(QString::fromStdString(data.solution_type())));
            // append multisolution
            m_multiSolutions.append(solutionID);

            // TODO: remove "problem time step structures"
            // define transient time step
            if (data.time_step() > time_step)
            {
                // new time step
                time_step = data.time_step();

                Agros2D::problem()->defineActualTimeStepLength(data.time_step_length().get());
            }

            SolutionRunTimeDetails::FileName fileNames;
            for (int j = 0; j < data.files().file().size(); j++)
            {
                XMLStructure::file file = data.files().file().at(j);

                fileNames = SolutionRunTimeDetails::FileName(QString::fromStdString(file.mesh_filename()),
                                                             QString::fromStdString(file.dof_filename()),
                                                             QString::fromStdString(file.solution_filename()));
            }

            SolutionRunTimeDetails runTime(data.time_step_length().get(),
                                           data.adaptivity_error().get(),
                                           data.dofs().get());
            runTime.setFileNames(fileNames);

            // append run time details
            m_multiSolutionRunTimeDetails.insert(solutionID,
                                                 runTime);
        }
    }
    catch (const xml_schema::exception& e)
    {
        std::cerr << e << std::endl;
    }
}

void SolutionStore::saveRunTimeDetails()
{
    QString fn = QString("%1/runtime.xml").arg(cacheProblemDir());

    try
    {
        XMLStructure::structure structure;
        foreach (FieldSolutionID solutionID, m_multiSolutions)
        {
            SolutionRunTimeDetails str = m_multiSolutionRunTimeDetails[solutionID];

            XMLStructure::files files;
            for (int solutionIndex = 0; solutionIndex < solutionID.group->numberOfSolutions(); solutionIndex++)
            {
                files.file().push_back(XMLStructure::file(solutionIndex,
                                                          str.fileNames().meshFileName().toStdString(),
                                                          str.fileNames().doFFileName().toStdString(),
                                                          str.fileNames().solutionFileName().toStdString()));
            }

            XMLStructure::newton_residuals newton_residuals;
            for (int iteration = 0; iteration < str.newtonResidual().size(); iteration++)
            {
                newton_residuals.residual().push_back(str.newtonResidual().at(iteration));
            }

            XMLStructure::newton_damping_coefficients newton_damping_coefficients;
            for (int iteration = 0; iteration < str.nonlinearDamping().size(); iteration++)
            {
                newton_damping_coefficients.damping_coefficient().push_back(str.nonlinearDamping().at(iteration));
            }

            // solution id
            XMLStructure::element_data data(files,
                                            newton_residuals,
                                            newton_damping_coefficients,
                                            solutionID.group->fieldId().toStdString(),
                                            solutionID.timeStep,
                                            solutionID.adaptivityStep,
                                            solutionTypeToStringKey(solutionID.solutionMode).toStdString());

            // properties
            data.time_step_length().set(str.timeStepLength());
            data.adaptivity_error().set(str.adaptivityError());
            data.dofs().set(str.DOFs());
            data.jacobian_calculations().set(str.jacobianCalculations());

            structure.element_data().push_back(data);
        }

        std::string mesh_schema_location("");

        mesh_schema_location.append(QString("%1/structure_xml.xsd").arg(QFileInfo(datadir() + XSDROOT).absoluteFilePath()).toStdString());
        ::xml_schema::namespace_info namespace_info_mesh("XMLStructure", mesh_schema_location);

        ::xml_schema::namespace_infomap namespace_info_map;
        namespace_info_map.insert(std::pair<std::basic_string<char>, xml_schema::namespace_info>("structure", namespace_info_mesh));

        std::ofstream out(compatibleFilename(fn).toStdString().c_str());
        XMLStructure::structure_(out, structure, namespace_info_map);
    }
    catch (const xml_schema::exception& e)
    {
        std::cerr << e << std::endl;
    }
}

void SolutionStore::multiSolutionRunTimeDetailReplace(FieldSolutionID solutionID, SolutionRunTimeDetails runTime)
{
    assert(m_multiSolutionRunTimeDetails.contains(solutionID));
    m_multiSolutionRunTimeDetails[solutionID] = runTime;

    // save structure to the file
    saveRunTimeDetails();
}
