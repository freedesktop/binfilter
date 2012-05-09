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

#include "svdtxhdl.hxx"
#include <limits.h>
#include "svdmodel.hxx"
#include "svdotext.hxx"
#include "svdoutl.hxx"

#include <outlobj.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////
// #101499#

#include <com/sun/star/i18n/ScriptType.hdl>

#include <com/sun/star/i18n/CharacterIteratorMode.hdl>

#include "xoutx.hxx"

#include <vector>

namespace binfilter {

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ ImpTextPortionHandler::ImpTextPortionHandler(SdrOutliner& rOutln, const SdrTextObj& rTxtObj):
/*N*/   rOutliner(rOutln),
/*N*/   rTextObj(rTxtObj),
/*N*/   // #101498# aPoly(0)
/*N*/   mpRecordPortions(0L)
/*N*/ {
/*N*/   pModel=rTextObj.GetModel();
/*N*/   MapMode aMap=aVDev.GetMapMode();
/*N*/
/*N*/   aMap.SetMapUnit(pModel->GetScaleUnit());
/*N*/   aMap.SetScaleX(pModel->GetScaleFraction());
/*N*/   aMap.SetScaleY(pModel->GetScaleFraction());
/*N*/   aVDev.SetMapMode(aMap);
/*N*/ }


/*N*/ void ImpTextPortionHandler::DrawTextToPath(ExtOutputDevice& rXOut, bool bDrawEffect)
/*N*/ {
/*N*/   aFormTextBoundRect = Rectangle();
/*N*/   rTextObj.GetSnapRect();
/*N*/
/*N*/   bDraw = bDrawEffect;
/*N*/
/*N*/   OutlinerParaObject* pPara=rTextObj.GetOutlinerParaObject();
/*N*/   if (pPara!=NULL) {
/*N*/       XPolyPolygon aXPP;
/*N*/       //rTextObj.TakeContour(aXPP);
/*N*/       rTextObj.TakeXorPoly(aXPP,FALSE);
/*N*/       pXOut=&rXOut;
/*N*/       Font aFont(rXOut.GetOutDev()->GetFont());
/*N*/       rOutliner.Clear();
/*N*/       rOutliner.SetPaperSize(Size(LONG_MAX,LONG_MAX));
/*N*/       rOutliner.SetText(*pPara);
/*N*/       USHORT nCnt = Min(aXPP.Count(), (USHORT) rOutliner.GetParagraphCount());
/*N*/
/*N*/       if ( nCnt == 1 )    bToLastPoint = TRUE;
/*N*/       else                bToLastPoint = FALSE;
/*N*/
/*N*/       // #101498# completely different methodology needed here for making this BIDI-able
/*N*/       // iterate over paragraphs and Polygons, thus each paragraph will be put to
/*N*/       // one Polygon
/*N*/       const sal_uInt32 nSavedLayoutMode(rXOut.GetOutDev()->GetLayoutMode());
/*N*/       sal_uInt32 nLayoutMode(nSavedLayoutMode);
/*N*/
/*N*/       if(TRUE)
/*N*/       {
/*N*/           nLayoutMode &= ~(TEXT_LAYOUT_BIDI_RTL|TEXT_LAYOUT_COMPLEX_DISABLED|TEXT_LAYOUT_BIDI_STRONG);
/*N*/       }
/*N*/       else
/*N*/       {
/*N*/           nLayoutMode &= ~(TEXT_LAYOUT_BIDI_RTL);
/*N*/           nLayoutMode |= ~(TEXT_LAYOUT_COMPLEX_DISABLED|TEXT_LAYOUT_BIDI_STRONG);
/*N*/       }
/*N*/
/*N*/       rXOut.GetOutDev()->SetLayoutMode(nLayoutMode);
/*N*/
/*N*/       for(nParagraph = 0; nParagraph < nCnt; nParagraph++)
/*N*/       {
/*N*/           Polygon aPoly = XOutCreatePolygon(aXPP[sal_uInt16(nParagraph)], rXOut.GetOutDev());
/*N*/
/*N*/           rOutliner.SetDrawPortionHdl(LINK(this, ImpTextPortionHandler, FormTextRecordPortionHdl));
/*N*/           rOutliner.StripPortions();
/*N*/
/*N*/           DrawFormTextRecordPortions(aPoly);
/*N*/           ClearFormTextRecordPortions();
/*N*/
/*N*/           const Rectangle& rFTBR = rXOut.GetFormTextBoundRect();
/*N*/           aFormTextBoundRect.Union(rFTBR);
/*N*/       }
/*N*/
/*N*/       rXOut.GetOutDev()->SetLayoutMode(nSavedLayoutMode);
/*N*/
/*N*/       rXOut.GetOutDev()->SetFont(aFont);
/*N*/       rOutliner.Clear();
/*N*/   }
/*N*/ }

// #101498# Record and sort all portions
/*N*/ IMPL_LINK(ImpTextPortionHandler, FormTextRecordPortionHdl, DrawPortionInfo*, pInfo)
/*N*/ {
/*N*/   if(pInfo->nPara == nParagraph)
/*N*/   {
/*N*/       SortedAddFormTextRecordPortion(pInfo);
/*N*/   }
/*N*/
/*N*/   return 0;
/*N*/ }

// #101498# Helper class to remember text portions in sorted manner
/*N*/ class ImpRecordPortion
/*N*/ {
/*N*/ public:
/*N*/   Point                       maPosition;
/*N*/   String                      maText;
/*N*/   xub_StrLen                  mnTextStart;
/*N*/   xub_StrLen                  mnTextLength;
/*N*/   sal_uInt16                  mnPara;
/*N*/   xub_StrLen                  mnIndex;
/*N*/
/*N*/   // #102819# Here SvxFont needs to be used instead of Font
/*N*/   SvxFont                     maFont;
/*N*/
/*N*/   sal_Int32*                  mpDXArray;
/*N*/   sal_uInt8                   mnBiDiLevel;
/*N*/
/*N*/   ImpRecordPortion(DrawPortionInfo* pInfo);
/*N*/   ~ImpRecordPortion();
/*N*/
/*N*/   sal_uInt8 GetBiDiLevel() const { return mnBiDiLevel; }
/*N*/ };

/*N*/ ImpRecordPortion::ImpRecordPortion(DrawPortionInfo* pInfo)
/*N*/ : maPosition(pInfo->rStartPos),
/*N*/   maText(pInfo->rText),
/*N*/   mnTextStart((xub_StrLen)pInfo->nTextStart),
/*N*/   mnTextLength((xub_StrLen)pInfo->nTextLen),
/*N*/   mnPara(pInfo->nPara),
/*N*/   mnIndex(pInfo->nIndex),
/*N*/   maFont(pInfo->rFont),
/*N*/   mpDXArray(NULL),
/*N*/   mnBiDiLevel(pInfo->GetBiDiLevel())
/*N*/ {
/*N*/   if(pInfo->pDXArray)
/*N*/   {
/*N*/       mpDXArray = new sal_Int32[pInfo->nTextLen];
/*N*/
/*N*/       for(sal_uInt32 a(0L); a < pInfo->nTextLen; a++)
/*N*/       {
/*N*/           mpDXArray[a] = pInfo->pDXArray[a];
/*N*/       }
/*N*/   }
/*N*/ }
/*N*/
/*N*/ ImpRecordPortion::~ImpRecordPortion()
/*N*/ {
/*N*/   if(mpDXArray)
/*N*/   {
/*N*/       delete[] mpDXArray;
/*N*/   }
/*N*/ }

/*N*/ // #101498# List classes for recording portions
typedef ::std::vector< ImpRecordPortion* > ImpRecordPortionList;
typedef ::std::vector< ImpRecordPortionList* > ImpRecordPortionListList;

/*N*/ // #101498# Draw recorded formtext along Poly
/*N*/ void ImpTextPortionHandler::DrawFormTextRecordPortions(Polygon aPoly)
/*N*/ {
/*N*/   sal_Int32 nTextWidth = -((sal_Int32)GetFormTextPortionsLength(pXOut->GetOutDev()));
/*N*/
/*N*/   ImpRecordPortionListList* pListList = (ImpRecordPortionListList*)mpRecordPortions;
/*N*/   if(pListList)
/*N*/   {
/*N*/       for(size_t a = 0, na = pListList->size(); a < na; a++)
/*N*/       {
/*N*/           ImpRecordPortionList* pList = (*pListList)[ a ];
/*N*/
/*N*/           for( size_t b = 0, nb = pList->size(); b < nb; ++b )
/*N*/           {
/*N*/               ImpRecordPortion* pPortion = (*pList)[ b ];
/*N*/
/*N*/               DrawPortionInfo aNewInfo(
/*N*/                   pPortion->maPosition,
/*N*/                   pPortion->maText,
/*N*/                   pPortion->mnTextStart,
/*N*/                   pPortion->mnTextLength,
/*N*/                   pPortion->maFont,
/*N*/                   pPortion->mnPara,
/*N*/                   pPortion->mnIndex,
/*N*/                   pPortion->mpDXArray,
/*N*/                   pPortion->mnBiDiLevel);
/*N*/
/*N*/               nTextWidth = pXOut->DrawFormText(&aNewInfo, aPoly, nTextWidth, bToLastPoint, bDraw);
/*N*/           }
/*N*/       }
/*N*/   }
/*N*/ }

/*N*/ // #101498# Insert new portion sorted
/*N*/ void ImpTextPortionHandler::SortedAddFormTextRecordPortion(DrawPortionInfo* pInfo)
/*N*/ {
/*N*/   // get ListList and create on demand
/*N*/   ImpRecordPortionListList* pListList = (ImpRecordPortionListList*)mpRecordPortions;
/*N*/   if(!pListList)
/*N*/   {
/*N*/       mpRecordPortions = (void*)(new ImpRecordPortionListList());
/*N*/       pListList = (ImpRecordPortionListList*)mpRecordPortions;
/*N*/   }
/*N*/
/*N*/   // create new portion
/*N*/   ImpRecordPortion* pNewPortion = new ImpRecordPortion(pInfo);
/*N*/
/*N*/   // look for the list where to insert new portion
/*N*/   ImpRecordPortionList* pList = 0L;
/*N*/
/*N*/   for(size_t nListListIndex(0L); !pList && nListListIndex < pListList->size(); nListListIndex++)
/*N*/   {
/*?*/       ImpRecordPortionList* pTmpList = (*pListList)[ nListListIndex ];
/*?*/
/*?*/       if( (*pTmpList)[0]->maPosition.Y() == pNewPortion->maPosition.Y())
/*?*/           pList = pTmpList;
/*N*/   }
/*N*/
/*N*/   if(!pList)
/*N*/   {
/*N*/       // no list for that Y-Coordinate yet, create a new one.
/*N*/       pList = new ImpRecordPortionList();
/*N*/       pList->push_back ( pNewPortion );
/*N*/       pListList->push_back( pList );
/*N*/   }
/*N*/   else
/*N*/   {
/*N*/       // found a list for that for that Y-Coordinate, sort in
/*?*/       sal_uInt32 nInsertInd(0L);
/*?*/
/*?*/       while( nInsertInd < pList->size()
/*?*/           && (*pList)[ nInsertInd ]->maPosition.X() < pNewPortion->maPosition.X())
/*?*/       {
/*?*/           nInsertInd++;
/*?*/       }
/*?*/
/*?*/       if( nInsertInd == pList->size() )
            {
                pList->push_back( pNewPortion );
            }
            else
            {
                ImpRecordPortionList::iterator it = pList->begin();
                ::std::advance( it, nInsertInd );
                pList->insert( it, pNewPortion );
            }
/*N*/   }
/*N*/ }

// #101498# Calculate complete length of FormTextPortions
/*N*/ sal_uInt32 ImpTextPortionHandler::GetFormTextPortionsLength(OutputDevice* pOut)
/*N*/ {
/*N*/   sal_uInt32 nRetval(0L);
/*N*/
/*N*/   ImpRecordPortionListList* pListList = (ImpRecordPortionListList*)mpRecordPortions;
/*N*/   if(pListList)
/*N*/   {
/*N*/       for( size_t a(0L); a < pListList->size(); a++)
/*N*/       {
/*N*/           ImpRecordPortionList* pList = (*pListList)[ a ];
/*N*/
/*N*/           for(size_t b(0L); b < pList->size(); b++)
/*N*/           {
/*N*/               ImpRecordPortion* pPortion = (*pList)[ b ];
/*N*/
/*N*/               if(pPortion->mpDXArray)
/*N*/               {
/*N*/                   if(pPortion->maFont.IsVertical() && pOut)
/*?*/                       nRetval += pOut->GetTextHeight() * pPortion->mnTextLength;
/*N*/                   else
/*N*/                       nRetval += pPortion->mpDXArray[pPortion->mnTextLength - 1];
/*N*/               }
/*N*/           }
/*N*/       }
/*N*/   }
/*N*/
/*N*/   return nRetval;
/*N*/ }

// #101498# Cleanup recorded portions
/*N*/ void ImpTextPortionHandler::ClearFormTextRecordPortions()
/*N*/ {
/*N*/   ImpRecordPortionListList* pListList = (ImpRecordPortionListList*)mpRecordPortions;
/*N*/   if(pListList)
/*N*/   {
/*N*/       for( size_t a(0L); a < pListList->size(); a++)
/*N*/       {
/*N*/           ImpRecordPortionList* pList = (*pListList)[ a ];
/*N*/           for( size_t b(0L); b < pList->size(); b++)
/*N*/               delete (*pList)[ b ];
/*N*/           delete pList;
/*N*/       }
/*N*/
/*N*/       delete pListList;
/*N*/       mpRecordPortions = (void*)0L;
/*N*/   }
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
