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

#include <hintids.hxx>

#include <bf_svx/protitem.hxx>

#include <pagefrm.hxx>

#include <horiornt.hxx>

#include <doc.hxx>
#include <docary.hxx>
#include <pamtyp.hxx>
#include <txtfrm.hxx>
#include <section.hxx>
#include <fmtcntnt.hxx>
#include <frmatr.hxx>
#include <crsskip.hxx>
namespace binfilter {


// fuer den dummen ?MSC-? Compiler
/*N*/inline xub_StrLen GetSttOrEnd( BOOL bCondition, const SwCntntNode& rNd )
/*N*/{
/*N*/   return bCondition ? 0 : rNd.Len();
/*N*/}

/*************************************************************************
|*
|*  SwPosition
|*
*************************************************************************/


/*N*/ SwPosition::SwPosition(const SwPosition &rPos)
/*N*/   : nNode(rPos.nNode),nContent(rPos.nContent)
/*N*/ {
/*N*/ }


/*N*/ SwPosition::SwPosition( const SwNodeIndex &rNode, const SwIndex &rCntnt )
/*N*/   : nNode( rNode ),nContent( rCntnt )
/*N*/ {
/*N*/ }

/*N*/ SwPosition::SwPosition( const SwNodeIndex &rNode )
/*N*/   : nNode( rNode ), nContent( 0 )
/*N*/ {
/*N*/ }

/*N*/ SwPosition::SwPosition( const SwNode& rNode )
/*N*/   : nNode( rNode ), nContent( 0 )
/*N*/ {
/*N*/ }


/*N*/ SwPosition &SwPosition::operator=(const SwPosition &rPos)
/*N*/ {
/*N*/   nNode = rPos.nNode;
/*N*/   nContent = rPos.nContent;
/*N*/   return *this;
/*N*/ }


/*N*/ bool SwPosition::operator<(const SwPosition &rPos) const
/*N*/ {
/*N*/   if( nNode < rPos.nNode )
/*N*/       return TRUE;
/*N*/   if( nNode == rPos.nNode )
/*N*/       return ( nContent < rPos.nContent );
/*N*/   return FALSE;
/*N*/ }


/*N*/ bool SwPosition::operator>(const SwPosition &rPos) const
/*N*/ {
/*N*/   if(nNode > rPos.nNode )
/*N*/       return TRUE;
/*N*/   if( nNode == rPos.nNode )
/*N*/       return ( nContent > rPos.nContent );
/*N*/   return FALSE;
/*N*/ }


/*N*/ bool SwPosition::operator<=(const SwPosition &rPos) const
/*N*/ {
/*N*/   if(nNode < rPos.nNode )
/*N*/       return TRUE;
/*N*/   if( nNode == rPos.nNode )
/*N*/       return ( nContent <= rPos.nContent );
/*N*/   return FALSE;
/*N*/ }


/*N*/ bool SwPosition::operator>=(const SwPosition &rPos) const
/*N*/ {
/*N*/   if(nNode > rPos.nNode )
/*N*/       return TRUE;
/*N*/   if( nNode == rPos.nNode )
/*N*/       return ( nContent >= rPos.nContent );
/*N*/   return FALSE;
/*N*/ }


/*N*/ bool SwPosition::operator==(const SwPosition &rPos) const
/*N*/ {
/*N*/   return
/*N*/       ( ( nNode == rPos.nNode ) && ( nContent == rPos.nContent ) ?
/*N*/           TRUE: FALSE);
/*N*/ }


/*N*/ bool SwPosition::operator!=(const SwPosition &rPos) const
/*N*/ {
/*N*/   if( nNode != rPos.nNode )
/*N*/       return TRUE;
/*N*/   return ( nContent != rPos.nContent );
/*N*/ }

enum CHKSECTION { Chk_Both, Chk_One, Chk_None };


/*N*/ CHKSECTION lcl_TstIdx( ULONG nSttIdx, ULONG nEndIdx, const SwNode& rEndNd )
/*N*/ {
/*N*/   ULONG nStt = rEndNd.StartOfSectionIndex(), nEnd = rEndNd.GetIndex();
/*N*/   CHKSECTION eSec = nStt < nSttIdx && nEnd >= nSttIdx ? Chk_One : Chk_None;
/*N*/   if( nStt < nEndIdx && nEnd >= nEndIdx )
/*N*/       return( eSec == Chk_One ? Chk_Both : Chk_One );
/*N*/   return eSec;
/*N*/ }


/*N*/ bool lcl_ChkOneRange( CHKSECTION eSec, BOOL bChkSections,
/*N*/                   const SwNode& rBaseEnd, ULONG nStt, ULONG nEnd )
/*N*/ {
/*N*/   if( eSec != Chk_Both )
/*N*/       return FALSE;
/*N*/
/*N*/   if( !bChkSections )
/*N*/       return TRUE;
/*N*/
/*N*/   // suche die umspannende Section
/*N*/   const SwNodes& rNds = rBaseEnd.GetNodes();
/*N*/   const SwNode *pTmp, *pNd = rNds[ nStt ];
/*N*/   if( !pNd->IsStartNode() )
/*N*/       pNd = pNd->StartOfSectionNode();
/*N*/
/*N*/   if( pNd == rNds[ nEnd ]->StartOfSectionNode() )
/*N*/       return TRUE;        // der gleiche StartNode, die selbe Section
/*N*/
/*N*/   // steht schon auf einem GrundSection Node ? Fehler !!!
/*N*/   if( !pNd->StartOfSectionIndex() )
/*N*/       return FALSE;
/*N*/
/*N*/   while( ( pTmp = pNd->StartOfSectionNode())->EndOfSectionNode() !=
/*N*/           &rBaseEnd )
/*N*/       pNd = pTmp;
/*N*/
/*N*/   ULONG nSttIdx = pNd->GetIndex(), nEndIdx = pNd->EndOfSectionIndex();
/*N*/   return nSttIdx <= nStt && nStt <= nEndIdx &&
/*N*/          nSttIdx <= nEnd && nEnd <= nEndIdx ? TRUE : FALSE;
/*N*/ }


/*N*/ bool CheckNodesRange( const SwNodeIndex& rStt,
/*N*/                       const SwNodeIndex& rEnd, bool bChkSection )
/*N*/ {
/*N*/   const SwNodes& rNds = rStt.GetNodes();
/*N*/   ULONG nStt = rStt.GetIndex(), nEnd = rEnd.GetIndex();
/*N*/   CHKSECTION eSec = lcl_TstIdx( nStt, nEnd, rNds.GetEndOfContent() );
/*N*/   if( Chk_None != eSec ) return eSec == Chk_Both ? TRUE : FALSE;
/*N*/
/*N*/   eSec = lcl_TstIdx( nStt, nEnd, rNds.GetEndOfAutotext() );
/*N*/   if( Chk_None != eSec )
/*N*/       return lcl_ChkOneRange( eSec, bChkSection,
/*N*/                           rNds.GetEndOfAutotext(), nStt, nEnd );
/*N*/
/*N*/   eSec = lcl_TstIdx( nStt, nEnd, rNds.GetEndOfPostIts() );
/*N*/   if( Chk_None != eSec )
/*N*/       return lcl_ChkOneRange( eSec, bChkSection,
/*N*/                           rNds.GetEndOfPostIts(), nStt, nEnd );
/*N*/
/*?*/   eSec = lcl_TstIdx( nStt, nEnd, rNds.GetEndOfInserts() );
/*?*/   if( Chk_None != eSec )
/*?*/       return lcl_ChkOneRange( eSec, bChkSection,
/*?*/                           rNds.GetEndOfInserts(), nStt, nEnd );
/*?*/
/*?*/   eSec = lcl_TstIdx( nStt, nEnd, rNds.GetEndOfRedlines() );
/*?*/   if( Chk_None != eSec )
/*?*/       return lcl_ChkOneRange( eSec, bChkSection,
/*?*/                           rNds.GetEndOfRedlines(), nStt, nEnd );
/*?*/
/*?*/   return FALSE;       // liegt irgendwo dazwischen, FEHLER
/*N*/ }


/*N*/ bool GoNext(SwNode* pNd, SwIndex * pIdx, USHORT nMode )
/*N*/ {
/*N*/   if( pNd->IsCntntNode() )
/*N*/       return ((SwCntntNode*)pNd)->GoNext( pIdx, nMode );
/*N*/   return FALSE;
/*N*/ }


/*N*/ bool GoPrevious( SwNode* pNd, SwIndex * pIdx, USHORT nMode )
/*N*/ {
/*N*/   if( pNd->IsCntntNode() )
/*N*/       return ((SwCntntNode*)pNd)->GoPrevious( pIdx, nMode );
/*N*/   return FALSE;
/*N*/ }


/*N*/ SwCntntNode* GoNextNds( SwNodeIndex* pIdx, bool bChk )
/*N*/ {
/*N*/   SwNodeIndex aIdx( *pIdx );
/*N*/   SwCntntNode* pNd = aIdx.GetNodes().GoNext( &aIdx );
/*N*/   if( pNd )
/*N*/   {
/*N*/       if( bChk && 1 != aIdx.GetIndex() - pIdx->GetIndex() &&
/*N*/           !CheckNodesRange( *pIdx, aIdx, TRUE ) )
/*N*/               pNd = 0;
/*N*/       else
/*N*/           *pIdx = aIdx;
/*N*/   }
/*N*/   return pNd;
/*N*/ }


/*N*/ SwCntntNode* GoPreviousNds( SwNodeIndex * pIdx, bool bChk )
/*N*/ {
/*N*/   SwNodeIndex aIdx( *pIdx );
/*N*/   SwCntntNode* pNd = aIdx.GetNodes().GoPrevious( &aIdx );
/*N*/   if( pNd )
/*N*/   {
/*N*/       if( bChk && 1 != pIdx->GetIndex() - aIdx.GetIndex() &&
/*N*/           !CheckNodesRange( *pIdx, aIdx, TRUE ) )
/*N*/               pNd = 0;
/*N*/       else
/*N*/           *pIdx = aIdx;
/*N*/   }
/*N*/   return pNd;
/*N*/ }

// ----------------------------------------------------------------------

/*************************************************************************
|*
|*  SwPointAndMark
|*
*************************************************************************/

/*N*/ SwPaM::SwPaM( const SwPosition& rPos, SwPaM* pRing )
/*N*/   : Ring( pRing )
/*N*/   , aBound1( rPos )
/*N*/   , aBound2( rPos )
/*N*/ {
/*N*/   pPoint = pMark = &aBound1;
/*N*/ }

/*N*/ SwPaM::SwPaM( const SwPosition& rMk, const SwPosition& rPt, SwPaM* pRing )
/*N*/   : Ring( pRing )
/*N*/   , aBound1( rMk )
/*N*/   , aBound2( rPt )
/*N*/ {
/*N*/   pMark = &aBound1;
/*N*/   pPoint = &aBound2;
/*N*/ }


/*N*/ SwPaM::SwPaM( const SwNode& rMk, const SwNode& rPt,
/*N*/               long nMkOffset, long nPtOffset, SwPaM* pRing )
/*N*/   : Ring( pRing )
/*N*/   , aBound1( rMk )
/*N*/   , aBound2( rPt )
/*N*/ {
/*N*/   if( nMkOffset )
/*N*/       aBound1.nNode += nMkOffset;
/*N*/   if( nPtOffset )
/*N*/       aBound2.nNode += nPtOffset;
/*N*/
/*N*/   aBound1.nContent.Assign( aBound1.nNode.GetNode().GetCntntNode(), 0 );
/*N*/   aBound2.nContent.Assign( aBound2.nNode.GetNode().GetCntntNode(), 0 );
/*N*/   pMark = &aBound1;
/*N*/   pPoint = &aBound2;
/*N*/ }


/*N*/ SwPaM::SwPaM( const SwNode& rMk, xub_StrLen nMkCntnt,
/*N*/             const SwNode& rPt, xub_StrLen nPtCntnt, SwPaM* pRing )
/*N*/   : Ring( pRing )
/*N*/   , aBound1( rMk )
/*N*/   , aBound2( rPt )
/*N*/ {
/*N*/   aBound1.nContent.Assign( aBound1.nNode.GetNode().GetCntntNode(), nMkCntnt );
/*N*/   aBound2.nContent.Assign( aBound2.nNode.GetNode().GetCntntNode(), nPtCntnt );
/*N*/   pMark = &aBound1;
/*N*/   pPoint = &aBound2;
/*N*/ }

/*N*/ SwPaM::SwPaM( SwPaM &rPam )
/*N*/   : Ring( &rPam )
/*N*/   , aBound1( *(rPam.pPoint) )
/*N*/   , aBound2( *(rPam.pMark) )
/*N*/ {
/*N*/   pPoint = &aBound1;
/*N*/   pMark  = rPam.HasMark() ? &aBound2 : pPoint;
/*N*/ }

/*N*/ SwPaM::SwPaM( const SwNode& rNd, xub_StrLen nCntnt, SwPaM* pRing )
/*N*/   : Ring( pRing )
/*N*/   , aBound1( rNd )
/*N*/   , aBound2( rNd )
/*N*/ {
/*N*/   aBound1.nContent.Assign( aBound1.nNode.GetNode().GetCntntNode(), nCntnt );
/*N*/   aBound2.nContent = aBound1.nContent;
/*N*/   pPoint = pMark = &aBound1;
/*N*/ }

/*N*/ SwPaM::SwPaM( const SwNodeIndex& rNd, xub_StrLen nCntnt, SwPaM* pRing )
/*N*/   : Ring( pRing )
/*N*/   , aBound1( rNd )
/*N*/   , aBound2( rNd )
/*N*/ {
/*N*/   aBound1.nContent.Assign( rNd.GetNode().GetCntntNode(), nCntnt );
/*N*/   aBound2.nContent = aBound1.nContent;
/*N*/   pPoint = pMark = &aBound1;
/*N*/ }

/*N*/ SwPaM::~SwPaM() {}

/*N*/ void SwPaM::SetMark()
/*N*/ {
/*N*/   if(pPoint == &aBound1)
/*N*/       pMark = &aBound2;
/*N*/   else
/*N*/       pMark = &aBound1;
/*N*/   (*pMark) = (*pPoint);
/*N*/ }

/*N*/ #ifdef DBG_UTIL

/*N*/ void SwPaM::Exchange()
/*N*/ {
/*N*/   if(pPoint != pMark)
/*N*/   {
/*N*/       SwPosition *pTmp = pPoint;
/*N*/       pPoint = pMark;
/*N*/       pMark = pTmp;
/*N*/   }
/*N*/ }
/*N*/ #endif


/*?*/SwPaM &SwPaM::operator=( SwPaM &rPam )
/*?*/{
/*?*/   *pPoint = *( rPam.pPoint );
/*?*/   if( rPam.HasMark() )
/*?*/   {
/*?*/       SetMark();
/*?*/       *pMark = *( rPam.pMark );
/*?*/   }
/*?*/   else
/*?*/       DeleteMark();
/*?*/   return *this;
/*?*/}

// Bewegen des Cursors


/*N*/ bool SwPaM::Move( SwMoveFn fnMove, SwGoInDoc fnGo )
/*N*/ {
/*N*/   return (*fnGo)( *this, fnMove );
/*N*/ }


/*************************************************************************
|*
|*    void SwPaM::MakeRegion( SwMoveFn, SwPaM*, const SwPaM* )
|*
|*    Beschreibung      Setzt den 1. SwPaM auf den uebergebenen SwPaM
|*                      oder setzt auf den Anfang oder Ende vom Document.
|*                      SPoint bleibt auf der Position stehen, GetMark aendert
|*                      sich entsprechend !
|*
|*    Parameter         SwDirection     gibt an, ob an Anfang / Ende
|*                      SwPaM *         der zu setzende Bereich
|*                      const SwPaM&    der enventuell vorgegeben Bereich
|*    Return-Werte      SwPaM*          der entsprehend neu gesetzte Bereich
|*
*************************************************************************/




/*N*/ USHORT SwPaM::GetPageNum( BOOL bAtPoint, const Point* pLayPos )
/*N*/ {
/*N*/   // return die Seitennummer am Cursor
/*N*/   // (fuer Reader + Seitengebundene Rahmen)
/*N*/   const SwCntntFrm* pCFrm;
/*N*/   const SwPageFrm *pPg;
/*N*/   const SwCntntNode *pNd ;
/*N*/   const SwPosition* pPos = bAtPoint ? pPoint : pMark;
/*N*/
/*N*/   if( 0 != ( pNd = pPos->nNode.GetNode().GetCntntNode() ) &&
/*N*/       0 != ( pCFrm = pNd->GetFrm( pLayPos, pPos, FALSE )) &&
/*N*/       0 != ( pPg = pCFrm->FindPageFrm() ))
/*N*/       return pPg->GetPhyPageNum();
/*N*/   return 0;
/*N*/ }

//--------------------  Suche nach Formaten( FormatNamen ) -----------------

// die Funktion gibt in Suchrichtung den folgenden Node zurueck.
// Ist in der Richtung keiner mehr vorhanden oder ist dieser ausserhalb
// des Bereiches, wird ein 0 Pointer returnt.
// Das rbFirst gibt an, ob es man zu erstenmal einen Node holt. Ist das der
// Fall, darf die Position vom Pam nicht veraendert werden!



// ----------------------------------------------------------------------

// hier folgen die Move-Methoden ( Foward, Backward; Content, Node, Doc )


/*N*/ void GoStartDoc( SwPosition * pPos )
/*N*/ {
/*N*/   SwNodes& rNodes = pPos->nNode.GetNodes();
/*N*/   pPos->nNode = *rNodes.GetEndOfContent().StartOfSectionNode();
/*N*/   // es muss immer ein ContentNode gefunden werden !!
/*N*/   SwCntntNode* pCNd = rNodes.GoNext( &pPos->nNode );
/*N*/   if( pCNd )
/*N*/       pCNd->MakeStartIndex( &pPos->nContent );
/*N*/ }


/*N*/ void GoEndDoc( SwPosition * pPos )
/*N*/ {
/*N*/   SwNodes& rNodes = pPos->nNode.GetNodes();
/*N*/   pPos->nNode = rNodes.GetEndOfContent();
/*N*/   SwCntntNode* pCNd = GoPreviousNds( &pPos->nNode, TRUE );
/*N*/   if( pCNd )
/*N*/       pCNd->MakeEndIndex( &pPos->nContent );
/*N*/ }


// These are used in bf_sw/source/core/crsr/sw_paminit.cxx
bool GoInCntntCells( SwPaM & /*rPam*/, SwMoveFn /*fnMove*/ ) { return FALSE; } // DBG_BF_ASSERT
bool GoPrevPara( SwPaM & /*rPam*/, SwPosPara /*aPosPara*/) { return FALSE; } // DBG_BF_ASSERT


/*N*/ bool GoInDoc( SwPaM & rPam, SwMoveFn fnMove )
/*N*/ {
/*N*/   (*fnMove->fnDoc)( rPam.GetPoint() );
/*N*/   return TRUE;
/*N*/ }


/*N*/ bool GoInNode( SwPaM & rPam, SwMoveFn fnMove )
/*N*/ {
/*N*/   SwCntntNode *pNd = (*fnMove->fnNds)( &rPam.GetPoint()->nNode, TRUE );
/*N*/   if( pNd )
/*N*/       rPam.GetPoint()->nContent.Assign( pNd,
/*N*/                       ::binfilter::GetSttOrEnd( fnMove == fnMoveForward, *pNd ) );
/*N*/   return 0 != pNd;
/*N*/ }


/*N*/ bool GoInCntnt( SwPaM & rPam, SwMoveFn fnMove )
/*N*/ {
/*N*/   if( (*fnMove->fnNd)( &rPam.GetPoint()->nNode.GetNode(),
/*N*/                       &rPam.GetPoint()->nContent, CRSR_SKIP_CHARS ))
/*N*/       return TRUE;
/*N*/   return GoInNode( rPam, fnMove );
/*N*/ }

// --------- Funktionsdefinitionen fuer die SwCrsrShell --------------

/*N*/ bool GoCurrPara( SwPaM & rPam, SwPosPara aPosPara )
/*N*/ {
/*N*/   SwPosition& rPos = *rPam.GetPoint();
/*N*/   SwCntntNode * pNd = rPos.nNode.GetNode().GetCntntNode();
/*N*/   if( pNd )
/*N*/   {
/*N*/       xub_StrLen nOld = rPos.nContent.GetIndex(),
/*N*/                  nNew = aPosPara == fnMoveForward ? 0 : pNd->Len();
/*N*/       // stand er schon auf dem Anfang/Ende dann zum naechsten/vorherigen
/*N*/       if( nOld != nNew )
/*N*/       {
/*N*/           rPos.nContent.Assign( pNd, nNew );
/*N*/           return TRUE;
/*N*/       }
/*N*/   }
/*N*/   // den Node noch etwas bewegen ( auf den naechsten/vorh. CntntNode)
/*N*/   if( ( aPosPara==fnParaStart && 0 != ( pNd =
/*N*/           GoPreviousNds( &rPos.nNode, TRUE ))) ||
/*N*/       ( aPosPara==fnParaEnd && 0 != ( pNd =
/*N*/           GoNextNds( &rPos.nNode, TRUE ))) )
/*N*/   {
/*N*/       rPos.nContent.Assign( pNd,
/*N*/                       ::binfilter::GetSttOrEnd( aPosPara == fnMoveForward, *pNd ));
/*N*/       return TRUE;
/*N*/   }
/*N*/   return FALSE;
/*N*/ }


/*N*/ bool GoNextPara( SwPaM & rPam, SwPosPara aPosPara )
/*N*/ {
/*N*/   if( rPam.Move( fnMoveForward, fnGoNode ) )
/*N*/   {
/*N*/       // steht immer auf einem ContentNode !
/*N*/       SwPosition& rPos = *rPam.GetPoint();
/*N*/       SwCntntNode * pNd = rPos.nNode.GetNode().GetCntntNode();
/*N*/       rPos.nContent.Assign( pNd,
/*N*/                       ::binfilter::GetSttOrEnd( aPosPara == fnMoveForward, *pNd ) );
/*N*/       return TRUE;
/*N*/   }
/*N*/   return FALSE;
/*N*/ }



/*N*/ bool GoCurrSection( SwPaM & rPam, SwMoveFn fnMove )
/*N*/ {
/*N*/   SwPosition& rPos = *rPam.GetPoint();
/*N*/   SwPosition aSavePos( rPos );        // eine Vergleichsposition
/*N*/   SwNodes& rNds = aSavePos.nNode.GetNodes();
/*N*/   (rNds.*fnMove->fnSection)( &rPos.nNode );
/*N*/   SwCntntNode *pNd;
/*N*/   if( 0 == ( pNd = rPos.nNode.GetNode().GetCntntNode()) &&
/*N*/       0 == ( pNd = (*fnMove->fnNds)( &rPos.nNode, TRUE )) )
/*N*/   {
/*?*/       rPos = aSavePos;        // Cusror nicht veraendern
/*?*/       return FALSE;
/*N*/   }
/*N*/
/*N*/   rPos.nContent.Assign( pNd,
/*N*/                       ::binfilter::GetSttOrEnd( fnMove == fnMoveForward, *pNd ) );
/*N*/   return aSavePos != rPos;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
