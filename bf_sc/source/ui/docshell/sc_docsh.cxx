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

#include "scitems.hxx"

#define ITEMID_FIELD EE_FEATURE_FIELD

#include <bf_svtools/ctrltool.hxx>
#include <bf_svtools/sfxecode.hxx>
#include <bf_sfx2/docfile.hxx>
#include <bf_sfx2/docfilt.hxx>
#include <bf_sfx2/sfx.hrc>
#include <bf_offmgr/app.hxx>
#include <comphelper/classids.hxx>
#include <bf_sfx2/request.hxx>

#include <com/sun/star/document/UpdateDocMode.hpp>

#include <sot/formats.hxx>
#define SOT_FORMATSTR_ID_STARCALC_30 SOT_FORMATSTR_ID_STARCALC

// INCLUDE ---------------------------------------------------------------

#include "viewdata.hxx"
#include "filter.hxx"
#include "scmod.hxx"
#include "docfunc.hxx"
#include "scresid.hxx"
#include "bf_sc.hrc"
#include "globstr.hrc"
#include "scerrors.hxx"
#include "stlpool.hxx"
#include "autostyl.hxx"
#include "docpool.hxx"      // LoadCompleted
#include "progress.hxx"
#include "pntlock.hxx"
#include "docuno.hxx"
#include "appoptio.hxx"
#include "detdata.hxx"
#include "printfun.hxx"
#include "chartlis.hxx"
#include "hints.hxx"
#include "drwlayer.hxx"
#include "dbcolect.hxx"

#include "docsh.hxx"
#include "ViewSettingsSequenceDefines.hxx"

#include <rtl/logfile.hxx>
#include <vcl/virdev.hxx>
namespace binfilter {
using namespace ::com::sun::star;

#ifndef SO2_DECL_SVSTORAGESTREAM_DEFINED
#define SO2_DECL_SVSTORAGESTREAM_DEFINED
SO2_DECL_REF(SvStorageStream)
#endif

// STATIC DATA -----------------------------------------------------------

//  Stream-Namen im Storage

const sal_Char ScDocShell::pStarCalcDoc[] = STRING_SCSTREAM;        // "StarCalcDocument"
const sal_Char ScDocShell::pStyleName[] = "SfxStyleSheets";

//  Filter-Namen (wie in sclib.cxx)

static const sal_Char pFilterSc50[]     = "StarCalc 5.0";
//static const sal_Char pFilterSc50Temp[]   = "StarCalc 5.0 Vorlage/Template";
static const sal_Char pFilterSc40[]     = "StarCalc 4.0";
//static const sal_Char pFilterSc40Temp[]   = "StarCalc 4.0 Vorlage/Template";
static const sal_Char pFilterSc30[]     = "StarCalc 3.0";
//static const sal_Char pFilterSc30Temp[]   = "StarCalc 3.0 Vorlage/Template";
static const sal_Char pFilterSc10[]     = "StarCalc 1.0";
static const sal_Char pFilterXML[]      = "StarOffice XML (Calc)";
static const sal_Char pFilterAscii[]        = "Text - txt - csv (StarCalc)";
static const sal_Char pFilterLotus[]        = "Lotus";
static const sal_Char pFilterExcel4[]   = "MS Excel 4.0";
static const sal_Char pFilterEx4Temp[]  = "MS Excel 4.0 Vorlage/Template";
static const sal_Char pFilterExcel5[]   = "MS Excel 5.0/95";
static const sal_Char pFilterEx5Temp[]  = "MS Excel 5.0/95 Vorlage/Template";
static const sal_Char pFilterExcel95[]  = "MS Excel 95";
static const sal_Char pFilterEx95Temp[] = "MS Excel 95 Vorlage/Template";
static const sal_Char pFilterExcel97[]  = "MS Excel 97";
static const sal_Char pFilterEx97Temp[] = "MS Excel 97 Vorlage/Template";
static const sal_Char pFilterDBase[]        = "dBase";
static const sal_Char pFilterDif[]      = "DIF";
static const sal_Char pFilterSylk[]     = "SYLK";
static const sal_Char pFilterHtml[]     = "HTML (StarCalc)";
static const sal_Char pFilterHtmlWebQ[] = "calc_HTML_WebQuery";
static const sal_Char pFilterRtf[]      = "Rich Text Format (StarCalc)";

//----------------------------------------------------------------------

//  GlobalName der aktuellen Version:
/*N*/ SFX_IMPL_OBJECTFACTORY_DLL(ScDocShell, SFXOBJECTSHELL_STD_NORMAL,
/*N*/                           SvGlobalName(BF_SO3_SC_CLASSID), Sc)

/*N*/ TYPEINIT1( ScDocShell, SfxObjectShell );      // SfxInPlaceObject: kein Type-Info ?

//------------------------------------------------------------------

/*N*/ void ScDocShell::FillClass( SvGlobalName* pClassName,
/*N*/                                       ULONG* pFormat,
/*N*/                                       String* pAppName,
/*N*/                                       String* pFullTypeName,
/*N*/                                       String* pShortTypeName,
/*N*/                                       long nFileFormat ) const
/*N*/ {
/*N*/   SfxInPlaceObject::FillClass( pClassName, pFormat, pAppName,
/*N*/                               pFullTypeName, pShortTypeName, nFileFormat );
/*N*/
/*N*/   if ( nFileFormat == SOFFICE_FILEFORMAT_31 )
/*N*/   {
/*N*/       *pClassName     = SvGlobalName( BF_SO3_SC_CLASSID_30 );
/*N*/       *pFormat        = SOT_FORMATSTR_ID_STARCALC_30;
/*N*/       *pAppName       = ScResId( SCSTR_30_APPLICATION ).toString();
/*N*/       *pFullTypeName  = ScResId( SCSTR_30_LONG_DOCNAME ).toString();
/*N*/       *pShortTypeName = ScResId( SCSTR_SHORT_SCDOC_NAME ).toString();
/*N*/   }
/*N*/   else if ( nFileFormat == SOFFICE_FILEFORMAT_40 )
/*N*/   {
/*N*/       *pClassName     = SvGlobalName( BF_SO3_SC_CLASSID_40 );
/*N*/       *pFormat        = SOT_FORMATSTR_ID_STARCALC_40;
/*N*/       *pAppName       = ScResId( SCSTR_40_APPLICATION ).toString();
/*N*/       *pFullTypeName  = ScResId( SCSTR_40_LONG_DOCNAME ).toString();
/*N*/       *pShortTypeName = ScResId( SCSTR_SHORT_SCDOC_NAME ).toString();
/*N*/   }
/*N*/   else if ( nFileFormat == SOFFICE_FILEFORMAT_50 )
/*N*/   {
/*N*/       *pClassName     = SvGlobalName( BF_SO3_SC_CLASSID_50 );
/*N*/       *pFormat        = SOT_FORMATSTR_ID_STARCALC_50;
/*N*/       *pAppName       = ScResId( SCSTR_50_APPLICATION ).toString();
/*N*/       *pFullTypeName  = ScResId( SCSTR_50_LONG_DOCNAME ).toString();
/*N*/       *pShortTypeName = ScResId( SCSTR_SHORT_SCDOC_NAME ).toString();
/*N*/   }
/*N*/   else if ( nFileFormat == SOFFICE_FILEFORMAT_60 ||
                  nFileFormat == SOFFICE_FILEFORMAT_CURRENT )   // #i41083# also allow CURRENT
/*N*/   {
/*N*/       // for binfilter, we need the FormatIDs to be set. Not setting them
            // has always been an error (!)
/*N*/       *pClassName     = SvGlobalName( BF_SO3_SC_CLASSID_60 );
/*N*/       *pFormat        = SOT_FORMATSTR_ID_STARCALC_60;
/*N*/       *pAppName       = ScResId( SCSTR_APPLICATION ).toString();
/*N*/
/*N*/       *pFullTypeName  = ScResId(SCSTR_LONG_SCDOC_NAME).toString();
/*N*/       *pShortTypeName = ScResId(SCSTR_SHORT_SCDOC_NAME).toString();

/*N*/   }
/*N*/   else
/*N*/       OSL_FAIL("wat fuer ne Version?");
/*N*/ }


/*N*/ BOOL ScDocShell::LoadCalc( SvStorage* pStor )     // StarCalc 3, 4 or 5 file
/*N*/ {
/*N*/   //  MacroCallMode is no longer needed, state is kept in SfxObjectShell now
/*N*/
/*N*/   BOOL bRet = TRUE;
/*N*/
/*N*/   SvStorageStreamRef aPoolStm = pStor->OpenStream( String::CreateFromAscii(pStyleName), STREAM_STD_READ );
/*N*/   SvStorageStreamRef aDocStm  = pStor->OpenStream( String::CreateFromAscii(pStarCalcDoc), STREAM_STD_READ );
/*N*/   ULONG nPoolErr = aPoolStm->GetError();
/*N*/   ULONG nDocErr  = aDocStm->GetError();
/*N*/
/*N*/   ScProgress* pProgress = NULL;
/*N*/   SfxObjectCreateMode eShellMode = GetCreateMode();
/*N*/   if ( eShellMode == SFX_CREATE_MODE_STANDARD && !nDocErr )
/*N*/   {
/*N*/       ULONG nCurPos = aDocStm->Tell();
/*N*/       ULONG nEndPos = aDocStm->Seek( STREAM_SEEK_TO_END );
/*N*/       aDocStm->Seek( nCurPos );
/*N*/       ULONG nRange = nEndPos - nCurPos;
/*N*/       pProgress = new ScProgress( this, ScGlobal::GetRscString(STR_LOAD_DOC), nRange );
/*N*/   }
/*N*/
/*N*/   if ( nPoolErr == ERRCODE_IO_NOTEXISTS && nDocErr == ERRCODE_IO_NOTEXISTS )
/*N*/   {
/*?*/       //  leerer Storage = leeres Dokument -> wie InitNew
/*?*/
/*?*/       aDocument.MakeTable(0);
/*?*/       Size aSize( (long) ( STD_COL_WIDTH           * HMM_PER_TWIPS * OLE_STD_CELLS_X ),
/*?*/                   (long) ( ScGlobal::nStdRowHeight * HMM_PER_TWIPS * OLE_STD_CELLS_Y ) );
/*?*/       // hier muss auch der Start angepasst werden
/*?*/       SetVisAreaOrSize( Rectangle( Point(), aSize ), TRUE );
/*?*/       aDocument.GetStyleSheetPool()->CreateStandardStyles();
/*?*/       aDocument.UpdStlShtPtrsFrmNms();
/*N*/   }
/*N*/   else if ( !nPoolErr && !nDocErr )
/*N*/   {
/*N*/       aPoolStm->SetVersion(pStor->GetVersion());
/*N*/       aDocStm->SetVersion(pStor->GetVersion());
/*N*/
/*N*/       aDocument.Clear();          // keine Referenzen auf Pool behalten!
/*N*/
/*N*/       RemoveItem( SID_ATTR_CHAR_FONTLIST );
/*N*/       RemoveItem( ITEMID_COLOR_TABLE );
/*N*/       RemoveItem( ITEMID_GRADIENT_LIST );
/*N*/       RemoveItem( ITEMID_HATCH_LIST );
/*N*/       RemoveItem( ITEMID_BITMAP_LIST );
/*N*/       RemoveItem( ITEMID_DASH_LIST );
/*N*/       RemoveItem( ITEMID_LINEEND_LIST );
/*N*/
/*N*/       aDocument.LoadPool( *aPoolStm, FALSE );     // FALSE: RefCounts nicht laden
/*N*/       bRet = (aPoolStm->GetError() == 0);
/*N*/
/*N*/       if ( !bRet )
/*N*/       {
/*N*/           pStor->SetError( aPoolStm->GetError() );
/*N*/           OSL_FAIL( "Fehler im Pool-Stream" );
/*N*/       }
/*N*/       else if (eShellMode != SFX_CREATE_MODE_ORGANIZER)
/*N*/       {
/*N*/           //  ViewOptions are not completely stored (to avoid warnings),
/*N*/           //  so must be initialized from global settings.
/*N*/           //! This can be removed if a new file format loads all ViewOptions !!!
/*N*/
/*N*/           aDocument.SetViewOptions( SC_MOD()->GetViewOptions() );
/*N*/
/*N*/           ULONG nErrCode = aPoolStm->GetErrorCode();
/*N*/           const ByteString aStrKey = pStor->GetKey();
/*N*/           const BOOL   bKeySet = (aStrKey.Len() > 0);
/*N*/
/*N*/           if ( bKeySet )
/*?*/               aDocStm->SetCryptMaskKey(aStrKey);          // Passwort setzen
/*N*/
/*N*/           bRet = aDocument.Load( *aDocStm, pProgress );
/*N*/
/*N*/           if ( !bRet && bKeySet ) // Passwort falsch
/*N*/           {
/*?*/               SetError( ERRCODE_SFX_WRONGPASSWORD );
/*?*/               pStor->SetError( ERRCODE_SFX_WRONGPASSWORD );
/*N*/           }
/*N*/
/*N*/           if ( aDocStm->GetError() )
/*N*/           {
/*?*/               //  Zeile-8192-Meldung braucht nur zu kommen, wenn die Zeilen
/*?*/               //  beschraenkt sind, sonst ist "falsches Format" besser
/*?*/ #ifdef SC_LIMIT_ROWS
/*?*/               if ( aDocument.HasLostData() )                  // zuviele Zeilen?
/*?*/                   pStor->SetError( SCERR_IMPORT_8K_LIMIT );
/*?*/               else
/*?*/ #endif
/*?*/                   pStor->SetError( aDocStm->GetError() );
/*?*/               bRet = FALSE;
/*?*/               OSL_FAIL( "Fehler im Document-Stream" );
/*N*/           }
/*N*/           else
/*N*/           {
/*N*/               if ( !(nErrCode & ERRCODE_WARNING_MASK) )
/*N*/                   nErrCode = aDocStm->GetErrorCode();
/*N*/               if ( nErrCode & ERRCODE_WARNING_MASK )
/*?*/                   SetError( nErrCode );
/*N*/           }
/*N*/
/*N*/           if (bRet)
/*N*/           {
/*N*/               aDocument.UpdateFontCharSet();
/*N*/
/*N*/               UpdateLinks();              // verknuepfte Tabellen in Link-Manager
/*N*/               RemoveUnknownObjects();     // unbekannte Ole-Objekte loeschen

                    // #116578# Manually load view state from sfx window data (no view is created).
                    // See SfxObjectShell::LoadWindows_Impl.
                    // Only the user data string is used, only from the first view.

                    SvStorageStreamRef aWinStm  = pStor->OpenStream(
                            String::CreateFromAscii("SfxWindows"), STREAM_STD_READ );
                    if ( aWinStm.Is() && aWinStm->GetError() == ERRCODE_NONE )
                    {
                        aWinStm->SetBufferSize(1024);
                        String aWinData = aWinStm->ReadUniOrByteString( aWinStm->GetStreamCharSet() );
                        if ( aWinData.Len() )
                        {
                            char cToken =',';
                            BOOL bOldFormat = TRUE;
                            if ( aWinData.GetToken( 0, cToken ).EqualsAscii( "TASK" ) )
                            {
                                bOldFormat = FALSE;
                                aWinData = aWinStm->ReadUniOrByteString( aWinStm->GetStreamCharSet() );    // read next string
                            }
                            USHORT nViewId = (USHORT) aWinData.GetToken( 0, cToken ).ToInt32();

                            // only ScTabViewShell is handled (ID as in ScDLL::Init)
                            if ( nViewId == 1 )
                            {
                                if ( bOldFormat )
                                {
                                    aUserData = aWinData.GetToken( 2, cToken );
                                }
                                else
                                {
                                    USHORT nPos=0;
                                    aWinData.GetToken( 3, cToken, nPos ).ToInt32();
                                    aUserData = aWinData.Copy( nPos );
                                }
                                // aUserData is used in ScModelObj::getViewData
                            }
                        }
                    }
/*N*/           }
/*N*/       }
/*N*/       else            // SFX_CREATE_MODE_ORGANIZER
/*N*/       {
/*?*/           //  UpdateStdNames is called from ScDocument::Load, but is also needed
/*?*/           //  if only the styles are loaded!
/*?*/           ScStyleSheetPool* pStylePool = aDocument.GetStyleSheetPool();
/*?*/           if (pStylePool)
/*?*/               pStylePool->UpdateStdNames();   // correct style names for different languages
/*N*/       }
/*N*/
/*N*/       if (bRet)
/*N*/           aDocument.GetPool()->MyLoadCompleted();
/*N*/   }
/*N*/   else
/*N*/   {
/*?*/       OSL_FAIL( "Stream-Fehler");
/*?*/       bRet = FALSE;
/*N*/   }
/*N*/
/*N*/   if (!aDocument.HasTable(0))
/*?*/       aDocument.MakeTable(0);
/*N*/
/*N*/   if (eShellMode == SFX_CREATE_MODE_EMBEDDED)
/*N*/   {
/*N*/       USHORT nVisTab = aDocument.GetVisibleTab();
/*N*/       BOOL bHasVis = aDocument.HasTable(nVisTab);
/*N*/       if ( SfxInPlaceObject::GetVisArea().IsEmpty() || !bHasVis )
/*N*/       {
/*?*/           if (!bHasVis)
/*?*/           {
/*?*/               nVisTab = 0;
/*?*/               aDocument.SetVisibleTab(nVisTab);
/*?*/           }
/*?*/           USHORT nStartCol,nStartRow;
/*?*/           aDocument.GetDataStart( nVisTab, nStartCol, nStartRow );
/*?*/           USHORT nEndCol,nEndRow;
/*?*/           aDocument.GetPrintArea( nVisTab, nEndCol, nEndRow );
/*?*/           if (nStartCol>nEndCol)
/*?*/               nStartCol = nEndCol;
/*?*/           if (nStartRow>nEndRow)
/*?*/               nStartRow = nEndRow;
/*?*/           // hier muss auch der Start angepasst werden
/*?*/           SetVisAreaOrSize( aDocument.GetMMRect( nStartCol,nStartRow, nEndCol,nEndRow, nVisTab ),
/*?*/                               TRUE );
/*N*/       }
/*N*/   }
/*N*/
/*N*/   delete pProgress;
/*N*/
/*N*/   return bRet;
/*N*/ }


/*N*/ void ScDocShell::BeforeXMLLoading()
/*N*/ {
/*N*/   // prevent unnecessary broadcasts and updates
/*N*/     DBG_ASSERT(pModificator == NULL, "The Modificator should not exist");
/*N*/   pModificator = new ScDocShellModificator( *this );
/*N*/
/*N*/     aDocument.SetImportingXML( TRUE );
/*N*/   // prevent unnecessary broadcasts and "half way listeners"
/*N*/   aDocument.SetInsertingFromOtherDoc( TRUE );
/*N*/
/*N*/   if (GetCreateMode() != SFX_CREATE_MODE_ORGANIZER)
/*N*/       ScColumn::bDoubleAlloc = sal_True;
/*N*/ }

/*N*/ void ScDocShell::AfterXMLLoading(sal_Bool /*bRet*/)
/*N*/ {
            aDocument.SetInsertingFromOtherDoc( FALSE );
            aDocument.SetImportingXML( FALSE );

            if (pModificator)
            {
                delete pModificator;
                pModificator = NULL;
            }
            else
                OSL_FAIL("The Modificator should exist");
/*N*/ }

/*N*/ BOOL ScDocShell::Load( SvStorage* pStor )
/*N*/ {
/*N*/   RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::Load" );
/*N*/
/*N*/   ScRefreshTimerProtector( aDocument.GetRefreshTimerControlAddress() );
/*N*/
/*N*/   DBG_ASSERT( pStor, "Load without storage?" );
/*N*/   BOOL bXML = ( pStor->GetVersion() >= SOFFICE_FILEFORMAT_60 );
/*N*/
/*N*/   //  only the latin script language is loaded
/*N*/   //  -> initialize the others from options (before loading)
/*N*/   InitOptions();
/*N*/
/*N*/   BOOL bRet = SfxInPlaceObject::Load( pStor );
/*N*/   if( bRet )
/*N*/   {
/*N*/         if (GetMedium())
/*N*/         {
/*N*/             SFX_ITEMSET_ARG( GetMedium()->GetItemSet(), pUpdateDocItem, SfxUInt16Item, SID_UPDATEDOCMODE, sal_False);
/*N*/             nCanUpdate = pUpdateDocItem ? pUpdateDocItem->GetValue() : ::com::sun::star::document::UpdateDocMode::NO_UPDATE;
/*N*/         }
/*N*/
/*N*/         if (bXML)
/*N*/       {
                OSL_ASSERT("XML import removed");
/*N*/       }
/*N*/       else
/*N*/           bRet = LoadCalc( pStor );
/*N*/   }
/*N*/
/*N*/   if (!bRet && !pStor->GetError())
/*?*/       pStor->SetError( SVSTREAM_FILEFORMAT_ERROR );
/*N*/
/*N*/   if (pStor->GetError())
/*?*/       SetError( pStor->GetError() );
/*N*/
/*N*/   InitItems();
/*N*/   CalcOutputFactor();
/*N*/
/*N*/   // #73762# invalidate eventually temporary table areas
/*N*/   if ( bRet )
/*N*/       aDocument.InvalidateTableArea();
/*N*/
/*N*/   bIsEmpty = FALSE;
/*N*/   FinishedLoading( SFX_LOADED_MAINDOCUMENT | SFX_LOADED_IMAGES );
/*N*/   return bRet;
/*N*/ }


/*N*/ void ScDocShell::SFX_NOTIFY( SfxBroadcaster& /*rBC*/, const TypeId& rBCType,
/*N*/                        const SfxHint& rHint, const TypeId& rHintType )
/*N*/ {
/*N*/   if (rHint.ISA(SfxSimpleHint))                               // ohne Parameter
/*N*/   {
/*N*/       ULONG nSlot = ((const SfxSimpleHint&)rHint).GetId();
/*N*/       switch ( nSlot )
/*N*/       {
/*N*/           case SFX_HINT_TITLECHANGED:
/*N*/               aDocument.SetName( SfxShell::GetName() );
/*N*/               //  RegisterNewTargetNames gibts nicht mehr
/*N*/               SFX_APP()->Broadcast(SfxSimpleHint( SC_HINT_DOCNAME_CHANGED )); // Navigator
/*N*/               break;
/*N*/       }
/*N*/   }
/*N*/ }

    // Inhalte fuer Organizer laden




/*N*/ BOOL ScDocShell::ConvertFrom( SfxMedium& rMedium )
/*N*/ {
/*N*/   RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::ConvertFrom" );
/*N*/
/*N*/   BOOL bRet = FALSE;              // FALSE heisst Benutzerabbruch !!
/*N*/                                   // bei Fehler: Fehler am Stream setzen!!
/*N*/
/*N*/   ScRefreshTimerProtector( aDocument.GetRefreshTimerControlAddress() );
/*N*/
/*N*/   // ob nach dem Import optimale Spaltenbreiten gesetzt werden sollen
/*N*/   BOOL bSetColWidths = FALSE;
/*N*/   BOOL bSetSimpleTextColWidths = FALSE;
/*N*/   BOOL bSimpleColWidth[MAXCOL+1];
/*N*/   memset( bSimpleColWidth, 1, (MAXCOL+1) * sizeof(BOOL) );
/*N*/   ScRange aColWidthRange;
/*N*/   // ob nach dem Import optimale Zeilenhoehen gesetzt werden sollen
/*N*/   BOOL bSetRowHeights = FALSE;
/*N*/
/*N*/   aConvFilterName.Erase(); //@ #BugId 54198
/*N*/
/*N*/   //  Alle Filter brauchen die komplette Datei am Stueck (nicht asynchron),
/*N*/   //  darum vorher per CreateFileStream dafuer sorgen, dass die komplette
/*N*/   //  Datei uebertragen wird.
/*N*/   rMedium.GetPhysicalName();  //! CreateFileStream direkt rufen, wenn verfuegbar
/*N*/
/*N*/     SFX_ITEMSET_ARG( rMedium.GetItemSet(), pUpdateDocItem, SfxUInt16Item, SID_UPDATEDOCMODE, sal_False);
/*N*/     nCanUpdate = pUpdateDocItem ? pUpdateDocItem->GetValue() : ::com::sun::star::document::UpdateDocMode::NO_UPDATE;
/*N*/
/*N*/     const SfxFilter* pFilter = rMedium.GetFilter();
/*N*/   if (pFilter)
/*N*/   {
/*N*/       String aFltName = pFilter->GetFilterName();
/*N*/
/*N*/       aConvFilterName=aFltName; //@ #BugId 54198
/*N*/
/*N*/       BOOL bCalc3 = ( aFltName.EqualsAscii(pFilterSc30) );
/*N*/       BOOL bCalc4 = ( aFltName.EqualsAscii(pFilterSc40) );
/*N*/       if (!bCalc3 && !bCalc4)
/*N*/           aDocument.SetInsertingFromOtherDoc( TRUE );
/*N*/
/*N*/       if (bCalc3 || bCalc4)                   // Calc3/4 - "Import"
/*N*/       {
/*?*/           //  wait cursor is handled with progress bar
/*?*/           SvStorage* pStor = rMedium.GetStorage();
/*?*/           if ( pStor )
/*?*/           {
/*?*/               bRet = SfxInPlaceObject::Load( pStor );
/*?*/               if( bRet )
/*?*/                   bRet = LoadCalc( pStor );
/*?*/               if (!bRet && !pStor->GetError())
/*?*/                   pStor->SetError( SVSTREAM_FILEFORMAT_ERROR );
/*?*/           }
/*?*/           else
/*?*/               OSL_FAIL("Calc3/4: kein Storage");
/*N*/       }
            else if (
                       aFltName.EqualsAscii(pFilterExcel4)
                    || aFltName.EqualsAscii(pFilterExcel5)
                    || aFltName.EqualsAscii(pFilterExcel95)
                    || aFltName.EqualsAscii(pFilterExcel97)
                    || aFltName.EqualsAscii(pFilterEx4Temp)
                    || aFltName.EqualsAscii(pFilterEx5Temp)
                    || aFltName.EqualsAscii(pFilterEx95Temp)
                    || aFltName.EqualsAscii(pFilterEx97Temp)
                    || aFltName.EqualsAscii(pFilterXML)
                    || aFltName.EqualsAscii(pFilterSc10)
                    || aFltName.EqualsAscii(pFilterLotus)
                    || aFltName.EqualsAscii(pFilterDBase)
                    || aFltName.EqualsAscii(pFilterRtf)
                    || aFltName.EqualsAscii(pFilterHtml)
                    || aFltName.EqualsAscii(pFilterHtmlWebQ)
                    || aFltName.EqualsAscii(pFilterDif)
                    || aFltName.EqualsAscii(pFilterAscii)
                    || aFltName.EqualsAscii(pFilterSylk)
                    )
            {
                // Ignore these filters; Don't let the else generate an error
            }
/*N*/       else
/*N*/       {
/*N*/           if (!GetError())
/*N*/               SetError(SCERR_IMPORT_NI);
/*N*/       }
/*N*/
/*N*/       if (!bCalc3)
/*N*/           aDocument.SetInsertingFromOtherDoc( FALSE );
/*N*/   }
/*N*/   else
/*N*/       OSL_FAIL("Kein Filter bei ConvertFrom");
/*N*/
/*N*/   InitItems();
/*N*/   CalcOutputFactor();
/*N*/   if ( bRet && (bSetColWidths || bSetRowHeights) )
/*N*/   {   // Spaltenbreiten/Zeilenhoehen anpassen, Basis 100% Zoom
/*N*/       Fraction aZoom( 1, 1 );
/*N*/       double nPPTX = ScGlobal::nScreenPPTX * (double) aZoom
/*N*/           / GetOutputFactor();    // Faktor ist Drucker zu Bildschirm
/*N*/       double nPPTY = ScGlobal::nScreenPPTY * (double) aZoom;
/*N*/       VirtualDevice aVirtDev;
/*N*/       //  all sheets (for Excel import)
/*N*/       USHORT nTabCount = aDocument.GetTableCount();
/*N*/       for (USHORT nTab=0; nTab<nTabCount; nTab++)
/*N*/       {
/*N*/           USHORT nEndCol, nEndRow;
/*N*/           aDocument.GetCellArea( nTab, nEndCol, nEndRow );
/*N*/           aColWidthRange.aEnd.SetCol( nEndCol );
/*N*/           aColWidthRange.aEnd.SetRow( nEndRow );
/*N*/           ScMarkData aMark;
/*N*/           aMark.SetMarkArea( aColWidthRange );
/*N*/           aMark.MarkToMulti();
/*N*/           // Reihenfolge erst Breite dann Hoehe ist wichtig (vergl. hund.rtf)
/*N*/           if ( bSetColWidths )
/*N*/           {
/*N*/               for ( USHORT nCol=0; nCol <= nEndCol; nCol++ )
/*N*/               {
/*N*/                   USHORT nWidth = aDocument.GetOptimalColWidth(
/*N*/                       nCol, nTab, &aVirtDev, nPPTX, nPPTY, aZoom, aZoom, FALSE, &aMark,
/*N*/                       (bSetSimpleTextColWidths && bSimpleColWidth[nCol]) );
/*N*/                   aDocument.SetColWidth( nCol, nTab,
/*N*/                       nWidth + (USHORT)ScGlobal::nLastColWidthExtra );
/*N*/               }
/*N*/           }
/*N*/       }
/*N*/   }
/*N*/   FinishedLoading( SFX_LOADED_MAINDOCUMENT | SFX_LOADED_IMAGES );
/*N*/
/*N*/   // #73762# invalidate eventually temporary table areas
/*N*/   if ( bRet )
/*N*/       aDocument.InvalidateTableArea();
/*N*/
/*N*/   bIsEmpty = FALSE;
/*N*/
/*N*/   return bRet;
/*N*/ }


/*N*/ void ScDocShell::HandsOff()
/*N*/ {
/*N*/   ScDrawLayer* pDrawLayer = aDocument.GetDrawLayer();
/*N*/
/*N*/   SfxInPlaceObject::HandsOff();
/*N*/
/*N*/   if( pDrawLayer )
/*N*/       pDrawLayer->ReleasePictureStorage();
/*N*/ }


// Xcl-like column width measured in characters of standard font.


/*N*/ String ScDocShell::GetOwnFilterName()         // static
/*N*/ {
/*N*/   return String::CreateFromAscii(pFilterSc50);
/*N*/ }

/*N*/ String ScDocShell::GetWebQueryFilterName()        // static
/*N*/ {
/*N*/   return String::CreateFromAscii(pFilterHtmlWebQ);
/*N*/ }

/*N*/ String ScDocShell::GetAsciiFilterName()           // static
/*N*/ {
/*N*/   return String::CreateFromAscii(pFilterAscii);
/*N*/ }

/*N*/ String ScDocShell::GetLotusFilterName()           // static
/*N*/ {
/*N*/   return String::CreateFromAscii(pFilterLotus);
/*N*/ }

/*N*/ String ScDocShell::GetDBaseFilterName()           // static
/*N*/ {
/*N*/   return String::CreateFromAscii(pFilterDBase);
/*N*/ }

/*N*/ String ScDocShell::GetDifFilterName()         // static
/*N*/ {
/*N*/   return String::CreateFromAscii(pFilterDif);
/*N*/ }

/*N*/ BOOL ScDocShell::HasAutomaticTableName( const String& rFilter )       // static
/*N*/ {
/*N*/   //  TRUE for those filters that keep the default table name
/*N*/   //  (which is language specific)
/*N*/
/*N*/   return rFilter.EqualsAscii( pFilterAscii )
/*N*/       || rFilter.EqualsAscii( pFilterLotus )
/*N*/       || rFilter.EqualsAscii( pFilterExcel4 )
/*N*/       || rFilter.EqualsAscii( pFilterEx4Temp )
/*N*/       || rFilter.EqualsAscii( pFilterDBase )
/*N*/       || rFilter.EqualsAscii( pFilterDif )
/*N*/       || rFilter.EqualsAscii( pFilterSylk )
/*N*/       || rFilter.EqualsAscii( pFilterHtml )
/*N*/       || rFilter.EqualsAscii( pFilterRtf );
/*N*/ }

//==================================================================

#define __SCDOCSHELL_INIT \
        aDocument       ( SCDOCMODE_DOCUMENT, this ), \
        aDdeTextFmt(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("TEXT"))), \
        nPrtToScreenFactor( 1.0 ), \
        pFontList       ( NULL ), \
        bHeaderOn       ( TRUE ), \
        bFooterOn       ( TRUE ), \
        bNoInformLost( TRUE ), \
        bIsEmpty        ( TRUE ), \
        bIsInUndo       ( FALSE ), \
        bDocumentModifiedPending( FALSE ), \
        nDocumentLock   ( 0 ), \
        nCanUpdate (::com::sun::star::document::UpdateDocMode::ACCORDING_TO_CONFIG), \
        bUpdateEnabled  ( TRUE ), \
        pDocHelper      ( NULL ), \
        pAutoStyleList  ( NULL ), \
        pPaintLockData  ( NULL ), \
        pOldJobSetup    ( NULL ), \
        pVirtualDevice_100th_mm ( NULL ), \
        pModificator    ( NULL )

//------------------------------------------------------------------

/*N*/ ScDocShell::ScDocShell( const ScDocShell& rShell )
/*N*/   : SvRefBase()
/*N*/   , SotObject()
/*N*/   , SvObject()
/*N*/   , SvPersist()
/*N*/   , SfxObjectShell( rShell.GetCreateMode() )
/*N*/   , SfxInPlaceObject()
/*N*/   , SfxListener()
/*N*/   , __SCDOCSHELL_INIT
/*N*/ {
/*N*/ }

//------------------------------------------------------------------

/*N*/ ScDocShell::ScDocShell( SfxObjectCreateMode eMode )
/*N*/   :   SfxObjectShell( eMode ),
/*N*/       __SCDOCSHELL_INIT
/*N*/ {
/*N*/   RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::ScDocShell" );
/*N*/
/*N*/   SetPool( &SC_MOD()->GetPool() );
/*N*/
/*N*/   SetShell(this);
/*N*/   bIsInplace = (eMode == SFX_CREATE_MODE_EMBEDDED);
/*N*/   //  wird zurueckgesetzt, wenn nicht inplace
/*N*/
/*N*/   pDocFunc = new ScDocFunc(*this);
/*N*/
/*N*/   //  SetBaseModel needs exception handling
/*N*/   ScModelObj::CreateAndSet( this );
/*N*/
/*N*/   StartListening(*this);
/*N*/   SfxStyleSheetPool* pStlPool = aDocument.GetStyleSheetPool();
/*N*/   if (pStlPool)
/*N*/       StartListening(*pStlPool);

/*N*/   aDocument.GetDBCollection()->SetRefreshHandler(
/*N*/       LINK( this, ScDocShell, RefreshDBDataHdl ) );
/*N*/
/*N*/   //  InitItems und CalcOutputFactor werden jetzt nach bei Load/ConvertFrom/InitNew gerufen
/*N*/ }

//------------------------------------------------------------------

/*N*/ ScDocShell::~ScDocShell()
/*N*/ {
/*N*/   ResetDrawObjectShell(); // #55570# falls der Drawing-Layer noch versucht, darauf zuzugreifen
/*N*/
/*N*/   SfxStyleSheetPool* pStlPool = aDocument.GetStyleSheetPool();
/*N*/   if (pStlPool)
/*N*/       EndListening(*pStlPool);
/*N*/   EndListening(*this);
/*N*/
/*N*/   delete pAutoStyleList;
/*N*/
/*N*/   delete pDocFunc;
/*N*/   delete pFontList;
/*N*/
/*N*/   delete pPaintLockData;
/*N*/
/*N*/   delete pOldJobSetup;        // gesetzt nur bei Fehler in StartJob()
/*N*/
/*N*/   delete pVirtualDevice_100th_mm;
/*N*/
/*N*/     if (pModificator)
/*N*/     {
/*N*/         OSL_FAIL("The Modificator should not exist");
/*N*/         delete pModificator;
/*N*/     }
/*N*/ }

//------------------------------------------------------------------


/*N*/ void ScDocShell::SetModified( BOOL bModified )
/*N*/ {
/*N*/   SfxInPlaceObject::SetModified( bModified );
/*N*/   Broadcast( SfxSimpleHint( SFX_HINT_DOCCHANGED ) );
/*N*/ }


/*N*/ void ScDocShell::SetDocumentModified( BOOL bInIsModified /* = TRUE */ )
/*N*/ {
/*N*/   //  BroadcastUno muss auch mit pPaintLockData sofort passieren
/*N*/   //! auch bei SetDrawModified, wenn Drawing angebunden ist
/*N*/   //! dann eigener Hint???
/*N*/
/*N*/   if (bInIsModified)
/*N*/       aDocument.BroadcastUno( SfxSimpleHint( SFX_HINT_DATACHANGED ) );
/*N*/
/*N*/   if ( pPaintLockData && bInIsModified )
/*N*/   {
/*N*/       pPaintLockData->SetModified();          // spaeter...
/*N*/       return;
/*N*/   }
/*N*/
/*N*/   SetDrawModified( bInIsModified );
/*N*/
/*N*/   if ( bInIsModified )
/*N*/   {
/*N*/       if ( aDocument.IsAutoCalcShellDisabled() )
/*?*/           SetDocumentModifiedPending( TRUE );
/*N*/       else
/*N*/       {
/*N*/           SetDocumentModifiedPending( FALSE );
/*N*/           aDocument.InvalidateStyleSheetUsage();
/*N*/           aDocument.InvalidateTableArea();
/*N*/           aDocument.InvalidateLastTableOpParams();
/*N*/           aDocument.Broadcast( SC_HINT_DATACHANGED, BCA_BRDCST_ALWAYS, NULL );
/*N*/           PostDataChanged();
/*N*/
/*N*/           //  Detective AutoUpdate:
/*N*/           //  Update if formulas were modified (DetectiveDirty) or the list contains
/*N*/           //  "Trace Error" entries (#75362# - Trace Error can look completely different
/*N*/           //  after changes to non-formula cells).
/*N*/
/*N*/           ScDetOpList* pList = aDocument.GetDetOpList();
/*N*/           if ( pList && ( aDocument.IsDetectiveDirty() || pList->HasAddError() ) &&
/*N*/                pList->Count() && !IsInUndo() && SC_MOD()->GetAppOptions().GetDetectiveAuto() )
/*N*/           {
/*N*/               GetDocFunc().DetectiveRefresh();
/*N*/           }
/*N*/           aDocument.SetDetectiveDirty(FALSE);         // always reset, also if not refreshed
/*N*/       }
/*N*/   }
/*N*/ }

//  SetDrawModified - ohne Formel-Update
//  (Drawing muss auch beim normalen SetDocumentModified upgedated werden,
//   z.B. bei Tabelle loeschen etc.)

/*N*/ void ScDocShell::SetDrawModified( BOOL bInIsModified /* = TRUE */ )
/*N*/ {
/*N*/
/*N*/   SetModified( bInIsModified );
/*N*/
/*N*/   if (bInIsModified)
/*N*/   {
/*N*/       if ( aDocument.IsChartListenerCollectionNeedsUpdate() )
/*N*/       {
/*N*/           aDocument.UpdateChartListenerCollection();
/*N*/           SFX_APP()->Broadcast(SfxSimpleHint( SC_HINT_DRAW_CHANGED ));    // Navigator
/*N*/       }
/*N*/       SC_MOD()->AnythingChanged();
/*N*/   }
/*N*/ }

/*N*/ Window* ScDocShell::GetDialogParent()
/*N*/ {
/*N*/       return Application::GetDefDialogParent();
/*N*/ }

/*N*/ VirtualDevice* ScDocShell::GetVirtualDevice_100th_mm()
/*N*/ {
/*N*/   if (!pVirtualDevice_100th_mm)
/*N*/   {
/*N*/       pVirtualDevice_100th_mm = new VirtualDevice;
/*N*/       pVirtualDevice_100th_mm->SetMapMode( MAP_100TH_MM );
/*N*/   }
/*N*/   return pVirtualDevice_100th_mm;
/*N*/ }

// --- ScDocShellModificator ------------------------------------------

/*N*/ ScDocShellModificator::ScDocShellModificator( ScDocShell& rDS )
/*N*/       :
/*N*/       rDocShell( rDS ),
/*N*/       aProtector( rDS.GetDocument()->GetRefreshTimerControlAddress() )
/*N*/ {
/*N*/   ScDocument* pDoc = rDocShell.GetDocument();
/*N*/   bAutoCalcShellDisabled = pDoc->IsAutoCalcShellDisabled();
/*N*/   bIdleDisabled = pDoc->IsIdleDisabled();
/*N*/   pDoc->SetAutoCalcShellDisabled( TRUE );
/*N*/   pDoc->DisableIdle( TRUE );
/*N*/ }


/*N*/ ScDocShellModificator::~ScDocShellModificator()
/*N*/ {
/*N*/   ScDocument* pDoc = rDocShell.GetDocument();
/*N*/   pDoc->SetAutoCalcShellDisabled( bAutoCalcShellDisabled );
/*N*/   if ( !bAutoCalcShellDisabled && rDocShell.IsDocumentModifiedPending() )
/*?*/       rDocShell.SetDocumentModified();    // last one shuts off the lights
/*N*/   pDoc->DisableIdle( bIdleDisabled );
/*N*/ }


/*N*/ void ScDocShellModificator::SetDocumentModified()
/*N*/ {
/*N*/   ScDocument* pDoc = rDocShell.GetDocument();
/*N*/   if ( !pDoc->IsImportingXML() )
/*N*/   {
/*N*/       // AutoCalcShellDisabled temporaer restaurieren
/*N*/       BOOL bDisabled = pDoc->IsAutoCalcShellDisabled();
/*N*/       pDoc->SetAutoCalcShellDisabled( bAutoCalcShellDisabled );
/*N*/       rDocShell.SetDocumentModified();
/*N*/       pDoc->SetAutoCalcShellDisabled( bDisabled );
/*N*/   }
/*N*/   else
/*N*/   {
/*?*/       // uno broadcast is necessary for api to work
/*?*/       // -> must also be done during xml import
/*?*/       pDoc->BroadcastUno( SfxSimpleHint( SFX_HINT_DATACHANGED ) );
/*N*/   }
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
