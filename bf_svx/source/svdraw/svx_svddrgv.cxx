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

#include "svddrgv.hxx"
#include "svdio.hxx"
#include "svdstr.hrc"   // Namen aus der Resource

namespace binfilter {

/*N*/ void SdrDragView::ImpClearVars()
/*N*/ {
/*N*/   eDragMode=SDRDRAG_MOVE;
/*N*/   bMarkedHitMovesAlways=FALSE;
/*N*/   bDragStripes=FALSE;
/*N*/   bNoDragHdl=TRUE;
/*N*/   bMirrRefDragObj=TRUE;
/*N*/   bSolidDragging=FALSE;
/*N*/   bDragWithCopy=FALSE;
/*N*/   bNoDragXorPolys=FALSE;
/*N*/ }

/*N*/ SdrDragView::SdrDragView(SdrModel* pModel1, OutputDevice* pOut):
/*N*/   SdrExchangeView(pModel1,pOut)
/*N*/ {
/*N*/   ImpClearVars();
/*N*/ }

/*N*/ SdrDragView::~SdrDragView()
/*N*/ {
/*N*/ }

/*N*/ BOOL SdrDragView::IsAction() const
/*N*/ {
/*N*/   return SdrExchangeView::IsAction();
/*N*/ }

/*N*/ void SdrDragView::BrkAction()
/*N*/ {
/*N*/   SdrExchangeView::BrkAction();
/*N*/ }

/*N*/ void SdrDragView::ToggleShownXor(OutputDevice* pOut, const Region* pRegion) const
/*N*/ {
/*N*/   SdrExchangeView::ToggleShownXor(pOut,pRegion);
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrDragView::SetNoDragXorPolys(BOOL bOn)
/*N*/ {
/*N*/   if (IsNoDragXorPolys()!=bOn) {
/*N*/       bNoDragXorPolys=bOn;
/*N*/   }
/*N*/ }

/*N*/ void SdrDragView::SetDragStripes(BOOL bOn)
/*N*/ {
/*N*/   bDragStripes=bOn;
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrDragView::WriteRecords(SvStream& rOut) const
/*N*/ {
/*N*/   SdrExchangeView::WriteRecords(rOut);
/*N*/   {
/*N*/       SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_VIEWDRAGSTRIPES);
/*N*/       rOut<<(BOOL)bDragStripes;
/*N*/   } {
/*N*/       SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_VIEWDRAGHIDEHDL);
/*N*/       rOut<<(BOOL)bNoDragHdl;
/*N*/   } {
/*N*/       SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_VIEWOBJHITMOVES);
/*N*/       rOut<<(BOOL)bMarkedHitMovesAlways;
/*N*/   } {
/*N*/       SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_VIEWMIRRDRAGOBJ);
/*N*/       rOut<<(BOOL)bMirrRefDragObj;
/*N*/   }
/*N*/ }

/*N*/ BOOL SdrDragView::ReadRecord(const SdrIOHeader& rViewHead,
/*N*/   const SdrNamedSubRecord& rSubHead,
/*N*/   SvStream& rIn)
/*N*/ {
/*N*/   BOOL bRet=FALSE;
/*N*/   if (rSubHead.GetInventor()==SdrInventor) {
/*N*/       bRet=TRUE;
/*N*/       switch (rSubHead.GetIdentifier()) {
/*N*/           case SDRIORECNAME_VIEWDRAGSTRIPES: {
/*N*/               BOOL bZwi; rIn >> bZwi; bDragStripes = bZwi;
/*N*/           } break;
/*N*/           case SDRIORECNAME_VIEWDRAGHIDEHDL: {
/*N*/               BOOL bZwi; rIn >> bZwi; bNoDragHdl = bZwi;
/*N*/           } break;
/*N*/           case SDRIORECNAME_VIEWOBJHITMOVES: {
/*N*/               BOOL bZwi; rIn >> bZwi; bMarkedHitMovesAlways = bZwi;
/*N*/           } break;
/*N*/           case SDRIORECNAME_VIEWMIRRDRAGOBJ: {
/*N*/               BOOL bZwi; rIn >> bZwi; bMirrRefDragObj = bZwi;
/*N*/           } break;
/*N*/           default: bRet=FALSE;
/*N*/       }
/*N*/   }
/*N*/   if (!bRet) bRet=SdrExchangeView::ReadRecord(rViewHead,rSubHead,rIn);
/*N*/   return bRet;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
