/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifdef _MSC_VER
#pragma hdrstop
#endif

// INCLUDE ---------------------------------------------------------------

#include "XMLDDELinksContext.hxx"
#include "xmlimprt.hxx"
#include "document.hxx"

#include <bf_xmloff/xmlnmspe.hxx>
#include <bf_xmloff/nmspmap.hxx>
#include <bf_xmloff/xmluconv.hxx>

#include <tools/debug.hxx>
namespace binfilter {

using namespace ::com::sun::star;
using namespace xmloff::token;

using rtl::OUString;

//------------------------------------------------------------------

ScXMLDDELinksContext::ScXMLDDELinksContext( ScXMLImport& rInImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& /*xAttrList*/) :
    SvXMLImportContext( rInImport, nPrfx, rLName )
{
    // here are no attributes
    rInImport.LockSolarMutex();
}

ScXMLDDELinksContext::~ScXMLDDELinksContext()
{
    GetScImport().UnlockSolarMutex();
}

SvXMLImportContext *ScXMLDDELinksContext::CreateChildContext( USHORT nInPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if ((nInPrefix == XML_NAMESPACE_TABLE) && IsXMLToken(rLName, XML_DDE_LINK))
        pContext = new ScXMLDDELinkContext(GetScImport(), nInPrefix, rLName, xAttrList);

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nInPrefix, rLName );

    return pContext;
}

void ScXMLDDELinksContext::EndElement()
{
}

ScXMLDDELinkContext::ScXMLDDELinkContext( ScXMLImport& rInImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& /*xAttrList*/) :
    SvXMLImportContext( rInImport, nPrfx, rLName ),
    aDDELinkTable(),
    aDDELinkRow(),
    sApplication(),
    sTopic(),
    sItem(),
    nPosition(-1),
    nColumns(0),
    nRows(0),
    nMode(SC_DDE_DEFAULT)
{
    // here are no attributes
}

ScXMLDDELinkContext::~ScXMLDDELinkContext()
{
}

SvXMLImportContext *ScXMLDDELinkContext::CreateChildContext( USHORT nInPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if ((nInPrefix == XML_NAMESPACE_OFFICE) && IsXMLToken(rLName, XML_DDE_SOURCE))
        pContext = new ScXMLDDESourceContext(GetScImport(), nInPrefix, rLName, xAttrList, this);
    else if ((nInPrefix == XML_NAMESPACE_TABLE) && IsXMLToken(rLName, XML_TABLE))
        pContext = new ScXMLDDETableContext(GetScImport(), nInPrefix, rLName, xAttrList, this);

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nInPrefix, rLName );

    return pContext;
}

void ScXMLDDELinkContext::CreateDDELink()
{
    if (GetScImport().GetDocument() &&
        sApplication.getLength() &&
        sTopic.getLength() &&
        sItem.getLength())
    {
        String sAppl(sApplication);
        String sTop(sTopic);
        String sIt(sItem);
        nPosition = -1;
        DBG_ASSERT(nPosition > -1, "DDE Link not inserted");
    }
}

void ScXMLDDELinkContext::AddCellToRow(const ScDDELinkCell& aCell)
{
    aDDELinkRow.push_back(aCell);
}

void ScXMLDDELinkContext::AddRowsToTable(const sal_Int32 nInRows)
{
    for (sal_Int32 i = 0; i < nInRows; i++)
        aDDELinkTable.insert(aDDELinkTable.end(), aDDELinkRow.begin(), aDDELinkRow.end());
    aDDELinkRow.clear();
}

void ScXMLDDELinkContext::EndElement()
{
}

ScXMLDDESourceContext::ScXMLDDESourceContext( ScXMLImport& rInImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDDELinkContext* pTempDDELink) :
    SvXMLImportContext( rInImport, nPrfx, rLName ),
    pDDELink(pTempDDELink)
{
    if( !xAttrList.is() ) return;

    sal_Int16               nAttrCount      = xAttrList->getLength();

    for( sal_Int16 nIndex = 0; nIndex < nAttrCount; nIndex++ )
    {
        OUString sAttrName  = xAttrList->getNameByIndex( nIndex );
        OUString sValue     = xAttrList->getValueByIndex( nIndex );
        OUString aLclLocalName;
        USHORT nLclPrefix       = GetScImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLclLocalName );

        if (nLclPrefix == XML_NAMESPACE_OFFICE)
        {
            if (IsXMLToken(aLclLocalName, XML_DDE_APPLICATION))
                pDDELink->SetApplication(sValue);
            else if (IsXMLToken(aLclLocalName, XML_DDE_TOPIC))
                pDDELink->SetTopic(sValue);
            else if (IsXMLToken(aLclLocalName, XML_DDE_ITEM))
                pDDELink->SetItem(sValue);
        }
        else if ((nLclPrefix == XML_NAMESPACE_TABLE) && IsXMLToken(aLclLocalName, XML_CONVERSION_MODE))
        {
            if (IsXMLToken(sValue, XML_INTO_ENGLISH_NUMBER))
                pDDELink->SetMode(SC_DDE_ENGLISH);
            else if (IsXMLToken(sValue, XML_LET_TEXT))
                pDDELink->SetMode(SC_DDE_TEXT);
            else
                pDDELink->SetMode(SC_DDE_DEFAULT);
        }
    }
}

ScXMLDDESourceContext::~ScXMLDDESourceContext()
{
}

SvXMLImportContext *ScXMLDDESourceContext::CreateChildContext( USHORT nInPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& /*xAttrList*/ )
{
    SvXMLImportContext *pContext = new SvXMLImportContext( GetImport(), nInPrefix, rLName );

    return pContext;
}

void ScXMLDDESourceContext::EndElement()
{
    pDDELink->CreateDDELink();
}

ScXMLDDETableContext::ScXMLDDETableContext( ScXMLImport& rInImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& /*xAttrList*/,
                                        ScXMLDDELinkContext* pTempDDELink) :
    SvXMLImportContext( rInImport, nPrfx, rLName ),
    pDDELink(pTempDDELink)
{
    // here are no attributes
}

ScXMLDDETableContext::~ScXMLDDETableContext()
{
}

SvXMLImportContext *ScXMLDDETableContext::CreateChildContext( USHORT nInPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = NULL;

    if (nInPrefix == XML_NAMESPACE_TABLE)
    {
        if (IsXMLToken(rLName, XML_TABLE_COLUMN))
            pContext = new ScXMLDDEColumnContext(GetScImport(), nInPrefix, rLName, xAttrList, pDDELink);
        else if (IsXMLToken(rLName, XML_TABLE_ROW))
            pContext = new ScXMLDDERowContext(GetScImport(), nInPrefix, rLName, xAttrList, pDDELink);
    }

    if (!pContext)
        pContext = new SvXMLImportContext( GetImport(), nInPrefix, rLName );

    return pContext;
}

void ScXMLDDETableContext::EndElement()
{
}

ScXMLDDEColumnContext::ScXMLDDEColumnContext( ScXMLImport& rInImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDDELinkContext* pTempDDELink) :
    SvXMLImportContext( rInImport, nPrfx, rLName ),
    pDDELink(pTempDDELink)
{
    if( !xAttrList.is() ) return;
    sal_Int32 nCols(1);

    sal_Int16               nAttrCount      = xAttrList->getLength();

    for( sal_Int16 nIndex = 0; nIndex < nAttrCount; nIndex++ )
    {
        OUString sAttrName  = xAttrList->getNameByIndex( nIndex );
        OUString sValue     = xAttrList->getValueByIndex( nIndex );
        OUString aLclLocalName;
        USHORT nLclPrefix       = GetScImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLclLocalName );

        if (nLclPrefix == XML_NAMESPACE_TABLE)
            if (IsXMLToken(aLclLocalName, XML_NUMBER_COLUMNS_REPEATED))
                GetScImport().GetMM100UnitConverter().convertNumber(nCols, sValue);
    }
    pDDELink->AddColumns(nCols);
}

ScXMLDDEColumnContext::~ScXMLDDEColumnContext()
{
}

SvXMLImportContext *ScXMLDDEColumnContext::CreateChildContext( USHORT nInPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& /*xAttrList*/ )
{
    SvXMLImportContext *pContext = new SvXMLImportContext( GetImport(), nInPrefix, rLName );

    return pContext;
}

void ScXMLDDEColumnContext::EndElement()
{
}

ScXMLDDERowContext::ScXMLDDERowContext( ScXMLImport& rInImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDDELinkContext* pTempDDELink) :
    SvXMLImportContext( rInImport, nPrfx, rLName ),
    pDDELink(pTempDDELink),
    nRows(1)
{
    if( !xAttrList.is() ) return;

    sal_Int16               nAttrCount      = xAttrList->getLength();

    for( sal_Int16 nIndex = 0; nIndex < nAttrCount; nIndex++ )
    {
        OUString sAttrName  = xAttrList->getNameByIndex( nIndex );
        OUString sValue     = xAttrList->getValueByIndex( nIndex );
        OUString aLclLocalName;
        USHORT nLclPrefix       = GetScImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLclLocalName );

        if (nLclPrefix == XML_NAMESPACE_TABLE)
            if (IsXMLToken(aLclLocalName, XML_NUMBER_ROWS_REPEATED))
                GetScImport().GetMM100UnitConverter().convertNumber(nRows, sValue);
    }
    pDDELink->AddRows(nRows);
}

ScXMLDDERowContext::~ScXMLDDERowContext()
{
}

SvXMLImportContext *ScXMLDDERowContext::CreateChildContext( USHORT nInPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = NULL;

    if (nInPrefix == XML_NAMESPACE_TABLE)
        if (IsXMLToken(rLName, XML_TABLE_CELL))
            pContext = new ScXMLDDECellContext(GetScImport(), nInPrefix, rLName, xAttrList, pDDELink);

    if (!pContext)
        pContext = new SvXMLImportContext( GetImport(), nInPrefix, rLName );

    return pContext;
}

void ScXMLDDERowContext::EndElement()
{
    pDDELink->AddRowsToTable(nRows);
}

ScXMLDDECellContext::ScXMLDDECellContext( ScXMLImport& rInImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDDELinkContext* pTempDDELink) :
    SvXMLImportContext( rInImport, nPrfx, rLName ),
    sValue(),
    fValue(),
    nCells(1),
    bString(sal_True),
    bString2(sal_True),
    bEmpty(sal_True),
    pDDELink(pTempDDELink)
{
    if( !xAttrList.is() ) return;

    sal_Int16               nAttrCount      = xAttrList->getLength();

    for( sal_Int16 nIndex = 0; nIndex < nAttrCount; nIndex++ )
    {
        OUString sAttrName  = xAttrList->getNameByIndex( nIndex );
        OUString sTempValue     = xAttrList->getValueByIndex( nIndex );
        OUString aLclLocalName;
        USHORT nLclPrefix       = GetScImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLclLocalName );

        if (nLclPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLclLocalName, XML_VALUE_TYPE))
            {
                if (IsXMLToken(sTempValue, XML_STRING))
                    bString = sal_True;
                else
                    bString = sal_False;
            }
            else if (IsXMLToken(aLclLocalName, XML_STRING_VALUE))
            {
                sValue = sTempValue;
                bEmpty = sal_False;
                bString2 = sal_True;
            }
            else if (IsXMLToken(aLclLocalName, XML_VALUE))
            {
                GetScImport().GetMM100UnitConverter().convertDouble(fValue, sTempValue);
                bEmpty = sal_False;
                bString2 = sal_False;
            }
            else if (IsXMLToken(aLclLocalName, XML_NUMBER_COLUMNS_REPEATED))
                GetScImport().GetMM100UnitConverter().convertNumber(nCells, sTempValue);
        }
    }
}

ScXMLDDECellContext::~ScXMLDDECellContext()
{
}

SvXMLImportContext *ScXMLDDECellContext::CreateChildContext( USHORT nInPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& /*xAttrList*/ )
{
    SvXMLImportContext *pContext = new SvXMLImportContext( GetImport(), nInPrefix, rLName );

    return pContext;
}

void ScXMLDDECellContext::EndElement()
{
    DBG_ASSERT(bString == bString2, "something wrong with this type");
    ScDDELinkCell aCell;
    aCell.sValue = sValue;
    aCell.fValue = fValue;
    aCell.bEmpty = bEmpty;
    aCell.bString = bString2;
    for(sal_Int32 i = 0; i < nCells; i++)
        pDDELink->AddCellToRow(aCell);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
