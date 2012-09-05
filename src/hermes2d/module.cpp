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

#include "module.h"
#include "module_agros.h"
#include "field.h"
#include "block.h"
#include "problem.h"
#include "logview.h"

#include "../weakform/src/weakform_factory.h"

#include "util.h"
#include "scene.h"
#include "scenebasic.h"
#include "scenemarkerdialog.h"
#include "scenelabel.h"
#include "sceneedge.h"
#include "hermes2d/solver.h"
#include "hermes2d/coupling.h"
#include "hermes2d/solutionstore.h"
#include "hermes2d/post_values.h"
#include "hermes2d/bdf2.h"

#include "mesh/mesh_reader_h2d.h"

#include "newton_solver.h"
#include "picard_solver.h"

//#include "solver.h"

#include "../../resources_source/classes/module_xml.h"

double actualTime;

QMap<QString, QString> availableModules()
{
    static QMap<QString, QString> modules;
    
    // read modules
    if (modules.size() == 0)
    {
        QDir dir(datadir() + MODULEROOT);

        QStringList filter;
        filter << "*.xml";
        QStringList list = dir.entryList(filter);

        foreach (QString filename, list)
        {
            std::auto_ptr<XMLModule::module> module_xsd = XMLModule::module_((datadir().toStdString() + MODULEROOT.toStdString() + "/" + filename.toStdString()).c_str());
            XMLModule::module *mod = module_xsd.get();

            // module name
            modules[filename.left(filename.size() - 4)] = QString::fromStdString(mod->general().name());
        }
    }
    
    // custom module
    // modules["custom"] = "Custom field";
    
    return modules;
}

QMap<AnalysisType, QString> availableAnalyses(const QString &fieldId)
{
    QMap<AnalysisType, QString> analyses;
    
    std::auto_ptr<XMLModule::module> module_xsd = XMLModule::module_((datadir().toStdString() + MODULEROOT.toStdString() + "/" + fieldId.toStdString() + ".xml").c_str());
    XMLModule::module *mod = module_xsd.get();
    
    for (int i = 0; i < mod->general().analyses().analysis().size(); i++)
    {
        XMLModule::analysis an = mod->general().analyses().analysis().at(i);

        analyses[analysisTypeFromStringKey(QString::fromStdString(an.id()))] = QString::fromStdString(an.name());
    }
    
    return analyses;
}

template <typename Scalar>
WeakFormAgros<Scalar>::WeakFormAgros(Block* block) :
    Hermes::Hermes2D::WeakForm<Scalar>(block->numSolutions()), m_block(block)
{
}

template <typename Scalar>
Hermes::Hermes2D::Form<Scalar> *factoryForm(WeakFormKind type, const QString &problemId,
                                            const QString &area, FormInfo *form,
                                            Marker* marker, Material* markerSecond, int offsetI, int offsetJ)
{
    //cout <<"ID: " << problemId.toStdString() << ", " << type << ", form position (" << form->i << ", " << form->j << ")"<< ", offset (" << offsetI << ", " << offsetJ << ")" << endl;
    if(type == WeakForm_MatVol)
        return factoryMatrixFormVol<Scalar>(problemId.toStdString(), form->i, form->j, area.toStdString(), form->sym, (SceneMaterial*) marker, markerSecond, offsetI, offsetJ);
    else if(type == WeakForm_MatSurf)
        return factoryMatrixFormSurf<Scalar>(problemId.toStdString(), form->i, form->j, area.toStdString(), (SceneBoundary*) marker, offsetI, offsetJ);
    else if(type == WeakForm_VecVol)
        return factoryVectorFormVol<Scalar>(problemId.toStdString(), form->i, form->j, area.toStdString(), (SceneMaterial*) marker, markerSecond, offsetI, offsetJ);
    else if(type == WeakForm_VecSurf)
        return factoryVectorFormSurf<Scalar>(problemId.toStdString(), form->i, form->j, area.toStdString(), (SceneBoundary*) marker, offsetI, offsetJ);
    else
        assert(0);
}

template <typename Scalar>
void WeakFormAgros<Scalar>::addForm(WeakFormKind type, Hermes::Hermes2D::Form<Scalar> *form)
{
    // Util::log()->printDebug("WeakFormAgros", QString("addForm: type: %1, i: %2, area: %3").
    //                         arg(weakFormString(type)).
    //                         arg(form->i).
    //                         arg(QString::fromStdString(form->areas.at(0))));

    if(type == WeakForm_MatVol)
        add_matrix_form((Hermes::Hermes2D::MatrixFormVol<Scalar>*) form);
    else if(type == WeakForm_MatSurf)
        add_matrix_form_surf((Hermes::Hermes2D::MatrixFormSurf<Scalar>*) form);
    else if(type == WeakForm_VecVol)
        add_vector_form((Hermes::Hermes2D::VectorFormVol<Scalar>*) form);
    else if(type == WeakForm_VecSurf)
        add_vector_form_surf((Hermes::Hermes2D::VectorFormSurf<Scalar>*) form);
    else
        assert(0);
}



template <typename Scalar>
void WeakFormAgros<Scalar>::registerForm(WeakFormKind type, Field *field, QString area, FormInfo *form, int offsetI, int offsetJ, Marker* marker)
{
    string problemId = field->fieldInfo()->fieldId().toStdString() + "_" +
            analysisTypeToStringKey(field->fieldInfo()->module()->analysisType()).toStdString()  + "_" +
            coordinateTypeToStringKey(field->fieldInfo()->module()->coordinateType()).toStdString() + "_" +
            ((field->fieldInfo()->linearityType() == LinearityType_Newton) ? "newton" : "linear") + "_";

    // compiled form
    Hermes::Hermes2D::Form<Scalar> *custom_form = factoryForm<Scalar>(type, QString::fromStdString(problemId), area, form, marker, NULL, offsetI, offsetJ);

    if (custom_form == NULL)
    {
        Util::log()->printWarning(QObject::tr("WeakForm"), QObject::tr("Cannot find compiled %1 (%2). %3, (%4, %5)").
                                  arg(field->fieldInfo()->fieldId()).arg(weakFormString(type)).arg(QString::fromStdString(problemId)).arg(offsetI).arg(offsetJ));
        throw AgrosSolverException("Compiled form not found");
    }

    // todo: remove, it will not be neccessary, since temporal forms will be handled separately
    if (field->fieldInfo()->analysisType() == AnalysisType_Transient)
    {
        FieldSolutionID solutionID = Util::solutionStore()->lastTimeAndAdaptiveSolution(field->fieldInfo(), SolutionMode_Finer);
        for (int comp = 0; comp < solutionID.group->module()->numberOfSolutions(); comp++)
        {
            custom_form->ext.push_back(Util::solutionStore()->solution(solutionID, comp).sln.data());
        }
    }

    addForm(type, custom_form);
}

template <typename Scalar>
void WeakFormAgros<Scalar>::registerTimeForms(BDF2Table *table)
{
    foreach(Field* field, m_block->fields())
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        assert(fieldInfo->fieldId() == "heat");

        // materials
        for (int labelNum = 0; labelNum<Util::scene()->labels->count(); labelNum++)
        {
            SceneMaterial *material = Util::scene()->labels->at(labelNum)->marker(fieldInfo);

            assert(material);
            if (material != Util::scene()->materials->getNone(fieldInfo))
            {
                Hermes::Hermes2D::Form<Scalar> *matrixForm = new CustomMatrixFormVol_time<Scalar>(0, 0, QString::number(labelNum).toStdString(), Hermes::Hermes2D::HERMES_NONSYM, material, table);
                Hermes::Hermes2D::Form<Scalar> *vectorForm = new CustomVectorFormVol_time<Scalar>(0, 0, QString::number(labelNum).toStdString(), material, table);
                Hermes::Hermes2D::Form<Scalar> *residualForm = new CustomVectorFormVol_time_residual<Scalar>(0, 0, QString::number(labelNum).toStdString(), material, table);

                // push previous solution in this order:
                // first all components of the solution in time level n-1
                // than all components of the solution in time level n-2
                // etc.
                int lastTimeStep = Util::solutionStore()->lastTimeStep(field->fieldInfo(), SolutionMode_Normal);
                for(int backLevel = 0; backLevel < table->n(); backLevel++)
                {
                    int timeStep = lastTimeStep - backLevel;
                    int adaptivityStep = Util::solutionStore()->lastAdaptiveStep(field->fieldInfo(), SolutionMode_Normal, timeStep);
                    FieldSolutionID solutionID(field->fieldInfo(), timeStep, adaptivityStep, SolutionMode_Reference);
                    if(! Util::solutionStore()->contains(solutionID))
                        solutionID.solutionMode = SolutionMode_Normal;
                    assert(Util::solutionStore()->contains(solutionID));

                    for (int comp = 0; comp < solutionID.group->module()->numberOfSolutions(); comp++)
                    {
                        matrixForm->ext.push_back(Util::solutionStore()->solution(solutionID, comp).sln.data());
                        vectorForm->ext.push_back(Util::solutionStore()->solution(solutionID, comp).sln.data());
                        residualForm->ext.push_back(Util::solutionStore()->solution(solutionID, comp).sln.data());
                    }
                }


                addForm(WeakForm_MatVol, matrixForm);
                addForm(WeakForm_VecVol, vectorForm);
                if(table->gamma()[1] != 0.0)
                    addForm(WeakForm_VecVol, residualForm);
            }
        }
    }
}

//TODO Source and target switched!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//TODO Source and target switched!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
template <typename Scalar>
void WeakFormAgros<Scalar>::registerFormCoupling(WeakFormKind type, QString area, FormInfo *form, int offsetI, int offsetJ,
                                         SceneMaterial* materialSource, SceneMaterial* materialTarget, CouplingInfo *couplingInfo)
{
    string problemId =
            materialSource->fieldInfo()->fieldId().toStdString() + "_" +
            materialTarget->fieldInfo()->fieldId().toStdString() + "_" +
            analysisTypeToStringKey(materialSource->fieldInfo()->module()->analysisType()).toStdString()  + "_" +
            analysisTypeToStringKey(materialTarget->fieldInfo()->module()->analysisType()).toStdString()  + "_" +
            coordinateTypeToStringKey(materialTarget->fieldInfo()->module()->coordinateType()).toStdString() + "_" +
            ((materialTarget->fieldInfo()->linearityType() == LinearityType_Newton) ? "newton" : "linear") + "_" +
            couplingTypeToStringKey(couplingInfo->couplingType()).toStdString();

    // compiled form
    //TODO Source and target switched!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //TODO Source and target switched!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    Hermes::Hermes2D::Form<Scalar> *custom_form = factoryForm<Scalar>(type, QString::fromStdString(problemId), area, form, materialTarget, materialSource, offsetI, offsetJ);

    if (!custom_form)
    {
        Util::log()->printWarning(QObject::tr("WeakForm"), QObject::tr("Cannot find compiled %2. %3, (%4, %5)").
                                  arg(weakFormString(type)).arg(QString::fromStdString(problemId)).arg(offsetI).arg(offsetJ));
        throw AgrosSolverException("Compiled form not found");
    }

    // TODO at the present moment, it is impossible to have more sources !
    //assert(field->m_couplingSources.size() <= 1);

    // push external solution for weak coupling
    if(couplingInfo->isWeak())
    {
        FieldSolutionID solutionID = Util::solutionStore()->lastTimeAndAdaptiveSolution(couplingInfo->sourceField(), SolutionMode_Finer);
        assert(solutionID.group->module()->numberOfSolutions() <= maxSourceFieldComponents);

        for (int comp = 0; comp < solutionID.group->module()->numberOfSolutions(); comp++)
        {
            custom_form->ext.push_back(Util::solutionStore()->solution(solutionID, comp).sln.data());
        }
    }

    addForm(type, custom_form);
}


template <typename Scalar>
void WeakFormAgros<Scalar>::registerForms()
{
    foreach(Field* field, m_block->fields())
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        
        // boundary conditions
        for (int edgeNum = 0; edgeNum<Util::scene()->edges->count(); edgeNum++)
        {
            SceneBoundary *boundary = Util::scene()->edges->at(edgeNum)->marker(fieldInfo);            
            if (boundary && boundary != Util::scene()->boundaries->getNone(fieldInfo))
            {
                Module::BoundaryType *boundary_type = fieldInfo->module()->boundaryType(boundary->getType());
                
                foreach (FormInfo *expression, boundary_type->wfMatrixSurface())
                    registerForm(WeakForm_MatSurf, field, QString::number(edgeNum), expression,
                                 m_block->offset(field), m_block->offset(field), boundary);
                
                foreach (FormInfo *expression, boundary_type->wfVectorSurface())
                    registerForm(WeakForm_VecSurf, field, QString::number(edgeNum), expression,
                                 m_block->offset(field), m_block->offset(field), boundary);
            }
        }
        
        // materials
        for (int labelNum = 0; labelNum<Util::scene()->labels->count(); labelNum++)
        {
            //cout << "material " << labelNum << endl;
            SceneMaterial *material = Util::scene()->labels->at(labelNum)->marker(fieldInfo);
            
            assert(material);
            if (material != Util::scene()->materials->getNone(fieldInfo))
            {
                foreach (FormInfo *expression, fieldInfo->module()->wfMatrixVolumeExpression())
                    registerForm(WeakForm_MatVol, field, QString::number(labelNum), expression,
                                 m_block->offset(field), m_block->offset(field), material);


                foreach (FormInfo *expression, fieldInfo->module()->wfVectorVolumeExpression())
                    registerForm(WeakForm_VecVol, field, QString::number(labelNum), expression,
                                 m_block->offset(field), m_block->offset(field), material);

                // weak coupling
                foreach(CouplingInfo* couplingInfo, field->m_couplingSources)
                {
                    foreach (FormInfo *expression, couplingInfo->coupling()->wfVectorVolumeExpression())
                    {
                        SceneMaterial* materialSource = Util::scene()->labels->at(labelNum)->marker(couplingInfo->sourceField());
                        assert(materialSource);

                        if (materialSource != Util::scene()->materials->getNone(couplingInfo->sourceField()))
                        {
                            cout << "register coupling form " << expression << endl;
                            registerFormCoupling(WeakForm_VecVol, QString::number(labelNum), expression,
                                         m_block->offset(field), m_block->offset(field), materialSource, material, couplingInfo);
                        }
                    }
                }
            }
        }
    }
    
    // hard coupling
    foreach (CouplingInfo* couplingInfo, m_block->couplings())
    {
        assert(couplingInfo->isHard());
        Coupling* coupling = couplingInfo->coupling();
        Field* sourceField = m_block->field(couplingInfo->sourceField());
        Field* targetField = m_block->field(couplingInfo->targetField());
        
        
        for (int labelNum = 0; labelNum<Util::scene()->labels->count(); labelNum++)
        {
            SceneMaterial *sourceMaterial = Util::scene()->labels->at(labelNum)->marker(sourceField->fieldInfo());
            SceneMaterial *targetMaterial = Util::scene()->labels->at(labelNum)->marker(targetField->fieldInfo());
            
            if (sourceMaterial && (sourceMaterial != Util::scene()->materials->getNone(sourceField->fieldInfo()))
                    && targetMaterial && (targetMaterial != Util::scene()->materials->getNone(targetField->fieldInfo())))
            {
                
                qDebug() << "hard coupling form on marker " << labelNum;

                foreach (FormInfo *pars, coupling->wfMatrixVolumeExpression())
                    registerFormCoupling(WeakForm_MatVol, QString::number(labelNum), pars,
                                 m_block->offset(targetField) - sourceField->fieldInfo()->module()->numberOfSolutions(), m_block->offset(sourceField),
                                 sourceMaterial, targetMaterial, couplingInfo);
                
                foreach (FormInfo *pars, coupling->wfVectorVolumeExpression())
                    registerFormCoupling(WeakForm_VecVol, QString::number(labelNum), pars,
                                 m_block->offset(targetField) - sourceField->fieldInfo()->module()->numberOfSolutions(), m_block->offset(sourceField),
                                 sourceMaterial, targetMaterial, couplingInfo);

            }
        }
    }
}

// ***********************************************************************************************

Module::LocalVariable::LocalVariable(XMLModule::localvariable lv,
                                     CoordinateType coordinateType, AnalysisType analysisType)
{
    m_id = QString::fromStdString(lv.id());
    m_name = QString::fromStdString(lv.name());
    m_shortname = QString::fromStdString(lv.shortname());
    m_shortnameHtml = (lv.shortname_html().present()) ? QString::fromStdString(lv.shortname_html().get()) : m_shortname;
    m_unit = QString::fromStdString(lv.unit());
    m_unitHtml = (lv.unit_html().present()) ? QString::fromStdString(lv.unit_html().get()) : m_unit;
    
    m_isScalar = (lv.type() == "scalar");
    
    for (int i = 0; i < lv.expression().size(); i++)
    {
        XMLModule::expression exp = lv.expression().at(i);
        if (exp.analysistype() == analysisTypeToStringKey(analysisType).toStdString())
        {
            if (coordinateType == CoordinateType_Planar)
                m_expression = Expression(m_isScalar ? QString::fromStdString(exp.planar().get()) : "",
                                          m_isScalar ? "" : QString::fromStdString(exp.planar_x().get()),
                                          m_isScalar ? "" : QString::fromStdString(exp.planar_y().get()));
            else
                m_expression = Expression(m_isScalar ? QString::fromStdString(exp.axi().get()) : "",
                                          m_isScalar ? "" : QString::fromStdString(exp.axi_r().get()),
                                          m_isScalar ? "" : QString::fromStdString(exp.axi_z().get()));
        }
    }
}

// ***********************************************************************************************

Module::MaterialTypeVariable::MaterialTypeVariable(XMLModule::quantity quant, CoordinateType coordinateType)
    : m_defaultValue(0), m_expressionNonlinear(""), m_isTimeDep(false)
{
    m_id = QString::fromStdString(quant.id());

    if (quant.shortname().present())
        m_shortname = QString::fromStdString(quant.shortname().get());

    if (quant.default_().present())
        m_defaultValue = quant.default_().get();
}

// ***********************************************************************************************

Module::BoundaryType::BoundaryType(QList<BoundaryTypeVariable> boundary_type_variables,
                                   XMLModule::boundary bdy,
                                   CoordinateType problem_type)
{
    m_id = QString::fromStdString(bdy.id());
    m_name = QString::fromStdString(bdy.name());
    
    // variables
    for (int i = 0; i < bdy.quantity().size(); i++)
    {
        XMLModule::quantity qty = bdy.quantity().at(i);
        
        foreach (Module::BoundaryTypeVariable old, boundary_type_variables)
        {
            if (old.id().toStdString() == qty.id())
            {
                bool isTimeDep = false;
                if (qty.dependence().present())
                    isTimeDep = (QString::fromStdString(qty.dependence().get()) == "time");

                Module::BoundaryTypeVariable *var = new Module::BoundaryTypeVariable(
                            old.id(), old.shortname(), old.defaultValue(), isTimeDep);
                
                m_variables.append(var);
            }
        }
    }
    
    // weakform
    for (int i = 0; i < bdy.matrix_form().size(); i++)
    {
        XMLModule::matrix_form form = bdy.matrix_form().at(i);
        m_wfMatrixSurface.append(new FormInfo(form.i(), form.j(), form.symmetric() ? Hermes::Hermes2D::HERMES_SYM : Hermes::Hermes2D::HERMES_NONSYM));
    }
    
    for (int i = 0; i < bdy.vector_form().size(); i++)
    {
        XMLModule::vector_form form = bdy.vector_form().at(i);
        m_wfVectorSurface.append(new FormInfo(form.i(), form.j()));
    }
    
    // essential
    for (int i = 0; i < bdy.essential_form().size(); i++)
    {
        XMLModule::essential_form form = bdy.essential_form().at(i);
        m_essential.append(new FormInfo(form.i()));
    }
}

Module::BoundaryTypeVariable::BoundaryTypeVariable(XMLModule::quantity quant)
{
    m_id = QString::fromStdString(quant.id());
    if (quant.shortname().present())
        m_shortname = QString::fromStdString(quant.shortname().get());
    if (quant.default_().present())
        m_defaultValue = quant.default_().get();
    else
        m_defaultValue = 0.0;
}

Module::BoundaryType::~BoundaryType()
{
    // essential
    m_essential.clear();
    
    // variables
    foreach (Module::BoundaryTypeVariable *variable, m_variables)
        delete variable;
    m_variables.clear();
    
    // volume weak form
    foreach (FormInfo *expression, m_wfMatrixSurface)
        delete expression;
    m_wfMatrixSurface.clear();
    
    foreach (FormInfo *expression, m_wfVectorSurface)
        delete expression;
    m_wfVectorSurface.clear();
    
    foreach (FormInfo *expression, m_wfVectorSurface)
        delete expression;
    m_wfVectorSurface.clear();
}

// ***********************************************************************************************

// dialog row UI
Module::DialogRow::DialogRow(XMLModule::quantity qty)
{
    m_id = QString::fromStdString(qty.id());
    m_name = (qty.name().present()) ? QString::fromStdString(qty.name().get()) : "";

    m_shortname = (qty.shortname().present()) ? QString::fromStdString(qty.shortname().get()) : "";
    m_shortnameHtml = (qty.shortname_html().present()) ? QString::fromStdString(qty.shortname_html().get()) : "";
    m_shortnameDependence = (qty.shortname_dependence().present()) ? QString::fromStdString(qty.shortname_dependence().get()) : "";
    m_shortnameDependenceHtml = (qty.shortname_dependence_html().present()) ? QString::fromStdString(qty.shortname_dependence_html().get()) : "";

    m_unit = (qty.unit().present()) ? QString::fromStdString(qty.unit().get()) : "";
    m_unitHtml = (qty.unit_html().present()) ? QString::fromStdString(qty.unit_html().get()) : "";
    m_unitLatex = (qty.unit_latex().present()) ? QString::fromStdString(qty.unit_latex().get()) : "";
    
    m_defaultValue = (qty.default_().present()) ? qty.default_().get() : 0.0;
    m_condition = (qty.condition().present()) ? QString::fromStdString(qty.condition().get()) : "0.0";
}

// dialog UI
Module::DialogUI::DialogUI(XMLModule::gui ui)
{
    for (int i = 0; i < ui.group().size(); i++)
    {
        XMLModule::group grp = ui.group().at(i);

        // group name
        QString name = (grp.name().present()) ? QString::fromStdString(grp.name().get()) : "";
        
        QList<Module::DialogRow> materials;
        for (int i = 0; i < grp.quantity().size(); i++)
        {
            XMLModule::quantity quant = grp.quantity().at(i);

            // append material
            materials.append(Module::DialogRow(quant));
        }
        
        m_groups[name] = materials;
    }
}

void Module::DialogUI::clear()
{
    m_groups.clear();
}

// ***********************************************************************************************

Module::BasicModule::BasicModule(const QString &fieldId, CoordinateType problemType, AnalysisType analysisType) :
    m_coordinateType(problemType), m_analysisType(analysisType),
    m_materialUI(NULL), m_boundaryUI(NULL),
    m_defaultViewScalarVariable(NULL), m_defaultViewVectorVariable(NULL), m_boundaryTypeDefault(NULL)
{
    clear();

    // open xml description
    read(datadir() + MODULEROOT + QDir::separator() + fieldId + ".xml");
}

Module::BasicModule::~BasicModule()
{
    clear();
}

void Module::BasicModule::read(const QString &filename)
{
    assert(QFile::exists(filename));

    // save current locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    module_xsd = XMLModule::module_(filename.toStdString().c_str());
    XMLModule::module *mod = module_xsd.get();

    // problem
    m_fieldid = QString::fromStdString(mod->general().id());
    m_name = QString::fromStdString(mod->general().name());
    m_hasDeformableShape = mod->general().deformed_shape();
    m_description = QString::fromStdString(mod->general().description());

    // analyses
    m_numberOfSolutions = 0;

    for (int i = 0; i < mod->general().analyses().analysis().size(); i++)
    {
        XMLModule::analysis an = mod->general().analyses().analysis().at(i);

        if (an.type() == analysisTypeToStringKey(m_analysisType).toStdString())
            m_numberOfSolutions = an.solutions();
    }

    // constants
    for (int i = 0; i < mod->constants().constant().size(); i++)
    {
        XMLModule::constant cnst = mod->constants().constant().at(i);
        m_constants[QString::fromStdString(cnst.id())] = cnst.value();
    }

    // macros
    /*
        for (rapidxml::xml_node<> *macro = doc.first_node("module")->first_node("macros")->first_node("macro");
             macro; macro = macro->next_sibling())
            macros[macro->first_attribute("id")->value()] = macro->first_attribute("expression")->value();
        */

    // surface weakforms
    QList<Module::BoundaryTypeVariable> boundary_type_variables_tmp;
    for (int i = 0; i < mod->surface().quantity().size(); i++)
    {
        XMLModule::quantity quant = mod->surface().quantity().at(i);
        boundary_type_variables_tmp.append(Module::BoundaryTypeVariable(quant));
    }

    for (int i = 0; i < mod->surface().weakforms_surface().weakform_surface().size(); i++)
    {
        XMLModule::weakform_surface wf = mod->surface().weakforms_surface().weakform_surface().at(i);

        if (wf.analysistype() == analysisTypeToStringKey(m_analysisType).toStdString())
            for (int i = 0; i < wf.boundary().size(); i++)
            {
                XMLModule::boundary bdy = wf.boundary().at(i);
                m_boundaryTypes.append(new Module::BoundaryType(boundary_type_variables_tmp, bdy, m_coordinateType));
            }

        // default
        m_boundaryTypeDefault = boundaryType(QString::fromStdString(wf.default_().get()));
    }
    boundary_type_variables_tmp.clear();

    // volumetric weakforms
    QList<Module::MaterialTypeVariable> material_type_variables_tmp;
    for (int i = 0; i < mod->volume().quantity().size(); i++)
    {
        XMLModule::quantity quant = mod->volume().quantity().at(i);
        material_type_variables_tmp.append(Module::MaterialTypeVariable(quant, m_coordinateType));
    }

    for (int i = 0; i < mod->volume().weakforms_volume().weakform_volume().size(); i++)
    {
        XMLModule::weakform_volume wf = mod->volume().weakforms_volume().weakform_volume().at(i);

        if (wf.analysistype() == analysisTypeToStringKey(m_analysisType).toStdString())
        {
            for (int i = 0; i < wf.quantity().size(); i++)
            {
                XMLModule::quantity qty = wf.quantity().at(i);

                for (QList<Module::MaterialTypeVariable>::iterator it = material_type_variables_tmp.begin();
                     it < material_type_variables_tmp.end(); ++it )
                {
                    Module::MaterialTypeVariable old = (Module::MaterialTypeVariable) *it;
                    if (old.id().toStdString() == qty.id())
                    {
                        QString nonlinearExpression;
                        if (m_coordinateType == CoordinateType_Planar && qty.nonlinearity_planar().present())
                            nonlinearExpression = QString::fromStdString(qty.nonlinearity_planar().get());
                        else
                            if (qty.nonlinearity_axi().present())
                                nonlinearExpression = QString::fromStdString(qty.nonlinearity_axi().get());

                        bool isTimeDep = false;
                        if (qty.dependence().present())
                            isTimeDep = (QString::fromStdString(qty.dependence().get()) == "time");

                        Module::MaterialTypeVariable *var = new Module::MaterialTypeVariable(
                                    old.id(), old.shortname(), old.defaultValue(),
                                    nonlinearExpression, isTimeDep);
                        m_materialTypeVariables.append(var);
                    }
                }
            }
            material_type_variables_tmp.clear();

            // weakform
            for (int i = 0; i < wf.matrix_form().size(); i++)
            {
                XMLModule::matrix_form form = wf.matrix_form().at(i);
                m_wfMatrixVolumeExpression.append(new FormInfo(form.i(), form.j(), form.symmetric() ? Hermes::Hermes2D::HERMES_SYM : Hermes::Hermes2D::HERMES_NONSYM));
            }

            for (int i = 0; i < wf.vector_form().size(); i++)
            {
                XMLModule::vector_form form = wf.vector_form().at(i);
                m_wfVectorVolumeExpression.append(new FormInfo(form.i(), form.j()));
            }
        }
    }

    // local variables
    for (int i = 0; i < mod->postprocessor().localvariables().localvariable().size(); i++)
    {
        XMLModule::localvariable lv = mod->postprocessor().localvariables().localvariable().at(i);

        // HACK
        for (int i = 0; i < lv.expression().size(); i++)
        {
            XMLModule::expression expr = lv.expression().at(i);
            if (expr.analysistype() == analysisTypeToStringKey(m_analysisType).toStdString())
            {
                Module::LocalVariable *var = new Module::LocalVariable(lv,
                                                                       m_coordinateType,
                                                                       m_analysisType);
                variables.append(var);

                // HACK - local point
                m_localPointVariables.append(var);
                // HACK - scalar view
                m_viewScalarVariables.append(var);
                // HACK - vector view
                if (!var->isScalar())
                    m_viewVectorVariables.append(var);
            }
        }
    }

    // scalar variables default
    for (int i = 0; i < mod->postprocessor().view().scalar_view().default_().size(); i++)
    {
        XMLModule::default_ def = mod->postprocessor().view().scalar_view().default_().at(i);
        if (def.analysistype() == analysisTypeToStringKey(m_analysisType).toStdString())
            m_defaultViewScalarVariable = localVariable(QString::fromStdString(def.id()));
    }

    // vector variables default
    for (int i = 0; i < mod->postprocessor().view().vector_view().default_().size(); i++)
    {
        XMLModule::default_ def = mod->postprocessor().view().vector_view().default_().at(i);
        if (def.analysistype() == analysisTypeToStringKey(m_analysisType).toStdString())
            m_defaultViewVectorVariable = localVariable(QString::fromStdString(def.id()));
    }

    // volume integral
    for (int i = 0; i < mod->postprocessor().volumeintegrals().volumeintegral().size(); i++)
    {
        XMLModule::volumeintegral vol = mod->postprocessor().volumeintegrals().volumeintegral().at(i);

        QString expr;
        for (int i = 0; i < vol.expression().size(); i++)
        {
            XMLModule::expression exp = vol.expression().at(i);
            if (exp.analysistype() == analysisTypeToStringKey(m_analysisType).toStdString())
            {
                if (m_coordinateType == CoordinateType_Planar)
                    expr = QString::fromStdString(exp.planar().get());
                else
                    expr = QString::fromStdString(exp.axi().get());
            }
        }
        expr = expr.trimmed();

        // new integral
        if (!expr.isEmpty())
        {
            Module::Integral *volint = new Module::Integral(
                        QString::fromStdString(vol.id()),
                        QString::fromStdString(vol.name()),
                        QString::fromStdString(vol.shortname()),
                        (vol.shortname_html().present()) ? QString::fromStdString(vol.shortname_html().get()) : QString::fromStdString(vol.shortname()),
                        QString::fromStdString(vol.unit()),
                        (vol.unit_html().present()) ? QString::fromStdString(vol.unit_html().get()) : QString::fromStdString(vol.unit()),
                        expr);

            m_volumeIntegrals.append(volint);
        }
    }

    // surface integral
    for (int i = 0; i < mod->postprocessor().surfaceintegrals().surfaceintegral().size(); i++)
    {
        XMLModule::surfaceintegral sur = mod->postprocessor().surfaceintegrals().surfaceintegral().at(i);

        QString expr;
        for (int i = 0; i < sur.expression().size(); i++)
        {
            XMLModule::expression exp = sur.expression().at(i);
            if (exp.analysistype() == analysisTypeToStringKey(m_analysisType).toStdString())
            {
                if (m_coordinateType == CoordinateType_Planar)
                    expr = QString::fromStdString(exp.planar().get());
                else
                    expr = QString::fromStdString(exp.axi().get());
            }
        }
        expr = expr.trimmed();

        // new integral
        if (!expr.isEmpty())
        {
            Module::Integral *surint = new Module::Integral(
                        QString::fromStdString(sur.id()),
                        QString::fromStdString(sur.name()),
                        QString::fromStdString(sur.shortname()),
                        (sur.shortname_html().present()) ? QString::fromStdString(sur.shortname_html().get()) : QString::fromStdString(sur.shortname()),
                        QString::fromStdString(sur.unit()),
                        (sur.unit_html().present()) ? QString::fromStdString(sur.unit_html().get()) : QString::fromStdString(sur.unit()),
                        expr);

            m_surfaceIntegrals.append(surint);
        }
    }

    // force
    XMLModule::force force = mod->postprocessor().force();
    for (int i = 0; i < force.expression().size(); i++)
    {
        XMLModule::expression exp = force.expression().at(i);
        if (exp.analysistype() == analysisTypeToStringKey(m_analysisType).toStdString())
            m_force = Module::Force((m_coordinateType == CoordinateType_Planar) ? QString::fromStdString(exp.planar_x().get()) : QString::fromStdString(exp.axi_r().get()),
                                    (m_coordinateType == CoordinateType_Planar) ? QString::fromStdString(exp.planar_y().get()) : QString::fromStdString(exp.axi_z().get()),
                                    (m_coordinateType == CoordinateType_Planar) ? QString::fromStdString(exp.planar_z().get()) : QString::fromStdString(exp.axi_phi().get()));
    }

    // preprocessor
    for (int i = 0; i < mod->preprocessor().gui().size(); i++)
    {
        XMLModule::gui ui = mod->preprocessor().gui().at(i);
        if (ui.type() == "volume")
            m_materialUI = new Module::DialogUI(ui);
        else if (ui.type() == "surface")
            m_boundaryUI = new Module::DialogUI(ui);
    }

    // set system locale
    setlocale(LC_NUMERIC, plocale);
}

void Module::BasicModule::clear()
{
    // general information
    m_fieldid = "";
    m_name = "";
    m_description = "";

    // constants
    m_constants.clear();

    // macros
    m_macros.clear();

    // boundary types
    for (QList<BoundaryType *>::iterator it = m_boundaryTypes.begin(); it < m_boundaryTypes.end(); ++it)
        delete *it;
    m_boundaryTypes.clear();
    m_boundaryTypeDefault = NULL;

    // material types
    for (QList<MaterialTypeVariable *>::iterator it = m_materialTypeVariables.begin(); it < m_materialTypeVariables.end(); ++it)
        delete *it;
    m_materialTypeVariables.clear();

    // variables
    foreach (Module::LocalVariable *variable, variables)
        delete variable;
    variables.clear();

    // scalar and vector variables
    m_viewScalarVariables.clear();
    m_viewVectorVariables.clear();

    // default variables
    m_defaultViewScalarVariable = NULL;
    m_defaultViewVectorVariable = NULL;

    // local variables
    m_localPointVariables.clear();

    // surface integrals
    m_surfaceIntegrals.clear();

    // volume integrals
    m_volumeIntegrals.clear();

    // volume weak form
    for (QList<FormInfo *>::iterator it = m_wfMatrixVolumeExpression.begin(); it < m_wfMatrixVolumeExpression.end(); ++it)
        delete *it;
    m_wfMatrixVolumeExpression.clear();

    for (QList<FormInfo *>::iterator it = m_wfVectorVolumeExpression.begin(); it < m_wfVectorVolumeExpression.end(); ++it)
        delete *it;
    m_wfVectorVolumeExpression.clear();

    if (m_materialUI)
    {
        m_materialUI->clear();
        delete m_materialUI;
    }
    m_materialUI = NULL;

    if (m_boundaryUI)
    {
        m_boundaryUI->clear();
        delete m_boundaryUI;
    }
    m_boundaryUI = NULL;
}

Module::LocalVariable *Module::BasicModule::localVariable(const QString &id)
{
    foreach (Module::LocalVariable *variable, variables)
        if (variable->id() == id)
            return variable;

    return NULL;
}

Module::BoundaryType *Module::BasicModule::boundaryType(const QString &id)
{
    foreach (Module::BoundaryType *boundaryType, m_boundaryTypes)
        if (boundaryType->id() == id)
            return boundaryType;

    return NULL;
}

Module::BoundaryTypeVariable *Module::BasicModule::boundaryTypeVariable(const QString &id)
{
    foreach (Module::BoundaryTypeVariable *variable, m_boundaryTypeVariables)
        if (variable->id() == id)
            return variable;

    return NULL;
}

Module::MaterialTypeVariable *Module::BasicModule::materialTypeVariable(const QString &id)
{
    foreach (Module::MaterialTypeVariable *variable, m_materialTypeVariables)
        if (variable->id() == id)
            return variable;

    return NULL;
}

mu::Parser *Module::BasicModule::expressionParser(const QString &expr)
{
    mu::Parser *parser = new mu::Parser();

    // pi
    parser->DefineConst("PI", M_PI);

    // frequency
    parser->DefineConst("f", Util::problem()->config()->frequency());

    // timestep
    parser->DefineConst("dt", Util::problem()->config()->timeStep().number());

    if (!expr.isEmpty())
        parser->SetExpr(expr.toStdString());

    QMapIterator<QString, double> itConstant(constants());
    while (itConstant.hasNext())
    {
        itConstant.next();

        parser->DefineConst(itConstant.key().toStdString(), itConstant.value());
    }

    parser->EnableOptimizer(true);

    return parser;
}

QString Module::BasicModule::expression(Module::LocalVariable *physicFieldVariable,
                                        PhysicFieldVariableComp physicFieldVariableComp)
{
    switch (physicFieldVariableComp)
    {
    case PhysicFieldVariableComp_Undefined:
        return "";
    case PhysicFieldVariableComp_Scalar:
        return physicFieldVariable->expression().scalar();
    case PhysicFieldVariableComp_X:
        return physicFieldVariable->expression().compX();
    case PhysicFieldVariableComp_Y:
        return physicFieldVariable->expression().compY();
    case PhysicFieldVariableComp_Magnitude:
        return "sqrt((" + physicFieldVariable->expression().compX() + ") * (" + physicFieldVariable->expression().compX() + ") + (" + physicFieldVariable->expression().compY() + ") * (" + physicFieldVariable->expression().compY() + "))";
    default:
        qCritical() << "Unknown type.";
        assert(0);
    }
}

ViewScalarFilter<double> *Module::BasicModule::viewScalarFilter(Module::LocalVariable *physicFieldVariable,
                                                                PhysicFieldVariableComp physicFieldVariableComp)
{
    // update time functions
    /*
    if (m_fieldInfo->analysisType() == AnalysisType_Transient)
        m_fieldInfo->module()->update_time_functions(Util::problem()->time());
    */

    Hermes::vector<Hermes::Hermes2D::MeshFunction<double> *> sln;
    for (int k = 0; k < numberOfSolutions(); k++)
    {
        FieldSolutionID fsid(Util::scene()->activeViewField(), Util::scene()->activeTimeStep(), Util::scene()->activeAdaptivityStep(), Util::scene()->activeSolutionType());
        sln.push_back(Util::solutionStore()->multiSolution(fsid).component(k).sln.data());
    }
    return new ViewScalarFilter<double>(Util::scene()->activeViewField(),
                                        sln,
                                        expression(physicFieldVariable, physicFieldVariableComp));
}

// ***********************************************************************************************

void readMeshDirtyFix()
{
    // fix precalulating matrices for mapping of curved elements

    // save locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    std::ostringstream os;
    os << "vertices = [" << std::endl <<
          "  [ 0, 0 ]," << std::endl <<
          "  [ 1, 0 ]," << std::endl <<
          "  [ 0, 1 ]" << std::endl <<
          "]" << std::endl << std::endl <<
          "elements = [" << std::endl <<
          "  [ 0, 1, 2, \"element_0\" ]" << std::endl << std::endl <<
          "boundaries = [" << std::endl <<
          "  [ 0, 1, \"0\" ]," << std::endl <<
          "  [ 1, 2, \"0\" ]," << std::endl <<
          "  [ 2, 0, \"0\" ]" << std::endl <<
          "]" << std::endl << std::endl <<
          "curves = [" << std::endl <<
          "  [ 0, 1, 90 ]" << std::endl <<
          "]" << std::endl;

    Hermes::Hermes2D::Mesh mesh;
    Hermes::Hermes2D::MeshReaderH2D meshloader;

    std::ofstream outputFile((tempProblemDir().toStdString() + "/dummy.mesh").c_str(), fstream::out);
    outputFile << os.str();
    outputFile.close();

    meshloader.load((tempProblemDir().toStdString() + "/dummy.mesh").c_str(), &mesh);

    // set system locale
    setlocale(LC_NUMERIC, plocale);
}

QMap<FieldInfo*, Hermes::Hermes2D::Mesh*> readMeshesFromFile(const QString &fileName)
{
    // save locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    // load the mesh file
    Hermes::Hermes2D::MeshReaderH2DXML meshloader;
    Hermes::vector<Hermes::Hermes2D::Mesh*> meshes;
    QMap<FieldInfo*, Hermes::Hermes2D::Mesh*> meshesMap;
    foreach(FieldInfo* fieldInfo, Util::problem()->fieldInfos())
    {
        Hermes::Hermes2D::Mesh *mesh = new Hermes::Hermes2D::Mesh();
        meshes.push_back(mesh);
        meshesMap[fieldInfo] = mesh;
    }

    meshloader.load(fileName.toStdString().c_str(), meshes);

    // set system locale
    setlocale(LC_NUMERIC, plocale);

    return meshesMap;
}

void writeMeshFromFile(const QString &fileName, Hermes::Hermes2D::Mesh *mesh)
{
    // save locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    Hermes::Hermes2D::MeshReaderH2D meshloader;
    meshloader.save(fileName.toStdString().c_str(), mesh);

    // set system locale
    setlocale(LC_NUMERIC, plocale);
}

void refineMesh(FieldInfo *fieldInfo, Hermes::Hermes2D::Mesh *mesh, bool refineGlobal, bool refineTowardsEdge)
{
    // refine mesh - global
    if (refineGlobal)
        for (int i = 0; i < fieldInfo->numberOfRefinements(); i++)
            mesh->refine_all_elements(0);

    // refine mesh - boundary
    int i = 0;
    if (refineTowardsEdge)
        foreach (SceneEdge *edge, Util::scene()->edges->items())
        {
            // TODO: add refine towards edge
            /*
            if (edge->refineTowardsEdge > 0)
                mesh->refine_towards_boundary(QString::number(((edge->getMarker(fieldInfo)
                                                                != SceneBoundaryContainer::getNone(fieldInfo)) ? i + 1 : -i)).toStdString(),
                                              edge->refineTowardsEdge);
            */
            i++;
        }
}

template class WeakFormAgros<double>;
