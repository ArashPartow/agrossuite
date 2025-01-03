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

#ifndef GENERATOR_MODULE_H
#define GENERATOR_MODULE_H

#include "generator.h"

class Parser;
struct FormInfo;

class Agros2DGeneratorModule : public Agros2DGeneratorBase
{
public:
    Agros2DGeneratorModule(const QString &moduleId);
    ~Agros2DGeneratorModule();

    void prepareWeakFormsOutput();
    void deleteWeakFormOutput();
    void generatePluginProjectFile();
    void generatePluginFilterFiles();
    void generatePluginForceFiles();
    void generatePluginLocalPointFiles();
    void generatePluginSurfaceIntegralFiles();
    void generatePluginVolumeIntegralFiles();
    void generatePluginInterfaceFiles();
    void generatePluginWeakFormFiles();
    void generatePluginDocumentationFiles();
    void generatePluginEquations();

private:
    std::unique_ptr<XMLModule::module> module_xsd;
    XMLModule::field *m_module;
    QString m_docString;
    QStringList m_names;
    QString m_id;
    // dictionary for variables used in weakforms
    QHash<QString, QString> m_volumeVariables;
    QHash<QString, QString> m_surfaceVariables;

    void getNames(const QString &moduleId);
    void generatePluginWeakFormSourceFiles();
    void generatePluginWeakFormHeaderFiles();

    void generateWeakForms(ctemplate::TemplateDictionary &output);

    //ToDo: make up better names
    template <typename WeakForm>
    void generateFormExpression(FormInfo form, LinearityType linearityType, CoordinateType coordinateType, ctemplate::TemplateDictionary &output, QString formType, WeakForm weakform, bool isSurface);
    template <typename WeakForm>
    ctemplate::TemplateDictionary *generateVolumeVariables(LinearityType linearityType, CoordinateType coordinateType, ctemplate::TemplateDictionary &output, WeakForm weakform, QString weakFormType);
    template <typename WeakForm>
    ctemplate::TemplateDictionary *generateSurfaceVariables(LinearityType linearityType, CoordinateType coordinateType, ctemplate::TemplateDictionary &output, WeakForm weakform, QString weakFormType, XMLModule::boundary *boundary);

    void createFilterExpression(ctemplate::TemplateDictionary &output, const QString &variable, AnalysisType analysisType, CoordinateType coordinateType, PhysicFieldVariableComp physicFieldVariableComp, const QString &expr);
    void createLocalValueExpression(ctemplate::TemplateDictionary &output, const QString &variable, AnalysisType analysisType, CoordinateType coordinateType, const QString &exprScalar, const QString &exprVectorX, const QString &exprVectorY);
    void createIntegralExpression(ctemplate::TemplateDictionary &output, const QString &section, const QString &variable, AnalysisType analysisType, CoordinateType coordinateType, const QString &expr, int pos);

    QString generateDocWeakFormExpression(AnalysisType analysisType, CoordinateType coordinateType, LinearityType linearityType, const QString &expr, bool includeVariables = true);
    QString underline(QString text, char symbol);
    QString capitalize(QString text);
    QString createTable(QList<QStringList>);

    QMap<QString, int> quantityOrdering;
    QMap<QString, bool> quantityIsNonlinear;
    QMap<QString, int> functionOrdering;

    ctemplate::TemplateDictionary* m_output;
};

#endif // GENERATOR_MODULE_H
