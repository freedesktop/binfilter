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

// INCLUDE ---------------------------------------------------------------

#include <bf_sfx2/app.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/waitobj.hxx>


#include "dbdocfun.hxx"
#include "bf_sc.hrc"
#include "dbcolect.hxx"
#include "undodat.hxx"
#include "docsh.hxx"
#include "docfunc.hxx"
#include "globstr.hrc"
#include "patattr.hxx"
#include "rangenam.hxx"
#include "dpobject.hxx"
#include "editable.hxx"
namespace binfilter {


/*N*/ BOOL ScDBDocFunc::AddDBRange( const String& rName, const ScRange& rRange )
/*N*/ {
/*N*/
/*N*/   ScDocShellModificator aModificator( rDocShell );
/*N*/
/*N*/   ScDocument* pDoc = rDocShell.GetDocument();
/*N*/   ScDBCollection* pDocColl = pDoc->GetDBCollection();
/*N*/
/*N*/   ScDBData* pNew = new ScDBData( rName, rRange.aStart.Tab(),
/*N*/                                   rRange.aStart.Col(), rRange.aStart.Row(),
/*N*/                                   rRange.aEnd.Col(), rRange.aEnd.Row() );
/*N*/
/*N*/   pDoc->CompileDBFormula( TRUE );     // CreateFormulaString
/*N*/   BOOL bOk = pDocColl->Insert( pNew );
/*N*/   pDoc->CompileDBFormula( FALSE );    // CompileFormulaString
/*N*/   if (!bOk)
/*N*/   {
/*N*/       delete pNew;
/*N*/       return FALSE;
/*N*/   }
/*N*/
/*N*/
/*N*/   aModificator.SetDocumentModified();
/*N*/   SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_DBAREAS_CHANGED ) );
/*N*/   return TRUE;
/*N*/ }
/*N*/
/*N*/ BOOL ScDBDocFunc::DeleteDBRange( const String& rName )
/*N*/ {
/*N*/   BOOL bDone = FALSE;
/*N*/   ScDocument* pDoc = rDocShell.GetDocument();
/*N*/   ScDBCollection* pDocColl = pDoc->GetDBCollection();
/*N*/
/*N*/   USHORT nPos = 0;
/*N*/   if (pDocColl->SearchName( rName, nPos ))
/*N*/   {
/*N*/       ScDocShellModificator aModificator( rDocShell );
/*N*/
/*N*/       pDoc->CompileDBFormula( TRUE );     // CreateFormulaString
/*N*/       pDocColl->AtFree( nPos );
/*N*/       pDoc->CompileDBFormula( FALSE );    // CompileFormulaString
/*N*/
/*N*/       aModificator.SetDocumentModified();
/*N*/       SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_DBAREAS_CHANGED ) );
/*N*/       bDone = TRUE;
/*N*/   }
/*N*/
/*N*/   return bDone;
/*N*/ }

/*N*/ BOOL ScDBDocFunc::RenameDBRange( const String& rOld, const String& rNew )
/*N*/ {
/*N*/   BOOL bDone = FALSE;
/*N*/   ScDocument* pDoc = rDocShell.GetDocument();
/*N*/   ScDBCollection* pDocColl = pDoc->GetDBCollection();
/*N*/
/*N*/   USHORT nPos = 0;
/*N*/   USHORT nDummy = 0;
/*N*/   if ( pDocColl->SearchName( rOld, nPos ) &&
/*N*/        !pDocColl->SearchName( rNew, nDummy ) )
/*N*/   {
/*N*/       ScDocShellModificator aModificator( rDocShell );
/*N*/
/*N*/       ScDBData* pData = (*pDocColl)[nPos];
/*N*/       ScDBData* pNewData = new ScDBData(*pData);
/*N*/       pNewData->SetName(rNew);
/*N*/
/*N*/       ScDBCollection* pUndoColl = new ScDBCollection( *pDocColl );
/*N*/
/*N*/       pDoc->CompileDBFormula( TRUE );             // CreateFormulaString
/*N*/       pDocColl->AtFree( nPos );
/*N*/       BOOL bInserted = pDocColl->Insert( pNewData );
/*N*/       pDoc->CompileDBFormula( FALSE );            // CompileFormulaString
/*N*/
/*N*/       if (bInserted)                              // Einfuegen hat geklappt
/*N*/       {
/*N*/               delete pUndoColl;
/*N*/
/*N*/           aModificator.SetDocumentModified();
/*N*/           SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_DBAREAS_CHANGED ) );
/*N*/           bDone = TRUE;
/*N*/       }
/*N*/   }
/*N*/
/*N*/   return bDone;
/*N*/ }

/*N*/ BOOL ScDBDocFunc::ModifyDBData( const ScDBData& rNewData )
/*N*/ {
/*N*/   BOOL bDone = FALSE;
/*N*/   ScDocument* pDoc = rDocShell.GetDocument();
/*N*/   ScDBCollection* pDocColl = pDoc->GetDBCollection();
/*N*/
/*N*/   USHORT nPos = 0;
/*N*/   if (pDocColl->SearchName( rNewData.GetName(), nPos ))
/*N*/   {
/*N*/       ScDocShellModificator aModificator( rDocShell );
/*N*/
/*N*/       ScDBData* pData = (*pDocColl)[nPos];
/*N*/
/*N*/       ScRange aOldRange, aNewRange;
/*N*/       pData->GetArea(aOldRange);
/*N*/       rNewData.GetArea(aNewRange);
/*N*/
/*N*/       *pData = rNewData;
/*N*/
/*N*/       aModificator.SetDocumentModified();
/*N*/       bDone = TRUE;
/*N*/   }
/*N*/
/*N*/   return bDone;
/*N*/ }


/*N*/ BOOL ScDBDocFunc::RepeatDB( const String& rDBName, BOOL bRecord )
/*N*/ {
/*N*/   //! auch fuer ScDBFunc::RepeatDB benutzen!
/*N*/
/*N*/   BOOL bDone = FALSE;
/*N*/   ScDocument* pDoc = rDocShell.GetDocument();
/*N*/   if (bRecord && !pDoc->IsUndoEnabled())
/*N*/       bRecord = FALSE;
/*N*/   ScDBCollection* pColl = pDoc->GetDBCollection();
/*N*/   USHORT nIndex;
/*N*/   if ( pColl && pColl->SearchName( rDBName, nIndex ) )
/*N*/   {
/*N*/       ScDBData* pDBData = (*pColl)[nIndex];
/*N*/
/*N*/       ScQueryParam aQueryParam;
/*N*/       pDBData->GetQueryParam( aQueryParam );
/*N*/       BOOL bQuery = aQueryParam.GetEntry(0).bDoQuery;
/*N*/
/*N*/       ScSortParam aSortParam;
/*N*/       pDBData->GetSortParam( aSortParam );
/*N*/       BOOL bSort = aSortParam.bDoSort[0];
/*N*/
/*N*/       ScSubTotalParam aSubTotalParam;
/*N*/       pDBData->GetSubTotalParam( aSubTotalParam );
/*N*/       BOOL bSubTotal = aSubTotalParam.bGroupActive[0] && !aSubTotalParam.bRemoveOnly;
/*N*/
/*N*/       if ( bQuery || bSort || bSubTotal )
/*N*/       {
/*N*/           ScRange aOldQuery;
/*N*/           ScRange aNewQuery;
/*N*/           if (bQuery && !aQueryParam.bInplace)
/*N*/           {
/*N*/               ScDBData* pDest = pDoc->GetDBAtCursor( aQueryParam.nDestCol, aQueryParam.nDestRow,
/*N*/                                                       aQueryParam.nDestTab, TRUE );
/*N*/               if (pDest && pDest->IsDoSize())
/*N*/                   pDest->GetArea( aOldQuery );
/*N*/           }
/*N*/
/*N*/           USHORT nTab;
/*N*/           USHORT nStartCol;
/*N*/           USHORT nStartRow;
/*N*/           USHORT nEndCol;
/*N*/           USHORT nEndRow;
/*N*/           pDBData->GetArea( nTab, nStartCol, nStartRow, nEndCol, nEndRow );
/*N*/
/*N*/           ScDocument* pUndoDoc = NULL;
/*N*/
/*N*/           if (bRecord)
/*N*/           {
/*N*/               USHORT nTabCount = pDoc->GetTableCount();
/*N*/               pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
/*N*/               ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab );
/*N*/               if (!pTable)
/*N*/                   pUndoDoc->InitUndo( pDoc, nTab, nTab, FALSE, TRUE );
/*N*/
/*N*/               //  Datenbereich sichern - incl. Filter-Ergebnis
/*N*/               pDoc->CopyToDocument( 0,nStartRow,nTab, MAXCOL,nEndRow,nTab, IDF_ALL, FALSE, pUndoDoc );
/*N*/
/*N*/               //  alle Formeln wegen Referenzen
/*N*/               pDoc->CopyToDocument( 0,0,0, MAXCOL,MAXROW,nTabCount-1, IDF_FORMULA, FALSE, pUndoDoc );
/*N*/           }
/*N*/
/*N*/           rDocShell.PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab,
/*N*/                                   PAINT_GRID | PAINT_LEFT | PAINT_TOP | PAINT_SIZE );
/*N*/           bDone = TRUE;
/*N*/       }
/*N*/   }
/*N*/
/*N*/   return bDone;
/*N*/ }


/*N*/ BOOL ScDBDocFunc::DataPilotUpdate( ScDPObject* pOldObj, const ScDPObject* pNewObj,
/*N*/                                       BOOL bRecord, BOOL )
/*N*/ {
/*N*/   ScDocShellModificator aModificator( rDocShell );
/*N*/   WaitObject aWait( rDocShell.GetDialogParent() );
/*N*/
/*N*/   BOOL bDone = FALSE;
/*N*/   USHORT nErrId = 0;
/*N*/
/*N*/   ScDocument* pOldUndoDoc = NULL;
/*N*/   ScDocument* pNewUndoDoc = NULL;
/*N*/   ScDPObject* pUndoDPObj = NULL;
/*N*/   if ( bRecord && pOldObj )
/*N*/       pUndoDPObj = new ScDPObject( *pOldObj );    // copy old settings for undo
/*N*/
/*N*/   ScDocument* pDoc = rDocShell.GetDocument();
/*N*/   if (bRecord && !pDoc->IsUndoEnabled())
/*N*/       bRecord = FALSE;
/*N*/   if ( !rDocShell.IsEditable() || pDoc->GetChangeTrack() )
/*N*/   {
/*N*/       //  not recorded -> disallow
/*N*/       //! different error messages?
/*N*/
/*N*/       nErrId = STR_PROTECTIONERR;
/*N*/   }
/*N*/   if ( pOldObj && !nErrId )
/*N*/   {
/*N*/       ScRange aOldOut = pOldObj->GetOutRange();
/*N*/       ScEditableTester aTester( pDoc, aOldOut );
/*N*/       if ( !aTester.IsEditable() )
/*N*/           nErrId = aTester.GetMessageId();
/*N*/   }
/*N*/   if ( pNewObj && !nErrId )
/*N*/   {
/*N*/       //  at least one cell at the output position must be editable
/*N*/       //  -> check in advance
/*N*/       //  (start of output range in pNewObj is valid)
/*N*/
/*N*/       ScRange aNewStart( pNewObj->GetOutRange().aStart );
/*N*/       ScEditableTester aTester( pDoc, aNewStart );
/*N*/       if ( !aTester.IsEditable() )
/*N*/           nErrId = aTester.GetMessageId();
/*N*/   }
/*N*/
/*N*/   ScDPObject* pDestObj = NULL;
/*N*/   if ( !nErrId )
/*N*/   {
/*N*/       if ( pOldObj && !pNewObj )
/*N*/       {
/*N*/           //  delete table
/*N*/
/*N*/           ScRange aRange = pOldObj->GetOutRange();
/*N*/           USHORT nTab = aRange.aStart.Tab();
/*N*/
/*N*/           if ( bRecord )
/*N*/           {
/*N*/               pOldUndoDoc = new ScDocument( SCDOCMODE_UNDO );
/*N*/               pOldUndoDoc->InitUndo( pDoc, nTab, nTab );
/*N*/               pDoc->CopyToDocument( aRange, IDF_ALL, FALSE, pOldUndoDoc );
/*N*/           }
/*N*/
/*N*/           pDoc->DeleteAreaTab( aRange.aStart.Col(), aRange.aStart.Row(),
/*N*/                                aRange.aEnd.Col(),   aRange.aEnd.Row(),
/*N*/                                nTab, IDF_ALL );
/*N*/
/*N*/           pDoc->GetDPCollection()->Free( pOldObj );   // object is deleted here
/*N*/
/*N*/           rDocShell.PostPaintGridAll();   //! only necessary parts
/*N*/           rDocShell.PostPaint( aRange.aStart.Col(), aRange.aStart.Row(), nTab,
/*N*/                                aRange.aEnd.Col(),   aRange.aEnd.Row(),   nTab,
/*N*/                                PAINT_GRID );
/*N*/           bDone = TRUE;
/*N*/       }
/*N*/       else if ( pNewObj )
/*N*/       {
/*N*/           if ( pOldObj )
/*N*/           {
/*N*/               if ( bRecord )
/*N*/               {
/*N*/                   ScRange aRange = pOldObj->GetOutRange();
/*N*/                   USHORT nTab = aRange.aStart.Tab();
/*N*/                   pOldUndoDoc = new ScDocument( SCDOCMODE_UNDO );
/*N*/                   pOldUndoDoc->InitUndo( pDoc, nTab, nTab );
/*N*/                   pDoc->CopyToDocument( aRange, IDF_ALL, FALSE, pOldUndoDoc );
/*N*/               }
/*N*/
/*N*/               pDestObj = pOldObj;
/*N*/           }
/*N*/           else
/*N*/           {
/*N*/               //  output range must be set at pNewObj
/*N*/
/*N*/               pDestObj = new ScDPObject( *pNewObj );
/*N*/               pDestObj->SetAlive(TRUE);
/*N*/               if ( !pDoc->GetDPCollection()->Insert(pDestObj) )
/*N*/               {
/*N*/                   OSL_FAIL("cannot insert DPObject");
/*N*/                   DELETEZ( pDestObj );
/*N*/               }
/*N*/           }
/*N*/       }
/*N*/       // else nothing (no old, no new)
/*N*/   }
/*N*/
/*N*/   delete pOldUndoDoc;     // if not used for undo
/*N*/   delete pNewUndoDoc;
/*N*/   delete pUndoDPObj;
/*N*/
/*N*/   if (bDone)
/*N*/       aModificator.SetDocumentModified();
/*N*/
/*N*/   return bDone;
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
