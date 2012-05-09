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

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#include <hintids.hxx>

#include <bf_svx/protitem.hxx>
#include <bf_sch/schdll.hxx>
#include <bf_sch/memchrt.hxx>

#include <fmtfsize.hxx>
#include <frmatr.hxx>

#include <horiornt.hxx>

#include <crsrsh.hxx>
#include <doc.hxx>
#include <ndole.hxx>
#include <cntfrm.hxx>
#include <tabfrm.hxx>
#include <rowfrm.hxx>
#include <cellfrm.hxx>
#include <viscrs.hxx>
#include <swtblfmt.hxx>
namespace binfilter {

//siehe auch swtable.cxx
#define COLFUZZY 20L

// defines, die bestimmen, wie Tabellen Boxen gemergt werden:
//  - 1. alle leeren Zeilen entfernen, alle Boxen werden mit Blank,
//      alle Lines mit ParaBreak getrennt
//  - 2. alle leeren Zeilen und alle leeren Boxen am Anfang und Ende
//      entfernen, alle Boxen werden mit Blank,
//      alle Lines mit ParaBreak getrennt
//  - 3. alle leeren Boxen entfernen, alle Boxen werden mit Blank,
//      alle Lines mit ParaBreak getrennt

#undef      DEL_ONLY_EMPTY_LINES
#undef      DEL_EMPTY_BOXES_AT_START_AND_END
#define     DEL_ALL_EMPTY_BOXES


/*N*/ _SV_IMPL_SORTAR_ALG( SwSelBoxes, SwTableBoxPtr )
/*N*/ BOOL SwSelBoxes::Seek_Entry( const SwTableBoxPtr rSrch, USHORT* pFndPos ) const
/*N*/ {
/*N*/   ULONG nIdx = rSrch->GetSttIdx();
/*N*/
/*N*/   register USHORT nO = Count(), nM, nU = 0;
/*N*/   if( nO > 0 )
/*N*/   {
/*N*/       nO--;
/*N*/       while( nU <= nO )
/*N*/       {
/*N*/           nM = nU + ( nO - nU ) / 2;
/*N*/           if( (*this)[ nM ]->GetSttNd() == rSrch->GetSttNd() )
/*N*/           {
/*N*/               if( pFndPos )
/*N*/                   *pFndPos = nM;
/*N*/               return TRUE;
/*N*/           }
/*N*/           else if( (*this)[ nM ]->GetSttIdx() < nIdx )
/*N*/               nU = nM + 1;
/*N*/           else if( nM == 0 )
/*N*/           {
/*N*/               if( pFndPos )
/*N*/                   *pFndPos = nU;
/*N*/               return FALSE;
/*N*/           }
/*N*/           else
/*N*/               nO = nM - 1;
/*N*/       }
/*N*/   }
/*N*/   if( pFndPos )
/*N*/       *pFndPos = nU;
/*N*/   return FALSE;
/*N*/ }


/*N*/ SV_IMPL_PTRARR( _FndBoxes, _FndBox* )
/*N*/ SV_IMPL_PTRARR( _FndLines, _FndLine* )


/*N*/ struct _Sort_CellFrm
/*N*/ {
/*N*/   const SwCellFrm* pFrm;
/*N*/
/*N*/     _Sort_CellFrm( const SwCellFrm& rCFrm )
/*N*/         : pFrm( &rCFrm ) {}
/*N*/ };

/*N*/ SV_DECL_VARARR( _Sort_CellFrms, _Sort_CellFrm, 16, 16 )
/*N*/ SV_IMPL_VARARR( _Sort_CellFrms, _Sort_CellFrm )

/*N*/ SV_IMPL_PTRARR( SwChartBoxes, SwTableBoxPtr );
/*N*/ SV_IMPL_PTRARR( SwChartLines, SwChartBoxes* );

/*N*/ const SwLayoutFrm *lcl_FindCellFrm( const SwLayoutFrm *pLay )
/*N*/ {
/*N*/   while ( pLay && !pLay->IsCellFrm() )
/*?*/       pLay = pLay->GetUpper();
/*N*/   return pLay;
/*N*/ }

/*N*/ const SwLayoutFrm *lcl_FindNextCellFrm( const SwLayoutFrm *pLay )
/*N*/ {
/*N*/   //Dafuer sorgen, dass die Zelle auch verlassen wird (Bereiche)
/*N*/   const SwLayoutFrm *pTmp = pLay;
/*N*/   do {
/*N*/       pTmp = pTmp->GetNextLayoutLeaf();
/*N*/   } while( pLay->IsAnLower( pTmp ) );
/*N*/
/*N*/   while( pTmp && !pTmp->IsCellFrm() )
/*N*/       pTmp = pTmp->GetUpper();
/*N*/   return pTmp;
/*N*/ }

/*N*/ void GetTblSel( const SwLayoutFrm* pStart, const SwLayoutFrm* pEnd,
/*N*/               SwSelBoxes& rBoxes, const SwTblSearchType eSearchType )
/*N*/ {
/*N*/   //Muss ein HeadlineRepeat beachtet werden?
/*N*/   const BOOL bRepeat = pStart->FindTabFrm()->GetTable()->IsHeadlineRepeat();
/*N*/   int bChkProtected = TBLSEARCH_PROTECT & eSearchType;
/*N*/
/*N*/   BOOL bTblIsValid;
/*N*/   int nLoopMax = 100;     //JP 28.06.99: max 100 loops - Bug 67292
/*N*/   do {
/*N*/       bTblIsValid = TRUE;
/*N*/
/*N*/       //Zuerst lassen wir uns die Tabellen und die Rechtecke heraussuchen.
/*N*/       SwSelUnions aUnions;
/*N*/       ::binfilter::MakeSelUnions( aUnions, pStart, pEnd, eSearchType );
/*N*/
/*N*/       //Jetzt zu jedem Eintrag die Boxen herausfischen und uebertragen.
            USHORT i=0;
/*N*/       for( i = 0; i < aUnions.Count() && bTblIsValid; ++i )
/*N*/       {
/*N*/           SwSelUnion *pUnion = aUnions[i];
/*N*/           const SwTabFrm *pTable = pUnion->GetTable();
/*N*/           if( !pTable->IsValid() && nLoopMax )
/*N*/           {
/*?*/               bTblIsValid = FALSE;
/*?*/               break;
/*N*/           }
/*N*/
/*N*/           const SwLayoutFrm *pRow = (const SwLayoutFrm*)pTable->Lower();
/*N*/           //Wenn die Row eine wiederholte Headline ist wird sie nicht
/*N*/           //beachtet.
/*N*/           if( bRepeat && pTable->IsFollow() )
/*?*/               pRow = (const SwLayoutFrm*)pRow->GetNext();
/*N*/
/*N*/           while( pRow && bTblIsValid )
/*N*/           {
/*N*/               if( !pRow->IsValid() && nLoopMax )
/*N*/               {
/*?*/                   bTblIsValid = FALSE;
/*?*/                   break;
/*N*/               }
/*N*/
/*N*/               if ( pRow->Frm().IsOver( pUnion->GetUnion() ) )
/*N*/               {
/*N*/                   const SwLayoutFrm *pCell = pRow->FirstCell();
/*N*/
/*N*/                   while( bTblIsValid && pCell && pRow->IsAnLower( pCell ) )
/*N*/                   {
/*N*/                       if( !pCell->IsValid() && nLoopMax )
/*N*/                       {
/*?*/                           bTblIsValid = FALSE;
/*?*/                           break;
/*N*/                       }
/*N*/
/*N*/                       OSL_ENSURE( pCell->IsCellFrm(), "Frame ohne Celle" );
/*N*/                       if( ::binfilter::IsFrmInTblSel( pUnion->GetUnion(), pCell ) )
/*N*/                       {
/*N*/                           SwTableBox* pBox = (SwTableBox*)
/*N*/                               ((SwCellFrm*)pCell)->GetTabBox();
/*N*/                           // Zellenschutzt beachten ??
/*N*/                           if( !bChkProtected ||
/*N*/                               !pBox->GetFrmFmt()->GetProtect().IsCntntProtected() )
/*N*/                               rBoxes.Insert( pBox );
/*N*/                       }
/*N*/                       if ( pCell->GetNext() )
/*N*/                       {
/*N*/                           pCell = (const SwLayoutFrm*)pCell->GetNext();
/*N*/                           if ( pCell->Lower()->IsRowFrm() )
/*?*/                               pCell = pCell->FirstCell();
/*N*/                       }
/*N*/                       else
/*N*/                           pCell = ::binfilter::lcl_FindNextCellFrm( pCell );
/*N*/                   }
/*N*/               }
/*N*/               pRow = (const SwLayoutFrm*)pRow->GetNext();
/*N*/           }
/*N*/       }
/*N*/
/*N*/       if( bTblIsValid )
/*N*/           break;
/*N*/
/*N*/       // ansonsten das Layout der Tabelle kurz "kalkulieren" lassen
/*N*/       // und nochmals neu aufsetzen
/*?*/       SwTabFrm *pTable = aUnions[0]->GetTable();
/*?*/       for( i = 0; i < aUnions.Count(); ++i )
/*?*/       {
/*?*/           if( pTable->IsValid() )
/*?*/               pTable->InvalidatePos();
/*?*/           pTable->SetONECalcLowers();
/*?*/           pTable->Calc();
/*?*/           pTable->SetCompletePaint();
/*?*/           if( 0 == (pTable = pTable->GetFollow()) )
/*?*/               break;
/*?*/       }
/*?*/       i = 0;
/*?*/       rBoxes.Remove( i, rBoxes.Count() );
/*?*/       --nLoopMax;
/*?*/
/*?*/   } while( TRUE );
/*N*/   OSL_ENSURE( nLoopMax, "das Layout der Tabelle wurde nicht valide!" );
/*N*/ }

/*N*/ BOOL IsFrmInTblSel( const SwRect& rUnion, const SwFrm* pCell )
/*N*/ {
/*N*/ #ifdef VERTICAL_LAYOUT
/*N*/     if( pCell->IsVertical() )
/*?*/         return ( rUnion.Right() >= pCell->Frm().Right() &&
/*?*/             rUnion.Left() <= pCell->Frm().Left() &&
/*?*/             (( rUnion.Top() <= pCell->Frm().Top()+20 &&
/*?*/                rUnion.Bottom() > pCell->Frm().Top() ) ||
/*?*/              ( rUnion.Top() >= pCell->Frm().Top() &&
/*?*/                rUnion.Bottom() < pCell->Frm().Bottom() )) ? TRUE : FALSE );
/*N*/ #endif
/*N*/   return (
/*N*/       rUnion.Top() <= pCell->Frm().Top() &&
/*N*/       rUnion.Bottom() >= pCell->Frm().Bottom() &&
/*N*/
/*N*/       (( rUnion.Left() <= pCell->Frm().Left()+20 &&
/*N*/          rUnion.Right() > pCell->Frm().Left() ) ||
/*N*/
/*N*/        ( rUnion.Left() >= pCell->Frm().Left() &&
/*N*/          rUnion.Right() < pCell->Frm().Right() )) ? TRUE : FALSE );
/*N*/ }


BOOL lcl_CheckCol( const _FndBox*& rpFndBox, void* pPara );

BOOL lcl_CheckRow( const _FndLine*& rpFndLine, void* pPara )
{
    ((_FndLine*)rpFndLine)->GetBoxes().ForEach( &::binfilter::lcl_CheckCol, pPara );
    return *(BOOL*)pPara;
}

BOOL lcl_CheckCol( const _FndBox*& rpFndBox, void* pPara )
{
    if( !rpFndBox->GetBox()->GetSttNd() )
    {
        if( rpFndBox->GetLines().Count() !=
            rpFndBox->GetBox()->GetTabLines().Count() )
            *((BOOL*)pPara) = FALSE;
        else
            ((_FndBox*)rpFndBox)->GetLines().ForEach( &lcl_CheckRow, pPara );
    }
    // Box geschuetzt ??
    else if( rpFndBox->GetBox()->GetFrmFmt()->GetProtect().IsCntntProtected() )
        *((BOOL*)pPara) = FALSE;
    return *(BOOL*)pPara;
}

//Ermittelt die von einer Tabellenselektion betroffenen Tabellen und die
//Union-Rechteckte der Selektionen - auch fuer aufgespaltene Tabellen.
/*N*/ SV_IMPL_PTRARR( SwSelUnions, SwSelUnion* );

/*M*/ SwTwips lcl_CalcWish( const SwLayoutFrm *pCell, long nWish,
/*M*/                                               const long nAct )
/*M*/ {
/*M*/   const SwLayoutFrm *pTmp = pCell;
/*M*/   if ( !nWish )
/*M*/       nWish = 1;
/*M*/
/*M*/ #ifdef BIDI
/*M*/   const sal_Bool bRTL = pCell->IsRightToLeft();
/*M*/   SwTwips nRet = bRTL ?
/*M*/                          nAct - pCell->Frm().Width() :
/*M*/                  0;
/*M*/ #else
/*M*/     SwTwips nRet = 0;
/*M*/ #endif
/*M*/
/*M*/
/*M*/   while ( pTmp )
/*M*/   {
/*M*/       while ( pTmp->GetPrev() )
/*M*/       {
/*M*/           pTmp = (SwLayoutFrm*)pTmp->GetPrev();
/*M*/           long nTmp = pTmp->GetFmt()->GetFrmSize().GetWidth();
/*M*/ #ifdef BIDI
/*M*/             nRet += ( bRTL ? ( -1 ) : 1 ) * nTmp * nAct / nWish;
/*M*/ #else
/*M*/             nRet += nTmp * nAct / nWish;
/*M*/ #endif
/*M*/       }
/*M*/       pTmp = pTmp->GetUpper()->GetUpper();
/*M*/       if ( pTmp && !pTmp->IsCellFrm() )
/*M*/           pTmp = 0;
/*M*/   }
/*M*/   return nRet;
/*M*/ }

/*N*/ void lcl_FindStartEndRow( const SwLayoutFrm *&rpStart,
/*N*/                            const SwLayoutFrm *&rpEnd,
/*N*/                            const int bChkProtected )
/*N*/ {
/*N*/   //Start an den Anfang seiner Zeile setzen.
/*N*/   //End an das Ende seiner Zeile setzen.
/*N*/   rpStart = (SwLayoutFrm*)rpStart->GetUpper()->Lower();
/*N*/   while ( rpEnd->GetNext() )
/*?*/       rpEnd = (SwLayoutFrm*)rpEnd->GetNext();
/*N*/
/*N*/   SvPtrarr aSttArr( 8, 8 ), aEndArr( 8, 8 );
/*N*/   const SwLayoutFrm *pTmp;
/*N*/   for( pTmp = rpStart; (FRM_CELL|FRM_ROW) & pTmp->GetType();
/*N*/               pTmp = pTmp->GetUpper() )
/*N*/   {
/*N*/       void* p = (void*)pTmp;
/*N*/       aSttArr.Insert( p, 0 );
/*N*/   }
/*N*/   for( pTmp = rpEnd; (FRM_CELL|FRM_ROW) & pTmp->GetType();
/*N*/               pTmp = pTmp->GetUpper() )
/*N*/   {
/*N*/       void* p = (void*)pTmp;
/*N*/       aEndArr.Insert( p, 0 );
/*N*/   }
/*N*/
/*N*/   for( USHORT n = 0; n < aEndArr.Count() && n < aSttArr.Count(); ++n )
/*N*/       if( aSttArr[ n ] != aEndArr[ n ] )
/*N*/       {
/*?*/           // first unequal line or box - all odds are
/*?*/           if( n & 1 )                 // 1, 3, 5, ... are boxes
/*?*/           {
/*?*/               rpStart = (SwLayoutFrm*)aSttArr[ n ];
/*?*/               rpEnd = (SwLayoutFrm*)aEndArr[ n ];
/*?*/           }
/*?*/           else                                // 0, 2, 4, ... are lines
/*?*/           {
/*?*/               // check if start & end line are the first & last Line of the
/*?*/               // box. If not return these cells.
/*?*/               // Else the hole line with all Boxes has to be deleted.
/*?*/               rpStart = (SwLayoutFrm*)aSttArr[ n+1 ];
/*?*/               rpEnd = (SwLayoutFrm*)aEndArr[ n+1 ];
/*?*/               if( n )
/*?*/               {
/*?*/                   const SwCellFrm* pCellFrm = (SwCellFrm*)aSttArr[ n-1 ];
/*?*/                   const SwTableLines& rLns = pCellFrm->
/*?*/                                               GetTabBox()->GetTabLines();
/*?*/                   if( rLns[ 0 ] == ((SwRowFrm*)aSttArr[ n ])->GetTabLine() &&
/*?*/                       rLns[ rLns.Count() - 1 ] ==
/*?*/                                   ((SwRowFrm*)aEndArr[ n ])->GetTabLine() )
/*?*/                   {
/*?*/                       rpStart = rpEnd = pCellFrm;
/*?*/                       while ( rpStart->GetPrev() )
/*?*/                           rpStart = (SwLayoutFrm*)rpStart->GetPrev();
/*?*/                       while ( rpEnd->GetNext() )
/*?*/                           rpEnd = (SwLayoutFrm*)rpEnd->GetNext();
/*?*/                   }
/*?*/               }
/*?*/           }
/*?*/           break;
/*N*/       }
/*N*/
/*N*/   if( !bChkProtected )    // geschuetzte Zellen beachten ?
/*N*/       return;
/*N*/
/*N*/
/*?*/   //Anfang und Ende duerfen nicht auf geschuetzten Zellen liegen.
/*?*/   while ( rpStart->GetFmt()->GetProtect().IsCntntProtected() )
/*?*/       rpStart = (SwLayoutFrm*)rpStart->GetNext();
/*?*/   while ( rpEnd->GetFmt()->GetProtect().IsCntntProtected() )
/*?*/       rpEnd = (SwLayoutFrm*)rpEnd->GetPrev();
/*N*/ }


/*M*/ void lcl_FindStartEndCol( const SwLayoutFrm *&rpStart,
/*M*/                            const SwLayoutFrm *&rpEnd,
/*M*/                            const int bChkProtected )
/*M*/ {
/*M*/   //Start und End senkrecht bis an den Rand der Tabelle denken; es muss
/*M*/   //die Gesamttabelle betrachtet werden, also inklusive Masters und
/*M*/   //Follows.
/*M*/   //Fuer den Start brauchen wir den Mutter-TabellenFrm.
/*M*/     if( !rpStart )
/*M*/         return;
/*M*/   const SwTabFrm *pOrg = rpStart->FindTabFrm();
/*M*/   const SwTabFrm *pTab = pOrg;
/*M*/
/*M*/   SWRECTFN( pTab )
/*M*/
/*M*/ #ifdef BIDI
/*M*/     sal_Bool bRTL = pTab->IsRightToLeft();
/*M*/ #endif
/*M*/
/*M*/   const long nWish = pOrg->GetFmt()->GetFrmSize().GetWidth();
/*M*/   while ( pTab->IsFollow() )
/*M*/   {
/*M*/       const SwFrm *pTmp = pTab->FindPrev();
/*M*/       OSL_ENSURE( pTmp->IsTabFrm(), "Vorgaenger vom Follow nicht der Master." );
/*M*/       pTab = (const SwTabFrm*)pTmp;
/*M*/   }
/*M*/
/*M*/     SwTwips nPrtWidth = (pTab->Prt().*fnRect->fnGetWidth)();
/*M*/     const SwTwips nSX = ::binfilter::lcl_CalcWish( rpStart, nWish, nPrtWidth ) +
/*M*/                       (pTab->*fnRect->fnGetPrtLeft)();
/*M*/
/*M*/   const SwTwips nSX2= nSX + (rpStart->GetFmt()->GetFrmSize().GetWidth() *
/*M*/                                           nPrtWidth / nWish);
/*M*/
/*M*/   const SwLayoutFrm *pTmp = pTab->FirstCell();
/*M*/
/*M*/ #ifdef BIDI
/*M*/   while ( pTmp &&
/*M*/           (!pTmp->IsCellFrm() ||
/*M*/              ( ( ! bRTL && (pTmp->Frm().*fnRect->fnGetLeft)() < nSX &&
/*M*/                            (pTmp->Frm().*fnRect->fnGetRight)()< nSX2 ) ||
/*M*/                (   bRTL && (pTmp->Frm().*fnRect->fnGetLeft)() > nSX &&
/*M*/                            (pTmp->Frm().*fnRect->fnGetRight)()> nSX2 ) ) ) )
/*M*/ #else
/*M*/     while ( pTmp &&
/*M*/             (!pTmp->IsCellFrm() ||
/*M*/              ( (pTmp->Frm().*fnRect->fnGetLeft)() < nSX &&
/*M*/                (pTmp->Frm().*fnRect->fnGetRight)()< nSX2 ) ) )
/*M*/ #endif
/*M*/         pTmp = pTmp->GetNextLayoutLeaf();
/*M*/
/*M*/   if ( pTmp )
/*M*/       rpStart = pTmp;
/*M*/
/*M*/   pTab = pOrg;
/*M*/   while ( pTab->GetFollow() )
/*M*/       pTab = pTab->GetFollow();
/*M*/
/*M*/   nPrtWidth = (pTab->Prt().*fnRect->fnGetWidth)();
/*M*/   const SwTwips nEX = ::binfilter::lcl_CalcWish( rpEnd, nWish, nPrtWidth ) +
/*M*/                         (pTab->*fnRect->fnGetPrtLeft)();
/*M*/   rpEnd = pTab->FindLastCntnt()->GetUpper();
/*M*/   while( !rpEnd->IsCellFrm() )
/*M*/       rpEnd = rpEnd->GetUpper();
/*M*/
/*M*/ #ifdef BIDI
/*M*/     while ( (   bRTL && (rpEnd->Frm().*fnRect->fnGetLeft)() < nEX ) ||
/*M*/             ( ! bRTL && (rpEnd->Frm().*fnRect->fnGetLeft)() > nEX ) )
/*M*/ #else
/*M*/     while ( (rpEnd->Frm().*fnRect->fnGetLeft)() > nEX ) )
/*M*/ #endif
/*M*/     {
/*M*/         const SwLayoutFrm* pTmp2 = rpEnd->GetPrevLayoutLeaf();
/*M*/         if( !pTmp2 || !pTab->IsAnLower( pTmp2 ) )
/*M*/             break;
/*M*/         rpEnd = pTmp2;
/*M*/     }
/*M*/
/*M*/   if( !bChkProtected )    // geschuetzte Zellen beachten ?
/*M*/       return;
/*M*/
/*M*/   //Anfang und Ende duerfen nicht auf geschuetzten Zellen liegen.
/*M*/   //Also muss ggf. nocheinmal rueckwaerts gesucht werden.
/*M*/   while ( rpStart->GetFmt()->GetProtect().IsCntntProtected() )
/*M*/   {
/*M*/       const SwLayoutFrm *pTmp3 = rpStart;
/*M*/       pTmp3 = pTmp3->GetNextLayoutLeaf();
/*M*/       while ( pTmp3 && (pTmp3->Frm().*fnRect->fnGetLeft)() > nEX )//erstmal die Zeile ueberspr.
/*M*/           pTmp3 = pTmp3->GetNextLayoutLeaf();
/*M*/       while ( pTmp3 && (pTmp3->Frm().*fnRect->fnGetLeft)() < nSX &&
/*M*/                       (pTmp3->Frm().*fnRect->fnGetRight)()< nSX2 )
/*M*/           pTmp3 = pTmp3->GetNextLayoutLeaf();
/*M*/       const SwTabFrm *pTab2 = rpStart->FindTabFrm();
/*M*/       if ( !pTab2->IsAnLower( pTmp3 ) )
/*M*/       {
/*M*/           pTab2 = pTab2->GetFollow();
/*M*/           rpStart = pTab2->FirstCell();
/*M*/           while ( (rpStart->Frm().*fnRect->fnGetLeft)() < nSX &&
/*M*/                   (rpStart->Frm().*fnRect->fnGetRight)()< nSX2 )
/*M*/               rpStart = rpStart->GetNextLayoutLeaf();
/*M*/       }
/*M*/       else
/*M*/           rpStart = pTmp3;
/*M*/   }
/*M*/   while ( rpEnd->GetFmt()->GetProtect().IsCntntProtected() )
/*M*/   {
/*M*/       const SwLayoutFrm *pTmp4 = rpEnd;
/*M*/       pTmp4 = pTmp4->GetPrevLayoutLeaf();
/*M*/       while ( pTmp4 && (pTmp4->Frm().*fnRect->fnGetLeft)() < nEX )//erstmal die Zeile ueberspr.
/*M*/           pTmp4 = pTmp4->GetPrevLayoutLeaf();
/*M*/       while ( pTmp4 && (pTmp4->Frm().*fnRect->fnGetLeft)() > nEX )
/*M*/           pTmp4 = pTmp4->GetPrevLayoutLeaf();
/*M*/       const SwTabFrm *pTab3 = rpEnd->FindTabFrm();
/*M*/       if ( !pTmp4 || !pTab3->IsAnLower( pTmp4 ) )
/*M*/       {
/*M*/           pTab3 = (const SwTabFrm*)pTab3->FindPrev();
/*M*/           OSL_ENSURE( pTab3->IsTabFrm(), "Vorgaenger vom Follow nicht der Master.");
/*M*/           rpEnd = pTab3->FindLastCntnt()->GetUpper();
/*M*/           while( !rpEnd->IsCellFrm() )
/*M*/               rpEnd = rpEnd->GetUpper();
/*M*/           while ( (rpEnd->Frm().*fnRect->fnGetLeft)() > nEX )
/*M*/               rpEnd = rpEnd->GetPrevLayoutLeaf();
/*M*/       }
/*M*/       else
/*M*/           rpEnd = pTmp4;
/*M*/   }
/*M*/ }


/*M*/ void MakeSelUnions( SwSelUnions& rUnions, const SwLayoutFrm *pStart,
/*M*/                   const SwLayoutFrm *pEnd, const SwTblSearchType eSearchType )
/*M*/ {
/*M*/   while ( !pStart->IsCellFrm() )
/*M*/       pStart = pStart->GetUpper();
/*M*/   while ( !pEnd->IsCellFrm() )
/*M*/       pEnd = pEnd->GetUpper();
/*M*/
/*M*/   const SwTabFrm *pTable = pStart->FindTabFrm();
/*M*/   const SwTabFrm *pEndTable = pEnd->FindTabFrm();
/*M*/     if( !pTable || !pEndTable )
/*M*/         return;
/*M*/   BOOL bExchange = FALSE;
/*M*/
/*M*/   if ( pTable != pEndTable )
/*M*/   {
/*M*/       if ( !pTable->IsAnFollow( pEndTable ) )
/*M*/       {
/*M*/           OSL_ENSURE( pEndTable->IsAnFollow( pTable ), "Tabkette verknotet." );
/*M*/           bExchange = TRUE;
/*M*/       }
/*M*/   }
/*M*/ #ifdef VERTICAL_LAYOUT
/*M*/     else
/*M*/     {
/*M*/         SWRECTFN( pStart )
/*M*/         long nSttTop = (pStart->Frm().*fnRect->fnGetTop)();
/*M*/         long nEndTop = (pEnd->Frm().*fnRect->fnGetTop)();
/*M*/         if( nSttTop == nEndTop )
/*M*/         {
/*M*/             if( (pStart->Frm().*fnRect->fnGetLeft)() >
/*M*/                 (pEnd->Frm().*fnRect->fnGetLeft)() )
/*M*/                 bExchange = TRUE;
/*M*/         }
/*M*/         else if( bVert == ( nSttTop < nEndTop ) )
/*M*/             bExchange = TRUE;
/*M*/     }
/*M*/ #else
/*M*/   else if ( pStart->Frm().Top() > pEnd->Frm().Top() ||
/*M*/            (pStart->Frm().Top() == pEnd->Frm().Top() &&
/*M*/             pStart->Frm().Left() > pEnd->Frm().Left()) )
/*M*/       bExchange = TRUE;
/*M*/ #endif
/*M*/   if ( bExchange )
/*M*/   {
/*M*/       const SwLayoutFrm *pTmp = pStart;
/*M*/       pStart = pEnd;
/*M*/       pEnd = pTmp;
/*M*/       //pTable und pEndTable nicht umsortieren, werden unten neu gesetzt.
/*M*/       //MA: 28. Dec. 93 Bug: 5190
/*M*/   }
/*M*/
/*M*/   //Start und End sind jetzt huebsch sortiert, jetzt muessen sie falls
/*M*/   //erwuenscht noch versetzt werden.
/*M*/   if( TBLSEARCH_ROW == ((~TBLSEARCH_PROTECT ) & eSearchType ) )
/*M*/       ::binfilter::lcl_FindStartEndRow( pStart, pEnd, TBLSEARCH_PROTECT & eSearchType );
/*M*/   else if( TBLSEARCH_COL == ((~TBLSEARCH_PROTECT ) & eSearchType ) )
/*M*/       ::binfilter::lcl_FindStartEndCol( pStart, pEnd, TBLSEARCH_PROTECT & eSearchType );
/*M*/
/*M*/   //neu besorgen, da sie jetzt verschoben sind. MA: 28. Dec. 93 Bug 5190
/*M*/   pTable = pStart->FindTabFrm();
/*M*/   pEndTable = pEnd->FindTabFrm();
/*M*/
/*M*/   const bool bRepeat = pTable->GetTable()->IsHeadlineRepeat();
/*M*/
/*M*/   const long nStSz = pStart->GetFmt()->GetFrmSize().GetWidth();
/*M*/   const long nEdSz = pEnd->GetFmt()->GetFrmSize().GetWidth();
/*M*/   const long nWish = Max( 1L, pTable->GetFmt()->GetFrmSize().GetWidth() );
/*M*/   while ( pTable )
/*M*/   {
/*M*/ #ifdef VERTICAL_LAYOUT
/*M*/         SWRECTFN( pTable )
/*M*/         const long nOfst = (pTable->*fnRect->fnGetPrtLeft)();
/*M*/         const long nPrtWidth = (pTable->Prt().*fnRect->fnGetWidth)();
/*M*/         long nSt1 = ::binfilter::lcl_CalcWish( pStart, nWish, nPrtWidth ) + nOfst;
/*M*/         long nEd1 = ::binfilter::lcl_CalcWish( pEnd,   nWish, nPrtWidth ) + nOfst;
/*M*/
/*M*/         if ( nSt1 <= nEd1 )
/*M*/             nEd1 += (long)((nEdSz * nPrtWidth) / nWish) - 1;
/*M*/       else
/*M*/             nSt1 += (long)((nStSz * nPrtWidth) / nWish) - 1;
/*M*/
/*M*/         long nSt2;
/*M*/         long nEd2;
/*M*/         if( pTable->IsAnLower( pStart ) )
/*M*/             nSt2 = (pStart->Frm().*fnRect->fnGetTop)();
/*M*/         else
/*M*/             nSt2 = (pTable->Frm().*fnRect->fnGetTop)();
/*M*/         if( pTable->IsAnLower( pEnd ) )
/*M*/             nEd2 = (pEnd->Frm().*fnRect->fnGetBottom)();
/*M*/         else
/*M*/             nEd2 = (pTable->Frm().*fnRect->fnGetBottom)();
/*M*/         Point aSt, aEd;
/*M*/         if( nSt1 > nEd1 )
/*M*/         {
/*M*/             long nTmp = nSt1;
/*M*/             nSt1 = nEd1;
/*M*/             nEd1 = nTmp;
/*M*/         }
/*M*/         if( nSt2 > nEd2 )
/*M*/         {
/*M*/             long nTmp = nSt2;
/*M*/             nSt2 = nEd2;
/*M*/             nEd2 = nTmp;
/*M*/         }
/*M*/         if( bVert )
/*M*/         {
/*M*/             aSt = Point( nSt2, nSt1 );
/*M*/             aEd = Point( nEd2, nEd1 );
/*M*/         }
/*M*/         else
/*M*/         {
/*M*/             aSt = Point( nSt1, nSt2 );
/*M*/             aEd = Point( nEd1, nEd2 );
/*M*/         }
/*M*/ #else
/*M*/       const long nOfst = pTable->Frm().Left() + pTable->Prt().Left();
/*M*/       long nSt = ::binfilter::lcl_CalcWish( pStart, nWish, pTable->Prt().Width() ) + nOfst;
/*M*/       long nEd = ::binfilter::lcl_CalcWish( pEnd,   nWish, pTable->Prt().Width() ) + nOfst;
/*M*/
/*M*/       if ( nSt <= nEd )
/*M*/           nEd += (long)((nEdSz * pTable->Prt().Width()) / nWish) - 1;
/*M*/       else
/*M*/           nSt += (long)((nStSz * pTable->Prt().Width()) / nWish) - 1;
/*M*/
/*M*/       Point aSt( nSt, pStart->Frm().Top() ),
/*M*/             aEd( nEd, pEnd->Frm().Bottom() );
/*M*/
/*M*/       if ( !pTable->IsAnLower( pStart ) )
/*M*/           aSt.Y() = pTable->Frm().Top();
/*M*/       if ( !pTable->IsAnLower( pEnd ) )
/*M*/           aEd.Y() = pTable->Frm().Bottom();
/*M*/ #endif
/*M*/
/*M*/       SwRect aUnion( aSt, aEd );
/*M*/       aUnion.Justify();
/*M*/
/*M*/       // fuers
/*M*/       if( !(TBLSEARCH_NO_UNION_CORRECT & eSearchType ))
/*M*/       {
/*M*/           //Leider ist die Union jetzt mit Rundungsfehlern behaftet und dadurch
/*M*/           //wuerden beim Split/Merge fehlertraechtige Umstaende entstehen.
/*M*/           //Um dies zu vermeiden werden jetzt fuer die Table die erste und
/*M*/           //letzte Zelle innerhalb der Union ermittelt und aus genau deren
/*M*/           //Werten wird die Union neu gebildet.
/*M*/           const SwLayoutFrm *pRow = (SwLayoutFrm*)pTable->Lower();
/*M*/           if ( bRepeat && pRow && pTable->IsFollow() )
/*M*/               pRow = (SwLayoutFrm*)pRow->GetNext();
/*M*/           while ( pRow && !pRow->Frm().IsOver( aUnion ) )
/*M*/               pRow = (SwLayoutFrm*)pRow->GetNext();
/*M*/           const SwLayoutFrm *pFirst = pRow ? pRow->FirstCell() : 0;
/*M*/           while ( pFirst && !::binfilter::IsFrmInTblSel( aUnion, pFirst ) )
/*M*/           {
/*M*/               if ( pFirst->GetNext() )
/*M*/               {
/*M*/                   pFirst = (const SwLayoutFrm*)pFirst->GetNext();
/*M*/                   if ( pFirst->Lower()->IsRowFrm() )
/*M*/                       pFirst = pFirst->FirstCell();
/*M*/               }
/*M*/               else
/*M*/                   pFirst = ::binfilter::lcl_FindNextCellFrm( pFirst );
/*M*/           }
/*M*/           const SwLayoutFrm *pLast = ::binfilter::lcl_FindCellFrm( pTable->FindLastCntnt()->GetUpper());
/*M*/           while ( pLast && !::binfilter::IsFrmInTblSel( aUnion, pLast ) )
/*M*/               pLast = ::binfilter::lcl_FindCellFrm( pLast->GetPrevLayoutLeaf() );
/*M*/
/*M*/           if ( pFirst && pLast ) //Robust
/*M*/ #ifdef VERTICAL_LAYOUT
/*M*/             {
/*M*/                 aUnion = pFirst->Frm();
/*M*/                 aUnion.Union( pLast->Frm() );
/*M*/             }
/*M*/ #else
/*M*/               aUnion = SwRect( pFirst->Frm().Pos(),
/*M*/                                Point( pLast->Frm().Right(),
/*M*/                                        pLast->Frm().Bottom() ) );
/*M*/ #endif
/*M*/           else
/*M*/               aUnion.Width( 0 );
/*M*/       }
/*M*/
/*M*/       if( (aUnion.*fnRect->fnGetWidth)() )
/*M*/       {
/*M*/           SwSelUnion *pTmp = new SwSelUnion( aUnion, (SwTabFrm*)pTable );
/*M*/           rUnions.C40_INSERT( SwSelUnion, pTmp, rUnions.Count() );
/*M*/       }
/*M*/
/*M*/       pTable = pTable->GetFollow();
/*M*/       if ( pTable != pEndTable && pEndTable->IsAnFollow( pTable ) )
/*M*/           pTable = 0;
/*M*/   }
/*M*/ }

 BOOL _FndBoxCopyCol( const SwTableBox*& rpBox, void* pPara )
 {
    _FndPara* pFndPara = (_FndPara*)pPara;
    _FndBox* pFndBox = new _FndBox( (SwTableBox*)rpBox, pFndPara->pFndLine );
    if( rpBox->GetTabLines().Count() )
    {
        _FndPara aPara( *pFndPara, pFndBox );
        pFndBox->GetBox()->GetTabLines().ForEach( &_FndLineCopyCol, &aPara );
        if( !pFndBox->GetLines().Count() )
        {
            delete pFndBox;
            return TRUE;
        }
    }
    else
    {
        SwTableBoxPtr pSrch = (SwTableBoxPtr)rpBox;
        USHORT nFndPos;
        if( !pFndPara->rBoxes.Seek_Entry( pSrch, &nFndPos ))
        {
            delete pFndBox;
            return TRUE;
        }
    }
    pFndPara->pFndLine->GetBoxes().C40_INSERT( _FndBox, pFndBox,
                    pFndPara->pFndLine->GetBoxes().Count() );
    return TRUE;
 }

/*N*/ BOOL _FndLineCopyCol( const SwTableLine*& rpLine, void* pPara )
/*N*/ {
     _FndPara* pFndPara = (_FndPara*)pPara;
     _FndLine* pFndLine = new _FndLine( (SwTableLine*)rpLine, pFndPara->pFndBox );
     _FndPara aPara( *pFndPara, pFndLine );
     pFndLine->GetLine()->GetTabBoxes().ForEach( &_FndBoxCopyCol, &aPara );
     if( pFndLine->GetBoxes().Count() )
     {
         pFndPara->pFndBox->GetLines().C40_INSERT( _FndLine, pFndLine,
                 pFndPara->pFndBox->GetLines().Count() );
     }
     else
         delete pFndLine;
/*N*/   return TRUE;
/*N*/ }

inline void UnsetFollow( SwFlowFrm *pTab )
{
    pTab->bIsFollow = FALSE;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
