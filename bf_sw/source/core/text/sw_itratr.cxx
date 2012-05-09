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

#include <horiornt.hxx>

#include <rootfrm.hxx>
#include <itrtxt.hxx>
#include <com/sun/star/i18n/ScriptType.hdl>
namespace binfilter {

using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star;

extern BYTE WhichFont( xub_StrLen nIdx, const String* pTxt,
                       const SwScriptInfo* pSI );

/*************************************************************************
 *                      SwAttrIter::Chg()
 *************************************************************************/

/*N*/ void SwAttrIter::Chg( SwTxtAttr *pHt )
/*N*/ {
/*N*/   OSL_ENSURE( pHt && pFnt, "No attribute of font available for change");
/*N*/   aAttrHandler.PushAndChg( *pHt, *pFnt );
/*N*/   nChgCnt++;
/*N*/ }

/*************************************************************************
 *                      SwAttrIter::Rst()
 *************************************************************************/

/*N*/ void SwAttrIter::Rst( SwTxtAttr *pHt )
/*N*/ {
/*N*/   OSL_ENSURE( pHt && pFnt, "No attribute of font available for reset");
/*N*/   // get top from stack after removing pHt
/*N*/   aAttrHandler.PopAndChg( *pHt, *pFnt );
/*N*/   nChgCnt--;
/*N*/ }

/*************************************************************************
 *              virtual SwAttrIter::~SwAttrIter()
 *************************************************************************/

/*N*/ SwAttrIter::~SwAttrIter()
/*N*/ {
/*N*/   delete pFnt;
/*N*/ }

/*************************************************************************
 *                      SwAttrIter::GetAttr()
 *
 * Liefert fuer eine Position das Attribut, wenn das Attribut genau auf
 * der Position nPos liegt und kein EndIndex besitzt.
 * GetAttr() wird fuer Attribute benoetigt, die die Formatierung beeinflussen
 * sollen, ohne dabei den Inhalt des Strings zu veraendern. Solche "entarteten"
 * Attribute sind z.B. Felder (die expandierten Text bereit halten) und
 * zeilengebundene Frames. Um Mehrdeutigkeiten zwischen verschiedenen
 * solcher Attribute zu vermeiden, werden beim Anlegen eines Attributs
 * an der Startposition ein Sonderzeichen in den String einfuegt.
 * Der Formatierer stoesst auf das Sonderzeichen und holt sich per
 * GetAttr() das entartete Attribut.
 *************************************************************************/

/*N*/ SwTxtAttr *SwAttrIter::GetAttr( const xub_StrLen nPos1 ) const
/*N*/ {
/*N*/   if( pHints )
/*N*/   {
/*N*/       for( MSHORT i = 0; i < pHints->Count(); ++i )
/*N*/       {
/*N*/           SwTxtAttr *pPos = pHints->GetHt(i);
/*N*/           xub_StrLen nStart = *pPos->GetStart();
/*N*/           if( nPos1 < nStart )
/*N*/               return 0;
/*N*/           if( nPos1 == nStart && !pPos->GetEnd() )
/*N*/               return pPos;
/*N*/       }
/*N*/   }
/*N*/   return 0;
/*N*/ }

/*************************************************************************
 *                        SwAttrIter::SeekAndChg()
 *************************************************************************/

/*N*/ sal_Bool SwAttrIter::SeekAndChg( const xub_StrLen nNewPos, OutputDevice *pOut )
/*N*/ {
/*N*/   sal_Bool bChg = nStartIndex && nNewPos == nPos ? pFnt->IsFntChg() : Seek( nNewPos );
/*N*/   if ( pLastOut != pOut )
/*N*/   {
/*N*/       pLastOut = pOut;
/*N*/       pFnt->SetFntChg( sal_True );
/*N*/       bChg = sal_True;
/*N*/   }
/*N*/   if( bChg )
/*N*/   {
/*N*/       // wenn der Aenderungszaehler auf Null ist, kennen wir die MagicNo
/*N*/       // des gewuenschten Fonts ...
/*N*/       if ( !nChgCnt && !nPropFont )
/*N*/           pFnt->SetMagic( aMagicNo[ pFnt->GetActual() ],
/*N*/               aFntIdx[ pFnt->GetActual() ], pFnt->GetActual() );
/*N*/       pFnt->ChgPhysFnt( pShell, pOut );
/*N*/   }
/*N*/   return bChg;
/*N*/ }


/*************************************************************************
 *                        SwAttrIter::SeekStartAndChg()
 *************************************************************************/

/*N*/ sal_Bool SwAttrIter::SeekStartAndChg( OutputDevice *pOut, const sal_Bool bParaFont )
/*N*/ {
/*N*/
/*N*/     // reset font to its original state
/*N*/     aAttrHandler.Reset();
/*N*/     aAttrHandler.ResetFont( *pFnt );
/*N*/
/*N*/     nStartIndex = nEndIndex = nPos = nChgCnt = 0;
/*N*/   if( nPropFont )
/*?*/       pFnt->SetProportion( nPropFont );
/*N*/
/*N*/   if ( pHints && !bParaFont )
/*N*/   {
/*N*/       SwTxtAttr *pTxtAttr;
/*N*/       // Solange wir noch nicht am Ende des StartArrays angekommen sind &&
/*N*/       // das TextAttribut an Position 0 beginnt ...
/*?*/       while ( ( nStartIndex < pHints->GetStartCount() ) &&
/*?*/               !(*(pTxtAttr=pHints->GetStart(nStartIndex))->GetStart()) )
/*?*/       {
/*?*/           // oeffne die TextAttribute
/*?*/           Chg( pTxtAttr );
/*?*/           nStartIndex++;
/*?*/       }
/*N*/   }
/*N*/
/*N*/   register sal_Bool bChg = pFnt->IsFntChg();
/*N*/   if ( pLastOut != pOut )
/*N*/   {
/*N*/       pLastOut = pOut;
/*N*/       pFnt->SetFntChg( sal_True );
/*N*/       bChg = sal_True;
/*N*/   }
/*N*/   if( bChg )
/*N*/   {
/*N*/       // wenn der Aenderungszaehler auf Null ist, kennen wir die MagicNo
/*N*/       // des gewuenschten Fonts ...
/*N*/       if ( !nChgCnt && !nPropFont )
/*N*/           pFnt->SetMagic( aMagicNo[ pFnt->GetActual() ],
/*N*/               aFntIdx[ pFnt->GetActual() ], pFnt->GetActual() );
/*N*/       pFnt->ChgPhysFnt( pShell, pOut );
/*N*/   }
/*N*/   return bChg;
/*N*/ }

/*************************************************************************
 *                       SwAttrIter::SeekFwd()
 *************************************************************************/

// AMA: Neuer AttrIter Nov 94

/*N*/ void SwAttrIter::SeekFwd( const xub_StrLen nNewPos )
/*N*/ {
/*N*/   SwTxtAttr *pTxtAttr;
/*N*/
/*N*/   if ( nStartIndex ) // wenn ueberhaupt schon Attribute geoeffnet wurden...
/*N*/   {
/*N*/       // Schliesse Attr, die z. Z. geoeffnet sind, vor nNewPos+1 aber enden.
/*N*/
/*N*/       // Solange wir noch nicht am Ende des EndArrays angekommen sind &&
/*N*/       // das TextAttribut vor oder an der neuen Position endet ...
/*N*/       while ( ( nEndIndex < pHints->GetEndCount() ) &&
/*N*/               (*(pTxtAttr=pHints->GetEnd(nEndIndex))->GetAnyEnd()<=nNewPos))
/*N*/       {
/*N*/           // schliesse die TextAttribute, deren StartPos vor
/*N*/           // oder an der alten nPos lag, die z.Z. geoeffnet sind.
/*N*/           if (*pTxtAttr->GetStart() <= nPos)  Rst( pTxtAttr );
/*N*/           nEndIndex++;
/*N*/       }
/*N*/   }
/*N*/   else // ueberlies die nicht geoeffneten Enden
/*N*/   {
/*N*/       while ( ( nEndIndex < pHints->GetEndCount() ) &&
/*N*/               (*(pTxtAttr=pHints->GetEnd(nEndIndex))->GetAnyEnd()<=nNewPos))
/*N*/       {
/*N*/           nEndIndex++;
/*N*/       }
/*N*/   }
/*N*/   // Solange wir noch nicht am Ende des StartArrays angekommen sind &&
/*N*/   // das TextAttribut vor oder an der neuen Position beginnt ...
/*N*/   while ( ( nStartIndex < pHints->GetStartCount() ) &&
/*N*/          (*(pTxtAttr=pHints->GetStart(nStartIndex))->GetStart()<=nNewPos))
/*N*/   {
/*N*/       // oeffne die TextAttribute, deren Ende hinter der neuen Position liegt
/*N*/       if ( *pTxtAttr->GetAnyEnd() > nNewPos )  Chg( pTxtAttr );
/*N*/       nStartIndex++;
/*N*/   }
/*N*/
/*N*/ }

/*************************************************************************
 *                       SwAttrIter::Seek()
 *************************************************************************/

/*N*/ sal_Bool SwAttrIter::Seek( const xub_StrLen nNewPos )
/*N*/ {
/*N*/
/*N*/   if( pHints )
/*N*/   {
/*N*/       if( !nNewPos || nNewPos < nPos )
/*N*/       {
/*N*/             // reset font to its original state
/*N*/             aAttrHandler.Reset();
/*N*/             aAttrHandler.ResetFont( *pFnt );
/*N*/
/*N*/             if( nPropFont )
/*?*/               pFnt->SetProportion( nPropFont );
/*N*/           nStartIndex = nEndIndex = nPos = 0;
/*N*/           nChgCnt = 0;
/*N*/       }
/*N*/       SeekFwd( nNewPos );
/*N*/   }
/*N*/
/*N*/     pFnt->SetActual( WhichFont( nNewPos, 0, pScriptInfo ) );
/*N*/
/*N*/   nPos = nNewPos;
/*N*/
/*N*/   if( nPropFont )
/*?*/       pFnt->SetProportion( nPropFont );
/*N*/
/*N*/   return pFnt->IsFntChg();
/*N*/ }

/*************************************************************************
 *                      SwAttrIter::GetNextAttr()
 *************************************************************************/

/*N*/ xub_StrLen SwAttrIter::GetNextAttr( ) const
/*N*/ {
/*N*/   xub_StrLen nNext = STRING_LEN;
/*N*/   if( pHints )
/*N*/   {
/*N*/       if (pHints->GetStartCount() > nStartIndex) // Gibt es noch Starts?
/*N*/          nNext = (*pHints->GetStart(nStartIndex)->GetStart());
/*N*/       if (pHints->GetEndCount() > nEndIndex) // Gibt es noch Enden?
/*N*/       {
/*N*/           xub_StrLen nNextEnd = (*pHints->GetEnd(nEndIndex)->GetAnyEnd());
/*N*/           if ( nNextEnd<nNext ) nNext = nNextEnd; // Wer ist naeher?
/*N*/       }
/*N*/   }
/*N*/   return nNext;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
