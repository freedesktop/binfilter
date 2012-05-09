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

#include <bf_svx/svxids.hrc>
#include <bf_svx/xlntrit.hxx>
#include <bf_svx/svdviter.hxx>
#include <bf_svx/svdview.hxx>
#ifndef _ZFORLIST_HXX //autogen
#ifndef _ZFORLIST_DECLARE_TABLE
#define _ZFORLIST_DECLARE_TABLE
#endif
#include <bf_svtools/zforlist.hxx>
#endif

#include "schattr.hxx"
#define ITEMID_ADJUST EE_PARA_JUST
#include <bf_svx/adjitem.hxx>

#ifndef _SVX_CHRTITEM_HXX //autogen
#define ITEMID_DOUBLE           0
#define ITEMID_CHARTDATADESCR   SCHATTR_DATADESCR_DESCR
#define ITEMID_CHARTTEXTORIENT  SCHATTR_TEXT_ORIENT
#endif

#define ITEMID_FONT        EE_CHAR_FONTINFO
#define ITEMID_COLOR       EE_CHAR_COLOR
#define ITEMID_FONTHEIGHT  EE_CHAR_FONTHEIGHT
#include "globfunc.hxx"
#include <bf_svx/svxids.hrc>
#include "schresid.hxx"

#include "glob.hrc"
#include <bf_svx/dialogs.hrc>
#include "math.h"

#include <bf_svx/xlnedcit.hxx>
#include <bf_svx/xlnstcit.hxx>
#include <bf_svx/xlnedwit.hxx>
#include <bf_svx/xlnstwit.hxx>
#include <bf_svx/xlnedit.hxx>
#include <bf_svx/xlnstit.hxx>
#include <bf_svx/xlndsit.hxx>
#include <bf_svx/xlnclit.hxx>
#include <bf_svx/xlnwtit.hxx>

#include "pairs.hxx"
#include "memchrt.hxx"
#include "chaxis.hxx"
#include "chmod3d.hxx"
#include "schmod.hxx"

namespace binfilter {

/************************************************************************/

/*N*/ const double fDefaultArr[DEFAULT_ROWCNT][DEFAULT_COLCNT] =
/*N*/ {
/*N*/   { 9.1,  3.2,    4.54 },
/*N*/   { 2.4,  8.8,    9.65 },
/*N*/   { 3.1,  1.5,    3.7 },
/*N*/   { 4.3,  9.02,   6.2 }
/*N*/ };

/*************************************************************************
|*
|* delete top-level object (called by BuildChart to create shape newly)
|*
\************************************************************************/

/*N*/ void ChartModel::DeleteObject( SdrObject* pObj )
/*N*/ {
/*N*/   if( pObj )
/*N*/   {
/*N*/       // unmark object in all views
/*N*/       SdrViewIter aIter( pObj );
/*N*/       SdrView* pView = aIter.FirstView();
/*N*/
/*N*/       while( pView )
/*N*/       {
/*N*/           // important: leave group so that SdrPageView's object list is always
/*N*/           // the same as the object list of the page
/*N*/           pView->LeaveAllGroup();
/*N*/           pView->UnmarkAll();
/*N*/
/*N*/           pView = aIter.NextView();
/*N*/       }
/*N*/
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*N*/           SdrObject* pDelObj = pObj->GetObjList()->RemoveObject( pObj->GetOrdNum());
/*N*/           DBG_ASSERT( pDelObj == pObj , "Wrong object removed!" );
/*N*/ #else
/*N*/           pObj->GetObjList()->RemoveObject( pObj->GetOrdNum());
/*N*/ #endif
/*N*/
/*N*/           delete pObj;
/*N*/   }
/*N*/   else
/*N*/       OSL_FAIL( "DeleteObject: Invalid object" );
/*N*/ }

/*N*/ const USHORT nExchangeTitleWhichPairs[] =
/*N*/ {                                                    //approx.:
/*N*/   SCHATTR_TEXT_ORIENT, SCHATTR_TEXT_ORIENT,       // 1
/*N*/   SCHATTR_TEXT_DEGREES,SCHATTR_TEXT_DEGREES,      //
/*N*/   XATTR_LINE_FIRST, XATTR_LINE_LAST,              // 1000
/*N*/   XATTR_FILL_FIRST, XATTR_FILL_LAST,              // 1020
/*N*/   SDRATTR_START, SDRATTR_END, //here changed to all SdrAttr
/*N*/   EE_ITEMS_START, EE_ITEMS_END,                   // 4000
/*N*/   0
/*N*/ };

/*************************************************************************
|*
|* Create text object for diagrams
|* FG: The parameter MaximumWidth is only used if >0. In that case lines are
|*     are wrapped if necessary. MaximumWidth has to passed to CalcMaxDescrSize
|*     too, otherwise the formatting is wrong.
|*
|*     At a maximum 2 lines of that width are created. The rest is cut off.
|*
|*      That should be reimplemented because:
|*      1)  The text is put into an outline object gepackt, in order to be
|*          able to process stacked text.
|*      2)  based on that object the size and the width are calculated
|*      3)  then this size is taken to create a SdrTextObj with the same
|*          attributes as the outline object.
|*
\************************************************************************/

/*N*/ SdrRectObj* ChartModel::CreateTextObj(UINT16           nId,
/*N*/                                     const Point      &rPos,
/*N*/                                     const String     &rText,
/*N*/                                     const SfxItemSet &rAttr,
/*N*/                                     BOOL             bIsTitle,
/*N*/                                     ChartAdjust      eAdjust,
/*N*/                                     const long       nMaximumWidth)
/*N*/ {
/*N*/   const SfxPoolItem* pPoolItem = NULL;
/*N*/   SvxChartTextOrient eOrient;
/*N*/
/*N*/   if (rAttr.GetItemState(SCHATTR_TEXT_ORIENT, TRUE, &pPoolItem) == SFX_ITEM_SET)
/*N*/       eOrient = ((const SvxChartTextOrientItem*)pPoolItem)->GetValue();
/*N*/   else
/*N*/       eOrient = CHTXTORIENT_STANDARD;
/*N*/
/*N*/   SfxItemSet aTextAttr(*pItemPool, nTextWhichPairs);
/*N*/   aTextAttr.Put(rAttr);
/*N*/
/*N*/   if (bIsTitle)
/*N*/       aTextAttr.Put(SvxAdjustItem(SVX_ADJUST_CENTER));
/*N*/   else
/*N*/       aTextAttr.Put(SvxAdjustItem((eOrient == CHTXTORIENT_BOTTOMTOP)
/*N*/                                        ? SVX_ADJUST_RIGHT
/*N*/                                        : SVX_ADJUST_LEFT));
/*N*/
/*N*/     // clear old outliner content
/*N*/   pOutliner->Clear();
/*N*/
/*N*/   if (eOrient == CHTXTORIENT_STACKED)
/*?*/       pOutliner->SetText(StackString(rText), pOutliner->GetParagraph( 0 ));
/*N*/   else
/*N*/       pOutliner->SetText(rText, pOutliner->GetParagraph( 0 ));
/*N*/
/*N*/       // FG: This routine actually calculates if the text has to be
            // wrapped
/*N*/   Size aSize = CalcTextSizeOfOneText (eOrient, aTextAttr, pOutliner, nMaximumWidth,FALSE);
/*N*/
/*N*/       // FG: What is the purpose of this statement?
/*N*/   OutlinerParaObject* pPara = pOutliner->CreateParaObject();
/*N*/
/*N*/       // FG: Now the text created and formatted by the statements above
            // is destroyed.
/*N*/   pOutliner->Clear();
/*N*/
/*N*/   //rPos=position in ChartRect, is passed as arg
/*N*/   //size results from CalcTextOf... (see above)
/*N*/   Rectangle aRect(rPos, aSize);
/*N*/   SdrRectObj* pObj;
/*N*/
/*N*/   AdjustRect(aRect, eAdjust);
/*N*/
/*N*/   switch(nId)
/*N*/   {
/*N*/       //All titles are responsible for their attributes
/*N*/       case CHOBJID_DIAGRAM_TITLE_X_AXIS:
/*N*/       case CHOBJID_DIAGRAM_TITLE_Y_AXIS:
/*N*/       case CHOBJID_DIAGRAM_TITLE_Z_AXIS:
/*N*/       case CHOBJID_TITLE_MAIN:
/*N*/       case CHOBJID_TITLE_SUB:
/*N*/           pObj = new SchRectObj(OBJ_TEXT, aRect);
/*N*/           break;
/*N*/       default:
/*N*/           pObj = new SdrRectObj(OBJ_TEXT, aRect);
/*N*/           break;
/*N*/
/*N*/   }
/*N*/
/*N*/   // Since 4/1998 text can be rotated by an arbitrary angle: SCHATTR_TEXT_DEGREES
/*N*/   long nDegrees=GetTextRotation((SfxItemSet&)rAttr,eOrient);
/*N*/   if(nDegrees)
/*N*/   {
/*N*/      Rectangle aOldBoundRect=pObj->GetBoundRect();
/*N*/      double fVal=nDegrees * nPi180;
/*N*/      pObj->Rotate(pObj->GetSnapRect().Center(), nDegrees, sin(fVal), cos(fVal));
/*N*/      pObj->NbcMove( AdjustRotatedRect(aOldBoundRect, eAdjust,pObj->GetBoundRect()));
/*N*/   }
/*N*/
/*N*/   //ToDo: appending of ,*(pItemPool->GetFrozenIdRanges()) ???, preliminaryly with these Whichpairs
/*N*/   SfxItemSet aAreaAttr(*pItemPool,nExchangeTitleWhichPairs);
/*N*/
/*N*/   //Neu: #52009#
/*N*/   aAreaAttr.Put(SdrTextAutoGrowHeightItem( bIsTitle ));
/*N*/   aAreaAttr.Put(SdrTextAutoGrowWidthItem( bIsTitle ));
/*N*/   aAreaAttr.Put(rAttr);
/*N*/
/*N*/   pObj->SetItemSet(aAreaAttr);//#63904# 10%
/*N*/
/*N*/   pObj->InsertUserData(new SchObjectId(nId));
/*N*/   pObj->InsertUserData(new SchObjectAdjust(eAdjust, eOrient));
/*N*/   pObj->NbcSetOutlinerParaObject(pPara);
/*N*/
/*N*/   return pObj;
/*N*/ }

/*************************************************************************
|*
|* Set text of diagram-text objects
|*
\************************************************************************/


/*************************************************************************
|*
|* Set attributes of diagram-text objects
|*
\************************************************************************/

/*N*/ void ChartModel::SetTextAttr(SdrTextObj& rTextObj,
/*N*/                            const SfxItemSet& rAttr,
/*N*/                            const long /*nMaximumWidth*/)
/*N*/ {
/*N*/     rTextObj.SetItemSetAndBroadcast( rAttr );
/*N*/
/*N*/   if ( rTextObj.GetOutlinerParaObject())
/*N*/   {
/*N*/       SfxItemSet aTextAttr(*pItemPool, nTextWhichPairs);
/*N*/       aTextAttr.Put(rAttr);
/*N*/       aTextAttr.Put(SvxAdjustItem());
/*N*/
/*N*/       pOutliner->SetText(*rTextObj.GetOutlinerParaObject());
/*N*/
/*N*/       SetTextAttributes (aTextAttr);
/*N*/
/*N*/       // in this case a text resize/reorg might be necessary
/*N*/
/*N*/       Size aSize = pOutliner->CalcTextSize();
/*N*/       aSize.Height() += TEXTHEIGHT_OFS;
/*N*/       aSize.Width () = (aSize.Width () * 6) / 5;
/*N*/
/*N*/       OutlinerParaObject* pPara = pOutliner->CreateParaObject();
/*N*/
/*N*/       pOutliner->Clear();
/*N*/
/*N*/       rTextObj.SetOutlinerParaObject(pPara);
/*N*/       AdjustTextSize(rTextObj, aSize);
/*N*/   }
/*N*/ }
/*************************************************************************
|*
|* Liefert die maximale Beschriftungs-Text-Groesse
|* FG: Hier wird der groesste Text berechnet und die Groesse des ersten
|*     und letzten (je nach Chart-Typ braucht man einen oder alle 3 Werte
|*     nNumberFormat ist der Index fuer ein Zahlenformat, dies kann noetig
|*       sin um die wahre Textbreite und Hoehe auszurechnen
|*     nMaximumWidth ist die Breite ab der der Text umgebrochen wird
|*
\************************************************************************/

/*N*/ Size ChartModel::CalcMaxDescrSize(BOOL               bRowDescr,
/*N*/                                 SvxChartTextOrient eOrient,
/*N*/                                 const UINT32       nNumberFormat,
/*N*/                                 long               nAxisUId,
/*N*/                                 const long         MaximumWidth,
/*N*/                                 Pair*              pFirstAndLast )
/*N*/ {
/*N*/   ChartAxis *pCurrentXAxis = (nAxisUId == CHAXIS_AXIS_A)? pChartAAxis: pChartXAxis;
/*N*/
/*N*/   BOOL bLogarithm = pCurrentXAxis->IsLogarithm();
/*N*/   short nCnt;
/*N*/   SfxItemSet* pLclAxisAttr;
/*N*/   if (bRowDescr)
/*N*/   {
/*N*/       nCnt            = GetRowCount();
/*N*/       pLclAxisAttr        = &GetAttr(CHOBJID_DIAGRAM_Z_AXIS);
/*N*/   }
/*N*/   else
/*N*/   {
/*N*/       nCnt            = GetColCount();
/*N*/       pLclAxisAttr        = &GetAttr(CHOBJID_DIAGRAM_X_AXIS);
/*N*/   }
/*N*/
/*N*/   pOutliner->SetUpdateMode (FALSE);
/*N*/
/*N*/   Size aMaxSize(0, 0);
/*N*/
/*N*/   if (IsXYChart())
/*N*/   {
        /**********************************************************************
        * XY-Chart oder Zeilen muessen zusammengefasst werden
        **********************************************************************/
/*N*/       if (/*IsXYChart() && */pCurrentXAxis->GetMin() == pCurrentXAxis->GetMax())//#55400#
/*N*/       {
/*?*/           return Size (0, 0);
/*N*/       }
/*N*/       else
/*N*/       {
/*N*/
/*N*/           // #55400#
/*N*/           // #67961#
/*N*/           double fMinX  = pCurrentXAxis->GetMin();
/*N*/           double fMaxX  = pCurrentXAxis->GetMax();
/*N*/           double fStepX = pCurrentXAxis->GetStep();
/*N*/
/*N*/           //  Collect the items which control the text appearence and set them to
/*N*/           //  the outliner.
/*N*/           SfxItemSet aTextAttr(*pItemPool, nTextOrientWhichPairs);
/*N*/           aTextAttr.Put(GetAttr(CHOBJID_DIAGRAM_X_AXIS));
/*N*/
/*N*/           double fAct = fMinX;
/*N*/
/*N*/           while (fAct <= fMaxX)
/*N*/           {
/*N*/               String aNumStr;
/*N*/               Color* pDummy = NULL;
/*N*/
/*N*/                 pNumFormatter->GetOutputString(fAct, nNumberFormat, aNumStr, &pDummy);
/*N*/
/*N*/               if (eOrient == CHTXTORIENT_STACKED)
/*?*/                   pOutliner->SetText(StackString(aNumStr), pOutliner->GetParagraph( 0 ));
/*N*/               else
/*N*/                   pOutliner->SetText(aNumStr, pOutliner->GetParagraph( 0 ));
/*N*/
/*N*/               // FG: Hier wird wirklich berechnet wie groß der Textbereich werden soll. Insbesondere
/*N*/               //     wird hier entschieden, ob der Text umgebrochen werden soll oder nicht!
/*N*/               SetTextAttributes (aTextAttr);
/*N*/               Size aSize = CalcTextSizeOfOneText (eOrient, aTextAttr, pOutliner, MaximumWidth,
/*N*/                   TRUE, FALSE);
/*N*/               pOutliner->SetUpdateMode (FALSE);
/*N*/
/*N*/               pOutliner->Clear();
/*N*/
/*N*/               if (aSize.Width() > aMaxSize.Width())
/*N*/                   aMaxSize.Width() = aSize.Width();
/*N*/               if (aSize.Height() > aMaxSize.Height())
/*N*/                   aMaxSize.Height() = aSize.Height();
/*N*/
/*N*/                   // FG: Die Berechnung erfolgt hier, damit die Raender in Create2DBackplane
/*N*/               if (fAct <= fMinX)
/*N*/               {
/*N*/                   nWidthOfFirstXAxisText = aSize.Width();
/*N*/                   if(pFirstAndLast)
/*N*/                       pFirstAndLast->A()=nWidthOfFirstXAxisText;
/*N*/
/*N*/               }
/*N*/               IncValue(fAct, fStepX, bLogarithm);
/*N*/                   // FG: Die Berechnung erfolgt hier, damit die Raender in Create2DBackplane
/*N*/               if (fAct >= fMaxX)
/*N*/               {
/*N*/                   nWidthOfLastXAxisText = aSize.Width();
/*N*/                   if(pFirstAndLast)
/*N*/                       pFirstAndLast->B()=nWidthOfLastXAxisText;
/*N*/               }
/*N*/
/*N*/           }
/*N*/       }
/*N*/   }
/*N*/   else
/*N*/   {
/*N*/       //  Collect the items which control the text appearence and set them to
/*N*/       //  the outliner.
/*N*/       SfxItemSet aTextAttr(*pItemPool, nTextOrientWhichPairs);
/*N*/       aTextAttr.Put(*pLclAxisAttr);
/*N*/
/*N*/       for (short i = 0; i < nCnt; i++)
/*N*/       {
/*N*/           String aDescrStr = bRowDescr ? RowText(i) : ColText(i);
/*N*/
/*N*/           if (eOrient == CHTXTORIENT_STACKED)
/*?*/               pOutliner->SetText(StackString(aDescrStr), pOutliner->GetParagraph( 0 ));
/*N*/           else
/*N*/               pOutliner->SetText(aDescrStr, pOutliner->GetParagraph( 0 ));
/*N*/
/*N*/           // FG: Hier wird wirklich berechnet wie groß der Textbereich werden soll. Insbesondere wird
/*N*/           //     hier entschieden, ob der Text umgebrochen werden soll oder nicht!
/*N*/           SetTextAttributes (aTextAttr);
/*N*/           Size aSize = CalcTextSizeOfOneText (eOrient, aTextAttr, pOutliner, MaximumWidth,TRUE,
/*N*/               FALSE);
/*N*/           pOutliner->SetUpdateMode (FALSE);
/*N*/           pOutliner->Clear();
/*N*/
/*N*/           if (aSize.Width() > aMaxSize.Width())
/*N*/               aMaxSize.Width() = aSize.Width();
/*N*/           if (aSize.Height() > aMaxSize.Height())
/*N*/               aMaxSize.Height() = aSize.Height();
/*N*/           // FG: Die Berechnung erfolgt hier, damit die Raender in Create2DBackplane
/*N*/           //     richtig berechnet werden koennen.
/*N*/           if (i == 0)
/*N*/           {
/*N*/               nWidthOfFirstXAxisText = aSize.Width();
/*N*/               if(pFirstAndLast)
/*N*/                   pFirstAndLast->A()=nWidthOfFirstXAxisText;
/*N*/           }
/*N*/           if (i == nCnt-1)
/*N*/           {
/*N*/               nWidthOfLastXAxisText = aSize.Width();
/*N*/               if(pFirstAndLast)
/*N*/                   pFirstAndLast->B()=nWidthOfLastXAxisText;
/*N*/           }
/*N*/       }
/*N*/   }
/*N*/
/*N*/   pOutliner->SetUpdateMode (TRUE);
/*N*/
/*N*/   return aMaxSize;
/*N*/ }

/*************************************************************************
|*
|* Datenzeilen-Attributsets initialisieren
|*
\************************************************************************/

//
// schneller machen, es muß nicht alles neu gebaut werden
//

/*N*/ void ChartModel::InitDataAttrs()
/*N*/ {
/*N*/   CHART_TRACE( "ChartModel::InitDataAttrs" );
/*N*/
/*N*/   DBG_ASSERT( pChartData, "ChartModel::InitDataAttrs: No ChartData-Object available!" );
/*N*/
/*N*/   short nDataColCnt   = pChartData->GetColCount();
/*N*/   short nDataRowCnt   = pChartData->GetRowCount();

    /* For pie charts we need the maximum of both dimensions, because the pie interprets data a bit
     * odd: only the first series (data-row) is used for the pie.  Each segment is a data point of
     * this single series.  However to get the correct colors the array containing series attributes
     * is used instead of the data point attributes.  Thus for a 1x10 data we need 10 series
     * attributes although we have only one series.  However for 20x10 data we need 20 series
     * attributes, because we actually have 20 series although just the first one is visible.  But
     * we need those attributes in case the chart type is switched.
     */
/*N*/     short nCnt          = IsPieChart()
/*N*/         ? ::std::max( GetColCount(), GetRowCount())
/*N*/         : GetRowCount();
/*N*/
/*N*/     short i;
/*N*/
/*N*/   if (nCnt != nPieSegCount)
/*N*/   {
/*N*/       long *pOfs = new long[nCnt];
/*N*/
/*N*/       if (nPieSegCount > nCnt)
/*N*/           for (i = 0; i < nCnt; i++)
/*N*/               pOfs[i] = pPieSegOfs[i];
/*N*/       else
/*N*/       {
/*N*/           for (i = 0; i < nPieSegCount; i++)
/*N*/               pOfs[i] = pPieSegOfs[i];
/*N*/           for (; i < nCnt; i++)
/*N*/               pOfs[i] = 0;
/*N*/       }
/*N*/
/*N*/       delete[] pPieSegOfs;
/*N*/       pPieSegOfs = pOfs;
/*N*/       nPieSegCount = nCnt;
/*N*/   }
/*N*/
/*N*/
/*N*/   long nRowListCnt ;
/*N*/   //regressattr
/*N*/   nRowListCnt = aRegressAttrList.size();
/*N*/
/*N*/   DBG_ASSERT( pDefaultColors, "invalid default colors" );
/*N*/   size_t nNumDefCol = pDefaultColors->size();
/*N*/   DBG_ASSERT( nNumDefCol, "Empty Default Color List" );
/*N*/
/*N*/   if (nCnt != nRowListCnt)
/*N*/   {
/*N*/       if (nRowListCnt > nCnt)
/*N*/       {
                for ( i = nCnt; i < nRowListCnt; ++i )
                {
                    ItemSetList::iterator it = aRegressAttrList.begin();
                    ::std::advance( it, nCnt );
                    delete *it;
                    aRegressAttrList.erase( it );
                }
/*N*/       }
/*N*/       else
/*N*/       {
/*N*/           for (i = nRowListCnt; i < nCnt; i++)
/*N*/           {
/*N*/               SfxItemSet* pRegressAttr = new SfxItemSet(*pItemPool, nGridWhichPairs);
/*N*/               aRegressAttrList.push_back( pRegressAttr );
/*N*/
/*N*/               pRegressAttr->Put(XLineStyleItem(XLINE_SOLID));
/*N*/               pRegressAttr->Put(XLineWidthItem(100));
/*N*/               if( nNumDefCol != 0 )
/*N*/               {
/*N*/                   XColorEntry* pEntry = (*pDefaultColors)[ i % nNumDefCol ];
/*N*/                   pRegressAttr->Put(XLineColorItem(pEntry->GetName(),
/*N*/                                                    pEntry->GetColor()));
/*N*/               }
/*N*/               pRegressAttr->Put(XLineDashItem());
/*N*/               pRegressAttr->Put(XLineStartItem());
/*N*/               pRegressAttr->Put(XLineEndItem());
/*N*/               pRegressAttr->Put(XLineStartWidthItem());
/*N*/               pRegressAttr->Put(XLineEndWidthItem());
/*N*/               pRegressAttr->Put(XLineStartCenterItem());
/*N*/               pRegressAttr->Put(XLineEndCenterItem());
/*N*/               pRegressAttr->Put(XLineTransparenceItem());
/*N*/           }
/*N*/       }
/*N*/   }
/*N*/
/*N*/   //average attr
/*N*/   nRowListCnt = aAverageAttrList.size();
/*N*/   if (nCnt != nRowListCnt)
/*N*/   {
/*N*/       if (nRowListCnt > nCnt)
/*N*/       {
                for ( i = nCnt; i < nRowListCnt; ++i )
                {
                    ItemSetList::iterator it = aAverageAttrList.begin();
                    ::std::advance( it, nCnt );
                    delete *it;
                    aAverageAttrList.erase( it );
                }
/*N*/       }
/*N*/       else
/*N*/       {
/*N*/           for (i = nRowListCnt; i < nCnt; i++)
/*N*/           {
/*N*/               SfxItemSet* pAverageAttr = new SfxItemSet(*pItemPool, nGridWhichPairs);
/*N*/               aAverageAttrList.push_back( pAverageAttr );
/*N*/
/*N*/               pAverageAttr->Put(XLineStyleItem(XLINE_SOLID));
/*N*/               pAverageAttr->Put(XLineWidthItem(0));
/*N*/               if( nNumDefCol != 0 )
/*N*/               {
/*N*/                   XColorEntry* pEntry = (*pDefaultColors)[ i % nNumDefCol ];
/*N*/                   pAverageAttr->Put(XLineColorItem(pEntry->GetName(),
/*N*/                                                    pEntry->GetColor()));
/*N*/               }
/*N*/               pAverageAttr->Put(XLineDashItem());
/*N*/               pAverageAttr->Put(XLineStartItem());
/*N*/               pAverageAttr->Put(XLineEndItem());
/*N*/               pAverageAttr->Put(XLineStartWidthItem());
/*N*/               pAverageAttr->Put(XLineEndWidthItem());
/*N*/               pAverageAttr->Put(XLineStartCenterItem());
/*N*/               pAverageAttr->Put(XLineEndCenterItem());
/*N*/               pAverageAttr->Put(XLineTransparenceItem());
/*N*/           }
/*N*/       }
/*N*/   }
/*N*/
/*N*/   //error attr
/*N*/   nRowListCnt = (short)aErrorAttrList.size();
/*N*/   if (nCnt != nRowListCnt)
/*N*/   {
/*N*/       if (nRowListCnt > nCnt)
/*N*/       {
                for ( i = nCnt; i < nRowListCnt; ++i )
                {
                    ItemSetList::iterator it = aErrorAttrList.begin();
                    ::std::advance( it, nCnt );
                    delete *it;
                    aErrorAttrList.erase( it );
                }
/*N*/       }
/*N*/       else
/*N*/       {
/*N*/           for (i = nRowListCnt; i < nCnt; i++)
/*N*/           {
/*N*/               SfxItemSet* pErrorAttr = new SfxItemSet(*pItemPool, nGridWhichPairs);
/*N*/               aErrorAttrList.push_back( pErrorAttr );
/*N*/
/*N*/               pErrorAttr->Put(XLineStyleItem(XLINE_SOLID));
/*N*/               pErrorAttr->Put(XLineWidthItem(0));
/*N*/               pErrorAttr->Put(XLineColorItem(String(), RGBColor(COL_BLACK)));
/*N*/               pErrorAttr->Put(XLineDashItem());
/*N*/               pErrorAttr->Put(XLineStartItem());
/*N*/               pErrorAttr->Put(XLineEndItem());
/*N*/               pErrorAttr->Put(XLineStartWidthItem());
/*N*/               pErrorAttr->Put(XLineEndWidthItem());
/*N*/               pErrorAttr->Put(XLineStartCenterItem());
/*N*/               pErrorAttr->Put(XLineEndCenterItem());
/*N*/               pErrorAttr->Put(XLineTransparenceItem());
/*N*/           }
/*N*/       }
/*N*/   }
/*N*/
/*N*/   // Point-Attr
/*N*/   long nPointCnt      = nDataColCnt * nDataRowCnt;
/*N*/   long nPointListCnt  = aDataPointAttrList.size();
/*N*/   if (nPointCnt != nPointListCnt)
/*N*/   {
/*N*/       if (nPointListCnt > nPointCnt)
/*N*/       {
/*N*/           while (nPointListCnt-- > nPointCnt)
/*N*/           {
                    ItemSetList::iterator it = aDataPointAttrList.begin();
                    ::std::advance( it, nPointCnt );
                    delete *it;
                    aDataPointAttrList.erase( it );
/*N*/           }
/*N*/       }
/*N*/       else for (long ii = nPointListCnt; ii < nPointCnt; ii++)
/*N*/            aDataPointAttrList.push_back( NULL );
/*N*/   }
/*N*/
/*N*/   // Switch-Point-Attr
/*N*/   nPointListCnt = aSwitchDataPointAttrList.size();
/*N*/   if (nPointCnt != nPointListCnt)
/*N*/   {
/*N*/       if (nPointListCnt > nPointCnt)
/*N*/       {
/*N*/           while (nPointListCnt-- > nPointCnt)
/*N*/           {
                    ItemSetList::iterator it = aSwitchDataPointAttrList.begin();
                    ::std::advance( it, nPointCnt );
                    delete *it;
                    aSwitchDataPointAttrList.erase( it );
/*N*/           }
/*N*/       }
/*N*/       else for (long iii = nPointListCnt; iii < nPointCnt; iii++)
/*N*/           aSwitchDataPointAttrList.push_back( NULL );
/*N*/   }
/*N*/
/*N*/   //row attr
/*N*/   nRowListCnt = (short)aDataRowAttrList.size();
/*N*/   if (nCnt != nRowListCnt)
/*N*/   {
/*N*/       if (nRowListCnt > nCnt)
/*N*/       {
/*N*/           //Jetzt darf erst der Ueberhang geloescht werden:
                for ( i = nCnt; i < nRowListCnt; ++i )
                {
                    ItemSetList::iterator it = aDataRowAttrList.begin();
                    ::std::advance( it, nCnt );
                    delete *it;
                    aDataRowAttrList.erase( it );
                }
/*N*/       }
/*N*/       else
/*N*/       {
/*N*/             bool bIsCombiChart =
/*N*/                 ( CHSTYLE_2D_LINE_COLUMN         ==  eChartStyle ) ||
/*N*/                 ( CHSTYLE_2D_LINE_STACKEDCOLUMN  ==  eChartStyle );
/*N*/
/*N*/             for (i = nRowListCnt; i < nCnt; i++)
/*N*/           {
/*N*/               SfxItemSet* pDataRowAttr = new SfxItemSet(*pItemPool, nRowWhichPairs);
/*N*/               aDataRowAttrList.push_back( pDataRowAttr );
/*N*/               SetDefAttrRow(pDataRowAttr,i);
/*N*/
/*N*/               //  Change the defaults for lines in mixed line-column charts.
/*N*/               if( bIsCombiChart && IsLine( i ) )
/*N*/                 {
/*N*/                     pDataRowAttr->ClearItem (SCHATTR_STYLE_SYMBOL);
/*N*/                     pDataRowAttr->Put (XLineStyleItem (XLINE_SOLID));
/*N*/                     pDataRowAttr->Put (XLineWidthItem (0));
/*N*/                 }
/*N*/           }
/*N*/             if( ! bIsCombiChart )
/*N*/                 SetupLineColors( SETLINES_FILLCOLOR, nRowListCnt );
/*N*/       }
/*N*/   }
/*N*/ }


/*************************************************************************
|*
|* RowAttr neu setzen:
|*
\************************************************************************/
/*N*/ void ChartModel::SetDefAttrRow(SfxItemSet* pDataRowAttr,const long i)
/*N*/ {
/*N*/   DBG_ASSERT( pDefaultColors, "Invalid default color list" );
/*N*/   if( ! pDefaultColors )
/*N*/       return;
/*N*/
/*N*/   CHART_TRACE1( "ChartModel::SetDefAttrRow - Row #%ld", i );
/*N*/
/*N*/   size_t nNumDefCol = pDefaultColors->size();
/*N*/   DBG_ASSERT( nNumDefCol, "invalid default colors" );
/*N*/
/*N*/   pDataRowAttr->Put(*pDummyAttr);
/*N*/   if( nNumDefCol != 0 )
/*N*/   {
/*N*/       XColorEntry* pEntry = (*pDefaultColors)[ i % nNumDefCol ];
/*N*/       pDataRowAttr->Put(XFillColorItem(pEntry->GetName(),
/*N*/                                        pEntry->GetColor()));
/*N*/
/*N*/       if(IsLine(i)) //#54870# bei Linien defaultfarbe der Linie=FillColor
/*N*/       {
/*N*/           pDataRowAttr->Put(XLineColorItem(pEntry->GetName(),
/*N*/                                            pEntry->GetColor()));
/*N*/       }
/*N*/   }
/*N*/   else
/*N*/   {
/*?*/       pDataRowAttr->Put(XLineColorItem(String(), RGBColor(COL_BLACK)));
/*N*/   }
/*N*/ }
/*************************************************************************
|*
|* ggf. Demo-Chart erzeugen
|*
\************************************************************************/

/*N*/ void ChartModel::InitChartData(BOOL bNewTitles)
/*N*/ {
/*N*/   if (!pChartData)
/*N*/   {
/*N*/       SchMemChart* pMemChart = new SchMemChart(DEFAULT_COLCNT, DEFAULT_ROWCNT);
/*N*/
/*N*/       pMemChart->SetMainTitle(SchResId(STR_TITLE_MAIN).toString());
/*N*/       pMemChart->SetSubTitle(SchResId(STR_TITLE_SUB).toString());
/*N*/       pMemChart->SetXAxisTitle(SchResId(STR_DIAGRAM_TITLE_X_AXIS).toString());
/*N*/       pMemChart->SetYAxisTitle(SchResId(STR_DIAGRAM_TITLE_Y_AXIS).toString());
/*N*/       pMemChart->SetZAxisTitle(SchResId(STR_DIAGRAM_TITLE_Z_AXIS).toString());
/*N*/
/*N*/       for( short nCol = 0; nCol < DEFAULT_COLCNT; nCol++ )
/*N*/       {
/*N*/           pMemChart->SetColText( nCol, pMemChart->GetDefaultColumnText( nCol ));
/*N*/
/*N*/           for( short nRow = 0; nRow < DEFAULT_ROWCNT; nRow++ )
/*N*/           {
/*N*/               pMemChart->SetData( nCol, nRow, fDefaultArr[ nRow ][ nCol ] );
/*N*/               pMemChart->SetRowText( nRow, pMemChart->GetDefaultRowText( nRow ));
/*N*/           }
/*N*/       }
/*N*/
/*N*/       SetChartData(*pMemChart, bNewTitles);
/*N*/   }
/*N*/ }

/*************************************************************************
|*
|* Defaultfarben erstellen
|*
\************************************************************************/

/*N*/ void ChartModel::CreateDefaultColors ()
/*N*/ {
/*N*/   SchOptions* pOptions = SCH_MOD1()->GetSchOptions();
/*N*/   size_t nCount;
/*N*/   ColorData* pDefaultCol = NULL;
/*N*/
/*N*/   if( pOptions )
/*N*/   {
/*N*/       const SchColorTable& aDefCols = pOptions->GetDefaultColors();
/*N*/       nCount = aDefCols.Count();
/*N*/       pDefaultCol = new ColorData[ nCount ];
/*N*/       DBG_ASSERT( nCount == ROW_COLOR_COUNT, "Chart: dynamic default color array size not supported yet" );
/*N*/
/*N*/       for( size_t i=0; i<nCount; i++ )
/*N*/       {
/*N*/           pDefaultCol[ i ] = aDefCols.GetColorData( i );
/*N*/       }
/*N*/   }
/*N*/   else
/*N*/   {
/*?*/       nCount = ROW_COLOR_COUNT;
/*?*/       pDefaultCol = new ColorData[ nCount ];
/*?*/
/*?*/       pDefaultCol[  0 ]  = RGB_COLORDATA( 0x99,  0x99, 0xff );
/*?*/       pDefaultCol[  1 ]  = RGB_COLORDATA( 0x99,  0x33, 0x66 );
/*?*/       pDefaultCol[  2 ]  = RGB_COLORDATA( 0xff,  0xff, 0xcc );
/*?*/       pDefaultCol[  3 ]  = RGB_COLORDATA( 0xcc,  0xff, 0xff );
/*?*/       pDefaultCol[  4 ]  = RGB_COLORDATA( 0x66,  0x00, 0x66 );
/*?*/       pDefaultCol[  5 ]  = RGB_COLORDATA( 0xff,  0x80, 0x80 );
/*?*/       pDefaultCol[  6 ]  = RGB_COLORDATA( 0x00,  0x66, 0xcc );
/*?*/       pDefaultCol[  7 ]  = RGB_COLORDATA( 0xcc,  0xcc, 0xff );
/*?*/       pDefaultCol[  8 ]  = RGB_COLORDATA( 0x00,  0x00, 0x80 );
/*?*/       pDefaultCol[  9 ]  = RGB_COLORDATA( 0xff,  0x00, 0xff );
/*?*/       pDefaultCol[ 10 ]  = RGB_COLORDATA( 0x00,  0xff, 0xff );
/*?*/       pDefaultCol[ 11 ]  = RGB_COLORDATA( 0xff,  0xff, 0x00 );
/*N*/   }
/*N*/
/*N*/   // create colors from table if they exist otherwise copy default colors
/*N*/   pDefaultColors = new XColorEntryList;
/*N*/   Color aCol;
/*N*/
/*N*/   for( size_t i = 0; i < nCount; i++ )
/*N*/   {
/*N*/       aCol.SetColor( pDefaultCol[ i ] );
/*N*/       pDefaultColors->push_back( new XColorEntry( aCol, String() ) );
/*N*/   }
/*N*/
/*N*/   delete[] pDefaultCol;
/*N*/ }

/*************************************************************************
|*
|* Defaultfarben loeschen
|*
\************************************************************************/

void ChartModel::DestroyDefaultColors ()
{
    if (pDefaultColors)
    {
        for( size_t i = 0, n = pDefaultColors->size(); i < n; ++i )
            delete (*pDefaultColors)[ i ];
        pDefaultColors->clear();
        delete pDefaultColors;
    }
    pDefaultColors = 0;
}

/*************************************************************************
|*
|* Berechnung der Breite und der Hoehe eines Textes
|*
|*    Derzeit ist diese funktion totaler Muell: Im wesentlichen werden die Werte direkt gesetzt
|*    Hoehe = 2 * Texthoehe, Breite hoechstens MaximumWidth
|*
|* FG: Damit dies Funktioniert muss per SetText im Outliner (pOutliner)
|*     der Text schon gesetzt sein!
|*
\************************************************************************/
#define MAXLEGENDLINES 3  //#49908# #NACHTRAG#
#define CDEG2RAD(fAngle) (  (double)(fAngle)*F_PI/18000.0 )


/*N*/ void  ChartModel::SetTextAttributes   (SfxItemSet & rTextAttributes)
/*N*/ {
/*N*/   ULONG nParagraphCount = pOutliner->GetParagraphCount();
/*N*/   for (ULONG i=0; i<nParagraphCount; i++)
/*N*/       pOutliner->SetParaAttribs(i, rTextAttributes);
/*N*/ }


// bGetRotated sollte TRUE sein, wenn der verbrauchte Platz des Textes gemessen werden soll,
// und False, falls das TextRect berechnet und dann gedreht wird (sonst doppelte Drehung)
//  The flag bSetTextAttributes tells the method wether to set the given attributes to the
//  outliner.
/*N*/ Size ChartModel::CalcTextSizeOfOneText (SvxChartTextOrient eOrient,
/*N*/                                       SfxItemSet         &rTextAttr,
/*N*/                                       SdrOutliner        *pInOutliner,
/*N*/                                       long    MaxW,
/*N*/                                       BOOL    bGetRotated/*=FALSE*/,
/*N*/                                       BOOL    bUseTextAttributes)
/*N*/ {
/*N*/   long MaximumWidth=MaxW;
/*N*/
/*N*/   pInOutliner->SetUpdateMode (FALSE);
/*N*/   ULONG nParaCnt = pInOutliner->GetParagraphCount();
/*N*/
/*N*/       // FG: Jeder Absatz muss die Text-Attribute einzeln zugewiesen bekommen. (jedenfalls scheint es so)
/*N*/       //     Besser waere es dass fuer alle Absaetze auf einmal setzen zu koennen.
/*N*/   if (bUseTextAttributes)
/*N*/       SetTextAttributes (rTextAttr);
/*N*/
/*N*/   Size OldPaperSize = pInOutliner->GetPaperSize();
/*N*/
/*N*/   long nDegrees=GetTextRotation((SfxItemSet&)rTextAttr,eOrient);//#62531#
/*N*/   double fDeg(0.0), fSin(0.0),
/*N*/       fCos = 1;               // BM: Initialize Cos for if statement after if(nDegrees)-Block
/*N*/   BOOL bBreakOK=TRUE; //s.u.
/*N*/   if(nDegrees)
/*N*/   {
/*N*/       //TVM: In einigen Bereichen macht Umbruch keinen Sinn, da die Breite steigt (90 bzw.
/*N*/       //270 Grad ist der triviale Fall!) Der genaue Bereich ist hier nicht festgelegt,
/*N*/       //denn jeder andere Fall ist leider nicht trivial! (Abhängig davon, wieviele Zeichen
/*N*/       //und welche umgebrochen werden, 2 Beispiele:
/*N*/       // 1. nur ein 'i' wird umgebrochen    => Text wird fast immer breiter (ausser bei ~ 0 Grad)
/*N*/       // 2. der halbe Text wird umgebrochen => fast immer sinnvoll
/*N*/       //Die genaue Berechnung erfordert einen Test und ein Undo, wenn es breiter wird
/*N*/       //Im folgenden wird einfach die Mitte genommen -> 90% - Lösung
/*N*/       bBreakOK =!(   ((nDegrees > 4500) && (nDegrees < 13500))
/*N*/                   || ((nDegrees >22500) && (nDegrees < 31500)));
/*N*/
/*N*/       fDeg=CDEG2RAD(nDegrees);
/*N*/       fSin=fabs(sin(fDeg));
/*N*/       fCos=fabs(cos(fDeg));
/*N*/
/*N*/   }
/*N*/
/*N*/   if( MaximumWidth > 0 && fCos!=0) //Kein Umbruch bei 90 und 270 Grad oder Max<=0
/*N*/       pInOutliner->SetPaperSize( Size( MaximumWidth, 0 ) );
/*N*/
/*N*/
/*N*/   pInOutliner->SetUpdateMode (TRUE);
/*N*/   Size aSize = pInOutliner->CalcTextSize();
/*N*/   pInOutliner->SetUpdateMode (FALSE);
/*N*/   Size aRot(aSize);
/*N*/   if(nDegrees)
/*N*/   {
/*N*/       aRot.Width() = (long)( (double)aSize.Width()*fCos + (double)aSize.Height()*fSin );
/*N*/       aRot.Height()= (long)( (double)aSize.Width()*fSin + (double)aSize.Height()*fCos );
/*N*/   }
/*N*/
/*N*/       // FG: Diese Groesse wird nun veraendert, falls MaximumWidth > 0 und
/*N*/       //     aSize.Width() > MaximumWidth, dann wird umgebrochen, genau einmal.
/*N*/       //     Es kommen also hoechstens 2 Zeilen raus, der Rest wird dann abgeschnitten.
/*N*/       //     An dieser Stelle werden aber nur die Attribute berechnet.
/*N*/
/*N*/   if ((MaximumWidth > 0) && (eOrient != CHTXTORIENT_STACKED))
/*N*/   {
/*N*/ #ifdef DBG_UTIL
/*N*/       if(!(aRot.Width() <= MaximumWidth))
/*N*/           DBG_WARNING("ChartModel::CalcTextSizeOfOneText:Doch breiter?" );
/*N*/ #endif
/*N*/       ULONG nLines = 0;
/*N*/       for( USHORT n = 0; n < pInOutliner->GetParagraphCount(); n++ )
/*N*/       {
/*N*/           nLines += pInOutliner->GetLineCount( n );
/*N*/       }
/*N*/
/*N*/       // Silbentrennung nur bei >MAXLEGENDLINES Zeilen oder einem zu langen wort...
/*N*/       if ( bBreakOK
/*N*/           && (    ( nLines > MAXLEGENDLINES )
/*N*/                || (    ( nLines >= 2 )
/*N*/                     && ( nParaCnt == 1 )
/*N*/                     && ( pInOutliner->GetText( pInOutliner->GetParagraph( 0 ) ).Search( ' ' )
/*N*/                                                            == STRING_NOTFOUND )
/*N*/                    )
/*N*/             )
/*N*/          )
/*N*/       {
/*?*/           ULONG nLclParaCnt = pInOutliner->GetParagraphCount();
/*?*/
/*?*/           for (ULONG i = 0; i < nLclParaCnt; i++)
/*?*/           {
/*?*/               // Stets Silbentrennung
/*?*/               SfxItemSet aAttr(pInOutliner->GetParaAttribs(i));
/*?*/               aAttr.Put( SfxBoolItem(EE_PARA_HYPHENATE, TRUE) );
/*?*/               pInOutliner->SetParaAttribs(i, aAttr);
/*?*/           }
/*N*/       }
/*N*/   }
/*N*/
/*N*/   pInOutliner->SetPaperSize(OldPaperSize);
/*N*/   pInOutliner->SetUpdateMode (TRUE);
/*N*/
/*N*/   return (bGetRotated && nDegrees) ? aRot : aSize;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
