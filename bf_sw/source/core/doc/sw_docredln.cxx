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

#include <tools/shl.hxx>
#include <swmodule.hxx>

#include <horiornt.hxx>
#include <osl/diagnose.h>

#include <doc.hxx>
#include <docary.hxx>
#include <redline.hxx>
#include <pamtyp.hxx>
#include <viewsh.hxx>
#include <rootfrm.hxx>
namespace binfilter {

/*N*/ SV_IMPL_OP_PTRARR_SORT( _SwRedlineTbl, SwRedlinePtr )

/*N*/ void SwDoc::SetRedlineMode( USHORT eMode )
/*N*/ {
/*N*/   if( eRedlineMode != eMode )
/*N*/   {
/*N*/       if( (REDLINE_SHOW_MASK & eRedlineMode) != (REDLINE_SHOW_MASK & eMode)
/*N*/           || 0 == (REDLINE_SHOW_MASK & eMode) )
/*N*/       {
/*N*/           switch( REDLINE_SHOW_MASK & eMode )
/*N*/           {
/*N*/           case REDLINE_SHOW_INSERT:
/*N*/           case REDLINE_SHOW_DELETE:
/*?*/               break;
/*N*/
/*N*/           default:
/*N*/               eMode |= REDLINE_SHOW_INSERT;
/*N*/               break;
/*N*/           }
/*N*/
/*N*/       }
/*N*/       eRedlineMode = (SwRedlineMode)eMode;
/*N*/   }
/*N*/ }


/*

Text heisst, nicht von Redline "verseuchter" Text.

Verhalten von Insert-Redline:
    - im Text                           - Redline Object einfuegen
    - im InsertRedline (eigenes)        - ignorieren, bestehendes wird
                                          aufgespannt
    - im InsertRedline (andere)         - Insert Redline aufsplitten
                                          Redline Object einfuegen
    - in DeleteRedline                  - Delete Redline aufsplitten oder
                                          am Ende/Anfang verschieben

Verhalten von Delete-Redline:
    - im Text                           - Redline Object einfuegen
    - im DeleteRedline (eigenes/andere) - ignorieren
    - im InsertRedline (eigenes)        - ignorieren, Zeichen aber loeschen
    - im InsertRedline (andere)         - Insert Redline aufsplitten
                                          Redline Object einfuegen
    - Ueberlappung von Text und         - Text in eigenen Insert loeschen,
      eigenem Insert                      im andereren Text aufspannen (bis
                                          zum Insert!
    - Ueberlappung von Text und         - Redline Object einfuegen, der
      anderem Insert                      andere Insert wird vom Delete
                                          ueberlappt
*/

/*N*/ BOOL SwDoc::DeleteRedline( const SwPaM& rRange, BOOL /*bSaveInUndo*/,
/*N*/                           USHORT /*nDelType*/ )
/*N*/ {
/*N*/   if( REDLINE_IGNOREDELETE_REDLINES & eRedlineMode ||
/*N*/       !rRange.HasMark() || *rRange.GetMark() == *rRange.GetPoint() )
/*N*/       return FALSE;
/*N*/
/*N*/   const SwPosition* pStt = rRange.Start();
/*N*/   pStt == rRange.GetPoint() ? rRange.GetMark() : rRange.GetPoint();

/*N*/   USHORT n = 0;
/*N*/   GetRedline( *pStt, &n );
/*N*/
/*N*/   return FALSE;
/*N*/ }

/*N*/ BOOL SwDoc::DeleteRedline( const SwStartNode& rNode, BOOL bSaveInUndo,
/*N*/                           USHORT nDelType )
/*N*/ {
/*N*/   SwPaM aTemp( *rNode.EndOfSectionNode(), rNode );
/*N*/   return DeleteRedline( aTemp, bSaveInUndo, nDelType );
/*N*/ }

/*N*/ const SwRedline* SwDoc::GetRedline( const SwPosition& /*rPos*/,
/*N*/                                   USHORT* pFndPos ) const
/*N*/ {
/*N*/   if( pFndPos )
/*N*/       *pFndPos = 0;
/*N*/   return 0;
/*N*/ }

// Kommentar am Redline setzen

    // fuer die Reader usw. - neuen Author in die Tabelle eintragen
/*N*/ USHORT SwDoc::InsertRedlineAuthor( const String& rNew )
/*N*/ {
/*N*/ return SW_MOD()->InsertRedlineAuthor(rNew); //SW50.SDW
/*N*/ }


    // setze Kommentar-Text fuers Redline, das dann per AppendRedline
    // hereinkommt. Wird vom Autoformat benutzt. 0-Pointer setzt den Modus
    // wieder zurueck. Pointer wird nicht kopiert, muss also gueltig bleiben!

/*N*/ void SwDoc::SetRedlinePasswd(
/*N*/           const ::com::sun::star::uno::Sequence <sal_Int8>& rNew )
/*N*/ {
/*N*/   aRedlinePasswd = rNew;
/*N*/   SetModified();
/*N*/ }


/*N*/ BOOL SwRedlineTbl::Insert( SwRedlinePtr& p, BOOL bIns ) //SW50.SDW
/*N*/ {
/*N*/   BOOL bRet = FALSE;
/*N*/   if( p->HasValidRange() )
/*N*/   {
/*N*/       bRet = _SwRedlineTbl::Insert( p );
/*N*/   }
/*N*/   else if( bIns )
/*N*/       bRet = InsertWithValidRanges( p );
/*N*/   else
/*N*/   {
/*N*/       OSL_ENSURE( !this, "Redline: falscher Bereich" );
/*N*/   }
/*N*/   return bRet;
/*N*/ }


/*N*/ BOOL SwRedlineTbl::InsertWithValidRanges( SwRedlinePtr& p, USHORT* pInsPos ) //SW50.SDW
/*N*/ {
/*N*/   // erzeuge aus den Selektion gueltige "Teilbereiche".
/*N*/   BOOL bAnyIns = FALSE;
/*N*/   SwPosition* pStt = p->Start(),
/*N*/             * pEnd = pStt == p->GetPoint() ? p->GetMark() : p->GetPoint();
/*N*/   SwPosition aNewStt( *pStt );
/*N*/   SwNodes& rNds = aNewStt.nNode.GetNodes();
/*N*/   SwCntntNode* pC;
/*N*/
/*N*/   if( !aNewStt.nNode.GetNode().IsCntntNode() )
/*N*/   {
/*N*/       pC = rNds.GoNext( &aNewStt.nNode );
/*N*/       if( pC )
/*N*/           aNewStt.nContent.Assign( pC, 0 );
/*N*/       else
/*N*/           aNewStt.nNode = rNds.GetEndOfContent();
/*N*/   }
/*N*/
/*N*/   SwRedline* pNew = 0;
/*N*/   USHORT nInsPos;
/*N*/
/*N*/   if( aNewStt < *pEnd )
/*N*/       do {
/*N*/           if( !pNew )
/*N*/               pNew = new SwRedline( p->GetRedlineData(), aNewStt );
/*N*/           else
/*N*/           {
/*N*/               pNew->DeleteMark();
/*N*/               *pNew->GetPoint() = aNewStt;
/*N*/           }
/*N*/
/*N*/           pNew->SetMark();
/*N*/           if( *pNew->GetPoint() > *pEnd )
/*N*/           {
/*N*/               pC = 0;
/*N*/               if( aNewStt.nNode != pEnd->nNode )
/*N*/                   do {
/*N*/                       SwNode& rCurNd = aNewStt.nNode.GetNode();
/*N*/                       if( rCurNd.IsStartNode() )
/*N*/                       {
/*N*/                           if( rCurNd.EndOfSectionIndex() < pEnd->nNode.GetIndex() )
/*N*/                               aNewStt.nNode = *rCurNd.EndOfSectionNode();
/*N*/                           else
/*N*/                               break;
/*N*/                       }
/*N*/                       else if( rCurNd.IsCntntNode() )
/*N*/                           pC = rCurNd.GetCntntNode();
/*N*/                       aNewStt.nNode++;
/*N*/                   } while( aNewStt.nNode.GetIndex() < pEnd->nNode.GetIndex() );
/*N*/
/*N*/               if( aNewStt.nNode == pEnd->nNode )
/*N*/                   aNewStt.nContent = pEnd->nContent;
/*N*/               else if( pC )
/*N*/               {
/*N*/                   aNewStt.nNode = *pC;
/*N*/                   aNewStt.nContent.Assign( pC, pC->Len() );
/*N*/               }
/*N*/
/*N*/               if( aNewStt <= *pEnd )
/*N*/                   *pNew->GetPoint() = aNewStt;
/*N*/           }
/*N*/           else
/*N*/               aNewStt = *pNew->GetPoint();
/*N*/
/*N*/           if( *pNew->GetPoint() != *pNew->GetMark() &&
/*N*/               _SwRedlineTbl::Insert( pNew, nInsPos ) )
/*N*/           {
/*N*/               bAnyIns = TRUE;
/*N*/               pNew = 0;
/*N*/               if( pInsPos && *pInsPos < nInsPos )
/*N*/                   *pInsPos = nInsPos;
/*N*/           }
/*N*/
/*N*/           if( aNewStt >= *pEnd ||
/*N*/               0 == (pC = rNds.GoNext( &aNewStt.nNode )) )
/*N*/               break;
/*N*/
/*N*/           aNewStt.nContent.Assign( pC, 0 );
/*N*/
/*N*/       } while( aNewStt < *pEnd );
/*N*/
/*N*/   delete pNew;
/*N*/   delete p, p = 0;
/*N*/   return bAnyIns;
/*N*/ }

/*N*/ void SwRedlineTbl::Remove( USHORT nP, USHORT nL ) //SW50.SDW
/*N*/ {
/*N*/   _SwRedlineTbl::Remove( nP, nL );
/*N*/ }


/*?*/SwRedlineData::SwRedlineData( const SwRedlineData& rCpy, BOOL bCpyNext )
/*?*/   : pNext( (bCpyNext && rCpy.pNext) ? new SwRedlineData( *rCpy.pNext ) : 0 )
/*?*/   , sComment( rCpy.sComment )
/*?*/   , aStamp( rCpy.aStamp )
/*?*/   , eType( rCpy.eType )
/*?*/   , nAuthor( rCpy.nAuthor )
/*?*/   , nSeqNo( rCpy.nSeqNo )
/*?*/{
/*?*/}

    // fuer sw3io: pNext geht in eigenen Besitz ueber!
/*?*/SwRedlineData::SwRedlineData( SwRedlineType eT, USHORT nAut, const DateTime& rDT,
/*?*/                  const String& rCmnt, SwRedlineData *pNxt )
/*?*/   : pNext( pNxt )
/*?*/   , sComment( rCmnt )
/*?*/   , aStamp( rDT )
/*?*/   , eType( eT )
/*?*/   , nAuthor( nAut )
/*?*/   , nSeqNo( 0 )
/*?*/{
/*?*/}

/*?*/SwRedlineData::~SwRedlineData()
/*?*/{
/*?*/   delete pNext;
/*?*/}


/*?*/SwRedline::SwRedline( const SwRedlineData& rData, const SwPosition& rPos )
/*?*/   : SwPaM( rPos )
/*?*/   , pRedlineData( new SwRedlineData( rData ))
/*?*/   , pCntntSect( 0 )
/*?*/{
/*?*/   bDelLastPara = bIsLastParaDelete = FALSE;
/*?*/   bIsVisible = TRUE;
/*?*/}

/*?*/SwRedline::SwRedline( const SwRedline& rCpy )
/*?*/   : SwPaM( *rCpy.GetMark(), *rCpy.GetPoint() )
/*?*/   , pRedlineData( new SwRedlineData( *rCpy.pRedlineData ))
/*?*/   , pCntntSect( 0 )
/*?*/{
/*?*/   bDelLastPara = bIsLastParaDelete = FALSE;
/*?*/   bIsVisible = TRUE;
/*?*/   if( !rCpy.HasMark() )
/*?*/       DeleteMark();
/*?*/}

/*?*/SwRedline::~SwRedline()
/*?*/{
/*?*/   if( pCntntSect )
/*?*/   {
/*?*/       // dann den Content Bereich loeschen
/*?*/       if( !GetDoc()->IsInDtor() )
/*?*/           GetDoc()->DeleteSection( &pCntntSect->GetNode() );
/*?*/       delete pCntntSect;
/*?*/   }
/*?*/   delete pRedlineData;
/*?*/}

// liegt eine gueltige Selektion vor?
/*N*/ BOOL SwRedline::HasValidRange() const //SW50.SDW
/*N*/ {
/*N*/   const SwNode* pPtNd = &GetPoint()->nNode.GetNode(),
/*N*/               * pMkNd = &GetMark()->nNode.GetNode();
/*N*/   if( pPtNd->FindStartNode() == pMkNd->FindStartNode() &&
/*N*/       !pPtNd->FindStartNode()->IsTableNode() &&
/*N*/         // invalid if points on the end of content
/*N*/         // end-of-content only invalid if no content index exists
/*N*/       ( pPtNd != pMkNd || GetContentIdx() != NULL ||
/*N*/           pPtNd != &pPtNd->GetNodes().GetEndOfContent() )
/*N*/       )
/*N*/       return TRUE;
/*N*/   return FALSE;
/*N*/ }


/*?*/const String& SwRedline::GetAuthorString( USHORT nPos ) const
/*?*/{
        SwRedlineData* pCur;
/*?*/   for( pCur = pRedlineData; nPos && pCur->pNext; --nPos )
/*?*/       pCur = pCur->pNext;
/*?*/   OSL_ENSURE( !nPos, "Pos angabe ist zu gross" );
/*?*/   return SW_MOD()->GetRedlineAuthor(pCur->nAuthor);
/*?*/}

/*N*/ int SwRedline::operator==( const SwRedline& rCmp ) const
/*N*/ {
/*N*/   return this == &rCmp;
/*N*/ }

/*N*/ int SwRedline::operator<( const SwRedline& rCmp ) const
/*N*/ {
/*N*/       BOOL nResult = FALSE;
/*N*/
/*N*/     if (*Start() < *rCmp.Start())
/*N*/         nResult = TRUE;
/*N*/     else if (*Start() == *rCmp.Start())
/*N*/         if (*End() < *rCmp.End())
/*N*/             nResult = TRUE;
/*N*/
/*N*/     return nResult;
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
