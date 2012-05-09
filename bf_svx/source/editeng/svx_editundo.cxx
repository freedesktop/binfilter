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


#include <vcl/window.hxx>

#include <impedit.hxx>
#include <editeng.hxx>
namespace binfilter {

DBG_NAME( EditUndo )

/*N*/ TYPEINIT1( EditUndoInsertChars, EditUndo );


// -----------------------------------------------------------------------
// EditUndoManager
// ------------------------------------------------------------------------
/*N*/ EditUndoManager::EditUndoManager( ImpEditEngine* p )
/*N*/ {
/*N*/   pImpEE = p;
/*N*/ }

// -----------------------------------------------------------------------
// EditUndo
// ------------------------------------------------------------------------
/*N*/ EditUndo::EditUndo( USHORT nI, ImpEditEngine* p )
/*N*/ {
/*N*/   DBG_CTOR( EditUndo, 0 );
/*N*/   nId = nI;
/*N*/   pImpEE = p;
/*N*/ }

/*N*/ EditUndo::~EditUndo()
/*N*/ {
/*N*/   DBG_DTOR( EditUndo, 0 );
/*N*/ }

/*N*/ USHORT EditUndo::GetId() const
/*N*/ {
/*N*/   DBG_CHKTHIS( EditUndo, 0 );
/*N*/   return nId;
/*N*/ }


/*N*/ XubString EditUndo::GetComment() const
/*N*/ {
/*N*/   XubString aComment;
/*N*/   if ( pImpEE )
/*N*/   {
/*N*/       EditEngine* pEditEng = pImpEE->GetEditEnginePtr();
/*N*/       aComment = pEditEng->GetUndoComment( GetId() );
/*N*/   }
/*N*/   return aComment;
/*N*/ }

// -----------------------------------------------------------------------
// EditUndoInsertChars
// ------------------------------------------------------------------------
/*N*/ EditUndoInsertChars::EditUndoInsertChars( ImpEditEngine* pInImpEE, const EPaM& rEPaM, const XubString& rStr )
/*N*/                   : EditUndo( EDITUNDO_INSERTCHARS, pInImpEE ),
/*N*/                       aEPaM( rEPaM ), aText( rStr )
/*N*/ {
/*N*/ }

/*N*/ BOOL EditUndoInsertChars::Merge( SfxUndoAction* pNextAction )
/*N*/ {
/*N*/   if ( !pNextAction->ISA( EditUndoInsertChars ) )
/*N*/       return FALSE;
/*N*/
/*N*/   EditUndoInsertChars* pNext = (EditUndoInsertChars*)pNextAction;
/*N*/
/*N*/   if ( aEPaM.nPara != pNext->aEPaM.nPara )
/*N*/       return FALSE;
/*N*/
/*N*/   if ( ( aEPaM.nIndex + aText.Len() ) == pNext->aEPaM.nIndex )
/*N*/   {
/*N*/       aText += pNext->aText;
/*N*/       return TRUE;
/*N*/   }
/*N*/   return FALSE;
/*N*/ }

// -----------------------------------------------------------------------
// EditUndoInsertFeature
// ------------------------------------------------------------------------
/*N*/ EditUndoInsertFeature::EditUndoInsertFeature( ImpEditEngine* pInImpEE, const EPaM& rEPaM, const SfxPoolItem& rFeature)
/*N*/                   : EditUndo( EDITUNDO_INSERTFEATURE, pInImpEE ), aEPaM( rEPaM )
/*N*/ {
/*N*/   pFeature = rFeature.Clone();
/*N*/   DBG_ASSERT( pFeature, "Feature konnte nicht dupliziert werden: EditUndoInsertFeature" );
/*N*/ }

/*N*/ EditUndoInsertFeature::~EditUndoInsertFeature()
/*N*/ {
/*N*/   delete pFeature;
/*N*/ }

// -----------------------------------------------------------------------
// EditUndoSetStyleSheet
// ------------------------------------------------------------------------
/*N*/ EditUndoSetStyleSheet::EditUndoSetStyleSheet( ImpEditEngine* pInImpEE, USHORT nP,
/*N*/                       const XubString& rPrevName, SfxStyleFamily ePrevFam,
/*N*/                       const XubString& rNewName, SfxStyleFamily eNewFam,
/*N*/                       const SfxItemSet& rPrevParaAttribs )
/*N*/   : EditUndo( EDITUNDO_STYLESHEET, pInImpEE ), aPrevName( rPrevName ), aNewName( rNewName ),
/*N*/     aPrevParaAttribs( rPrevParaAttribs )
/*N*/ {
/*N*/   ePrevFamily = ePrevFam;
/*N*/   eNewFamily = eNewFam;
/*N*/   nPara = nP;
/*N*/ }

// -----------------------------------------------------------------------
// EditUndoSetParaAttribs
// ------------------------------------------------------------------------
/*N*/ EditUndoSetParaAttribs::EditUndoSetParaAttribs( ImpEditEngine* pInImpEE, USHORT nP, const SfxItemSet& rPrevItems, const SfxItemSet& rNewItems )
/*N*/   : EditUndo( EDITUNDO_PARAATTRIBS, pInImpEE ),
/*N*/     aPrevItems( rPrevItems ),
/*N*/     aNewItems(rNewItems )
/*N*/ {
/*N*/   nPara = nP;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
