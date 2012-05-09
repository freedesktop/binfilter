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

#include <bf_svx/svdmodel.hxx>
#include <bf_svx/svdpage.hxx>

#include <fmtpdsc.hxx>
#include <swtable.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <viewsh.hxx>

#include <horiornt.hxx>

#include <doc.hxx>
#include <dflyobj.hxx>
#include <frmtool.hxx>
#include <virtoutp.hxx>
#include <blink.hxx>
#include <ndindex.hxx>
#include <sectfrm.hxx>
#include <notxtfrm.hxx>
#include <pagedesc.hxx>
namespace binfilter {

/*N*/ #ifndef VERTICAL_LAYOUT
/*N*/ PtPtr pX = &Point::nA;
/*N*/ PtPtr pY = &Point::nB;
/*N*/ SzPtr pWidth = &Size::nA;
/*N*/ SzPtr pHeight = &Size::nB;
/*N*/ #endif

/*N*/ SwLayVout   *SwRootFrm::pVout = 0;
/*N*/ BOOL         SwRootFrm::bInPaint = FALSE;
/*N*/ BOOL         SwRootFrm::bNoVirDev = FALSE;

/*N*/ SwCache *SwFrm::pCache = 0;

/*N*/ Bitmap* SwNoTxtFrm::pErrorBmp = 0;
/*N*/ Bitmap* SwNoTxtFrm::pReplaceBmp = 0;
/*N*/
/*N*/ #ifdef VERTICAL_LAYOUT

/*N*/ long FirstMinusSecond( long nFirst, long nSecond )
/*N*/     { return nFirst - nSecond; }
/*N*/ long SecondMinusFirst( long nFirst, long nSecond )
/*N*/     { return nSecond - nFirst; }
/*N*/ long SwIncrement( long nA, long nAdd )
/*N*/     { return nA + nAdd; }
/*N*/ long SwDecrement( long nA, long nSub )
/*N*/     { return nA - nSub; }

/*N*/ static SwRectFnCollection aHorizontal = {
/*N*/     /* fnRectGet      */
/*N*/     &SwRect::_Top,
/*N*/     &SwRect::_Bottom,
/*N*/     &SwRect::_Left,
/*N*/     &SwRect::_Right,
/*N*/     &SwRect::_Width,
/*N*/     &SwRect::_Height,
/*N*/     &SwRect::TopLeft,
/*N*/     &SwRect::_Size,
/*N*/     /* fnRectSet      */
/*N*/     &SwRect::_Top,
/*N*/     &SwRect::_Bottom,
/*N*/     &SwRect::_Left,
/*N*/     &SwRect::_Right,
/*N*/     &SwRect::_Width,
/*N*/     &SwRect::_Height,
/*N*/
/*N*/     &SwRect::SubTop,
/*N*/     &SwRect::AddBottom,
/*N*/     &SwRect::SubLeft,
/*N*/     &SwRect::AddRight,
/*N*/     &SwRect::AddWidth,
/*N*/     &SwRect::AddHeight,
/*N*/
/*N*/     &SwRect::SetPosX,
/*N*/     &SwRect::SetPosY,
/*N*/
/*N*/     &SwFrm::GetTopMargin,
/*N*/     &SwFrm::GetBottomMargin,
/*N*/     &SwFrm::GetLeftMargin,
/*N*/     &SwFrm::GetRightMargin,
/*N*/     &SwFrm::SetLeftRightMargins,
/*N*/     &SwFrm::SetTopBottomMargins,
/*N*/     &SwFrm::GetPrtTop,
/*N*/     &SwFrm::GetPrtBottom,
/*N*/     &SwFrm::GetPrtLeft,
/*N*/     &SwFrm::GetPrtRight,
/*N*/     &SwRect::GetTopDistance,
/*N*/     &SwRect::GetBottomDistance,
/*N*/     &SwRect::GetLeftDistance,
/*N*/     &SwRect::GetRightDistance,
/*N*/     &SwFrm::SetMaxBottom,
/*N*/     &SwRect::OverStepBottom,
/*N*/
/*N*/     &SwRect::SetUpperLeftCorner,
/*N*/     &SwFrm::MakeBelowPos,
/*N*/     &FirstMinusSecond,
/*N*/     &FirstMinusSecond,
/*N*/     &SwIncrement,
/*N*/     &SwIncrement,
/*N*/     &SwRect::SetLeftAndWidth,
/*N*/     &SwRect::SetTopAndHeight
/*N*/ };
/*N*/
/*N*/ static SwRectFnCollection aVertical = {
/*N*/     /* fnRectGet      */
/*N*/     &SwRect::_Right,
/*N*/     &SwRect::_Left,
/*N*/     &SwRect::_Top,
/*N*/     &SwRect::_Bottom,
/*N*/     &SwRect::_Height,
/*N*/     &SwRect::_Width,
/*N*/     &SwRect::TopRight,
/*N*/     &SwRect::SwappedSize,
/*N*/     /* fnRectSet      */
/*N*/     &SwRect::_Right,
/*N*/     &SwRect::_Left,
/*N*/     &SwRect::_Top,
/*N*/     &SwRect::_Bottom,
/*N*/     &SwRect::_Height,
/*N*/     &SwRect::_Width,
/*N*/
/*N*/     &SwRect::AddRight,
/*N*/     &SwRect::SubLeft,
/*N*/     &SwRect::SubTop,
/*N*/     &SwRect::AddBottom,
/*N*/     &SwRect::AddHeight,
/*N*/     &SwRect::AddWidth,
/*N*/
/*N*/     &SwRect::SetPosY,
/*N*/     &SwRect::SetPosX,
/*N*/
/*N*/     &SwFrm::GetRightMargin,
/*N*/     &SwFrm::GetLeftMargin,
/*N*/     &SwFrm::GetTopMargin,
/*N*/     &SwFrm::GetBottomMargin,
/*N*/     &SwFrm::SetTopBottomMargins,
/*N*/     &SwFrm::SetRightLeftMargins,
/*N*/     &SwFrm::GetPrtRight,
/*N*/     &SwFrm::GetPrtLeft,
/*N*/     &SwFrm::GetPrtTop,
/*N*/     &SwFrm::GetPrtBottom,
/*N*/     &SwRect::GetRightDistance,
/*N*/     &SwRect::GetLeftDistance,
/*N*/     &SwRect::GetTopDistance,
/*N*/     &SwRect::GetBottomDistance,
/*N*/     &SwFrm::SetMinLeft,
/*N*/     &SwRect::OverStepLeft,
/*N*/
/*N*/     &SwRect::SetUpperRightCorner,
/*N*/     &SwFrm::MakeLeftPos,
/*N*/     &FirstMinusSecond,
/*N*/     &SecondMinusFirst,
/*N*/     &SwIncrement,
/*N*/     &SwDecrement,
/*N*/     &SwRect::SetTopAndHeight,
/*N*/     &SwRect::SetRightAndWidth
/*N*/ };
/*N*/
/*N*/ static SwRectFnCollection aBottomToTop = {
/*N*/     /* fnRectGet      */
/*N*/     &SwRect::_Bottom,
/*N*/     &SwRect::_Top,
/*N*/     &SwRect::_Left,
/*N*/     &SwRect::_Right,
/*N*/     &SwRect::_Width,
/*N*/     &SwRect::_Height,
/*N*/     &SwRect::BottomLeft,
/*N*/     &SwRect::_Size,
/*N*/     /* fnRectSet      */
/*N*/     &SwRect::_Bottom,
/*N*/     &SwRect::_Top,
/*N*/     &SwRect::_Left,
/*N*/     &SwRect::_Right,
/*N*/     &SwRect::_Width,
/*N*/     &SwRect::_Height,
/*N*/
/*N*/     &SwRect::AddBottom,
/*N*/     &SwRect::SubTop,
/*N*/     &SwRect::SubLeft,
/*N*/     &SwRect::AddRight,
/*N*/     &SwRect::AddWidth,
/*N*/     &SwRect::AddHeight,
/*N*/
/*N*/     &SwRect::SetPosX,
/*N*/     &SwRect::SetPosY,
/*N*/
/*N*/     &SwFrm::GetBottomMargin,
/*N*/     &SwFrm::GetTopMargin,
/*N*/     &SwFrm::GetLeftMargin,
/*N*/     &SwFrm::GetRightMargin,
/*N*/     &SwFrm::SetLeftRightMargins,
/*N*/     &SwFrm::SetBottomTopMargins,
/*N*/     &SwFrm::GetPrtBottom,
/*N*/     &SwFrm::GetPrtTop,
/*N*/     &SwFrm::GetPrtLeft,
/*N*/     &SwFrm::GetPrtRight,
/*N*/     &SwRect::GetBottomDistance,
/*N*/     &SwRect::GetTopDistance,
/*N*/     &SwRect::GetLeftDistance,
/*N*/     &SwRect::GetRightDistance,
/*N*/     &SwFrm::SetMinTop,
/*N*/     &SwRect::OverStepTop,
/*N*/
/*N*/     &SwRect::SetLowerLeftCorner,
/*N*/     &SwFrm::MakeUpperPos,
/*N*/     &FirstMinusSecond,
/*N*/     &SecondMinusFirst,
/*N*/     &SwIncrement,
/*N*/     &SwDecrement,
/*N*/     &SwRect::SetLeftAndWidth,
/*N*/     &SwRect::SetBottomAndHeight
/*N*/ };
/*N*/
/*N*/ static SwRectFnCollection aVerticalRightToLeft = {
/*N*/     /* fnRectGet      */
/*N*/     &SwRect::_Left,
/*N*/     &SwRect::_Right,
/*N*/     &SwRect::_Top,
/*N*/     &SwRect::_Bottom,
/*N*/     &SwRect::_Height,
/*N*/     &SwRect::_Width,
/*N*/     &SwRect::BottomRight,
/*N*/     &SwRect::SwappedSize,
/*N*/     /* fnRectSet      */
/*N*/     &SwRect::_Left,
/*N*/     &SwRect::_Right,
/*N*/     &SwRect::_Top,
/*N*/     &SwRect::_Bottom,
/*N*/     &SwRect::_Height,
/*N*/     &SwRect::_Width,
/*N*/
/*N*/     &SwRect::SubLeft,
/*N*/     &SwRect::AddRight,
/*N*/     &SwRect::SubTop,
/*N*/     &SwRect::AddBottom,
/*N*/     &SwRect::AddHeight,
/*N*/     &SwRect::AddWidth,
/*N*/
/*N*/     &SwRect::SetPosY,
/*N*/     &SwRect::SetPosX,
/*N*/
/*N*/     &SwFrm::GetLeftMargin,
/*N*/     &SwFrm::GetRightMargin,
/*N*/     &SwFrm::GetTopMargin,
/*N*/     &SwFrm::GetBottomMargin,
/*N*/     &SwFrm::SetTopBottomMargins,
/*N*/     &SwFrm::SetLeftRightMargins,
/*N*/     &SwFrm::GetPrtLeft,
/*N*/     &SwFrm::GetPrtRight,
/*N*/     &SwFrm::GetPrtBottom,
/*N*/     &SwFrm::GetPrtTop,
/*N*/     &SwRect::GetLeftDistance,
/*N*/     &SwRect::GetRightDistance,
/*N*/     &SwRect::GetBottomDistance,
/*N*/     &SwRect::GetTopDistance,
/*N*/     &SwFrm::SetMaxRight,
/*N*/     &SwRect::OverStepRight,
/*N*/
/*N*/     &SwRect::SetLowerLeftCorner,
/*N*/     &SwFrm::MakeRightPos,
/*N*/     &FirstMinusSecond,
/*N*/     &FirstMinusSecond,
/*N*/     &SwDecrement,
/*N*/     &SwIncrement,
/*N*/     &SwRect::SetBottomAndHeight,
/*N*/     &SwRect::SetLeftAndWidth
/*N*/ };
/*N*/
/*N*/ SwRectFn fnRectHori = &aHorizontal;
/*N*/ SwRectFn fnRectVert = &aVertical;
/*N*/ SwRectFn fnRectB2T = &aBottomToTop;
/*N*/ SwRectFn fnRectVL2R = &aVerticalRightToLeft;

/*N*/ #endif

/*N*/ #ifdef DBG_UTIL
/*N*/ USHORT SwFrm::nLastFrmId=0;
/*N*/ #endif


/*N*/ TYPEINIT1(SwFrm,SwClient);        //rtti fuer SwFrm
/*N*/ TYPEINIT1(SwCntntFrm,SwFrm);  //rtti fuer SwCntntFrm


/*N*/ void _FrmInit()
/*N*/ {
/*N*/   SwRootFrm::pVout = new SwLayVout();
/*N*/   SwCache *pNew = new SwCache( 100, 100
/*N*/ #ifdef DBG_UTIL
/*N*/   , "static SwBorderAttrs::pCache"
/*N*/ #endif
/*N*/   );
/*N*/   SwFrm::SetCache( pNew );
/*N*/ }



/*N*/ void _FrmFinit()
/*N*/ {
/*N*/ #ifdef DBG_UTIL
/*N*/   // im Chache duerfen nur noch 0-Pointer stehen
/*N*/   for( USHORT n = SwFrm::GetCachePtr()->Count(); n; )
/*N*/       if( (*SwFrm::GetCachePtr())[ --n ] )
/*N*/       {
/*N*/           SwCacheObj* pObj = (*SwFrm::GetCachePtr())[ n ];
/*N*/           OSL_ENSURE( !pObj, "Wer hat sich nicht ausgetragen?");
/*N*/       }
/*N*/ #endif
/*N*/   delete SwRootFrm::pVout;
/*N*/   delete SwFrm::GetCachePtr();
/*N*/ }

/*N*/ CurrShell::CurrShell( ViewShell *pNew )
/*N*/ {
/*N*/   OSL_ENSURE( pNew, "0-Shell einsetzen?" );
        (void) pNew; // avoid warnings
/*?*/   pPrev = 0;
/*N*/ }

/*N*/ CurrShell::~CurrShell()
/*N*/ {
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
