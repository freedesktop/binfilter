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

#include "scitems.hxx"

#define ITEMID_FIELD EE_FEATURE_FIELD
#include <bf_svx/editview.hxx>
#include <bf_svx/outliner.hxx>
#include <bf_svx/zoomitem.hxx>
#include <bf_xmloff/xmluconv.hxx>
#include <rtl/ustrbuf.hxx>
#include <legacysmgr/legacy_binfilters_smgr.hxx>

#include "viewdata.hxx"
#include "attrib.hxx"
#include "docsh.hxx"
#include "bf_sc.hrc"
#include "editutil.hxx"
#include "miscuno.hxx"
#include "unonames.hxx"

#include "ViewSettingsSequenceDefines.hxx"

namespace binfilter {
using namespace ::com::sun::star;

// STATIC DATA -----------------------------------------------------------

#define SC_GROWY_SMALL_EXTRA    100
#define SC_GROWY_BIG_EXTRA      200

#define TAG_TABBARWIDTH "tw:"

USHORT nEditAdjust = SVX_ADJUST_LEFT;       //! Member !!!

//==================================================================

/*N*/ ScViewDataTable::ScViewDataTable() :
/*N*/               nHSplitPos( 0 ),
/*N*/               nVSplitPos( 0 ),
/*N*/               eHSplitMode( SC_SPLIT_NONE ),
/*N*/               eVSplitMode( SC_SPLIT_NONE ),
/*N*/               eWhichActive( SC_SPLIT_BOTTOMLEFT ),
/*N*/               nFixPosX( 0 ),
/*N*/               nFixPosY( 0 ),
/*N*/               nCurX( 0 ),
/*N*/               nCurY( 0 ),
/*N*/               bOldCurValid( FALSE )
/*N*/ {
/*N*/   nPosX[0]=nPosX[1]=
/*N*/   nPosY[0]=nPosY[1]=0;
/*N*/   nTPosX[0]=nTPosX[1]=
/*N*/   nTPosY[0]=nTPosY[1]=0;
/*N*/   nMPosX[0]=nMPosX[1]=
/*N*/   nMPosY[0]=nMPosY[1]=0;
/*N*/   nPixPosX[0]=nPixPosX[1]=
/*N*/   nPixPosY[0]=nPixPosY[1]=0;
/*N*/ }


/*N*/ ScViewDataTable::~ScViewDataTable()
/*N*/ {
/*N*/ }

/*N*/ void ScViewDataTable::WriteUserDataSequence(uno::Sequence <beans::PropertyValue>& rSettings)
/*N*/ {
/*N*/   rSettings.realloc(SC_TABLE_VIEWSETTINGS_COUNT);
/*N*/   beans::PropertyValue* pSettings = rSettings.getArray();
/*N*/   if (pSettings)
/*N*/   {
/*N*/       pSettings[SC_CURSOR_X].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_CURSORPOSITIONX));
/*N*/       pSettings[SC_CURSOR_X].Value <<= sal_Int32(nCurX);
/*N*/       pSettings[SC_CURSOR_Y].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_CURSORPOSITIONY));
/*N*/       pSettings[SC_CURSOR_Y].Value <<= sal_Int32(nCurY);
/*N*/       pSettings[SC_HORIZONTAL_SPLIT_MODE].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_HORIZONTALSPLITMODE));
/*N*/       pSettings[SC_HORIZONTAL_SPLIT_MODE].Value <<= sal_Int16(eHSplitMode);
/*N*/       pSettings[SC_VERTICAL_SPLIT_MODE].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_VERTICALSPLITMODE));
/*N*/       pSettings[SC_VERTICAL_SPLIT_MODE].Value <<= sal_Int16(eVSplitMode);
/*N*/       pSettings[SC_HORIZONTAL_SPLIT_POSITION].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_HORIZONTALSPLITPOSITION));
/*N*/       if (eHSplitMode == SC_SPLIT_FIX)
/*N*/           pSettings[SC_HORIZONTAL_SPLIT_POSITION].Value <<= sal_Int32(nFixPosX);
/*N*/       else
/*N*/           pSettings[SC_HORIZONTAL_SPLIT_POSITION].Value <<= nHSplitPos;
/*N*/       pSettings[SC_VERTICAL_SPLIT_POSITION].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_VERTICALSPLITPOSITION));
/*N*/       if (eVSplitMode == SC_SPLIT_FIX)
/*N*/           pSettings[SC_VERTICAL_SPLIT_POSITION].Value <<= sal_Int32(nFixPosY);
/*N*/       else
/*N*/           pSettings[SC_VERTICAL_SPLIT_POSITION].Value <<= nVSplitPos;
/*N*/       pSettings[SC_ACTIVE_SPLIT_RANGE].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_ACTIVESPLITRANGE));
/*N*/       pSettings[SC_ACTIVE_SPLIT_RANGE].Value <<= sal_Int16(eWhichActive);
/*N*/       pSettings[SC_POSITION_LEFT].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_POSITIONLEFT));
/*N*/       pSettings[SC_POSITION_LEFT].Value <<= sal_Int32(nPosX[SC_SPLIT_LEFT]);
/*N*/       pSettings[SC_POSITION_RIGHT].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_POSITIONRIGHT));
/*N*/       pSettings[SC_POSITION_RIGHT].Value <<= sal_Int32(nPosX[SC_SPLIT_RIGHT]);
/*N*/       pSettings[SC_POSITION_TOP].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_POSITIONTOP));
/*N*/       pSettings[SC_POSITION_TOP].Value <<= sal_Int32(nPosY[SC_SPLIT_TOP]);
/*N*/       pSettings[SC_POSITION_BOTTOM].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_POSITIONBOTTOM));
/*N*/       pSettings[SC_POSITION_BOTTOM].Value <<= sal_Int32(nPosY[SC_SPLIT_BOTTOM]);
/*N*/   }
/*N*/ }

//==================================================================

/*N*/ ScViewData::ScViewData( ScDocShell* pDocSh )
/*N*/   :   pDocShell   ( pDocSh ),
/*N*/       pDoc        ( NULL ),
/*N*/       pOptions    ( new ScViewOptions ),
/*N*/       pSpellingView ( NULL ),
/*N*/       aLogicMode  ( MAP_100TH_MM ),
/*N*/       aZoomX      ( 1,1 ),
/*N*/       aZoomY      ( 1,1 ),
/*N*/       aPageZoomX  ( 3,5 ),                    // Page-Default: 60%
/*N*/       aPageZoomY  ( 3,5 ),
/*N*/       eRefType    ( SC_REFTYPE_NONE ),
/*N*/       nTabNo      ( 0 ),
/*N*/       nRefTabNo   ( 0 ),
/*N*/       bActive     ( TRUE ),                   //! wie initialisieren?
/*N*/       bIsRefMode  ( FALSE ),
/*N*/       bDelMarkValid( FALSE ),
/*N*/       nFillMode   ( SC_FILL_NONE ),
/*N*/       bPagebreak  ( FALSE ),
            nTabBarWidth( 0 )
/*N*/ {
/*N*/   SetGridMode     ( TRUE );
/*N*/   SetSyntaxMode   ( FALSE );
/*N*/   SetHeaderMode   ( TRUE );
/*N*/   SetTabMode      ( TRUE );
/*N*/   SetVScrollMode  ( TRUE );
/*N*/   SetHScrollMode  ( TRUE );
/*N*/   SetOutlineMode  ( TRUE );
/*N*/
/*N*/   aScrSize = Size( (long) ( STD_COL_WIDTH           * PIXEL_PER_TWIPS * OLE_STD_CELLS_X ),
/*N*/                    (long) ( ScGlobal::nStdRowHeight * PIXEL_PER_TWIPS * OLE_STD_CELLS_Y ) );
/*N*/   pTabData[0] = new ScViewDataTable;
/*N*/   USHORT i;
/*N*/   for (i=1; i<=MAXTAB; i++) pTabData[i] = NULL;
/*N*/   pThisTab = pTabData[nTabNo];
/*N*/   for (i=0; i<4; i++)
/*N*/   {
/*N*/       pEditView[i] = NULL;
/*N*/       bEditActive[i] = FALSE;
/*N*/   }
/*N*/
/*N*/   nEditEndCol = nEditCol = nEditEndRow = nEditRow = 0;
/*N*/   nTabStartCol = SC_TABSTART_NONE;
/*N*/
/*N*/   if (pDocShell)
/*N*/   {
/*N*/       pDoc = pDocShell->GetDocument();
/*N*/       *pOptions = pDoc->GetViewOptions();
/*N*/   }
/*N*/
/*N*/   //  keine ausgeblendete Tabelle anzeigen:
/*N*/   if (pDoc && !pDoc->IsVisible(nTabNo))
/*N*/   {
/*?*/       while ( !pDoc->IsVisible(nTabNo) && pDoc->HasTable(nTabNo+1) )
/*?*/           ++nTabNo;
/*?*/
/*?*/       pTabData[nTabNo] = new ScViewDataTable;
/*?*/       pThisTab = pTabData[nTabNo];
/*N*/   }
/*N*/
/*N*/   CalcPPT();
/*N*/ }





/*N*/ ScDocument* ScViewData::GetDocument() const
/*N*/ {
/*N*/   if (pDoc)
/*N*/       return pDoc;
/*N*/   else if (pDocShell)
/*?*/       return pDocShell->GetDocument();
/*N*/
/*N*/   OSL_FAIL("kein Document an ViewData");
/*N*/   return NULL;
/*N*/ }

/*N*/ ScViewData::~ScViewData()
/*N*/ {
/*N*/   for (USHORT i=0; i<=MAXTAB; i++)
/*N*/       if (pTabData[i])
/*N*/           delete pTabData[i];
/*N*/
/*N*/   KillEditView();
/*N*/   delete pOptions;
/*N*/ }

// #116578#
void ScViewData::SetPagebreakMode( BOOL bSet )
{
    bPagebreak = bSet;

    CalcPPT();
    RecalcPixPos();
    aScenButSize = Size(0,0);
    aLogicMode.SetScaleX( GetZoomX() );
    aLogicMode.SetScaleY( GetZoomY() );
}

/*N*/ void ScViewData::KillEditView()
/*N*/ {
/*N*/   for (USHORT i=0; i<4; i++)
/*N*/       if (pEditView[i])
/*N*/       {
/*?*/           if (bEditActive[i])
/*?*/               pEditView[i]->GetEditEngine()->RemoveView(pEditView[i]);
/*?*/           delete pEditView[i];
/*?*/           pEditView[i] = NULL;
/*N*/       }
/*N*/ }


// #116578#
void ScViewData::SetTabNo( USHORT nNewTab )
{
    if (nNewTab>MAXTAB)
    {
        OSL_FAIL("falsche Tabellennummer");
        return;
    }

    nTabNo = nNewTab;
    if (!pTabData[nTabNo])
        pTabData[nTabNo] = new ScViewDataTable;
    pThisTab = pTabData[nTabNo];

    CalcPPT();          //  for common column width correction
    RecalcPixPos();     //! nicht immer noetig!
}




/*N*/ void ScViewData::RecalcPixPos()               // nach Zoom-Aenderungen
/*N*/ {
/*N*/   USHORT i;
/*N*/
/*N*/   for (USHORT eWhich=0; eWhich<2; eWhich++)
/*N*/   {
/*N*/       long nPixPosX = 0;
/*N*/       USHORT nPosX = pThisTab->nPosX[eWhich];
/*N*/       for (i=0; i<nPosX; i++)
/*?*/           nPixPosX -= ToPixel(pDoc->GetColWidth(i,nTabNo), nPPTX);
/*N*/       pThisTab->nPixPosX[eWhich] = nPixPosX;
/*N*/
/*N*/       long nPixPosY = 0;
/*N*/       USHORT nPosY = pThisTab->nPosY[eWhich];
/*N*/       for (i=0; i<nPosY; i++)
/*?*/           nPixPosY -= ToPixel(pDoc->FastGetRowHeight(i,nTabNo), nPPTY);
/*N*/       pThisTab->nPixPosY[eWhich] = nPixPosY;
/*N*/   }
/*N*/ }

/*N*/ void ScViewData::CalcPPT()
/*N*/ {
/*N*/   nPPTX = ScGlobal::nScreenPPTX * (double) GetZoomX();
/*N*/   if (pDocShell)
/*N*/       nPPTX = nPPTX / pDocShell->GetOutputFactor();   // Faktor ist Drucker zu Bildschirm
/*N*/   nPPTY = ScGlobal::nScreenPPTY * (double) GetZoomY();
/*N*/ }

//------------------------------------------------------------------

#define SC_OLD_TABSEP   '/'
#define SC_NEW_TABSEP   '+'

// #116578# ReadUserData is needed, must not access pView
void ScViewData::ReadUserData(const String& rData)
{
    if (!rData.Len())       // Leerer String kommt bei "neu Laden"
        return;             // dann auch ohne Assertion beenden

    xub_StrLen nCount = rData.GetTokenCount(';');
    if ( nCount <= 2 )
    {
        //  #45208# beim Reload in der Seitenansicht sind evtl. die Preview-UserData
        //  stehengelassen worden. Den Zoom von der Preview will man hier nicht...
        OSL_FAIL("ReadUserData: das sind nicht meine Daten");
        return;
    }

    String aTabOpt;
    xub_StrLen nTagLen = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(TAG_TABBARWIDTH)).Len();

    //-------------------
    // nicht pro Tabelle:
    //-------------------
    USHORT nTabStart = 2;

    String aZoomStr = rData.GetToken(0);                        // Zoom/PageZoom/Modus
    USHORT nNormZoom = aZoomStr.GetToken(0,'/').ToInt32();
    if ( nNormZoom >= MINZOOM && nNormZoom <= MAXZOOM )
        aZoomX = aZoomY = Fraction( nNormZoom, 100 );           //  "normaler" Zoom (immer)
    USHORT nPageZoom = aZoomStr.GetToken(1,'/').ToInt32();
    if ( nPageZoom >= MINZOOM && nPageZoom <= MAXZOOM )
        aPageZoomX = aPageZoomY = Fraction( nPageZoom, 100 );   // Pagebreak-Zoom, wenn gesetzt
    sal_Unicode cMode = aZoomStr.GetToken(2,'/').GetChar(0);    // 0 oder "0"/"1"
    SetPagebreakMode( cMode == '1' );
    // SetPagebreakMode muss immer gerufen werden wegen CalcPPT / RecalcPixPos()

    //
    //  Tabelle kann ungueltig geworden sein (z.B. letzte Version):
    //
    USHORT nNewTab = rData.GetToken(1).ToInt32();
    if (pDoc->HasTable( nNewTab ))
        SetTabNo(nNewTab);

    //
    // wenn vorhanden, TabBar-Breite holen:
    //
    aTabOpt = rData.GetToken(2);

    if ( nTagLen && aTabOpt.Copy(0,nTagLen).EqualsAscii(TAG_TABBARWIDTH) )
    {
        // #116578# store tab bar width locally
        nTabBarWidth = aTabOpt.Copy(nTagLen).ToInt32();
        nTabStart = 3;
    }

    //-------------
    // pro Tabelle:
    //-------------
    USHORT nPos = 0;
    while ( nCount > nPos+nTabStart )
    {
        aTabOpt = rData.GetToken(nPos+nTabStart);
        if (!pTabData[nPos])
            pTabData[nPos] = new ScViewDataTable;

        sal_Unicode cTabSep = 0;
        if (aTabOpt.GetTokenCount(SC_OLD_TABSEP) >= 11)
            cTabSep = SC_OLD_TABSEP;
#ifndef SC_LIMIT_ROWS
        else if (aTabOpt.GetTokenCount(SC_NEW_TABSEP) >= 11)
            cTabSep = SC_NEW_TABSEP;
        // '+' ist nur erlaubt, wenn wir mit Zeilen > 8192 umgehen koennen
#endif

        if (cTabSep)
        {
            pTabData[nPos]->nCurX = aTabOpt.GetToken(0,cTabSep).ToInt32();
            pTabData[nPos]->nCurY = aTabOpt.GetToken(1,cTabSep).ToInt32();
            pTabData[nPos]->eHSplitMode = (ScSplitMode) aTabOpt.GetToken(2,cTabSep).ToInt32();
            pTabData[nPos]->eVSplitMode = (ScSplitMode) aTabOpt.GetToken(3,cTabSep).ToInt32();

            if ( pTabData[nPos]->eHSplitMode == SC_SPLIT_FIX )
            {
                pTabData[nPos]->nFixPosX = aTabOpt.GetToken(4,cTabSep).ToInt32();
//              UpdateFixX(nPos);
            }
            else
                pTabData[nPos]->nHSplitPos = aTabOpt.GetToken(4,cTabSep).ToInt32();

            if ( pTabData[nPos]->eVSplitMode == SC_SPLIT_FIX )
            {
                pTabData[nPos]->nFixPosY = aTabOpt.GetToken(5,cTabSep).ToInt32();
//              UpdateFixY(nPos);
            }
            else
                pTabData[nPos]->nVSplitPos = aTabOpt.GetToken(5,cTabSep).ToInt32();

            pTabData[nPos]->eWhichActive = (ScSplitPos) aTabOpt.GetToken(6,cTabSep).ToInt32();
            pTabData[nPos]->nPosX[0] = aTabOpt.GetToken(7,cTabSep).ToInt32();
            pTabData[nPos]->nPosX[1] = aTabOpt.GetToken(8,cTabSep).ToInt32();
            pTabData[nPos]->nPosY[0] = aTabOpt.GetToken(9,cTabSep).ToInt32();
            pTabData[nPos]->nPosY[1] = aTabOpt.GetToken(10,cTabSep).ToInt32();

            //  Test, ob der aktive Teil laut SplitMode ueberhaupt existiert
            //  (Bug #44516#)
            ScSplitPos eTest = pTabData[nPos]->eWhichActive;
            if ( ( WhichH( eTest ) == SC_SPLIT_RIGHT &&
                    pTabData[nPos]->eHSplitMode == SC_SPLIT_NONE ) ||
                 ( WhichV( eTest ) == SC_SPLIT_TOP &&
                    pTabData[nPos]->eVSplitMode == SC_SPLIT_NONE ) )
            {
                //  dann wieder auf Default (unten links)
                pTabData[nPos]->eWhichActive = SC_SPLIT_BOTTOMLEFT;
                OSL_FAIL("SplitPos musste korrigiert werden");
            }
        }
        ++nPos;
    }

    RecalcPixPos();
}



// #116578# WriteUserDataSequence must not access pView
/*N*/ void ScViewData::WriteUserDataSequence(uno::Sequence <beans::PropertyValue>& rSettings)
/*N*/ {
/*N*/   rSettings.realloc(SC_VIEWSETTINGS_COUNT);
/*N*/   // + 1, because we have to put the view id in the sequence
/*N*/   beans::PropertyValue* pSettings = rSettings.getArray();
/*N*/   if (pSettings)
/*N*/   {
            sal_uInt16 nViewID = 1;     // #116578# always ScTabViewShell (ID from ScDLL::Init)
/*N*/       pSettings[SC_VIEW_ID].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_VIEWID));
/*N*/       ::rtl::OUStringBuffer sBuffer(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_VIEW)));
/*N*/       SvXMLUnitConverter::convertNumber(sBuffer, static_cast<sal_Int32>(nViewID));
/*N*/       pSettings[SC_VIEW_ID].Value <<= sBuffer.makeStringAndClear();
/*N*/
/*N*/       USHORT nTabCount (pDoc->GetTableCount());
/*N*/       uno::Reference<lang::XMultiServiceFactory> xServiceFactory =
/*N*/                                       ::legacy_binfilters::getLegacyProcessServiceFactory();
/*N*/       DBG_ASSERT( xServiceFactory.is(), "got no service manager" );
/*N*/       if( xServiceFactory.is() )
/*N*/       {
/*N*/           ::rtl::OUString sName(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.NamedPropertyValues"));
/*N*/           uno::Reference<container::XNameContainer> xNameContainer = uno::Reference<container::XNameContainer>(xServiceFactory->createInstance(sName), uno::UNO_QUERY);
/*N*/           if (xNameContainer.is())
/*N*/           {
/*N*/               for (USHORT nTab=0; nTab<nTabCount; nTab++)
/*N*/               {
/*N*/                   if (pTabData[nTab])
/*N*/                   {
/*N*/                       uno::Sequence <beans::PropertyValue> aTableViewSettings;
/*N*/                       pTabData[nTab]->WriteUserDataSequence(aTableViewSettings);
/*N*/                       String _sName;
/*N*/                       GetDocument()->GetName( nTab, _sName );
/*N*/                       ::rtl::OUString sOUName(_sName);
/*N*/                       uno::Any aAny;
/*N*/                       aAny <<= aTableViewSettings;
/*N*/                         try
/*N*/                         {
/*N*/                           xNameContainer->insertByName(sName, aAny);
/*N*/                         }
/*N*/                         //#101739#; two tables with the same name are possible
/*N*/                         catch ( container::ElementExistException& )
/*N*/                         {
/*N*/                             DBG_ERRORFILE("seems there are two tables with the same name");
/*N*/                         }
/*N*/                         catch ( uno::RuntimeException& )
/*N*/                         {
/*N*/                             DBG_ERRORFILE("something went wrong");
/*N*/                         }
/*N*/                   }
/*N*/               }
/*N*/               pSettings[SC_TABLE_VIEWSETTINGS].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_TABLES));
/*N*/               pSettings[SC_TABLE_VIEWSETTINGS].Value <<= xNameContainer;
/*N*/           }
/*N*/       }
/*N*/
/*N*/       String sName;
/*N*/       GetDocument()->GetName( nTabNo, sName );
/*N*/       ::rtl::OUString sOUName(sName);
/*N*/       pSettings[SC_ACTIVE_TABLE].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_ACTIVETABLE));
/*N*/       pSettings[SC_ACTIVE_TABLE].Value <<= sOUName;
/*N*/       pSettings[SC_HORIZONTAL_SCROLL_BAR_WIDTH].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_HORIZONTALSCROLLBARWIDTH));
            pSettings[SC_HORIZONTAL_SCROLL_BAR_WIDTH].Value <<= sal_Int32( nTabBarWidth );  // #116578# use stored value
/*N*/       sal_Int32 nZoomValue ((aZoomY.GetNumerator() * 100) / aZoomY.GetDenominator());
/*N*/       sal_Int32 nPageZoomValue ((aPageZoomY.GetNumerator() * 100) / aPageZoomY.GetDenominator());
/*N*/       pSettings[SC_ZOOM_TYPE].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_ZOOMTYPE));
            pSettings[SC_ZOOM_TYPE].Value <<= sal_Int16( SVX_ZOOM_PERCENT );  // #116578# always direct value for old files
/*N*/       pSettings[SC_ZOOM_VALUE].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_ZOOMVALUE));
/*N*/       pSettings[SC_ZOOM_VALUE].Value <<= nZoomValue;
/*N*/       pSettings[SC_PAGE_VIEW_ZOOM_VALUE].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_PAGEVIEWZOOMVALUE));
/*N*/       pSettings[SC_PAGE_VIEW_ZOOM_VALUE].Value <<= nPageZoomValue;
/*N*/       pSettings[SC_PAGE_BREAK_PREVIEW].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_SHOWPAGEBREAKPREVIEW));
/*N*/       ScUnoHelpFunctions::SetBoolInAny( pSettings[SC_PAGE_BREAK_PREVIEW].Value, bPagebreak);
/*N*/
/*N*/       if (pOptions)
/*N*/       {
/*N*/           pSettings[SC_SHOWZERO].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_SHOWZERO));
/*N*/           ScUnoHelpFunctions::SetBoolInAny( pSettings[SC_SHOWZERO].Value, pOptions->GetOption( VOPT_NULLVALS ) );
/*N*/           pSettings[SC_SHOWNOTES].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_SHOWNOTES));
/*N*/           ScUnoHelpFunctions::SetBoolInAny( pSettings[SC_SHOWNOTES].Value, pOptions->GetOption( VOPT_NOTES ) );
/*N*/           pSettings[SC_SHOWGRID].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_SHOWGRID));
/*N*/           ScUnoHelpFunctions::SetBoolInAny( pSettings[SC_SHOWGRID].Value, pOptions->GetOption( VOPT_GRID ) );
/*N*/           pSettings[SC_GRIDCOLOR].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_GRIDCOLOR));
/*N*/           String aColorName;
/*N*/           Color aColor = pOptions->GetGridColor(&aColorName);
/*N*/           pSettings[SC_GRIDCOLOR].Value <<= static_cast<sal_Int64>(aColor.GetColor());
/*N*/           pSettings[SC_SHOWPAGEBR].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_SHOWPAGEBR));
/*N*/           ScUnoHelpFunctions::SetBoolInAny( pSettings[SC_SHOWPAGEBR].Value, pOptions->GetOption( VOPT_PAGEBREAKS ) );
/*N*/           pSettings[SC_COLROWHDR].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_COLROWHDR));
/*N*/           ScUnoHelpFunctions::SetBoolInAny( pSettings[SC_COLROWHDR].Value, pOptions->GetOption( VOPT_HEADER ) );
/*N*/           pSettings[SC_SHEETTABS].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_SHEETTABS));
/*N*/           ScUnoHelpFunctions::SetBoolInAny( pSettings[SC_SHEETTABS].Value, pOptions->GetOption( VOPT_TABCONTROLS ) );
/*N*/           pSettings[SC_OUTLSYMB].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_OUTLSYMB));
/*N*/           ScUnoHelpFunctions::SetBoolInAny( pSettings[SC_OUTLSYMB].Value, pOptions->GetOption( VOPT_OUTLINER ) );
/*N*/
/*N*/           const ScGridOptions& aGridOpt = pOptions->GetGridOptions();
/*N*/           pSettings[SC_SNAPTORASTER].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_SNAPTORASTER));
/*N*/           ScUnoHelpFunctions::SetBoolInAny( pSettings[SC_SNAPTORASTER].Value, aGridOpt.GetUseGridSnap() );
/*N*/           pSettings[SC_RASTERVIS].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_RASTERVIS));
/*N*/           ScUnoHelpFunctions::SetBoolInAny( pSettings[SC_RASTERVIS].Value, aGridOpt.GetGridVisible() );
/*N*/           pSettings[SC_RASTERRESX].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_RASTERRESX));
/*N*/           pSettings[SC_RASTERRESX].Value <<= static_cast<sal_Int32> ( aGridOpt.GetFldDrawX() );
/*N*/           pSettings[SC_RASTERRESY].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_RASTERRESY));
/*N*/           pSettings[SC_RASTERRESY].Value <<= static_cast<sal_Int32> ( aGridOpt.GetFldDrawY() );
/*N*/           pSettings[SC_RASTERSUBX].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_RASTERSUBX));
/*N*/           pSettings[SC_RASTERSUBX].Value <<= static_cast<sal_Int32> ( aGridOpt.GetFldDivisionX() );
/*N*/           pSettings[SC_RASTERSUBY].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_RASTERSUBY));
/*N*/           pSettings[SC_RASTERSUBY].Value <<= static_cast<sal_Int32> ( aGridOpt.GetFldDivisionY() );
/*N*/           pSettings[SC_RASTERSYNC].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_RASTERSYNC));
/*N*/           ScUnoHelpFunctions::SetBoolInAny( pSettings[SC_RASTERSYNC].Value, aGridOpt.GetSynchronize() );
/*N*/       }
/*N*/   }
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
