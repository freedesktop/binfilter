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


// So kann man die Linguistik-Statistik ( (Tmp-Path)\swlingu.stk ) aktivieren:
//#define LINGU_STATISTIK
#ifdef LINGU_STATISTIK
    #include <stdio.h>          // in SwLinguStatistik::DTOR
    #include <stdlib.h>         // getenv()
    #include <time.h>           // clock()
#endif

#include <hintids.hxx>

#include <vcl/svapp.hxx>
#include <bf_svx/langitem.hxx>
#include <bf_svx/scripttypeitem.hxx>
#include <com/sun/star/i18n/WordType.hdl>
#include <com/sun/star/i18n/ScriptType.hdl>

#include <horiornt.hxx>

#include <doc.hxx>      // GetDoc()
#include <txatbase.hxx>
#include <hints.hxx>
#include <ndtxt.hxx>
#include <txtfrm.hxx>
#include <wrong.hxx>
#include <breakit.hxx>
#include <drawfont.hxx> // SwDrawTextInfo
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic2;

#define C2U(cChar) ::rtl::OUString::createFromAscii(cChar)

// Wir ersparen uns in Hyphenate ein GetFrm()
// Achtung: in edlingu.cxx stehen die Variablen!
extern const SwTxtNode *pLinguNode;
extern       SwTxtFrm  *pLinguFrm;

bool lcl_IsSkippableWhiteSpace( sal_Unicode cCh )
{
    return 0x3000 == cCh ||
           ' ' == cCh ||
           '\t' == cCh ||
           0x0a == cCh;
}

/*
 * Ein Zeichen wurde eingefuegt.
 */

/*N*/ SwTxtNode& SwTxtNode::Insert( sal_Unicode c, const SwIndex &rIdx )
/*N*/ {
/*N*/   xub_StrLen nOrigLen = aText.Len();
/*N*/
/*N*/   OSL_ENSURE( rIdx <= nOrigLen, "Array ueberindiziert." );
/*N*/   OSL_ENSURE( nOrigLen < STRING_LEN, "USHRT_MAX ueberschritten." );
/*N*/
/*N*/   if( nOrigLen == aText.Insert( c, rIdx.GetIndex() ).Len() )
/*N*/       return *this;
/*N*/
/*N*/   Update(rIdx,1);
/*N*/
/*N*/   // leere Hints und Feldattribute an rIdx.GetIndex suchen
/*N*/   if( pSwpHints )
/*N*/   {
/*N*/       USHORT* pEndIdx;
/*N*/       for( USHORT i=0; i < pSwpHints->Count() &&
/*N*/               rIdx >= *(*pSwpHints)[i]->GetStart(); ++i)
/*N*/       {
/*N*/           SwTxtAttr *pHt = pSwpHints->GetHt(i);
/*N*/           if( 0 != ( pEndIdx = pHt->GetEnd()) )
/*N*/           {
/*N*/               // leere Hints an rIdx.GetIndex ?
/*N*/               BOOL bEmpty = *pEndIdx == *pHt->GetStart()
/*N*/                           && rIdx == *pHt->GetStart();
/*N*/
/*N*/               if( bEmpty )
/*N*/               {
/*N*/                   pSwpHints->DeleteAtPos(i);
/*N*/                   if( bEmpty )
/*N*/                       *pHt->GetStart() -= 1;
/*N*/                   else
/*N*/                       *pEndIdx -= 1;
/*N*/                   Insert(pHt);
/*N*/               }
/*N*/           }
/*N*/       }
/*N*/       if ( pSwpHints->CanBeDeleted() )
/*N*/           DELETEZ( pSwpHints );
/*N*/   }
/*N*/   // den Frames Bescheid sagen
/*N*/   SwInsChr aHint( rIdx.GetIndex()-1 );
/*N*/   SwModify::Modify( 0, &aHint );
/*N*/   return *this;
/*N*/ }


/*
 * void SwTxtNode::RstAttr(const SwIndex &rIdx, USHORT nLen)
 *
 * loescht alle Attribute ab der Position rIdx ueber eine Laenge
 * von nLen.
 */

/* 5 Faelle:
 * 1) Das Attribut liegt vollstaendig im Bereich:
 *    -> loeschen
 * 2) Das Attributende liegt im Bereich:
 *    -> Loeschen, mit neuem Ende einfuegen
 * 3) Der Attributanfang liegt im Bereich:
 *    -> Loeschen, mit neuem Anfang einfuegen
 * 4) Das Attrib umfasst den Bereich:
 *       Aufsplitten, d.h.
 *    -> Loeschen, mit alten Anfang und Anfang des Bereiches einfuegen
 *    -> Neues Attribut mit Ende des Bereiches und altem Ende einfuegen
 * 5) Das Attribut liegt ausserhalb des Bereiches
 *     -> nichts tun.
 */



/*M*/ void SwTxtNode::RstAttr(const SwIndex &rIdx)
/*M*/ {
/*M*/   // Attribute?
/*M*/   if ( !GetpSwpHints() )
/*M*/       return;
/*M*/
/*M*/   xub_StrLen nStart = rIdx.GetIndex();
/*M*/
/*M*/   BOOL    bChanged = FALSE;
/*M*/
/*M*/   // nMin und nMax werden invers auf das Maximum bzw. Minimum gesetzt.
/*M*/   xub_StrLen nMin = aText.Len();
/*M*/   xub_StrLen nMax = nStart;
/*M*/
/*M*/   if ( pSwpHints && pSwpHints->CanBeDeleted() )
/*M*/       DELETEZ( pSwpHints );

/*M*/   if(bChanged)
/*M*/   {
/*M*/       if ( pSwpHints )
/*M*/       {
/*M*/           pSwpHints->ClearDummies( *this );
/*M*/           ((SwpHintsArr*)pSwpHints)->Resort();
/*M*/           pSwpHints->Merge( *this );
/*M*/       }
/*M*/       //TxtFrm's reagieren auf aHint, andere auf aNew
/*M*/       SwUpdateAttr aHint( nMin, nMax, 0 );
/*M*/       SwModify::Modify( 0, &aHint );
/*M*/       SwFmtChg aNew( GetFmtColl() );
/*M*/       SwModify::Modify( 0, &aNew );
/*M*/   }
/*M*/ }



/*************************************************************************
 *                SwTxtNode::GetCurWord()
 *
 * Aktuelles Wort zurueckliefern:
 * Wir suchen immer von links nach rechts, es wird also das Wort
 * vor nPos gesucht. Es sei denn, wir befinden uns am Anfang des
 * Absatzes, dann wird das erste Wort zurueckgeliefert.
 * Wenn dieses erste Wort nur aus Whitespaces besteht, returnen wir
 * einen leeren String.
 *************************************************************************/





/*M*/ void SwTxtNode::SetWrong( SwWrongList *pNew )
/*M*/ {
/*M*/   delete pWrong;
/*M*/   pWrong = pNew;
/*M*/ }

/*M*/ SwScanner::SwScanner( const SwTxtNode& rNd, const SwWrongList* pWrng,
/*M*/                       USHORT nType, xub_StrLen nStart, xub_StrLen nEnde,
/*M*/                       BOOL bRev, BOOL bOS )
/*M*/     : pWrong( pWrng ), rNode( rNd ), nLen( 0 ), nWordType( nType ),
/*M*/       bReverse( bRev ), bStart( TRUE ), bIsOnlineSpell( bOS )
/*M*/ {
/*M*/     OSL_ENSURE( rNd.GetTxt().Len(), "SwScanner: EmptyString" );
/*M*/   if( bReverse )
/*M*/   {
/*M*/       nBegin = nEnde;
/*M*/       nEndPos = nStart;
/*M*/   }
/*M*/   else
/*M*/   {
/*M*/       nBegin = nStart;
/*M*/       nEndPos = nEnde;
/*M*/   }
/*M*/
/*M*/     aCurrLang = rNd.GetLang( nBegin );
/*M*/ }


/*N*/ BOOL SwScanner::NextWord()
/*N*/ {
/*N*/     OSL_ENSURE( ! bReverse,
/*N*/             "SwScanner::NextWord() currently not implemented for reverse mode" );
/*N*/
/*N*/     nBegin += nLen;
/*N*/
/*N*/     // first we have to skip some whitespace characters
/*N*/     const XubString& rText = rNode.GetTxt();
/*N*/     Boundary aBound;
/*N*/
/*N*/     while ( true )
/*N*/     {
/*N*/
/*N*/     while ( nBegin < rText.Len() &&
/*N*/             lcl_IsSkippableWhiteSpace( rText.GetChar( nBegin ) ) )
/*N*/         ++nBegin;
/*N*/
/*N*/     if ( nBegin >= rText.Len() || nBegin >= nEndPos )
/*N*/         return FALSE;
/*N*/
/*N*/     // get next language in order to find next or previous word
/*N*/     const USHORT nNextScript =
/*N*/             pBreakIt->xBreak->getScriptType( rText, nBegin );
/*N*/     if ( nNextScript != GetI18NScriptTypeOfLanguage( aCurrLang ) )
/*N*/     {
/*N*/         LanguageType aNextLang = rNode.GetLang( nBegin, nNextScript );
/*N*/         aCurrLang = aNextLang;
/*N*/     }
/*N*/
/*N*/     // get the word boundaries
/*N*/     aBound = pBreakIt->xBreak->getWordBoundary( rText, nBegin,
/*N*/             pBreakIt->GetLocale( aCurrLang ), nWordType, sal_True );
/*N*/
/*N*/      //no word boundaries could be found
/*N*/      if(aBound.endPos == aBound.startPos)
/*N*/          return FALSE;
/*N*/
/*N*/      if( nBegin == aBound.endPos )
/*N*/          ++nBegin;
/*N*/      else
/*N*/          break;
/*N*/
/*N*/      } // end while( true )

/*N*/
/*N*/     // we have to differenciate between these cases:
/*N*/     if ( aBound.startPos <= nBegin )
/*N*/     {
/*N*/         OSL_ENSURE( aBound.endPos >= nBegin, "Unexpected aBound result" );
/*N*/
/*N*/         // restrict boundaries to script boundaries and nEndPos
/*N*/         const USHORT nCurrScript =
/*N*/                 pBreakIt->xBreak->getScriptType( rText, nBegin );
/*N*/
/*N*/         XubString aTmpWord = rText.Copy( nBegin, aBound.endPos - nBegin );
/*N*/         const sal_Int32 nScriptEnd = nBegin +
/*N*/             pBreakIt->xBreak->endOfScript( aTmpWord, 0, nCurrScript );
/*N*/         const sal_Int32 nEnd = Min( aBound.endPos, nScriptEnd );
/*N*/
/*N*/         // restrict word start to last script change position
/*N*/         sal_Int32 nScriptBegin = 0;
/*N*/         if ( aBound.startPos < nBegin )
/*N*/         {
/*N*/             // search from nBegin backwards until the next script change
/*N*/             aTmpWord = rText.Copy( aBound.startPos, nBegin - aBound.startPos + 1 );
/*N*/             nScriptBegin = aBound.startPos +
/*N*/                 pBreakIt->xBreak->beginOfScript( aTmpWord, nBegin - aBound.startPos,
/*N*/                                                 nCurrScript );
/*N*/         }
/*N*/
/*N*/         nBegin = (xub_StrLen)Max( aBound.startPos, nScriptBegin );
/*N*/         nLen = (xub_StrLen)(nEnd - nBegin);
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         const USHORT nCurrScript =
/*N*/                 pBreakIt->xBreak->getScriptType( rText, aBound.startPos );
/*N*/         XubString aTmpWord = rText.Copy( aBound.startPos, aBound.endPos - aBound.startPos );
/*N*/         const sal_Int32 nScriptEnd = aBound.startPos +
/*N*/             pBreakIt->xBreak->endOfScript( aTmpWord, 0, nCurrScript );
/*N*/         const sal_Int32 nEnd = Min( aBound.endPos, nScriptEnd );
/*N*/         nBegin = (xub_StrLen)aBound.startPos;
/*N*/         nLen = (xub_StrLen)(nEnd - nBegin);
/*N*/     }
/*N*/
/*N*/     if( ! nLen )
/*N*/         return FALSE;
/*N*/
/*N*/     aWord = rText.Copy( nBegin, nLen );
/*N*/
/*N*/     return TRUE;
/*N*/ }

#ifdef LINGU_STATISTIK

// globale Variable
SwLinguStatistik aSwLinguStat;



#endif

// change text to Upper/Lower/Hiragana/Katagana/...

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
