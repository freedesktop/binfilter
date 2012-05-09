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

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/PosSize.hpp>

#include "svdpagv.hxx"
#include "svdpage.hxx"
#include "svdvmark.hxx"
#include "svdio.hxx"
#include "svdview.hxx"
#include "svdograf.hxx"
#include "svdouno.hxx"

#include <bf_svtools/whiter.hxx>
#include <bf_svtools/style.hxx>

#include "xoutx.hxx"

namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ FrameAnimator::FrameAnimator(SdrView& rNewView):
/*N*/   rView(rNewView)
/*N*/ {
/*N*/   aTim.SetTimeoutHdl(LINK(this,FrameAnimator,Hdl));
/*N*/   aTim.SetTimeout(50);
/*N*/   pOut=NULL;
/*N*/ }

IMPL_LINK(FrameAnimator,Hdl,AutoTimer*,EMPTYARG) { return 0; } // DBG_BF_ASSERT

/*N*/ SdrViewWinRec::SdrViewWinRec(OutputDevice* pW)
/*N*/ : pWin(pW),
/*N*/   bXorVisible(FALSE)
/*N*/ {
/*N*/ }

/*N*/ SdrViewWinRec::~SdrViewWinRec()
/*N*/ {
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrViewWinList::Clear()
/*N*/ {
/*N*/   USHORT nAnz=GetCount();
/*N*/   for (USHORT i=0; i<nAnz; i++) {
/*N*/       delete GetObject(i);
/*N*/   }
/*N*/   aList.Clear();
/*N*/ }

/*N*/ USHORT SdrViewWinList::Find(OutputDevice* pW) const
/*N*/ {
/*N*/   USHORT nAnz=GetCount();
/*N*/   USHORT nRet=SDRVIEWWIN_NOTFOUND;
/*N*/   for (USHORT nNum=0; nNum<nAnz && nRet==SDRVIEWWIN_NOTFOUND; nNum++) {
/*N*/       if (GetObject(nNum)->pWin==pW) nRet=nNum;
/*N*/   }
/*N*/   return nRet;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ TYPEINIT1( SvxViewHint, SfxSimpleHint );
/*N*/
/*N*/ SvxViewHint::SvxViewHint( ULONG _nId ) :
/*N*/     SfxSimpleHint( _nId )
/*N*/ {
/*N*/ }

/*N*/ TYPEINIT2(SdrPaintView,SfxListener,SfxRepeatTarget);

/*N*/ DBG_NAME(SdrPaintView)

/*N*/ void SdrPaintView::ImpClearVars()
/*N*/ {
/*N*/   pXOut=NULL;
/*N*/   bLayerSortedRedraw=FALSE;
/*N*/   bPageVisible=TRUE;
/*N*/   bBordVisible=TRUE;
/*N*/   bGridVisible=TRUE;
/*N*/   bGridFront  =FALSE;
/*N*/   bHlplVisible=TRUE;
/*N*/   bHlplFront  =TRUE;
/*N*/   bGlueVisible=FALSE;
/*N*/   bSwapAsynchron=FALSE;
/*N*/   bPrintPreview=FALSE;
/*N*/   bLineDraft=FALSE;
/*N*/   bFillDraft=FALSE;
/*N*/   bGrafDraft=FALSE;
/*N*/   bTextDraft=FALSE;
/*N*/
/*N*/   eAnimationMode = SDR_ANIMATION_ANIMATE;
/*N*/
/*N*/   pDefaultStyleSheet=NULL;
/*N*/   bSomeObjChgdFlag=FALSE;
/*N*/
/*N*/   bMasterBmp=FALSE;
/*N*/
/*N*/   nGraphicManagerDrawMode = GRFMGR_DRAW_STANDARD;
/*N*/
/*N*/   aComeBackTimer.SetTimeout(1);
/*N*/   aComeBackTimer.SetTimeoutHdl(LINK(this,SdrPaintView,ImpComeBackHdl));
/*N*/   aAfterPaintTimer.SetTimeout(1);
/*N*/   aAfterPaintTimer.SetTimeoutHdl(LINK(this,SdrPaintView,ImpAfterPaintHdl));
/*N*/   aUserMarkerAnimator.SetTimeout(50);
/*N*/   aUserMarkerAnimator.SetTimeoutHdl(LINK(this,SdrPaintView,ImpUserMarkerAnimatorHdl));
/*N*/
/*N*/   String aNam;    // System::GetUserName() just return an empty string
/*N*/
/*N*/   if (pMod)
/*N*/       SetDefaultStyleSheet(pMod->GetDefaultStyleSheet(), TRUE);
/*N*/
/*N*/   aNam.ToUpperAscii();
/*N*/
/*N*/   maGridColor = Color( COL_BLACK );
/*N*/ }


/*N*/ SdrPaintView::SdrPaintView(SdrModel* pModel1, OutputDevice* pOut):
/*N*/   aPagV(1024,16,16),
/*N*/   aPagHide(1024,16,16),
/*N*/   aAni(*(SdrView*)this),
/*N*/   aDefaultAttr(pModel1->GetItemPool()),
/*N*/   aUserMarkers(1024,16,16)
/*N*/ {
/*N*/   DBG_CTOR(SdrPaintView,NULL);
/*N*/   pMod=pModel1;
/*N*/   ImpClearVars();
/*N*/   if (pOut!=NULL) AddWin(pOut);
/*N*/   pXOut=new ExtOutputDevice(pOut);
/*N*/
/*N*/   StartListening( maColorConfig );
/*N*/   onChangeColorConfig();
/*N*/ }

/*N*/ SdrPaintView::~SdrPaintView()
/*N*/ {
/*N*/   DBG_DTOR(SdrPaintView,NULL);
/*N*/   aAfterPaintTimer.Stop();
/*N*/
/*N*/   EndListening( maColorConfig );
/*N*/
/*N*/   ClearAll();
/*N*/   if (pXOut!=NULL) {
/*N*/       delete pXOut;
/*N*/   }
/*N*/   USHORT nAnz=ImpGetUserMarkerCount();
/*N*/   for (USHORT nNum=0; nNum<nAnz; nNum++) {
/*?*/       SdrViewUserMarker* pUM=ImpGetUserMarker(nNum);
/*?*/       pUM->pView=NULL; // Weil's mich gleich nichtmehr gibt.
/*N*/   }
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrPaintView::SFX_NOTIFY(SfxBroadcaster& /*rBC*/, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType)
/*N*/ {
/*N*/   BOOL bObjChg=!bSomeObjChgdFlag; // TRUE= auswerten fuer ComeBack-Timer
/*N*/   if ( bObjChg ) {
/*N*/       SdrHint* pSdrHint=PTR_CAST(SdrHint,&rHint);
/*N*/       if (pSdrHint!=NULL) {
/*N*/           SdrHintKind eKind=pSdrHint->GetKind();
/*N*/           if (eKind==HINT_OBJCHG || eKind==HINT_OBJINSERTED || eKind==HINT_OBJREMOVED) {
/*N*/               if (bObjChg) {
/*N*/                   bSomeObjChgdFlag=TRUE;
/*N*/                   aComeBackTimer.Start();
/*N*/               }
/*N*/           }
/*N*/           if (eKind==HINT_PAGEORDERCHG) {
/*N*/               const SdrPage* pPg=pSdrHint->GetPage();
/*N*/               if (!pPg->IsInserted()) { // aha, Seite geloescht: also hiden
/*N*/                   USHORT nAnz=GetPageViewCount();
/*N*/                   USHORT nv;
/*N*/                   for (nv=nAnz; nv>0;) {
/*?*/                       nv--;
/*?*/                       SdrPageView* pPV=GetPageViewPvNum(nv);
/*?*/                       if (pPV->GetPage()==pPg) {
/*?*/                           HidePagePvNum(nv);
/*N*/                       }
/*N*/                   }
/*N*/               }
/*N*/           }
/*N*/       }
/*N*/   }
/*N*/
/*N*/   if( rHint.ISA( SfxSimpleHint ) && ( (SfxSimpleHint&) rHint ).GetId() == SFX_HINT_COLORS_CHANGED )
/*N*/   {
/*?*/       onChangeColorConfig();
/*?*/       InvalidateAllWin();
/*N*/   }
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ IMPL_LINK_INLINE_START(SdrPaintView,ImpComeBackHdl,Timer*,EMPTYARG)
/*N*/ {
/*N*/   if (bSomeObjChgdFlag) {
/*N*/       bSomeObjChgdFlag=FALSE;
/*N*/       ModelHasChanged();
/*N*/   }
/*N*/   return 0;
/*N*/ }

/*N*/ IMPL_LINK_INLINE_END(SdrPaintView,ImpComeBackHdl,Timer*,pTimer)

////////////////////////////////////////////////////////////////////////////////////////////////////

IMPL_LINK(SdrPaintView,ImpAfterPaintHdl,Timer*,EMPTYARG) { return 0; } // DBG_BF_ASSERT

/*N*/ void SdrPaintView::ModelHasChanged()
/*N*/ {
/*N*/   // Auch alle PageViews benachrichtigen
/*N*/   USHORT nv;
/*N*/   USHORT nAnz;
/*N*/   nAnz=GetPageViewCount();
/*N*/   for (nv=nAnz; nv>0;) {
/*N*/       nv--;
/*N*/       SdrPageView* pPV=GetPageViewPvNum(nv);
/*N*/       if (!pPV->GetPage()->IsInserted()) {
/*?*/           HidePage(pPV);
/*N*/       }
/*N*/   }
/*N*/   nAnz=GetPageViewCount();
/*N*/   for (nv=0; nv<nAnz; nv++) {
/*N*/       SdrPageView* pPV=GetPageViewPvNum(nv);
/*N*/       pPV->ModelHasChanged();
/*N*/   }
/*N*/   nAnz=GetPageHideCount();
/*N*/   for (nv=0; nv<nAnz; nv++) {
/*?*/       SdrPageView* pPV=GetPageHidePvNum(nv);
/*?*/       pPV->ModelHasChanged();
/*N*/   }
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

IMPL_LINK_INLINE_START(SdrPaintView,ImpUserMarkerAnimatorHdl,AutoTimer*,EMPTYARG) { return 0; } // DBG_BF_ASSERT

/*N*/ IMPL_LINK_INLINE_END(SdrPaintView,ImpUserMarkerAnimatorHdl,AutoTimer*,pTimer)

/*N*/ void SdrPaintView::ImpInsertUserMarker(SdrViewUserMarker* pMarker)
/*N*/ {
/*N*/   aUserMarkers.Insert(pMarker);
/*N*/   ImpCheckMarkerAnimator();
/*N*/ }
/*N*/
/*N*/ void SdrPaintView::ImpRemoveUserMarker(SdrViewUserMarker* pMarker)
/*N*/ {
/*N*/   aUserMarkers.Remove(pMarker);
/*N*/   ImpCheckMarkerAnimator();
/*N*/ }
/*N*/
/*N*/ void SdrPaintView::ImpCheckMarkerAnimator()
/*N*/ {
/*N*/   BOOL bNeed=FALSE;
/*N*/   USHORT nAnz=ImpGetUserMarkerCount();
/*N*/   for (USHORT nNum=0; nNum<nAnz && !bNeed; nNum++) {
/*N*/       SdrViewUserMarker* pUM=ImpGetUserMarker(nNum);
/*N*/       bNeed=pUM->IsAnimate();
/*N*/   }
/*N*/   if (bNeed) aUserMarkerAnimator.Start();
/*N*/   else aUserMarkerAnimator.Stop();
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrPaintView::ClearPageViews()
/*N*/ {
/*N*/   BrkAction();
/*N*/   for (USHORT nv=0; nv<GetPageViewCount(); nv++) {
/*N*/       SdrPageView* pPV=GetPageViewPvNum(nv);
/*N*/       InvalidateAllWin(pPV->GetPageRect());
/*N*/       delete pPV;
/*N*/   }
/*N*/   aPagV.Clear();
/*N*/ }

/*N*/ void SdrPaintView::ClearHideViews()
/*N*/ {
/*N*/   for (USHORT nh=0; nh<GetPageHideCount(); nh++) {
/*?*/       SdrPageView* pPV=GetPageHidePvNum(nh);
/*?*/       delete pPV;
/*N*/   }
/*N*/   aPagHide.Clear();
/*N*/ }

/*N*/ void SdrPaintView::Clear()
/*N*/ {
/*N*/   ClearPageViews();
/*N*/   ClearHideViews();
/*N*/ }

/*N*/ void SdrPaintView::ClearAll()
/*N*/ {
/*N*/   ClearPageViews();
/*N*/   ClearHideViews();
/*N*/ }

/*N*/ SdrPageView* SdrPaintView::ShowPage(SdrPage* pPage, const Point& rOffs)
/*N*/ {
/*N*/   SdrPageView* pPV=NULL;
/*N*/   if (pPage!=NULL) {
/*N*/       SdrPageView* pTmpPV = GetPageView(pPage); // Evtl. jede Seite nur einmal!
/*N*/       if (pTmpPV==NULL) {
/*N*/           USHORT nPos=GetHiddenPV(pPage);   // War die schon mal da?
/*N*/           if ( !(nPos<GetPageHideCount()) ) {
/*N*/               pPV=new SdrPageView(pPage,rOffs,*((SdrView*)this));
/*N*/           }
/*N*/           if (pPV!=NULL) {
/*N*/               aPagV.Insert(pPV,CONTAINER_APPEND);
/*N*/               pPV->Show();
/*N*/           }
/*N*/       }
/*N*/   }
/*N*/   return pPV;
/*N*/ }

/*N*/ void SdrPaintView::HidePage(SdrPageView* pPV)
/*N*/ {
/*N*/   if (pPV!=NULL) {
/*N*/       ULONG nPos=aPagV.GetPos(pPV);
/*N*/       if (nPos!=CONTAINER_ENTRY_NOTFOUND) {
/*N*/           aPagV.Remove(nPos);
/*N*/           pPV->Hide();
/*N*/           delete pPV;
/*N*/       }
/*N*/   }
/*N*/ }


/*N*/ SdrPageView* SdrPaintView::GetPageView(const SdrPage* pPage) const
/*N*/ {
/*N*/   if (pPage==NULL) return NULL;
/*N*/   BOOL bWeiter=TRUE;
/*N*/   SdrPageView* pPV=NULL;
/*N*/   for (USHORT i=0; i<GetPageViewCount() && bWeiter; i++) {
/*?*/       pPV=GetPageViewPvNum(i);
/*?*/       bWeiter=(pPV->GetPage()!=pPage);
/*N*/   }
/*N*/   if (bWeiter) return NULL;
/*N*/   else return pPV;
/*N*/ }

/*N*/ USHORT SdrPaintView::GetHiddenPV(const SdrPage* pPage) const
/*N*/ {
/*N*/   BOOL bWeiter=TRUE;
/*N*/   USHORT i;
/*N*/   for (i=0; i<GetPageHideCount() && bWeiter;) {
/*?*/       SdrPageView* pPV=GetPageHidePvNum(i);
/*?*/       bWeiter=(pPV->GetPage()!=pPage);
/*?*/       if (bWeiter) i++;
/*N*/   }
/*N*/   return i;
/*N*/ }


////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrPaintView::AddWin(OutputDevice* pWin1)
/*N*/ {
/*N*/   SdrViewWinRec* pWinRec=new SdrViewWinRec(pWin1);
/*N*/   pWinRec->bXorVisible=TRUE; // Normalerweise ist Xor nicht unterdrueckt
/*N*/   aWinList.Insert(pWinRec);
/*N*/   for (USHORT i=0; i<GetPageViewCount(); i++) {
/*?*/       GetPageViewPvNum(i)->AddWin(pWin1);
/*N*/   }
/*N*/ }

/*N*/ void SdrPaintView::DelWin(OutputDevice* pWin1)
/*N*/ {
/*N*/   USHORT nPos=aWinList.Find(pWin1);
/*N*/   if (nPos!=SDRVIEWWIN_NOTFOUND) {
/*N*/       for (USHORT i=0; i<GetPageViewCount(); i++) {
/*N*/           GetPageViewPvNum(i)->DelWin(pWin1);
/*N*/       }
/*N*/       aWinList.Delete(nPos);
/*N*/   }
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrPaintView::InvalidateAllWin()
/*N*/ {
/*N*/   for (USHORT i=0; i<GetWinCount(); i++) {
/*N*/       OutputDevice* pOut=GetWin(i);
/*N*/       if (pOut->GetOutDevType()==OUTDEV_WINDOW) {
/*N*/           InvalidateOneWin(*(Window*)pOut);
/*N*/       }
/*N*/   }
/*N*/ }

/*N*/ void SdrPaintView::InvalidateAllWin(const Rectangle& rRect, BOOL bPlus1Pix)
/*N*/ {
/*N*/   USHORT nWinAnz=GetWinCount();
/*N*/   for (USHORT i=0; i<nWinAnz; i++) {
/*N*/       OutputDevice* pOut=GetWin(i);
/*N*/       if (pOut->GetOutDevType()==OUTDEV_WINDOW) {
/*N*/           Rectangle aRect(rRect);
/*N*/           if (bPlus1Pix) {
/*N*/               Size aPixSiz(1,1);
/*N*/               Size aSiz(pOut->PixelToLogic(aPixSiz));
/*N*/               aRect.Left  ()-=aSiz.Width();
/*N*/               aRect.Top   ()-=aSiz.Height();
/*N*/               aRect.Right ()+=aSiz.Width();
/*N*/               aRect.Bottom()+=aSiz.Height();
/*N*/           }
/*N*/           Point aOrg(pOut->GetMapMode().GetOrigin());
/*N*/           aOrg.X()=-aOrg.X(); aOrg.Y()=-aOrg.Y();
/*N*/           Rectangle aOutRect(aOrg,pOut->GetOutputSize());
/*N*/           if (aRect.IsOver(aOutRect)) {
/*N*/               InvalidateOneWin(*(Window*)pOut,aRect);
/*N*/           }
/*N*/       }
/*N*/   }
/*N*/ }

/*N*/ void SdrPaintView::InvalidateOneWin(Window& rWin)
/*N*/ {
/*N*/   rWin.Invalidate();
/*N*/ }

/*N*/ void SdrPaintView::InvalidateOneWin(Window& rWin, const Rectangle& rRect)
/*N*/ {
/*N*/   rWin.Invalidate(rRect);
/*N*/ }


/*N*/ void SdrPaintView::LeaveAllGroup()
/*N*/ {
/*N*/   for (USHORT nv=0; nv<GetPageViewCount(); nv++) {
/*N*/       SdrPageView* pPV=GetPageViewPvNum(nv);
/*N*/       pPV->LeaveAllGroup();
/*N*/   }
/*N*/ }

/*N*/ void SdrPaintView::SetMasterPagePaintCaching( BOOL bOn )
/*N*/ {
/*N*/   bMasterBmp = bOn;
/*N*/ }


/*N*/ void SdrPaintView::SetDefaultStyleSheet(SfxStyleSheet* pStyleSheet, BOOL bDontRemoveHardAttr)
/*N*/ {
/*N*/   pDefaultStyleSheet=pStyleSheet;
/*N*/   if (pStyleSheet!=NULL && !bDontRemoveHardAttr) {
/*?*/       SfxWhichIter aIter(pStyleSheet->GetItemSet());
/*?*/       USHORT nWhich=aIter.FirstWhich();
/*?*/       while (nWhich!=0) {
/*?*/           if (pStyleSheet->GetItemSet().GetItemState(nWhich,TRUE)==SFX_ITEM_SET) {
/*?*/               aDefaultAttr.ClearItem(nWhich);
/*?*/           }
/*?*/           nWhich=aIter.NextWhich();
/*?*/       }
/*N*/   }
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrPaintView::WriteRecords(SvStream& rOut) const
/*N*/ {
/*N*/   {
/*N*/       SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_VIEWPAGEVIEWS);
/*N*/       USHORT nv;
/*N*/       for (nv=0; nv<GetPageViewCount(); nv++) {
/*N*/           SdrPageView* pPV=GetPageViewPvNum(nv);
/*N*/           if (pPV->GetPage()->IsInserted()) {
/*?*/               rOut<<*pPV;
/*N*/           }
/*N*/       }
/*N*/       for (nv=0; nv<GetPageHideCount(); nv++) {
/*?*/           SdrPageView* pPV=GetPageHidePvNum(nv);
/*?*/           if (pPV->GetPage()->IsInserted()) {
/*?*/               rOut<<*pPV;
/*N*/           }
/*N*/       }
/*N*/   } {
/*N*/       SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_VIEWVISIELEM);
/*N*/       rOut<<BOOL(bLayerSortedRedraw);
/*N*/       rOut<<BOOL(bPageVisible);
/*N*/       rOut<<BOOL(bBordVisible);
/*N*/       rOut<<BOOL(bGridVisible);
/*N*/       rOut<<BOOL(bGridFront);
/*N*/       rOut<<BOOL(bHlplVisible);
/*N*/       rOut<<BOOL(bHlplFront);
/*N*/       rOut<<BOOL(bGlueVisible);
/*N*/       rOut<<aGridBig;
/*N*/       rOut<<aGridFin;
/*N*/       rOut<<aGridWdtX;
/*N*/       rOut<<aGridWdtY;
/*N*/       rOut<<aGridSubdiv;
/*N*/   } {
/*N*/       SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_VIEWAKTLAYER);
/*N*/
/*N*/       rOut.WriteUniOrByteString(aAktLayer, rOut.GetStreamCharSet());
/*N*/       rOut.WriteUniOrByteString(aMeasureLayer, rOut.GetStreamCharSet());
/*N*/   }
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ BOOL SdrPaintView::ReadRecord(const SdrIOHeader& /*rViewHead*/,
/*N*/   const SdrNamedSubRecord& rSubHead,
/*N*/   SvStream& rIn)
/*N*/ {
/*N*/   BOOL bRet=FALSE;
/*N*/   if (rSubHead.GetInventor()==SdrInventor) {
/*N*/       bRet=TRUE;
/*N*/       switch (rSubHead.GetIdentifier()) {
/*N*/           case SDRIORECNAME_VIEWPAGEVIEWS: {
/*N*/               while (rSubHead.GetBytesLeft()>0 && rIn.GetError()==0 && !rIn.IsEof()) {
/*N*/                   SdrPageView* pPV=new SdrPageView(NULL,Point(),*(SdrView*)this);
/*N*/                   rIn>>*pPV;
/*N*/                   if (pPV->GetPage()!=NULL) {
/*N*/                       if (pPV->IsVisible()) {
/*N*/                           aPagV.Insert(pPV,CONTAINER_APPEND);
/*N*/                       } else aPagHide.Insert(pPV,CONTAINER_APPEND);
/*N*/                   } else {
/*N*/                       OSL_FAIL("SdrPaintView::ReadRecord(): Seite der PageView nicht gefunden");
/*N*/                       delete pPV;
/*N*/                   }
/*N*/               }
/*N*/           } break;
/*N*/
/*N*/           case SDRIORECNAME_VIEWVISIELEM:
/*N*/           {
/*N*/               BOOL bTemp;
/*N*/
/*N*/               rIn>>bTemp; bLayerSortedRedraw=bTemp;
/*N*/               rIn>>bTemp; bPageVisible      =bTemp;
/*N*/               rIn>>bTemp; bBordVisible      =bTemp;
/*N*/               rIn>>bTemp; bGridVisible      =bTemp;
/*N*/               rIn>>bTemp; bGridFront        =bTemp;
/*N*/               rIn>>bTemp; bHlplVisible      =bTemp;
/*N*/               rIn>>bTemp; bHlplFront        =bTemp;
/*N*/               rIn>>bTemp; bGlueVisible      =bTemp;
/*N*/               rIn>>aGridBig;
/*N*/               rIn>>aGridFin;
/*N*/
/*N*/               if(rSubHead.GetBytesLeft() > 0)
/*N*/               {
/*N*/                   rIn >> aGridWdtX;
/*N*/                   rIn >> aGridWdtY;
/*N*/                   rIn >> aGridSubdiv;
/*N*/               }
/*N*/               break;
/*N*/           }
/*N*/           case SDRIORECNAME_VIEWAKTLAYER:
/*N*/           {
/*N*/               aAktLayer = rIn.ReadUniOrByteString(rIn.GetStreamCharSet());
/*N*/
/*N*/               if(rSubHead.GetBytesLeft() > 0)
/*N*/               {
/*N*/                   aMeasureLayer = rIn.ReadUniOrByteString(rIn.GetStreamCharSet());
/*N*/               }
/*N*/               break;
/*N*/           }
/*N*/
/*N*/           default:
/*N*/               bRet = FALSE;
/*N*/       }
/*N*/   }
/*N*/   return bRet;
/*N*/ }

/*N*/ void SdrPaintView::onChangeColorConfig()
/*N*/ {
/*N*/     SetGridColor( Color( maColorConfig.GetColorValue( DRAWGRID ).nColor ) );
/*N*/ }

/*N*/ void SdrPaintView::SetGridColor( Color aColor )
/*N*/ {
/*N*/   maGridColor = aColor;
/*N*/ }

// eof
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
