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


#include <fmtanchr.hxx>
#include <fmtcntnt.hxx>

#include <horiornt.hxx>

#include <fmtfsize.hxx>
#include <fesh.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <txtfrm.hxx>
#include <viewimp.hxx>
#include <viscrs.hxx>
#include <doc.hxx>
#include <dview.hxx>
#include <dflyobj.hxx>
#include <frmfmt.hxx>
#include <ndtxt.hxx>
#include <ndgrf.hxx>
#include <flyfrms.hxx>
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;

//Zum anmelden von Flys in Flys in ...
//definiert in layout/frmtool.cxx

/***********************************************************************
#*  Class       :  SwDoc
#*  Methode     :  UseSpzLayoutFmt
#*  Beschreibung:  Anhand des Request werden zu dem Format entsprechende
#*      Aenderungen an den Spezifischen Layouts vorgenommen.
#*  Datum       :  MA 23. Sep. 92
#*  Update      :  JP 09.03.98
#***********************************************************************/


/*N*/ BOOL lcl_FindAnchorPos( SwDoc& rDoc, const Point& rPt, const SwFrm& rFrm,
/*N*/                       SfxItemSet& rSet )
/*N*/ {
/*N*/   BOOL bRet = TRUE;
/*N*/   SwFmtAnchor aNewAnch( (SwFmtAnchor&)rSet.Get( RES_ANCHOR ) );
/*N*/   RndStdIds nNew = aNewAnch.GetAnchorId();
/*N*/   const SwFrm *pNewAnch;
/*N*/
/*N*/   //Neuen Anker ermitteln
/*N*/   Point aTmpPnt( rPt );
/*N*/   switch( nNew )
/*N*/   {
/*N*/   case FLY_IN_CNTNT:  // sollte der nicht auch mit hinein?
/*N*/   case FLY_AT_CNTNT:
/*N*/   case FLY_AUTO_CNTNT: // LAYER_IMPL
/*N*/       {
/*N*/           //Ausgehend von der linken oberen Ecke des Fly den
/*N*/           //dichtesten CntntFrm suchen.
/*N*/           const SwFrm* pFrm = rFrm.IsFlyFrm() ? ((SwFlyFrm&)rFrm).GetAnchor()
/*N*/                                               : &rFrm;
/*N*/           pNewAnch = ::binfilter::FindAnchor( pFrm, aTmpPnt );
/*N*/           if( pNewAnch->IsProtected() )
/*N*/           {
/*N*/               bRet = FALSE;
/*N*/               break;
/*N*/           }
/*N*/
/*N*/           SwPosition aPos( *((SwCntntFrm*)pNewAnch)->GetNode() );
/*N*/           if( FLY_AUTO_CNTNT == nNew || FLY_IN_CNTNT == nNew )
/*N*/           {
/*N*/               // es muss ein TextNode gefunden werden, denn nur in diesen
/*N*/               // ist ein Inhaltsgebundene Frames zu verankern
/*N*/               SwCrsrMoveState aState( MV_SETONLYTEXT );
/*N*/               aTmpPnt.X() -= 1;                   //nicht im Fly landen!!
/*N*/               if( !pNewAnch->GetCrsrOfst( &aPos, aTmpPnt, &aState ) )
/*N*/               {
/*N*/                   SwCntntNode* pCNd = ((SwCntntFrm*)pNewAnch)->GetNode();
/*N*/                   if( pNewAnch->Frm().Bottom() < aTmpPnt.Y() )
/*N*/                       pCNd->MakeStartIndex( &aPos.nContent );
/*N*/                   else
/*N*/                       pCNd->MakeEndIndex( &aPos.nContent );
/*N*/               }
/*N*/           }
/*N*/           aNewAnch.SetAnchor( &aPos );
/*N*/       }
/*N*/       break;
/*N*/
/*N*/   case FLY_AT_FLY: // LAYER_IMPL
/*N*/       {
/*N*/           //Ausgehend von der linken oberen Ecke des Fly den
/*N*/           //dichtesten SwFlyFrm suchen.
/*N*/           SwCrsrMoveState aState( MV_SETONLYTEXT );
/*N*/           SwPosition aPos( rDoc.GetNodes() );
/*N*/           aTmpPnt.X() -= 1;                   //nicht im Fly landen!!
/*N*/           rDoc.GetRootFrm()->GetCrsrOfst( &aPos, aTmpPnt, &aState );
/*N*/           pNewAnch = ::binfilter::FindAnchor(
/*N*/               aPos.nNode.GetNode().GetCntntNode()->GetFrm( 0, 0, sal_False ),
/*N*/               aTmpPnt )->FindFlyFrm();
/*N*/
/*N*/           if( pNewAnch && &rFrm != pNewAnch && !pNewAnch->IsProtected() )
/*N*/           {
/*N*/               aPos.nNode = *((SwFlyFrm*)pNewAnch)->GetFmt()->GetCntnt().
/*N*/                               GetCntntIdx();
/*N*/               aNewAnch.SetAnchor( &aPos );
/*N*/               break;
/*N*/           }
/*N*/       }
/*N*/
/*N*/       aNewAnch.SetType( nNew = FLY_PAGE );
/*N*/       // no break
/*N*/
/*N*/   case FLY_PAGE:
/*N*/       pNewAnch = rFrm.FindPageFrm();
/*N*/       aNewAnch.SetPageNum( pNewAnch->GetPhyPageNum() );
/*N*/       break;
/*N*/
/*N*/   default:
/*N*/       OSL_ENSURE( !&rDoc, "Falsche ID fuer neuen Anker." );
/*N*/   }
/*N*/
/*N*/   rSet.Put( aNewAnch );
/*N*/   return bRet;
/*N*/ }

//
//! also used in unoframe.cxx
//
/*N*/ sal_Bool lcl_ChkAndSetNewAnchor( const SwFlyFrm& rFly, SfxItemSet& rSet )
/*N*/ {
/*N*/   const SwFrmFmt& rFmt = *rFly.GetFmt();
/*N*/   const SwFmtAnchor &rOldAnch = rFmt.GetAnchor();
/*N*/   const RndStdIds nOld = rOldAnch.GetAnchorId();
/*N*/
/*N*/   RndStdIds nNew = ((SwFmtAnchor&)rSet.Get( RES_ANCHOR )).GetAnchorId();
/*N*/
/*N*/   if( nOld == nNew )
/*N*/       return sal_False;
/*N*/
/*N*/   SwDoc* pDoc = (SwDoc*)rFmt.GetDoc();
/*N*/
/*N*/ #ifdef DBG_UTIL
/*N*/   OSL_ENSURE( !(nNew == FLY_PAGE &&
/*N*/       (FLY_AT_CNTNT==nOld || FLY_AUTO_CNTNT==nOld || FLY_IN_CNTNT==nOld ) &&
/*N*/       pDoc->IsInHeaderFooter( rOldAnch.GetCntntAnchor()->nNode )),
/*N*/           "Unerlaubter Ankerwechsel in Head/Foot." );
/*N*/ #endif
/*N*/
/*N*/   return ::binfilter::lcl_FindAnchorPos( *pDoc, rFly.Frm().Pos(), rFly, rSet );
/*N*/ }

} //namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
