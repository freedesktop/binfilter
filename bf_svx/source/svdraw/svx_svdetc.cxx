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

#include "forbiddencharacterstable.hxx"

#include "svdetc.hxx"
#include "svditext.hxx"
#include "svdstr.hrc"
#include "svdviter.hxx"
#include "svdview.hxx"
#include "svdoutl.hxx"

#include <vcl/bmpacc.hxx>
#include <eeitem.hxx>
#include <bf_svtools/cacheoptions.hxx>

#include "xflclit.hxx"
#include "xflhtit.hxx"
#include "xbtmpit.hxx"
#include "xflgrit.hxx"
#include "svdoole2.hxx"

#include <bf_svtools/syslocale.hxx>

// #97870#
#include "xflbckit.hxx"

#include <bf_goodies/b3dcolor.hxx>

namespace binfilter {

/******************************************************************************
* Globale Daten der DrawingEngine
******************************************************************************/

/*N*/ SdrGlobalData::SdrGlobalData() :
/*N*/   pOutliner(NULL),
/*N*/   pDefaults(NULL),
/*N*/   pResMgr(NULL),
/*N*/   pStrCache(NULL),
/*N*/   nExchangeFormat(0)
/*N*/ {
/*N*/     pSysLocale = new SvtSysLocale;
/*N*/     pCharClass = pSysLocale->GetCharClassPtr();
/*N*/     pLocaleData = pSysLocale->GetLocaleDataPtr();
/*N*/ }

/*N*/ SdrGlobalData::~SdrGlobalData()
/*N*/ {
/*N*/   delete pOutliner;
/*N*/   delete pDefaults;
/*N*/   delete pResMgr;
/*N*/   delete [] pStrCache;
/*N*/     //! do NOT delete pCharClass and pLocaleData
/*N*/     delete pSysLocale;
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ OLEObjCache::OLEObjCache()
/*N*/ : Container( 0 )
/*N*/ {
/*N*/   SvtCacheOptions aCacheOptions;
/*N*/
/*N*/   nSize = aCacheOptions.GetDrawingEngineOLE_Objects();
/*N*/   pTimer = new AutoTimer();
/*N*/     Link aLink = LINK(this, OLEObjCache, UnloadCheckHdl);
/*N*/
/*N*/     pTimer->SetTimeoutHdl(aLink);
/*N*/     pTimer->SetTimeout(20000);
/*N*/     pTimer->Start();
/*N*/
/*N*/   aLink.Call(pTimer);
/*N*/ }

/*N*/ OLEObjCache::~OLEObjCache()
/*N*/ {
/*N*/   pTimer->Stop();
/*N*/   delete pTimer;
/*N*/   // Kein Unload notwendig, da zu diesem Zeitpunkt
/*N*/   // die Objekte nicht mehr vorhanden sind
/*N*/ }


/*N*/ void OLEObjCache::InsertObj(SdrOle2Obj* pObj)
/*N*/ {
/*N*/   if (nSize <= Count())
/*N*/   {
/*?*/       // Eintraege reduzieren
/*?*/       ULONG nIndex = Count() - 1;
/*?*/
/*?*/       for (ULONG i = nIndex; i + 1 >= nSize; i--)
/*?*/       {
/*?*/           // Pruefen, ob Objekte entfernt werden koennen
/*?*/           SdrOle2Obj* pCacheObj = (SdrOle2Obj*) GetObject(i);
/*?*/
/*?*/           if ( pCacheObj != pObj &&  UnloadObj(pCacheObj) )
/*?*/           {
/*?*/               Remove(i);
/*?*/           }
/*?*/       }
/*N*/   }
/*N*/
/*N*/   // Objekt ggf. entfernen und an erster Position einfuegen
/*N*/   Remove(pObj);
/*N*/   Insert(pObj, (ULONG) 0L);
/*N*/ }

/*N*/ void OLEObjCache::RemoveObj(SdrOle2Obj* pObj)
/*N*/ {
/*N*/   UnloadObj( (SdrOle2Obj*) Remove(pObj) );
/*N*/ }

/*N*/ BOOL OLEObjCache::UnloadObj(SdrOle2Obj* pObj)
/*N*/ {
/*N*/   BOOL bUnloaded = FALSE;
/*N*/
/*N*/   if (pObj)
/*N*/   {
/*N*/       BOOL bVisible = FALSE;
/*N*/           SdrViewIter aIter(pObj);
/*N*/       SdrView* pView = aIter.FirstView();
/*N*/
/*N*/       while (!bVisible && pView!=NULL)
/*N*/       {
/*?*/           bVisible = !pView->IsGrafDraft();
/*?*/
/*?*/           if (!bVisible)
/*?*/               pView = aIter.NextView();
/*N*/       }
/*N*/
/*N*/       if (!bVisible)
/*N*/           bUnloaded = pObj->Unload();
/*N*/   }
/*N*/
/*N*/   return bUnloaded;
/*N*/ }

/*N*/ IMPL_LINK(OLEObjCache, UnloadCheckHdl, AutoTimer*, EMPTYARG)
/*N*/ {
/*N*/   if (nSize <= Count())
/*N*/   {
/*?*/       // Eintraege reduzieren
/*?*/       ULONG nIndex = Count() - 1;
/*?*/
/*?*/       for (ULONG i = nIndex; i + 1 >= nSize; i--)
/*?*/       {
/*?*/           // Pruefen, ob Objekte entfernt werden koennen
/*?*/           SdrOle2Obj* pCacheObj = (SdrOle2Obj*) GetObject(i);
/*?*/
/*?*/           if ( UnloadObj(pCacheObj) )
/*?*/               Remove(i);
/*?*/       }
/*?*/   }
/*N*/
/*N*/     return 0;
/*N*/ }

/*?*/ void ContainerSorter::ImpSubSort(long nL, long nR) const
/*?*/ {
/*?*/   long i,j;
/*?*/   const void* pX;
/*?*/   void* pI;
/*?*/   void* pJ;
/*?*/   i=nL;
/*?*/   j=nR;
/*?*/   pX=rCont.GetObject((nL+nR)/2);
/*?*/   do {
/*?*/       pI=rCont.Seek(i);
/*?*/       while (pI!=pX && Compare(pI,pX)<0) { i++; pI=rCont.Next(); }
/*?*/       pJ=rCont.Seek(j);
/*?*/       while (pJ!=pX && Compare(pX,pJ)<0) { j--; pJ=rCont.Prev(); }
/*?*/       if (i<=j) {
/*?*/           rCont.Replace(pJ,i);
/*?*/           rCont.Replace(pI,j);
/*?*/           i++;
/*?*/           j--;
/*?*/       }
/*?*/   } while (i<=j);
/*?*/   if (nL<j) ImpSubSort(nL,j);
/*?*/   if (i<nR) ImpSubSort(i,nR);
/*?*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////////////////////////

/*?*/ class ImpClipMerk {
/*?*/   Region aClip;
/*?*/   bool   bClip;
/*?*/ public:
/*?*/   ImpClipMerk(const OutputDevice& rOut): aClip(rOut.GetClipRegion()),bClip(rOut.IsClipRegion()) {}
/*?*/   void Restore(OutputDevice& rOut)
/*?*/   {
/*?*/       // Kein Clipping in die Metafileaufzeichnung
/*?*/       GDIMetaFile* pMtf=rOut.GetConnectMetaFile();
/*?*/       if (pMtf!=NULL && (!pMtf->IsRecord() || pMtf->IsPause())) pMtf=NULL;
/*?*/       if (pMtf!=NULL) pMtf->Pause(TRUE);
/*?*/       if (bClip) rOut.SetClipRegion(aClip);
/*?*/       else rOut.SetClipRegion();
/*?*/       if (pMtf!=NULL) pMtf->Pause(FALSE);
/*?*/   }
/*?*/ };
/*?*/
/*?*/ class ImpColorMerk {
/*?*/   Color aLineColor;
/*?*/   Color aFillColor;
/*?*/   Color aBckgrdColor;
/*?*/   Font  aFont;
/*?*/ public:
/*?*/   ImpColorMerk(const OutputDevice& rOut):
/*?*/       aLineColor( rOut.GetLineColor() ),
/*?*/       aFillColor( rOut.GetFillColor() ),
/*?*/       aBckgrdColor( rOut.GetBackground().GetColor() ),
/*?*/       aFont (rOut.GetFont()) {}
/*?*/
/*?*/   ImpColorMerk(const OutputDevice& rOut, USHORT nMode)
/*?*/   {
/*?*/       if ( (nMode & SDRHDC_SAVEPEN) == SDRHDC_SAVEPEN )
/*?*/           aLineColor = rOut.GetLineColor();
/*?*/
/*?*/       if ( (nMode & SDRHDC_SAVEBRUSH) == SDRHDC_SAVEBRUSH)
/*?*/       {
/*?*/           aFillColor = rOut.GetFillColor();
/*?*/           aBckgrdColor = rOut.GetBackground().GetColor();
/*?*/       }
/*?*/
/*?*/       if ( (nMode & SDRHDC_SAVEFONT) == SDRHDC_SAVEFONT)
/*?*/           aFont=rOut.GetFont();
/*?*/   }
/*?*/
/*?*/   void Restore(OutputDevice& rOut, USHORT nMode=SDRHDC_SAVEPENANDBRUSHANDFONT)
/*?*/   {
/*?*/       if ( (nMode & SDRHDC_SAVEPEN) == SDRHDC_SAVEPEN)
/*?*/           rOut.SetLineColor( aLineColor );
/*?*/
/*?*/       if ( (nMode & SDRHDC_SAVEBRUSH) == SDRHDC_SAVEBRUSH)
/*?*/       {
/*?*/           rOut.SetFillColor( aFillColor );
/*?*/           rOut.SetBackground( Wallpaper( aBckgrdColor ) );
/*?*/       }
/*?*/       if ((nMode & SDRHDC_SAVEFONT) ==SDRHDC_SAVEFONT)
/*?*/       {
/*?*/           if (!rOut.GetFont().IsSameInstance(aFont))
/*?*/           {
/*?*/               rOut.SetFont(aFont);
/*?*/           }
/*?*/       }
/*?*/   }
/*?*/
/*?*/   const Color& GetLineColor() const { return aLineColor; }
/*?*/ };

/*N*/ ImpSdrHdcMerk::ImpSdrHdcMerk(const OutputDevice& rOut, USHORT nNewMode, bool bAutoMerk):
/*N*/   pFarbMerk(NULL),
/*N*/   pClipMerk(NULL),
/*N*/   pLineColorMerk(NULL),
/*N*/   nMode(nNewMode)
/*N*/ {
/*N*/   if (bAutoMerk) Save(rOut);
/*N*/ }

/*N*/ ImpSdrHdcMerk::~ImpSdrHdcMerk()
/*N*/ {
/*N*/   if (pFarbMerk!=NULL) delete pFarbMerk;
/*N*/   if (pClipMerk!=NULL) delete pClipMerk;
/*N*/   if (pLineColorMerk !=NULL) delete pLineColorMerk;
/*N*/ }

/*N*/ void ImpSdrHdcMerk::Restore(OutputDevice& rOut, USHORT nMask) const
/*N*/ {
/*N*/   nMask&=nMode; // nur restaurieren, was auch gesichert wurde
/*N*/
/*N*/   if ((nMask & SDRHDC_SAVECLIPPING) ==SDRHDC_SAVECLIPPING && pClipMerk!=NULL)
/*N*/       pClipMerk->Restore(rOut);
/*N*/
/*N*/   USHORT nCol=nMask & SDRHDC_SAVEPENANDBRUSHANDFONT;
/*N*/
/*N*/   if (nCol==SDRHDC_SAVEPEN)
/*N*/   {
/*N*/       if (pLineColorMerk!=NULL)
/*N*/           rOut.SetLineColor(*pLineColorMerk);
/*N*/       else if (pFarbMerk!=NULL)
/*N*/           rOut.SetLineColor( pFarbMerk->GetLineColor() );
/*N*/   } else if (nCol!=0 && pFarbMerk!=NULL)
/*N*/       pFarbMerk->Restore(rOut,nCol);
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrLinkList::Clear()
/*N*/ {
/*N*/   unsigned nAnz=GetLinkCount();
/*N*/   for (unsigned i=0; i<nAnz; i++) {
/*N*/       delete (Link*)aList.GetObject(i);
/*N*/   }
/*N*/   aList.Clear();
/*N*/ }

/*N*/ unsigned SdrLinkList::FindEntry(const Link& rLink) const
/*N*/ {
/*N*/   unsigned nAnz=GetLinkCount();
/*N*/   for (unsigned i=0; i<nAnz; i++) {
/*N*/       if (GetLink(i)==rLink) return i;
/*N*/   }
/*N*/   return 0xFFFF;
/*N*/ }

/*N*/ void SdrLinkList::InsertLink(const Link& rLink, unsigned nPos)
/*N*/ {
/*N*/   unsigned nFnd=FindEntry(rLink);
/*N*/   if (nFnd==0xFFFF) {
/*N*/       if (rLink.IsSet()) {
/*N*/           aList.Insert(new Link(rLink),nPos);
/*N*/       } else {
/*N*/           OSL_FAIL("SdrLinkList::InsertLink(): Versuch, einen nicht gesetzten Link einzufuegen");
/*N*/       }
/*N*/   } else {
/*N*/       OSL_FAIL("SdrLinkList::InsertLink(): Link schon vorhanden");
/*N*/   }
/*N*/ }
/*N*/
/*N*/ void SdrLinkList::RemoveLink(const Link& rLink)
/*N*/ {
/*N*/   unsigned nFnd=FindEntry(rLink);
/*N*/   if (nFnd!=0xFFFF) {
/*N*/       Link* pLink=(Link*)aList.Remove(nFnd);
/*N*/       delete pLink;
/*N*/   } else {
/*N*/       OSL_FAIL("SdrLinkList::RemoveLink(): Link nicht gefunden");
/*N*/   }
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// #98988# Re-implement GetDraftFillColor(...)

/*N*/ bool GetDraftFillColor(const SfxItemSet& rSet, Color& rCol)
/*N*/ {
/*N*/   XFillStyle eFill=((XFillStyleItem&)rSet.Get(XATTR_FILLSTYLE)).GetValue();
/*N*/   bool bRetval(FALSE);
/*N*/
/*N*/   switch(eFill)
/*N*/   {
/*N*/       case XFILL_SOLID:
/*N*/       {
/*N*/           rCol = ((XFillColorItem&)rSet.Get(XATTR_FILLCOLOR)).GetValue();
/*N*/           bRetval = TRUE;
/*N*/
/*N*/           break;
/*N*/       }
/*N*/       case XFILL_HATCH:
/*N*/       {
/*?*/           Color aCol1(((XFillHatchItem&)rSet.Get(XATTR_FILLHATCH)).GetValue().GetColor());
/*?*/           Color aCol2(COL_WHITE);
/*?*/
/*?*/           // #97870# when hatch background is activated, use object fill color as hatch color
/*?*/           sal_Bool bFillHatchBackground = ((const XFillBackgroundItem&)(rSet.Get(XATTR_FILLBACKGROUND))).GetValue();
/*?*/           if(bFillHatchBackground)
/*?*/           {
/*?*/               aCol2 = ((const XFillColorItem&)(rSet.Get(XATTR_FILLCOLOR))).GetValue();
/*?*/           }
/*?*/
/*?*/           ((B3dColor&)rCol).CalcMiddle(aCol1, aCol2);
/*?*/           bRetval = TRUE;
/*?*/
/*?*/           break;
/*N*/       }
/*N*/       case XFILL_GRADIENT: {
/*N*/           const XGradient& rGrad=((XFillGradientItem&)rSet.Get(XATTR_FILLGRADIENT)).GetValue();
/*N*/           Color aCol1(rGrad.GetStartColor());
/*N*/           Color aCol2(rGrad.GetEndColor());
/*N*/           ((B3dColor&)rCol).CalcMiddle(aCol1, aCol2);
/*N*/           bRetval = TRUE;
/*N*/
/*N*/           break;
/*N*/       }
/*N*/       case XFILL_BITMAP:
/*N*/       {
/*?*/           const Bitmap& rBitmap = ((XFillBitmapItem&)rSet.Get(XATTR_FILLBITMAP)).GetValue().GetBitmap();
/*?*/           const Size aSize(rBitmap.GetSizePixel());
/*?*/           const sal_uInt32 nWidth = aSize.Width();
/*?*/           const sal_uInt32 nHeight = aSize.Height();
/*?*/           Bitmap aBitmap(rBitmap);
/*?*/           BitmapReadAccess* pAccess = aBitmap.AcquireReadAccess();
/*?*/
/*?*/           if(pAccess && nWidth > 0 && nHeight > 0)
/*?*/           {
/*?*/               sal_uInt32 nRt(0L);
/*?*/               sal_uInt32 nGn(0L);
/*?*/               sal_uInt32 nBl(0L);
/*?*/               const sal_uInt32 nMaxSteps(8L);
/*?*/               const sal_uInt32 nXStep((nWidth > nMaxSteps) ? nWidth / nMaxSteps : 1L);
/*?*/               const sal_uInt32 nYStep((nHeight > nMaxSteps) ? nHeight / nMaxSteps : 1L);
/*?*/               sal_uInt32 nAnz(0L);
/*?*/
/*?*/               for(sal_uInt32 nY(0L); nY < nHeight; nY += nYStep)
/*?*/               {
/*?*/                   for(sal_uInt32 nX(0L); nX < nWidth; nX += nXStep)
/*?*/                   {
/*?*/                       const BitmapColor& rLclCol = (pAccess->HasPalette())
/*?*/                           ? pAccess->GetPaletteColor((BYTE)pAccess->GetPixel(nY, nX))
/*?*/                           : pAccess->GetPixel(nY, nX);
/*?*/
/*?*/                       nRt += rLclCol.GetRed();
/*?*/                       nGn += rLclCol.GetGreen();
/*?*/                       nBl += rLclCol.GetBlue();
/*?*/                       nAnz++;
/*?*/                   }
/*?*/               }
/*?*/
/*?*/               nRt /= nAnz;
/*?*/               nGn /= nAnz;
/*?*/               nBl /= nAnz;
/*?*/
/*?*/               rCol = Color(UINT8(nRt), UINT8(nGn), UINT8(nBl));
/*?*/
/*?*/               bRetval = TRUE;
/*?*/           }
/*?*/
/*?*/           if(pAccess)
/*?*/           {
/*?*/               aBitmap.ReleaseAccess(pAccess);
/*?*/           }
/*?*/
/*?*/           break;
/*?*/       }
            case XFILL_NONE:
                break;
/*N*/   }
/*N*/
/*N*/   return bRetval;
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ SdrEngineDefaults::SdrEngineDefaults():
/*N*/   aFontName( OutputDevice::GetDefaultFont( DEFAULTFONT_SERIF, LANGUAGE_SYSTEM, DEFAULTFONT_FLAGS_ONLYONE ).GetName() ),
/*N*/   eFontFamily(FAMILY_ROMAN),
/*N*/   aFontColor(COL_AUTO),
/*N*/   nFontHeight(847),             // 847/100mm = ca. 24 Point
/*N*/   eMapUnit(MAP_100TH_MM),
/*N*/   aMapFraction(1,1)
/*N*/ {
/*N*/ }
/*N*/
/*N*/ SdrEngineDefaults& SdrEngineDefaults::GetDefaults()
/*N*/ {
/*N*/   SdrGlobalData& rGlobalData=GetSdrGlobalData();
/*N*/   if (rGlobalData.pDefaults==NULL) {
/*N*/       rGlobalData.pDefaults=new SdrEngineDefaults;
/*N*/   }
/*N*/   return *rGlobalData.pDefaults;
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ SdrOutliner* SdrMakeOutliner( USHORT nOutlinerMode, SdrModel* pModel )
/*N*/ {
/*N*/   /*SdrEngineDefaults& rDefaults =*/ SdrEngineDefaults::GetDefaults();

/*N*/   SfxItemPool* pPool = &pModel->GetItemPool();
/*N*/   SdrOutliner* pOutl = new SdrOutliner( pPool, nOutlinerMode );
/*N*/   pOutl->SetEditTextObjectPool( pPool );
/*N*/   pOutl->SetStyleSheetPool( (SfxStyleSheetPool*) pModel->GetStyleSheetPool() );
/*N*/   pOutl->SetDefTab( pModel->GetDefaultTabulator() );
/*N*/   pOutl->SetForbiddenCharsTable( pModel->GetForbiddenCharsTable() );
/*N*/   pOutl->SetAsianCompressionMode( pModel->GetCharCompressType() );
/*N*/   pOutl->SetKernAsianPunctuation( pModel->IsKernAsianPunctuation() );
/*N*/
/*N*/   return pOutl;
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////


/*N*/ SdrLinkList& ImpGetUserMakeObjHdl()
/*N*/ {
/*N*/   SdrGlobalData& rGlobalData=GetSdrGlobalData();
/*N*/   return rGlobalData.aUserMakeObjHdl;
/*N*/ }

/*N*/ SdrLinkList& ImpGetUserMakeObjUserDataHdl()
/*N*/ {
/*N*/   SdrGlobalData& rGlobalData=GetSdrGlobalData();
/*N*/   return rGlobalData.aUserMakeObjUserDataHdl;
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ ResMgr* ImpGetResMgr()
/*N*/ {
/*N*/   SdrGlobalData& rGlobalData = GetSdrGlobalData();
/*N*/
/*N*/   if(!rGlobalData.pResMgr)
/*N*/   {
/*N*/ #ifndef SVX_LIGHT
/*N*/       ByteString aName("bf_svx");
/*N*/ #else
/*N*/       ByteString aName("bf_svl");
/*N*/ #endif
/*N*/       rGlobalData.pResMgr =
/*N*/             ResMgr::CreateResMgr( aName.GetBuffer(), Application::GetSettings().GetUILocale() );
/*N*/   }
/*N*/
/*N*/   return rGlobalData.pResMgr;
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
