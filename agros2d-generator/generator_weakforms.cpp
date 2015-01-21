#include "generator.h"
#include "generator_module.h"
#include "parser.h"
#include "util/constants.h"

#include "solver/weak_form.h"
#include "solver/module.h"
#include "solver/coupling.h"

void Agros2DGeneratorModule::generatePluginWeakFormFiles()
{
    generatePluginWeakFormSourceFiles();
    generatePluginWeakFormHeaderFiles();
}

void Agros2DGeneratorModule::generatePluginWeakFormSourceFiles()
{
    qDebug() << (QString("generating weakform source file").toLatin1());

    QString id = QString::fromStdString(m_module->general_field().id());
    std::string textWeakform;

    ExpandTemplate(compatibleFilename(QString("%1/%2/weakform_cpp.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                   ctemplate::DO_NOT_STRIP, m_output, &textWeakform);

    // source - save to file
    writeStringContent(QString("%1/%2/%3/%3_weakform.cpp").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(textWeakform));

    std::string textExtFunction;

    ExpandTemplate(compatibleFilename(QString("%1/%2/extfunction_cpp.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                   ctemplate::DO_NOT_STRIP, m_output, &textExtFunction);

    // source - save to file
    writeStringContent(QString("%1/%2/%3/%3_extfunction.cpp").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(textExtFunction));
}

void Agros2DGeneratorModule::generatePluginWeakFormHeaderFiles()
{
    qDebug() << (QString("generating weakform header file").toLatin1());

    QString id = QString::fromStdString(m_module->general_field().id());
    std::string textWeakform;

    // header - expand template
    std::string text;
    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/weakform_h.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, m_output, &textWeakform);

    // header - save to file
    writeStringContent(QString("%1/%2/%3/%3_weakform.h").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(textWeakform));

    std::string textExtFunction;

    // header - expand ext functions
    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/extfunction_h.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, m_output, &textExtFunction);

    // header - save to file
    writeStringContent(QString("%1/%2/%3/%3_extfunction.h").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(textExtFunction));
}


void Agros2DGeneratorModule::generateExtFunctions(ctemplate::TemplateDictionary &output)
{
    foreach(XMLModule::weakform_volume weakform, m_module->volume().weakforms_volume().weakform_volume())
    {
        AnalysisType analysisType = analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype().c_str()));
        foreach(XMLModule::linearity_option linearityOption, weakform.linearity_option())
        {
            LinearityType linearityType = linearityTypeFromStringKey(QString::fromStdString(linearityOption.type().c_str()));
            foreach (CoordinateType coordinateType, Agros2DGenerator::coordinateTypeList())
            {
                ctemplate::TemplateDictionary *section;
                section = output.AddSectionDictionary("EXT_FUNCTIONS_PART");

                QString sectionName = QString("extFunctions_%1_%2_%3").
                        arg(analysisTypeToStringKey(analysisType)).
                        arg(linearityTypeToStringKey(linearityType)).
                        arg(coordinateTypeToStringKey(coordinateType));
                section->SetValue("PART_NAME", sectionName.toStdString());
                section->SetValue("COORDINATE_TYPE", Agros2DGenerator::coordinateTypeStringEnum(coordinateType).toStdString());
                section->SetValue("ANALYSIS_TYPE", Agros2DGenerator::analysisTypeStringEnum(analysisType).toStdString());
                section->SetValue("LINEARITY_TYPE", Agros2DGenerator::linearityTypeStringEnum(linearityType).toStdString());

                foreach(XMLModule::quantity quantity, weakform.quantity())
                {
                    generateExtFunction(quantity, analysisType, linearityType, coordinateType, false, false, *section);
                    if(quantityIsNonlinear[QString::fromStdString(quantity.id())])
                    {
                        // generate derivative
                        generateExtFunction(quantity, analysisType, linearityType, coordinateType, true, false, *section);

                        // generate linearized version
                        generateExtFunction(quantity, analysisType, linearityType, coordinateType, false, true, *section);
                    }
                }

                foreach(XMLModule::function_use functionUse, weakform.function_use())
                {
                    foreach(XMLModule::function function, m_module->volume().function())
                    {
                        if(function.id() == functionUse.id())
                        {
//                            if(function.interpolation_count().present())
//                                generateSpecialFunction(function, analysisType, linearityType, coordinateType, *section);
//                            else
//                            {
                                generateValueExtFunction(function, analysisType, linearityType, coordinateType, false, *section);
                                generateValueExtFunction(function, analysisType, linearityType, coordinateType, true, *section);
//                            }
                        }
                    }
                }

            }
        }
    }
}


void Agros2DGeneratorModule::generateWeakForms(ctemplate::TemplateDictionary &output)
{
    this->m_docString = "";

    QMap<QString, QString> availableModules = Module::availableModules();
    QMap<QString, XMLModule::coupling *> xml_couplings;

    QMap<QString, std::shared_ptr<XMLModule::module> > couplings_xsd;
    std::shared_ptr<XMLModule::module> coupling_xsd;

    //qDebug() << couplingList()->availableCouplings();
    foreach(QString sourceField, availableModules.keys())
    {
        if(couplingList()->isCouplingAvailable(sourceField, this->m_id, CouplingType_Weak))
        {
            ctemplate::TemplateDictionary *coupling = output.AddSectionDictionary("COUPLING_SOURCE");
            coupling->SetValue("COUPLING_SOURCE_ID", sourceField.toStdString());

            coupling_xsd = XMLModule::module_(compatibleFilename(datadir() + COUPLINGROOT + "/" + sourceField + "-" + this->m_id + ".xml").toStdString(), xml_schema::flags::dont_validate);
            XMLModule::module *mod = coupling_xsd.get();
            assert(mod->coupling().present());
            xml_couplings[sourceField] = &mod->coupling().get();
            couplings_xsd[sourceField] = coupling_xsd;
        }
    }

    // volume
    foreach(XMLModule::weakform_volume weakform, m_module->volume().weakforms_volume().weakform_volume())
    {
        AnalysisType analysisType = analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype().c_str()));

        foreach (CoordinateType coordinateType, Agros2DGenerator::coordinateTypeList())
        {
            foreach(XMLModule::linearity_option option, weakform.linearity_option())
            {
                LinearityType linearityType = linearityTypeFromStringKey(QString::fromStdString(option.type().c_str()));

                ctemplate::TemplateDictionary *fieldVolume = generateVolumeVariables(linearityType, coordinateType, output, weakform, "VOLUME");
                QList<FormInfo> matrixForms = Module::wfMatrixVolumeSeparated(m_module, analysisType, linearityType);
                foreach(FormInfo formInfo, matrixForms)
                {
                    generateFormExpression(formInfo, linearityType, coordinateType, *fieldVolume, "MATRIX", weakform);
                }

                QList<FormInfo> vectorForms = Module::wfVectorVolumeSeparated(m_module, analysisType, linearityType);
                foreach(FormInfo formInfo, vectorForms)
                {
                    generateFormExpression(formInfo, linearityType, coordinateType, *fieldVolume, "VECTOR", weakform);
                }
                foreach(QString sourceField, xml_couplings.keys())
                {
                    ctemplate::TemplateDictionary *coupling = fieldVolume->AddSectionDictionary("COUPLING_SOURCE");
                    coupling->SetValue("COUPLING_SOURCE_ID", sourceField.toStdString());

                    // todo: loop over source analysis types
                    QList<FormInfo> vectorForms = CouplingInfo::wfVectorVolumeSeparated(&(xml_couplings[sourceField]->volume()), analysisType, analysisType, CouplingType_Weak, linearityType);
                    if (!vectorForms.isEmpty())
                    {
                        foreach(FormInfo formInfo, vectorForms)
                        {
                            generateFormExpression(formInfo, linearityType, coordinateType, *coupling, "VECTOR", weakform);
                        }
                    }
                }

                // find if there are any possible coupling sources
                foreach(QString mod, availableModules.keys())
                {
                    //qDebug() << mod;
                }
            }
        }
    }

    // surface
    foreach(XMLModule::weakform_surface weakform, m_module->surface().weakforms_surface().weakform_surface())
    {
        AnalysisType analysisType = analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype().c_str()));
        foreach(XMLModule::boundary boundary, weakform.boundary())
        {
            foreach (CoordinateType coordinateType, Agros2DGenerator::coordinateTypeList())
            {
                foreach(XMLModule::linearity_option option, boundary.linearity_option())
                {
                    LinearityType linearityType = linearityTypeFromStringKey(QString::fromStdString(option.type().c_str()));

                    ctemplate::TemplateDictionary *fieldSurface = generateSurfaceVariables(linearityType, coordinateType, output, weakform, "SURFACE", &boundary);
                    QList<FormInfo> matrixForms = Module::wfMatrixSurface(&m_module->surface(), &boundary, analysisType, linearityType);
                    foreach(FormInfo formInfo, matrixForms)
                    {
                        generateFormExpression(formInfo, linearityType, coordinateType, *fieldSurface, "MATRIX", weakform);
                    }

                    QList<FormInfo> vectorForms = Module::wfVectorSurface(&m_module->surface(), &boundary, analysisType, linearityType);
                    foreach(FormInfo formInfo, vectorForms)
                    {
                        generateFormExpression(formInfo, linearityType, coordinateType, *fieldSurface, "VECTOR", weakform);
                    }

                    QList<FormInfo> essentialForms = Module::essential(&m_module->surface(), &boundary, analysisType, linearityType);
                    if (!essentialForms.isEmpty())
                    {
                        ctemplate::TemplateDictionary *fieldExact = generateSurfaceVariables(linearityType, coordinateType, output, weakform, "EXACT", &boundary);

                        // TODO: do it better
                        // form mask (for essential bcs)
                        int numSolutions = Agros2DGenerator::numberOfSolutions(m_module->general_field().analyses(), analysisType);
                        fieldExact->SetValue("NUM_SOLUTIONS", QString::number(numSolutions).toStdString());

                        for (int i = 0; i < numSolutions; i++)
                        {
                            QString maskValue = "false";
                            ctemplate::TemplateDictionary *mask = fieldExact->AddSectionDictionary("FORM_EXPRESSION_MASK");
                            foreach (FormInfo formInfo, essentialForms)
                            {
                                if (formInfo.i - 1 == i)
                                    maskValue = "true";
                            }
                            mask->SetValue("MASK", maskValue.toStdString());
                        }

                        foreach (FormInfo formInfo, essentialForms)
                        {
                            generateFormExpression(formInfo, linearityType, coordinateType, *fieldExact, "ESSENTIAL", weakform);
                        }
                    }
                }
            }
        }
    }
}


void Agros2DGeneratorModule::generateExtFunction(XMLModule::quantity quantity, AnalysisType analysisType, LinearityType linearityType, CoordinateType coordinateType, bool derivative, bool linearize, ctemplate::TemplateDictionary &output)
{
    QString type;
    if(derivative)
    {
        if(linearize)
            assert(0);
        else
            type = "derivative";
    }
    else
    {
        if(linearize)
            type = "value_linearized";
        else
            type = "value";
    }

    QString functionName = QString("ext_function_%1_%2_%3_%4_%5_%6").
            arg(QString::fromStdString(m_module->general_field().id())).
            arg(analysisTypeToStringKey(analysisType)).
            arg(linearityTypeToStringKey(linearityType)).
            arg(coordinateTypeToStringKey(coordinateType)).
            arg(QString::fromStdString(quantity.id())).
            arg(type);

    ctemplate::TemplateDictionary *field;
    field = output.AddSectionDictionary("EXT_FUNCTION");
    field->SetValue("EXT_FUNCTION_NAME", functionName.toStdString());
    QString dependence("0");

    if (linearityType != LinearityType_Linear)
    {
        if ((coordinateType == CoordinateType_Planar) && (quantity.nonlinearity_planar().present()))
            dependence = QString::fromStdString(quantity.nonlinearity_planar().get());
        if ((coordinateType == CoordinateType_Axisymmetric) && (quantity.nonlinearity_axi().present()))
            dependence = QString::fromStdString(quantity.nonlinearity_axi().get());

        ParserModuleInfo pmi(*m_module, analysisType, coordinateType, linearityType);

        // if linearized, we use dependence on allready calculated values form previous time level or weakly coupled source field
        if (linearize)
            dependence = Parser::parseLinearizeDependence(pmi, dependence);
        else
            dependence = Parser::parseWeakFormExpression(pmi, dependence);
    }

    // nonlinear or constant (in which case numberFromTable returns just a constant number)
    QString valueMethod("numberFromTable");
    if(derivative)
        valueMethod = "derivativeFromTable";

    // other dependence
    if(quantity.dependence().present())
    {
        if(quantity.dependence().get() == "time")
        {
            //                valueMethod = "numberAtTime";
            //                dependence = "Agros2D::problem()->actualTime()";
        }
        else if(quantity.dependence().get() == "")
        {
            // todo: why are for some quantities in XML dependence=""? remove?
        }
        else
        {
            std::cout << "not implemented " << quantity.dependence().get() << std::endl;
            assert(0);
        }
    }

    QString shortname;
    foreach(XMLModule::quantity quantityDefinition, m_module->volume().quantity())
    {
        if(quantityDefinition.id() == quantity.id())
        {
            shortname = QString::fromStdString(quantityDefinition.shortname().get());
            break;
        }
    }

    field->SetValue("DEPENDENCE", dependence.toStdString());
    field->SetValue("VALUE_METHOD", valueMethod.toStdString());
    field->SetValue("COORDINATE_TYPE", Agros2DGenerator::coordinateTypeStringEnum(coordinateType).toStdString());
    field->SetValue("ANALYSIS_TYPE", Agros2DGenerator::analysisTypeStringEnum(analysisType).toStdString());
    field->SetValue("LINEARITY_TYPE", Agros2DGenerator::linearityTypeStringEnum(linearityType).toStdString());
    field->SetValue("QUANTITY_ID", quantity.id());
    field->SetValue("QUANTITY_SHORTNAME", shortname.toStdString());

    if (derivative)
        field->SetValue("IS_DERIVATIVE", "true");
    else
        field->SetValue("IS_DERIVATIVE", "false");
    if (linearize)
        field->SetValue("IS_LINEARIZED", "true");
    else
        field->SetValue("IS_LINEARIZED", "false");
}

template <typename WeakForm>
void Agros2DGeneratorModule::generateFormExpression(FormInfo formInfo,
                                                    LinearityType linearityType,
                                                    CoordinateType coordinateType,
                                                    ctemplate::TemplateDictionary &output,
                                                    QString formType,
                                                    WeakForm weakform)
{
    QString expression = (coordinateType == CoordinateType_Planar ? formInfo.expr_planar : formInfo.expr_axi);

    if (!expression.isEmpty())
    {
        ctemplate::TemplateDictionary *expr = output.AddSectionDictionary(("FORM_EXPRESSION_" + formType).toStdString());

        expr->SetValue("EXPRESSION_ID", formInfo.id.toStdString());

        if (formInfo.j != 0)
        {
            expr->SetValue("COLUMN_INDEX", QString::number(formInfo.j - 1).toStdString());
        }
        else
        {
            expr->SetValue("COLUMN_INDEX", "0");
        }

//        foreach(XMLModule::function_use functionUse, weakform.function_use())
//        {
//            foreach(XMLModule::function functionDefinition, m_module->volume().function())
//            {
//                if (functionUse.id() == functionDefinition.id())
//                {
//                    generateSpecialFunction(functionDefinition, analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype())), linearityType, coordinateType, *expr);
//                }
//            }
//        }

        expr->SetValue("ROW_INDEX", QString::number(formInfo.i - 1).toStdString());

        ParserModuleInfo pmi(*m_module, analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype())), coordinateType, linearityType);

        // expression
        QString exprCpp = Parser::parseWeakFormExpression(pmi, expression);
        expr->SetValue("EXPRESSION", exprCpp.toStdString());

        QString exprCppCheck = Parser::parseWeakFormExpressionCheck(pmi, formInfo.condition);
        if (exprCppCheck == "")
            exprCppCheck = "true";
        expr->SetValue("EXPRESSION_CHECK", exprCppCheck.toStdString());
    }
}

template <typename WeakForm>
ctemplate::TemplateDictionary *Agros2DGeneratorModule::generateVolumeVariables(LinearityType linearityType,
                                                                               CoordinateType coordinateType,
                                                                               ctemplate::TemplateDictionary &output,
                                                                               WeakForm weakform,
                                                                               QString weakFormType)
{
    ctemplate::TemplateDictionary *field = output.AddSectionDictionary(weakFormType.toStdString() + "_SOURCE");

    field->SetValue("COORDINATE_TYPE", Agros2DGenerator::coordinateTypeStringEnum(coordinateType).toStdString());
    field->SetValue("LINEARITY_TYPE", Agros2DGenerator::linearityTypeStringEnum(linearityType).toStdString());
    field->SetValue("ANALYSIS_TYPE", Agros2DGenerator::analysisTypeStringEnum(analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype()))).toStdString());

    ParserModuleInfo pmi(*m_module, analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype())), coordinateType, linearityType);

    foreach(XMLModule::quantity quantity, weakform.quantity())
    {
        QString nonlinearExpr = pmi.nonlinearExpressionVolume(QString::fromStdString(quantity.id()));

        if (linearityType != LinearityType_Linear && quantityIsNonlinear[QString::fromStdString(quantity.id())])
        {
            ctemplate::TemplateDictionary *subFieldNonlinear = field->AddSectionDictionary("VARIABLE_SOURCE_NONLINEAR");
            subFieldNonlinear->SetValue("VARIABLE", quantity.id().c_str());
            subFieldNonlinear->SetValue("VARIABLE_HASH", QString::number(qHash(QString::fromStdString(quantity.id()))).toStdString());
            subFieldNonlinear->SetValue("VARIABLE_SHORT", m_volumeVariables.value(QString::fromStdString(quantity.id().c_str())).toStdString());

            // nonlinear value and derivative
            subFieldNonlinear->SetValue("VARIABLE_VALUE", QString("numberFromTable(%1)").
                                        arg(Parser::parseErrorExpression(pmi, nonlinearExpr)).toStdString());
            subFieldNonlinear->SetValue("VARIABLE_DERIVATIVE", QString("derivativeFromTable(%1)").
                                        arg(Parser::parseErrorExpression(pmi, nonlinearExpr)).toStdString());
        }
        else
        {
            // linear only value

            ctemplate::TemplateDictionary *subFieldLinear = field->AddSectionDictionary("VARIABLE_SOURCE_LINEAR");
            subFieldLinear->SetValue("VARIABLE", quantity.id().c_str());
            subFieldLinear->SetValue("VARIABLE_HASH", QString::number(qHash(QString::fromStdString(quantity.id()))).toStdString());
            subFieldLinear->SetValue("VARIABLE_SHORT", m_volumeVariables.value(QString::fromStdString(quantity.id().c_str())).toStdString());

            subFieldLinear->SetValue("VARIABLE_VALUE", QString("number()").toStdString());
        }
    }

    // new generation of functions. It is much simpler than whe using Hermes, where external functions had to be created
    foreach(XMLModule::function_use function_use, weakform.function_use())
    {
//        QString nonlinearExpr = pmi.nonlinearExpressionVolume(QString::fromStdString(quantity.id()));

//        if (linearityType != LinearityType_Linear && quantityIsNonlinear[QString::fromStdString(quantity.id())])
//        {
//            ctemplate::TemplateDictionary *subFieldNonlinear = field->AddSectionDictionary("VARIABLE_SOURCE_NONLINEAR");
//            subFieldNonlinear->SetValue("VARIABLE", quantity.id().c_str());
//            subFieldNonlinear->SetValue("VARIABLE_SHORT", m_volumeVariables.value(QString::fromStdString(quantity.id().c_str())).toStdString());

//            // nonlinear value and derivative
//            subFieldNonlinear->SetValue("VARIABLE_VALUE", QString("numberFromTable(%1)").
//                                        arg(Parser::parseErrorExpression(pmi, nonlinearExpr)).toStdString());
//            subFieldNonlinear->SetValue("VARIABLE_DERIVATIVE", QString("derivativeFromTable(%1)").
//                                        arg(Parser::parseErrorExpression(pmi, nonlinearExpr)).toStdString());
//        }
//        else
        {
            // linear only value

            foreach(XMLModule::function function, pmi.volume.function())
            {
                if(function.id() == function_use.id())
                {
                    bool is_constant = (function.type() == "constant");

                    if(linearityType != LinearityType_Linear)
                    {
                        // find out if it does depend on some quantity, which is not constant on element
                        foreach(XMLModule::quantity quantity, function.quantity())
                        {
                            if(quantityIsNonlinear[QString::fromStdString(quantity.id())])
                            {
                                is_constant = false;
                            }
                        }
                    }

                    if((function.type() == "nonlinear") && linearityType == LinearityType_Linear)
                    {

                    }
                    else
                    {
                        ctemplate::TemplateDictionary *subFieldLinear;
                        if(is_constant)
                            subFieldLinear= field->AddSectionDictionary("FUNCTION_SOURCE_CONSTANT");
                        else
                            subFieldLinear= field->AddSectionDictionary("FUNCTION_SOURCE_NONCONSTANT");

                        QString expression;
                        if((coordinateType == CoordinateType_Axisymmetric) && function.function_variant()[0].expr_axi().present())
                            expression = QString::fromStdString(function.function_variant()[0].expr_axi().get());
                        else
                            expression = QString::fromStdString(function.function_variant()[0].expr());

                        subFieldLinear->SetValue("FUNCTION_SHORT", m_volumeVariables.value(QString::fromStdString(function_use.id().c_str())).toStdString());
                        subFieldLinear->SetValue("FUNCTION_EXPRESSION", Parser::parseWeakFormExpression(pmi, expression).toStdString());
                    }
                }
            }

        }
    }

    return field;
}

template <typename WeakForm>
ctemplate::TemplateDictionary *Agros2DGeneratorModule::generateSurfaceVariables(LinearityType linearityType,
                                                                                CoordinateType coordinateType,
                                                                                ctemplate::TemplateDictionary &output,
                                                                                WeakForm weakform,
                                                                                QString weakFormType,
                                                                                XMLModule::boundary *boundary)
{
    ctemplate::TemplateDictionary *field = output.AddSectionDictionary(weakFormType.toStdString() + "_SOURCE");

    field->SetValue("COORDINATE_TYPE", Agros2DGenerator::coordinateTypeStringEnum(coordinateType).toStdString());
    field->SetValue("LINEARITY_TYPE", Agros2DGenerator::linearityTypeStringEnum(linearityType).toStdString());
    field->SetValue("ANALYSIS_TYPE", Agros2DGenerator::analysisTypeStringEnum(analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype()))).toStdString());

    field->SetValue("BOUNDARY_ID", boundary->id().c_str());

    ParserModuleInfo pmi(*m_module, analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype())), coordinateType, linearityType);

    foreach(XMLModule::quantity quantity, boundary->quantity())
    {
        QString dep = pmi.dependenceSurface(QString::fromStdString(quantity.id()));

        // value
        if (dep.isEmpty())
        {
            // linear only value
            ctemplate::TemplateDictionary *subFieldLinear = field->AddSectionDictionary("VARIABLE_SOURCE_LINEAR");
            subFieldLinear->SetValue("VARIABLE", quantity.id().c_str());
            subFieldLinear->SetValue("VARIABLE_HASH", QString::number(qHash(QString::fromStdString(quantity.id()))).toStdString());
            subFieldLinear->SetValue("VARIABLE_SHORT", m_surfaceVariables.value(QString::fromStdString(quantity.id().c_str())).toStdString());

            subFieldLinear->SetValue("VARIABLE_VALUE", QString("number()").toStdString());
        }
        else if (dep == "time")
        {
            // linear only value
            ctemplate::TemplateDictionary *subFieldLinear = field->AddSectionDictionary("VARIABLE_SOURCE_LINEAR");
            subFieldLinear->SetValue("VARIABLE", quantity.id().c_str());
            subFieldLinear->SetValue("VARIABLE_HASH", QString::number(qHash(QString::fromStdString(quantity.id()))).toStdString());
            subFieldLinear->SetValue("VARIABLE_SHORT", m_surfaceVariables.value(QString::fromStdString(quantity.id().c_str())).toStdString());

            // linear boundary condition
            subFieldLinear->SetValue("VARIABLE_VALUE", QString("numberAtTime(Agros2D::problem()->actualTime())").toStdString());
        }
        else if (dep == "space")
        {
            // nonlinear case
            ctemplate::TemplateDictionary *subFieldNonlinear = field->AddSectionDictionary("VARIABLE_SOURCE_NONLINEAR");
            subFieldNonlinear->SetValue("VARIABLE", quantity.id().c_str());
            subFieldNonlinear->SetValue("VARIABLE_HASH", QString::number(qHash(QString::fromStdString(quantity.id()))).toStdString());
            subFieldNonlinear->SetValue("VARIABLE_SHORT", m_surfaceVariables.value(QString::fromStdString(quantity.id().c_str())).toStdString());

            // spacedep boundary condition
            subFieldNonlinear->SetValue("VARIABLE_VALUE", QString("numberAtPoint(Point(p[0], p[1]))").toStdString());
        }
        else if (dep == "time-space")
        {
            // nonlinear case
            ctemplate::TemplateDictionary *subFieldNonlinear = field->AddSectionDictionary("VARIABLE_SOURCE_NONLINEAR");
            subFieldNonlinear->SetValue("VARIABLE", quantity.id().c_str());
            subFieldNonlinear->SetValue("VARIABLE_HASH", QString::number(qHash(QString::fromStdString(quantity.id()))).toStdString());
            subFieldNonlinear->SetValue("VARIABLE_SHORT", m_surfaceVariables.value(QString::fromStdString(quantity.id().c_str())).toStdString());

            // spacedep boundary condition
            subFieldNonlinear->SetValue("VARIABLE_VALUE", QString("numberAtTimeAndPoint(Agros2D::problem()->actualTime(), Point(p[0], p[1]))").toStdString());
        }
    }

    return field;
}

void Agros2DGeneratorModule::generateValueExtFunction(XMLModule::function function, AnalysisType analysisType, LinearityType linearityType, CoordinateType coordinateType, bool linearize, ctemplate::TemplateDictionary &output)
{
    ctemplate::TemplateDictionary *functionTemplate = output.AddSectionDictionary("VALUE_FUNCTION_SOURCE");
    functionTemplate->SetValue("VALUE_FUNCTION_NAME", function.shortname());
    functionTemplate->SetValue("VALUE_FUNCTION_ID", function.id());
    functionTemplate->SetValue("COORDINATE_TYPE", Agros2DGenerator::coordinateTypeStringEnum(coordinateType).toStdString());
    functionTemplate->SetValue("LINEARITY_TYPE", Agros2DGenerator::linearityTypeStringEnum(linearityType).toStdString());
    functionTemplate->SetValue("ANALYSIS_TYPE", Agros2DGenerator::analysisTypeStringEnum(analysisType).toStdString());

    QString fullName = QString("%1_ext_function_%2_%3_%4_%5").
            arg(QString::fromStdString(m_module->general_field().id())).
            arg(analysisTypeToStringKey(analysisType)).
            arg(coordinateTypeToStringKey(coordinateType)).
            arg(linearityTypeToStringKey(linearityType)).
            arg(QString::fromStdString(function.shortname()));

    if(linearize)
        fullName.append("_linearized");

    functionTemplate->SetValue("VALUE_FUNCTION_FULL_NAME", fullName.toStdString());

    if(linearize)
        functionTemplate->SetValue("IS_LINEARIZED", "true");
    else
        functionTemplate->SetValue("IS_LINEARIZED", "false");

    ParserModuleInfo pmi(*m_module, analysisType, coordinateType, linearityType);
    QString dependence("0");
//    if (linearityType != LinearityType_Linear)
//        dependence = pmi.specialFunctionNonlinearExpression(QString::fromStdString(function.id()));

    // if linearized, we use dependence on allready calculated values form previous time level or weakly coupled source field
    if(linearize)
        dependence = Parser::parseLinearizeDependence(pmi, dependence);
    else
        dependence = Parser::parseWeakFormExpression(pmi, dependence);

    functionTemplate->SetValue("DEPENDENCE", dependence.toStdString());

    foreach(XMLModule::weakform_volume weakForm, m_module->volume().weakforms_volume().weakform_volume())
    {
        if(QString::fromStdString(weakForm.analysistype()) == analysisTypeToStringKey(analysisType))
        {
            foreach(XMLModule::quantity quantity, function.quantity())
            {
                QString section;

                foreach(XMLModule::quantity quantDepend, weakForm.quantity())
                {
                    if(quantDepend.id() == quantity.id())
                    {
                        if(quantDepend.nonlinearity_axi().present() || quantDepend.nonlinearity_planar().present())
                            section = "PARAMETERS_NONLINEAR";
                        else
                            section = "PARAMETERS_LINEAR";
                        break;
                    }
                }

                ctemplate::TemplateDictionary *functionParameters = functionTemplate->AddSectionDictionary(section.toStdString());
                for(int i = 0; i < m_module->volume().quantity().size(); i++)
                {
                    if(m_module->volume().quantity().at(i).id() == quantity.id())
                    {
                        functionParameters->SetValue("PARAMETER_NAME", m_module->volume().quantity().at(i).shortname().get().c_str());
                        functionParameters->SetValue("PARAMETER_ID", m_module->volume().quantity().at(i).id().c_str());
                        functionParameters->SetValue("PARAMETER_FULL_NAME", m_module->volume().quantity().at(i).id().c_str());
                        break;
                    }
                }
            }

            break;
        }
    }


    // todo: get rid of variants
    foreach(XMLModule::function_variant variant, function.function_variant())
    {
        QString expression = QString::fromStdString(variant.expr());

        // todo:
        //expression = parseWeakFormExpression(analysisType, coordinateType, linearityType, expression, false, false);
        functionTemplate->SetValue("EXPR", expression.toStdString());
    }

}

//void Agros2DGeneratorModule::generateSpecialFunction(XMLModule::function function, AnalysisType analysisType, LinearityType linearityType, CoordinateType coordinateType, ctemplate::TemplateDictionary &output)
//{
//    ctemplate::TemplateDictionary *functionTemplate = output.AddSectionDictionary("SPECIAL_FUNCTION_SOURCE");
//    functionTemplate->SetValue("SPECIAL_FUNCTION_NAME", function.shortname());
//    functionTemplate->SetValue("SPECIAL_FUNCTION_ID", function.id());
//    functionTemplate->SetValue("COORDINATE_TYPE", Agros2DGenerator::coordinateTypeStringEnum(coordinateType).toStdString());
//    functionTemplate->SetValue("LINEARITY_TYPE", Agros2DGenerator::linearityTypeStringEnum(linearityType).toStdString());
//    functionTemplate->SetValue("ANALYSIS_TYPE", Agros2DGenerator::analysisTypeStringEnum(analysisType).toStdString());

//    QString fullName = QString("%1_ext_function_%2_%3_%4_%5").
//            arg(QString::fromStdString(m_module->general_field().id())).
//            arg(analysisTypeToStringKey(analysisType)).
//            arg(coordinateTypeToStringKey(coordinateType)).
//            arg(linearityTypeToStringKey(linearityType)).
//            arg(QString::fromStdString(function.shortname()));

//    functionTemplate->SetValue("SPECIAL_EXT_FUNCTION_FULL_NAME", fullName.toStdString());
//    functionTemplate->SetValue("FROM", function.bound_low().present() ? function.bound_low().get() : "-1");
//    functionTemplate->SetValue("TO", function.bound_hi().present() ? function.bound_hi().get() : "1");
//    functionTemplate->SetValue("TYPE", function.type());

//    ParserModuleInfo pmi(*m_module, analysisType, coordinateType, linearityType);
//    QString dependence("0");
//    if(linearityType != LinearityType_Linear)
//        dependence = pmi.specialFunctionNonlinearExpression(QString::fromStdString(function.id()));

//    dependence = Parser::parseWeakFormExpression(pmi, dependence);

//    functionTemplate->SetValue("DEPENDENCE", dependence.toStdString());
//    functionTemplate->SetValue("INTERPOLATION_COUNT", function.interpolation_count().present() ? function.interpolation_count().get() : "0");
//    if(function.extrapolate_low().present())
//    {
//        functionTemplate->SetValue("EXTRAPOLATE_LOW_PRESENT", "true");
//        functionTemplate->SetValue("EXTRAPOLATE_LOW", function.extrapolate_low().get());
//    }
//    else
//    {
//        functionTemplate->SetValue("EXTRAPOLATE_LOW_PRESENT", "false");
//        functionTemplate->SetValue("EXTRAPOLATE_LOW", "-123456");
//    }
//    if(function.extrapolate_hi().present())
//    {
//        functionTemplate->SetValue("EXTRAPOLATE_HI_PRESENT", "true");
//        functionTemplate->SetValue("EXTRAPOLATE_HI", function.extrapolate_hi().get());
//    }
//    else
//    {
//        functionTemplate->SetValue("EXTRAPOLATE_HI_PRESENT", "false");
//        functionTemplate->SetValue("EXTRAPOLATE_HI", "-123456");
//    }
//    QString selectedVariant("no_variant");
//    if(function.switch_combo().present())
//    {
//        foreach(XMLModule::gui gui, m_module->preprocessor().gui())
//        {
//            if(gui.type() == "volume")
//            {
//                foreach(XMLModule::group group, gui.group())
//                {
//                    foreach(XMLModule::switch_combo switch_combo, group.switch_combo())
//                    {
//                        if(switch_combo.id() == function.switch_combo().get())
//                        {
//                            selectedVariant = QString::fromStdString(switch_combo.implicit_option());
//                        }
//                    }
//                }
//            }
//        }
//    }
//    functionTemplate->SetValue("SELECTED_VARIANT", selectedVariant.toStdString().c_str());

//    foreach(XMLModule::quantity quantity, function.quantity())
//    {
//        ctemplate::TemplateDictionary *functionParameters = functionTemplate->AddSectionDictionary("PARAMETERS");
//        for(int i = 0; i < m_module->volume().quantity().size(); i++)
//        {
//            if(m_module->volume().quantity().at(i).id() == quantity.id())
//            {
//                functionParameters->SetValue("PARAMETER_NAME", m_module->volume().quantity().at(i).shortname().get().c_str());
//                functionParameters->SetValue("PARAMETER_ID", m_module->volume().quantity().at(i).id().c_str());
//                functionParameters->SetValue("PARAMETER_FULL_NAME", m_module->volume().quantity().at(i).id().c_str());
//                break;
//            }
//        }
//    }
//    foreach(XMLModule::function_variant variant, function.function_variant())
//    {
//        ctemplate::TemplateDictionary *functionVariant = functionTemplate->AddSectionDictionary("VARIANT");
//        functionVariant->SetValue("ID", variant.switch_value().present() ? variant.switch_value().get().c_str() : "no_variant");
//        QString expression = QString::fromStdString(variant.expr());

//        // todo:
//        //expression = parseWeakFormExpression(analysisType, coordinateType, linearityType, expression, false, false);
//        functionVariant->SetValue("EXPR", expression.toStdString());
//    }
//}
