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

#include "svdogrp.hxx"

#include <bf_svtools/urihelper.hxx>

#include "xpool.hxx"

#include "svdmodel.hxx"
#include "svdpage.hxx"
#include "svditer.hxx"
#include "svdio.hxx"
#include "svdoedge.hxx"  // #32383# Die Verbinder nach Move nochmal anbroadcasten
#include "svdstr.hrc"    // Objektname

#include "svxids.hrc"

#include <bf_svtools/whiter.hxx>
#include "xoutx.hxx"
#include "bf_so3/staticbaseurl.hxx"
namespace binfilter {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// @@    @@ @@  @@ @@  @@  @@  @@  @@@@@ @@@@@@ @@@@@   @@@@@   @@@@  @@@@@@  @@@@
// @@    @@ @@@ @@ @@ @@   @@  @@ @@     @@     @@  @@  @@  @@ @@  @@   @@   @@  @@
// @@    @@ @@@@@@ @@@@    @@  @@  @@@@  @@@@@  @@@@@   @@  @@ @@@@@@   @@   @@@@@@
// @@    @@ @@ @@@ @@@@@   @@  @@     @@ @@     @@  @@  @@  @@ @@  @@   @@   @@  @@
// @@@@@ @@ @@  @@ @@  @@   @@@@  @@@@@  @@@@@@ @@  @@  @@@@@  @@  @@   @@   @@  @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ TYPEINIT1(ImpSdrObjGroupLinkUserData,SdrObjUserData);


/*N*/ ImpSdrObjGroupLinkUserData::ImpSdrObjGroupLinkUserData(SdrObject* pObj1):
/*N*/   SdrObjUserData(SdrInventor,SDRUSERDATA_OBJGROUPLINK,0),
/*N*/   pObj(pObj1),
        aFileDate0( DateTime::EMPTY ),
/*N*/   nDrehWink0(0),
/*N*/   nShearWink0(0),
/*N*/   pLink(NULL),
/*N*/   nObjNum(0),
/*N*/   nPageNum(0),
/*N*/   bMasterPage(FALSE),
/*N*/   bOrigPos(FALSE),
/*N*/   bOrigSize(FALSE),
/*N*/   bOrigRotate(FALSE),
/*N*/   bOrigShear(FALSE)
/*N*/ {
/*N*/ }


/*N*/ ImpSdrObjGroupLinkUserData::~ImpSdrObjGroupLinkUserData()
/*N*/ {
/*N*/ }


/*N*/ SdrObjUserData* ImpSdrObjGroupLinkUserData::Clone(SdrObject* pObj1) const
/*N*/ {
/*N*/   ImpSdrObjGroupLinkUserData* pData=new ImpSdrObjGroupLinkUserData(pObj1);
/*N*/   pData->aFileName  =aFileName;
/*N*/   pData->aObjName   =aObjName;
/*N*/   pData->aFileDate0 =aFileDate0;
/*N*/   pData->aSnapRect0 =aSnapRect0;
/*N*/   pData->nDrehWink0 =nDrehWink0;
/*N*/   pData->nShearWink0=nShearWink0;
/*N*/   pData->nObjNum    =nObjNum;
/*N*/   pData->nPageNum   =nPageNum;
/*N*/   pData->bMasterPage=bMasterPage;
/*N*/   pData->bOrigPos   =bOrigPos;
/*N*/   pData->bOrigSize  =bOrigSize;
/*N*/   pData->bOrigRotate=bOrigRotate;
/*N*/   pData->bOrigShear =bOrigShear;
/*N*/   pData->pLink=NULL;
/*N*/   return pData;
/*N*/ }

/*N*/ void ImpSdrObjGroupLinkUserData::ReadData(SvStream& rIn)
/*N*/ {
/*N*/   SdrObjUserData::ReadData(rIn);
/*N*/   // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/   SdrDownCompat aCompat(rIn, STREAM_READ);
/*N*/
/*N*/ #ifdef DBG_UTIL
/*N*/   aCompat.SetID("ImpSdrObjGroupLinkUserData");
/*N*/ #endif
/*N*/
/*N*/   BOOL bTmp;
/*N*/   UINT32 nTmp32;
/*N*/   String aFileNameRel;
/*N*/
/*N*/   aFileNameRel = rIn.ReadUniOrByteString(rIn.GetStreamCharSet());
/*N*/
/*N*/   if( aFileNameRel.Len() )
/*N*/   {
/*N*/       aFileName = ::binfilter::StaticBaseUrl::SmartRelToAbs( aFileNameRel, FALSE,
/*N*/                                               INetURLObject::WAS_ENCODED,
/*N*/                                               INetURLObject::DECODE_UNAMBIGUOUS );
/*N*/   }
/*N*/   else
/*N*/       aFileName.Erase();
/*N*/
/*N*/   // UNICODE: rIn >> aObjName;
/*N*/   aObjName = rIn.ReadUniOrByteString(rIn.GetStreamCharSet());
/*N*/
/*N*/   rIn >> nTmp32; aFileDate0.SetDate(nTmp32);
/*N*/   rIn >> nTmp32; aFileDate0.SetTime(nTmp32);
/*N*/   rIn >> aSnapRect0;
/*N*/   rIn >> nDrehWink0;
/*N*/   rIn >> nShearWink0;
/*N*/   rIn >> bTmp; bMasterPage=bTmp;
/*N*/   rIn >> nPageNum;
/*N*/   rIn >> nObjNum;
/*N*/   rIn >> bTmp; bOrigPos   =bTmp;
/*N*/   rIn >> bTmp; bOrigSize  =bTmp;
/*N*/   rIn >> bTmp; bOrigRotate=bTmp;
/*N*/   rIn >> bTmp; bOrigShear =bTmp;
/*N*/ }


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   @@@@  @@@@@  @@@@@@   @@@@  @@@@@   @@@@  @@  @@ @@@@@
//  @@  @@ @@  @@     @@  @@     @@  @@ @@  @@ @@  @@ @@  @@
//  @@  @@ @@@@@      @@  @@ @@@ @@@@@  @@  @@ @@  @@ @@@@@
//  @@  @@ @@  @@ @@  @@  @@  @@ @@  @@ @@  @@ @@  @@ @@
//   @@@@  @@@@@   @@@@    @@@@@ @@  @@  @@@@   @@@@  @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ TYPEINIT1(SdrObjGroup,SdrObject);


/*N*/ SdrObjGroup::SdrObjGroup()
/*N*/ {
/*N*/   pSub=new SdrObjList(NULL,NULL);
/*N*/   pSub->SetOwnerObj(this);
/*N*/   pSub->SetListKind(SDROBJLIST_GROUPOBJ);
/*N*/   bRefPoint=FALSE;
/*N*/   nDrehWink=0;
/*N*/   nShearWink=0;
/*N*/   bClosedObj=FALSE;
/*N*/   mpGroupItemSet = NULL;
/*N*/ }


/*N*/ SdrObjGroup::~SdrObjGroup()
/*N*/ {
/*N*/   delete pSub;
/*N*/   if(mpGroupItemSet)
/*?*/       delete mpGroupItemSet;
/*N*/ }


/*N*/ ImpSdrObjGroupLinkUserData* SdrObjGroup::GetLinkUserData() const
/*N*/ {
/*N*/   ImpSdrObjGroupLinkUserData* pData=NULL;
/*N*/   USHORT nAnz=GetUserDataCount();
/*N*/   for (USHORT nNum=nAnz; nNum>0 && pData==NULL;) {
/*N*/       nNum--;
/*N*/       pData=(ImpSdrObjGroupLinkUserData*)GetUserData(nNum);
/*N*/       if (pData->GetInventor()!=SdrInventor || pData->GetId()!=SDRUSERDATA_OBJGROUPLINK) {
/*N*/           pData=NULL;
/*N*/       }
/*N*/   }
/*N*/   return pData;
/*N*/ }


/*N*/ SdrLayerID SdrObjGroup::GetLayer() const
/*N*/ {
/*N*/   bool b1st=TRUE;
/*N*/   SdrLayerID nLay=SdrLayerID(nLayerId);
/*N*/   SdrObjList* pOL=pSub;
/*N*/   ULONG nObjAnz=pOL->GetObjCount();
/*N*/   for (ULONG i=0; i<nObjAnz; i++) {
/*N*/       SdrLayerID nLay1=pOL->GetObj(i)->GetLayer();
/*N*/       if (b1st) { nLay=nLay1; b1st=FALSE; }
/*N*/       else if (nLay1!=nLay) return 0;
/*N*/   }
/*N*/   return nLay;
/*N*/ }


/*N*/ void SdrObjGroup::NbcSetLayer(SdrLayerID nLayer)
/*N*/ {
/*N*/   SdrObject::NbcSetLayer(nLayer);
/*N*/   SdrObjList* pOL=pSub;
/*N*/   ULONG nObjAnz=pOL->GetObjCount();
/*N*/   for (ULONG i=0; i<nObjAnz; i++) {
/*N*/       pOL->GetObj(i)->NbcSetLayer(nLayer);
/*N*/   }
/*N*/ }


/*N*/ void SdrObjGroup::SetObjList(SdrObjList* pNewObjList)
/*N*/ {
/*N*/   SdrObject::SetObjList(pNewObjList);
/*N*/   pSub->SetUpList(pNewObjList);
/*N*/ }


/*N*/ void SdrObjGroup::SetPage(SdrPage* pNewPage)
/*N*/ {
/*N*/   SdrObject::SetPage(pNewPage);
/*N*/   pSub->SetPage(pNewPage);
/*N*/ }


/*N*/ void SdrObjGroup::SetModel(SdrModel* pNewModel)
/*N*/ {
/*N*/   SdrObject::SetModel(pNewModel);
/*N*/   pSub->SetModel(pNewModel);
/*N*/ }


/*N*/ SdrObjList* SdrObjGroup::GetSubList() const
/*N*/ {
/*N*/   return pSub;
/*N*/ }



/*N*/ void SdrObjGroup::SetName(const XubString& rStr)
/*N*/ {
/*N*/   aName=rStr; SetChanged();
/*N*/ }


/*N*/ XubString SdrObjGroup::GetName() const
/*N*/ {
/*N*/   return aName;
/*N*/ }


/*N*/ const Rectangle& SdrObjGroup::GetBoundRect() const
/*N*/ {
/*N*/   if (pSub->GetObjCount()!=0) {
/*N*/       // hier auch das aOutRect=AllObjSnapRect setzen, da GetSnapRect zu selten gerufen wird.
/*N*/       ((SdrObjGroup*)this)->aOutRect=pSub->GetAllObjSnapRect();
/*N*/       return pSub->GetAllObjBoundRect();
/*N*/   } else {
/*N*/       return aOutRect;
/*N*/   }
/*N*/ }


/*N*/ const Rectangle& SdrObjGroup::GetSnapRect() const
/*N*/ {
/*N*/   if (pSub->GetObjCount()!=0) {
/*N*/       ((SdrObjGroup*)this)->aOutRect=pSub->GetAllObjSnapRect();
/*N*/   }
/*N*/   return aOutRect;
/*N*/ }


/*N*/ bool SdrObjGroup::Paint(ExtOutputDevice& rXOut, const SdrPaintInfoRec& rInfoRec) const
/*N*/ {
/*N*/   bool bOk=TRUE;
/*N*/   if (pSub->GetObjCount()!=0) {
/*N*/       bOk=pSub->Paint(rXOut,rInfoRec);
/*N*/   } else { // ansonsten ist es eine leere Gruppe
/*?*/       if (!rInfoRec.bPrinter && rInfoRec.aPaintLayer.IsSet(nLayerId)) {
/*?*/           OutputDevice* pOutDev=rXOut.GetOutDev();
/*?*/           pOutDev->SetFillColor();
/*?*/           pOutDev->SetLineColor(COL_LIGHTGRAY);
/*?*/           pOutDev->DrawRect(aOutRect);
/*N*/       }
/*N*/   }
/*N*/   return bOk;
/*N*/ }



/*N*/ void SdrObjGroup::operator=(const SdrObject& rObj)
/*N*/ {
/*N*/   if (rObj.IsGroupObject()) {
/*N*/       SdrObject::operator=(rObj);
/*N*/       pSub->CopyObjects(*rObj.GetSubList());
/*N*/       nDrehWink  =((SdrObjGroup&)rObj).nDrehWink;
/*N*/       nShearWink =((SdrObjGroup&)rObj).nShearWink;
/*N*/       aName      =((SdrObjGroup&)rObj).aName;
/*N*/       aRefPoint  =((SdrObjGroup&)rObj).aRefPoint;
/*N*/       bRefPoint  =((SdrObjGroup&)rObj).bRefPoint;
/*N*/   }
/*N*/ }


/*N*/ void MergePoly(XPolyPolygon& rDst, const XPolyPolygon& rSrc)
/*N*/ {
/*N*/   USHORT nAnz=rSrc.Count();
/*N*/   USHORT i;
/*N*/   for (i=0; i<nAnz; i++) {
/*N*/       rDst.Insert(rSrc.GetObject(i));
/*N*/   }
/*N*/ }


/*N*/ void SdrObjGroup::TakeXorPoly(XPolyPolygon& rPoly, bool bDetail) const
/*N*/ {
/*N*/   rPoly.Clear();
/*N*/   ULONG nAnz=pSub->GetObjCount();
/*N*/   ULONG i=0;
/*N*/   while (i<nAnz) {
/*N*/       SdrObject* pObj=pSub->GetObj(i);
/*N*/       XPolyPolygon aPP;
/*N*/       pObj->TakeXorPoly(aPP,bDetail);
/*N*/       MergePoly(rPoly,aPP);
/*N*/       i++;
/*N*/   }
/*N*/   if (rPoly.Count()==0) {
/*?*/       rPoly.Insert(XPolygon(aOutRect));
/*N*/   }
/*N*/ }

/*N*/ long SdrObjGroup::GetRotateAngle() const
/*N*/ {
/*N*/   return nDrehWink;
/*N*/ }

/*N*/ long SdrObjGroup::GetShearAngle(bool /*bVertical*/) const
/*N*/ {
/*N*/   return nShearWink;
/*N*/ }

/*N*/ void SdrObjGroup::NbcSetSnapRect(const Rectangle& rRect)
/*N*/ {
/*N*/   Rectangle aOld(GetSnapRect());
/*N*/   long nDivX=aOld.Right()-aOld.Left();
/*N*/   long nDivY=aOld.Bottom()-aOld.Top();
/*N*/   if (nDivX==0) { nDivX=1; }
/*N*/   if (nDivY==0) { nDivY=1; }
/*N*/   if (rRect.Left()!=aOld.Left() || rRect.Top()!=aOld.Top()) {
/*N*/       NbcMove(Size(rRect.Left()-aOld.Left(),rRect.Top()-aOld.Top()));
/*N*/   }
/*N*/ }


/*N*/ void SdrObjGroup::NbcSetLogicRect(const Rectangle& rRect)
/*N*/ {
/*N*/   NbcSetSnapRect(rRect);
/*N*/ }


/*N*/ void SdrObjGroup::NbcMove(const Size& rSiz)
/*N*/ {
/*N*/   MovePoint(aRefPoint,rSiz);
/*N*/   if (pSub->GetObjCount()!=0) {
/*N*/       SdrObjList* pOL=pSub;
/*N*/       ULONG nObjAnz=pOL->GetObjCount();
/*N*/       for (ULONG i=0; i<nObjAnz; i++) {
/*N*/           SdrObject* pObj=pOL->GetObj(i);
/*N*/           pObj->NbcMove(rSiz);
/*N*/       }
/*N*/   } else {
/*?*/       MoveRect(aOutRect,rSiz);
/*?*/       SetRectsDirty();
/*N*/   }
/*N*/ }


/*N*/ void SdrObjGroup::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
/*N*/ {
/*N*/   ResizePoint(aRefPoint,rRef,xFact,yFact);
/*N*/   if (pSub->GetObjCount()!=0) {
/*N*/       SdrObjList* pOL=pSub;
/*N*/       ULONG nObjAnz=pOL->GetObjCount();
/*N*/       for (ULONG i=0; i<nObjAnz; i++) {
/*N*/           SdrObject* pObj=pOL->GetObj(i);
/*N*/           pObj->NbcResize(rRef,xFact,yFact);
/*N*/       }
/*N*/   } else {
/*N*/       ResizeRect(aOutRect,rRef,xFact,yFact);
/*N*/       SetRectsDirty();
/*N*/   }
/*N*/ }


/*N*/ void SdrObjGroup::NbcSetAnchorPos(const Point& rPnt)
/*N*/ {
/*N*/   aAnchor=rPnt;
/*N*/   Size aSiz(rPnt.X()-aAnchor.X(),rPnt.Y()-aAnchor.Y());
/*N*/   MovePoint(aRefPoint,aSiz);
/*N*/   SdrObjList* pOL=pSub;
/*N*/   ULONG nObjAnz=pOL->GetObjCount();
/*N*/   for (ULONG i=0; i<nObjAnz; i++) {
/*N*/       SdrObject* pObj=pOL->GetObj(i);
/*N*/       pObj->NbcSetAnchorPos(rPnt);
/*N*/   }
/*N*/ }


/*N*/ void SdrObjGroup::SetSnapRect(const Rectangle& rRect)
/*N*/ {
/*N*/   Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
/*N*/   Rectangle aOld(GetSnapRect());
/*N*/   long nMulX=rRect.Right()-rRect.Left();
/*N*/   long nDivX=aOld.Right()-aOld.Left();
/*N*/   long nMulY=rRect.Bottom()-rRect.Top();
/*N*/   long nDivY=aOld.Bottom()-aOld.Top();
/*N*/   if (nDivX==0) { nMulX=1; nDivX=1; }
/*N*/   if (nDivY==0) { nMulY=1; nDivY=1; }
/*N*/   if (nMulX!=nDivX || nMulY!=nDivY) {
/*N*/       Fraction aX(nMulX,nDivX);
/*N*/       Fraction aY(nMulY,nDivY);
/*N*/       Resize(aOld.TopLeft(),aX,aY);
/*N*/   }
/*N*/   if (rRect.Left()!=aOld.Left() || rRect.Top()!=aOld.Top()) {
/*?*/       Move(Size(rRect.Left()-aOld.Left(),rRect.Top()-aOld.Top()));
/*N*/   }
/*N*/   SendRepaintBroadcast(TRUE);
/*N*/   SetChanged();
/*N*/   SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
/*N*/ }


/*N*/ void SdrObjGroup::Move(const Size& rSiz)
/*N*/ {
/*N*/   if (rSiz.Width()!=0 || rSiz.Height()!=0) {
/*N*/       Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
/*N*/       MovePoint(aRefPoint,rSiz);
/*N*/       if (pSub->GetObjCount()!=0) {
/*N*/           // #32383# Erst die Verbinder verschieben, dann den Rest
/*N*/           SdrObjList* pOL=pSub;
/*N*/           ULONG nObjAnz=pOL->GetObjCount();
/*N*/           ULONG i;
/*N*/           for (i=0; i<nObjAnz; i++) {
/*N*/               SdrObject* pObj=pOL->GetObj(i);
/*N*/               if (pObj->IsEdgeObj()) pObj->Move(rSiz);
/*N*/           }
/*N*/           for (i=0; i<nObjAnz; i++) {
/*N*/               SdrObject* pObj=pOL->GetObj(i);
/*N*/               if (!pObj->IsEdgeObj()) pObj->Move(rSiz);
/*N*/           }
/*N*/           SendRepaintBroadcast(TRUE);
/*N*/       } else {
/*N*/           SendRepaintBroadcast();
/*N*/           MoveRect(aOutRect,rSiz);
/*N*/           SetRectsDirty();
/*N*/           SendRepaintBroadcast();
/*N*/       }
/*N*/       SetChanged();
/*N*/       SendUserCall(SDRUSERCALL_MOVEONLY,aBoundRect0);
/*N*/   }
/*N*/ }


/*N*/ void SdrObjGroup::Resize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
/*N*/ {
/*N*/   if (xFact.GetNumerator()!=xFact.GetDenominator() || yFact.GetNumerator()!=yFact.GetDenominator()) {
/*N*/       Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
/*N*/       ResizePoint(aRefPoint,rRef,xFact,yFact);
/*N*/       if (pSub->GetObjCount()!=0) {
/*?*/           // #32383# Erst die Verbinder verschieben, dann den Rest
/*?*/           SdrObjList* pOL=pSub;
/*?*/           ULONG nObjAnz=pOL->GetObjCount();
/*?*/           ULONG i;
/*?*/           for (i=0; i<nObjAnz; i++) {
/*?*/               SdrObject* pObj=pOL->GetObj(i);
/*?*/               if (pObj->IsEdgeObj()) pObj->Resize(rRef,xFact,yFact);
/*?*/           }
/*?*/           for (i=0; i<nObjAnz; i++) {
/*?*/               SdrObject* pObj=pOL->GetObj(i);
/*?*/               if (!pObj->IsEdgeObj()) pObj->Resize(rRef,xFact,yFact);
/*?*/           }
/*?*/           SendRepaintBroadcast(TRUE);
/*N*/       } else {
/*N*/           SendRepaintBroadcast();
/*N*/           ResizeRect(aOutRect,rRef,xFact,yFact);
/*N*/           SetRectsDirty();
/*N*/           SendRepaintBroadcast();
/*N*/       }
/*N*/       SetChanged();
/*N*/       SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
/*N*/   }
/*N*/ }


/*N*/ void SdrObjGroup::SetAnchorPos(const Point& rPnt)
/*N*/ {
/*N*/   Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
/*N*/   bool bChg=aAnchor!=rPnt;
/*N*/   aAnchor=rPnt;
/*N*/   Size aSiz(rPnt.X()-aAnchor.X(),rPnt.Y()-aAnchor.Y());
/*N*/   MovePoint(aRefPoint,aSiz);
/*N*/   // #32383# Erst die Verbinder verschieben, dann den Rest
/*N*/   SdrObjList* pOL=pSub;
/*N*/   ULONG nObjAnz=pOL->GetObjCount();
/*N*/   ULONG i;
/*N*/   for (i=0; i<nObjAnz; i++) {
/*N*/       SdrObject* pObj=pOL->GetObj(i);
/*N*/       if (pObj->IsEdgeObj()) pObj->SetAnchorPos(rPnt);
/*N*/   }
/*N*/   for (i=0; i<nObjAnz; i++) {
/*N*/       SdrObject* pObj=pOL->GetObj(i);
/*N*/       if (!pObj->IsEdgeObj()) pObj->SetAnchorPos(rPnt);
/*N*/   }
/*N*/   if (bChg) {
/*N*/       SendRepaintBroadcast(TRUE);
/*N*/       SetChanged();
/*N*/       SendUserCall(SDRUSERCALL_MOVEONLY,aBoundRect0);
/*N*/   }
/*N*/ }


/*N*/ void SdrObjGroup::SetRelativePos(const Point& rPnt)
/*N*/ {
/*N*/   Point aRelPos0(GetSnapRect().TopLeft()-aAnchor);
/*N*/   Size aSiz(rPnt.X()-aRelPos0.X(),rPnt.Y()-aRelPos0.Y());
/*N*/   if (aSiz.Width()!=0 || aSiz.Height()!=0) Move(aSiz); // Der ruft auch das SetRectsDirty() und Broadcast, ...
/*N*/ }


////////////////////////////////////////////////////////////////////////////////////////////////////
// ItemSet access

/*N*/ const SfxItemSet& SdrObjGroup::GetItemSet() const
/*N*/ {
/*N*/   // prepare ItemSet
/*N*/   if(mpGroupItemSet)
/*N*/       mpGroupItemSet->ClearItem();
/*N*/   else
/*N*/   {
/*N*/       ((SdrObjGroup*)this)->mpGroupItemSet =
/*N*/           ((SdrObjGroup*)this)->CreateNewItemSet((SfxItemPool&)(*GetItemPool()));
/*N*/       DBG_ASSERT(mpGroupItemSet, "Could not create an SfxItemSet(!)");
/*N*/   }
/*N*/
/*N*/   // collect all ItemSets in mpGroupItemSet
/*N*/   sal_uInt32 nCount(pSub->GetObjCount());
/*N*/   for(sal_uInt32 a(0); a < nCount; a++)
/*N*/   {
/*N*/       const SfxItemSet& rSet = pSub->GetObj(a)->GetItemSet();
/*N*/       SfxWhichIter aIter(rSet);
/*N*/       sal_uInt16 nWhich(aIter.FirstWhich());
/*N*/
/*N*/       while(nWhich)
/*N*/       {
/*N*/           if(SFX_ITEM_DONTCARE == rSet.GetItemState(nWhich, FALSE))
/*N*/               mpGroupItemSet->InvalidateItem(nWhich);
/*N*/           else
/*N*/               mpGroupItemSet->MergeValue(rSet.Get(nWhich), TRUE);
/*N*/
/*N*/           nWhich = aIter.NextWhich();
/*N*/       }
/*N*/   }
/*N*/
/*N*/   return *mpGroupItemSet;
/*N*/ }

/*N*/ SfxItemSet* SdrObjGroup::CreateNewItemSet(SfxItemPool& rPool)
/*N*/ {
/*N*/   // include ALL items
/*N*/   return new SfxItemSet(rPool, SDRATTR_START, SDRATTR_END);
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// private support routines for ItemSet access

/*N*/ void SdrObjGroup::ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem)
/*N*/ {
/*N*/   if(!IsLinkedGroup())
/*N*/   {
/*N*/       // ItemChange at all contained objects
/*N*/       List aPostItemChangeList;
/*N*/       sal_uInt32 nCount(pSub->GetObjCount());
/*N*/
/*N*/       sal_uInt32 a; for(a = 0; a < nCount; a++)
/*N*/       {
/*N*/           SdrObject* pObj = pSub->GetObj(a);
/*N*/           pObj->ItemChange(nWhich, pNewItem);
/*N*/           aPostItemChangeList.Insert((void*)pObj, LIST_APPEND);
/*N*/       }
/*N*/
/*N*/       for(a = 0; a < aPostItemChangeList.Count(); a++)
/*N*/       {
/*N*/           SdrObject* pObj = (SdrObject*)aPostItemChangeList.GetObject(a);
/*N*/           pObj->PostItemChange(nWhich);
/*N*/       }
/*N*/   }
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// ItemSet was changed, maybe user wants to react

/*N*/ void SdrObjGroup::ItemSetChanged(const SfxItemSet& rSet)
/*N*/ {
/*N*/   if(!IsLinkedGroup())
/*N*/   {
/*N*/       sal_uInt32 nCount(pSub->GetObjCount());
/*N*/       for(sal_uInt32 a(0); a < nCount; a++)
/*N*/           pSub->GetObj(a)->ItemSetChanged(rSet);
/*N*/   }
/*N*/ }

/*N*/ SfxStyleSheet* SdrObjGroup::GetStyleSheet() const
/*N*/ {
/*N*/   // Hier mit 'nem Iterator. Es koennte sonst passieren dass ein
/*N*/   // verschachteltes Gruppenobjekt wegen DontCare NULL liefert.
/*N*/   // Das koennte ich dann nicht unterscheiden von NotSet.
/*N*/   SfxStyleSheet* pRet=NULL;
/*N*/   bool b1st=TRUE;
/*N*/   SdrObjListIter aIter(*this,IM_DEEPNOGROUPS);
/*N*/   while (aIter.IsMore()) {
/*N*/       SdrObject* pObj=aIter.Next();
/*N*/       if (b1st) {
/*N*/           b1st=FALSE;
/*N*/           pRet=pObj->GetStyleSheet(); // Der Erste
/*N*/       } else {
/*N*/           if (pObj->GetStyleSheet()!=pRet) return NULL; // Unterschiedlich!
/*N*/       }
/*N*/   }
/*N*/   // Wenn hier angekommen, sind alle gleich.
/*N*/   return pRet;
/*N*/ }


/*N*/ void SdrObjGroup::NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr)
/*N*/ {
/*N*/   if (!IsLinkedGroup()) {
/*N*/       SdrObjList* pOL=pSub;
/*N*/       ULONG nObjAnz=pOL->GetObjCount();
/*N*/       for (ULONG i=0; i<nObjAnz; i++) {
/*N*/           pOL->GetObj(i)->NbcSetStyleSheet(pNewStyleSheet,bDontRemoveHardAttr);
/*N*/       }
/*N*/   }
/*N*/ }


/*N*/ void SdrObjGroup::ReformatText()
/*N*/ {
/*N*/   pSub->ReformatAllTextObjects();
/*N*/ }


/*N*/ void SdrObjGroup::RestartAnimation(SdrPageView* pPageView) const
/*N*/ {
/*N*/   pSub->RestartAllAnimations(pPageView);
/*N*/ }

/*N*/ void SdrObjGroup::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
/*N*/ {
/*N*/   if(rIn.GetError())
/*N*/       return;
/*N*/
/*N*/   SdrObject::ReadData(rHead, rIn);
/*N*/   // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/   SdrDownCompat aCompat(rIn, STREAM_READ);
/*N*/
/*N*/ #ifdef DBG_UTIL
/*N*/   aCompat.SetID("SdrObjGroup");
/*N*/ #endif
/*N*/
/*N*/   // UNICODE: rIn >> aName;
/*N*/   aName = rIn.ReadUniOrByteString(rIn.GetStreamCharSet());
/*N*/
/*N*/   UINT8 nTemp; rIn >> nTemp; bRefPoint = nTemp;
/*N*/   rIn >> aRefPoint;
/*N*/   pSub->Load(rIn, *pPage);
/*N*/
/*N*/   if(rHead.GetVersion() >= 2)
/*N*/   {
/*N*/       INT32 n32;
/*N*/
/*N*/       rIn >> n32; nDrehWink = n32;
/*N*/       rIn >> n32; nShearWink = n32;
/*N*/   }
/*N*/ }


/*N*/ void SdrObjGroup::AfterRead()
/*N*/ {
/*N*/   SdrObject::AfterRead();
/*N*/   pSub->AfterRead();
/*N*/
/*N*/   // #80049# as fix for errors after #69055#
/*N*/   if(aAnchor.X() || aAnchor.Y())
/*N*/       NbcSetAnchorPos(aAnchor);
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
