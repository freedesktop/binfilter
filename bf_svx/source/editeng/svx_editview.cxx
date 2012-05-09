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

#define _SOLAR__PRIVATE 1

#include <vcl/window.hxx>

#include <impedit.hxx>
#include <editeng.hxx>
#include <editview.hxx>

#include "itemdata.hxx"

#include <editeng.hrc>
#include <helpid.hrc>

#include <com/sun/star/beans/PropertyValues.hdl>

namespace binfilter {

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::linguistic2;


/*N*/ DBG_NAME( EditView )

// From SW => Create common method

/*N*/ EditView::~EditView()
/*N*/ {
/*N*/   DBG_DTOR( EditView, 0 );
/*N*/   delete pImpEditView;
/*N*/ }


/*N*/ EditEngine* EditView::GetEditEngine() const
/*N*/ {
/*N*/   DBG_CHKTHIS( EditView, 0 );
/*N*/   DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
/*N*/   return pImpEditView->pEditEngine;
/*N*/ }

/*N*/ void EditView::SetSelection( const ESelection& rESel )
/*N*/ {
/*N*/   DBG_CHKTHIS( EditView, 0 );
/*N*/   DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
/*N*/
/*N*/   EditSelection aNewSelection( PIMPEE->ConvertSelection( rESel.nStartPara, rESel.nStartPos, rESel.nEndPara, rESel.nEndPos ) );
/*N*/
/*N*/   // Selektion darf nicht bei einem unsichtbaren Absatz Starten/Enden:
/*N*/   PIMPEE->FindParaPortion( aNewSelection.Min().GetNode() ); // WaE cleanup: left in for potential side effects
/*N*/   PIMPEE->FindParaPortion( aNewSelection.Max().GetNode() ); // WaE cleanup: left in for potential side effects
/*N*/
/*N*/   pImpEditView->DrawSelection();  // alte Selektion 'weg-zeichnen'
/*N*/   pImpEditView->SetEditSelection( aNewSelection );
/*N*/   pImpEditView->DrawSelection();
/*N*/   sal_Bool bGotoCursor = pImpEditView->DoAutoScroll();
/*N*/   ShowCursor( bGotoCursor );
/*N*/ }

/*N*/ ESelection EditView::GetSelection() const
/*N*/ {
/*N*/   DBG_CHKTHIS( EditView, 0 );
/*N*/   DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
/*N*/
/*N*/   ESelection aSelection;
/*N*/
/*N*/   aSelection.nStartPara = PIMPEE->GetEditDoc().GetPos( pImpEditView->GetEditSelection().Min().GetNode() );
/*N*/   aSelection.nEndPara = PIMPEE->GetEditDoc().GetPos( pImpEditView->GetEditSelection().Max().GetNode() );
/*N*/
/*N*/   aSelection.nStartPos = pImpEditView->GetEditSelection().Min().GetIndex();
/*N*/   aSelection.nEndPos = pImpEditView->GetEditSelection().Max().GetIndex();
/*N*/
/*N*/   return aSelection;
/*N*/ }

/*N*/ sal_Bool EditView::HasSelection() const
/*N*/ {
/*N*/   DBG_CHKTHIS( EditView, 0 );
/*N*/   return pImpEditView->HasSelection();
/*N*/ }


/*N*/ Window*   EditView::GetWindow() const
/*N*/ {
/*N*/   DBG_CHKTHIS( EditView, 0 );
/*N*/   return pImpEditView->pOutWin;
/*N*/ }

/*N*/ const Rectangle& EditView::GetVisArea() const
/*N*/ {
/*N*/   DBG_CHKTHIS( EditView, 0 );
/*N*/   // Change return value to Rectangle in next incompatible build !!!
/*N*/   static Rectangle aRect;
/*N*/   aRect = pImpEditView->GetVisDocArea();
/*N*/   return aRect;
/*N*/ }

/*N*/ const Rectangle& EditView::GetOutputArea() const
/*N*/ {
/*N*/   DBG_CHKTHIS( EditView, 0 );
/*N*/   return pImpEditView->GetOutputArea();
/*N*/ }


/*N*/ void EditView::ShowCursor( sal_Bool, sal_Bool )
/*N*/ {
/*N*/ }

/*N*/ void EditView::HideCursor()
/*N*/ {
/*N*/   DBG_CHKTHIS( EditView, 0 );
/*N*/   pImpEditView->GetCursor()->Hide();
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
