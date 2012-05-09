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

#include "bf_so3/staticbaseurl.hxx"

#include <tools/tenccvt.hxx>

#include <bf_svtools/urihelper.hxx>

#include <bf_sfx2/docfile.hxx>
#include <bf_svx/svdotext.hxx>
#include <bf_svx/xmlcnitm.hxx>
#include <bf_svx/svditer.hxx>


#include "sdpage.hxx"
#include "glob.hxx"
#include "glob.hrc"
#include "drawdoc.hxx"
#include "stlpool.hxx"
#include "sdiocmpt.hxx"
#include "strmname.h"
#include "anminfo.hxx"

namespace binfilter {

using namespace ::com::sun::star;

void SdPage::EndListenOutlineText()
{
    SdrObject* pOutlineTextObj = GetPresObj(PRESOBJ_OUTLINE);

    if (pOutlineTextObj)
    {
        SdStyleSheetPool* pSPool = (SdStyleSheetPool*)pModel->GetStyleSheetPool();
        DBG_ASSERT(pSPool, "StyleSheetPool nicht gefunden");
        String aTrueLayoutName(aLayoutName);
        aTrueLayoutName.Erase( aTrueLayoutName.SearchAscii( SD_LT_SEPARATOR ));
        List* pOutlineStyles = pSPool->CreateOutlineSheetList(aTrueLayoutName);
        for (SfxStyleSheet* pSheet = (SfxStyleSheet*)pOutlineStyles->First();
             pSheet;
             pSheet = (SfxStyleSheet*)pOutlineStyles->Next())
            {
                pOutlineTextObj->EndListening(*pSheet);
            }

        delete pOutlineStyles;
    }
}

void SdPage::ReadData(const SdrIOHeader& rHead, SvStream& rIn)
{
    FmFormPage::ReadData( rHead, rIn );

    rIn.SetStreamCharSet(GetSOLoadTextEncoding(osl_getThreadTextEncoding()));

    if ( pModel->IsStreamingSdrModel() )
    {
        return;
    }

    SdIOCompat aIO(rIn, STREAM_READ);

    BOOL bDummy;
    BOOL bManual;

    rIn>>bDummy;                      // ehem. bTemplateMode
    rIn>>bDummy;                      // ehem. bBackgroundMode
    rIn>>bDummy;                      // ehem. bOutlineMode

    UINT16 nAutoLayout;
    rIn>>nAutoLayout;
    eAutoLayout = (AutoLayout) nAutoLayout;

    // Selektionskennung ist nicht persistent, wird nicht gelesen

    sal_uInt32 nULTemp;
    rIn >> nULTemp; eFadeSpeed  = (FadeSpeed)nULTemp;
    rIn >> nULTemp; eFadeEffect = (presentation::FadeEffect)nULTemp;
    rIn >> bManual;
    rIn >> nTime;
    rIn >> bSoundOn;
    rIn >> bExcluded;
    aLayoutName = rIn.ReadUniOrByteString( rIn.GetStreamCharSet() );

    if (IsObjOrdNumsDirty())
        RecalcObjOrdNums();

    UINT32 nCount;
    UINT32 nOrdNum;
    rIn >> nCount;
    for (UINT32 nObj = 0; nObj < nCount; nObj++)
    {
        rIn >> nOrdNum;
        SdrObject* pObj = GetObj(nOrdNum);
        aPresObjList.Insert(pObj, LIST_APPEND);
    }

    // ab hier werden Daten der Versionen >=1 eingelesen
    if (aIO.GetVersion() >= 1)
    {
        UINT16 nPageKind;
        rIn >> nPageKind;
        ePageKind = (PageKind) nPageKind;
    }

    // ab hier werden Daten der Versionen >=2 eingelesen
    if (aIO.GetVersion() >=2)
    {
        UINT32 nUserCallCount;
        UINT32 nUserCallOrdNum;
        rIn >> nUserCallCount;
        for (UINT32 nObj = 0; nObj < nUserCallCount; nObj++)
        {
            rIn >> nUserCallOrdNum;
            SdrObject* pObj = GetObj(nUserCallOrdNum);

            if (pObj)
                pObj->SetUserCall(this);
        }
    }

    // ab hier werden Daten der Versionen >=3 eingelesen
    if (aIO.GetVersion() >=3)
    {
        INT16 nCharSet;
        rIn >> nCharSet;    // nur Einlesen, Konvertierung ab 303 durch Stream

        // #90477# eCharSet = (CharSet) nCharSet;
        eCharSet = (CharSet)GetSOLoadTextEncoding((rtl_TextEncoding)nCharSet);

        String aSoundFileRel = rIn.ReadUniOrByteString( rIn.GetStreamCharSet() );
        INetURLObject aURLObj(::binfilter::StaticBaseUrl::SmartRelToAbs(aSoundFileRel, FALSE,
                                                         INetURLObject::WAS_ENCODED,
                                                         INetURLObject::DECODE_UNAMBIGUOUS));
        aSoundFile = aURLObj.GetMainURL( INetURLObject::NO_DECODE );
    }

    // ab hier werden Daten der Versionen >=4 eingelesen
    if (aIO.GetVersion() >=4)
    {
        String aFileNameRel = rIn.ReadUniOrByteString( rIn.GetStreamCharSet() );
        INetURLObject aURLObj(::binfilter::StaticBaseUrl::SmartRelToAbs(aFileNameRel, FALSE,
                                                         INetURLObject::WAS_ENCODED,
                                                         INetURLObject::DECODE_UNAMBIGUOUS));
        aFileName = aURLObj.GetMainURL( INetURLObject::NO_DECODE );

        aBookmarkName = rIn.ReadUniOrByteString( rIn.GetStreamCharSet() );
    }

    // ab hier werden Daten der Versionen >=5 eingelesen
    if (aIO.GetVersion() >=5)
    {
        UINT16 nPaperBinTemp;
        rIn >> nPaperBinTemp;
        nPaperBin = nPaperBinTemp;
    }

    // ab hier werden Daten der Versionen >=6 eingelesen
    if (aIO.GetVersion() >=6)
    {
        UINT16 nOrientationTemp;
        rIn >> nOrientationTemp;
        eOrientation = (Orientation) nOrientationTemp;
    }
    else
    {
        // In aelteren Versionen wird die Orientation aus der Seitengroesse bestimmt
        Size aPageSize(GetSize());

        if (aPageSize.Width() > aPageSize.Height())
        {
            eOrientation = ORIENTATION_LANDSCAPE;
        }
        else
        {
            eOrientation = ORIENTATION_PORTRAIT;
        }
    }

    // ab hier werden Daten der Versionen >=7 eingelesen
    if( aIO.GetVersion() >= 7 )
    {
        UINT16 nPresChangeTemp;
        rIn >> nPresChangeTemp;
        ePresChange = (PresChange) nPresChangeTemp;
    }
    else
        ePresChange = ( bManual ? PRESCHANGE_MANUAL : PRESCHANGE_AUTO );
}

void SdPage::SetModel(SdrModel* pNewModel)
{
    FmFormPage::SetModel(pNewModel);
}

SdPage::SdPage(const SdPage& rSrcPage)
: FmFormPage(rSrcPage), SdrObjUserCall(rSrcPage)
{
    ePageKind           = rSrcPage.ePageKind;
    eAutoLayout         = rSrcPage.eAutoLayout;
    bOwnArrangement     = FALSE;

    UINT32 nCount = (UINT32) rSrcPage.aPresObjList.Count();

    for (UINT32 nObj = 0; nObj < nCount; nObj++)
    {
        // Liste der Praesenationsobjekte fuellen
        SdrObject* pSrcObj = (SdrObject*) rSrcPage.aPresObjList.GetObject(nObj);

        if (pSrcObj)
        {
            aPresObjList.Insert(GetObj(pSrcObj->GetOrdNum()), LIST_APPEND);
        }
    }

    bSelected           = FALSE;
    eFadeSpeed          = rSrcPage.eFadeSpeed;
    eFadeEffect         = rSrcPage.eFadeEffect;
    ePresChange         = rSrcPage.ePresChange;
    nTime               = rSrcPage.nTime;
    bSoundOn            = rSrcPage.bSoundOn;
    bExcluded           = rSrcPage.bExcluded;

    aLayoutName         = rSrcPage.aLayoutName;
    aSoundFile          = rSrcPage.aSoundFile;
    aCreatedPageName    = String();
    aFileName           = rSrcPage.aFileName;
    aBookmarkName       = rSrcPage.aBookmarkName;
    bScaleObjects       = rSrcPage.bScaleObjects;
    bBackgroundFullSize = rSrcPage.bBackgroundFullSize;
    eCharSet            = rSrcPage.eCharSet;
    nPaperBin           = rSrcPage.nPaperBin;
    eOrientation        = rSrcPage.eOrientation;
}

SdrPage* SdPage::Clone() const
{
    SdPage* pLclPage = new SdPage(*this);

    if( (PK_STANDARD == ePageKind) && !IsMasterPage() )
    {
        // preserve presentation order on slide duplications
        SdrObjListIter aSrcIter( *this, IM_DEEPWITHGROUPS );
        SdrObjListIter aDstIter( *pLclPage, IM_DEEPWITHGROUPS );

        while( aSrcIter.IsMore() && aDstIter.IsMore() )
        {
            SdrObject* pSrc = aSrcIter.Next();
            SdrObject* pDst = aDstIter.Next();

            SdAnimationInfo* pSrcInfo = ((SdDrawDocument*)pModel)->GetAnimationInfo(pSrc);
            if( pSrcInfo && (pSrcInfo->nPresOrder != LIST_APPEND) )
            {
                SdAnimationInfo* pDstInfo = ((SdDrawDocument*)pModel)->GetAnimationInfo(pDst);
                DBG_ASSERT( pDstInfo, "shape should have an animation info after clone!" );

                if( pDstInfo )
                    pDstInfo->nPresOrder = pSrcInfo->nPresOrder;
            }
        }

        DBG_ASSERT( !aSrcIter.IsMore() && !aDstIter.IsMore(), "unequal shape numbers after a page clone?" );
    }

    return(pLclPage);
}

SfxStyleSheet* SdPage::GetTextStyleSheetForObject( SdrObject* pObj ) const
{
    const PresObjKind eKind = ((SdPage*)this)->GetPresObjKind(pObj);
    if( eKind != PRESOBJ_NONE )
    {
        return ((SdPage*)this)->GetStyleSheetForPresObj(eKind);
    }

    return FmFormPage::GetTextStyleSheetForObject( pObj );
}

SfxItemSet* SdPage::getOrCreateItems()
{
     if( mpItems == NULL )
        mpItems = new SfxItemSet( pModel->GetItemPool(), SDRATTR_XMLATTRIBUTES, SDRATTR_XMLATTRIBUTES );

     return mpItems;
}

sal_Bool SdPage::setAlienAttributes( const ::com::sun::star::uno::Any& rAttributes )
{
    SfxItemSet* pSet = getOrCreateItems();

    SvXMLAttrContainerItem aAlienAttributes( SDRATTR_XMLATTRIBUTES );
    if( aAlienAttributes.PutValue( rAttributes, 0 ) )
    {
        pSet->Put( aAlienAttributes );
        return sal_True;
    }

    return sal_False;
}

void SdPage::getAlienAttributes( ::com::sun::star::uno::Any& rAttributes )
{
    const SfxPoolItem* pItem;

    if( (mpItems == NULL) || ( SFX_ITEM_SET != mpItems->GetItemState( SDRATTR_XMLATTRIBUTES, sal_False, &pItem ) ) )
    {
        SvXMLAttrContainerItem aAlienAttributes;
        aAlienAttributes.QueryValue( rAttributes, 0 );
    }
    else
    {
        ((SvXMLAttrContainerItem*)pItem)->QueryValue( rAttributes, 0 );
    }
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
