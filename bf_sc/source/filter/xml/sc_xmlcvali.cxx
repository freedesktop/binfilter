/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifdef _MSC_VER
#pragma hdrstop
#endif


#include "xmlcvali.hxx"
#include "xmlimprt.hxx"
#include "xmlconti.hxx"
#include "XMLConverter.hxx"

#include <bf_xmloff/nmspmap.hxx>
#include <bf_xmloff/xmlnmspe.hxx>
#include <bf_xmloff/XMLEventsImportContext.hxx>

#include <tools/debug.hxx>
namespace binfilter {

using namespace ::com::sun::star;
using namespace xmloff::token;

class ScXMLContentValidationContext : public SvXMLImportContext
{
    ::rtl::OUString sName;
    ::rtl::OUString sHelpTitle;
    ::rtl::OUString sHelpMessage;
    ::rtl::OUString sErrorTitle;
    ::rtl::OUString sErrorMessage;
    ::rtl::OUString sErrorMessageType;
    ::rtl::OUString sBaseCellAddress;
    ::rtl::OUString sCondition;
    sal_Bool        bAllowEmptyCell : 1;
    sal_Bool        bDisplayHelp : 1;
    sal_Bool        bDisplayError : 1;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

    void GetAlertStyle(const ::rtl::OUString& sMessageType, ::com::sun::star::sheet::ValidationAlertStyle& aAlertStyle);
    void SetFormulas(const ::rtl::OUString& sFormulas, ::rtl::OUString& sFormula1, ::rtl::OUString& sFormula2) const;
    void GetCondition(const ::rtl::OUString& sCondition, ::rtl::OUString& sFormula1, ::rtl::OUString& sFormula2,
        ::com::sun::star::sheet::ValidationType& aValidationType,
        ::com::sun::star::sheet::ConditionOperator& aOperator);

public:

    ScXMLContentValidationContext( ScXMLImport& rImport, USHORT nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList);

    virtual ~ScXMLContentValidationContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();

    void SetHelpMessage(const ::rtl::OUString& sTitle, const ::rtl::OUString& sMessage, const sal_Bool bDisplay);
    void SetErrorMessage(const ::rtl::OUString& sTitle, const ::rtl::OUString& sMessage, const ::rtl::OUString& sMessageType, const sal_Bool bDisplay);
    void SetErrorMacro(const ::rtl::OUString& sName, const sal_Bool bExecute);
};

class ScXMLHelpMessageContext : public SvXMLImportContext
{
    ::rtl::OUString sTitle;
    ::rtl::OUStringBuffer   sMessage;
    sal_Int32       nParagraphCount;
    sal_Bool        bDisplay : 1;

    ScXMLContentValidationContext* pValidationContext;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLHelpMessageContext( ScXMLImport& rImport, USHORT nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLContentValidationContext* pValidationContext);

    virtual ~ScXMLHelpMessageContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLErrorMessageContext : public SvXMLImportContext
{
    ::rtl::OUString sTitle;
    ::rtl::OUStringBuffer   sMessage;
    ::rtl::OUString sMessageType;
    sal_Int32       nParagraphCount;
    sal_Bool        bDisplay : 1;

    ScXMLContentValidationContext* pValidationContext;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLErrorMessageContext( ScXMLImport& rImport, USHORT nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLContentValidationContext* pValidationContext);

    virtual ~ScXMLErrorMessageContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLErrorMacroContext : public SvXMLImportContext
{
    ::rtl::OUString sName;
    sal_Bool        bExecute : 1;

    ScXMLContentValidationContext*  pValidationContext;
    SvXMLImportContextRef           xEventContext;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLErrorMacroContext( ScXMLImport& rImport, USHORT nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLContentValidationContext* pValidationContext);

    virtual ~ScXMLErrorMacroContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

//------------------------------------------------------------------

ScXMLContentValidationsContext::ScXMLContentValidationsContext( ScXMLImport& rInImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& /*xAttrList*/) :
    SvXMLImportContext( rInImport, nPrfx, rLName )
{
    // here are no attributes
}

ScXMLContentValidationsContext::~ScXMLContentValidationsContext()
{
}

SvXMLImportContext *ScXMLContentValidationsContext::CreateChildContext( USHORT nInPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetContentValidationsElemTokenMap();
    switch( rTokenMap.Get( nInPrefix, rLName ) )
    {
        case XML_TOK_CONTENT_VALIDATION:
            pContext = new ScXMLContentValidationContext( GetScImport(), nInPrefix, rLName, xAttrList);
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nInPrefix, rLName );

    return pContext;
}

void ScXMLContentValidationsContext::EndElement()
{
}

ScXMLContentValidationContext::ScXMLContentValidationContext( ScXMLImport& rInImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList) :
    SvXMLImportContext( rInImport, nPrfx, rLName ),
    sName(),
    sHelpTitle(),
    sHelpMessage(),
    sErrorTitle(),
    sErrorMessage(),
    sErrorMessageType(),
    sBaseCellAddress(),
    sCondition(),
    bAllowEmptyCell(sal_True),
    bDisplayHelp(sal_False),
    bDisplayError(sal_False)
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetContentValidationAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        ::rtl::OUString aLclLocalName;
        USHORT nLclPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLclLocalName );
        ::rtl::OUString sValue = xAttrList->getValueByIndex( i );

        switch( rAttrTokenMap.Get( nLclPrefix, aLclLocalName ) )
        {
            case XML_TOK_CONTENT_VALIDATION_NAME:
                sName = sValue;
            break;
            case XML_TOK_CONTENT_VALIDATION_CONDITION:
                sCondition = sValue;
            break;
            case XML_TOK_CONTENT_VALIDATION_BASE_CELL_ADDRESS:
                sBaseCellAddress = sValue;
            break;
            case XML_TOK_CONTENT_VALIDATION_ALLOW_EMPTY_CELL:
                if (IsXMLToken(sValue, XML_FALSE))
                    bAllowEmptyCell = sal_True;
            break;
        }
    }
}

ScXMLContentValidationContext::~ScXMLContentValidationContext()
{
}

SvXMLImportContext *ScXMLContentValidationContext::CreateChildContext( USHORT nInPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetContentValidationElemTokenMap();
    switch( rTokenMap.Get( nInPrefix, rLName ) )
    {
        case XML_TOK_CONTENT_VALIDATION_ELEM_HELP_MESSAGE:
            pContext = new ScXMLHelpMessageContext( GetScImport(), nInPrefix, rLName, xAttrList, this);
        break;
        case XML_TOK_CONTENT_VALIDATION_ELEM_ERROR_MESSAGE:
            pContext = new ScXMLErrorMessageContext( GetScImport(), nInPrefix, rLName, xAttrList, this);
        break;
        case XML_TOK_CONTENT_VALIDATION_ELEM_ERROR_MACRO:
            pContext = new ScXMLErrorMacroContext( GetScImport(), nInPrefix, rLName, xAttrList, this);
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nInPrefix, rLName );

    return pContext;
}

void ScXMLContentValidationContext::GetAlertStyle(const ::rtl::OUString& sMessageType, ::com::sun::star::sheet::ValidationAlertStyle& aAlertStyle)
{
    if (IsXMLToken(sMessageType, XML_MACRO))
        aAlertStyle = sheet::ValidationAlertStyle_MACRO;
    else if (IsXMLToken(sMessageType, XML_STOP))
        aAlertStyle = sheet::ValidationAlertStyle_STOP;
    else if (IsXMLToken(sMessageType, XML_WARNING))
        aAlertStyle = sheet::ValidationAlertStyle_WARNING;
    else if (IsXMLToken(sMessageType, XML_INFORMATION))
        aAlertStyle = sheet::ValidationAlertStyle_INFO;
}

void ScXMLContentValidationContext::SetFormulas(const ::rtl::OUString& sFormulas, ::rtl::OUString& sFormula1, ::rtl::OUString& sFormula2) const
{
    sal_Int32 i = 0;
    sal_Bool bString = sal_False;
    sal_Int32 nBrakes = 0;
    while ((sFormulas[i] != ',' || nBrakes > 0 || bString) && i < sFormulas.getLength())
    {
        if (sFormulas[i] == '(')
            nBrakes++;
        if (sFormulas[i] == ')')
            nBrakes--;
        if (sFormulas[i] == '"')
            bString = !bString;
        i++;
    }
    if (sFormulas[i] == ',')
    {
        sFormula1 = sFormulas.copy(0, i);
        sFormula2 = sFormulas.copy(i + 1);
    }
}

void ScXMLContentValidationContext::GetCondition(const ::rtl::OUString& sTempCondition, ::rtl::OUString& sFormula1, ::rtl::OUString& sFormula2,
        ::com::sun::star::sheet::ValidationType& aValidationType,
        ::com::sun::star::sheet::ConditionOperator& aOperator)
{
    ::rtl::OUString sLclCondition = sTempCondition;
    if (sLclCondition.getLength())
    {
        // ToDo: erase all blanks in the condition, but not in formulas or strings
        ::rtl::OUString scell_content(RTL_CONSTASCII_USTRINGPARAM("cell_content"));
        ::rtl::OUString scell_content_is_date(RTL_CONSTASCII_USTRINGPARAM("cell-content-is-date"));
        ::rtl::OUString scell_content_is_time(RTL_CONSTASCII_USTRINGPARAM("cell-content-is-time"));
        ::rtl::OUString scell_content_is_between(RTL_CONSTASCII_USTRINGPARAM("cell_content_is_between"));
        ::rtl::OUString scell_content_text_length(RTL_CONSTASCII_USTRINGPARAM("cell-content-text-length"));
        ::rtl::OUString scell_content_is_not_between(RTL_CONSTASCII_USTRINGPARAM("cell_content_is_not_between"));
        ::rtl::OUString scell_content_is_whole_number(RTL_CONSTASCII_USTRINGPARAM("cell-content-is-whole-number"));
        ::rtl::OUString scell_content_is_decimal_number(RTL_CONSTASCII_USTRINGPARAM("cell-content-is-decimal-number"));
        ::rtl::OUString scell_content_text_length_is_between(RTL_CONSTASCII_USTRINGPARAM("cell-content-text-length-is-between"));
        ::rtl::OUString scell_content_text_length_is_not_between(RTL_CONSTASCII_USTRINGPARAM("cell-content-text-length-is-not-between"));
        sal_Int32 i = 0;
        sal_Bool bAnd(sal_True);
        while (sLclCondition[i] != '(' && i < sLclCondition.getLength())
            i++;
        if (sLclCondition[i] == '(')
        {
            if (i != scell_content_text_length.getLength() &&
                i != scell_content_text_length_is_between.getLength() &&
                i != scell_content_text_length_is_not_between.getLength())
            {
                if (i == scell_content_is_time.getLength())
                {
                    ::rtl::OUString sTemp = sLclCondition.copy(0, i);
                    if (sTemp == scell_content_is_time)
                        aValidationType = sheet::ValidationType_TIME;
                    else
                        aValidationType = sheet::ValidationType_DATE;
                }
                else if (i == scell_content_is_whole_number.getLength())
                    aValidationType = sheet::ValidationType_WHOLE;
                else if (i == scell_content_is_decimal_number.getLength())
                    aValidationType = sheet::ValidationType_DECIMAL;
                sLclCondition = sLclCondition.copy(i + 2);
                ::rtl::OUString sTemp = sLclCondition.copy(0, 5);
                if (sTemp.compareToAscii(" and ") == 0)
                    sLclCondition = sLclCondition.copy(5);
                else
                    bAnd = sal_False;
            }
            if (sLclCondition.getLength() && bAnd)
            {
                i = 0;
                while (sLclCondition[i] != '(' && i < sLclCondition.getLength())
                    i++;
                if (sLclCondition[i] == '(')
                {
                    sLclCondition = sLclCondition.copy(i + 1);
                    if (i == scell_content_is_between.getLength() ||
                        i == scell_content_text_length_is_between.getLength())
                    {
                        if (i == scell_content_text_length_is_between.getLength())
                            aValidationType = sheet::ValidationType_TEXT_LEN;
                        aOperator = sheet::ConditionOperator_BETWEEN;
                        sLclCondition = sLclCondition.copy(0, sLclCondition.getLength() - 1);
                        SetFormulas(sLclCondition, sFormula1, sFormula2);
                    }
                    else if (i == scell_content_is_not_between.getLength() ||
                        i == scell_content_text_length_is_not_between.getLength())
                    {
                        if (i == scell_content_text_length_is_not_between.getLength())
                            aValidationType = sheet::ValidationType_TEXT_LEN;
                        aOperator = sheet::ConditionOperator_NOT_BETWEEN;
                        sLclCondition = sLclCondition.copy(0, sLclCondition.getLength() - 1);
                        SetFormulas(sLclCondition, sFormula1, sFormula2);
                    }
                    else if (i == scell_content.getLength() ||
                        i == scell_content_text_length.getLength())
                    {
                        if (i == scell_content_text_length.getLength())
                            aValidationType = sheet::ValidationType_TEXT_LEN;
                        sLclCondition = sLclCondition.copy(1);
                        switch (sLclCondition[0])
                        {
                            case '<' :
                            {
                                if (sLclCondition[1] == '=')
                                {
                                    aOperator = sheet::ConditionOperator_LESS_EQUAL;
                                    sLclCondition = sLclCondition.copy(2);
                                }
                                else
                                {
                                    aOperator = sheet::ConditionOperator_LESS;
                                    sLclCondition = sLclCondition.copy(1);
                                }
                            }
                            break;
                            case '>' :
                            {
                                if (sLclCondition[1] == '=')
                                {
                                    aOperator = sheet::ConditionOperator_GREATER_EQUAL;
                                    sLclCondition = sLclCondition.copy(2);
                                }
                                else
                                {
                                    aOperator = sheet::ConditionOperator_GREATER;
                                    sLclCondition = sLclCondition.copy(1);
                                }
                            }
                            break;
                            case '=' :
                            {
                                aOperator = sheet::ConditionOperator_EQUAL;
                                sLclCondition = sLclCondition.copy(1);
                            }
                            break;
                            case '!' :
                            {
                                aOperator = sheet::ConditionOperator_NOT_EQUAL;
                                sLclCondition = sLclCondition.copy(1);
                            }
                            break;
                        }
                        sFormula1 = sLclCondition;
                    }
                }
            }
        }
    }
}

void ScXMLContentValidationContext::EndElement()
{
    ScMyImportValidation aValidation;
    aValidation.sName = sName;
    aValidation.sBaseCellAddress = sBaseCellAddress;
    aValidation.sImputTitle = sHelpTitle;
    aValidation.sImputMessage = sHelpMessage;
    aValidation.sErrorTitle = sErrorTitle;
    aValidation.sErrorMessage = sErrorMessage;
    GetCondition(sCondition, aValidation.sFormula1, aValidation.sFormula2, aValidation.aValidationType, aValidation.aOperator);
    if (aValidation.sFormula1.getLength())
        ScXMLConverter::ParseFormula(aValidation.sFormula1);
    if (aValidation.sFormula2.getLength())
        ScXMLConverter::ParseFormula(aValidation.sFormula2);
    GetAlertStyle(sErrorMessageType, aValidation.aAlertStyle);
    aValidation.bShowErrorMessage = bDisplayError;
    aValidation.bShowImputMessage = bDisplayHelp;
    aValidation.bIgnoreBlanks = bAllowEmptyCell;
    GetScImport().AddValidation(aValidation);
}

void ScXMLContentValidationContext::SetHelpMessage(const ::rtl::OUString& sTitle, const ::rtl::OUString& sMessage, const sal_Bool bDisplay)
{
    sHelpTitle = sTitle;
    sHelpMessage = sMessage;
    bDisplayHelp = bDisplay;
}

void ScXMLContentValidationContext::SetErrorMessage(const ::rtl::OUString& sTitle, const ::rtl::OUString& sMessage,
    const ::rtl::OUString& sMessageType, const sal_Bool bDisplay)
{
    sErrorTitle = sTitle;
    sErrorMessage = sMessage;
    sErrorMessageType = sMessageType;
    bDisplayError = bDisplay;
}

void ScXMLContentValidationContext::SetErrorMacro(const ::rtl::OUString& sInName, const sal_Bool bExecute)
{
    sErrorTitle = sInName;
    sErrorMessageType = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("macro"));
    bDisplayError = bExecute;
}

ScXMLHelpMessageContext::ScXMLHelpMessageContext( ScXMLImport& rInImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLContentValidationContext* pTempValidationContext) :
    SvXMLImportContext( rInImport, nPrfx, rLName ),
    sTitle(),
    sMessage(),
    nParagraphCount(0),
    bDisplay(sal_False)
{
    pValidationContext = pTempValidationContext;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetContentValidationHelpMessageAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        ::rtl::OUString aLclLocalName;
        USHORT nLclPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLclLocalName );
        ::rtl::OUString sValue = xAttrList->getValueByIndex( i );

        switch( rAttrTokenMap.Get( nLclPrefix, aLclLocalName ) )
        {
            case XML_TOK_HELP_MESSAGE_ATTR_TITLE:
                sTitle = sValue;
            break;
            case XML_TOK_HELP_MESSAGE_ATTR_DISPLAY:
                bDisplay = IsXMLToken(sValue, XML_TRUE);
            break;
        }
    }
}

ScXMLHelpMessageContext::~ScXMLHelpMessageContext()
{
}

SvXMLImportContext *ScXMLHelpMessageContext::CreateChildContext( USHORT nInPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetContentValidationMessageElemTokenMap();
    switch( rTokenMap.Get( nInPrefix, rLName ) )
    {
        case XML_TOK_P:
        {
            if(nParagraphCount)
                sMessage.append(static_cast<sal_Unicode>('\n'));
            nParagraphCount++;
            pContext = new ScXMLContentContext( GetScImport(), nInPrefix, rLName, xAttrList, sMessage);
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nInPrefix, rLName );

    return pContext;
}

void ScXMLHelpMessageContext::EndElement()
{
    pValidationContext->SetHelpMessage(sTitle, sMessage.makeStringAndClear(), bDisplay);
}

ScXMLErrorMessageContext::ScXMLErrorMessageContext( ScXMLImport& rInImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLContentValidationContext* pTempValidationContext) :
    SvXMLImportContext( rInImport, nPrfx, rLName ),
    sTitle(),
    sMessage(),
    sMessageType(),
    nParagraphCount(0),
    bDisplay(sal_False)
{
    pValidationContext = pTempValidationContext;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetContentValidationErrorMessageAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        ::rtl::OUString aLclLocalName;
        USHORT nLclPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLclLocalName );
        ::rtl::OUString sValue = xAttrList->getValueByIndex( i );

        switch( rAttrTokenMap.Get( nLclPrefix, aLclLocalName ) )
        {
            case XML_TOK_ERROR_MESSAGE_ATTR_TITLE:
                sTitle = sValue;
            break;
            case XML_TOK_ERROR_MESSAGE_ATTR_MESSAGE_TYPE:
                sMessageType = sValue;
            break;
            case XML_TOK_ERROR_MESSAGE_ATTR_DISPLAY:
                bDisplay = IsXMLToken(sValue, XML_TRUE);
            break;
        }
    }
}

ScXMLErrorMessageContext::~ScXMLErrorMessageContext()
{
}

SvXMLImportContext *ScXMLErrorMessageContext::CreateChildContext( USHORT nInPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetContentValidationMessageElemTokenMap();
    switch( rTokenMap.Get( nInPrefix, rLName ) )
    {
        case XML_TOK_P:
        {
            if(nParagraphCount)
                sMessage.append(static_cast<sal_Unicode>('\n'));
            nParagraphCount++;
            pContext = new ScXMLContentContext( GetScImport(), nInPrefix, rLName, xAttrList, sMessage);
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nInPrefix, rLName );

    return pContext;
}

void ScXMLErrorMessageContext::EndElement()
{
    pValidationContext->SetErrorMessage(sTitle, sMessage.makeStringAndClear(), sMessageType, bDisplay);
}

ScXMLErrorMacroContext::ScXMLErrorMacroContext( ScXMLImport& rInImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLContentValidationContext* pTempValidationContext) :
    SvXMLImportContext( rInImport, nPrfx, rLName ),
    sName(),
    bExecute(sal_False)
{
    pValidationContext = pTempValidationContext;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetContentValidationErrorMacroAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        ::rtl::OUString aLclLocalName;
        USHORT nLclPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLclLocalName );
        ::rtl::OUString sValue = xAttrList->getValueByIndex( i );

        switch( rAttrTokenMap.Get( nLclPrefix, aLclLocalName ) )
        {
            case XML_TOK_ERROR_MACRO_ATTR_NAME:
                sName = sValue;
            break;
            case XML_TOK_ERROR_MACRO_ATTR_EXECUTE:
                bExecute = IsXMLToken(sValue, XML_TRUE);
            break;
        }
    }
}

ScXMLErrorMacroContext::~ScXMLErrorMacroContext()
{
}

SvXMLImportContext *ScXMLErrorMacroContext::CreateChildContext( USHORT nInPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& /*xAttrList*/ )
{
    SvXMLImportContext *pContext = NULL;

    if ((nInPrefix == XML_NAMESPACE_SCRIPT) && IsXMLToken(rLName, XML_EVENTS))
    {
        DBG_ASSERT(!sName.getLength(), "here is something wrong in the file");
        pContext = new XMLEventsImportContext(GetImport(), nInPrefix, rLName);
        xEventContext = pContext;
    }
    if (!pContext)
        pContext = new SvXMLImportContext( GetImport(), nInPrefix, rLName );

    return pContext;
}

void ScXMLErrorMacroContext::EndElement()
{
    if (xEventContext.Is())
    {
        ::rtl::OUString sOnError(RTL_CONSTASCII_USTRINGPARAM("OnError"));
        XMLEventsImportContext* pEvents =
            (XMLEventsImportContext*)&xEventContext;
        uno::Sequence<beans::PropertyValue> aValues;
        pEvents->GetEventSequence( sOnError, aValues );

        sal_Int32 nLength = aValues.getLength();
        for( sal_Int32 i = 0; i < nLength; i++ )
        {
            if ( aValues[i].Name.equalsAsciiL( "MacroName",
                                                    sizeof("MacroName")-1 ) )
            {
                aValues[i].Value >>= sName;
                break;
            }
        }
    }
    else
    DBG_ASSERT(sName.getLength(), "no macro name given");
    pValidationContext->SetErrorMacro(sName, bExecute);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
