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
#include <vcl/window.hxx>
#include <fmtsrnd.hxx>
#include <frmfmt.hxx>
#include <osl/diagnose.h>
#include <viscrs.hxx>
#include <fesh.hxx>
#include <doc.hxx>
#include <flyfrm.hxx>
#include <frmtool.hxx>
#include <viewopt.hxx>
#include <hints.hxx>
#include <dflyobj.hxx>
#include <notxtfrm.hxx>
#include <ndgrf.hxx>

#include <mdiexp.hxx>
#include <comcore.hrc>
namespace binfilter {

#define DEFTEXTSIZE  12

//Zum asynchronen (erstmaligem) anfordern von Grafiken


extern void ClrContourCache( const SdrObject *pObj ); // TxtFly.Cxx

/*************************************************************************
|*
|*    SwGrfFrm::SwGrfFrm(ViewShell * const,SwGrfNode *)
|*
*************************************************************************/


/*N*/ SwNoTxtFrm::SwNoTxtFrm(SwNoTxtNode * const pNode)
/*N*/   : SwCntntFrm(pNode)
/*N*/ {
/*N*/   InitCtor();
/*N*/ }

// Initialisierung: z.Zt. Eintragen des Frames im Cache


/*N*/ void SwNoTxtFrm::InitCtor()
/*N*/ {
/*N*/     nType = FRMC_NOTXT;
/*N*/   // Das Gewicht der Grafik ist 0, wenn sie noch nicht
/*N*/   // gelesen ist, < 0, wenn ein Lesefehler auftrat und
/*N*/   // Ersatzdarstellung angewendet werden musste und >0,
/*N*/   // wenn sie zur Verfuegung steht.
/*N*/   nWeight = 0;
/*N*/ }

/*************************************************************************
|*
|*    SwNoTxtNode::MakeFrm()
|*
*************************************************************************/


/*N*/ SwCntntFrm *SwNoTxtNode::MakeFrm()
/*N*/ {
/*N*/   return new SwNoTxtFrm(this);
/*N*/ }

/*************************************************************************
|*
|*    SwNoTxtFrm::~SwNoTxtFrm()
|*
*************************************************************************/

/*N*/ SwNoTxtFrm::~SwNoTxtFrm()
/*N*/ {
/*N*/   StopAnimation();
/*N*/ }

/*************************************************************************
|*
|*    Size SwNoTxtFrm::GetSize()
|*
|*    Beschreibung      Gebe die Groesse des umgebenen FLys und
|*                      damit die der Grafik zurueck.
|*
*************************************************************************/


/*N*/ const Size& SwNoTxtFrm::GetSize() const
/*N*/ {
/*N*/   // gebe die Groesse des Frames zurueck
/*N*/   const SwFrm *pFly = FindFlyFrm();
/*N*/   if( !pFly )
/*?*/       pFly = this;
/*N*/   return pFly->Prt().SSize();
/*N*/ }

/*************************************************************************
|*
|*    SwNoTxtFrm::MakeAll()
|*
*************************************************************************/


/*N*/ void SwNoTxtFrm::MakeAll()
/*N*/ {
/*N*/   SwCntntNotify aNotify( this );
/*N*/   SwBorderAttrAccess aAccess( SwFrm::GetCache(), this );
/*N*/   const SwBorderAttrs &rAttrs = *aAccess.Get();
/*N*/
/*N*/   while ( !bValidPos || !bValidSize || !bValidPrtArea )
/*N*/   {
/*N*/       MakePos();
/*N*/
/*N*/       if ( !bValidSize )
/*N*/           Frm().Width( GetUpper()->Prt().Width() );
/*N*/
/*N*/       MakePrtArea( rAttrs );
/*N*/
/*N*/       if ( !bValidSize )
/*N*/       {   bValidSize = TRUE;
/*N*/           Format();
/*N*/       }
/*N*/   }
/*N*/ }

/*************************************************************************
|*
|*    SwNoTxtFrm::Format()
|*
|*    Beschreibung      Errechne die Groesse der Bitmap, wenn noetig
|*
*************************************************************************/


/*M*/ void SwNoTxtFrm::Format( const SwBorderAttrs * )
/*M*/ {
/*M*/   const Size aNewSize( GetSize() );
/*M*/
/*M*/   // hat sich die Hoehe geaendert?
/*M*/   SwTwips nChgHght = IsVertical() ?
/*M*/       (SwTwips)(aNewSize.Width() - Prt().Width()) :
/*M*/       (SwTwips)(aNewSize.Height() - Prt().Height());
/*M*/ #ifdef VERTICAL_LAYOUT
/*M*/   if( nChgHght > 0)
/*M*/         Grow( nChgHght );
/*M*/   else if( nChgHght < 0)
/*M*/         Shrink( Min(Prt().Height(), -nChgHght) );
/*M*/ #else
/*M*/   const SzPtr pVar = pVARSIZE;
/*M*/   if( nChgHght > 0)
/*M*/       Grow( nChgHght, pVar );
/*M*/   else if( nChgHght < 0)
/*M*/       Shrink( Min(Prt().Height(), -nChgHght), pVar );
/*M*/ #endif
/*M*/ }


/*N*/ #define CLEARCACHE( pNd ) {\
/*N*/   (pNd)->GetGrfObj().ReleaseFromCache();\
/*N*/   SwFlyFrm* pFly = FindFlyFrm();\
/*N*/   if( pFly && pFly->GetFmt()->GetSurround().IsContour() )\
/*N*/   {\
/*N*/       ClrContourCache( pFly->GetVirtDrawObj() );\
/*N*/       pFly->NotifyBackground( FindPageFrm(), Prt(), PREP_FLY_ATTR_CHG );\
/*N*/   }\
/*N*/ }

/*N*/ void SwNoTxtFrm::Modify( SfxPoolItem* pOld, SfxPoolItem* pNew )
/*N*/ {
/*N*/   USHORT nWhich = pNew ? pNew->Which() : pOld ? pOld->Which() : 0;
/*N*/
/*N*/   if ( RES_GRAPHIC_PIECE_ARRIVED != nWhich &&
/*N*/        RES_GRAPHIC_ARRIVED != nWhich &&
/*N*/        RES_GRF_REREAD_AND_INCACHE != nWhich )
/*N*/       SwCntntFrm::Modify( pOld, pNew );
/*N*/
/*N*/   bool bCompletePaint2 = TRUE;
/*N*/
/*N*/   switch( nWhich )
/*N*/   {
/*N*/   case RES_OBJECTDYING:
/*N*/       break;
/*N*/
/*?*/   case RES_GRF_REREAD_AND_INCACHE:
/*?*/       break;
/*?*/
/*N*/   case RES_UPDATE_ATTR:
/*N*/   case RES_FMT_CHG:
/*N*/       CLEARCACHE( (SwGrfNode*) GetNode() )
/*N*/       break;
/*N*/
/*N*/   case RES_ATTRSET_CHG:
/*N*/       {
                USHORT n;
/*N*/           for( n = RES_GRFATR_BEGIN; n < RES_GRFATR_END; ++n )
/*N*/               if( SFX_ITEM_SET == ((SwAttrSetChg*)pOld)->GetChgSet()->
/*N*/                               GetItemState( n, FALSE ))
/*N*/               {
/*N*/                   CLEARCACHE( (SwGrfNode*) GetNode() )
/*N*/                   break;
/*N*/               }
/*N*/           if( RES_GRFATR_END == n )           // not found
/*N*/               return ;
/*N*/       }
/*N*/       break;
/*N*/
/*N*/   case RES_GRAPHIC_PIECE_ARRIVED:
/*N*/   case RES_GRAPHIC_ARRIVED:
/*N*/       if ( GetNode()->GetNodeType() == ND_GRFNODE )
/*N*/       {
/*N*/           bCompletePaint2 = FALSE;
/*N*/           SwGrfNode* pNd = (SwGrfNode*) GetNode();
/*N*/
/*N*/           CLEARCACHE( pNd )
/*N*/
/*N*/           SwRect aRect( Frm() );
/*N*/       }
/*N*/       break;
/*N*/
/*N*/   default:
/*?*/       if( !pNew || RES_GRFATR_BEGIN > nWhich || nWhich >= RES_GRFATR_END )
/*?*/           return;
/*N*/   }
/*N*/
/*N*/   if( bCompletePaint2 )
/*N*/   {
/*N*/       InvalidatePrt();
/*N*/       SetCompletePaint();
/*N*/   }
/*N*/ }

/*N*/ void SwNoTxtFrm::StopAnimation( OutputDevice* pOut ) const
/*N*/ {
/*N*/   //animierte Grafiken anhalten
/*N*/   SwGrfNode* pGrfNd = (SwGrfNode*)GetNode()->GetGrfNode();
/*N*/   if( pGrfNd && pGrfNd->IsAnimated() )
/*N*/       pGrfNd->GetGrfObj().StopAnimation( pOut, long(this) );
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
