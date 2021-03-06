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

#include <tools/debug.hxx>

#include <com/sun/star/style/XStyle.hpp>

#include "xmlimp.hxx"
#include "xmlnumi.hxx"
#include "nmspmap.hxx"

#include "XMLTextListItemContext.hxx"
#include "XMLTextListBlockContext.hxx"
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::beans;
using namespace ::binfilter::xmloff::token;

using rtl::OUString;

TYPEINIT1( XMLTextListBlockContext, SvXMLImportContext );

XMLTextListBlockContext::XMLTextListBlockContext(
        SvXMLImport& rInImport,
        XMLTextImportHelper& rTxtImp,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        sal_Bool bOrd ) :
    SvXMLImportContext( rInImport, nPrfx, rLName ),
    rTxtImport( rTxtImp ),
    sNumberingRules( RTL_CONSTASCII_USTRINGPARAM( "NumberingRules" ) ),
    xParentListBlock( rTxtImp.GetListBlock() ),
    nLevel( 0 ),
    nLevels( 0 ),
    bOrdered( bOrd ),
    bRestartNumbering( sal_True ),
    bSetDefaults( sal_False )
{
    // Inherit style name from parent list, as well as the flags whether
    // numbering must be restarted and formats have to be created.
    OUString sParentStyleName;
    if( xParentListBlock.Is() )
    {
        XMLTextListBlockContext *pParent =
            (XMLTextListBlockContext *)&xParentListBlock;
        sStyleName = pParent->GetStyleName();
        xNumRules = pParent->GetNumRules();
        sParentStyleName = sStyleName;
        nLevels = pParent->nLevels;
        nLevel = pParent->GetLevel() + 1;
        bRestartNumbering = pParent->IsRestartNumbering();
        bSetDefaults = pParent->bSetDefaults;
    }

    const SvXMLTokenMap& rTokenMap =
        rTxtImport.GetTextListBlockAttrTokenMap();

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        OUString aLclLocalName;
        sal_uInt16 nLclPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLclLocalName );
        switch( rTokenMap.Get( nLclPrefix, aLclLocalName ) )
        {
        case XML_TOK_TEXT_LIST_BLOCK_CONTINUE_NUMBERING:
            bRestartNumbering = !IsXMLToken(rValue, XML_TRUE);
            break;
        case XML_TOK_TEXT_LIST_BLOCK_STYLE_NAME:
            sStyleName = rValue;
            break;
        }
    }

    if( sStyleName.getLength() && sStyleName != sParentStyleName )
    {
        const Reference < XNameContainer >& rNumStyles =
            rTxtImport.GetNumberingStyles();
        if( rNumStyles.is() && rNumStyles->hasByName( sStyleName ) )
        {
            Reference < XStyle > xStyle;
            Any aAny = rNumStyles->getByName( sStyleName );
            aAny >>= xStyle;

            // If the style has not been used, the restart numbering has
            // to be set never.
            if( bRestartNumbering && !xStyle->isInUse() )
                bRestartNumbering = sal_False;

            Reference< XPropertySet > xPropSet( xStyle, UNO_QUERY );
            aAny = xPropSet->getPropertyValue( sNumberingRules );
            aAny >>= xNumRules;
            nLevels = xNumRules->getCount();
        }
        else
        {
            const SvxXMLListStyleContext *pListStyle =
                rTxtImport.FindAutoListStyle( sStyleName );
            if( pListStyle )
            {
                xNumRules = pListStyle->GetNumRules();
                sal_Bool bUsed = xNumRules.is();
                if( !xNumRules.is() )
                {
                    pListStyle->CreateAndInsertAuto();
                    xNumRules = pListStyle->GetNumRules();
                }
                if( bRestartNumbering && !bUsed )
                    bRestartNumbering = sal_False;
                nLevels = pListStyle->GetLevels();
            }
        }
    }

    if( !xNumRules.is() )
    {
        // If no style name has been specified for this style and for any
        // parent or if no num rule this the specified name is existing,
        // create a new one.

        xNumRules =
            SvxXMLListStyleContext::CreateNumRule( GetImport().GetModel() );
        DBG_ASSERT( xNumRules.is(), "go no numbering rule" );
        if( !xNumRules.is() )
            return;

        nLevels = xNumRules->getCount();

        // Because its a new num rule, numbering mist be restarted never.
        bRestartNumbering = sal_False;
        bSetDefaults = sal_True;
    }

    if( nLevel >= nLevels )
        nLevel = nLevels-1;

    if( bSetDefaults )
    {
        // Because there is no list style sheet for this style, a default
        // format must be set for any level of this num rule.
        SvxXMLListStyleContext::SetDefaultStyle( xNumRules, nLevel,
                                                   bOrdered );
    }

    // Remember this list block.
    rTxtImport.SetListBlock( this );

    // There is no list item by now.
    rTxtImport.SetListItem( 0 );
}

XMLTextListBlockContext::~XMLTextListBlockContext()
{
}

void XMLTextListBlockContext::EndElement()
{
    // Numbering has not to be restarted if it has been restarted within
    // a child list.
    XMLTextListBlockContext *pParent =
        (XMLTextListBlockContext *)&xParentListBlock;
    if( pParent )
    {
        pParent->bRestartNumbering = bRestartNumbering;
    }

    // Restore current list block.
    rTxtImport.SetListBlock( pParent );

    // Any paragraph following the list within the same list item mist not
    // be numbered.
    rTxtImport.SetListItem( 0 );
}

SvXMLImportContext *XMLTextListBlockContext::CreateChildContext(
        sal_uInt16 nInPrefix,
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap =
                        rTxtImport.GetTextListBlockElemTokenMap();
    sal_Bool bHeader = sal_False;
    switch( rTokenMap.Get( nInPrefix, rLocalName ) )
    {
    case XML_TOK_TEXT_LIST_HEADER:
        bHeader = sal_True;
    case XML_TOK_TEXT_LIST_ITEM:
        pContext = new XMLTextListItemContext( GetImport(), rTxtImport,
                                                nInPrefix, rLocalName,
                                              xAttrList, bHeader );
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nInPrefix, rLocalName );

    return pContext;
}



}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
