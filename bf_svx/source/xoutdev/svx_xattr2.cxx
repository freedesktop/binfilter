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

// include ---------------------------------------------------------------

#include <com/sun/star/drawing/LineJoint.hpp>

#include "dialogs.hrc"

#include <bf_svx/xdef.hxx>

#include "xattr.hxx"

#include "rectenum.hxx"
#include "xflbckit.hxx"
#include "xflbmpit.hxx"
#include "xflbmsli.hxx"
#include "xflbmsxy.hxx"
#include "xflbmtit.hxx"
#include "xflboxy.hxx"
#include "xflbstit.hxx"
#include "xflbtoxy.hxx"
#include "xfltrit.hxx"
#include "xftshtit.hxx"
#include "xgrscit.hxx"
#include "xlinjoit.hxx"
#include "xlntrit.hxx"

namespace binfilter {

#define GLOBALOVERFLOW

/************************************************************************/

//------------------------------
// class XLineTransparenceItem
//------------------------------
/*N*/ TYPEINIT1_AUTOFACTORY(XLineTransparenceItem, SfxUInt16Item);

/*************************************************************************
|*
|*    XLineTransparenceItem::XLineTransparenceItem(USHORT)
|*
*************************************************************************/

/*N*/ XLineTransparenceItem::XLineTransparenceItem(USHORT nLineTransparence) :
/*N*/   SfxUInt16Item(XATTR_LINETRANSPARENCE, nLineTransparence)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*    XLineTransparenceItem::XLineTransparenceItem(SvStream& rIn)
|*
*************************************************************************/

/*N*/ XLineTransparenceItem::XLineTransparenceItem(SvStream& rIn) :
/*N*/   SfxUInt16Item(XATTR_LINETRANSPARENCE, rIn)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*    XLineTransparenceItem::Clone(SfxItemPool* pPool) const
|*
*************************************************************************/

/*N*/ SfxPoolItem* XLineTransparenceItem::Clone(SfxItemPool* /*pPool*/) const
/*N*/ {
/*N*/   return new XLineTransparenceItem(*this);
/*N*/ }

/*************************************************************************
|*
|*    SfxPoolItem* XLineTransparenceItem::Create(SvStream& rIn, USHORT nVer) const
|*
*************************************************************************/

/*N*/ SfxPoolItem* XLineTransparenceItem::Create(SvStream& rIn, USHORT /*nVer*/) const
/*N*/ {
/*N*/   return new XLineTransparenceItem(rIn);
/*N*/ }

//------------------------------------------------------------------------


//-----------------------
// class XLineJointItem -
//-----------------------

/*N*/ TYPEINIT1_AUTOFACTORY(XLineJointItem, SfxEnumItem);

// -----------------------------------------------------------------------------

/*N*/ XLineJointItem::XLineJointItem( XLineJoint eLineJoint ) :
/*N*/   SfxEnumItem(XATTR_LINEJOINT, eLineJoint)
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ XLineJointItem::XLineJointItem( SvStream& rIn ) :
/*N*/   SfxEnumItem( XATTR_LINEJOINT, rIn )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ USHORT XLineJointItem::GetVersion( USHORT /*nFileFormatVersion*/ ) const
/*N*/ {
/*N*/   return 1;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ SfxPoolItem* XLineJointItem::Create( SvStream& rIn, USHORT nVer ) const
/*N*/ {
/*N*/   XLineJointItem* pRet = new XLineJointItem( rIn );
/*N*/
/*N*/   if(nVer < 1)
/*?*/       pRet->SetValue(XLINEJOINT_ROUND);
/*N*/
/*N*/   return pRet;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ SfxPoolItem* XLineJointItem::Clone(SfxItemPool* /*pPool*/) const
/*N*/ {
/*N*/   return new XLineJointItem( *this );
/*N*/ }

// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------

/*N*/ bool XLineJointItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE /*nMemberId*/ ) const
/*N*/ {
/*N*/   ::com::sun::star::drawing::LineJoint eJoint = ::com::sun::star::drawing::LineJoint_NONE;
/*N*/
/*N*/   switch( GetValue() )
/*N*/   {
/*N*/   case XLINEJOINT_NONE:
/*N*/       break;
/*N*/   case XLINEJOINT_MIDDLE:
/*N*/       eJoint = ::com::sun::star::drawing::LineJoint_MIDDLE;
/*N*/       break;
/*N*/   case XLINEJOINT_BEVEL:
/*N*/       eJoint = ::com::sun::star::drawing::LineJoint_BEVEL;
/*N*/       break;
/*N*/   case XLINEJOINT_MITER:
/*N*/       eJoint = ::com::sun::star::drawing::LineJoint_MITER;
/*N*/       break;
/*N*/   case XLINEJOINT_ROUND:
/*N*/       eJoint = ::com::sun::star::drawing::LineJoint_ROUND;
/*N*/       break;
/*N*/   default:
/*N*/       OSL_FAIL( "Unknown LineJoint enum value!" );
/*N*/   }
/*N*/
/*N*/   rVal <<= eJoint;
/*N*/   return true;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ bool XLineJointItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE /*nMemberId*/ )
/*N*/ {
/*N*/   XLineJoint eJoint = XLINEJOINT_NONE;
/*N*/   ::com::sun::star::drawing::LineJoint eUnoJoint;
/*N*/
/*N*/   if(!(rVal >>= eUnoJoint))
/*N*/     {
/*?*/         // also try an int (for Basic)
/*?*/         sal_Int32 nLJ = 0;
/*?*/         if(!(rVal >>= nLJ))
/*?*/             return sal_False;
/*?*/         eUnoJoint = (::com::sun::star::drawing::LineJoint)nLJ;
/*N*/     }

/*N*/   switch( eUnoJoint )
/*N*/   {
/*N*/   case ::com::sun::star::drawing::LineJoint_NONE:
/*N*/       break;
/*N*/   case ::com::sun::star::drawing::LineJoint_MIDDLE:
/*N*/       eJoint = XLINEJOINT_MIDDLE;
/*N*/       break;
/*N*/   case ::com::sun::star::drawing::LineJoint_BEVEL:
/*N*/       eJoint = XLINEJOINT_BEVEL;
/*N*/       break;
/*N*/   case ::com::sun::star::drawing::LineJoint_MITER:
/*N*/       eJoint = XLINEJOINT_MITER;
/*N*/       break;
/*N*/   case ::com::sun::star::drawing::LineJoint_ROUND:
/*N*/       eJoint = XLINEJOINT_ROUND;
/*N*/       break;
/*N*/   default:
/*N*/       break;
/*N*/   }
/*N*/
/*N*/   SetValue( eJoint );
/*N*/
/*N*/   return true;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ USHORT XLineJointItem::GetValueCount() const
/*N*/ {
/*N*/   // don't forget to update the api interface also
/*N*/   return 5;
/*N*/ }

//------------------------------
// class XFillTransparenceItem
//------------------------------
/*N*/ TYPEINIT1_AUTOFACTORY(XFillTransparenceItem, SfxUInt16Item);

/*************************************************************************
|*
|*    XFillTransparenceItem::XFillTransparenceItem(USHORT)
|*
*************************************************************************/

/*N*/ XFillTransparenceItem::XFillTransparenceItem(USHORT nFillTransparence) :
/*N*/   SfxUInt16Item(XATTR_FILLTRANSPARENCE, nFillTransparence)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*    XFillTransparenceItem::XFillTransparenceItem(SvStream& rIn)
|*
*************************************************************************/

/*N*/ XFillTransparenceItem::XFillTransparenceItem(SvStream& rIn) :
/*N*/   SfxUInt16Item(XATTR_FILLTRANSPARENCE, rIn)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*    XFillTransparenceItem::Clone(SfxItemPool* pPool) const
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFillTransparenceItem::Clone(SfxItemPool* /*pPool*/) const
/*N*/ {
/*N*/   return new XFillTransparenceItem(*this);
/*N*/ }

/*************************************************************************
|*
|*    SfxPoolItem* XFillTransparenceItem::Create(SvStream& rIn, USHORT nVer) const
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFillTransparenceItem::Create(SvStream& rIn, USHORT /*nVer*/) const
/*N*/ {
/*N*/   return new XFillTransparenceItem(rIn);
/*N*/ }

//------------------------------------------------------------------------


//------------------------------
// class XFormTextShadowTranspItem
//------------------------------
/*N*/ TYPEINIT1_AUTOFACTORY(XFormTextShadowTranspItem, SfxUInt16Item);

/*************************************************************************
|*
|*    XFormTextShadowTranspItem::XFormTextShadowTranspItem(USHORT)
|*
*************************************************************************/

/*N*/ XFormTextShadowTranspItem::XFormTextShadowTranspItem(USHORT nShdwTransparence) :
/*N*/   SfxUInt16Item(XATTR_FORMTXTSHDWTRANSP, nShdwTransparence)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*    XFormTextShadowTranspItem::XFormTextShadowTranspItem(SvStream& rIn)
|*
*************************************************************************/

/*N*/ XFormTextShadowTranspItem::XFormTextShadowTranspItem(SvStream& rIn) :
/*N*/   SfxUInt16Item(XATTR_FORMTXTSHDWTRANSP, rIn)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*    XFormTextShadowTranspItem::Clone(SfxItemPool* pPool) const
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFormTextShadowTranspItem::Clone(SfxItemPool* /*pPool*/) const
/*N*/ {
/*N*/   return new XFormTextShadowTranspItem(*this);
/*N*/ }

/*************************************************************************
|*
|*    SfxPoolItem* XFormTextShadowTranspItem::Create(SvStream& rIn, USHORT nVer) const
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFormTextShadowTranspItem::Create(SvStream& rIn, USHORT /*nVer*/) const
/*N*/ {
/*N*/   return new XFormTextShadowTranspItem(rIn);
/*N*/ }


//------------------------------
// class XFillGradientStepCountItem
//------------------------------
/*N*/ TYPEINIT1_AUTOFACTORY(XGradientStepCountItem, SfxUInt16Item);

/*************************************************************************
|*
|*    XGradientStepCountItem::XGradientStepCountItem( USHORT )
|*
*************************************************************************/

/*N*/ XGradientStepCountItem::XGradientStepCountItem( USHORT nStepCount ) :
/*N*/   SfxUInt16Item( XATTR_GRADIENTSTEPCOUNT, nStepCount )
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*    XGradientStepCountItem::XGradientStepCountItem( SvStream& rIn )
|*
*************************************************************************/

/*N*/ XGradientStepCountItem::XGradientStepCountItem( SvStream& rIn ) :
/*N*/   SfxUInt16Item( XATTR_GRADIENTSTEPCOUNT, rIn )
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*    XGradientStepCountItem::Clone( SfxItemPool* pPool ) const
|*
*************************************************************************/

/*N*/ SfxPoolItem* XGradientStepCountItem::Clone( SfxItemPool* /*pPool*/ ) const
/*N*/ {
/*N*/   return new XGradientStepCountItem( *this );
/*N*/ }

/*************************************************************************
|*
|*    SfxPoolItem* XGradientStepCountItem::Create(SvStream& rIn, USHORT nVer) const
|*
*************************************************************************/

/*N*/ SfxPoolItem* XGradientStepCountItem::Create(SvStream& rIn, USHORT /*nVer*/) const
/*N*/ {
/*N*/   return new XGradientStepCountItem( rIn );
/*N*/ }

//------------------------------------------------------------------------



//------------------------------
// class XFillBmpTileItem
//------------------------------
/*N*/ TYPEINIT1_AUTOFACTORY( XFillBmpTileItem, SfxBoolItem );

/*N*/ XFillBmpTileItem::XFillBmpTileItem( BOOL bTile ) :
/*N*/           SfxBoolItem( XATTR_FILLBMP_TILE, bTile )
/*N*/ {
/*N*/ }

/*N*/ XFillBmpTileItem::XFillBmpTileItem( SvStream& rIn ) :
/*N*/           SfxBoolItem( XATTR_FILLBMP_TILE, rIn )
/*N*/ {
/*N*/ }

/*N*/ SfxPoolItem* XFillBmpTileItem::Clone( SfxItemPool* /*pPool*/ ) const
/*N*/ {
/*N*/   return new XFillBmpTileItem( *this );
/*N*/ }

/*N*/ SfxPoolItem* XFillBmpTileItem::Create( SvStream& rIn, USHORT /*nVer*/ ) const
/*N*/ {
/*N*/   return new XFillBmpTileItem( rIn );
/*N*/ }

//------------------------------
// class XFillBmpTilePosItem
//------------------------------
/*N*/ TYPEINIT1_AUTOFACTORY( XFillBmpPosItem, SfxEnumItem );

/*N*/ XFillBmpPosItem::XFillBmpPosItem( RECT_POINT eRP ) :
/*N*/           SfxEnumItem( XATTR_FILLBMP_POS, eRP )
/*N*/ {
/*N*/ }

/*N*/ XFillBmpPosItem::XFillBmpPosItem( SvStream& rIn ) :
/*N*/           SfxEnumItem( XATTR_FILLBMP_POS, rIn )
/*N*/ {
/*N*/ }

/*N*/ SfxPoolItem* XFillBmpPosItem::Clone( SfxItemPool* /*pPool*/ ) const
/*N*/ {
/*N*/   return new XFillBmpPosItem( *this );
/*N*/ }

/*N*/ SfxPoolItem* XFillBmpPosItem::Create( SvStream& rIn, USHORT /*nVer*/ ) const
/*N*/ {
/*N*/   return new XFillBmpPosItem( rIn );
/*N*/ }

/******************************************************************************
|*
|*
|*
\******************************************************************************/

/*N*/ USHORT XFillBmpPosItem::GetValueCount() const
/*N*/ {
/*N*/   return 9;
/*N*/ }


//------------------------------
// class XFillBmpTileSizeXItem
//------------------------------
/*N*/ TYPEINIT1_AUTOFACTORY( XFillBmpSizeXItem, SfxMetricItem );

/*N*/ XFillBmpSizeXItem::XFillBmpSizeXItem( long nSizeX ) :
/*N*/           SfxMetricItem( XATTR_FILLBMP_SIZEX, nSizeX )
/*N*/ {
/*N*/ }

/*N*/ XFillBmpSizeXItem::XFillBmpSizeXItem( SvStream& rIn ) :
/*N*/           SfxMetricItem( XATTR_FILLBMP_SIZEX, rIn )
/*N*/ {
/*N*/ }

/*N*/ SfxPoolItem* XFillBmpSizeXItem::Clone( SfxItemPool* /*pPool*/ ) const
/*N*/ {
/*N*/   return new XFillBmpSizeXItem( *this );
/*N*/ }

/*N*/ SfxPoolItem* XFillBmpSizeXItem::Create( SvStream& rIn, USHORT /*nVer*/ ) const
/*N*/ {
/*N*/   return new XFillBmpSizeXItem( rIn );
/*N*/ }

/*************************************************************************
|*
\*************************************************************************/



//------------------------------
// class XFillBmpTileSizeYItem
//------------------------------
/*N*/ TYPEINIT1_AUTOFACTORY( XFillBmpSizeYItem, SfxMetricItem );

/*N*/ XFillBmpSizeYItem::XFillBmpSizeYItem( long nSizeY ) :
/*N*/           SfxMetricItem( XATTR_FILLBMP_SIZEY, nSizeY )
/*N*/ {
/*N*/ }

/*N*/ XFillBmpSizeYItem::XFillBmpSizeYItem( SvStream& rIn ) :
/*N*/           SfxMetricItem( XATTR_FILLBMP_SIZEY, rIn )
/*N*/ {
/*N*/ }

/*N*/ SfxPoolItem* XFillBmpSizeYItem::Clone( SfxItemPool* /*pPool*/ ) const
/*N*/ {
/*N*/   return new XFillBmpSizeYItem( *this );
/*N*/ }

/*N*/ SfxPoolItem* XFillBmpSizeYItem::Create( SvStream& rIn, USHORT /*nVer*/ ) const
/*N*/ {
/*N*/   return new XFillBmpSizeYItem( rIn );
/*N*/ }

/*************************************************************************
|*
\*************************************************************************/



//------------------------------
// class XFillBmpTileLogItem
//------------------------------
/*N*/ TYPEINIT1_AUTOFACTORY( XFillBmpSizeLogItem, SfxBoolItem );

/*N*/ XFillBmpSizeLogItem::XFillBmpSizeLogItem( BOOL bLog ) :
/*N*/           SfxBoolItem( XATTR_FILLBMP_SIZELOG, bLog )
/*N*/ {
/*N*/ }

/*N*/ XFillBmpSizeLogItem::XFillBmpSizeLogItem( SvStream& rIn ) :
/*N*/           SfxBoolItem( XATTR_FILLBMP_SIZELOG, rIn )
/*N*/ {
/*N*/ }

/*N*/ SfxPoolItem* XFillBmpSizeLogItem::Clone( SfxItemPool* /*pPool*/ ) const
/*N*/ {
/*N*/   return new XFillBmpSizeLogItem( *this );
/*N*/ }

/*N*/ SfxPoolItem* XFillBmpSizeLogItem::Create( SvStream& rIn, USHORT /*nVer*/ ) const
/*N*/ {
/*N*/   return new XFillBmpSizeLogItem( rIn );
/*N*/ }

//------------------------------
// class XFillBmpTileOffXItem
//------------------------------
/*N*/ TYPEINIT1_AUTOFACTORY( XFillBmpTileOffsetXItem, SfxUInt16Item );

/*N*/ XFillBmpTileOffsetXItem::XFillBmpTileOffsetXItem( USHORT nOffX ) :
/*N*/           SfxUInt16Item( XATTR_FILLBMP_TILEOFFSETX, nOffX )
/*N*/ {
/*N*/ }

/*N*/ XFillBmpTileOffsetXItem::XFillBmpTileOffsetXItem( SvStream& rIn ) :
/*N*/           SfxUInt16Item( XATTR_FILLBMP_TILEOFFSETX, rIn )
/*N*/ {
/*N*/ }

/*N*/ SfxPoolItem* XFillBmpTileOffsetXItem::Clone( SfxItemPool* /*pPool*/ ) const
/*N*/ {
/*N*/   return new XFillBmpTileOffsetXItem( *this );
/*N*/ }

/*N*/ SfxPoolItem* XFillBmpTileOffsetXItem::Create( SvStream& rIn, USHORT /*nVer*/ ) const
/*N*/ {
/*N*/   return new XFillBmpTileOffsetXItem( rIn );
/*N*/ }

//------------------------------
// class XFillBmpTileOffYItem
//------------------------------
/*N*/ TYPEINIT1_AUTOFACTORY( XFillBmpTileOffsetYItem, SfxUInt16Item );

/*N*/ XFillBmpTileOffsetYItem::XFillBmpTileOffsetYItem( USHORT nOffY ) :
/*N*/           SfxUInt16Item( XATTR_FILLBMP_TILEOFFSETY, nOffY )
/*N*/ {
/*N*/ }

/*N*/ XFillBmpTileOffsetYItem::XFillBmpTileOffsetYItem( SvStream& rIn ) :
/*N*/           SfxUInt16Item( XATTR_FILLBMP_TILEOFFSETY, rIn )
/*N*/ {
/*N*/ }

/*N*/ SfxPoolItem* XFillBmpTileOffsetYItem::Clone( SfxItemPool* /*pPool*/ ) const
/*N*/ {
/*N*/   return new XFillBmpTileOffsetYItem( *this );
/*N*/ }

/*N*/ SfxPoolItem* XFillBmpTileOffsetYItem::Create( SvStream& rIn, USHORT /*nVer*/ ) const
/*N*/ {
/*N*/   return new XFillBmpTileOffsetYItem( rIn );
/*N*/ }

//------------------------------
// class XFillBmpStretchItem
//------------------------------
/*N*/ TYPEINIT1_AUTOFACTORY( XFillBmpStretchItem, SfxBoolItem );

/*N*/ XFillBmpStretchItem::XFillBmpStretchItem( BOOL bStretch ) :
/*N*/           SfxBoolItem( XATTR_FILLBMP_STRETCH, bStretch )
/*N*/ {
/*N*/ }

/*N*/ XFillBmpStretchItem::XFillBmpStretchItem( SvStream& rIn ) :
/*N*/           SfxBoolItem( XATTR_FILLBMP_STRETCH, rIn )
/*N*/ {
/*N*/ }

/*N*/ SfxPoolItem* XFillBmpStretchItem::Clone( SfxItemPool* /*pPool*/ ) const
/*N*/ {
/*N*/   return new XFillBmpStretchItem( *this );
/*N*/ }

/*N*/ SfxPoolItem* XFillBmpStretchItem::Create( SvStream& rIn, USHORT /*nVer*/ ) const
/*N*/ {
/*N*/   return new XFillBmpStretchItem( rIn );
/*N*/ }

//------------------------------
// class XFillBmpTileOffPosXItem
//------------------------------
/*N*/ TYPEINIT1_AUTOFACTORY( XFillBmpPosOffsetXItem, SfxUInt16Item );

/*N*/ XFillBmpPosOffsetXItem::XFillBmpPosOffsetXItem( USHORT nOffPosX ) :
/*N*/           SfxUInt16Item( XATTR_FILLBMP_POSOFFSETX, nOffPosX )
/*N*/ {
/*N*/ }

/*N*/ XFillBmpPosOffsetXItem::XFillBmpPosOffsetXItem( SvStream& rIn ) :
/*N*/           SfxUInt16Item( XATTR_FILLBMP_POSOFFSETX, rIn )
/*N*/ {
/*N*/ }

/*N*/ SfxPoolItem* XFillBmpPosOffsetXItem::Clone( SfxItemPool* /*pPool*/ ) const
/*N*/ {
/*N*/   return new XFillBmpPosOffsetXItem( *this );
/*N*/ }

/*N*/ SfxPoolItem* XFillBmpPosOffsetXItem::Create( SvStream& rIn, USHORT /*nVer*/ ) const
/*N*/ {
/*N*/   return new XFillBmpPosOffsetXItem( rIn );
/*N*/ }

//------------------------------
// class XFillBmpTileOffPosYItem
//------------------------------
/*N*/ TYPEINIT1_AUTOFACTORY( XFillBmpPosOffsetYItem, SfxUInt16Item );

/*N*/ XFillBmpPosOffsetYItem::XFillBmpPosOffsetYItem( USHORT nOffPosY ) :
/*N*/           SfxUInt16Item( XATTR_FILLBMP_POSOFFSETY, nOffPosY )
/*N*/ {
/*N*/ }

/*N*/ XFillBmpPosOffsetYItem::XFillBmpPosOffsetYItem( SvStream& rIn ) :
/*N*/           SfxUInt16Item( XATTR_FILLBMP_POSOFFSETY, rIn )
/*N*/ {
/*N*/ }

/*N*/ SfxPoolItem* XFillBmpPosOffsetYItem::Clone( SfxItemPool* /*pPool*/ ) const
/*N*/ {
/*N*/   return new XFillBmpPosOffsetYItem( *this );
/*N*/ }

/*N*/ SfxPoolItem* XFillBmpPosOffsetYItem::Create( SvStream& rIn, USHORT /*nVer*/ ) const
/*N*/ {
/*N*/   return new XFillBmpPosOffsetYItem( rIn );
/*N*/ }

//--------------------------
// class XFillBackgroundItem
//--------------------------
/*N*/ TYPEINIT1_AUTOFACTORY(XFillBackgroundItem, SfxBoolItem);

/*************************************************************************
|*
|*    XFillBackgroundItem::XFillBackgroundItem( BOOL )
|*
*************************************************************************/

/*N*/ XFillBackgroundItem::XFillBackgroundItem( BOOL bFill ) :
/*N*/   SfxBoolItem( XATTR_FILLBACKGROUND, bFill )
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*    XFillBackgroundItem::XFillBackgroundItem( SvStream& rIn )
|*
*************************************************************************/

/*N*/ XFillBackgroundItem::XFillBackgroundItem( SvStream& rIn ) :
/*N*/   SfxBoolItem( XATTR_FILLBACKGROUND, rIn )
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*    XFillBackgroundItem::Clone( SfxItemPool* pPool ) const
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFillBackgroundItem::Clone( SfxItemPool* /*pPool*/ ) const
/*N*/ {
/*N*/   return new XFillBackgroundItem( *this );
/*N*/ }

/*************************************************************************
|*
|*    SfxPoolItem* XFillBackgroundItem::Create(SvStream& rIn, USHORT nVer) const
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFillBackgroundItem::Create(SvStream& rIn, USHORT /*nVer*/) const
/*N*/ {
/*N*/   return new XFillBackgroundItem( rIn );
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
