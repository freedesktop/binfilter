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

#include <bf_svx/tabline.hxx>
#include <bf_svx/svdograf.hxx>
#include <bf_svx/svdopath.hxx>
#include <bf_svtools/whiter.hxx>
#include <bf_svx/xbtmpit.hxx>

#include <bf_svx/eeitem.hxx>
#ifndef _SVX_COLRITEM_HXX //autogen
#define ITEMID_COLOR       EE_CHAR_COLOR
#endif
#include <bf_svx/xflftrit.hxx>
#include <bf_svx/xflhtit.hxx>
#include <bf_svx/xtable.hxx>
#include "schattr.hxx"

// header for SvxChartTextOrientItem
#ifndef _SVX_CHRTITEM_HXX
#define ITEMID_DOUBLE           0
#define ITEMID_CHARTTEXTORIENT  SCHATTR_TEXT_ORIENT
#endif

#define ITEMID_FONTHEIGHT  EE_CHAR_FONTHEIGHT
#include <globfunc.hxx>
#include <bf_svx/svxids.hrc>



#include "schattr.hxx"
#ifndef _SVX_BRSHITEM_HXX //autogen
#define ITEMID_BRUSH SCHATTR_SYMBOL_BRUSH
#include <bf_svx/brshitem.hxx>
#endif
#ifndef _SVX_SIZEITEM_HXX //autogen
#define ITEMID_SIZE 0
#include <bf_svx/sizeitem.hxx>
#endif

#include <bf_svx/xlnedit.hxx>
#include <bf_svx/xlnstit.hxx>
#include <bf_svx/xlndsit.hxx>
#include <bf_svx/xlnclit.hxx>
#include <bf_svx/xlnwtit.hxx>

#include "glob.hrc"

#include "chtscene.hxx"
#include "pairs.hxx"

#include "chaxis.hxx"

namespace binfilter {

/*************************************************************************
|*
|* Achsen-Attribute ermitteln
|*
\************************************************************************/
/*N*/ ChartAxis* ChartModel::GetAxis(long nId)
/*N*/ {
/*N*/   switch(nId)
/*N*/   {
/*N*/       case CHOBJID_DIAGRAM_X_AXIS:
/*N*/           return pChartXAxis;
/*N*/       case CHOBJID_DIAGRAM_Y_AXIS:
/*N*/           return pChartYAxis;
/*N*/       case CHOBJID_DIAGRAM_Z_AXIS:
/*N*/           return pChartZAxis;
/*N*/       case CHOBJID_DIAGRAM_B_AXIS:
/*N*/           return pChartBAxis;
/*?*/       case CHOBJID_DIAGRAM_A_AXIS:
/*?*/           return pChartAAxis;
/*N*/   }
/*?*/   OSL_FAIL("ChartModel::GetAxis() illegal argument (nId=CHOBJID_)");
/*?*/   return pChartYAxis;
/*N*/ }



/*
    Get axis attributes.
    If pAxisObj is NULL then the intersection of the attributes of all five
    axes is taken, regardless of wether the axes are visible or not.
*/
/*N*/ SfxItemSet ChartModel::GetFullAxisAttr( const SdrObjGroup* pAxisObj, bool bOnlyInserted ) const
/*N*/ {
/*N*/   if (pAxisObj)
/*N*/   {
/*?*/       SchObjectId* pObjId = GetObjectId(*pAxisObj);
/*?*/
/*?*/       if (pObjId)
/*?*/       {
/*?*/           long nId=pObjId->GetObjId();
/*?*/
/*?*/           SfxItemSet aAttr(*pItemPool,nAxisWhichPairs);
/*?*/           aAttr.Put(GetAttr(nId));
/*?*/           ((ChartModel*)this)->GetAxis(nId)->GetMembersAsAttr(aAttr);
/*?*/           return aAttr;
/*?*/       }
/*N*/   }
/*N*/   else
/*N*/   {
/*N*/         // clear member item set for all axes
/*N*/       pAxisAttr->ClearItem();
/*N*/         bool bIsFirst = true;
/*N*/
/*N*/         if( ! bOnlyInserted || HasAxis( CHOBJID_DIAGRAM_X_AXIS ))
/*N*/         {
/*N*/             if( bIsFirst )
/*N*/             {
/*N*/                 pAxisAttr->Set( GetAttr( CHOBJID_DIAGRAM_X_AXIS ));
/*N*/                 bIsFirst = false;
/*N*/             }
/*N*/             else
/*?*/                 IntersectSets( GetAttr( CHOBJID_DIAGRAM_X_AXIS ), *pAxisAttr );
/*N*/         }
/*N*/
/*N*/         if( ! bOnlyInserted || HasAxis( CHOBJID_DIAGRAM_Y_AXIS ))
/*N*/         {
/*N*/             if( bIsFirst )
/*N*/             {
/*?*/                 pAxisAttr->Set( GetAttr( CHOBJID_DIAGRAM_Y_AXIS ));
/*?*/                 bIsFirst = false;
/*N*/             }
/*N*/             else
/*N*/                 IntersectSets( GetAttr( CHOBJID_DIAGRAM_Y_AXIS ), *pAxisAttr );
/*N*/         }
/*N*/
/*N*/         if( ! bOnlyInserted || (Is3DChart() && HasAxis( CHOBJID_DIAGRAM_Z_AXIS )))
/*N*/         {
/*N*/             if( bIsFirst )
/*N*/             {
/*?*/                 pAxisAttr->Set( GetAttr( CHOBJID_DIAGRAM_Z_AXIS ));
/*?*/                 bIsFirst = false;
/*N*/             }
/*N*/             else
/*N*/                 IntersectSets( GetAttr( CHOBJID_DIAGRAM_Z_AXIS ), *pAxisAttr );
/*N*/         }
/*N*/
/*N*/         if( ! bOnlyInserted || HasAxis( CHOBJID_DIAGRAM_A_AXIS ))
/*N*/         {
/*?*/             if( bIsFirst )
/*?*/             {
/*?*/                 pAxisAttr->Set( GetAttr( CHOBJID_DIAGRAM_A_AXIS ));
/*?*/                 bIsFirst = false;
/*?*/             }
/*?*/             else
/*?*/                 IntersectSets( GetAttr( CHOBJID_DIAGRAM_A_AXIS ), *pAxisAttr );
/*N*/         }
/*N*/
/*N*/         if( ! bOnlyInserted || HasAxis( CHOBJID_DIAGRAM_B_AXIS ))
/*N*/         {
/*?*/             if( bIsFirst )
/*?*/             {
/*?*/                 pAxisAttr->Set( GetAttr( CHOBJID_DIAGRAM_B_AXIS ));
/*?*/                 bIsFirst = false;
/*?*/             }
/*?*/             else
/*?*/                 IntersectSets( GetAttr( CHOBJID_DIAGRAM_B_AXIS ), *pAxisAttr );
/*N*/         }
/*N*/   }
/*N*/
/*N*/   return *pAxisAttr;
/*N*/ }

/*************************************************************************
|*
|* Achsen-Attribute aendern;
|* Liefert bei geaenderten Attributen TRUE.
|*
\************************************************************************/

/*N*/ BOOL ChartModel::ChangeAxisAttr(const SfxItemSet &rAttr,SdrObjGroup *pAxisObj,BOOL bMerge)
/*N*/ {
/*N*/   if (pAxisObj)
/*N*/   {
/*N*/       SchObjectId *pObjId = GetObjectId(*pAxisObj);
/*N*/
/*N*/       if (! pObjId) return FALSE;
/*N*/       else
/*N*/       {
/*N*/           SfxItemSet aSet(*pItemPool,nAxisWhichPairs);
/*N*/           aSet.Put(rAttr);
/*N*/
/*N*/           const SfxPoolItem *pPoolItem  = NULL;
/*N*/
/*N*/           if(rAttr.GetItemState(SID_ATTR_NUMBERFORMAT_VALUE,TRUE,&pPoolItem)==SFX_ITEM_SET)
/*N*/           {
/*N*/               UINT32 nTmp=((const SfxUInt32Item*)pPoolItem)->GetValue();
/*N*/               aSet.Put(SfxUInt32Item(IsPercentChart()
/*N*/                   ? SCHATTR_AXIS_NUMFMTPERCENT : SCHATTR_AXIS_NUMFMT , nTmp));
/*N*/           }
/*N*/
/*N*/           long nId=pObjId->GetObjId();
/*N*/           SetAttributes( nId, aSet, bMerge );             // at the model
/*N*/           SetAxisAttributes( &GetAttr( nId ), pAxisObj ); // at the drawing object
/*N*/
/*N*/             // at the axis itself!
/*N*/             long nAxisUId = ChartAxis::GetUniqueIdByObjectId( nId );
/*N*/             if( nAxisUId != CHAXIS_AXIS_UNKNOWN )
/*N*/             {
/*N*/                 // this method always returns a valid pointer
/*N*/                 GetAxisByUID( nAxisUId )->SetAttributes( rAttr );
/*N*/             }
/*N*/             else
/*N*/                 OSL_FAIL( "ChartAxis not found for Object" );
/*N*/
/*N*/           BuildChart(FALSE,nId); //z.B. auch Texte skalieren!
/*N*/           return TRUE;
/*N*/       }
/*N*/   }
/*N*/   else
/*N*/   {
            return FALSE;
/*N*/   }
/*N*/ }

/*************************************************************************
|*
|* Datenreihen-Attribute setzen
|*
\************************************************************************/
/*N*/ void ChartModel::PutDataRowAttrAll(const SfxItemSet& rAttr,BOOL bMerge,BOOL bClearPoints)
/*N*/ {
/*N*/   long nCnt=aDataRowAttrList.size();
/*N*/   while(nCnt--)
/*N*/       PutDataRowAttr(nCnt,rAttr,bMerge,bClearPoints);
/*N*/ }

/*N*/ void ChartModel::PutDataRowAttr(long nRow, const SfxItemSet& rAttr,BOOL bMerge, BOOL /*bClearPoints*/)
/*N*/ {
/*N*/
/*N*/   CHART_TRACE1( "ChartModel::PutDataRowAttr %smerge", bMerge? "": "NO " );
/*N*/
/*N*/     if( aDataRowAttrList.size() <= (unsigned long)nRow )
/*N*/     {
/*?*/         OSL_FAIL( "Invalid index to array requested" );
/*?*/         return;
/*N*/     }
/*N*/
/*N*/     if(!bMerge)
/*?*/       aDataRowAttrList[ nRow ]->ClearItem();
/*N*/
/*N*/   PutItemSetWithNameCreation( *aDataRowAttrList[ nRow ], rAttr );
/*N*/ }

/*************************************************************************
|*
|* Datenreihen-Attribute ermitteln
|*
\************************************************************************/

/*N*/ const SfxItemSet& ChartModel::GetDataRowAttr( long nRow ) const
/*N*/ {
/*N*/   if( nRow < (long)aDataRowAttrList.size() )
/*N*/   {
/*N*/       SfxItemSet* pSet = aDataRowAttrList[ nRow ];
/*N*/       DBG_ASSERT( pSet, "Invalid ItemSet" );
/*N*/       return *pSet;
/*N*/   }
/*N*/   else
/*N*/   {
/*N*/       OSL_FAIL( "Requested data row attribute is unavailable" );
/*N*/
/*N*/       // return something
/*?*/       DBG_ASSERT( pChartAttr, "Invalid Chart-ItemSet" );
/*?*/       return *pChartAttr;
/*N*/   }
/*N*/ }

/*************************************************************************
|*
|* Erstelle Symbole fuer Diagrammtypen mit Symbolen
|*
\************************************************************************/

/*N*/ SdrObject* ChartModel::CreateSymbol (Point      aPoint,
/*N*/                                    int        nRow,
/*N*/                                    int        nColumn,
/*N*/                                    SfxItemSet &aDataAttr,
/*N*/                                    long       nSymbolSize,
/*N*/                                    BOOL       bInsert)
/*N*/ {
/*N*/   long nWhatSymbol=SVX_SYMBOLTYPE_AUTO;
/*N*/   SdrObject* pObj=NULL;
/*N*/   long nHalfSymbolSizeX = nSymbolSize / 4;
/*N*/   long nHalfSymbolSizeY = nSymbolSize / 4;
/*N*/   Size aSize(1,1);
/*N*/   long n1Pixel=pRefOutDev ? pRefOutDev->PixelToLogic(aSize).Width() : nHalfSymbolSizeY/2;
/*N*/
/*N*/   if(!bInsert)
/*N*/       n1Pixel=nHalfSymbolSizeY/2; //für Legende vergrößern
/*N*/
/*N*/   const SfxPoolItem* pPoolItem;
/*N*/   if(aDataAttr.GetItemState(SCHATTR_STYLE_SYMBOL, TRUE, &pPoolItem) == SFX_ITEM_SET)
/*N*/   {
/*N*/       nWhatSymbol = ((const SfxInt32Item*) pPoolItem)->GetValue();
/*N*/   }
/*N*/   if(nWhatSymbol<0)
/*N*/   {
/*N*/       BOOL bStock=FALSE;
/*N*/       switch(nWhatSymbol)
/*N*/       {
/*N*/           case SVX_SYMBOLTYPE_NONE:
/*N*/           {
/*N*/               //  Create a square as symbol.  Its fill and line style is
/*N*/               //  later to invisible.  This does not show the symbol but
/*N*/               //  leaves the data point selectable.
/*N*/               XPolygon aPolygon (5);
/*N*/               aPolygon[0].X() = aPoint.X() - nHalfSymbolSizeX;
/*N*/               aPolygon[0].Y() = aPoint.Y() - nHalfSymbolSizeY;
/*N*/               aPolygon[1].X() = aPoint.X() - nHalfSymbolSizeX;
/*N*/               aPolygon[1].Y() = aPoint.Y() + nHalfSymbolSizeY;
/*N*/               aPolygon[2].X() = aPoint.X() + nHalfSymbolSizeX;
/*N*/               aPolygon[2].Y() = aPoint.Y() + nHalfSymbolSizeY;
/*N*/               aPolygon[3].X() = aPoint.X() + nHalfSymbolSizeX;
/*N*/               aPolygon[3].Y() = aPoint.Y() - nHalfSymbolSizeY;
/*N*/               aPolygon[4].X() = aPolygon[0].X();
/*N*/               aPolygon[4].Y() = aPolygon[0].Y();
/*N*/               pObj = new SdrPathObj(OBJ_POLY, XPolyPolygon(aPolygon));
/*N*/               break;
/*N*/           }
/*N*/           case SVX_SYMBOLTYPE_UNKNOWN:
/*N*/           case SVX_SYMBOLTYPE_AUTO:
/*N*/               CHART_TRACE( "creating SVX_SYMBOLTYPE_AUTO" );
/*N*/
/*N*/
/*N*/               switch(eChartStyle)
/*N*/               {
/*N*/                       default:
/*N*/                       break;
/*N*/                       case CHSTYLE_2D_STOCK_2:
/*N*/                       case CHSTYLE_2D_STOCK_4:
/*N*/                           bStock=TRUE;
/*N*/                           nHalfSymbolSizeX=n1Pixel;
/*N*/                           break;
/*N*/                       case CHSTYLE_2D_STOCK_1:
/*N*/                           bStock=TRUE;
/*N*/                           if(nRow!=2)
/*N*/                               nHalfSymbolSizeX=n1Pixel;
/*N*/                           break;
/*N*/                       case CHSTYLE_2D_STOCK_3:
/*?*/                           bStock=TRUE;
/*?*/                           if(nRow!=3)
/*?*/                               nHalfSymbolSizeX=n1Pixel;
/*N*/                           break;
/*N*/               }
/*N*/               if(bStock)
/*N*/               {
/*N*/                   XPolygon aPolygon(2);
/*N*/
/*N*/                   aPolygon[0].X() = aPoint.X();
/*N*/                   aPolygon[0].Y() = aPoint.Y();
/*N*/                   aPolygon[1].X() = aPoint.X() + nHalfSymbolSizeX;
/*N*/                   aPolygon[1].Y() = aPoint.Y();
/*N*/                   pObj = new SdrPathObj(OBJ_POLY, XPolyPolygon(aPolygon));
/*N*/                   break;
/*N*/               }
/*N*/               nWhatSymbol=nRow;
/*N*/               aDataAttr.ClearItem(SCHATTR_SYMBOL_BRUSH);//Größe gegebenenfalls löschen
/*N*/               aDataAttr.ClearItem(SCHATTR_SYMBOL_SIZE);//Größe gegebenenfalls löschen
/*N*/               break;
/*N*/
/*N*/           case SVX_SYMBOLTYPE_BRUSHITEM:
/*N*/           {
/*?*/               if(aDataAttr.GetItemState(SCHATTR_SYMBOL_BRUSH,TRUE,&pPoolItem)==SFX_ITEM_SET)
/*?*/               {
/*?*/                   CHART_TRACE( "creating SVX_SYMBOLTYPE_BRUSHITEM" );
/*?*/                   const Graphic*  pGraphic = ((const SvxBrushItem *)pPoolItem)->GetGraphic();
/*?*/                   Size            aLclSize;
/*?*/
/*?*/                   if( pGraphic )
/*?*/                   {
/*?*/                       if(!pObj && aDataAttr.GetItemState(SCHATTR_SYMBOL_SIZE,TRUE,&pPoolItem)==SFX_ITEM_SET)
/*?*/                       {
/*?*/                           CHART_TRACE( "Size by Item" );
/*?*/                           aLclSize=((const SvxSizeItem*)pPoolItem)->GetSize();
/*?*/                       }
/*?*/                       else
/*?*/                       {
/*?*/                           CHART_TRACE( "Size by Graphic" );
/*?*/                           if( pGraphic )
/*?*/                               aLclSize = ( OutputDevice::LogicToLogic( pGraphic->GetPrefSize(),
/*?*/                                                                     pGraphic->GetPrefMapMode(),
/*?*/                                                                     MAP_100TH_MM ));
/*?*/                       }
/*?*/
/*?*/                       Rectangle   aRect(aPoint.X()-aLclSize.Width()/2,aPoint.Y()-aLclSize.Height()/2
/*?*/                                         ,aPoint.X()+aLclSize.Width()/2,aPoint.Y()+aLclSize.Height()/2);
/*?*/                       pObj      = new SdrGrafObj(*pGraphic);
/*?*/                       GetPage(0)->NbcInsertObject(pObj,0);
/*?*/                       pObj->NbcSetSnapRect(aRect);
/*?*/                       GetPage(0)->RemoveObject(0);
/*?*/                   }
/*?*/               }
/*?*/               else
/*?*/               {
/*?*/                   CHART_TRACE( "failed in SVX_SYMBOLTYPE_BRUSHITEM" );
/*?*/                   nWhatSymbol=nRow;
/*?*/               }
/*N*/           }
/*N*/           break;
/*N*/       }
/*N*/   }
/*N*/   else
/*N*/   {
/*?*/       CHART_TRACE1( "creating SVX_SYMBOLTYPE = %d", nWhatSymbol );
/*N*/   }
/*N*/
/*N*/
/*N*/   if(!pObj && aDataAttr.GetItemState(SCHATTR_SYMBOL_SIZE,TRUE,&pPoolItem)==SFX_ITEM_SET)
/*N*/   {
/*?*/       Size aLclSize=((const SvxSizeItem*)pPoolItem)->GetSize();
/*?*/       nHalfSymbolSizeX = aLclSize.Width() / 2;
/*?*/       nHalfSymbolSizeY = aLclSize.Height() / 2;
/*?*/       CHART_TRACE2( "reading SCHATTR_SYMBOL_SIZE -> Size = (%ld, %ld)", aLclSize.Width(), aLclSize.Height() );
/*N*/   }
/*N*/
/*N*/   if(!pObj) //dann default generieren
/*N*/   {
/*N*/       switch (nWhatSymbol % 8)
/*N*/       {
/*N*/           case 0 :
/*N*/           {
/*N*/               XPolygon aPolygon (5);
/*N*/
/*N*/               aPolygon[0].X() = aPoint.X() - nHalfSymbolSizeX;
/*N*/               aPolygon[0].Y() = aPoint.Y() - nHalfSymbolSizeY;
/*N*/               aPolygon[1].X() = aPoint.X() - nHalfSymbolSizeX;
/*N*/               aPolygon[1].Y() = aPoint.Y() + nHalfSymbolSizeY;
/*N*/               aPolygon[2].X() = aPoint.X() + nHalfSymbolSizeX;
/*N*/               aPolygon[2].Y() = aPoint.Y() + nHalfSymbolSizeY;
/*N*/               aPolygon[3].X() = aPoint.X() + nHalfSymbolSizeX;
/*N*/               aPolygon[3].Y() = aPoint.Y() - nHalfSymbolSizeY;
/*N*/               aPolygon[4].X() = aPolygon[0].X();
/*N*/               aPolygon[4].Y() = aPolygon[0].Y();
/*N*/
/*N*/               pObj = new SdrPathObj(OBJ_POLY, XPolyPolygon(aPolygon));
/*N*/               break;
/*N*/           }
/*N*/
/*N*/           case 1 :
/*N*/           {
/*N*/               XPolygon aPolygon (5);
/*N*/
/*N*/               aPolygon[0].X() = aPoint.X();
/*N*/               aPolygon[0].Y() = aPoint.Y() - nHalfSymbolSizeY;
/*N*/               aPolygon[1].X() = aPoint.X() + nHalfSymbolSizeX;
/*N*/               aPolygon[1].Y() = aPoint.Y();
/*N*/               aPolygon[2].X() = aPoint.X();
/*N*/               aPolygon[2].Y() = aPoint.Y() + nHalfSymbolSizeY;;
/*N*/               aPolygon[3].X() = aPoint.X() - nHalfSymbolSizeX;
/*N*/               aPolygon[3].Y() = aPoint.Y();
/*N*/               aPolygon[4].X() = aPolygon[0].X();
/*N*/               aPolygon[4].Y() = aPolygon[0].Y();
/*N*/
/*N*/               pObj = new SdrPathObj(OBJ_POLY, XPolyPolygon(aPolygon));
/*N*/               break;
/*N*/           }
/*N*/
/*N*/           case 2 :
/*N*/           {
/*N*/               XPolygon aPolygon (4);
/*N*/
/*N*/               aPolygon[0].X() = aPoint.X() - nHalfSymbolSizeX;
/*N*/               aPolygon[0].Y() = aPoint.Y() - nHalfSymbolSizeY;
/*N*/               aPolygon[1].X() = aPoint.X() + nHalfSymbolSizeX;
/*N*/               aPolygon[1].Y() = aPoint.Y() - nHalfSymbolSizeY;
/*N*/               aPolygon[2].X() = aPoint.X();
/*N*/               aPolygon[2].Y() = aPoint.Y() + nHalfSymbolSizeY;
/*N*/               aPolygon[3].X() = aPolygon[0].X();
/*N*/               aPolygon[3].Y() = aPolygon[0].Y();
/*N*/
/*N*/               pObj = new SdrPathObj(OBJ_POLY, XPolyPolygon(aPolygon));
/*N*/               break;
/*N*/           }
/*N*/
/*N*/           case 3 :
/*N*/           {
/*N*/               XPolygon aPolygon (4);
/*N*/
/*N*/               aPolygon[0].X() = aPoint.X() - nHalfSymbolSizeX;
/*N*/               aPolygon[0].Y() = aPoint.Y() + nHalfSymbolSizeY;
/*N*/               aPolygon[1].X() = aPoint.X() + nHalfSymbolSizeX;
/*N*/               aPolygon[1].Y() = aPoint.Y() + nHalfSymbolSizeY;
/*N*/               aPolygon[2].X() = aPoint.X();
/*N*/               aPolygon[2].Y() = aPoint.Y() - nHalfSymbolSizeY;
/*N*/               aPolygon[3].X() = aPolygon[0].X();
/*N*/               aPolygon[3].Y() = aPolygon[0].Y();
/*N*/
/*N*/               pObj = new SdrPathObj(OBJ_POLY, XPolyPolygon(aPolygon));
/*N*/               break;
/*N*/           }
/*N*/
/*N*/           case 4 :
/*N*/           {
/*?*/               XPolygon aPolygon (4);
/*?*/
/*?*/               aPolygon[0].X() = aPoint.X() - nHalfSymbolSizeX;
/*?*/               aPolygon[0].Y() = aPoint.Y() - nHalfSymbolSizeY;
/*?*/               aPolygon[1].X() = aPoint.X() + nHalfSymbolSizeX;
/*?*/               aPolygon[1].Y() = aPoint.Y();
/*?*/               aPolygon[2].X() = aPoint.X() - nHalfSymbolSizeX;
/*?*/               aPolygon[2].Y() = aPoint.Y() + nHalfSymbolSizeY;
/*?*/               aPolygon[3].X() = aPolygon[0].X();
/*?*/               aPolygon[3].Y() = aPolygon[0].Y();
/*?*/
/*?*/               pObj = new SdrPathObj(OBJ_POLY, XPolyPolygon(aPolygon));
/*?*/               break;
/*?*/           }
/*?*/
/*?*/           case 5 :
/*?*/           {
/*?*/               XPolygon aPolygon (4);
/*?*/
/*?*/               aPolygon[0].X() = aPoint.X() + nHalfSymbolSizeX;
/*?*/               aPolygon[0].Y() = aPoint.Y() - nHalfSymbolSizeY;
/*?*/               aPolygon[1].X() = aPoint.X() - nHalfSymbolSizeX;
/*?*/               aPolygon[1].Y() = aPoint.Y();
/*?*/               aPolygon[2].X() = aPoint.X() + nHalfSymbolSizeX;
/*?*/               aPolygon[2].Y() = aPoint.Y() + nHalfSymbolSizeY;
/*?*/               aPolygon[3].X() = aPolygon[0].X();
/*?*/               aPolygon[3].Y() = aPolygon[0].Y();
/*?*/
/*?*/               pObj = new SdrPathObj(OBJ_POLY, XPolyPolygon(aPolygon));
/*?*/               break;
/*?*/           }
/*?*/
/*?*/           case 6 :
/*?*/           {
/*?*/               XPolygon aPolygon (5);
/*?*/
/*?*/               aPolygon[0].X() = aPoint.X() - nHalfSymbolSizeX;
/*?*/               aPolygon[0].Y() = aPoint.Y() - nHalfSymbolSizeY;
/*?*/               aPolygon[1].X() = aPoint.X() + nHalfSymbolSizeX;
/*?*/               aPolygon[1].Y() = aPoint.Y() + nHalfSymbolSizeY;
/*?*/               aPolygon[2].X() = aPoint.X() + nHalfSymbolSizeX;
/*?*/               aPolygon[2].Y() = aPoint.Y() - nHalfSymbolSizeY;
/*?*/               aPolygon[3].X() = aPoint.X() - nHalfSymbolSizeX;
/*?*/               aPolygon[3].Y() = aPoint.Y() + nHalfSymbolSizeY;
/*?*/               aPolygon[4].X() = aPolygon[0].X();
/*?*/               aPolygon[4].Y() = aPolygon[0].Y();
/*?*/
/*?*/               pObj = new SdrPathObj(OBJ_POLY, XPolyPolygon(aPolygon));
/*?*/               break;
/*?*/           }
/*?*/
/*?*/           case 7 :
/*?*/           {
/*?*/               XPolygon aPolygon (5);
/*?*/
/*?*/               aPolygon[0].X() = aPoint.X() - nHalfSymbolSizeX;
/*?*/               aPolygon[0].Y() = aPoint.Y() - nHalfSymbolSizeY;
/*?*/               aPolygon[1].X() = aPoint.X() + nHalfSymbolSizeX;
/*?*/               aPolygon[1].Y() = aPoint.Y() + nHalfSymbolSizeY;
/*?*/               aPolygon[2].X() = aPoint.X() - nHalfSymbolSizeX;
/*?*/               aPolygon[2].Y() = aPoint.Y() + nHalfSymbolSizeY;
/*?*/               aPolygon[3].X() = aPoint.X() + nHalfSymbolSizeX;
/*?*/               aPolygon[3].Y() = aPoint.Y() - nHalfSymbolSizeY;
/*?*/               aPolygon[4].X() = aPolygon[0].X();
/*?*/               aPolygon[4].Y() = aPolygon[0].Y();
/*?*/
/*?*/               pObj = new SdrPathObj(OBJ_POLY, XPolyPolygon(aPolygon));
/*?*/               break;
/*?*/           }
/*?*/
/*N*/       }
/*N*/   }
/*N*/
/*N*/   if (bInsert)  //nur in CreateLegend FALSE!
/*N*/   {
/*N*/       SfxItemSet aSymbolAttr(aDataAttr);
/*N*/       GenerateSymbolAttr(aSymbolAttr,nRow,SYMBOLMODE_ROW);
/*N*/       if (nWhatSymbol == SVX_SYMBOLTYPE_NONE)
/*N*/       {
/*N*/           //  Don´t show the symbol.  It is only needed for selection of the
/*N*/           //  data point.
/*N*/           aSymbolAttr.Put(XFillStyleItem (XFILL_NONE));
/*N*/           aSymbolAttr.Put(XLineStyleItem (XLINE_NONE));
/*N*/       }
            if( pObj )
            {
/*N*/           pObj->SetModel( this );
/*N*/           pObj = SetObjectAttr (pObj, CHOBJID_DIAGRAM_DATA, TRUE, TRUE, &aSymbolAttr);
/*N*/           pObj->InsertUserData(new SchDataPoint(nColumn, nRow));
            }
/*N*/
/*N*/   }
/*N*/
/*N*/   return pObj;
/*N*/ }

/*N*/ void ChartModel::GenerateSymbolAttr(/*const SfxItemSet& rAttr,*/SfxItemSet& rSymbolAttr,const long nRow,const long nMode)
/*N*/ {
/*N*/   // Symbole immer gleiche Umrandung und Füllfarbe = Linienfarbe, wenn es sich um ein "echtes"
/*N*/   // Symbol handelt (d.h. nicht ein Symbol für Bars/Säulen,Pies etc.)
/*N*/
/*N*/   switch(nMode)
/*N*/   {
/*N*/       case SYMBOLMODE_LEGEND:
/*N*/       case SYMBOLMODE_ROW:
/*N*/
/*N*/           if(HasSymbols(nRow) && IsLine(nRow)) //Symbol und Linie => ein echtes Symbol wird angezeigt :)
/*N*/           {
/*N*/               rSymbolAttr.Put(XFillColorItem(String(),
/*N*/                                              ((XLineColorItem &)rSymbolAttr.Get(XATTR_LINECOLOR)).GetValue()));
/*N*/
/*N*/               rSymbolAttr.Put(XLineStyleItem(XLINE_SOLID));
/*N*/               rSymbolAttr.Put(XLineColorItem(String(),RGBColor(COL_BLACK)));
/*N*/               rSymbolAttr.Put(XLineWidthItem (0));
/*N*/
/*N*/           }
/*N*/           break;
/*N*/
/*N*/       case SYMBOLMODE_DESCRIPTION:
/*?*/           if(IsLine(nRow))
/*?*/           {
/*?*/               rSymbolAttr.Put(XFillColorItem(String(),
/*?*/                                              ((XLineColorItem &)rSymbolAttr.Get(XATTR_LINECOLOR)).GetValue()));
/*?*/
/*?*/               rSymbolAttr.Put(XLineStyleItem(XLINE_SOLID));
/*?*/               rSymbolAttr.Put(XLineColorItem(String(),RGBColor(COL_BLACK)));
/*?*/               rSymbolAttr.Put(XLineWidthItem (0));
/*?*/           }
/*?*/           else        // rectangle with border, if linesytle is NONE
/*?*/           {
/*?*/               XLineStyle eLineStyle =
/*?*/                   (static_cast< const XLineStyleItem * >( &(rSymbolAttr.Get( XATTR_LINESTYLE )) ))->GetValue(); // bug in Win-C++ compiler: casting to pointer
/*?*/
/*?*/               if( eLineStyle == XLINE_NONE )            // clear items for defaults to take effect
/*?*/               {
/*?*/                   rSymbolAttr.ClearItem( XATTR_LINESTYLE );
/*?*/                   rSymbolAttr.ClearItem( XATTR_LINEWIDTH );
/*?*/                   rSymbolAttr.ClearItem( XATTR_LINECOLOR );
/*?*/               }
/*N*/           }
/*N*/
/*N*/           break;
/*N*/       case SYMBOLMODE_LINE:
/*N*/           break;
/*N*/   }
/*N*/
/*N*/ }

/*************************************************************************
|*
|* Datenpunkt-Attribute setzen
|*
\************************************************************************/

/*N*/ void ChartModel::PutDataPointAttr( long nCol, long nRow,const SfxItemSet& rAttr, BOOL bMerge /*=TRUE*/)
/*N*/ {
/*N*/   CHART_TRACE( "ChartModel::PutDataPointAttr" );
/*N*/   DBG_ITEMS((SfxItemSet&)rAttr,this);
/*N*/
/*N*/   ItemSetList* pAttrList = IsDataSwitched()   //abhaengig vom Charttyp - statt bSwitchData
/*N*/                          ? &aSwitchDataPointAttrList
/*N*/                          : &aDataPointAttrList;
/*N*/
/*N*/   SfxItemSet* pItemSet = (*pAttrList)[ nCol * GetRowCount() + nRow ];
/*N*/   if (pItemSet == NULL)
/*N*/   {
/*N*/       pItemSet = new SfxItemSet(*pItemPool, nRowWhichPairs);
/*N*/       (*pAttrList)[nCol * GetRowCount() + nRow] = pItemSet;
/*N*/   }
/*N*/   if(!bMerge)
/*?*/       pItemSet->ClearItem();
/*N*/     PutItemSetWithNameCreation( *pItemSet, rAttr );
/*N*/ }

// this method exists in analogy to GetFullDataPointAttr
// it is necessary for API (=>XML) and pie charts

/*************************************************************************
|*
|* Datenpunkt-Attribute ermitteln
|*
\************************************************************************/

/*N*/ const SfxItemSet& ChartModel::GetDataPointAttr( long nCol, long nRow) const
/*N*/ {
/*N*/   long nIdx = nCol * GetRowCount() + nRow;
/*N*/   const ItemSetList* aAttrList = IsDataSwitched()
/*N*/       ? &aSwitchDataPointAttrList
/*N*/       : &aDataPointAttrList;
/*N*/
/*N*/   if( nIdx < (long)aAttrList->size() )
/*N*/   {
/*N*/       SfxItemSet* pSet = (*aAttrList)[ nIdx ];
/*N*/       if (pSet == NULL)
/*N*/           return (GetDataRowAttr(nRow));
/*N*/       else
/*N*/           return *pSet;
/*N*/   }
/*N*/   else
/*N*/   {
/*?*/       OSL_FAIL( "Requested data point attribute is unavailable" );
/*?*/
/*?*/       // return something
/*?*/       DBG_ASSERT( pChartAttr, "Invalid Chart-ItemSet" );
/*?*/       return *pChartAttr;
/*N*/   }
/*N*/ }


/*N*/ const SfxItemSet * ChartModel::GetRawDataPointAttr    (long nCol,long nRow) const
/*N*/ {
/*N*/   long nIndex = nCol * GetRowCount() + nRow;
/*N*/   const ItemSetList* aAttrList = IsDataSwitched()
/*N*/       ? &aSwitchDataPointAttrList
/*N*/       : &aDataPointAttrList;
/*N*/
/*N*/   if (nIndex < (long)aAttrList->size())
/*N*/       return (*aAttrList)[ nIndex ];
/*N*/   else
/*N*/       return NULL;
/*N*/ }

/*************************************************************************
|*
|* Datenpunkt-Attribute ermitteln
|*
\************************************************************************/

/*N*/ SfxItemSet ChartModel::GetFullDataPointAttr( long nCol, long nRow ) const
/*N*/ {
/*N*/   const ItemSetList* pAttrList = IsDataSwitched()
/*N*/         ? &aSwitchDataPointAttrList
/*N*/         : &aDataPointAttrList;
/*N*/
/*N*/   if( !IsPieChart())
/*N*/   {
/*N*/         // get series' attributes and merge with data-point attributes if available
/*N*/       SfxItemSet aAttr( GetDataRowAttr( nRow ));
/*N*/       SfxItemSet *pObj= (*pAttrList)[ nCol * GetRowCount() + nRow ];
/*N*/       if( pObj )
/*N*/             aAttr.Put( *pObj );
/*N*/       return aAttr;
/*N*/   }

    /* the following code is for pie charts only

       data description attributes (SCHATTR_DATADESCR_DESCR and
       SCHATTR_DATADESCR_SHOW_SYM) are contained in data point item set while
       all other (graphical) attributes are contained in data series (data row)
       item set */

    // get data description attributes
/*N*/     SfxItemSet aDescrAttrib( *pItemPool, SCHATTR_DATADESCR_START, SCHATTR_DATADESCR_END );
/*N*/     aDescrAttrib.Put( GetDataRowAttr( 0 )); // a pie chart always has only one series
/*N*/
/*N*/     // the different pies use series attributes rather than data-point attributes
/*N*/     long nPieCount        = GetColCount();
/*N*/     long nSecondDimension = GetRowCount(); // pie has only one series, but the data may contain more
/*N*/
/*N*/   SfxItemSet aAttr( GetDataRowAttr( nCol % nPieCount ));
/*N*/
/*N*/     if( ( nCol >= nPieCount ) &&
/*N*/         pDefaultColors )
/*N*/     {
/*?*/         XColorEntry* pColEntry = (*pDefaultColors)[ nCol % pDefaultColors->size() ];
/*?*/         aAttr.Put( XFillColorItem( pColEntry->GetName(), pColEntry->GetColor()));
/*N*/     }
/*N*/
/*N*/     // add description attributes of series
/*N*/     aAttr.ClearItem( SCHATTR_DATADESCR_DESCR );
/*N*/     aAttr.ClearItem( SCHATTR_DATADESCR_SHOW_SYM );
/*N*/     aAttr.Put( aDescrAttrib );
/*N*/
/*N*/     SfxItemSet* pAttr = (*pAttrList)[ nCol * nSecondDimension ];
/*N*/     if( ( pAttr != NULL ) && pAttr->Count() )
/*N*/         aAttr.Put( *pAttr );
/*N*/
/*N*/     return aAttr;
/*N*/ }

/*************************************************************************
|*
|* Datenpunkt-Attribute ermitteln
|*
\************************************************************************/

/*N*/ SfxItemSet& ChartModel::MergeDataPointAttr( SfxItemSet& rAttr, long nCol, long nRow ) const
/*N*/ {
/*N*/   CHART_TRACE2( "ChartModel::MergeDataPointAttr nCol=%ld, nRow=%ld", nCol, nRow );
/*N*/   const ItemSetList* pAttrList = IsDataSwitched()   //abhaengig vom Charttyp - statt bSwitchData
/*N*/                           ? &aSwitchDataPointAttrList
/*N*/                           : &aDataPointAttrList;
/*N*/
/*N*/   SfxItemSet *pObj = (*pAttrList)[ nCol * GetRowCount() + nRow ];
/*N*/   if(pObj)
/*?*/       rAttr.Put(*pObj);
/*N*/   return rAttr;
/*N*/ }


/*N*/ BOOL  ChartModel::IsDataPointAttrSet  (long nCol, long nRow)  const
/*N*/ {
/*N*/   UINT32 nIndex = nCol * GetRowCount() + nRow;
/*N*/   const ItemSetList* aAttrList = IsDataSwitched()
/*N*/                                  ? &aSwitchDataPointAttrList
/*N*/                                  : &aDataPointAttrList;
/*N*/
/*N*/   if (nIndex < static_cast<UINT32>(aAttrList->size()))
/*N*/       return (*aAttrList)[ nIndex ] != NULL;
/*N*/   else
/*N*/       //  Specified data point does not exist.  Therefore an item set does not exist also.
/*?*/       return false;
/*N*/ }


/*************************************************************************
|*
|* Aendere die Attribute einer Achse
|*
\************************************************************************/

/*N*/ void ChartModel::SetAxisAttributes (const SfxItemSet* pAttr,const SdrObjGroup* pAxisObj)
/*N*/ {
/*N*/   if (pAttr && pAxisObj)
/*N*/   {
/*N*/       SfxItemSet aTextAttr(*pItemPool, nTextWhichPairs);
/*N*/
/*N*/       aTextAttr.Put(*pAttr);
/*N*/
/*N*/       SdrObjListIter aIterator(*pAxisObj->GetSubList(), IM_FLAT);
/*N*/       while (aIterator.IsMore())
/*N*/       {
/*N*/           SdrObject   *pObj   = aIterator.Next();
/*N*/           SchObjectId *pObjId = GetObjectId(*pObj);
/*N*/
/*N*/           if (pObjId)
/*N*/               switch (pObjId->GetObjId())
/*N*/               {
/*N*/                   case CHOBJID_LINE :
/*N*/                       pObj->SetItemSetAndBroadcast(*pAttr);
/*N*/                       break;
/*N*/
/*N*/                   case CHOBJID_TEXT :
/*?*/                       SetTextAttr(*(SdrTextObj*)pObj, aTextAttr);
/*N*/
/*N*/                   default :
/*N*/                       ;
/*N*/               }
/*N*/       }
/*N*/   }
/*N*/ }

/*************************************************************************
|*
|* Aendere die Attribute einer Achse
|*
\************************************************************************/


/*N*/ void ChartModel::SetTextFromObject( SdrTextObj* pObj,OutlinerParaObject* pTextObject )
/*N*/ {
/*N*/   DBG_ASSERT( pObj, "ChartModel::SetTextFromObject: Object is NULL" );
/*N*/   if( (!bAttrAutoStorage && pTextObject) // not during BuildChart
/*N*/       || !pObj )
/*N*/       return;
/*N*/
/*N*/   CHART_TRACE( "ChartModel::SetTextFromObject" );
/*N*/
/*N*/   SchObjectId* pObjId = GetObjectId( *pObj );
/*N*/
/*N*/   String* pStrToChange = NULL;
/*N*/
/*N*/   if( pTextObject )
/*N*/   {
/*N*/       if( pObjId )
/*N*/       {
/*N*/           pOutliner->SetText( *pTextObject );
/*N*/           String aTitle = pOutliner->GetText( pOutliner->GetParagraph( 0 ), pOutliner->GetParagraphCount() );
/*N*/           pOutliner->Clear();
/*N*/
/*N*/           long nId = pObjId->GetObjId();
/*N*/           switch( nId )
/*N*/           {
/*N*/               case CHOBJID_DIAGRAM_TITLE_X_AXIS:
/*N*/                   pStrToChange = &aXAxisTitle;
/*N*/                   break;
/*N*/               case CHOBJID_DIAGRAM_TITLE_Y_AXIS:
/*N*/                   pStrToChange = &aYAxisTitle;
/*N*/                   break;
/*N*/               case CHOBJID_DIAGRAM_TITLE_Z_AXIS:
/*N*/                   pStrToChange = &aZAxisTitle;
/*N*/                   break;
/*N*/               case CHOBJID_TITLE_MAIN:
/*N*/                   pStrToChange = &aMainTitle;
/*N*/                   break;
/*N*/               case CHOBJID_TITLE_SUB:
/*N*/                   pStrToChange = &aSubTitle;
/*N*/                   break;
/*N*/           }
/*N*/
/*N*/           if( pStrToChange )
/*N*/               *(pStrToChange) = aTitle;
/*N*/       }
/*N*/   }
/*N*/ }
/*N*/
/*N*/ //Achtung! diese Funktion sollte nur für 3-D-Objekte ausgeführt werden,
/*N*/ //bei z.B. Legendensymbolen führt der Aufruf zu einer Endlos-Schleife !!!!!!!!!!!!!
/*N*/ //Im BuildChart wird ueber bAttrAutoStorage=FALSE doppelte Ausfuehrung unterbunden
/*N*/ void ChartModel::StoreObjectsAttributes(SdrObject* pObj,const SfxItemSet& rAttr, bool bReplaceAll)
/*N*/ {//#52277#
/*N*/   if(!bAttrAutoStorage)
/*N*/       return;
/*N*/   bAttrAutoStorage=FALSE; //Rekursionen verhindern
/*N*/
/*N*/   DBG_ITEMS(((SfxItemSet&)rAttr),this);
/*N*/
/*N*/   SchObjectId* pObjId = GetObjectId(*pObj);
/*N*/   if(!pObjId)
/*N*/   {
/*?*/       OSL_FAIL("ChartModel::StoreObjectsAttributes failed, no ObjId");
/*N*/   }
/*N*/   long nId=pObjId->GetObjId();
/*N*/
/*N*/   switch(nId)
/*N*/   {
/*N*/
/*N*/   case CHOBJID_DIAGRAM_SPECIAL_GROUP:
/*N*/   case CHOBJID_DIAGRAM_ROWGROUP:
/*N*/   case CHOBJID_LEGEND_SYMBOL_ROW:
/*?*/       break;
/*?*/
/*?*/   case CHOBJID_DIAGRAM_DATA:
/*?*/   case CHOBJID_LEGEND_SYMBOL_COL:
/*?*/       break;
/*?*/
/*?*/   case CHOBJID_DIAGRAM_STATISTICS_GROUP :
/*?*/   case CHOBJID_DIAGRAM_AVERAGEVALUE :
/*?*/   case CHOBJID_DIAGRAM_REGRESSION :
/*?*/       break;
/*?*/
/*N*/   case CHOBJID_DIAGRAM_WALL:
/*N*/       {
/*N*/           //Spezialfall, 2.Wand suchen
/*N*/           ChartScene* pLclScene=GetScene();
/*N*/           if(pLclScene)
/*N*/           {
/*N*/               SdrObjListIter aIterator(*pLclScene->GetSubList(), IM_FLAT);
/*N*/               while (aIterator.IsMore())
/*N*/               {
/*N*/                   SdrObject   *pO   = aIterator.Next();
/*N*/                   SchObjectId *pI   = GetObjectId(*pO);
/*N*/                   if(pI && pI->GetObjId()==CHOBJID_DIAGRAM_WALL && pO!=pObj)
/*N*/                       pO->SetItemSetAndBroadcast(rAttr);
/*N*/               }
/*N*/           }
/*N*/       }
/*N*/       if(bReplaceAll)
/*?*/           GetAttr(nId).ClearItem();
/*N*/       GetAttr(nId).Put(rAttr,TRUE);
/*N*/       break;
/*N*/   default:
/*N*/       if(bReplaceAll)
/*?*/           GetAttr(nId).ClearItem();
/*N*/       GetAttr(nId).Put(rAttr,TRUE);
/*N*/       break;
/*N*/   }
/*N*/
/*N*/   bAttrAutoStorage=TRUE;
/*N*/ }

/*************************************************************************
|*
|* Aendere die Attribute bestimmter Objekte in der Page und im Model, bzw.
|* lokalisiere bestimmte Objekte anhand der Id.
|* Wird derzeit nur fuer UNO verwendet, koennte aber allgemeiner und sollte vor
|* allem vollständiger werden
|*
|* ACHTUNG: es gibt weitere Methoden gleichen/ähnlichen Namens und anderen Argumenten,
|* die ähnlich wie diese Funktionen arbeiten!
|*
\************************************************************************/

/*N*/ void ChartModel::SetTitle(const long nId, const String& rTitle)
/*N*/ {
/*N*/   switch(nId)
/*N*/   {
/*N*/
/*N*/       case CHOBJID_TITLE_MAIN:
/*N*/           MainTitle()=rTitle;
/*N*/           //  Replace the current text object with a new one with the given text.
/*N*/           {
/*N*/               SdrObject * pGroupObject = GetObjWithId( CHOBJID_TITLE_MAIN, *GetPage(0) );
/*N*/               if (pGroupObject != NULL)
/*N*/               {
/*N*/                   aTitleTopCenter = pGroupObject->GetBoundRect().TopCenter();
/*N*/                   DeleteObject(pGroupObject);
/*N*/               }
/*N*/               SdrTextObj * pObj = CreateTextObj(CHOBJID_TITLE_MAIN, aTitleTopCenter,
/*N*/                   aMainTitle, *pMainTitleAttr, TRUE, CHADJUST_TOP_CENTER);
/*N*/               pObj->SetResizeProtect(TRUE);
/*N*/               GetPage(0)->NbcInsertObject(pObj);
/*N*/           }
/*N*/           break;
/*N*/       case CHOBJID_TITLE_SUB:
/*N*/           SubTitle()=rTitle;
/*N*/           //  Replace the current text object with a new one with the given text.
/*N*/           {
/*N*/
/*N*/               SdrObject * pGroupObject = GetObjWithId( CHOBJID_TITLE_SUB, *GetPage(0) );
/*N*/               if (pGroupObject != NULL)
/*N*/               {
/*N*/                   aSubTitleTopCenter = pGroupObject->GetBoundRect().TopCenter();
/*N*/                   DeleteObject(pGroupObject);
/*N*/               }
/*N*/               SdrTextObj * pObj = CreateTextObj(CHOBJID_TITLE_SUB, aSubTitleTopCenter,
/*N*/                   aSubTitle, *pSubTitleAttr, TRUE, CHADJUST_TOP_CENTER);
/*N*/               pObj->SetResizeProtect(TRUE);
/*N*/               GetPage(0)->NbcInsertObject(pObj);
/*N*/           }
/*N*/           break;
/*N*/       case CHOBJID_DIAGRAM_TITLE_X_AXIS:
/*N*/           XAxisTitle()=rTitle;
/*N*/           break;
/*N*/       case CHOBJID_DIAGRAM_TITLE_Y_AXIS:
/*N*/           YAxisTitle()=rTitle;
/*N*/           break;
/*N*/       case CHOBJID_DIAGRAM_TITLE_Z_AXIS:
/*N*/           ZAxisTitle()=rTitle;
/*N*/           break;
/*N*/       default:
/*N*/           break;
/*N*/   }
/*N*/ }

/*N*/ String ChartModel::GetTitle( const long nId )
/*N*/ {
/*N*/   switch(nId)
/*N*/   {
/*N*/     case CHOBJID_TITLE_MAIN:
/*N*/         return MainTitle();
/*N*/     case CHOBJID_TITLE_SUB:
/*N*/         return SubTitle();
/*N*/     case CHOBJID_DIAGRAM_TITLE_X_AXIS:
/*N*/         return XAxisTitle();
/*N*/     case CHOBJID_DIAGRAM_TITLE_Y_AXIS:
/*N*/         return YAxisTitle();
/*N*/     case CHOBJID_DIAGRAM_TITLE_Z_AXIS:
/*N*/         return ZAxisTitle();
/*N*/     default:
/*?*/         return String();
/*N*/   }
/*N*/ }

/*N*/ void ChartModel::SetHasBeenMoved(const long nId,BOOL bMoved)
/*N*/ {
/*N*/   switch(nId)
/*N*/   {
/*N*/
/*N*/       case CHOBJID_TITLE_MAIN:
/*N*/           SetMainTitleHasBeenMoved(bMoved);
/*N*/           break;
/*N*/       case CHOBJID_TITLE_SUB:
/*N*/           SetSubTitleHasBeenMoved(bMoved);
/*N*/           break;
/*N*/       case CHOBJID_DIAGRAM_TITLE_X_AXIS:
/*N*/           SetXAxisTitleHasBeenMoved(bMoved);
/*N*/           break;
/*N*/       case CHOBJID_DIAGRAM_TITLE_Y_AXIS:
/*N*/           SetYAxisTitleHasBeenMoved(bMoved);
/*N*/           break;
/*N*/       case CHOBJID_DIAGRAM_TITLE_Z_AXIS:
/*N*/           SetZAxisTitleHasBeenMoved(bMoved);
/*N*/           break;
/*N*/       case CHOBJID_LEGEND:
/*N*/           SetLegendHasBeenMoved(bMoved);
/*N*/           break;
/*N*/       default:
/*N*/           break;
/*N*/   }
/*N*/ }

/*N*/ SdrObject* ChartModel::GetObjectWithId(const long nId,const long /*nCol*/,const long /*nRow*/)
/*N*/ {
/*N*/   //ToDo: Das hier könnte man auch schöner machen (kein DEEP!)
/*N*/   SdrObject* pObj=(GetObjWithId ((USHORT)nId,*GetPage(0),0,IM_DEEPWITHGROUPS));
/*N*/   return pObj;
/*N*/ }

/*N*/ void ChartModel::SetAttributes(const long nId,const SfxItemSet& rAttr,BOOL bMerge)
/*N*/ {
/*N*/   CHART_TRACE( "ChartModel::SetAttributes" );
/*N*/   SfxItemSet& rItemSet=GetAttr(nId);
/*N*/
/*N*/   if(!bMerge)
/*N*/       rItemSet.ClearItem();

/*N*/   rItemSet.Put(rAttr);
/*N*/ }
/*N*/ SfxItemSet& ChartModel::GetAttr(const long nObjId,const long nIndex1/*=-1*/) const
/*N*/ {
/*N*/   CHART_TRACE( "ChartModel::GetAttr(const long nObjId)" );
/*N*/
/*N*/   switch(nObjId)
/*N*/   {
/*N*/       case CHOBJID_DIAGRAM_STOCKLINE_GROUP:
/*?*/           return *pStockLineAttr;
/*N*/
/*N*/       case CHOBJID_DIAGRAM_STOCKPLUS_GROUP:
/*N*/           return *pStockPlusAttr;
/*N*/
/*N*/       case CHOBJID_DIAGRAM_STOCKLOSS_GROUP:
/*N*/           return *pStockLossAttr;
/*N*/
/*N*/       case CHOBJID_DIAGRAM_X_GRID_MAIN_GROUP:
/*N*/           return *pXGridMainAttr;
/*N*/
/*N*/       case CHOBJID_DIAGRAM_Y_GRID_MAIN_GROUP:
/*N*/           return *pYGridMainAttr;
/*N*/
/*N*/       case CHOBJID_DIAGRAM_Z_GRID_MAIN_GROUP:
/*N*/           return *pZGridMainAttr;
/*N*/
/*N*/       case CHOBJID_DIAGRAM_X_GRID_HELP_GROUP:
/*?*/           return *pXGridHelpAttr;
/*N*/
/*N*/       case CHOBJID_DIAGRAM_Y_GRID_HELP_GROUP:
/*?*/           return *pYGridHelpAttr;
/*N*/
/*N*/       case CHOBJID_DIAGRAM_Z_GRID_HELP_GROUP:
/*?*/           return *pZGridHelpAttr;
/*N*/
/*N*/       case CHOBJID_DIAGRAM_AREA:
/*N*/           return *pDiagramAreaAttr;
/*N*/
/*N*/       case CHOBJID_DIAGRAM:          //Leider mmehrfache bedeutung, im 2D ist das wohl das selbe
/*N*/       case CHOBJID_DIAGRAM_WALL:
/*N*/           return *pDiagramWallAttr;
/*N*/
/*N*/       case CHOBJID_DIAGRAM_FLOOR:
/*N*/           return *pDiagramFloorAttr;
/*N*/
/*N*/       case CHOBJID_DIAGRAM_TITLE_X_AXIS:
/*N*/           return *pXAxisTitleAttr;
/*N*/       case CHOBJID_DIAGRAM_TITLE_Y_AXIS:
/*N*/           return *pYAxisTitleAttr;
/*N*/       case CHOBJID_DIAGRAM_TITLE_Z_AXIS:
/*N*/           return *pZAxisTitleAttr;
/*N*/
/*N*/       case CHOBJID_TITLE_MAIN:
/*N*/           return *pMainTitleAttr;
/*N*/       case CHOBJID_TITLE_SUB:
/*N*/           return *pSubTitleAttr;
/*N*/
/*N*/       case CHOBJID_DIAGRAM_X_AXIS:
/*N*/           return *pChartXAxis->GetItemSet();
/*N*/       case CHOBJID_DIAGRAM_Y_AXIS:
/*N*/           return *pChartYAxis->GetItemSet();
/*N*/       case CHOBJID_DIAGRAM_Z_AXIS:
/*N*/           return *pChartZAxis->GetItemSet();
/*N*/       case CHOBJID_DIAGRAM_B_AXIS:
/*N*/           return *pChartBAxis->GetItemSet();
/*N*/       case CHOBJID_DIAGRAM_A_AXIS:
/*N*/           return *pChartAAxis->GetItemSet();
/*N*/
/*N*/       case CHOBJID_LEGEND_BACK: //Achtung, dies ist nur das Drawing-Objekt, normalerweise erhält man die andere ID für das Gruppenobjekt
/*N*/       case CHOBJID_LEGEND:
/*N*/           return *pLegendAttr;
/*N*/
/*N*/       case CHOBJID_DIAGRAM_REGRESSION:
/*?*/           return *aRegressAttrList[ nIndex1 ];
/*N*/       case CHOBJID_DIAGRAM_ERROR:
/*?*/           return *aErrorAttrList[ nIndex1 ];
/*N*/       case CHOBJID_DIAGRAM_AVERAGEVALUE:
/*?*/           return *aAverageAttrList[ nIndex1 ];
/*N*/
/*N*/       default:
/*N*/           CHART_TRACE1( "GetAttr illegal Object Id (%ld), returning dummy", nObjId );
/*N*/           return *pDummyAttr;
/*N*/   }
/*N*/ }


//  This method fills rAttr with the items stored at the object specified
//  by nObjId.  Effectively calls GetAttr(long,long) but handles diagram
//  axes as special case.
//  The item set of the graphical object representation is not merged in
//  anymore.
/*N*/ void ChartModel::GetAttr( const long nObjId, SfxItemSet& rAttr, const long nIndex1 /*=-1*/ )
/*N*/ {
/*N*/   CHART_TRACE( "ChartModel::GetAttr( long nObjId, SfxItemSet& rAttr)" );
/*N*/
/*N*/   // items at model (and axis object)
/*N*/   switch( nObjId )
/*N*/   {
/*N*/       case CHOBJID_DIAGRAM_X_AXIS:
/*N*/       case CHOBJID_DIAGRAM_Y_AXIS:
/*N*/       case CHOBJID_DIAGRAM_A_AXIS:
/*N*/       case CHOBJID_DIAGRAM_B_AXIS:
/*N*/       case CHOBJID_DIAGRAM_Z_AXIS:
/*N*/           {
/*N*/               // general item set
/*N*/               rAttr.Put( *pAxisAttr );
/*N*/
/*N*/               // specialized item set and members
/*N*/               ChartAxis* pAxis = GetAxis( nObjId );
/*N*/               if( pAxis )
/*N*/               {
/*N*/                   rAttr.Put( *(pAxis->GetItemSet()));
/*N*/                   pAxis->GetMembersAsAttr( rAttr );
/*N*/               }
/*N*/               // ChartModel members for axes
/*N*/               GetTextRotation( rAttr );
/*N*/               switch( nObjId )
/*N*/               {
/*N*/                   case CHOBJID_DIAGRAM_X_AXIS:
/*N*/                   case CHOBJID_DIAGRAM_A_AXIS:
/*N*/                       rAttr.Put( SfxBoolItem( SID_TEXTBREAK, GetFormatXAxisTextInMultipleLinesIfNecessary() ) );
/*N*/                       break;
/*N*/                   case CHOBJID_DIAGRAM_Y_AXIS:
/*N*/                   case CHOBJID_DIAGRAM_B_AXIS:
/*N*/                       rAttr.Put( SfxBoolItem( SID_TEXTBREAK, GetFormatYAxisTextInMultipleLinesIfNecessary() ) );
/*N*/                       break;
/*N*/                   case CHOBJID_DIAGRAM_Z_AXIS:
/*N*/                       rAttr.Put( SfxBoolItem( SID_TEXTBREAK, GetFormatZAxisTextInMultipleLinesIfNecessary() ) );
/*N*/                       break;
/*N*/               }
/*N*/           }
/*N*/           break;
/*N*/
/*N*/       default:
/*N*/           rAttr.Put( GetAttr( nObjId, nIndex1 ));
/*N*/           break;
/*N*/   }
/*N*/ }

/*N*/ BOOL ChartModel::ChangeAttr(const SfxItemSet& rAttr,const long nId,const long nIndex1/*=-1*/)
/*N*/ {
/*N*/   BOOL bNeedChanges=TRUE; //noch ungenutzt, zur evtl. Optimierung
/*N*/
/*N*/   //ToDo: optimieren! klappt wegen XChartView so nicht:
/*N*/   //BOOL bForceBuild=FALSE;
/*N*/   BOOL bForceBuild=TRUE;
/*N*/
/*N*/
/*N*/   SdrObject* pObj=NULL;
/*N*/   //Objektattribute am Objekt setzen, falls nicht indiziert
/*N*/   if(nIndex1==-1)
/*N*/   {
/*N*/       pObj=(GetObjWithId ((USHORT)nId,*GetPage(0),0,IM_DEEPWITHGROUPS));
/*N*/      if(pObj)
/*N*/           pObj->SetItemSetAndBroadcast(rAttr);
/*N*/   }
/*N*/
/*N*/
/*N*/   //und auch am Model
/*N*/   switch(nId)
/*N*/   {
/*N*/       case CHOBJID_DIAGRAM_X_AXIS:
/*N*/       case CHOBJID_DIAGRAM_Y_AXIS:
/*N*/       case CHOBJID_DIAGRAM_A_AXIS:
/*N*/       case CHOBJID_DIAGRAM_B_AXIS:
/*N*/       case CHOBJID_DIAGRAM_Z_AXIS:
/*N*/           {
/*N*/               if(pObj)
/*N*/               {
/*N*/                   ChangeAxisAttr(rAttr,(SdrObjGroup*)pObj);
/*N*/               }
/*N*/               else
/*N*/               {
/*N*/                   const SfxPoolItem* pPoolItem=NULL;
/*N*/                   if(rAttr.GetItemState(SID_ATTR_NUMBERFORMAT_VALUE,TRUE,&pPoolItem)==SFX_ITEM_SET)
/*N*/                   {
/*N*/                       SfxItemSet aSet(rAttr);
/*N*/                       UINT32 nTmp=((const SfxUInt32Item*)pPoolItem)->GetValue();
/*N*/                       aSet.Put(SfxUInt32Item(IsPercentChart()
/*N*/                           ? SCHATTR_AXIS_NUMFMTPERCENT : SCHATTR_AXIS_NUMFMT , nTmp));
/*N*/                       SetAttributes(nId,aSet); //im Model
/*N*/                   }
/*N*/                   else
/*N*/                   {
/*N*/                       SetAttributes(nId,rAttr); //im Model
/*N*/                   }
/*N*/
/*N*/               }
/*N*/
/*N*/               const SfxPoolItem *pPoolItem;
/*N*/               if( SFX_ITEM_SET == rAttr.GetItemState( SID_TEXTBREAK, TRUE, &pPoolItem ) )
/*N*/               {
/*N*/                   switch(nId)
/*N*/                   {
/*N*/                       case CHOBJID_DIAGRAM_X_AXIS:
/*N*/                       case CHOBJID_DIAGRAM_A_AXIS:
/*N*/                           SetFormatXAxisTextInMultipleLinesIfNecessary(((const SfxBoolItem*)pPoolItem)->GetValue());
/*N*/                           break;
/*N*/                       case CHOBJID_DIAGRAM_Y_AXIS:
/*N*/                       case CHOBJID_DIAGRAM_B_AXIS:
/*N*/                           SetFormatYAxisTextInMultipleLinesIfNecessary(((const SfxBoolItem*)pPoolItem)->GetValue());
/*N*/                           break;
/*N*/                       case CHOBJID_DIAGRAM_Z_AXIS:
/*N*/                           SetFormatZAxisTextInMultipleLinesIfNecessary(((const SfxBoolItem*)pPoolItem)->GetValue());
/*N*/                           break;
/*N*/                   }
/*N*/               }
/*N*/           }
/*N*/           break;
/*N*/         case CHOBJID_AREA:
/*?*/             break;
/*N*/       default:
/*N*/           GetAttr(nId,nIndex1).Put(rAttr,TRUE);
/*N*/           break;
/*N*/   }
/*N*/
/*N*/     // calculate result here because BuildChart will delete pObj (dangling pointer)
/*N*/     BOOL bResult = ( pObj != NULL || nIndex1 != -1 );
/*N*/
/*N*/   if(bForceBuild || nIndex1!=-1)
/*N*/       BuildChart(FALSE);
/*N*/   else if(bNeedChanges)
/*?*/       BuildChart(FALSE);
/*N*/
/*N*/   return bResult;
/*N*/ }

/*N*/ UINT32 ChartModel::GetNumFmt(long nObjId,BOOL bPercent)
/*N*/ {
/*N*/   return ((const SfxUInt32Item&)GetAttr(nObjId).Get(bPercent
/*N*/           ? SCHATTR_AXIS_NUMFMTPERCENT:SCHATTR_AXIS_NUMFMT)).GetValue();
/*N*/ }
/*N*/ void ChartModel::SetNumFmt(long nObjId,UINT32 nFmt,BOOL bPercent)
/*N*/ {
/*N*/   SfxItemSet aSet(*pItemPool,SCHATTR_AXIS_NUMFMT,SCHATTR_AXIS_NUMFMTPERCENT);
/*N*/   aSet.Put(SfxUInt32Item(bPercent? SCHATTR_AXIS_NUMFMTPERCENT:SCHATTR_AXIS_NUMFMT,nFmt));
/*N*/   SetAttributes(nObjId,aSet);
/*N*/ }


// Prerequisite: rOutAttributes is empty but can hold all interesting attributes
// Result:       All Items that are contained in all data row item sets are set
//               (using the Intersect method of the SfxItemSet)
/*N*/ void ChartModel::GetDataRowAttrAll( SfxItemSet& rOutAttributes )
/*N*/ {
/*N*/     long nListSize = aDataRowAttrList.size();
/*N*/
/*N*/     // no itemsets => result stays empty
/*N*/     if( nListSize == 0 )
/*N*/         return;
/*N*/
/*N*/     // set items of first data row and then intersect with all remaining
/*N*/     rOutAttributes.Put( *aDataRowAttrList[ 0 ] );
/*N*/   for( long nRow = 1; nRow < nListSize; nRow++ )
/*N*/         rOutAttributes.Intersect( *aDataRowAttrList[ nRow ] );
/*N*/ }

/*N*/ void ChartModel::SetItemWithNameCreation( SfxItemSet& rDestItemSet, const SfxPoolItem* pNewItem )
/*N*/ {
/*N*/     if( pNewItem == NULL )
/*N*/         return;
/*N*/
/*N*/     const SfxPoolItem* pItemToSet = pNewItem;
/*N*/
/*N*/     // this code comes from bf_svx/source/svdraw/svdoattr.cxx: SdrAttrObj::ItemChange()
/*N*/     switch( pItemToSet->Which())
/*N*/     {
/*N*/       case XATTR_FILLBITMAP:
/*N*/           pItemToSet = ((XFillBitmapItem*)pItemToSet)->checkForUniqueItem( this );
/*N*/           break;
/*N*/       case XATTR_LINEDASH:
/*N*/           pItemToSet = ((XLineDashItem*)pItemToSet)->checkForUniqueItem( this );
/*N*/           break;
/*N*/       case XATTR_LINESTART:
/*N*/           pItemToSet = ((XLineStartItem*)pItemToSet)->checkForUniqueItem( this );
/*N*/           break;
/*N*/       case XATTR_LINEEND:
/*N*/           pItemToSet = ((XLineEndItem*)pItemToSet)->checkForUniqueItem( this );
/*N*/           break;
/*N*/       case XATTR_FILLGRADIENT:
/*N*/           pItemToSet = ((XFillGradientItem*)pItemToSet)->checkForUniqueItem( this );
/*N*/           break;
/*N*/       case XATTR_FILLFLOATTRANSPARENCE:
/*N*/           pItemToSet = ((XFillFloatTransparenceItem*)pItemToSet)->checkForUniqueItem( this );
/*N*/           break;
/*N*/       case XATTR_FILLHATCH:
/*N*/           pItemToSet = ((XFillHatchItem*)pItemToSet)->checkForUniqueItem( this );
/*N*/           break;
/*N*/     }
/*N*/
/*N*/     if( pItemToSet )
/*N*/         rDestItemSet.Put( *pItemToSet );
/*N*/
/*N*/     // delete item if it was a generated one
/*N*/     if( pNewItem != pItemToSet )
/*N*/         delete pItemToSet;
/*N*/ }

/*N*/ void ChartModel::PutItemSetWithNameCreation( SfxItemSet& rDestItemSet, const SfxItemSet& rNewItemSet )
/*N*/ {
/*N*/     // copy all attributes
/*N*/     rDestItemSet.Put( rNewItemSet );
/*N*/
/*N*/     // check for items that need name creation
/*N*/     SfxWhichIter        aIter( rNewItemSet );
/*N*/     sal_uInt16          nWhich = aIter.FirstWhich();
/*N*/     const SfxPoolItem*  pPoolItem = NULL;
/*N*/
/*N*/     while( nWhich )
/*N*/     {
/*N*/         switch( nWhich )
/*N*/         {
/*N*/             case XATTR_FILLBITMAP:
/*N*/             case XATTR_LINEDASH:
/*N*/             case XATTR_LINESTART:
/*N*/             case XATTR_LINEEND:
/*N*/             case XATTR_FILLGRADIENT:
/*N*/             case XATTR_FILLFLOATTRANSPARENCE:
/*N*/             case XATTR_FILLHATCH:
/*N*/                 if( SFX_ITEM_SET == rNewItemSet.GetItemState( nWhich, TRUE, &pPoolItem ))
/*N*/                     SetItemWithNameCreation( rDestItemSet, pPoolItem );
/*N*/         }
/*N*/         nWhich = aIter.NextWhich();
/*N*/     }
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
