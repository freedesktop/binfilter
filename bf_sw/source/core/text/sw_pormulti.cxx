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

#include <hintids.hxx>

#include <com/sun/star/i18n/ScriptType.hdl>
#include <bf_svx/twolinesitem.hxx>
#include <bf_svx/charrotateitem.hxx>

#include <charfmt.hxx>
#include <txtinet.hxx>
#include <fchrfmt.hxx>
#include <pormulti.hxx>     // SwMultiPortion
#include <itrform2.hxx>     // SwTxtFormatter

#include <horiornt.hxx>

#include <porfld.hxx>       // SwFldPortion
namespace binfilter {

using namespace ::com::sun::star;
extern sal_Bool IsUnderlineBreak( const SwLinePortion& rPor, const SwFont& rFnt );
extern BYTE WhichFont( xub_StrLen nIdx, const String* pTxt,
                       const SwScriptInfo* pSI );

/*--------------------------------------------------
 * SwTxtFormatter::MakeRestPortion(..)
 * When a fieldportion at the end of line breaks and needs a following
 * fieldportion in the next line, then the "restportion" of the formatinfo
 * has to be set. Normally this happens during the formatting of the first
 * part of the fieldportion.
 * But sometimes the formatting starts at the line with the following part,
 * exspecally when the following part is on the next page.
 * In this case the MakeRestPortion-function has to create the following part.
 * The first parameter is the line that contains possibly a first part
 * of a field. When the function finds such field part, it creates the right
 * restportion. This may be a multiportion, e.g. if the field is surrounded by
 * a doubleline- or ruby-portion.
 * The second parameter is the start index of the line.
 * --------------------------------------------------*/

/*N*/ SwLinePortion* SwTxtFormatter::MakeRestPortion( const SwLineLayout* pLine,
/*N*/   xub_StrLen nPos1 )
/*N*/ {
/*N*/   if( !nPos1 )
/*N*/       return NULL;
/*N*/   xub_StrLen nMultiPos = nPos1 - pLine->GetLen();
/*N*/   const SwMultiPortion *pMulti1 = NULL;
/*N*/   const SwLinePortion* pPor = pLine->GetFirstPortion();
/*N*/   SwFldPortion *pFld = NULL;
/*N*/   while( pPor )
/*N*/   {
/*N*/       if( pPor->GetLen() )
/*N*/       {
/*N*/           if( !pMulti1 )
/*N*/           {
/*N*/               nMultiPos += pPor->GetLen();
/*N*/           }
/*N*/       }
/*N*/       if( pPor->InFldGrp() )
/*N*/       {
/*?*/           pFld = (SwFldPortion*)pPor;
/*N*/       }
/*N*/       else if( pPor->IsMultiPortion() )
/*N*/       {
/*?*/ #ifdef BIDI
/*?*/             OSL_ENSURE( !pMulti1 || pMulti1->IsBidi(),
/*?*/                     "Nested multiportions are forbidden." );
/*?*/ #else
/*?*/           OSL_ENSURE( !pMulti1, "Nested multiportions are forbidden." );
/*?*/ #endif
/*?*/
/*?*/           pFld = NULL;
/*N*/       }
/*N*/       pPor = pPor->GetPortion();
/*N*/   }
/*N*/   if( pFld && !pFld->HasFollow() )
/*N*/       pFld = NULL;
/*N*/
/*N*/   SwLinePortion *pRest = NULL;
/*N*/   if( pFld )
/*N*/   {
/*?*/       const SwTxtAttr *pHint = GetAttr( nPos1 - 1 );
/*?*/       if( pHint && pHint->Which() == RES_TXTATR_FIELD )
/*?*/       {
/*?*/           pRest = NewFldPortion( GetInfo(), pHint );
/*?*/           if( !pRest->InFldGrp() )
/*?*/           {
/*?*/               delete pRest;
/*?*/               pRest = NULL;
/*?*/           }
/*?*/       }
/*N*/   }
/*N*/   if( !pMulti1 )
/*N*/       return pRest;

/*?*/   return pRest;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
