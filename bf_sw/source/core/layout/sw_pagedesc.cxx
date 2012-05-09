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

#include <bf_svx/pbinitem.hxx>
#include <bf_svx/ulspitem.hxx>
#include <bf_svx/boxitem.hxx>
#include <bf_svx/brshitem.hxx>
#include <bf_svx/shaditem.hxx>
#include <bf_svx/lrspitem.hxx>
#include "bf_svx/frmdiritem.hxx"
#include <fmtclds.hxx>
#include <fmtfsize.hxx>
#include <frmatr.hxx>
#include <pagefrm.hxx>
#include <pagedesc.hxx>
#include <node.hxx>

#include <horiornt.hxx>

#include <doc.hxx>          // fuer GetAttrPool
namespace binfilter {

/*************************************************************************
|*
|*  SwPageDesc::SwPageDesc()
|*
|*************************************************************************/



/*N*/ SwPageDesc::SwPageDesc( const String& rName, SwFrmFmt *pFmt, SwDoc *pDc )
/*N*/   : SwModify( 0 )
/*N*/   , aDescName( rName )
/*N*/   , aMaster( pDc->GetAttrPool(), rName, pFmt )
/*N*/   , aLeft( pDc->GetAttrPool(), rName, pFmt )
/*N*/   , aDepend( this, 0 )
/*N*/   , pFollow( this )
/*N*/   , nRegHeight( 0 )
/*N*/   , nRegAscent( 0 )
/*N*/   , eUse( (UseOnPage)(PD_ALL | PD_HEADERSHARE | PD_FOOTERSHARE) )
/*N*/   , bLandscape( FALSE )
/*N*/   , aFtnInfo()
/*N*/ {
/*N*/ }



/*N*/ SwPageDesc::SwPageDesc( const SwPageDesc &rCpy )
/*N*/   : SwModify( 0 )
/*N*/   , aDescName( rCpy.GetName() )
/*N*/   , aNumType( rCpy.GetNumType() )
/*N*/   , aMaster( rCpy.GetMaster() )
/*N*/   , aLeft( rCpy.GetLeft() )
/*N*/   , aDepend( this, (SwModify*)rCpy.aDepend.GetRegisteredIn() )
/*N*/   , pFollow( rCpy.pFollow )
/*N*/   , nRegHeight( rCpy.GetRegHeight() )
/*N*/   , nRegAscent( rCpy.GetRegAscent() )
/*N*/   , eUse( rCpy.ReadUseOn() )
/*N*/   , bLandscape( rCpy.GetLandscape() )
/*N*/   , aFtnInfo( rCpy.GetFtnInfo() )
/*N*/ {
/*N*/ }



/*N*/ SwPageDesc::~SwPageDesc()
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*  SwPageDesc::Mirror()
|*
|*  Beschreibung        Gespiegelt werden nur die Raender.
|*      Attribute wie Umrandung und dergleichen werden 1:1 kopiert.
|*
|*************************************************************************/



/*N*/ void SwPageDesc::Mirror()
/*N*/ {
/*N*/   //Das Spiegeln findet nur beim RandAttribut statt, alle anderen Werte
/*N*/   //werden schlicht uebertragen.
/*N*/   SvxLRSpaceItem aLR;
/*N*/   const SvxLRSpaceItem &rLR = aMaster.GetLRSpace();
/*N*/   aLR.SetLeft(  rLR.GetRight() );
/*N*/   aLR.SetRight( rLR.GetLeft() );
/*N*/
/*N*/   SfxItemSet aSet( *aMaster.GetAttrSet().GetPool(),
/*N*/                    aMaster.GetAttrSet().GetRanges() );
/*N*/   aSet.Put( aLR );
/*N*/   aSet.Put( aMaster.GetFrmSize() );
/*N*/   aSet.Put( aMaster.GetPaperBin() );
/*N*/   aSet.Put( aMaster.GetULSpace() );
/*N*/   aSet.Put( aMaster.GetBox() );
/*N*/   aSet.Put( aMaster.GetBackground() );
/*N*/   aSet.Put( aMaster.GetShadow() );
/*N*/   aSet.Put( aMaster.GetCol() );
/*N*/   aLeft.SetAttr( aSet );
/*N*/ }

/*N*/ void SwPageDesc::ResetAllAttr( sal_Bool bLeft )
/*N*/ {
/*N*/   SwFrmFmt& rFmt = bLeft ? GetLeft() : GetMaster();
/*N*/
/*N*/   rFmt.ResetAllAttr();
/*N*/   rFmt.SetAttr( SvxFrameDirectionItem() );
/*N*/ }

/*************************************************************************
|*
|*                SwPageDesc::GetInfo()
|*
|*    Beschreibung      erfragt Informationen
|*
*************************************************************************/


    // erfrage vom Modify Informationen

/*************************************************************************
|*
|*                SwPageDesc::SetRegisterFmtColl()
|*
|*    Beschreibung      setzt die Vorlage fuer die Registerhaltigkeit
|*
*************************************************************************/


/*N*/ void SwPageDesc::SetRegisterFmtColl( const SwTxtFmtColl* pFmt )
/*N*/ {
/*N*/   if( pFmt != GetRegisterFmtColl() )
/*N*/   {
/*N*/       if( pFmt )
/*N*/           ((SwTxtFmtColl*)pFmt)->Add( &aDepend );
/*N*/       else
/*?*/           ((SwTxtFmtColl*)GetRegisterFmtColl())->Remove( &aDepend );
/*N*/
/*N*/       RegisterChange();
/*N*/   }
/*N*/ }

/*************************************************************************
|*
|*                SwPageDesc::GetRegisterFmtColl()
|*
|*    Beschreibung      holt die Vorlage fuer die Registerhaltigkeit
|*
*************************************************************************/


/*N*/ const SwTxtFmtColl* SwPageDesc::GetRegisterFmtColl() const
/*N*/ {
/*N*/   const SwModify* pReg = aDepend.GetRegisteredIn();
/*N*/   return (SwTxtFmtColl*)pReg;
/*N*/ }

/*************************************************************************
|*
|*                SwPageDesc::RegisterChange()
|*
|*    Beschreibung      benachrichtigt alle betroffenen PageFrames
|*
*************************************************************************/


/*N*/ void SwPageDesc::RegisterChange()
/*N*/ {
/*N*/   nRegHeight = 0;
/*N*/   {
/*N*/       SwClientIter aIter( GetMaster() );
/*N*/       for( SwClient* pLast = aIter.First(TYPE(SwFrm)); pLast;
/*N*/               pLast = aIter.Next() )
/*N*/       {
/*N*/           if( ((SwFrm*)pLast)->IsPageFrm() )
/*N*/               ((SwPageFrm*)pLast)->PrepareRegisterChg();
/*N*/       }
/*N*/   }
/*N*/   {
/*N*/       SwClientIter aIter( GetLeft() );
/*N*/       for( SwClient* pLast = aIter.First(TYPE(SwFrm)); pLast;
/*N*/               pLast = aIter.Next() )
/*N*/       {
/*N*/           if( ((SwFrm*)pLast)->IsPageFrm() )
/*N*/               ((SwPageFrm*)pLast)->PrepareRegisterChg();
/*N*/       }
/*N*/   }
/*N*/ }

/*************************************************************************
|*
|*                SwPageDesc::Modify()
|*
|*    Beschreibung      reagiert insbesondere auf Aenderungen
|*                      der Vorlage fuer die Registerhaltigkeit
|*
*************************************************************************/


/*N*/ void SwPageDesc::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew )
/*N*/ {
/*N*/   const USHORT nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
/*N*/   SwModify::Modify( pOld, pNew );
/*N*/
/*N*/   if( RES_ATTRSET_CHG == nWhich || RES_FMT_CHG == nWhich ||
/*N*/       ( nWhich >= RES_CHRATR_BEGIN && nWhich < RES_CHRATR_END ) ||
/*N*/        nWhich == RES_PARATR_LINESPACING )
/*N*/       RegisterChange();
/*N*/ }





/*************************************************************************
|*
|*  SwPageFtnInfo::SwPageFtnInfo()
|*
|*************************************************************************/



/*N*/ SwPageFtnInfo::SwPageFtnInfo() :
/*N*/   nMaxHeight( 0 ),
/*N*/ //    aPen( PEN_SOLID ),
/*N*/   nLineWidth(10),
/*N*/   aWidth( 25, 100 ),
/*N*/   eAdj( FTNADJ_LEFT ),
/*N*/   nTopDist( 57 ),         //1mm
/*N*/   nBottomDist( 57 )
/*N*/ {
/*N*/ //    aPen.SetWidth( 10 );
/*N*/ }



/*N*/ SwPageFtnInfo::SwPageFtnInfo( const SwPageFtnInfo &rCpy ) :
/*N*/   nMaxHeight( rCpy.GetHeight() ),
/*N*/ //    aPen( rCpy.GetPen() ),
/*N*/   nLineWidth(rCpy.nLineWidth),
/*N*/   aLineColor(rCpy.aLineColor),
/*N*/   aWidth( rCpy.GetWidth() ),
/*N*/   eAdj( rCpy.GetAdj() ),
/*N*/   nTopDist( rCpy.GetTopDist() ),
/*N*/   nBottomDist( rCpy.GetBottomDist() )
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*  SwPageFtnInfo::operator=
|*
|*************************************************************************/



/*N*/ SwPageFtnInfo &SwPageFtnInfo::operator=( const SwPageFtnInfo& rCpy )
/*N*/ {
/*N*/   nMaxHeight  = rCpy.GetHeight();
/*N*/ //    aPen        = rCpy.GetPen();
/*N*/   nLineWidth  = rCpy.nLineWidth;
/*N*/   aLineColor  = rCpy.aLineColor;
/*N*/   aWidth      = rCpy.GetWidth();
/*N*/   eAdj        = rCpy.GetAdj();
/*N*/   nTopDist    = rCpy.GetTopDist();
/*N*/   nBottomDist = rCpy.GetBottomDist();
/*N*/   return *this;
/*N*/ }
/*************************************************************************
|*
|*  SwPageFtnInfo::operator==
|*
|*************************************************************************/



/*N*/ BOOL SwPageFtnInfo::operator==( const SwPageFtnInfo& rCmp ) const
/*N*/ {
/*N*/ //    const Pen aTmp( rCmp.GetPen() );
/*N*/   return ( nMaxHeight == rCmp.GetHeight() &&
/*N*/ //             aPen       == aTmp &&
/*N*/            nLineWidth == rCmp.nLineWidth &&
/*N*/            aLineColor == rCmp.aLineColor &&
/*N*/            aWidth     == rCmp.GetWidth() &&
/*N*/            eAdj       == rCmp.GetAdj() &&
/*N*/            nTopDist   == rCmp.GetTopDist() &&
/*N*/            nBottomDist== rCmp.GetBottomDist() );
/*N*/ }




}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
