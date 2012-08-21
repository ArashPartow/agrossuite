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


#include "generator.h"
#include "hermes2d/module.h"

#include "parser/lex.h"
#include "parser/tree.h"

const QString GENERATOR_TEMPLATEROOT = "generator/templates";
const QString GENERATOR_PLUGINROOT = "weakform_new/plugins/";

Agros2DGenerator::Agros2DGenerator(int &argc, char **argv) : QCoreApplication(argc, argv)
{
    /*
    QString expr = "0.5  * (dx1 * dx1 + dy1 * dy1)   ";

    LexicalAnalyser lex;
    lex.addVariable("dx1");
    lex.addVariable("dy1");
    lex.addVariable("epsr");
    lex.addVariable("EPS0");

    lex.setExpression(expr);
    lex.printTokens();
    */
    /*
    double dx1 = 2;
    double dy1 = 3;
    double el_epsr = 5;
    double eps0 = 4;

    SyntacticAnalyser synt;
    synt.m_variableMap.insert("dx1", &dx1);
    synt.m_variableMap.insert("dy1", &dy1);
    synt.m_variableMap.insert("epsr", &el_epsr);
    synt.m_variableMap.insert("EPS0", &eps0);
    synt.parse(lex.tokens());
    std::cout << synt.evalTree();
    */
}

void Agros2DGenerator::run()
{
    // create directory
    QDir root(QApplication::applicationDirPath());
    root.mkpath(GENERATOR_PLUGINROOT);

    QMap<QString, QString> modules = availableModules();

    foreach (QString moduleId, modules.keys())
    {
        // read module
        std::auto_ptr<XMLModule::module> module_xsd = XMLModule::module_((datadir().toStdString() + MODULEROOT.toStdString() + "/" + moduleId.toStdString() + ".xml").c_str());
        XMLModule::module *module = module_xsd.get();

        // generate project file
        root.mkpath(GENERATOR_PLUGINROOT + "/" + moduleId);

        generatePluginProjectFile(module);
        generatePluginInterfaceFiles(module);
        generatePluginFilterFiles(module);
        generatePluginWeakFormSourceFiles(module);
        generatePluginWeakFormHeaderFiles(module);
    }

    exit(0);
}

void Agros2DGenerator::generatePluginProjectFile(XMLModule::module *module)
{
    QString id = QString::fromStdString(module->general().id());
    QString header = id.toLower();

    ctemplate::TemplateDictionary output("output");
    output.SetValue("ID", id.toStdString());

    // expand template
    std::string text;
    ctemplate::ExpandTemplate(QString("%1/%2/module_pro.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

    // save to file
    writeStringContent(QString("%1/%2/%3/%3.pro").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));
}

void Agros2DGenerator::generatePluginInterfaceFiles(XMLModule::module *module)
{
    QString id = QString::fromStdString(module->general().id());

    ctemplate::TemplateDictionary output("output");

    output.SetValue("ID", module->general().id());
    output.SetValue("CLASS", (id.left(1).toUpper() + id.right(id.length() - 1)).toStdString());

    std::string text;

    // header - expand template
    ctemplate::ExpandTemplate(QString("%1/%2/interface_h.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

    // header - save to file
    writeStringContent(QString("%1/%2/%3/%3_interface.h").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));

    // source - expand template
    text.clear();
    foreach(XMLModule::weakform_volume weakform, module->volume().weakforms_volume().weakform_volume())
    {
        generateVolumeMatrixForm(weakform, output, module);
        generateVolumeVectorForm(weakform, output, module);
    }

    foreach(XMLModule::weakform_surface weakform, module->surface().weakforms_surface().weakform_surface())
    {
        foreach(XMLModule::boundary boundary, weakform.boundary())
        {
            generateSurfaceMatrixForm(boundary, output, module, weakform);
            generateSurfaceVectorForm(boundary, output, module, weakform);
            generateExactSolution(boundary, output, module, weakform);
        }
    }


    ctemplate::ExpandTemplate(QString("%1/%2/interface_cpp.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);
    // source - save to file
    writeStringContent(QString("%1/%2/%3/%3_interface.cpp").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));
}

void Agros2DGenerator::generatePluginWeakFormSourceFiles(XMLModule::module *module)
{
    QString id = QString::fromStdString(module->general().id());

    ctemplate::TemplateDictionary output("output");

    output.SetValue("ID", module->general().id());
    output.SetValue("CLASS", (id.left(1).toUpper() + id.right(id.length() - 1)).toStdString());

    std::string text;

    // source - expand template
    text.clear();
    foreach(XMLModule::weakform_volume weakform, module->volume().weakforms_volume().weakform_volume())
    {
        generateVolumeMatrixForm(weakform, output, module);
        generateVolumeVectorForm(weakform, output, module);
    }

    foreach(XMLModule::weakform_surface weakform, module->surface().weakforms_surface().weakform_surface())
    {
        foreach(XMLModule::boundary boundary, weakform.boundary())
        {

            generateSurfaceMatrixForm(boundary, output, module, weakform);
            generateSurfaceVectorForm(boundary, output, module, weakform);
            generateExactSolution(boundary, output, module, weakform);
        }
    }

    ctemplate::ExpandTemplate(QString("%1/%2/weakform_cpp.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

    // source - save to file
    writeStringContent(QString("%1/%2/%3/%3_weakform.cpp").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));
}

void Agros2DGenerator::generatePluginWeakFormHeaderFiles(XMLModule::module *module)
{
    QString id = QString::fromStdString(module->general().id());

    ctemplate::TemplateDictionary output("output");

    output.SetValue("ID", module->general().id());
    output.SetValue("CLASS", (id.left(1).toUpper() + id.right(id.length() - 1)).toStdString());

    std::string text;

    // header - expand template
    text.clear();
    foreach(XMLModule::weakform_volume weakform, module->volume().weakforms_volume().weakform_volume())
    {
        generateVolumeMatrixForm(weakform, output, module);
        generateVolumeVectorForm(weakform, output, module);
    }

    foreach(XMLModule::weakform_surface weakform, module->surface().weakforms_surface().weakform_surface())
    {
        foreach(XMLModule::boundary boundary, weakform.boundary())
        {

            generateSurfaceMatrixForm(boundary, output, module, weakform);
            generateSurfaceVectorForm(boundary, output, module, weakform);
            generateExactSolution(boundary, output, module, weakform);
        }
    }

    // header - expand template
    ctemplate::ExpandTemplate(QString("%1/%2/weakform_h.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

    // header - save to file
    writeStringContent(QString("%1/%2/%3/%3_weakform.h").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));
}

void Agros2DGenerator::generatePluginFilterFiles(XMLModule::module *module)
{
    QString id = QString::fromStdString(module->general().id());

    ctemplate::TemplateDictionary output("output");

    output.SetValue("ID", id.toStdString());
    output.SetValue("CLASS", (id.left(1).toUpper() + id.right(id.length() - 1)).toStdString());

    std::string text;

    // header - expand template
    ctemplate::ExpandTemplate(QString("%1/%2/filter_h.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

    for (int i = 0; i < module->postprocessor().localvariables().localvariable().size(); i++)
    {
        XMLModule::localvariable lv = module->postprocessor().localvariables().localvariable().at(i);

        for (int j = 0; j < lv.expression().size(); j++)
        {
            XMLModule::expression expr = lv.expression().at(j);

            foreach (CoordinateType coordinateType, coordinateTypeList())
            {
                QString exprVar;
                if (coordinateType == CoordinateType_Planar)
                {
                    if (expr.planar().present())
                        exprVar = QString::fromStdString(expr.planar().get());
                }
                else
                {
                    if (expr.axi().present())
                        exprVar = QString::fromStdString(expr.axi().get());
                }

                if (!exprVar.isEmpty())
                {
                    ctemplate::TemplateDictionary *expression = output.AddSectionDictionary("VARIABLE_SOURCE");

                    expression->SetValue("VARIABLE", lv.id());
                    expression->SetValue("ANALYSIS_TYPE", analysisTypeStringEnum(analysisTypeFromStringKey(QString::fromStdString(expr.analysistype()))).toStdString());
                    expression->SetValue("COORDINATE_TYPE", coordinateTypeStringEnum(coordinateType).toStdString());
                    expression->SetValue("EXPRESSION", parsePostprocessorExpression(module, analysisTypeFromStringKey(QString::fromStdString(expr.analysistype())), coordinateType, exprVar).toStdString());
                }
            }
        }
    }

    // header - save to file
    writeStringContent(QString("%1/%2/%3/%3_filter.h").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));

    // source - expand template
    text.clear();
    ctemplate::ExpandTemplate(QString("%1/%2/filter_cpp.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

    // source - save to file
    writeStringContent(QString("%1/%2/%3/%3_filter.cpp").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));
}

int Agros2DGenerator::numberOfSolutions(XMLModule::analyses analyses, AnalysisType analysisType)
{
    for (int i = 0; i < analyses.analysis().size(); i++)
    {
        XMLModule::analysis analysis = analyses.analysis().at(i);
        if (analysis.id() == analysisTypeToStringKey(analysisType).toStdString())
            return analysis.solutions();
    }

    return -1;
}

QString Agros2DGenerator::parsePostprocessorExpression(XMLModule::module *module, AnalysisType analysisType, CoordinateType coordinateType, const QString &expr)
{
    int numOfSol = numberOfSolutions(module->general().analyses(), analysisType);

    LexicalAnalyser lex;

    // coordinates
    if (coordinateType == CoordinateType_Planar)
    {
        lex.addVariable(QString("x"));
        lex.addVariable(QString("y"));
    }
    else
    {
        lex.addVariable(QString("r"));
        lex.addVariable(QString("z"));
    }

    // functions
    for (int i = 1; i < numOfSol + 1; i++)
    {
        lex.addVariable(QString("value%1").arg(i));
        if (coordinateType == CoordinateType_Planar)
        {
            lex.addVariable(QString("dx%1").arg(i));
            lex.addVariable(QString("dy%1").arg(i));
        }
        else
        {
            lex.addVariable(QString("dr%1").arg(i));
            lex.addVariable(QString("dz%1").arg(i));
        }
    }

    // constants
    lex.addVariable("PI");
    lex.addVariable("f");
    for (int i = 0; i < module->constants().constant().size(); i++)
    {
        XMLModule::constant cnst = module->constants().constant().at(i);
        lex.addVariable(QString::fromStdString(cnst.id()));
    }

    // variables
    for (int i = 0; i < module->volume().quantity().size(); i++)
    {
        XMLModule::quantity quantity = module->volume().quantity().at(i);

        if (quantity.shortname().present())
            lex.addVariable(QString::fromStdString(quantity.shortname().get()));
    }

    try
    {
        lex.setExpression(expr);

        // replace tokens
        QString exprCpp;
        foreach (Token token, lex.tokens())
        {
            QString repl = token.toString();

            bool isReplaced = false;

            // coordinates
            if (coordinateType == CoordinateType_Planar)
            {
                if (repl == "x") { exprCpp += "x[i]"; isReplaced = true; }
                if (repl == "y") { exprCpp += "y[i]"; isReplaced = true; }
            }
            else
            {
                if (repl == "r") { exprCpp += "x[i]"; isReplaced = true; }
                if (repl == "z") { exprCpp += "y[i]"; isReplaced = true; }
            }

            // constants
            if (repl == "M_PI") { exprCpp += "M_PI"; isReplaced = true; }
            if (repl == "f") { exprCpp += "Util::problem()->config()->frequency()"; isReplaced = true; }
            for (int i = 0; i < module->constants().constant().size(); i++)
            {
                XMLModule::constant cnst = module->constants().constant().at(i);
                if (repl == QString::fromStdString(cnst.id())) { exprCpp += QString::number(cnst.value()); isReplaced = true; }
            }

            // functions
            for (int i = 1; i < numOfSol + 1; i++)
            {
                if (repl == QString("value%1").arg(i)) { exprCpp += QString("value[%1][i]").arg(i-1); isReplaced = true; }
                if (coordinateType == CoordinateType_Planar)
                {
                    if (repl == QString("dx%1").arg(i)) { exprCpp += QString("dudx[%1][i]").arg(i-1); isReplaced = true; }
                    if (repl == QString("dy%1").arg(i)) { exprCpp += QString("dudy[%1][i]").arg(i-1); isReplaced = true; }
                }
                else
                {
                    if (repl == QString("dr%1").arg(i)) { exprCpp += QString("dudx[%1][i]").arg(i-1); isReplaced = true; }
                    if (repl == QString("dz%1").arg(i)) { exprCpp += QString("dudy[%1][i]").arg(i-1); isReplaced = true; }
                }
            }

            // variables
            for (int i = 0; i < module->volume().quantity().size(); i++)
            {
                XMLModule::quantity quantity = module->volume().quantity().at(i);

                if (quantity.shortname().present())
                    if (repl == QString::fromStdString(quantity.shortname().get()))
                    {
                        if (!quantity.dependence().present())
                            // linear material
                            exprCpp += QString("material->value(\"%1\").number()").arg(QString::fromStdString(quantity.shortname().get()));
                        else
                            // nonlinear material
                            exprCpp += QString("material->value(%1).value(%2)").
                                    arg(QString::fromStdString(quantity.shortname().get())).
                                    arg(parsePostprocessorExpression(module, analysisType, coordinateType, QString::fromStdString(quantity.dependence().get())));

                        isReplaced = true;
                    }
            }

            // operators and other
            if (!isReplaced)
               exprCpp += repl;
        }

        return exprCpp;
    }
    catch (ParserException e)
    {
        qDebug() << e.toString();

        return "";
    }
}

QString Agros2DGenerator::getExpression(CoordinateType coordinateType, LinearityType linearityType, XMLModule::matrix_form matrix_form)
{
    QString expression;

    if((linearityType == LinearityType_Linear) && (coordinateType == CoordinateType_Planar))
    {
        expression = matrix_form.planar_linear().c_str();
    }

    if((linearityType == LinearityType_Newton) && (coordinateType == CoordinateType_Planar))
    {
        expression = matrix_form.planar_newton().c_str();
    }

    if((linearityType == LinearityType_Linear) && (coordinateType == CoordinateType_Axisymmetric))
    {
        expression = matrix_form.axi_linear().c_str();
    }
    if((linearityType == LinearityType_Newton) && (coordinateType == CoordinateType_Axisymmetric))
    {
        expression = matrix_form.axi_newton().c_str();
    }

    return expression;
}

QString Agros2DGenerator::getExpression(CoordinateType coordinateType, LinearityType linearityType, XMLModule::vector_form vector_form)
{
    QString expression;

    if((linearityType == LinearityType_Linear) && (coordinateType == CoordinateType_Planar))
    {
        expression = vector_form.planar_linear().c_str();
    }

    if((linearityType == LinearityType_Newton) && (coordinateType == CoordinateType_Planar))
    {
        expression = vector_form.planar_newton().c_str();
    }

    if((linearityType == LinearityType_Linear) && (coordinateType == CoordinateType_Axisymmetric))
    {
        expression = vector_form.axi_linear().c_str();
    }
    if((linearityType == LinearityType_Newton) && (coordinateType == CoordinateType_Axisymmetric))
    {
        expression = vector_form.axi_newton().c_str();
    }

    return expression;
}

QString Agros2DGenerator::getExpression(CoordinateType coordinateType, LinearityType linearityType, XMLModule::essential_form essential_form)
{
    QString expression;

    if((linearityType == LinearityType_Linear) && (coordinateType == CoordinateType_Planar))
    {
        expression = essential_form.planar_linear().c_str();
    }

    if((linearityType == LinearityType_Newton) && (coordinateType == CoordinateType_Planar))
    {
        expression = essential_form.planar_newton().c_str();
    }

    if((linearityType == LinearityType_Linear) && (coordinateType == CoordinateType_Axisymmetric))
    {
        expression = essential_form.axi_linear().c_str();
    }
    if((linearityType == LinearityType_Newton) && (coordinateType == CoordinateType_Axisymmetric))
    {
        expression = essential_form.axi_newton().c_str();
    }

    return expression;
}


void Agros2DGenerator::generateVolumeMatrixForm(XMLModule::weakform_volume weakform, ctemplate::TemplateDictionary &output, XMLModule::module *module)
{
    foreach(XMLModule::matrix_form matrix_form, weakform.matrix_form())
    {
        foreach(LinearityType linearityType, linearityTypeList())
        {
            foreach (CoordinateType coordinateType, coordinateTypeList())
            {
                QString expression = getExpression(coordinateType, linearityType, matrix_form);

                if(expression != "")
                {
                    QString functionName = QString("%1_%2_%3_%4_%5").
                            arg("volume_matrix").
                            arg(QString::fromStdString(module->general().id())).
                            arg(QString::fromStdString(weakform.analysistype())).
                            arg(coordinateTypeToStringKey(coordinateType)).
                            arg(linearityTypeToStringKey(linearityType));

                    ctemplate::TemplateDictionary *field;
                    field = output.AddSectionDictionary("MATRIX_VOL_SOURCE");
                    field->SetValue("FUNCTION_NAME", functionName.toStdString());
                    field->SetValue("COORDINATE_TYPE",  coordinateTypeStringEnum(coordinateType).toStdString());
                    field->SetValue("LINEARITY_TYPE",linearityTypeStringEnum(linearityType).toStdString());
                    field->SetValue("ANALYSIS_TYPE", weakform.analysistype());
                    field->SetValue("ROW_INDEX", QString::number(matrix_form.i()).toStdString());
                    field->SetValue("COLUMN_INDEX", QString::number(matrix_form.j()).toStdString());
                    field->SetValue("MODULE_ID", module->general().id());

                    field = output.AddSectionDictionary("SOURCE");
                    field->SetValue("FUNCTION_NAME", functionName.toStdString());
                }
            }
        }
    }

}

void Agros2DGenerator::generateVolumeVectorForm(XMLModule::weakform_volume weakform, ctemplate::TemplateDictionary &output, XMLModule::module *module)
{
    foreach(XMLModule::vector_form vector_form, weakform.vector_form())
    {
        foreach(LinearityType linearityType, linearityTypeList())
        {
            foreach (CoordinateType coordinateType, coordinateTypeList())
            {
                QString expression = getExpression(coordinateType, linearityType, vector_form);
                if(expression != "")
                {
                    QString functionName = QString("%1_%2_%3_%4_%5").
                            arg("volume_vector").
                            arg(QString::fromStdString(module->general().id())).
                            arg(QString::fromStdString(weakform.analysistype())).
                            arg(coordinateTypeToStringKey(coordinateType)).
                            arg(linearityTypeToStringKey(linearityType));


                    ctemplate::TemplateDictionary *field;
                    field = output.AddSectionDictionary("VECTOR_VOL_SOURCE"); // parameter output
                    field->SetValue("FUNCTION_NAME", functionName.toStdString());
                    field->SetValue("COORDINATE_TYPE",  coordinateTypeStringEnum(coordinateType).toStdString());
                    field->SetValue("LINEARITY_TYPE",linearityTypeStringEnum(linearityType).toStdString());
                    field->SetValue("ANALYSIS_TYPE", weakform.analysistype());
                    field->SetValue("ROW_INDEX", QString::number(vector_form.i()).toStdString());
                    field->SetValue("COLUMN_INDEX", QString::number(vector_form.j()).toStdString());
                    field->SetValue("MODULE_ID", module->general().id());
                    field = output.AddSectionDictionary("SOURCE");
                    field->SetValue("FUNCTION_NAME", functionName.toStdString());
                }
            }
        }
    }
}

void Agros2DGenerator::generateSurfaceMatrixForm(XMLModule::boundary boundary, ctemplate::TemplateDictionary &output, XMLModule::module *module, XMLModule::weakform_surface weakform)
{
    foreach(XMLModule::matrix_form matrix_form, boundary.matrix_form())
    {
        foreach(LinearityType linearityType, linearityTypeList())
        {
            foreach (CoordinateType coordinateType, coordinateTypeList())
            {
                QString expression = getExpression(coordinateType, linearityType, matrix_form);
                if(expression != "")
                {
                    QString functionName = QString("%1_%2_%3_%4_%5_%6").
                            arg("surface_matrix").
                            arg(QString::fromStdString(module->general().id())).
                            arg(QString::fromStdString(weakform.analysistype())).
                            arg(coordinateTypeToStringKey(coordinateType)).
                            arg(linearityTypeToStringKey(linearityType)).
                            arg( QString::fromStdString(boundary.id()));

                    ctemplate::TemplateDictionary *field;
                    field = output.AddSectionDictionary("MATRIX_SURF_SOURCE");
                    field->SetValue("FUNCTION_NAME", functionName.toStdString());
                    field->SetValue("BOUNDARY_TYPE", boundary.id());
                    field->SetValue("COORDINATE_TYPE",  coordinateTypeStringEnum(coordinateType).toStdString());
                    field->SetValue("LINEARITY_TYPE",linearityTypeStringEnum(linearityType).toStdString());
                    field->SetValue("ANALYSIS_TYPE", weakform.analysistype());
                    field->SetValue("ROW_INDEX", QString::number(matrix_form.i()).toStdString());
                    field->SetValue("COLUMN_INDEX", QString::number(matrix_form.j()).toStdString());
                    field->SetValue("MODULE_ID", module->general().id());
                    field = output.AddSectionDictionary("SOURCE");
                    field->SetValue("FUNCTION_NAME", functionName.toStdString());
                }
            }
        }
    }
}

void Agros2DGenerator::generateSurfaceVectorForm(XMLModule::boundary boundary, ctemplate::TemplateDictionary &output, XMLModule::module *module, XMLModule::weakform_surface weakform)
{
    foreach(XMLModule::vector_form vector_form, boundary.vector_form())
    {
        foreach(LinearityType linearityType, linearityTypeList())
        {
            foreach (CoordinateType coordinateType, coordinateTypeList())
            {
                QString expression = getExpression(coordinateType, linearityType, vector_form);
                if(expression != "")
                {
                    QString functionName = QString("%1_%2_%3_%4_%5_%6").
                            arg("surface_vector").
                            arg(QString::fromStdString(module->general().id())).
                            arg(QString::fromStdString(weakform.analysistype())).
                            arg(coordinateTypeToStringKey(coordinateType)).
                            arg(linearityTypeToStringKey(linearityType)).
                            arg( QString::fromStdString(boundary.id()));

                    ctemplate::TemplateDictionary *field;
                    field = output.AddSectionDictionary("VECTOR_SURF_SOURCE");
                    field->SetValue("FUNCTION_NAME", functionName.toStdString());
                    field->SetValue("BOUNDARY_TYPE", boundary.id());
                    field->SetValue("COORDINATE_TYPE",  coordinateTypeStringEnum(coordinateType).toStdString());
                    field->SetValue("LINEARITY_TYPE",linearityTypeStringEnum(linearityType).toStdString());
                    field->SetValue("ANALYSIS_TYPE", weakform.analysistype());
                    field->SetValue("ROW_INDEX", QString::number(vector_form.i()).toStdString());
                    field->SetValue("COLUMN_INDEX", QString::number(vector_form.j()).toStdString());
                    field->SetValue("MODULE_ID", module->general().id());
                    field = output.AddSectionDictionary("SOURCE");
                    field->SetValue("FUNCTION_NAME", functionName.toStdString());
                }
            }
        }
    }
}

void Agros2DGenerator::generateExactSolution(XMLModule::boundary boundary, ctemplate::TemplateDictionary &output, XMLModule::module *module, XMLModule::weakform_surface weakform)
{
    foreach(XMLModule::essential_form essential_form, boundary.essential_form())
    {
        foreach(LinearityType linearityType, linearityTypeList())
        {
            foreach (CoordinateType coordinateType, coordinateTypeList())
            {
                QString expression = getExpression(coordinateType, linearityType, essential_form);
                if(expression != "")
                {
                    QString functionName = QString("%1_%2_%3_%4_%5_%6").
                            arg("exact_source").
                            arg(QString::fromStdString(module->general().id())).
                            arg(QString::fromStdString(weakform.analysistype())).
                            arg(coordinateTypeToStringKey(coordinateType)).
                            arg(linearityTypeToStringKey(linearityType)).
                            arg( QString::fromStdString(boundary.id()));

                    ctemplate::TemplateDictionary *field;
                    field = output.AddSectionDictionary("EXACT_SOURCE");
                    field->SetValue("FUNCTION_NAME", functionName.toStdString());
                    field->SetValue("BOUNDARY_TYPE", boundary.id());
                    field->SetValue("COORDINATE_TYPE",  coordinateTypeStringEnum(coordinateType).toStdString());
                    field->SetValue("LINEARITY_TYPE",linearityTypeStringEnum(linearityType).toStdString());
                    field->SetValue("ANALYSIS_TYPE", weakform.analysistype());
                    field->SetValue("ROW_INDEX", QString::number(essential_form.i()).toStdString());
                    field->SetValue("COLUMN_INDEX", "0");
                    field->SetValue("MODULE_ID", module->general().id());
                    field = output.AddSectionDictionary("SOURCE");
                    field->SetValue("FUNCTION_NAME", functionName.toStdString());
                }
            }
        }
    }
}

