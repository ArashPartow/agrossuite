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

#include "parser/lex.h"
#include "parser.h"
#include "parser_instance.h"

// todo: m_parserModuleInfoSource has to be initialized by something
// todo: this is not good, but implicit initialization is not possible
ParserInstance::ParserInstance(ParserModuleInfo pmi) : m_parserModuleInfo(pmi), m_parserModuleInfoSource(pmi)
{

}

ParserInstance::ParserInstance(ParserModuleInfo pmiSource, ParserModuleInfo pmi) : m_parserModuleInfo(pmi), m_parserModuleInfoSource(pmiSource)
{

}

QString ParserInstance::parse(QString expr)
{
    try
    {
        m_lexicalAnalyser->setExpression(expr);
        QString exprCpp = m_lexicalAnalyser->replaceVariables(m_dict);

        // TODO: move from lex
        exprCpp = m_lexicalAnalyser->replaceOperatorByFunction(exprCpp);

        return exprCpp;
    }
    catch (ParserException e)
    {
        qWarning() << (QString("%1 in module %2").arg(e.toString()).arg(m_parserModuleInfo.id).toLatin1());

        return "";
    }

}

ParserInstanceWeakForm::ParserInstanceWeakForm(ParserModuleInfo pmi) : ParserInstance(pmi)
{
    m_lexicalAnalyser = Parser::weakFormLexicalAnalyser(m_parserModuleInfo);

    addBasicWeakformTokens(pmi);
    addConstants(m_parserModuleInfo);
    addPreviousSolWeakform(pmi.numSolutions);

    // todo: simplification: all possible (and impossible) source tokens generated
    addCouplingWeakformTokens(4);
    addVolumeVariablesWeakform(pmi, false);
    addSurfaceVariables();
}

ParserInstanceErrorExpression::ParserInstanceErrorExpression(ParserModuleInfo pmi, bool withVariables) : ParserInstance(pmi)
{
    m_lexicalAnalyser = Parser::weakFormLexicalAnalyser(m_parserModuleInfo);

    addBasicWeakformTokens(pmi);
    addConstants(m_parserModuleInfo);
    addPreviousSolErrorCalculation();

    if (withVariables)
    {
        addVolumeVariablesErrorCalculation();
        addSurfaceVariables();
    }
}

ParserInstanceLinearizeDependence::ParserInstanceLinearizeDependence(ParserModuleInfo pmi) : ParserInstance(pmi)
{
    m_lexicalAnalyser = Parser::weakFormLexicalAnalyser(m_parserModuleInfo);

    addBasicWeakformTokens(pmi);
    addConstants(m_parserModuleInfo);
    addPreviousSolLinearizeDependence();
}

ParserInstanceWeakformCheck::ParserInstanceWeakformCheck(ParserModuleInfo pmi) : ParserInstance(pmi)
{
    m_lexicalAnalyser =  Parser::weakFormLexicalAnalyser(m_parserModuleInfo);

    addBasicWeakformTokens(pmi);
    addConstants(m_parserModuleInfo);
    addPreviousSolWeakform(pmi.numSolutions);
    addVolumeVariablesWeakform(pmi, false);
}

ParserInstancePostprocessorExpression::ParserInstancePostprocessorExpression(ParserModuleInfo pmi, bool withVariables) : ParserInstance(pmi)
{
    m_lexicalAnalyser = Parser::postprocessorLexicalAnalyser(m_parserModuleInfo);

    addPostprocessorBasic();
    addConstants(m_parserModuleInfo);

    if(withVariables)
    {
        addPostprocessorVariables();
    }
}

ParserInstanceFilterExpression::ParserInstanceFilterExpression(ParserModuleInfo pmi, bool withVariables) : ParserInstance(pmi)
{
    m_lexicalAnalyser = Parser::postprocessorLexicalAnalyser(m_parserModuleInfo);

    addPostprocessorBasic();
    addConstants(m_parserModuleInfo);

    if(withVariables)
    {
        addFilterVariables();
    }
}

ParserInstanceCouplingWeakForm::ParserInstanceCouplingWeakForm(ParserModuleInfo pmiSource, ParserModuleInfo pmi)
    :ParserInstance(pmiSource, pmi)
{
    m_lexicalAnalyser = Parser::weakFormCouplingLexicalAnalyser(m_parserModuleInfoSource, m_parserModuleInfo);

    addBasicWeakformTokens(pmi);
    addConstants(m_parserModuleInfo);
    addConstants(m_parserModuleInfoSource);
    addCouplingWeakformTokens(pmiSource.numSolutions);
    addPreviousSolWeakform(pmi.numSolutions + pmiSource.numSolutions);
    addVolumeVariablesWeakform(pmi, false);
    addVolumeVariablesWeakform(pmiSource, true);
}
