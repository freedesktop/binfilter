/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifdef _MSC_VER
#pragma optimize("",off)
#endif

#include "schattr.hxx"
#include "memchrt.hxx"

#ifndef _SVX_CHRTITEM_HXX //autogen
#define ITEMID_DOUBLE           0
#define ITEMID_CHARTTEXTORDER   SCHATTR_TEXT_ORDER
#define ITEMID_CHARTTEXTORIENT  SCHATTR_TEXT_ORIENT
#define ITEMID_CHARTLEGENDPOS   SCHATTR_LEGEND_POS
#define ITEMID_CHARTDATADESCR   SCHATTR_DATADESCR_DESCR

#include <bf_svtools/eitem.hxx>

#endif


#include <bf_svx/eeitem.hxx>

#define ITEMID_FONT        EE_CHAR_FONTINFO
#define ITEMID_COLOR       EE_CHAR_COLOR
#define ITEMID_FONTHEIGHT  EE_CHAR_FONTHEIGHT
#include <bf_svx/svxids.hrc>
#include <globfunc.hxx>





#include "chaxis.hxx"
namespace binfilter {

// Hier abhaengig von der Reihe den Style zur�ckgeben, vorerst ist nur aufsplitten der
// Verbundcharts von n�ten:
//TVM: bitte nicht benutzen! renovierungsbed�rftig!


/*N*/ SchMemChart* ChartModel::GetChartData() const
/*N*/ {
/*N*/   return pChartData;
/*N*/ }


/*N*/ long ChartModel::GetColCount() const
/*N*/ {
/*N*/   if( !pChartData ) return 0;         // GPF via GetMenuState
/*N*/
/*N*/   return IsDataSwitched()   //abhaengig vom Charttyp - statt bSwitchData
/*N*/              ? pChartData->GetRowCount()
/*N*/              : pChartData->GetColCount();
/*N*/ }


/*N*/ long ChartModel::GetRowCount() const
/*N*/ {
/*N*/   if( !pChartData ) return 0;         // GPF via GetMenuState
/*N*/
/*N*/   return IsDataSwitched()   //abhaengig vom Charttyp - statt bSwitchData
/*N*/              ? pChartData->GetColCount()
/*N*/              : pChartData->GetRowCount();
/*N*/ }






/*N*/ SvxChartStyle ChartModel::ChartStyle () const
/*N*/ {
/*N*/   return eChartStyle;
/*N*/ }


/*N*/ SvxChartStyle &ChartModel::ChartStyle()
/*N*/ {
/*N*/   return eChartStyle;
/*N*/ }



/*N*/ long ChartModel::PieSegOfs( long nCol ) const
/*N*/ {
/*N*/     DBG_ASSERT( nCol < nPieSegCount, "pie segment requested is out of bounds" );
/*N*/
/*N*/     if( IsPieChart() &&
/*N*/         nCol >= 0 && nCol < nPieSegCount )
/*N*/     {
/*N*/         return pPieSegOfs[ nCol ];
/*N*/     }
/*N*/
/*N*/     return 0;
/*N*/ }



/*N*/ BOOL& ChartModel::ShowMainTitle ()
/*N*/ {
/*N*/   return bShowMainTitle;
/*N*/ }




/*N*/ String& ChartModel::MainTitle ()
/*N*/ {
/*N*/   return aMainTitle;
/*N*/ }




/*N*/ BOOL& ChartModel::ShowSubTitle()
/*N*/ {
/*N*/   return bShowSubTitle;
/*N*/ }




/*N*/ String& ChartModel::SubTitle ()
/*N*/ {
/*N*/   return aSubTitle;
/*N*/ }




/*N*/ BOOL& ChartModel::ShowXAxisTitle ()
/*N*/ {
/*N*/   return bShowXAxisTitle;
/*N*/ }


/*N*/ String& ChartModel::XAxisTitle ()
/*N*/ {
/*N*/   return aXAxisTitle;
/*N*/ }






/*N*/ BOOL& ChartModel::ShowYAxisTitle ()
/*N*/ {
/*N*/   return bShowYAxisTitle;
/*N*/ }
/*N*/



/*N*/ String& ChartModel::YAxisTitle ()
/*N*/ {
/*N*/   return aYAxisTitle;
/*N*/ }




/*N*/ BOOL& ChartModel::ShowZAxisTitle ()
/*N*/ {
/*N*/   return bShowZAxisTitle;
/*N*/ }




/*N*/ String& ChartModel::ZAxisTitle()
/*N*/ {
/*N*/   return aZAxisTitle;
/*N*/ }


/*N*/ BOOL ChartModel::ShowXAxis () const
/*N*/ {
/*N*/   return pChartXAxis->IsVisible();
/*N*/ }


/*N*/ void ChartModel::ShowXAxis (BOOL b)
/*N*/ {
/*N*/   pChartXAxis->ShowAxis(b);
/*N*/ }




/*N*/ BOOL& ChartModel::ShowXGridMain ()
/*N*/ {
/*N*/   return bShowXGridMain;
/*N*/ }




/*N*/ BOOL& ChartModel::ShowXGridHelp ()
/*N*/ {
/*N*/   return bShowXGridHelp;
/*N*/ }


/*N*/ BOOL ChartModel::ShowXDescr () const
/*N*/ {
/*N*/   return pChartXAxis->HasDescription();
/*N*/ }


/*N*/ void ChartModel::ShowXDescr(BOOL b)
/*N*/ {
/*N*/   pChartXAxis->ShowDescr(b);
/*N*/ }


/*N*/ BOOL ChartModel::ShowYAxis () const
/*N*/ {
/*N*/   return pChartYAxis->IsVisible();
/*N*/ }
/*N*/ void ChartModel::ShowYAxis (BOOL b)
/*N*/ {
/*N*/   pChartYAxis->ShowAxis(b);
/*N*/ }



/*N*/ BOOL& ChartModel::ShowYGridMain ()
/*N*/ {
/*N*/   return bShowYGridMain;
/*N*/ }




/*N*/ BOOL& ChartModel::ShowYGridHelp ()
/*N*/ {
/*N*/   return bShowYGridHelp;
/*N*/ }


/*N*/ BOOL ChartModel::ShowYDescr () const
/*N*/ {
/*N*/   return pChartYAxis->HasDescription();
/*N*/ }


/*N*/ void ChartModel::ShowYDescr( BOOL b)
/*N*/ {
/*N*/   pChartYAxis->ShowDescr(b);
/*N*/ }


/*N*/ BOOL ChartModel::ShowZAxis () const
/*N*/ {
/*N*/   return pChartZAxis->IsVisible();
/*N*/ }


/*N*/ void ChartModel::ShowZAxis(BOOL b)
/*N*/ {
/*N*/   pChartZAxis->ShowAxis(b);
/*N*/ }




/*N*/ BOOL& ChartModel::ShowZGridMain ()
/*N*/ {
/*N*/   return bShowZGridMain;
/*N*/ }




/*N*/ BOOL& ChartModel::ShowZGridHelp ()
/*N*/ {
/*N*/   return bShowZGridHelp;
/*N*/ }


/*N*/ BOOL ChartModel::ShowZDescr () const
/*N*/ {
/*N*/   return pChartZAxis->HasDescription();
/*N*/ }


/*N*/ void ChartModel::ShowZDescr (BOOL b)
/*N*/ {
/*N*/   pChartZAxis->ShowDescr(b);
/*N*/ }


/*N*/ BOOL ChartModel::HasAxis( long nObjectId ) const
/*N*/ {
/*N*/     if( CHOBJID_ANY == nObjectId )
/*N*/         return IsAxisChart() &&
/*N*/             ( pChartXAxis->IsVisible() ||
/*N*/               pChartYAxis->IsVisible() ||
/*N*/               pChartAAxis->IsVisible() ||
/*N*/               pChartBAxis->IsVisible() ||
/*N*/               (Is3DChart() && pChartZAxis->IsVisible()) );
/*N*/
/*N*/     switch( nObjectId )
/*N*/     {
/*N*/         case CHOBJID_DIAGRAM_X_AXIS:
/*N*/             return  pChartXAxis->IsVisible();
/*N*/         case CHOBJID_DIAGRAM_Y_AXIS:
/*N*/             return  pChartYAxis->IsVisible();
/*N*/         case CHOBJID_DIAGRAM_Z_AXIS:
/*N*/             return  (Is3DChart() && pChartZAxis->IsVisible());
/*N*/         case CHOBJID_DIAGRAM_A_AXIS:
/*N*/             return  pChartAAxis->IsVisible();
/*N*/         case CHOBJID_DIAGRAM_B_AXIS:
/*N*/             return  pChartBAxis->IsVisible();
/*N*/     }
/*N*/
/*N*/     return FALSE;
/*N*/ }


/*N*/ const SfxItemSet& ChartModel::GetLegendAttr() const
/*N*/ {
/*N*/   return *pLegendAttr;
/*N*/ }









//ToDo: diese Fkt. �berfl�ssig machen (Problem: Austausch der Achseneigenschaften nicht NUR �ber Attr (??? stimmt das ???)













































/*N*/ SdrOutliner* ChartModel::GetOutliner() const
/*N*/ {
/*N*/   return pOutliner;
/*N*/ }

/*N*/ BOOL ChartModel::IsInitialized() const
/*N*/ {
/*N*/   return mbIsInitialized;
/*N*/ }




/*N*/ BOOL& ChartModel::ReadError ()
/*N*/ {
/*N*/   return bReadError;
/*N*/ }


/*N*/ ChartScene* ChartModel::GetScene()
/*N*/ {
/*N*/   return pScene;
/*N*/ }

/*N*/ SvNumberFormatter* ChartModel::GetNumFormatter() const
/*N*/ {
/*N*/   return pNumFormatter;
/*N*/ }






























/*N*/ int& ChartModel::Granularity ()
/*N*/ {
/*N*/   return nGranularity;
/*N*/ }




/*N*/ Size& ChartModel::InitialSize ()
/*N*/ {
/*N*/   return aInitialSize;
/*N*/ }


















/*N*/ void ChartModel::SetPieSegOfs( long nCol, long nOfs )
/*N*/ {
/*N*/     DBG_ASSERT( pPieSegOfs, "Invalid Array" );
/*N*/     DBG_ASSERT( nCol < nPieSegCount, "trying to set pie offset out of bounds" );
/*N*/
/*N*/     if( nCol >= 0 && nCol < nPieSegCount )
/*N*/         pPieSegOfs[ nCol ] = nOfs;
/*N*/ }


/*N*/ String& ChartModel::ColText( long nCol )
/*N*/ {
/*N*/   return IsDataSwitched()   //abhaengig vom Charttyp - statt bSwitchData
/*N*/              ? (String&) pChartData->GetTransRowText(nCol)
/*N*/              : (String&) pChartData->GetTransColText(nCol);
/*N*/ }


/*N*/ String& ChartModel::RowText( long nRow )
/*N*/ {
/*N*/   return IsDataSwitched()   //abhaengig vom Charttyp - statt bSwitchData
/*N*/              ? (String&) pChartData->GetTransColText(nRow)
/*N*/              : (String&) pChartData->GetTransRowText(nRow);
/*N*/ }




// FG: Ist eine Abfrage ob die Beschriftung eines Charts Zentriert unter einem Datenpunkt (Regelfall) steht
//     oder zentriert zwischen 2 Marken.
/*N*/ BOOL ChartModel::IsDescriptionCenteredUnderDataPoint ()
/*N*/ {
/*N*/   switch (eChartStyle)
/*N*/   {
/*N*/       case CHSTYLE_2D_STACKEDCOLUMN :
/*N*/       case CHSTYLE_2D_COLUMN :
/*N*/       case CHSTYLE_2D_PERCENTCOLUMN :
/*N*/           return FALSE;

/*N*/       default :
/*N*/           return TRUE;
/*N*/   }
/*N*/ }
/*N*/ void ChartModel::LockBuild()
/*N*/ {
/*N*/   bNoBuildChart=TRUE;
/*N*/ }
/*N*/ void ChartModel::UnlockBuild()
/*N*/ {
/*N*/   bNoBuildChart=FALSE;
/*N*/   if(bShouldBuildChart)
/*N*/       BuildChart(FALSE); //evtl. TRUE, um Ranges zu pr�fen???
/*N*/
/*N*/   // force broadcast SFX_HINT_DOCCHANGED
/*N*/   SfxObjectShell* pObjSh = GetObjectShell();
/*N*/   if( pObjSh )
/*N*/   {
/*N*/       pObjSh->SetModified( pObjSh->IsModified());
/*N*/   }
/*N*/ }
/*N*/ BOOL ChartModel::IsLockedBuild()
/*N*/ {
/*N*/   return bNoBuildChart;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
