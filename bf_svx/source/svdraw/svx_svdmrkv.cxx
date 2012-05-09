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

#include "svdview.hxx"
#include "svdpagv.hxx"
#include "svdio.hxx"
#include "svdoole2.hxx"

#include "svdstr.hrc"
#include "svdopath.hxx"
#include "scene3d.hxx"

#include <svdovirt.hxx>

namespace binfilter {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MARKVIEW
//
////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrMarkView::ImpClearVars()
/*N*/ {
/*N*/   eDragMode=SDRDRAG_MOVE;
/*N*/   eEditMode=SDREDITMODE_EDIT;
/*N*/   bDesignMode=FALSE;
/*N*/   pMarkedObj=NULL;
/*N*/   pMarkedPV=NULL;
/*N*/   bForceFrameHandles=FALSE;
/*N*/   bPlusHdlAlways=FALSE;
/*N*/   nFrameHandlesLimit=50;
/*N*/   bEdgesOfMarkedNodesDirty=FALSE;
/*N*/   bMarkedObjRectDirty=FALSE;
/*N*/   bMarkedPointsRectsDirty=FALSE;
/*N*/   bMarkHdlWhenTextEdit=FALSE;
/*N*/ }

/*N*/ SdrMarkView::SdrMarkView(SdrModel* pModel1, OutputDevice* pOut):
/*N*/   SdrSnapView(pModel1,pOut),
/*N*/   aHdl(this)
/*N*/ {
/*N*/   ImpClearVars();
/*N*/   StartListening(*pModel1);
/*N*/ }

/*N*/ void SdrMarkView::SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType)
/*N*/ {
/*N*/   SdrHint* pSdrHint=PTR_CAST(SdrHint,&rHint);
/*N*/   if (pSdrHint!=NULL) {
/*N*/       SdrHintKind eKind=pSdrHint->GetKind();
/*N*/       if (eKind==HINT_OBJLISTCLEARED) {
/*N*/           USHORT nAnz=GetPageViewCount();
/*N*/           BOOL bMLChgd=FALSE;
/*N*/           for (USHORT nv=0; nv<nAnz; nv++) {
/*N*/               SdrPageView* pPV=GetPageViewPvNum(nv);
/*N*/               if (pPV->GetObjList()==pSdrHint->GetObjList()) {
/*?*/                   aMark.DeletePageView(*pPV);
/*?*/                   bMLChgd=TRUE;
/*N*/               }
/*N*/           }
/*N*/           if (bMLChgd) MarkListHasChanged();
/*N*/       }
/*N*/       if (eKind==HINT_OBJCHG || eKind==HINT_OBJINSERTED || eKind==HINT_OBJREMOVED)
/*N*/       {
/*N*/           bMarkedObjRectDirty=TRUE;
/*N*/           bMarkedPointsRectsDirty=TRUE;
/*N*/       }
/*N*/   }
/*N*/   SdrSnapView::SFX_NOTIFY(rBC,rBCType,rHint,rHintType);
/*N*/ }

/*N*/ void SdrMarkView::ModelHasChanged()
/*N*/ {
/*N*/   SdrPaintView::ModelHasChanged();
/*N*/   aMark.SetNameDirty();
/*N*/   bMarkedObjRectDirty=TRUE;
/*N*/   bMarkedPointsRectsDirty=TRUE;
/*N*/   // Es sind beispielsweise Obj markiert und aMark ist Sorted.
/*N*/   // In einer anderen View 2 wird die ObjOrder veraendert
/*N*/   // (z.B. MovToTop()). Dann ist Neusortieren der MarkList erforderlich.
/*N*/   aMark.SetUnsorted();
/*N*/   aMark.ForceSort();
/*N*/   SdrView* pV=(SdrView*)this;
/*N*/   if (pV!=NULL && !pV->IsInsObjPoint()) { // an dieser Stelle habe ich ein ziemliches Problem !!!
/*N*/       AdjustMarkHdl();
    }
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ BOOL SdrMarkView::IsAction() const
/*N*/ {
/*N*/   return SdrSnapView::IsAction();
/*N*/ }

/*N*/ void SdrMarkView::BrkAction()
/*N*/ {
/*N*/   SdrSnapView::BrkAction();
/*N*/ }


/*N*/ void SdrMarkView::ToggleShownXor(OutputDevice* pOut, const Region* pRegion) const
/*N*/ {
/*N*/   SdrSnapView::ToggleShownXor(pOut,pRegion);
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrMarkView::ClearPageViews()
/*N*/ {
/*N*/   UnmarkAllObj();
/*N*/   SdrSnapView::ClearPageViews();
/*N*/ }

/*N*/ void SdrMarkView::HidePage(SdrPageView* pPV)
/*N*/ {
/*N*/   if(pPV)
/*N*/   {
/*N*/       // break all creation actions when hiding page (#75081#)
/*N*/       BrkAction();
/*N*/
/*N*/       // Alle Markierungen dieser Seite verwerfen
/*N*/       BOOL bMrkChg(aMark.DeletePageView(*pPV));
/*N*/       SdrSnapView::HidePage(pPV);
/*N*/
/*N*/       if(bMrkChg)
/*N*/       {
/*?*/           MarkListHasChanged();
/*?*/           AdjustMarkHdl();
/*N*/       }
/*N*/   }
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrMarkView::ShowMarkHdl(OutputDevice* /*pOut*/, BOOL /*bNoRefHdl*/)
/*N*/ {
/*N*/ }


/*N*/ void SdrMarkView::HideMarkHdl(OutputDevice* /*pOut*/, BOOL /*bNoRefHdl*/)
/*N*/ {
/*N*/ }

/*N*/ BOOL SdrMarkView::ImpIsFrameHandles() const
/*N*/ {
/*N*/   ULONG nMarkAnz=aMark.GetMarkCount();
/*N*/   BOOL bFrmHdl=nMarkAnz>nFrameHandlesLimit || bForceFrameHandles;
/*N*/   BOOL bStdDrag=eDragMode==SDRDRAG_MOVE;
/*N*/   if (!bStdDrag && !bFrmHdl) {
/*?*/       // Grundsaetzlich erstmal alle anderen Dragmodi nur mit FrameHandles
/*?*/       bFrmHdl=TRUE;
/*?*/       if (eDragMode==SDRDRAG_ROTATE) {
/*?*/           // bei Rotate ObjOwn-Drag, wenn mind. 1 PolyObj
/*?*/           for (ULONG nMarkNum=0; nMarkNum<nMarkAnz && bFrmHdl; nMarkNum++) {
/*?*/               const SdrMark* pM=aMark.GetMark(nMarkNum);
/*?*/               const SdrObject* pObj=pM->GetObj();
/*?*/               bFrmHdl=!pObj->IsPolyObj();
/*?*/           }
/*?*/       }
/*N*/   }
/*N*/   return bFrmHdl;
/*N*/ }

/*N*/ void SdrMarkView::SetMarkHandles()
/*N*/ {
/*N*/   aHdl.Clear();
/*N*/   aHdl.SetRotateShear(eDragMode==SDRDRAG_ROTATE);
/*N*/   aHdl.SetDistortShear(eDragMode==SDRDRAG_SHEAR);
/*N*/   pMarkedObj=NULL;
/*N*/   pMarkedPV=NULL;
/*N*/   ULONG nMarkAnz=aMark.GetMarkCount();
/*N*/   if (nMarkAnz==1) {
/*?*/       pMarkedObj=aMark.GetMark(0)->GetObj();
/*N*/   }
/*N*/   BOOL bFrmHdl=ImpIsFrameHandles();
/*N*/   if (nMarkAnz>0) {
/*?*/       pMarkedPV=aMark.GetMark(0)->GetPageView();
/*?*/       for (ULONG nMarkNum=0; nMarkNum<nMarkAnz && (pMarkedPV!=NULL || !bFrmHdl); nMarkNum++) {
/*?*/           const SdrMark* pM=aMark.GetMark(nMarkNum);
/*?*/           if (pMarkedPV!=pM->GetPageView()) {
/*?*/               pMarkedPV=NULL;
/*?*/           }
/*N*/       }
/*N*/   }
/*N*/   if (bFrmHdl) {
/*N*/       Rectangle aRect(GetMarkedObjRect());
/*N*/   }

    // Drehpunkt/Spiegelachse
/*N*/   AddDragModeHdl(eDragMode);
/*N*/
/*N*/   // add custom handles (used by other apps, e.g. AnchorPos)
/*N*/   AddCustomHdl();
/*N*/ }

/*N*/ void SdrMarkView::AddCustomHdl()
/*N*/ {DBG_BF_ASSERT(0, "STRIP");
/*N*/ }

/*N*/ void SdrMarkView::AddDragModeHdl(SdrDragMode /*eMode*/)
/*N*/ {DBG_BF_ASSERT(0, "STRIP");
/*N*/ }


/*N*/ void SdrMarkView::CheckMarked()
/*N*/ {
/*N*/   bMarkedObjRectDirty = TRUE;
/*N*/ }

/*N*/ void SdrMarkView::SetMarkRects()
/*N*/ {
/*N*/   for (USHORT nv=0; nv<GetPageViewCount(); nv++) {
/*N*/       SdrPageView* pPV=GetPageViewPvNum(nv);
/*N*/       pPV->SetHasMarkedObj(aMark.TakeSnapRect(pPV,pPV->MarkSnap()));
/*N*/       aMark.TakeBoundRect(pPV,pPV->MarkBound());
/*N*/   }
/*N*/ }

/*N*/ void SdrMarkView::SetFrameHandles(BOOL bOn)
/*N*/ {
/*N*/   if (bOn!=bForceFrameHandles) {
/*N*/       BOOL bOld=ImpIsFrameHandles();
/*N*/       bForceFrameHandles=bOn;
/*N*/       BOOL bNew=ImpIsFrameHandles();
/*N*/       if (bNew!=bOld) {
/*N*/           AdjustMarkHdl(TRUE);
/*N*/           MarkListHasChanged();
/*N*/       }
/*N*/   }
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ BOOL SdrMarkView::IsObjMarkable(SdrObject* pObj, SdrPageView* pPV) const
/*N*/ {
/*N*/   if (pObj)
/*N*/   {
/*N*/       if (pObj->IsMarkProtect() ||
/*N*/           (!bDesignMode && pObj->IsUnoObj()))
/*N*/       {
/*N*/           // Objekt nicht selektierbar oder
/*N*/           // SdrUnoObj nicht im DesignMode
/*N*/           return FALSE;
/*N*/       }
/*N*/   }
/*N*/   return pPV!=NULL ? pPV->IsObjMarkable(pObj) : TRUE;
/*N*/ }

/*N*/ void SdrMarkView::MarkObj(SdrObject* pObj, SdrPageView* pPV, BOOL bUnmark, BOOL bImpNoSetMarkHdl)
/*N*/ {
/*N*/   if (pObj!=NULL && pPV!=NULL && IsObjMarkable(pObj, pPV)) {
/*N*/       BrkAction();
/*N*/       if (bUnmark)
            {
/*N*/           ULONG nPos=aMark.FindObject(pObj);
/*N*/           if (nPos!=CONTAINER_ENTRY_NOTFOUND) {
/*N*/           }
/*N*/       }
/*N*/       if (!bImpNoSetMarkHdl) {
/*N*/           MarkListHasChanged();
/*N*/           AdjustMarkHdl(TRUE);
/*N*/           if (!bSomeObjChgdFlag) {
/*N*/               // ShowMarkHdl kommt sonst mit dem AfterPaintTimer
/*N*/               ShowMarkHdl(NULL);
/*N*/           }
/*N*/       }
/*N*/   }
/*N*/ }

/*N*/ #define SDRSEARCH_IMPISMASTER 0x80000000 /* MasterPage wird gerade durchsucht */
/*N*/ SdrObject* SdrMarkView::ImpCheckObjHit(const Point& rPnt, USHORT nTol, SdrObject* pObj, SdrPageView* pPV, ULONG nOptions, const SetOfByte* pMVisLay) const
/*N*/ {
/*N*/   if ((nOptions & SDRSEARCH_IMPISMASTER) !=0 && pObj->IsNotVisibleAsMaster()) {
/*N*/       return NULL;
/*N*/   }
/*N*/   BOOL bCheckIfMarkable=(nOptions & SDRSEARCH_TESTMARKABLE)!=0;
/*N*/   BOOL bDeep=(nOptions & SDRSEARCH_DEEP)!=0;
/*N*/   BOOL bOLE=pObj->ISA(SdrOle2Obj);
/*N*/   SdrObject* pRet=NULL;
/*N*/   Point aPnt1(rPnt-pPV->GetOffset()); // rPnt auf PageView transformieren
/*N*/   Rectangle aRect(pObj->GetBoundRect());
/*N*/   USHORT nTol2=nTol;
/*N*/   // Doppelte Tolezanz fuer ein an dieser View im TextEdit befindliches Objekt
/*N*/   if (bOLE || pObj==((SdrObjEditView*)this)->GetTextEditObject()) nTol2*=2;
/*N*/   aRect.Left  ()-=nTol2; // Einmal Toleranz drauf fuer alle Objekte
/*N*/   aRect.Top   ()-=nTol2;
/*N*/   aRect.Right ()+=nTol2;
/*N*/   aRect.Bottom()+=nTol2;
/*N*/   if (aRect.IsInside(aPnt1)) {
/*N*/       if ((!bCheckIfMarkable || IsObjMarkable(pObj,pPV))) {
/*N*/           SdrObjList* pOL=pObj->GetSubList();
/*N*/           if (pOL!=NULL && pOL->GetObjCount()!=0) {
/*N*/               SdrObject* pTmpObj;
/*N*/                 // adjustment hit point for virtual objects.
/*N*/                 Point aPnt( rPnt );
/*N*/                 if ( pObj->ISA(SdrVirtObj) )
/*N*/                 {
/*N*/                     Point aOffset = static_cast<SdrVirtObj*>(pObj)->GetOffset();
/*N*/                     aPnt.Move( -aOffset.X(), -aOffset.Y() );
/*N*/                 }
/*N*/                 pRet=ImpCheckObjHit(aPnt,nTol,pOL,pPV,nOptions,pMVisLay,pTmpObj);
/*N*/           } else {
/*N*/               SdrLayerID nLay=pObj->GetLayer();
/*N*/               if (pPV->GetVisibleLayers().IsSet(nLay) &&
/*N*/                   (pMVisLay==NULL || pMVisLay->IsSet(nLay)))
/*N*/               {
/*N*/                   pRet=pObj->CheckHit(aPnt1,nTol2,&pPV->GetVisibleLayers());
/*N*/               }
/*N*/           }
/*N*/       }
/*N*/   }
/*N*/   if (!bDeep && pRet!=NULL) pRet=pObj;
/*N*/   return pRet;
/*N*/ }

/*N*/ SdrObject* SdrMarkView::ImpCheckObjHit(const Point& rPnt, USHORT nTol, SdrObjList* pOL, SdrPageView* pPV, ULONG nOptions, const SetOfByte* pMVisLay, SdrObject*& rpRootObj) const
/*N*/ {
/*N*/   BOOL bBack=(nOptions & SDRSEARCH_BACKWARD)!=0;
/*N*/   SdrObject* pRet=NULL;
/*N*/   rpRootObj=NULL;
/*N*/   if (pOL!=NULL) {
/*N*/       ULONG nObjAnz=pOL->GetObjCount();
/*N*/       ULONG nObjNum=bBack ? 0 : nObjAnz;
/*N*/       while (pRet==NULL && (bBack ? nObjNum<nObjAnz : nObjNum>0)) {
/*N*/           if (!bBack) nObjNum--;
/*N*/           SdrObject* pObj=pOL->GetObj(nObjNum);
/*N*/           pRet=ImpCheckObjHit(rPnt,nTol,pObj,pPV,nOptions,pMVisLay);
/*N*/           if (pRet!=NULL) rpRootObj=pObj;
/*N*/           if (bBack) nObjNum++;
/*N*/       }
/*N*/   }
/*N*/   return pRet;
/*N*/ }

/*N*/ void SdrMarkView::UnmarkAllObj(SdrPageView* pPV)
/*N*/ {
/*N*/   if (aMark.GetMarkCount()!=0) {
/*?*/       BrkAction();
/*?*/       if (pPV!=NULL) {
/*?*/           aMark.DeletePageView(*pPV);
/*?*/       } else {
/*?*/           aMark.Clear();
/*?*/       }
/*?*/       pMarkedObj=NULL;
/*?*/       pMarkedPV=NULL;
/*?*/       MarkListHasChanged();
/*?*/       AdjustMarkHdl(TRUE);
/*?*/   }
/*N*/ }


/*N*/ void SdrMarkView::AdjustMarkHdl(BOOL /*bRestraintPaint*/)
/*N*/ {
/*N*/   CheckMarked();
/*N*/   SetMarkRects();
/*N*/   SetMarkHandles();
/*N*/ }


/*N*/ const Rectangle& SdrMarkView::GetMarkedObjRect() const
/*N*/ {
/*N*/   if (bMarkedObjRectDirty) {
/*N*/       ((SdrMarkView*)this)->bMarkedObjRectDirty=FALSE;
/*N*/       Rectangle aRect;
/*N*/       for (ULONG nm=0; nm<aMark.GetMarkCount(); nm++) {
/*?*/           SdrMark* pM=aMark.GetMark(nm);
/*?*/           SdrObject* pO=pM->GetObj();
/*?*/           Rectangle aR1(pO->GetSnapRect());
/*?*/           aR1+=pM->GetPageView()->GetOffset();
/*?*/           if (aRect.IsEmpty()) aRect=aR1;
/*?*/           else aRect.Union(aR1);
/*N*/       }
/*N*/       ((SdrMarkView*)this)->aMarkedObjRect=aRect;
/*N*/   }
/*N*/   return aMarkedObjRect;
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrMarkView::MarkListHasChanged()
/*N*/ {
/*N*/   aMark.SetNameDirty();
/*N*/   bEdgesOfMarkedNodesDirty=TRUE;
/*N*/   bMarkedObjRectDirty=TRUE;
/*N*/   bMarkedPointsRectsDirty=TRUE;
/*N*/ }
/*N*/
////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrMarkView::WriteRecords(SvStream& rOut) const
/*N*/ {
/*N*/   SdrSnapView::WriteRecords(rOut);
/*N*/   {
/*N*/       SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_VIEWDRAGMODE);
/*N*/       rOut<<BOOL(eEditMode==SDREDITMODE_EDIT); // wg. Kompatibilitaet
/*N*/       rOut<<USHORT(eDragMode);
/*N*/       rOut<<aRef1;
/*N*/       rOut<<aRef2;
/*N*/       rOut<<BOOL(bForceFrameHandles);
/*N*/       rOut<<BOOL(bPlusHdlAlways);
/*N*/       rOut<<BOOL(eEditMode==SDREDITMODE_GLUEPOINTEDIT); // wg. Kompatibilitaet
/*N*/       rOut<<USHORT(eEditMode);
/*N*/       rOut<<BOOL(bMarkHdlWhenTextEdit);
/*N*/   } {
/*N*/       SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_VIEWCROOKCENTER);
/*N*/       rOut<<aLastCrookCenter;
/*N*/   }
/*N*/ }

/*N*/ BOOL SdrMarkView::ReadRecord(const SdrIOHeader& rViewHead,
/*N*/   const SdrNamedSubRecord& rSubHead,
/*N*/   SvStream& rIn)
/*N*/ {
/*N*/   BOOL bRet=FALSE;
/*N*/   if (rSubHead.GetInventor()==SdrInventor) {
/*N*/       bRet=TRUE;
/*N*/       switch (rSubHead.GetIdentifier()) {
/*N*/           case SDRIORECNAME_VIEWDRAGMODE: {
/*N*/               eEditMode=SDREDITMODE_EDIT;
/*N*/               BOOL bTmpBool;
/*N*/               USHORT nTmpUShort;
/*N*/               rIn>>bTmpBool; if (!bTmpBool) eEditMode=SDREDITMODE_CREATE; // wg. Kompatibilitaet
/*N*/               USHORT nDragMode;
/*N*/               rIn>>nDragMode;
/*N*/               eDragMode=SdrDragMode(nDragMode);
/*N*/               rIn>>aRef1;
/*N*/               rIn>>aRef2;
/*N*/               rIn>>bTmpBool; bForceFrameHandles=bTmpBool;
/*N*/               rIn>>bTmpBool; bPlusHdlAlways=bTmpBool;
/*N*/               if (rSubHead.GetBytesLeft()!=0) {
/*N*/                   rIn>>bTmpBool;
/*N*/                   if (bTmpBool) eEditMode=SDREDITMODE_GLUEPOINTEDIT; // wg. Kompatibilitaet
/*N*/               }
/*N*/               if (rSubHead.GetBytesLeft()!=0) {
/*N*/                   rIn>>nTmpUShort;
/*N*/                   eEditMode=(SdrViewEditMode)nTmpUShort;
/*N*/               }
/*N*/               if (rSubHead.GetBytesLeft()!=0) {
/*N*/                   rIn>>bTmpBool;
/*N*/                   bMarkHdlWhenTextEdit=bTmpBool;
/*N*/               }
/*N*/           } break;
/*N*/           case SDRIORECNAME_VIEWCROOKCENTER: {
/*N*/               rIn>>aLastCrookCenter;
/*N*/           } break;
/*N*/           default: bRet=FALSE;
/*N*/       }
/*N*/   }
/*N*/   if (!bRet) bRet=SdrSnapView::ReadRecord(rViewHead,rSubHead,rIn);
/*N*/   return bRet;
/*N*/ }


/*N*/ void SdrMarkView::SetDesignMode(BOOL bOn)
/*N*/ {
/*N*/   if (bDesignMode != bOn)
/*N*/   {
/*N*/       bDesignMode = bOn;
/*N*/       // Setzen des Modes fuer alle Controls
/*N*/       USHORT nAnz = GetPageViewCount();
/*N*/       for (USHORT nv = 0; nv<nAnz; nv++)
/*N*/       {
/*N*/           SdrPageView* pPV = GetPageViewPvNum(nv);
/*N*/           const SdrPageViewWinList& rWinList = pPV->GetWinList();
/*N*/           for (ULONG i = 0; i < rWinList.GetCount(); i++)
/*N*/           {
/*N*/               const SdrPageViewWinRec& rWR = rWinList[ (USHORT) i];
/*N*/               const SdrUnoControlList& rControlList = rWR.GetControlList();
/*N*/
/*N*/               for(UINT32 j = 0; j < rControlList.GetCount(); j++)
/*N*/               {
/*?*/                   ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > xControl = rControlList[ (USHORT) j].GetControl();
/*?*/                   DBG_ASSERT( xControl.is(), "SdrMarkView::SetDesignMode: no control at this position!" );
/*?*/                   if ( xControl.is() )
/*?*/                       xControl->setDesignMode(bOn);
/*?*/               }
/*N*/           }
/*N*/       }
/*N*/   }
/*N*/ }

// MarkHandles Objektaenderung:
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// - Bei Notify mit HINT_OBJCHG (oder so) werden die Handles erstmal versteckt
//   (wenn nicht schon wegen Dragging versteckt).
// - XorHdl: Bei ModelHasChanged() werden sie dann wieder angezeigt.
// - PaintEvents kommen nun durch.
//   - Die XorHandles werden z.T. wieder uebermalt.
//   - Xor:  Nach dem Painten werden die Handles im (vom PaintHandler gerufenen)
//           InitRedraw per ToggleShownXor bei gesetzter ClipRegion nochmal gemalt
//           und damit ist alles in Butter.
//   - Der AfterPaintTimer wird gestartet.
//   Da die Handles zu diesem Zeitpunkt nicht angezeigt sind wird:
//   - SaveBackground durchgefuehrt.
//   - DrawMarkHdl gerufen und bHdlShown gesetzt.
//
// MarkHandles bei sonstigem Invalidate:
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// In diesem Fall bekomme ich kein Notify und beim Aufruf des

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
