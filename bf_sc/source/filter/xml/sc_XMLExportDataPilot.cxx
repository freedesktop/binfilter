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


#include "XMLExportDataPilot.hxx"

#include <bf_xmloff/xmlnmspe.hxx>
#include <bf_xmloff/xmluconv.hxx>
#include <bf_xmloff/nmspmap.hxx>

#include "xmlexprt.hxx"
#include "XMLConverter.hxx"
#include "document.hxx"
#include "dpobject.hxx"
#include "dociter.hxx"
#include "attrib.hxx"
#include "patattr.hxx"
#include "scitems.hxx"
#include "dpsave.hxx"
#include "dpshttab.hxx"
#include "dpsdbtab.hxx"

#include <com/sun/star/sheet/DataImportMode.hpp>
namespace binfilter {

using namespace ::com::sun::star;
using namespace xmloff::token;

ScXMLExportDataPilot::ScXMLExportDataPilot(ScXMLExport& rTempExport)
    : rExport(rTempExport),
    pDoc( NULL )
{
}

ScXMLExportDataPilot::~ScXMLExportDataPilot()
{
}

rtl::OUString ScXMLExportDataPilot::getDPOperatorXML(const ScQueryOp aFilterOperator, const sal_Bool bUseRegularExpressions,
    const sal_Bool bIsString, const double dVal, const String& sVal) const
{
    switch (aFilterOperator)
    {
        case SC_EQUAL :
        {
            ::rtl::OUString sReturn;
            if (bUseRegularExpressions)
                sReturn = GetXMLToken(XML_MATCH);
            else
                sReturn = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("="));

            if (!bIsString && sVal == EMPTY_STRING)
            {
                if (dVal == SC_EMPTYFIELDS)
                    sReturn = GetXMLToken(XML_EMPTY);
                else if (dVal == SC_NONEMPTYFIELDS)
                    sReturn = GetXMLToken(XML_NOEMPTY);
            }

            return sReturn;
        }
        break;
        case SC_NOT_EQUAL :
        {
            if (bUseRegularExpressions)
                return GetXMLToken(XML_NOMATCH);
            else
                return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("!="));
        }
        break;
        case SC_BOTPERC :
            return GetXMLToken(XML_BOTTOM_PERCENT);
            break;
        case SC_BOTVAL :
            return GetXMLToken(XML_BOTTOM_VALUES);
            break;
        case SC_GREATER :
            return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(">"));
            break;
        case SC_GREATER_EQUAL :
            return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(">="));
            break;
        case SC_LESS :
            return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("<"));
            break;
        case SC_LESS_EQUAL :
            return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("<="));
            break;
        case SC_TOPPERC :
            return GetXMLToken(XML_TOP_PERCENT);
            break;
        case SC_TOPVAL :
            return GetXMLToken(XML_TOP_VALUES);
            break;
        default:
            OSL_FAIL("This FilterOperator is not supported.");
    }
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("="));
}

void ScXMLExportDataPilot::WriteDPCondition(const ScQueryEntry& aQueryEntry, sal_Bool bIsCaseSensitive, sal_Bool bUseRegularExpressions)
{
    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_FIELD_NUMBER, ::rtl::OUString::valueOf(sal_Int32(aQueryEntry.nField)));
    if (bIsCaseSensitive)
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_CASE_SENSITIVE, XML_TRUE);
    if (aQueryEntry.bQueryByString)
    {
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_VALUE, *aQueryEntry.pStr);
    }
    else
    {
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATA_TYPE, XML_NUMBER);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_VALUE, ::rtl::OUString(*aQueryEntry.pStr));
    }
    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_OPERATOR, getDPOperatorXML(aQueryEntry.eOp, bUseRegularExpressions,
        aQueryEntry.bQueryByString, aQueryEntry.nVal, *aQueryEntry.pStr));
    SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TABLE, XML_FILTER_CONDITION, sal_True, sal_True);
}

void ScXMLExportDataPilot::WriteDPFilter(const ScQueryParam& aQueryParam)
{
    sal_Int16 nQueryEntryCount = aQueryParam.GetEntryCount();
    if (nQueryEntryCount > 0)
    {
        sal_Bool bAnd(sal_False);
        sal_Bool bOr(sal_False);
        sal_Bool bHasEntries(sal_True);
        sal_Int16 nEntries(0);
        sal_Int32 j;

        for (j = 0; (j < nQueryEntryCount) && bHasEntries; j++)
        {
            ScQueryEntry aEntry = aQueryParam.GetEntry(static_cast<USHORT>(j));
            if (aEntry.bDoQuery)
            {
                if (nEntries > 0)
                {
                    if (aEntry.eConnect == SC_AND)
                        bAnd = sal_True;
                    else
                        bOr = sal_True;
                }
                nEntries++;
            }
            else
                bHasEntries = sal_False;
        }
        nQueryEntryCount = nEntries;
        if (nQueryEntryCount)
        {
            // There is never a target range in a data pilot.
/*          if (!aQueryParam.bInplace)
            {
                ScAddress aTargetAddress(aQueryParam.nDestCol, aQueryParam.nDestRow, aQueryParam.nDestTab);
                ::rtl::OUString sAddress;
                ScXMLConverter::GetStringFromAddress( sAddress, aTargetAddress, pDoc );
                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_TARGET_RANGE_ADDRESS, sAddress);
            }*/
            if(!((aQueryParam.nCol1 == aQueryParam.nCol2) && (aQueryParam.nRow1 == aQueryParam.nRow2) && (aQueryParam.nCol1 == aQueryParam.nRow1)
                && (aQueryParam.nCol1 == 0) && (aQueryParam.nTab == USHRT_MAX)))
            {
                ScRange aConditionRange(aQueryParam.nCol1, aQueryParam.nRow1, aQueryParam.nTab,
                    aQueryParam.nCol2, aQueryParam.nRow2, aQueryParam.nTab);
                ::rtl::OUString sConditionRange;
                ScXMLConverter::GetStringFromRange( sConditionRange, aConditionRange, pDoc );
                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_CONDITION_SOURCE_RANGE_ADDRESS, sConditionRange);
            }
            if (!aQueryParam.bDuplicate)
                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DISPLAY_DUPLICATES, XML_FALSE);
            SvXMLElementExport aElemDPF(rExport, XML_NAMESPACE_TABLE, XML_FILTER, sal_True, sal_True);
            rExport.CheckAttrList();
            if (nQueryEntryCount  == 1)
            {
                    WriteDPCondition(aQueryParam.GetEntry(0), aQueryParam.bCaseSens, aQueryParam.bRegExp);
            }
            else if (bOr && !bAnd)
            {
                SvXMLElementExport aElemOr(rExport, XML_NAMESPACE_TABLE, XML_FILTER_OR, sal_True, sal_True);
                for (j = 0; j < nQueryEntryCount; j++)
                {
                    WriteDPCondition(aQueryParam.GetEntry(static_cast<USHORT>(j)), aQueryParam.bCaseSens, aQueryParam.bRegExp);
                }
            }
            else if (bAnd && !bOr)
            {
                SvXMLElementExport aElemAnd(rExport, XML_NAMESPACE_TABLE, XML_FILTER_AND, sal_True, sal_True);
                for (j = 0; j < nQueryEntryCount; j++)
                {
                    WriteDPCondition(aQueryParam.GetEntry(static_cast<USHORT>(j)), aQueryParam.bCaseSens, aQueryParam.bRegExp);
                }
            }
            else
            {
                SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TABLE, XML_FILTER_OR, sal_True, sal_True);
                ScQueryEntry aPrevFilterField = aQueryParam.GetEntry(0);
                ScQueryConnect aConnection = aQueryParam.GetEntry(1).eConnect;
                sal_Bool bOpenAndElement;
                ::rtl::OUString aName = rExport.GetNamespaceMap().GetQNameByKey(XML_NAMESPACE_TABLE, GetXMLToken(XML_FILTER_AND));
                if (aConnection == SC_AND)
                {
                    rExport.StartElement( aName, sal_True );
                    bOpenAndElement = sal_True;
                }
                else
                    bOpenAndElement = sal_False;
                for (j = 1; j < nQueryEntryCount; j++)
                {
                    if (aConnection != aQueryParam.GetEntry(static_cast<USHORT>(j)).eConnect)
                    {
                        aConnection = aQueryParam.GetEntry(static_cast<USHORT>(j)).eConnect;
                        if (aQueryParam.GetEntry(static_cast<USHORT>(j)).eConnect == SC_AND)
                        {
                            rExport.StartElement( aName, sal_True );
                            bOpenAndElement = sal_True;
                            WriteDPCondition(aPrevFilterField, aQueryParam.bCaseSens, aQueryParam.bRegExp);
                            aPrevFilterField = aQueryParam.GetEntry(static_cast<USHORT>(j));
                            if (j == nQueryEntryCount - 1)
                            {
                                WriteDPCondition(aPrevFilterField, aQueryParam.bCaseSens, aQueryParam.bRegExp);
                                rExport.EndElement(aName, sal_True);
                                bOpenAndElement = sal_False;
                            }
                        }
                        else
                        {
                            WriteDPCondition(aPrevFilterField, aQueryParam.bCaseSens, aQueryParam.bRegExp);
                            aPrevFilterField = aQueryParam.GetEntry(static_cast<USHORT>(j));
                            if (bOpenAndElement)
                            {
                                rExport.EndElement(aName, sal_True);
                                bOpenAndElement = sal_False;
                            }
                            if (j == nQueryEntryCount - 1)
                            {
                                WriteDPCondition(aPrevFilterField, aQueryParam.bCaseSens, aQueryParam.bRegExp);
                            }
                        }
                    }
                    else
                    {
                        WriteDPCondition(aPrevFilterField, aQueryParam.bCaseSens, aQueryParam.bRegExp);
                        aPrevFilterField = aQueryParam.GetEntry(static_cast<USHORT>(j));
                        if (j == nQueryEntryCount - 1)
                            WriteDPCondition(aPrevFilterField, aQueryParam.bCaseSens, aQueryParam.bRegExp);
                    }
                }
            }
        }
    }
}

void ScXMLExportDataPilot::WriteDataPilots(const uno::Reference <sheet::XSpreadsheetDocument>& /*xSpreadDoc*/)
{
    pDoc = rExport.GetDocument();
    if (pDoc)
    {
        ScDPCollection* pDPs = pDoc->GetDPCollection();
        if (pDPs)
        {
            sal_Int16 nDPCount = pDPs->GetCount();
            if (nDPCount > 0)
            {
                SvXMLElementExport aElemDPs(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_TABLES, sal_True, sal_True);
                rExport.CheckAttrList();
                for (sal_Int16 i = 0; i < nDPCount; i++)
                {
                    ScDPSaveData* pDPSave = (*pDPs)[i]->GetSaveData();
                    if (pDPSave)
                    {
                        ScRange aOutRange = (*pDPs)[i]->GetOutRange();
                        ::rtl::OUString sTargetRangeAddress;
                        ScXMLConverter::GetStringFromRange( sTargetRangeAddress, aOutRange, pDoc );
                        ScDocAttrIterator aAttrItr(pDoc, aOutRange.aStart.Tab(),
                            aOutRange.aStart.Col(), aOutRange.aStart.Row(),
                            aOutRange.aEnd.Col(), aOutRange.aEnd.Row());
                        sal_uInt16 nCol, nRow1, nRow2;
                        ::rtl::OUString sOUButtonList;
                        const ScPatternAttr* pAttr = aAttrItr.GetNext(nCol, nRow1, nRow2);
                        while (pAttr)
                        {
                            ScMergeFlagAttr& rItem = (ScMergeFlagAttr&)pAttr->GetItem(ATTR_MERGE_FLAG);
                            if (rItem.HasButton())
                            {
                                for (sal_uInt16 nButtonRow = nRow1; nButtonRow <= nRow2; nButtonRow++)
                                {
                                    ScAddress aButtonAddr(nCol, nButtonRow, aOutRange.aStart.Tab());
                                    ScXMLConverter::GetStringFromAddress(
                                        sOUButtonList, aButtonAddr, pDoc, sal_True );
                                }
                            }
                            pAttr = aAttrItr.GetNext(nCol, nRow1, nRow2);
                        }
                        ::rtl::OUString sName((*pDPs)[i]->GetName());
                        ::rtl::OUString sApplicationData((*pDPs)[i]->GetTag());
                        sal_Bool bRowGrand = pDPSave->GetRowGrand();
                        sal_Bool bColumnGrand = pDPSave->GetColumnGrand();
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_NAME, sName);
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_APPLICATION_DATA, sApplicationData);
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_TARGET_RANGE_ADDRESS, sTargetRangeAddress);
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_BUTTONS, sOUButtonList);
                        if (!(bRowGrand && bColumnGrand))
                        {
                            if (bRowGrand)
                                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_GRAND_TOTAL, XML_ROW);
                            else if (bColumnGrand)
                                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_GRAND_TOTAL, XML_COLUMN);
                            else
                                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_GRAND_TOTAL, XML_NONE);
                        }
                        if (pDPSave->GetIgnoreEmptyRows())
                            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_IGNORE_EMPTY_ROWS, XML_TRUE);
                        if (pDPSave->GetRepeatIfEmpty())
                            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_IDENTIFY_CATEGORIES, XML_TRUE);
                        SvXMLElementExport aElemDP(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_TABLE, sal_True, sal_True);
                        rExport.CheckAttrList();
                        if ((*pDPs)[i]->IsSheetData())
                        {
                            const ScSheetSourceDesc* pSheetSource = (*pDPs)[i]->GetSheetDesc();
                            ::rtl::OUString sCellRangeAddress;
                            ScXMLConverter::GetStringFromRange( sCellRangeAddress, pSheetSource->aSourceRange, pDoc );
                            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_CELL_RANGE_ADDRESS, sCellRangeAddress);
                            SvXMLElementExport aElemSCR(rExport, XML_NAMESPACE_TABLE, XML_SOURCE_CELL_RANGE, sal_True, sal_True);
                            rExport.CheckAttrList();
                            WriteDPFilter(pSheetSource->aQueryParam);
                        }
                        else if ((*pDPs)[i]->IsImportData())
                        {
                            const ScImportSourceDesc* pImpSource = (*pDPs)[i]->GetImportSourceDesc();
                            switch (pImpSource->nType)
                            {
                                case sheet::DataImportMode_NONE : break;
                                case sheet::DataImportMode_QUERY :
                                {
                                    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATABASE_NAME, ::rtl::OUString(pImpSource->aDBName));
                                    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_QUERY_NAME, ::rtl::OUString(pImpSource->aObject));
                                    SvXMLElementExport aElemID(rExport, XML_NAMESPACE_TABLE, XML_DATABASE_SOURCE_QUERY, sal_True, sal_True);
                                    rExport.CheckAttrList();
                                }
                                break;
                                case sheet::DataImportMode_TABLE :
                                {
                                    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATABASE_NAME, ::rtl::OUString(pImpSource->aDBName));
                                    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_TABLE_NAME, ::rtl::OUString(pImpSource->aObject));
                                    SvXMLElementExport aElemID(rExport, XML_NAMESPACE_TABLE, XML_DATABASE_SOURCE_TABLE, sal_True, sal_True);
                                    rExport.CheckAttrList();
                                }
                                break;
                                case sheet::DataImportMode_SQL :
                                {
                                    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATABASE_NAME, ::rtl::OUString(pImpSource->aDBName));
                                    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_SQL_STATEMENT, ::rtl::OUString(pImpSource->aObject));
                                    if (!pImpSource->bNative)
                                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_PARSE_SQL_STATEMENT, XML_TRUE);
                                    SvXMLElementExport aElemID(rExport, XML_NAMESPACE_TABLE, XML_DATABASE_SOURCE_SQL, sal_True, sal_True);
                                    rExport.CheckAttrList();
                                }
                                break;
                            }
                        }
                        else if ((*pDPs)[i]->IsServiceData())
                        {
                            const ScDPServiceDesc* pServSource = (*pDPs)[i]->GetDPServiceDesc();
                            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_NAME, ::rtl::OUString(pServSource->aServiceName));
                            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_SOURCE_NAME, ::rtl::OUString(pServSource->aParSource));
                            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_OBJECT_NAME, ::rtl::OUString(pServSource->aParName));
                            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_USERNAME, ::rtl::OUString(pServSource->aParUser));
                            // How to write the Passwort? We must know, whether the passwort shoulb be written encrypted and how or not
                            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_PASSWORT, ::rtl::OUString(pServSource->aParPass));
                            SvXMLElementExport aElemSD(rExport, XML_NAMESPACE_TABLE, XML_SOURCE_SERVICE, sal_True, sal_True);
                            rExport.CheckAttrList();
                        }
                        type_ScDPDimensionList aDimensions = pDPSave->GetDimensions();
                        size_t nDimCount = aDimensions.size();
                        for (size_t nDim = 0; nDim < nDimCount; nDim++)
                        {
                            ScDPSaveDimension* pDim = aDimensions[ nDim ];
                            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_SOURCE_FIELD_NAME, ::rtl::OUString(pDim->GetName()));
                            if (pDim->IsDataLayout())
                                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_IS_DATA_LAYOUT_FIELD, XML_TRUE);
                            ::rtl::OUString sValueStr;
                            ScXMLConverter::GetStringFromOrientation( sValueStr,
                                (sheet::DataPilotFieldOrientation) pDim->GetOrientation() );
                            if( sValueStr.getLength() )
                                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ORIENTATION, sValueStr );
                            if (pDim->GetUsedHierarchy() != 1)
                            {
                                ::rtl::OUStringBuffer sBuffer;
                                SvXMLUnitConverter::convertNumber(sBuffer, pDim->GetUsedHierarchy());
                                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_USED_HIERARCHY, sBuffer.makeStringAndClear());
                            }
                            ScXMLConverter::GetStringFromFunction( sValueStr,
                                (sheet::GeneralFunction) pDim->GetFunction() );
                            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_FUNCTION, sValueStr);
                            SvXMLElementExport aElemDPF(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_FIELD, sal_True, sal_True);
                            rExport.CheckAttrList();
                            {
                                ::rtl::OUStringBuffer sBuffer;
                                SvXMLUnitConverter::convertBool(sBuffer, pDim->GetShowEmpty());
                                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DISPLAY_EMPTY, sBuffer.makeStringAndClear());
                                SvXMLElementExport aElemDPL(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_LEVEL, sal_True, sal_True);
                                rExport.CheckAttrList();
                                sal_Int32 nSubTotalCount = pDim->GetSubTotalsCount();
                                if (nSubTotalCount > 0)
                                {
                                    SvXMLElementExport aElemSTs(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_SUBTOTALS, sal_True, sal_True);
                                    rExport.CheckAttrList();
                                    for (sal_Int32 nSubTotal = 0; nSubTotal < nSubTotalCount; nSubTotal++)
                                    {
                                        ::rtl::OUString sFunction;
                                        ScXMLConverter::GetStringFromFunction( sFunction, (sheet::GeneralFunction)pDim->GetSubTotalFunc(nSubTotal) );
                                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_FUNCTION, sFunction);
                                        SvXMLElementExport aElemST(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_SUBTOTAL, sal_True, sal_True);
                                    }
                                }
                                type_MemberList aMembers = pDim->GetMembers();
                                size_t nMemberCount = aMembers.size();
                                if (nMemberCount > 0)
                                {
                                    SvXMLElementExport aElemDPMs(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_MEMBERS, sal_True, sal_True);
                                    rExport.CheckAttrList();
                                    for (size_t nMember = 0; nMember < nMemberCount; nMember++)
                                    {
                                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_NAME, ::rtl::OUString( aMembers[ nMember ]->GetName() ) );
                                        ::rtl::OUStringBuffer sTmpBuffer;
                                        SvXMLUnitConverter::convertBool(sTmpBuffer, aMembers[ nMember ]->GetIsVisible() );
                                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DISPLAY, sTmpBuffer.makeStringAndClear());
                                        SvXMLUnitConverter::convertBool(sTmpBuffer, aMembers[ nMember ]->GetShowDetails() );
                                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DISPLAY_DETAILS, sTmpBuffer.makeStringAndClear());
                                        SvXMLElementExport aElemDPM(rExport, XML_NAMESPACE_TABLE, XML_DATA_PILOT_MEMBER, sal_True, sal_True);
                                        rExport.CheckAttrList();
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
