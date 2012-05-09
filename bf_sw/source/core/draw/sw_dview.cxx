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

#include "hintids.hxx"




#include <horiornt.hxx>

#include "frmfmt.hxx"
#include "dcontact.hxx"
#include "viewsh.hxx"
#include "viewimp.hxx"
#include "dview.hxx"

#include <fmtanchr.hxx>

// #i7672#

#include <vector>
namespace binfilter {

class SwSdrHdl : public SdrHdl
{
public:
    SwSdrHdl(const Point& rPnt, bool bTopRight ) :
        SdrHdl( rPnt, bTopRight ? HDL_ANCHOR_TR : HDL_ANCHOR ) {}
};



/*************************************************************************
|*
|*  SwDrawView::Ctor
|*
*************************************************************************/



/*N*/ SwDrawView::SwDrawView( SwViewImp &rI, SdrModel *pMd, OutputDevice *pOutDev) :
/*N*/   FmFormView( (FmFormModel*)pMd, pOutDev ),
/*N*/   rImp( rI )
/*N*/ {
/*N*/   SetPageVisible( FALSE );
/*N*/   SetBordVisible( FALSE );
/*N*/   SetGridVisible( FALSE );
/*N*/   SetHlplVisible( FALSE );
/*N*/   SetGlueVisible( FALSE );
/*N*/   SetFrameDragSingles( TRUE );
/*N*/   SetVirtualObjectBundling( TRUE );
/*N*/   SetSwapAsynchron( TRUE );
/*N*/
/*N*/   EnableExtendedKeyInputDispatcher( FALSE );
/*N*/   EnableExtendedMouseEventDispatcher( FALSE );
/*N*/   EnableExtendedCommandEventDispatcher( FALSE );
/*N*/
/*N*/   SetHitTolerancePixel( GetMarkHdlSizePixel()/2 );
/*N*/
/*N*/   SetPrintPreview( rI.GetShell()->IsPreView() );
/*N*/ }

/*************************************************************************
|*
|*  SwDrawView::AddCustomHdl()
|*
|*  Gets called every time the handles need to be build
|*
*************************************************************************/

/*M*/ void SwDrawView::AddCustomHdl()
/*M*/ {
/*M*/   const SdrMarkList &rMrkList = GetMarkList();
/*M*/
/*M*/   if(rMrkList.GetMarkCount() != 1 || !GetUserCall(rMrkList.GetMark( 0 )->GetObj()))
/*M*/       return;
/*M*/
/*?*/   SdrObject *pObj = rMrkList.GetMark(0)->GetObj();
/*?*/   const SwFmtAnchor &rAnchor = ::binfilter::FindFrmFmt(pObj)->GetAnchor();
/*?*/
/*?*/   if(FLY_IN_CNTNT == rAnchor.GetAnchorId())
/*?*/       return;
/*M*/ }


/*************************************************************************
|*
|*  SwDrawView::MarkListHasChanged()
|*
*************************************************************************/

/*N*/ void SwDrawView::MarkListHasChanged()
/*N*/ {
/*N*/   Imp().GetShell()->DrawSelChanged(this);
/*N*/   FmFormView::MarkListHasChanged();
/*N*/ }

// #i7672#
/*N*/ void SwDrawView::ModelHasChanged()
/*N*/ {
    // The ModelHasChanged() call in DrawingLayer also updates
    // a eventually active text edit view (OutlinerView). This also leads
    // to newly setting the background color for that edit view. Thus,
    // this method rescues the current background color if a OutlinerView
    // exists and re-establishes it then. To be more safe, the OutlinerView
    // will be fetched again (maybe textedit has ended).
/*N*/
/*N*/   // call parent
/*N*/   FmFormView::ModelHasChanged();
/*N*/
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
