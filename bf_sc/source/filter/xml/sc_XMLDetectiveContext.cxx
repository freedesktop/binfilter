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

//___________________________________________________________________


#include <bf_xmloff/nmspmap.hxx>
#include <bf_xmloff/xmluconv.hxx>

#include "convuno.hxx"
#include "xmlimprt.hxx"
#include "XMLConverter.hxx"

#include <algorithm>
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace xmloff::token;


//___________________________________________________________________

ScMyImpDetectiveObj::ScMyImpDetectiveObj() :
    aSourceRange(),
    eObjType( SC_DETOBJ_NONE ),
    bHasError( sal_False )
{
}

//___________________________________________________________________

sal_Bool ScMyImpDetectiveOp::operator<(const ScMyImpDetectiveOp& rDetOp) const
{
    return (nIndex < rDetOp.nIndex);
}

void ScMyImpDetectiveOpArray::Sort()
{
    aDetectiveOpList.sort();
}

sal_Bool ScMyImpDetectiveOpArray::GetFirstOp( ScMyImpDetectiveOp& rDetOp )
{
    if( aDetectiveOpList.empty() )
        return sal_False;
    ScMyImpDetectiveOpList::iterator aItr = aDetectiveOpList.begin();
    rDetOp = *aItr;
    aDetectiveOpList.erase( aItr );
    return sal_True;
}


//___________________________________________________________________

ScXMLDetectiveContext::ScXMLDetectiveContext(
        ScXMLImport& rInImport,
        USHORT nPrfx,
        const OUString& rLName,
        ScMyImpDetectiveObjVec* pNewDetectiveObjVec ) :
    SvXMLImportContext( rInImport, nPrfx, rLName ),
    pDetectiveObjVec( pNewDetectiveObjVec )
{
}

ScXMLDetectiveContext::~ScXMLDetectiveContext()
{
}

SvXMLImportContext *ScXMLDetectiveContext::CreateChildContext(
        USHORT nInPrefix,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext*     pContext    = NULL;
    const SvXMLTokenMap&    rTokenMap   = GetScImport().GetDetectiveElemTokenMap();

    switch( rTokenMap.Get( nInPrefix, rLName ) )
    {
        case XML_TOK_DETECTIVE_ELEM_HIGHLIGHTED:
            pContext = new ScXMLDetectiveHighlightedContext( GetScImport(), nInPrefix, rLName, xAttrList, pDetectiveObjVec );
        break;
        case XML_TOK_DETECTIVE_ELEM_OPERATION:
            pContext = new ScXMLDetectiveOperationContext( GetScImport(), nInPrefix, rLName, xAttrList );
        break;
    }
    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nInPrefix, rLName );

    return pContext;
}

void ScXMLDetectiveContext::EndElement()
{
}


//___________________________________________________________________

ScXMLDetectiveHighlightedContext::ScXMLDetectiveHighlightedContext(
        ScXMLImport& rInImport,
        USHORT nPrfx,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList >& xAttrList,
        ScMyImpDetectiveObjVec* pNewDetectiveObjVec ):
    SvXMLImportContext( rInImport, nPrfx, rLName ),
    pDetectiveObjVec( pNewDetectiveObjVec ),
    aDetectiveObj(),
    bValid( sal_False )
{
    if( !xAttrList.is() ) return;

    sal_Int16               nAttrCount      = xAttrList->getLength();
    const SvXMLTokenMap&    rAttrTokenMap   = GetScImport().GetDetectiveHighlightedAttrTokenMap();

    for( sal_Int16 nIndex = 0; nIndex < nAttrCount; nIndex++ )
    {
        OUString sAttrName  = xAttrList->getNameByIndex( nIndex );
        OUString sValue     = xAttrList->getValueByIndex( nIndex );
        OUString aLclLocalName;
        USHORT nLclPrefix       = GetScImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLclLocalName );

        switch( rAttrTokenMap.Get( nLclPrefix, aLclLocalName ) )
        {
            case XML_TOK_DETECTIVE_HIGHLIGHTED_ATTR_CELL_RANGE:
            {
                sal_Int32 nOffset(0);
                GetScImport().LockSolarMutex();
                bValid = ScXMLConverter::GetRangeFromString( aDetectiveObj.aSourceRange, sValue, GetScImport().GetDocument(), nOffset );
                GetScImport().UnlockSolarMutex();
            }
            break;
            case XML_TOK_DETECTIVE_HIGHLIGHTED_ATTR_DIRECTION:
                aDetectiveObj.eObjType = ScXMLConverter::GetDetObjTypeFromString( sValue );
            break;
            case XML_TOK_DETECTIVE_HIGHLIGHTED_ATTR_CONTAINS_ERROR:
                aDetectiveObj.bHasError = IsXMLToken(sValue, XML_TRUE);
            break;
            case XML_TOK_DETECTIVE_HIGHLIGHTED_ATTR_MARKED_INVALID:
                {
                    if (IsXMLToken(sValue, XML_TRUE))
                        aDetectiveObj.eObjType = SC_DETOBJ_CIRCLE;
                }
            break;
        }
    }
}

ScXMLDetectiveHighlightedContext::~ScXMLDetectiveHighlightedContext()
{
}

SvXMLImportContext *ScXMLDetectiveHighlightedContext::CreateChildContext(
        USHORT nInPrefix,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList>& /*xAttrList*/ )
{
    return new SvXMLImportContext( GetImport(), nInPrefix, rLName );
}

void ScXMLDetectiveHighlightedContext::EndElement()
{
    switch( aDetectiveObj.eObjType )
    {
        case SC_DETOBJ_ARROW:
        case SC_DETOBJ_TOOTHERTAB:
        break;
        case SC_DETOBJ_FROMOTHERTAB:
        case SC_DETOBJ_CIRCLE:
            bValid = sal_True;
        break;
        default:
            bValid = sal_False;
    }
    if( bValid )
        pDetectiveObjVec->push_back( aDetectiveObj );
}


//___________________________________________________________________

ScXMLDetectiveOperationContext::ScXMLDetectiveOperationContext(
        ScXMLImport& rInImport,
        USHORT nPrfx,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList >& xAttrList ) :
    SvXMLImportContext( rInImport, nPrfx, rLName ),
    aDetectiveOp(),
    bHasType( sal_False )
{
    if( !xAttrList.is() ) return;

    sal_Int16               nAttrCount      = xAttrList->getLength();
    const SvXMLTokenMap&    rAttrTokenMap   = GetScImport().GetDetectiveOperationAttrTokenMap();

    for( sal_Int16 nIndex = 0; nIndex < nAttrCount; nIndex++ )
    {
        OUString sAttrName  = xAttrList->getNameByIndex( nIndex );
        OUString sValue     = xAttrList->getValueByIndex( nIndex );
        OUString aLclLocalName;
        USHORT nLclPrefix       = GetScImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLclLocalName );

        switch( rAttrTokenMap.Get( nLclPrefix, aLclLocalName ) )
        {
            case XML_TOK_DETECTIVE_OPERATION_ATTR_NAME:
                bHasType = ScXMLConverter::GetDetOpTypeFromString( aDetectiveOp.eOpType, sValue );
            break;
            case XML_TOK_DETECTIVE_OPERATION_ATTR_INDEX:
            {
                sal_Int32 nValue;
                if( SvXMLUnitConverter::convertNumber( nValue, sValue, 0 ) )
                    aDetectiveOp.nIndex = nValue;
            }
            break;
        }
    }
    ScUnoConversion::FillScAddress( aDetectiveOp.aPosition, rInImport.GetTables().GetRealCellPos() );
}

ScXMLDetectiveOperationContext::~ScXMLDetectiveOperationContext()
{
}

SvXMLImportContext *ScXMLDetectiveOperationContext::CreateChildContext(
        USHORT nInPrefix,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList>& /*xAttrList*/ )
{
    return new SvXMLImportContext( GetImport(), nInPrefix, rLName );
}

void ScXMLDetectiveOperationContext::EndElement()
{
    if( bHasType && (aDetectiveOp.nIndex >= 0) )
        GetScImport().GetDetectiveOpArray()->AddDetectiveOp( aDetectiveOp );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
