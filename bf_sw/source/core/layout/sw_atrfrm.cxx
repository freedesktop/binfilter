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
#pragma hdrstop
#endif

#include <hintids.hxx>

#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/text/TextGridMode.hpp>
#include <bf_svtools/unoimap.hxx>
#include <bf_svtools/imap.hxx>
#include <bf_svx/ulspitem.hxx>
#include <bf_svx/lrspitem.hxx>

#include <unosett.hxx>
#include <fmtclds.hxx>

#include <horiornt.hxx>

#include <fmtornt.hxx>
#include <fmthdft.hxx>
#include <fmtpdsc.hxx>
#include <fmtfsize.hxx>
#include <fmtfordr.hxx>
#include <fmtsrnd.hxx>
#include <fmtanchr.hxx>
#include <fmtlsplt.hxx>
#include <fmtftntx.hxx>
#include <fmteiro.hxx>
#include <fmturl.hxx>
#include <fmtcnct.hxx>
#include <section.hxx>
#include <fmtline.hxx>
#include <tgrditem.hxx>
#include <hfspacingitem.hxx>
#include <doc.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <cntfrm.hxx>
#include <crsrsh.hxx>
#include <dflyobj.hxx>
#include <dcontact.hxx>
#include <frmtool.hxx>
#include <hints.hxx>
#include <flyfrms.hxx>
#include <pagedesc.hxx>
#include <docary.hxx>
#include <node2lay.hxx>
#include <fmtclbl.hxx>
#include <swunohelper.hxx>
#include <unocoll.hxx>
#include <unoframe.hxx>
#include <SwStyleNameMapper.hxx>
///     include definition of class SvxBrushItem and GraphicObject
///     in order to determine, if background is transparent.
#include <bf_svx/brshitem.hxx>
#include <bf_goodies/graphicobject.hxx>

#include <cmdid.h>
#include <unomid.h>

namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;

using rtl::OUString;

SV_IMPL_PTRARR(SwColumns,SwColumn*)

TYPEINIT1(SwFrmFmt,    SwFmt );  //rtti fuer SwFrmFmt
TYPEINIT1(SwFlyFrmFmt,  SwFrmFmt);
TYPEINIT1(SwDrawFrmFmt, SwFrmFmt);
TYPEINIT1(SwFmtVertOrient, SfxPoolItem);
TYPEINIT1(SwFmtHoriOrient, SfxPoolItem);
TYPEINIT2(SwFmtHeader,  SfxPoolItem, SwClient );
TYPEINIT2(SwFmtFooter,  SfxPoolItem, SwClient );
TYPEINIT2(SwFmtPageDesc,  SfxPoolItem, SwClient );
TYPEINIT1_AUTOFACTORY(SwFmtLineNumber, SfxPoolItem);

/* --------------------------------------------------
 *  Umwandlung fuer QueryValue
 * --------------------------------------------------*/
sal_Int16 lcl_RelToINT(SwRelationOrient eRelation)
{
    sal_Int16 nRet = text::RelOrientation::FRAME;
    switch(eRelation)
    {
    case  PRTAREA:          nRet = text::RelOrientation::PRINT_AREA; break;
    case  REL_CHAR:         nRet = text::RelOrientation::CHAR; break;
    case  REL_PG_LEFT:      nRet = text::RelOrientation::PAGE_LEFT; break;
    case  REL_PG_RIGHT:     nRet = text::RelOrientation::PAGE_RIGHT; break;
    case  REL_FRM_LEFT:     nRet = text::RelOrientation::FRAME_LEFT; break;
    case  REL_FRM_RIGHT:    nRet = text::RelOrientation::FRAME_RIGHT; break;
    case  REL_PG_FRAME:     nRet = text::RelOrientation::PAGE_FRAME; break;
    case  REL_PG_PRTAREA:   nRet = text::RelOrientation::PAGE_PRINT_AREA; break;
    case  FRAME:            break;
    case  LAST_ENUM_DUMMY:  break;
    }
    return nRet;
}
SwRelationOrient   lcl_IntToRelation(const uno::Any& rVal)
{
    SwRelationOrient eRet = FRAME;
    sal_Int16 nVal(0);
    rVal >>= nVal;
    switch(nVal)
    {
    case  text::RelOrientation::PRINT_AREA:     eRet =   PRTAREA           ; break;
    case  text::RelOrientation::CHAR:       eRet =   REL_CHAR          ; break;
    case  text::RelOrientation::PAGE_LEFT:    eRet =   REL_PG_LEFT       ; break;
    case  text::RelOrientation::PAGE_RIGHT:    eRet =   REL_PG_RIGHT      ; break;
    case  text::RelOrientation::FRAME_LEFT:    eRet =   REL_FRM_LEFT      ; break;
    case  text::RelOrientation::FRAME_RIGHT:    eRet =   REL_FRM_RIGHT     ; break;
    case  text::RelOrientation::PAGE_FRAME:    eRet =   REL_PG_FRAME      ; break;
    case  text::RelOrientation::PAGE_PRINT_AREA:    eRet =   REL_PG_PRTAREA    ; break;
    }
    return eRet;
}

void DelHFFormat( SwClient *pToRemove, SwFrmFmt *pFmt )
{
    //Wenn der Client der letzte ist der das Format benutzt, so muss dieses
    //vernichtet werden. Zuvor muss jedoch ggf. die Inhaltssection vernichtet
    //werden.
    SwDoc* pDoc = pFmt->GetDoc();
    pFmt->Remove( pToRemove );
    if( pDoc->IsInDtor() )
    {
        delete pFmt;
        return;
    }

    //Nur noch Frms angemeldet?
    sal_Bool bDel = sal_True;
    {
        // Klammer, weil im DTOR SwClientIter das Flag bTreeChg zurueck
        // gesetzt wird. Unguenstig, wenn das Format vorher zerstoert wird.
        SwClientIter aIter( *pFmt );
        SwClient *pLast = aIter.GoStart();
        if( pLast )
            do {
                bDel = pLast->IsA( TYPE(SwFrm) )|| pLast->IsA(TYPE(SwXHeadFootText));
            } while( bDel && 0 != ( pLast = aIter++ ));
    }

    if ( bDel )
    {
        //Wenn in einem der Nodes noch ein Crsr angemeldet ist, muss das
        //ParkCrsr einer (beliebigen) Shell gerufen werden.
        SwFmtCntnt& rCnt = (SwFmtCntnt&)pFmt->GetCntnt();
        if ( rCnt.GetCntntIdx() )
        {
            SwNode *pNode = 0;
            {
                SwNodeIndex aIdx( *rCnt.GetCntntIdx(), 1 );
                //Wenn in einem der Nodes noch ein Crsr angemeldet ist, muss das
                //ParkCrsr einer (beliebigen) Shell gerufen werden.
                pNode = pDoc->GetNodes()[ aIdx ];
                sal_uInt32 nEnd = pNode->EndOfSectionIndex();
                while ( aIdx < nEnd )
                {
                    aIdx++;
                    pNode = pDoc->GetNodes()[ aIdx ];
                }
            }
            rCnt.SetNewCntntIdx( (const SwNodeIndex*)0 );

            // beim Loeschen von Header/Footer-Formaten IMMER das Undo
            // abschalten! (Bug 31069)

            OSL_ENSURE( pNode, "Ein grosses Problem." );
            pDoc->DeleteSection( pNode );

        }
        delete pFmt;
    }
}

//  class SwFmtFrmSize
//  Implementierung teilweise inline im hxx

SwFmtFrmSize::SwFmtFrmSize( SwFrmSize eSize, SwTwips nWidth, SwTwips nHeight )
    : SfxPoolItem( RES_FRM_SIZE )
    , aSize( nWidth, nHeight )
    , eFrmSize( eSize )
{
    nWidthPercent = nHeightPercent = 0;
}

SwFmtFrmSize& SwFmtFrmSize::operator=( const SwFmtFrmSize& rCpy )
{
    aSize = rCpy.GetSize();
    eFrmSize = rCpy.GetSizeType();
    nHeightPercent = rCpy.GetHeightPercent();
    nWidthPercent  = rCpy.GetWidthPercent();
    return *this;
}

int  SwFmtFrmSize::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return( eFrmSize        == ((SwFmtFrmSize&)rAttr).eFrmSize &&
            aSize           == ((SwFmtFrmSize&)rAttr).GetSize()&&
            nWidthPercent   == ((SwFmtFrmSize&)rAttr).GetWidthPercent() &&
            nHeightPercent  == ((SwFmtFrmSize&)rAttr).GetHeightPercent() );
}

SfxPoolItem*  SwFmtFrmSize::Clone( SfxItemPool* ) const
{
    return new SwFmtFrmSize( *this );
}



bool SwFmtFrmSize::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
    case MID_FRMSIZE_SIZE:
    {
        awt::Size aTmp;
        aTmp.Height = TWIP_TO_MM100(aSize.Height());
        aTmp.Width = TWIP_TO_MM100(aSize.Width());
        rVal.setValue(&aTmp, ::getCppuType((const awt::Size*)0));
    }
    break;
    case MID_FRMSIZE_REL_HEIGHT:
        rVal <<= (sal_Int16)(GetHeightPercent() != 0xFF ? GetHeightPercent() : 0);
        break;
    case MID_FRMSIZE_REL_WIDTH:
        rVal <<= (sal_Int16)(GetWidthPercent() != 0xFF ? GetWidthPercent() : 0);
        break;
    case MID_FRMSIZE_IS_SYNC_HEIGHT_TO_WIDTH:
    {
        BOOL bTmp = 0xFF == GetHeightPercent();
        rVal.setValue(&bTmp, ::getBooleanCppuType());
    }
    break;
    case MID_FRMSIZE_IS_SYNC_WIDTH_TO_HEIGHT:
    {
        BOOL bTmp = 0xFF == GetWidthPercent();
        rVal.setValue(&bTmp, ::getBooleanCppuType());
    }
    break;
    case MID_FRMSIZE_WIDTH :
        rVal <<= (sal_Int32)TWIP_TO_MM100(aSize.Width());
        break;
    case MID_FRMSIZE_HEIGHT:
        // #95848# returned size should never be zero.
        // (there was a bug that allowed for setting height to 0.
        // Thus there some documents existing with that not allowed
        // attribut value which may cause problems on import.)
        rVal <<= (sal_Int32)TWIP_TO_MM100(aSize.Height() < MINLAY ? MINLAY : aSize.Height() );
        break;
    case MID_FRMSIZE_SIZE_TYPE:
        rVal <<= (sal_Int16)GetSizeType();
        break;
    case MID_FRMSIZE_IS_AUTO_HEIGHT:
    {
        BOOL bTmp = ATT_FIX_SIZE != GetSizeType();
        rVal.setValue(&bTmp, ::getBooleanCppuType());
    }
    break;
    }
    return true;
}


bool SwFmtFrmSize::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    sal_Bool bConvert = 0 != (nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch ( nMemberId )
    {
    case MID_FRMSIZE_SIZE:
    {
        awt::Size aVal;
        if(!(rVal >>= aVal))
            bRet = false;
        else
        {
            Size aTmp(aVal.Width, aVal.Height);
            if(bConvert)
            {
                aTmp.Height() = MM100_TO_TWIP(aTmp.Height());
                aTmp.Width() = MM100_TO_TWIP(aTmp.Width());
            }
            if(aTmp.Height() && aTmp.Width())
                aSize = aTmp;
            else
                bRet = false;
        }
    }
    break;
    case MID_FRMSIZE_REL_HEIGHT:
    {
        sal_Int16 nSet(0);
        rVal >>= nSet;
        if(nSet >= 0 && nSet <= 0xfe)
            SetHeightPercent((BYTE)nSet);
        else
            bRet = false;
    }
    break;
    case MID_FRMSIZE_REL_WIDTH:
    {
        sal_Int16 nSet(0);
        rVal >>= nSet;
        if(nSet >= 0 && nSet <= 0xfe)
            SetWidthPercent((BYTE)nSet);
        else
            bRet = false;
    }
    break;
    case MID_FRMSIZE_IS_SYNC_HEIGHT_TO_WIDTH:
    {
        sal_Bool bSet = *(sal_Bool*)rVal.getValue();
        if(bSet)
            SetHeightPercent(0xff);
        else if( 0xff == GetHeightPercent() )
            SetHeightPercent( 0 );
    }
    break;
    case MID_FRMSIZE_IS_SYNC_WIDTH_TO_HEIGHT:
    {
        sal_Bool bSet = *(sal_Bool*)rVal.getValue();
        if(bSet)
            SetWidthPercent(0xff);
        else if( 0xff == GetWidthPercent() )
            SetWidthPercent(0);
    }
    break;
    case MID_FRMSIZE_WIDTH :
    {
        sal_Int32 nWd(0);
        if(rVal >>= nWd)
        {
            if(bConvert)
                nWd = MM100_TO_TWIP(nWd);
            if(nWd > 0)
                aSize.Width() = nWd;
            else
                bRet = false;
        }
        else
            bRet = false;
    }
    break;
    case MID_FRMSIZE_HEIGHT:
    {
        sal_Int32 nHg(0);
        if(rVal >>= nHg)
        {
            if(bConvert)
                nHg = MM100_TO_TWIP(nHg);
            if(nHg > 0)
                aSize.Height() = nHg;
            else
                bRet = false;
        }
        else
            bRet = false;
    }
    break;
    case MID_FRMSIZE_SIZE_TYPE:
    {
        sal_Int16 nType(0);
        if((rVal >>= nType) && nType >= 0 && nType <= ATT_MIN_SIZE )
        {
            SetSizeType((SwFrmSize)nType);
        }
        else
            bRet = false;
    }
    break;
    case MID_FRMSIZE_IS_AUTO_HEIGHT:
    {
        sal_Bool bSet = *(sal_Bool*)rVal.getValue();
        SetSizeType(bSet ? ATT_VAR_SIZE : ATT_FIX_SIZE);
    }
    break;
    default:
        bRet = false;
    }
    return bRet;
}

Size  SwFmtFrmSize::GetSizeConvertedFromSw31(
    const SvxLRSpaceItem *pLRSpace,
    const SvxULSpaceItem *pULSpace ) const
{
    // Sw4.0: Groesse enthaelt keine Raender
    // Sw3.x: Groesse enthaelt Raender
    // ==> Raender subtrahieren
    Size aNewSize = GetSize();
    if( pLRSpace )
    {
        aNewSize.Width() -= pLRSpace->GetLeft();
        aNewSize.Width() -= pLRSpace->GetRight();
    }
    if( pULSpace )
    {
        aNewSize.Height() -= pULSpace->GetUpper();
        aNewSize.Height() -=  pULSpace->GetLower();
    }
    return aNewSize;
}


//  class SwFmtFillOrder
//  Implementierung teilweise inline im hxx

SwFmtFillOrder::SwFmtFillOrder( SwFillOrder nFO )
    : SfxEnumItem( RES_FILL_ORDER, sal_uInt16(nFO) )
{}

SfxPoolItem*  SwFmtFillOrder::Clone( SfxItemPool* ) const
{
    return new SwFmtFillOrder( GetFillOrder() );
}


//  class SwFmtHeader
//  Implementierung teilweise inline im hxx

SwFmtHeader::SwFmtHeader( SwFrmFmt *pHeaderFmt )
    : SfxPoolItem( RES_HEADER ),
      SwClient( pHeaderFmt ),
      bActive( pHeaderFmt ? sal_True : sal_False )
{
}

SwFmtHeader::SwFmtHeader( const SwFmtHeader &rCpy )
    : SfxPoolItem( RES_HEADER ),
      SwClient( (SwModify*)rCpy.GetRegisteredIn() ),
      bActive( rCpy.IsActive() )
{
}

SwFmtHeader::SwFmtHeader( sal_Bool bOn )
    : SfxPoolItem( RES_HEADER ),
      SwClient( 0 ),
      bActive( bOn )
{
}

SwFmtHeader::~SwFmtHeader()
{
    if ( GetHeaderFmt() )
        DelHFFormat( this, GetHeaderFmt() );
}

int  SwFmtHeader::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return ( pRegisteredIn == ((SwFmtHeader&)rAttr).GetRegisteredIn() &&
             bActive == ((SwFmtHeader&)rAttr).IsActive() );
}

SfxPoolItem*  SwFmtHeader::Clone( SfxItemPool* ) const
{
    return new SwFmtHeader( *this );
}

//  class SwFmtFooter
//  Implementierung teilweise inline im hxx

SwFmtFooter::SwFmtFooter( SwFrmFmt *pFooterFmt )
    : SfxPoolItem( RES_FOOTER ),
      SwClient( pFooterFmt ),
      bActive( pFooterFmt ? sal_True : sal_False )
{
}

SwFmtFooter::SwFmtFooter( const SwFmtFooter &rCpy )
    : SfxPoolItem( RES_FOOTER ),
      SwClient( (SwModify*)rCpy.GetRegisteredIn() ),
      bActive( rCpy.IsActive() )
{
}

SwFmtFooter::SwFmtFooter( sal_Bool bOn )
    : SfxPoolItem( RES_FOOTER ),
      SwClient( 0 ),
      bActive( bOn )
{
}

SwFmtFooter::~SwFmtFooter()
{
    if ( GetFooterFmt() )
        DelHFFormat( this, GetFooterFmt() );
}

int  SwFmtFooter::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return ( pRegisteredIn == ((SwFmtFooter&)rAttr).GetRegisteredIn() &&
             bActive == ((SwFmtFooter&)rAttr).IsActive() );
}

SfxPoolItem*  SwFmtFooter::Clone( SfxItemPool* ) const
{
    return new SwFmtFooter( *this );
}

//  class SwFmtCntnt
//  Implementierung teilweise inline im hxx

SwFmtCntnt::SwFmtCntnt( const SwFmtCntnt &rCpy )
    : SfxPoolItem( RES_CNTNT )
{
    pStartNode = rCpy.GetCntntIdx() ?
        new SwNodeIndex( *rCpy.GetCntntIdx() ) : 0;
}

SwFmtCntnt::SwFmtCntnt( const SwStartNode *pStartNd )
    : SfxPoolItem( RES_CNTNT )
{
    pStartNode = pStartNd ? new SwNodeIndex( *pStartNd ) : 0;
}

SwFmtCntnt::~SwFmtCntnt()
{
    delete pStartNode;
}

void SwFmtCntnt::SetNewCntntIdx( const SwNodeIndex *pIdx )
{
    delete pStartNode;
    pStartNode = pIdx ? new SwNodeIndex( *pIdx ) : 0;
}

int  SwFmtCntnt::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    if( (long)pStartNode ^ (long)((SwFmtCntnt&)rAttr).pStartNode )
        return 0;
    if( pStartNode )
        return ( *pStartNode == *((SwFmtCntnt&)rAttr).GetCntntIdx() );
    return 1;
}

SfxPoolItem*  SwFmtCntnt::Clone( SfxItemPool* ) const
{
    return new SwFmtCntnt( *this );
}

//  class SwFmtPageDesc
//  Implementierung teilweise inline im hxx

SwFmtPageDesc::SwFmtPageDesc( const SwFmtPageDesc &rCpy )
    : SfxPoolItem( RES_PAGEDESC ),
      SwClient( (SwPageDesc*)rCpy.GetPageDesc() ),
      pDefinedIn( 0 ),
      nNumOffset( rCpy.nNumOffset ),
      nDescNameIdx( rCpy.nDescNameIdx )
{
}

SwFmtPageDesc::SwFmtPageDesc( const SwPageDesc *pDesc )
    : SfxPoolItem( RES_PAGEDESC ),
      SwClient( (SwPageDesc*)pDesc ),
      pDefinedIn( 0 ),
      nNumOffset( 0 ),
      nDescNameIdx( 0xFFFF )  // IDX_NO_VALUE
{
}

SwFmtPageDesc::~SwFmtPageDesc() {}

int  SwFmtPageDesc::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return  ( pDefinedIn == ((SwFmtPageDesc&)rAttr).pDefinedIn ) &&
        ( nNumOffset == ((SwFmtPageDesc&)rAttr).nNumOffset ) &&
        ( GetPageDesc() == ((SwFmtPageDesc&)rAttr).GetPageDesc() );
}

SfxPoolItem*  SwFmtPageDesc::Clone( SfxItemPool* ) const
{
    return new SwFmtPageDesc( *this );
}

void SwFmtPageDesc::Modify( SfxPoolItem* pOld, SfxPoolItem* pNew )
{
    if( !pDefinedIn )
        return;

    sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    switch( nWhich )
    {
    case RES_OBJECTDYING:
        //Der Pagedesc, bei dem ich angemeldet bin stirbt, ich trage
        //mich also bei meinem Format aus.
        //Dabei werden ich Deletet!!!
        if( IS_TYPE( SwFmt, pDefinedIn ))
#ifdef DBG_UTIL
        {
            sal_Bool bDel = ((SwFmt*)pDefinedIn)->ResetAttr( RES_PAGEDESC );
            OSL_ENSURE( bDel, ";-) FmtPageDesc nicht zerstoert." );
        }
#else
        ((SwFmt*)pDefinedIn)->ResetAttr( RES_PAGEDESC );
#endif
        else if( IS_TYPE( SwCntntNode, pDefinedIn ))
#ifdef DBG_UTIL
        {
            sal_Bool bDel = ((SwCntntNode*)pDefinedIn)->ResetAttr( RES_PAGEDESC );
            OSL_ENSURE( bDel, ";-) FmtPageDesc nicht zerstoert." );
        }
#else
        ((SwCntntNode*)pDefinedIn)->ResetAttr( RES_PAGEDESC );
#endif
        break;

    default:
        /* do nothing */;
    }
}

bool SwFmtPageDesc::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    bool    bRet = true;
    switch ( nMemberId )
    {
    case MID_PAGEDESC_PAGENUMOFFSET:
        rVal <<= (sal_Int16)GetNumOffset();
        break;

    case MID_PAGEDESC_PAGEDESCNAME:
    {
        const SwPageDesc* pDesc = GetPageDesc();
        if( pDesc )
        {
            String aString;
            SwStyleNameMapper::FillProgName(pDesc->GetName(), aString, GET_POOLID_PAGEDESC, sal_True );
            rVal <<= OUString( aString );
        }
        else
            rVal.clear();
    }
    break;
    default:
        OSL_ENSURE( !this, "unknown MemberId" );
        bRet = false;
    }
    return bRet;
}

bool SwFmtPageDesc::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch ( nMemberId )
    {
    case MID_PAGEDESC_PAGENUMOFFSET:
    {
        sal_Int16 nOffset(0);
        if(rVal >>= nOffset)
            SetNumOffset( nOffset );
        else
            bRet = false;
    }
    break;

    case MID_PAGEDESC_PAGEDESCNAME:
        /* geht nicht, weil das Attribut eigentlich nicht den Namen
         * sondern einen Pointer auf den PageDesc braucht (ist Client davon).
         * Der Pointer waere aber ueber den Namen nur vom Dokument zu erfragen.
         */
    default:
        OSL_ENSURE( !this, "unknown MemberId" );
        bRet = false;
    }
    return bRet;
}


//  class SwFmtCol
//  Implementierung teilweise inline im hxx

SwColumn::SwColumn()
    : nWish ( 0 )
    , nUpper( 0 )
    , nLower( 0 )
    , nLeft ( 0 )
    , nRight( 0 )
{
}

sal_Bool SwColumn::operator==( const SwColumn &rCmp )
{
    return (nWish    == rCmp.GetWishWidth() &&
            GetLeft()  == rCmp.GetLeft() &&
            GetRight() == rCmp.GetRight() &&
            GetUpper() == rCmp.GetUpper() &&
            GetLower() == rCmp.GetLower()) ? sal_True : sal_False;
}

SwFmtCol::SwFmtCol( const SwFmtCol& rCpy )
    : SfxPoolItem( RES_COL )
    , nLineWidth( rCpy.nLineWidth)
    , aLineColor( rCpy.aLineColor)
    , nLineHeight( rCpy.GetLineHeight() )
    , eAdj( rCpy.GetLineAdj() )
    , aColumns( (sal_Int8)rCpy.GetNumCols(), 1 )
    , nWidth( rCpy.GetWishWidth() )
    , bOrtho( rCpy.IsOrtho() )
{
    for ( sal_uInt16 i = 0; i < rCpy.GetNumCols(); ++i )
    {
        SwColumn *pCol = new SwColumn( *rCpy.GetColumns()[i] );
        aColumns.Insert( pCol, aColumns.Count() );
    }
}

SwFmtCol::~SwFmtCol() {}

SwFmtCol& SwFmtCol::operator=( const SwFmtCol& rCpy )
{
    nLineWidth  = rCpy.nLineWidth;
    aLineColor  = rCpy.aLineColor;
    nLineHeight = rCpy.GetLineHeight();
    eAdj        = rCpy.GetLineAdj();
    nWidth      = rCpy.GetWishWidth();
    bOrtho      = rCpy.IsOrtho();

    if ( aColumns.Count() )
        aColumns.DeleteAndDestroy( 0, aColumns.Count() );
    for ( sal_uInt16 i = 0; i < rCpy.GetNumCols(); ++i )
    {
        SwColumn *pCol = new SwColumn( *rCpy.GetColumns()[i] );
        aColumns.Insert( pCol, aColumns.Count() );
    }
    return *this;
}

SwFmtCol::SwFmtCol()
    : SfxPoolItem( RES_COL )
    , nLineWidth(0)
    , nLineHeight( 100 )
    , eAdj( COLADJ_NONE )
    , nWidth( USHRT_MAX )
    , bOrtho( sal_True )
{
}

int SwFmtCol::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    const SwFmtCol &rCmp = (const SwFmtCol&)rAttr;
    if( !(nLineWidth        == rCmp.nLineWidth  &&
          aLineColor        == rCmp.aLineColor  &&
          nLineHeight        == rCmp.GetLineHeight() &&
          eAdj               == rCmp.GetLineAdj() &&
          nWidth             == rCmp.GetWishWidth() &&
          bOrtho             == rCmp.IsOrtho() &&
          aColumns.Count() == rCmp.GetNumCols()) )
        return 0;

    for ( sal_uInt16 i = 0; i < aColumns.Count(); ++i )
        if ( !(*aColumns[i] == *rCmp.GetColumns()[i]) )
            return 0;

    return 1;
}

SfxPoolItem*  SwFmtCol::Clone( SfxItemPool* ) const
{
    return new SwFmtCol( *this );
}

sal_uInt16 SwFmtCol::GetGutterWidth( sal_Bool bMin ) const
{
    sal_uInt16 nRet = 0;
    if ( aColumns.Count() == 2 )
        nRet = aColumns[0]->GetRight() + aColumns[1]->GetLeft();
    else if ( aColumns.Count() > 2 )
    {
        sal_Bool bSet = sal_False;
        for ( sal_uInt16 i = 1; i < aColumns.Count()-1; ++i )
        {
            const sal_uInt16 nTmp = aColumns[i]->GetRight() + aColumns[i+1]->GetLeft();
            if ( bSet )
            {
                if ( nTmp != nRet )
                {
                    if ( !bMin )
                        return USHRT_MAX;
                    if ( nRet > nTmp )
                        nRet = nTmp;
                }
            }
            else
            {   bSet = sal_True;
                nRet = nTmp;
            }
        }
    }
    return nRet;
}


void SwFmtCol::Init( sal_uInt16 nNumCols, sal_uInt16 nGutterWidth, sal_uInt16 nAct )
{
    //Loeschen scheint hier auf den erste Blick vielleicht etwas zu heftig;
    //anderfalls muessten allerdings alle Werte der verbleibenden SwColumn's
    //initialisiert werden.
    if ( aColumns.Count() )
        aColumns.DeleteAndDestroy( 0, aColumns.Count() );
    for ( sal_uInt16 i = 0; i < nNumCols; ++i )
    {   SwColumn *pCol = new SwColumn;
        aColumns.Insert( pCol, i );
    }
    bOrtho = sal_True;
    nWidth = USHRT_MAX;
    if( nNumCols )
        Calc( nGutterWidth, nAct );
}


sal_uInt16 SwFmtCol::CalcColWidth( sal_uInt16 nCol, sal_uInt16 nAct ) const
{
    OSL_ENSURE( nCol < aColumns.Count(), ":-( ColumnsArr ueberindiziert." );
    if ( nWidth != nAct )
    {
        long nW = aColumns[nCol]->GetWishWidth();
        nW *= nAct;
        nW /= nWidth;
        return sal_uInt16(nW);
    }
    else
        return aColumns[nCol]->GetWishWidth();
}


void SwFmtCol::Calc( sal_uInt16 nGutterWidth, sal_uInt16 nAct )
{
    //Erstmal die Spalten mit der Aktuellen Breite einstellen, dann die
    //Wunschbreite der Spalten anhand der Gesamtwunschbreite hochrechnen.

    const sal_uInt16 nGutterHalf = nGutterWidth ? nGutterWidth / 2 : 0;

    //Breite der PrtAreas ist Gesamtbreite - Zwischenraeume / Anzahl
    const sal_uInt16 nPrtWidth =
        (nAct - ((GetNumCols()-1) * nGutterWidth)) / GetNumCols();
    sal_uInt16 nAvail = nAct;

    //Die erste Spalte ist PrtBreite + (Zwischenraumbreite/2)
    const sal_uInt16 nLeftWidth = nPrtWidth + nGutterHalf;
    SwColumn *pCol = aColumns[0];
    pCol->SetWishWidth( nLeftWidth );
    pCol->SetRight( nGutterHalf );
    pCol->SetLeft ( 0 );
    nAvail -= nLeftWidth;

    //Spalte 2 bis n-1 ist PrtBreite + Zwischenraumbreite
    const sal_uInt16 nMidWidth = nPrtWidth + nGutterWidth;
    sal_uInt16 i; for ( i = 1; i < GetNumCols()-1; ++i )
                  {
                      pCol = aColumns[i];
                      pCol->SetWishWidth( nMidWidth );
                      pCol->SetLeft ( nGutterHalf );
                      pCol->SetRight( nGutterHalf );
                      nAvail -= nMidWidth;
                  }

    //Die Letzte Spalte entspricht wieder der ersten, um Rundungsfehler
    //auszugleichen wird der letzten Spalte alles zugeschlagen was die
    //anderen nicht verbraucht haben.
    pCol = aColumns[aColumns.Count()-1];
    pCol->SetWishWidth( nAvail );
    pCol->SetLeft ( nGutterHalf );
    pCol->SetRight( 0 );

    //Umrechnen der aktuellen Breiten in Wunschbreiten.
    for ( i = 0; i < aColumns.Count(); ++i )
    {
        pCol = aColumns[i];
        long nTmp = pCol->GetWishWidth();
        nTmp *= GetWishWidth();
        nTmp /= nAct;
        pCol->SetWishWidth( sal_uInt16(nTmp) );
    }
}

bool SwFmtCol::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    if(MID_COLUMN_SEPARATOR_LINE == nMemberId)
    {
        OSL_FAIL("not implemented");
    }
    else
    {
        uno::Reference< text::XTextColumns >  xCols = new SwXTextColumns(*this);
        rVal.setValue(&xCols, ::getCppuType((uno::Reference< text::XTextColumns>*)0));
    }
    return true;
}

bool SwFmtCol::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = false;
    if(MID_COLUMN_SEPARATOR_LINE == nMemberId)
    {
        OSL_FAIL("not implemented");
    }
    else
    {
        uno::Reference< text::XTextColumns > xCols;
        rVal >>= xCols;
        if(xCols.is())
        {
            uno::Sequence<text::TextColumn> aSetColumns = xCols->getColumns();
            const text::TextColumn* pArray = aSetColumns.getConstArray();
            aColumns.DeleteAndDestroy(0, aColumns.Count());
            //max. Count ist hier 64K - das kann das Array aber nicht
            sal_uInt16 nCount = Min( (sal_uInt16)aSetColumns.getLength(),
                                     (sal_uInt16) 0x3fff );
            sal_uInt16 nWidthSum = 0;
            // #101224# one column is no column
            //
            if(nCount > 1)
                for(sal_uInt16 i = 0; i < nCount; i++)
                {
                    SwColumn* pCol = new SwColumn;
                    pCol->SetWishWidth( pArray[i].Width );
                    nWidthSum += pArray[i].Width;
                    pCol->SetLeft ( MM100_TO_TWIP(pArray[i].LeftMargin) );
                    pCol->SetRight( MM100_TO_TWIP(pArray[i].RightMargin) );
                    aColumns.Insert(pCol, i);
                }
            bRet = true;
            nWidth = nWidthSum;
            bOrtho = sal_False;

            uno::Reference<lang::XUnoTunnel> xNumTunnel(xCols, uno::UNO_QUERY);
            SwXTextColumns* pSwColums = 0;
            if(xNumTunnel.is())
            {
                pSwColums = (SwXTextColumns*)
                    xNumTunnel->getSomething( SwXTextColumns::getUnoTunnelId() );
            }
            if(pSwColums)
            {
                bOrtho = pSwColums->IsAutomaticWidth();
                nLineWidth = pSwColums->GetSepLineWidth();
                aLineColor.SetColor(pSwColums->GetSepLineColor());
                nLineHeight = pSwColums->GetSepLineHeightRelative();
                if(!pSwColums->GetSepLineIsOn())
                    eAdj = COLADJ_NONE;
                else switch(pSwColums->GetSepLineVertAlign())
                     {
                     case 0: eAdj = COLADJ_TOP;  break;  //VerticalAlignment_TOP
                     case 1: eAdj = COLADJ_CENTER;break; //VerticalAlignment_MIDDLE
                     case 2: eAdj = COLADJ_BOTTOM;break; //VerticalAlignment_BOTTOM
                     }
            }
        }
    }
    return bRet;
}


//  class SwFmtSurround
//  Implementierung teilweise inline im hxx

SwFmtSurround::SwFmtSurround( SwSurround eFly ) :
    SfxEnumItem( RES_SURROUND, sal_uInt16( eFly ) )
{
    bAnchorOnly = bContour = bOutside = sal_False;
}

SwFmtSurround::SwFmtSurround( const SwFmtSurround &rCpy ) :
    SfxEnumItem( RES_SURROUND, rCpy.GetValue() )
{
    bAnchorOnly = rCpy.bAnchorOnly;
    bContour = rCpy.bContour;
    bOutside = rCpy.bOutside;
}

int  SwFmtSurround::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return ( GetValue() == ((SwFmtSurround&)rAttr).GetValue() &&
             bAnchorOnly== ((SwFmtSurround&)rAttr).bAnchorOnly &&
             bContour== ((SwFmtSurround&)rAttr).bContour &&
             bOutside== ((SwFmtSurround&)rAttr).bOutside );
}

SfxPoolItem*  SwFmtSurround::Clone( SfxItemPool* ) const
{
    return new SwFmtSurround( *this );
}



bool SwFmtSurround::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch ( nMemberId )
    {
    case MID_SURROUND_SURROUNDTYPE:
        rVal <<= (text::WrapTextMode)GetSurround();
        break;
    case MID_SURROUND_ANCHORONLY:
    {
        BOOL bTmp = IsAnchorOnly();
        rVal.setValue(&bTmp, ::getBooleanCppuType());
    }
    break;
    case MID_SURROUND_CONTOUR:
    {
        BOOL bTmp = IsContour();
        rVal.setValue(&bTmp, ::getBooleanCppuType());
    }
    break;
    case MID_SURROUND_CONTOUROUTSIDE:
    {
        BOOL bTmp = IsOutside();
        rVal.setValue(&bTmp, ::getBooleanCppuType());
    }
    break;
    default:
        OSL_ENSURE( !this, "unknown MemberId" );
        bRet = false;
    }
    return bRet;
}

bool SwFmtSurround::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool bRet = sal_True;
    switch ( nMemberId )
    {
    case MID_SURROUND_SURROUNDTYPE:
    {
        sal_Int32 eVal = SWUnoHelper::GetEnumAsInt32( rVal );
        if( eVal >= 0 && eVal < (sal_Int16)SURROUND_END )
            SetValue( eVal );
        else
        {}    //exception
    }
    break;

    case MID_SURROUND_ANCHORONLY:
        SetAnchorOnly( *(sal_Bool*)rVal.getValue() );
        break;
    case MID_SURROUND_CONTOUR:
        SetContour( *(sal_Bool*)rVal.getValue() );
        break;
    case MID_SURROUND_CONTOUROUTSIDE:
        SetOutside( *(sal_Bool*)rVal.getValue() );
        break;
    default:
        OSL_ENSURE( !this, "unknown MemberId" );
        bRet = false;
    }
    return bRet;
}

//  class SwFmtVertOrient
//  Implementierung teilweise inline im hxx

SwFmtVertOrient::SwFmtVertOrient( SwTwips nY, SwVertOrient eVert,
                                  SwRelationOrient eRel )
    : SfxPoolItem( RES_VERT_ORIENT ),
      nYPos( nY ),
      eOrient( eVert ),
      eRelation( eRel )
{}

int  SwFmtVertOrient::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return ( nYPos     == ((SwFmtVertOrient&)rAttr).nYPos &&
             eOrient   == ((SwFmtVertOrient&)rAttr).eOrient &&
             eRelation == ((SwFmtVertOrient&)rAttr).eRelation );
}

SfxPoolItem*  SwFmtVertOrient::Clone( SfxItemPool* ) const
{
    return new SwFmtVertOrient( nYPos, eOrient, eRelation );
}


SwTwips  SwFmtVertOrient::GetPosConvertedFromSw31(
    const SvxULSpaceItem *pULSpace ) const
{
    SwTwips nNewPos = GetPos();

    if( VERT_NONE==GetVertOrient() && pULSpace )
    {
        nNewPos += pULSpace->GetUpper();
    }

    return nNewPos;
}


bool SwFmtVertOrient::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch ( nMemberId )
    {
    case MID_VERTORIENT_ORIENT:
    {
        sal_Int16 nRet = text::VertOrientation::NONE;
        switch( eOrient )
        {
        case VERT_TOP        :  nRet = text::VertOrientation::TOP        ;break;
        case VERT_CENTER     :  nRet = text::VertOrientation::CENTER     ;break;
        case VERT_BOTTOM     :  nRet = text::VertOrientation::BOTTOM     ;break;
        case VERT_CHAR_TOP   :  nRet = text::VertOrientation::CHAR_TOP   ;break;
        case VERT_CHAR_CENTER:  nRet = text::VertOrientation::CHAR_CENTER;break;
        case VERT_CHAR_BOTTOM:  nRet = text::VertOrientation::CHAR_BOTTOM;break;
        case VERT_LINE_TOP   :  nRet = text::VertOrientation::LINE_TOP   ;break;
        case VERT_LINE_CENTER:  nRet = text::VertOrientation::LINE_CENTER;break;
        case VERT_LINE_BOTTOM:  nRet = text::VertOrientation::LINE_BOTTOM;break;
        case VERT_NONE:         break;
        }
        rVal <<= nRet;
    }
    break;
    case MID_VERTORIENT_RELATION:
        rVal <<= lcl_RelToINT(eRelation);
        break;
    case MID_VERTORIENT_POSITION:
        rVal <<= (sal_Int32)TWIP_TO_MM100(GetPos());
        break;
    default:
        OSL_ENSURE( !this, "unknown MemberId" );
        bRet = false;
    }
    return bRet;
}

bool SwFmtVertOrient::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    sal_Bool bConvert = 0 != (nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch ( nMemberId )
    {
    case MID_VERTORIENT_ORIENT:
    {
        sal_uInt16 nVal(0);
        rVal >>= nVal;
        switch( nVal )
        {
        case text::VertOrientation::NONE:           eOrient = VERT_NONE;    break;
        case text::VertOrientation::TOP        :    eOrient = VERT_TOP;     break;
        case text::VertOrientation::CENTER     :    eOrient = VERT_CENTER;     break;
        case text::VertOrientation::BOTTOM     :    eOrient = VERT_BOTTOM;     break;
        case text::VertOrientation::CHAR_TOP   :    eOrient = VERT_CHAR_TOP;   break;
        case text::VertOrientation::CHAR_CENTER:    eOrient = VERT_CHAR_CENTER;break;
        case text::VertOrientation::CHAR_BOTTOM:    eOrient = VERT_CHAR_BOTTOM;break;
        case text::VertOrientation::LINE_TOP   :    eOrient = VERT_LINE_TOP;    break;
        case text::VertOrientation::LINE_CENTER:    eOrient = VERT_LINE_CENTER;break;
        case text::VertOrientation::LINE_BOTTOM:    eOrient = VERT_LINE_BOTTOM;break;
        }
    }
    break;
    case MID_VERTORIENT_RELATION:
    {
        eRelation = lcl_IntToRelation(rVal);
    }
    break;
    case MID_VERTORIENT_POSITION:
    {
        sal_Int32 nVal(0);
        rVal >>= nVal;
        if(bConvert)
            nVal = MM100_TO_TWIP(nVal);
        SetPos( nVal );
    }
    break;
    default:
        OSL_ENSURE( !this, "unknown MemberId" );
        bRet = false;
    }
    return bRet;
}



//  class SwFmtHoriOrient
//  Implementierung teilweise inline im hxx

SwFmtHoriOrient::SwFmtHoriOrient( SwTwips nX, SwHoriOrient eHori,
                                  SwRelationOrient eRel, sal_Bool bPos )
    : SfxPoolItem( RES_HORI_ORIENT ),
      nXPos( nX ),
      eOrient( eHori ),
      eRelation( eRel ),
      bPosToggle( bPos )
{}

int  SwFmtHoriOrient::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return ( nXPos == ((SwFmtHoriOrient&)rAttr).nXPos &&
             eOrient == ((SwFmtHoriOrient&)rAttr).eOrient &&
             eRelation == ((SwFmtHoriOrient&)rAttr).eRelation &&
             bPosToggle == ((SwFmtHoriOrient&)rAttr).bPosToggle );
}

SfxPoolItem*  SwFmtHoriOrient::Clone( SfxItemPool* ) const
{
    return new SwFmtHoriOrient( nXPos, eOrient, eRelation, bPosToggle );
}


SwTwips  SwFmtHoriOrient::GetPosConvertedFromSw31(
    const SvxLRSpaceItem *pLRSpace ) const
{
    SwTwips nNewPos = GetPos();

    if( HORI_NONE==GetHoriOrient() && pLRSpace )
    {
        nNewPos += pLRSpace->GetLeft();
    }

    return nNewPos;
}

bool SwFmtHoriOrient::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch ( nMemberId )
    {
    case MID_HORIORIENT_ORIENT:
    {
        sal_Int16 nRet = text::HoriOrientation::NONE;
        switch( eOrient )
        {
        case HORI_RIGHT:    nRet = text::HoriOrientation::RIGHT; break;
        case HORI_CENTER :  nRet = text::HoriOrientation::CENTER; break;
        case HORI_LEFT   :  nRet = text::HoriOrientation::LEFT; break;
        case HORI_INSIDE :  nRet = text::HoriOrientation::INSIDE; break;
        case HORI_OUTSIDE:  nRet = text::HoriOrientation::OUTSIDE; break;
        case HORI_FULL:     nRet = text::HoriOrientation::FULL; break;
        case HORI_LEFT_AND_WIDTH :
            nRet = text::HoriOrientation::LEFT_AND_WIDTH;
            break;
        case HORI_NONE:     break;
        }
        rVal <<= nRet;
    }
    break;
    case MID_HORIORIENT_RELATION:
        rVal <<= lcl_RelToINT(eRelation);
        break;
    case MID_HORIORIENT_POSITION:
        rVal <<= (sal_Int32)TWIP_TO_MM100(GetPos());
        break;
    case MID_HORIORIENT_PAGETOGGLE:
    {
        BOOL bTmp = IsPosToggle();
        rVal.setValue(&bTmp, ::getBooleanCppuType());
    }
    break;
    default:
        OSL_ENSURE( !this, "unknown MemberId" );
        bRet = false;
    }
    return bRet;
}

bool SwFmtHoriOrient::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    sal_Bool bConvert = 0 != (nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch ( nMemberId )
    {
    case MID_HORIORIENT_ORIENT:
    {
        sal_Int16 nVal(0);
        rVal >>= nVal;
        switch( nVal )
        {
        case text::HoriOrientation::NONE:       eOrient = HORI_NONE ;   break;
        case text::HoriOrientation::RIGHT:  eOrient = HORI_RIGHT;   break;
        case text::HoriOrientation::CENTER :    eOrient = HORI_CENTER;  break;
        case text::HoriOrientation::LEFT   :    eOrient = HORI_LEFT;    break;
        case text::HoriOrientation::INSIDE :    eOrient = HORI_INSIDE;  break;
        case text::HoriOrientation::OUTSIDE:    eOrient = HORI_OUTSIDE; break;
        case text::HoriOrientation::FULL:      eOrient = HORI_FULL;     break;
        case text::HoriOrientation::LEFT_AND_WIDTH:
            eOrient = HORI_LEFT_AND_WIDTH;
            break;
        }
    }
    break;
    case MID_HORIORIENT_RELATION:
    {
        eRelation = lcl_IntToRelation(rVal);
    }
    break;
    case MID_HORIORIENT_POSITION:
    {
        sal_Int32 nVal(0);
        if(!(rVal >>= nVal))
            bRet = false;
        if(bConvert)
            nVal = MM100_TO_TWIP(nVal);
        SetPos( nVal );
    }
    break;
    case MID_HORIORIENT_PAGETOGGLE:
        SetPosToggle( *(sal_Bool*)rVal.getValue());
        break;
    default:
        OSL_ENSURE( !this, "unknown MemberId" );
        bRet = false;
    }
    return bRet;
}



//  class SwFmtAnchor
//  Implementierung teilweise inline im hxx

SwFmtAnchor::SwFmtAnchor( RndStdIds nRnd, sal_uInt16 nPage )
    : SfxPoolItem( RES_ANCHOR ),
      pCntntAnchor( 0 ),
      nAnchorId( nRnd ),
      nPageNum( nPage )
{}

SwFmtAnchor::SwFmtAnchor( const SwFmtAnchor &rCpy )
    : SfxPoolItem( RES_ANCHOR ),
      nAnchorId( rCpy.GetAnchorId() ),
      nPageNum( rCpy.GetPageNum() )
{
    pCntntAnchor = rCpy.GetCntntAnchor() ?
        new SwPosition( *rCpy.GetCntntAnchor() ) : 0;
}

SwFmtAnchor::~SwFmtAnchor()
{
    delete pCntntAnchor;
}

void SwFmtAnchor::SetAnchor( const SwPosition *pPos )
{
    if ( pCntntAnchor )
        delete pCntntAnchor;
    pCntntAnchor = pPos ? new SwPosition( *pPos ) : 0;
    //AM Absatz gebundene Flys sollten nie in den Absatz hineinzeigen.
    if ( pCntntAnchor && ( FLY_AT_CNTNT == nAnchorId ||
                           FLY_AT_FLY == nAnchorId ))
        pCntntAnchor->nContent.Assign( 0, 0 );
}

SwFmtAnchor& SwFmtAnchor::operator=(const SwFmtAnchor& rAnchor)
{
    nAnchorId  = rAnchor.GetAnchorId();
    nPageNum   = rAnchor.GetPageNum();

    delete pCntntAnchor;
    pCntntAnchor = rAnchor.pCntntAnchor ?
        new SwPosition(*(rAnchor.pCntntAnchor)) : 0;
    return *this;
}

int  SwFmtAnchor::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return ( nAnchorId == ((SwFmtAnchor&)rAttr).GetAnchorId() &&
             nPageNum == ((SwFmtAnchor&)rAttr).GetPageNum()   &&
             //Anker vergleichen. Entweder zeigen beide auf das gleiche
             //Attribut bzw. sind 0 oder die SwPosition* sind beide
             //gueltig und die SwPositions sind gleich.
             (pCntntAnchor == ((SwFmtAnchor&)rAttr).GetCntntAnchor() ||
              (pCntntAnchor && ((SwFmtAnchor&)rAttr).GetCntntAnchor() &&
               *pCntntAnchor == *((SwFmtAnchor&)rAttr).GetCntntAnchor())));
}

SfxPoolItem*  SwFmtAnchor::Clone( SfxItemPool* ) const
{
    return new SwFmtAnchor( *this );
}


bool SwFmtAnchor::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch ( nMemberId )
    {
    case MID_ANCHOR_ANCHORTYPE:

        text::TextContentAnchorType eRet;
        switch((sal_Int16)GetAnchorId())
        {
        case  FLY_AUTO_CNTNT : eRet = text::TextContentAnchorType_AT_CHARACTER;break;
        case  FLY_PAGE       : eRet = text::TextContentAnchorType_AT_PAGE;      break;
        case  FLY_AT_FLY     : eRet = text::TextContentAnchorType_AT_FRAME;    break;
        case  FLY_IN_CNTNT   : eRet = text::TextContentAnchorType_AS_CHARACTER;break;
            //case  FLY_AT_CNTNT  :
        default: eRet = text::TextContentAnchorType_AT_PARAGRAPH;
        }
        rVal <<= eRet;
        break;
    case MID_ANCHOR_PAGENUM:
        rVal <<= (sal_Int16)GetPageNum();
        break;
    case MID_ANCHOR_ANCHORFRAME:
    {
        if(pCntntAnchor && FLY_AT_FLY == nAnchorId)
        {
            SwFrmFmt* pFmt = pCntntAnchor->nNode.GetNode().GetFlyFmt();
            if(pFmt)
            {
                Reference<XNamed> xNamed = SwXFrames::GetObject( *pFmt, FLYCNTTYPE_FRM );
                Reference<XTextFrame> xRet(xNamed, UNO_QUERY);
                rVal <<= xRet;
            }
        }
    }
    break;
    default:
        OSL_ENSURE( !this, "unknown MemberId" );
        bRet = false;
    }
    return bRet;
}

bool SwFmtAnchor::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch ( nMemberId )
    {
    case MID_ANCHOR_ANCHORTYPE:
    {
        RndStdIds   eAnchor;
        switch( SWUnoHelper::GetEnumAsInt32( rVal ) )
        {
        case  text::TextContentAnchorType_AS_CHARACTER:
            eAnchor = FLY_IN_CNTNT;
            break;
        case  text::TextContentAnchorType_AT_PAGE:
            eAnchor = FLY_PAGE;
            if( GetPageNum() > 0 && pCntntAnchor )
            {
                // If the anchor type is page and a valid page number
                // has been set, the content position isn't required
                // any longer.
                delete pCntntAnchor;
                pCntntAnchor = 0;
            }
            break;
        case  text::TextContentAnchorType_AT_FRAME:
            eAnchor = FLY_AT_FLY;
            break;
        case  text::TextContentAnchorType_AT_CHARACTER:
            eAnchor = FLY_AUTO_CNTNT;
            break;
            //case  text::TextContentAnchorType_AT_PARAGRAPH:
        default:
            eAnchor = FLY_AT_CNTNT;
            break;
        }
        SetType( eAnchor );
    }
    break;
    case MID_ANCHOR_PAGENUM:
    {
        sal_Int16 nVal(0);
        if((rVal >>= nVal) && nVal > 0)
        {
            SetPageNum( nVal );
            if( FLY_PAGE == GetAnchorId() && pCntntAnchor )
            {
                // If the anchor type is page and a valid page number
                // is set, the content paoition has to be deleted to not
                // confuse the layout (frmtool.cxx). However, if the
                // anchor type is not page, any content position will
                // be kept.
                delete pCntntAnchor;
                pCntntAnchor = 0;
            }
        }
        else
            bRet = false;
    }
    break;
    case MID_ANCHOR_ANCHORFRAME:
        //no break here!;
    default:
        OSL_ENSURE( !this, "unknown MemberId" );
        bRet = false;
    }
    return bRet;
}

//  class SwFmtURL
//  Implementierung teilweise inline im hxx

SwFmtURL::SwFmtURL()
    : SfxPoolItem( RES_URL )
    , pMap( 0 )
    , bIsServerMap( sal_False )
{
}

SwFmtURL::SwFmtURL( const SwFmtURL &rURL)
    : SfxPoolItem( RES_URL )
    , sTargetFrameName( rURL.GetTargetFrameName() )
    , sURL( rURL.GetURL() )
    , sName( rURL.GetName() )
    , bIsServerMap( rURL.IsServerMap() )
{
    pMap = rURL.GetMap() ? new ImageMap( *rURL.GetMap() ) : 0;
}

SwFmtURL::~SwFmtURL()
{
    if ( pMap )
        delete pMap;
}

int SwFmtURL::operator==( const SfxPoolItem &rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    const SwFmtURL &rCmp = (SwFmtURL&)rAttr;
    sal_Bool bRet = bIsServerMap     == rCmp.IsServerMap() &&
        sURL             == rCmp.GetURL() &&
        sTargetFrameName == rCmp.GetTargetFrameName() &&
        sName            == rCmp.GetName();
    if ( bRet )
    {
        if ( pMap && rCmp.GetMap() )
            bRet = *pMap == *rCmp.GetMap();
        else
            bRet = pMap == rCmp.GetMap();
    }
    return bRet;
}

SfxPoolItem* SwFmtURL::Clone( SfxItemPool* /*pPool*/ ) const
{
    return new SwFmtURL( *this );
}

void SwFmtURL::SetURL( const XubString &rURL, sal_Bool bServerMap )
{
    sURL = rURL;
    bIsServerMap = bServerMap;
}

void SwFmtURL::SetMap( const ImageMap *pM )
{
    if ( pMap )
        delete pMap;
    pMap = pM ? new ImageMap( *pM ) : 0;
}
extern const SvEventDescription* lcl_GetSupportedMacroItems();

bool SwFmtURL::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch ( nMemberId )
    {
    case MID_URL_URL:
    {
        OUString sRet = GetURL();
        rVal <<= sRet;
    }
    break;
    case MID_URL_TARGET:
    {
        OUString sRet = GetTargetFrameName();
        rVal <<= sRet;
    }
    break;
    case MID_URL_HYPERLINKNAME:
        rVal <<= OUString( GetName() );
        break;
    case MID_URL_CLIENTMAP:
    {
        Reference< XInterface > xInt;
        if(pMap)
        {
            xInt = SvUnoImageMap_createInstance( *pMap, lcl_GetSupportedMacroItems() );
        }
        else
        {
            ImageMap aEmptyMap;
            xInt = SvUnoImageMap_createInstance( aEmptyMap, lcl_GetSupportedMacroItems() );
        }
        Reference< XIndexContainer > xCont(xInt, UNO_QUERY);
        rVal <<= xCont;
    }
    break;
    case MID_URL_SERVERMAP:
    {
        BOOL bTmp = IsServerMap();
        rVal.setValue(&bTmp, ::getBooleanCppuType());
    }
    break;
    default:
        OSL_ENSURE( !this, "unknown MemberId" );
        bRet = false;
    }
    return bRet;
}

bool SwFmtURL::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch ( nMemberId )
    {
    case MID_URL_URL:
    {
        OUString sTmp;
        rVal >>= sTmp;
        SetURL( sTmp, bIsServerMap );
    }
    break;
    case MID_URL_TARGET:
    {
        OUString sTmp;
        rVal >>= sTmp;
        SetTargetFrameName( sTmp );
    }
    break;
    case MID_URL_HYPERLINKNAME:
    {
        OUString sTmp;
        rVal >>= sTmp;
        SetName( sTmp );
    }
    break;
    case MID_URL_CLIENTMAP:
    {
        Reference<XIndexContainer> xCont;
        if(!rVal.hasValue())
            DELETEZ(pMap);
        else if(rVal >>= xCont)
        {
            if(!pMap)
                pMap = new ImageMap;
            bRet = SvUnoImageMap_fillImageMap( xCont, *pMap );
        }
        else
            bRet = false;
    }
    break;
    case MID_URL_SERVERMAP:
        bIsServerMap = *(sal_Bool*)rVal.getValue();
        break;
    default:
        OSL_ENSURE( !this, "unknown MemberId" );
        bRet = false;
    }
    return bRet;
}


// class SwNoReadOnly

SfxPoolItem* SwFmtEditInReadonly::Clone( SfxItemPool* /*pPool*/ ) const
{
    return new SwFmtEditInReadonly( Which(), GetValue() );
}

// class SwFmtLayoutSplit

SfxPoolItem* SwFmtLayoutSplit::Clone( SfxItemPool* /*pPool*/ ) const
{
    return new SwFmtLayoutSplit( GetValue() );
}

// class SwFmtNoBalancedColumns

SfxPoolItem* SwFmtNoBalancedColumns::Clone( SfxItemPool* /*pPool*/ ) const
{
    return new SwFmtNoBalancedColumns( GetValue() );
}

// class SwFmtFtnEndAtTxtEnd


SwFmtFtnEndAtTxtEnd& SwFmtFtnEndAtTxtEnd::operator=(
    const SwFmtFtnEndAtTxtEnd& rAttr )
{
    SfxEnumItem::SetValue( rAttr.GetValue() );
    aFmt = rAttr.aFmt;
    nOffset = rAttr.nOffset;
    sPrefix = rAttr.sPrefix;
    sSuffix = rAttr.sSuffix;
    return *this;
}

int SwFmtFtnEndAtTxtEnd::operator==( const SfxPoolItem& rItem ) const
{
    const SwFmtFtnEndAtTxtEnd& rAttr = (SwFmtFtnEndAtTxtEnd&)rItem;
    return SfxEnumItem::operator==( rAttr ) &&
        aFmt.GetNumberingType() == rAttr.aFmt.GetNumberingType() &&
        nOffset == rAttr.nOffset &&
        sPrefix == rAttr.sPrefix &&
        sSuffix == rAttr.sSuffix;
}

bool SwFmtFtnEndAtTxtEnd::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
    case MID_COLLECT     :
    {
        sal_Bool bVal = GetValue() >= FTNEND_ATTXTEND;
        rVal.setValue(&bVal, ::getBooleanCppuType());
    }
    break;
    case MID_RESTART_NUM :
    {
        sal_Bool bVal = GetValue() >= FTNEND_ATTXTEND_OWNNUMSEQ;
        rVal.setValue(&bVal, ::getBooleanCppuType());
    }
    break;
    case MID_NUM_START_AT: rVal <<= (sal_Int16) nOffset; break;
    case MID_OWN_NUM     :
    {
        sal_Bool bVal = GetValue() >= FTNEND_ATTXTEND_OWNNUMANDFMT;
        rVal.setValue(&bVal, ::getBooleanCppuType());
    }
    break;
    case MID_NUM_TYPE    : rVal <<= aFmt.GetNumberingType(); break;
    case MID_PREFIX      : rVal <<= OUString(sPrefix); break;
    case MID_SUFFIX      : rVal <<= OUString(sSuffix); break;
    default: return false;
    }
    return true;
}

bool SwFmtFtnEndAtTxtEnd::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    bool bRet = true;
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
    case MID_COLLECT     :
    {
        sal_Bool bVal = *(sal_Bool*)rVal.getValue();
        if(!bVal && GetValue() >= FTNEND_ATTXTEND)
            SetValue(FTNEND_ATPGORDOCEND);
        else if(bVal && GetValue() < FTNEND_ATTXTEND)
            SetValue(FTNEND_ATTXTEND);
    }
    break;
    case MID_RESTART_NUM :
    {
        sal_Bool bVal = *(sal_Bool*)rVal.getValue();
        if(!bVal && GetValue() >= FTNEND_ATTXTEND_OWNNUMSEQ)
            SetValue(FTNEND_ATTXTEND);
        else if(bVal && GetValue() < FTNEND_ATTXTEND_OWNNUMSEQ)
            SetValue(FTNEND_ATTXTEND_OWNNUMSEQ);
    }
    break;
    case MID_NUM_START_AT:
    {
        sal_Int16 nVal(0);
        rVal >>= nVal;
        if(nVal >= 0)
            nOffset = nVal;
        else
            bRet = false;
    }
    break;
    case MID_OWN_NUM     :
    {
        sal_Bool bVal = *(sal_Bool*)rVal.getValue();
        if(!bVal && GetValue() >= FTNEND_ATTXTEND_OWNNUMANDFMT)
            SetValue(FTNEND_ATTXTEND_OWNNUMSEQ);
        else if(bVal && GetValue() < FTNEND_ATTXTEND_OWNNUMANDFMT)
            SetValue(FTNEND_ATTXTEND_OWNNUMANDFMT);
    }
    break;
    case MID_NUM_TYPE    :
    {
        sal_Int16 nVal(0);
        rVal >>= nVal;
        if(nVal >= 0 &&
           (nVal <= SVX_NUM_ARABIC ||
            SVX_NUM_CHARS_UPPER_LETTER_N == nVal ||
            SVX_NUM_CHARS_LOWER_LETTER_N == nVal ))
            aFmt.SetNumberingType(nVal);
        else
            bRet = false;
    }
    break;
    case MID_PREFIX      :
    {
        OUString sVal; rVal >>= sVal;
        sPrefix = sVal;
    }
    break;
    case MID_SUFFIX      :
    {
        OUString sVal; rVal >>= sVal;
        sSuffix = sVal;
    }
    break;
    default: bRet = false;
    }
    return bRet;
}


// class SwFmtFtnAtTxtEnd

SfxPoolItem* SwFmtFtnAtTxtEnd::Clone( SfxItemPool* /*pPool*/ ) const
{
    SwFmtFtnAtTxtEnd* pNew = new SwFmtFtnAtTxtEnd;
    *pNew = *this;
    return pNew;
}

// class SwFmtEndAtTxtEnd

SfxPoolItem* SwFmtEndAtTxtEnd::Clone( SfxItemPool* /*pPool*/ ) const
{
    SwFmtEndAtTxtEnd* pNew = new SwFmtEndAtTxtEnd;
    *pNew = *this;
    return pNew;
}

//class SwFmtChain


int SwFmtChain::operator==( const SfxPoolItem &rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );

    return GetPrev() == ((SwFmtChain&)rAttr).GetPrev() &&
        GetNext() == ((SwFmtChain&)rAttr).GetNext();
}

SwFmtChain::SwFmtChain( const SwFmtChain &rCpy ) :
    SfxPoolItem( RES_CHAIN )
{
    SetPrev( rCpy.GetPrev() );
    SetNext( rCpy.GetNext() );
}

SfxPoolItem* SwFmtChain::Clone( SfxItemPool* /*pPool*/ ) const
{
    SwFmtChain *pRet = new SwFmtChain;
    pRet->SetPrev( GetPrev() );
    pRet->SetNext( GetNext() );
    return pRet;
}

void SwFmtChain::SetPrev( SwFlyFrmFmt *pFmt )
{
    if ( pFmt )
        pFmt->Add( &aPrev );
    else if ( aPrev.GetRegisteredIn() )
        ((SwModify*)aPrev.GetRegisteredIn())->Remove( &aPrev );
}

void SwFmtChain::SetNext( SwFlyFrmFmt *pFmt )
{
    if ( pFmt )
        pFmt->Add( &aNext );
    else if ( aNext.GetRegisteredIn() )
        ((SwModify*)aNext.GetRegisteredIn())->Remove( &aNext );
}

bool SwFmtChain::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    bool   bRet = true;
    XubString aRet;
    switch ( nMemberId )
    {
    case MID_CHAIN_PREVNAME:
        if ( GetPrev() )
            aRet = GetPrev()->GetName();
        break;
    case MID_CHAIN_NEXTNAME:
        if ( GetNext() )
            aRet = GetNext()->GetName();
        break;
    default:
        OSL_ENSURE( !this, "unknown MemberId" );
        bRet = false;
    }
    rVal <<= OUString(aRet);
    return bRet;
}




//class SwFmtLineNumber

SwFmtLineNumber::SwFmtLineNumber() :
    SfxPoolItem( RES_LINENUMBER )
{
    nStartValue = 0;
    bCountLines = sal_True;
}

SwFmtLineNumber::~SwFmtLineNumber()
{
}

int SwFmtLineNumber::operator==( const SfxPoolItem &rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );

    return nStartValue  == ((SwFmtLineNumber&)rAttr).GetStartValue() &&
        bCountLines  == ((SwFmtLineNumber&)rAttr).IsCount();
}

SfxPoolItem* SwFmtLineNumber::Clone( SfxItemPool* /*pPool*/ ) const
{
    return new SwFmtLineNumber( *this );
}

bool SwFmtLineNumber::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch ( nMemberId )
    {
    case MID_LINENUMBER_COUNT:
    {
        BOOL bTmp = IsCount();
        rVal.setValue(&bTmp, ::getBooleanCppuType());
    }
    break;
    case MID_LINENUMBER_STARTVALUE:
        rVal <<= (sal_Int32)GetStartValue();
        break;
    default:
        OSL_ENSURE( !this, "unknown MemberId" );
        bRet = false;
    }
    return bRet;
}

bool SwFmtLineNumber::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    // hier wird immer konvertiert!
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch ( nMemberId )
    {
    case MID_LINENUMBER_COUNT:
        SetCountLines( *(sal_Bool*)rVal.getValue() );
        break;
    case MID_LINENUMBER_STARTVALUE:
    {
        sal_Int32 nVal(0);
        if(rVal >>= nVal)
            SetStartValue( nVal );
        else
            bRet = false;
    }
    break;
    default:
        OSL_ENSURE( !this, "unknown MemberId" );
        bRet = false;
    }
    return bRet;
}

/*************************************************************************
 *    class SwTextGridItem
 *************************************************************************/

SwTextGridItem::SwTextGridItem()
    : SfxPoolItem( RES_TEXTGRID ), aColor( COL_LIGHTGRAY ), nLines( 20 ),
      nBaseHeight( 400 ), nRubyHeight( 200 ), eGridType( GRID_NONE ),
      bRubyTextBelow( 0 ), bPrintGrid( 1 ), bDisplayGrid( 1 )
{
}

SwTextGridItem::~SwTextGridItem()
{
}

int SwTextGridItem::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return eGridType == ((SwTextGridItem&)rAttr).GetGridType() &&
        nLines == ((SwTextGridItem&)rAttr).GetLines() &&
        nBaseHeight == ((SwTextGridItem&)rAttr).GetBaseHeight() &&
        nRubyHeight == ((SwTextGridItem&)rAttr).GetRubyHeight() &&
        bRubyTextBelow == ((SwTextGridItem&)rAttr).GetRubyTextBelow() &&
        bDisplayGrid == ((SwTextGridItem&)rAttr).GetDisplayGrid() &&
        bPrintGrid == ((SwTextGridItem&)rAttr).GetPrintGrid() &&
        aColor == ((SwTextGridItem&)rAttr).GetColor();
}

SfxPoolItem* SwTextGridItem::Clone( SfxItemPool* /*pPool*/ ) const
{
    return new SwTextGridItem( *this );
}

SwTextGridItem& SwTextGridItem::operator=( const SwTextGridItem& rCpy )
{
    aColor = rCpy.GetColor();
    nLines = rCpy.GetLines();
    nBaseHeight = rCpy.GetBaseHeight();
    nRubyHeight = rCpy.GetRubyHeight();
    eGridType = rCpy.GetGridType();
    bRubyTextBelow = rCpy.GetRubyTextBelow();
    bPrintGrid = rCpy.GetPrintGrid();
    bDisplayGrid = rCpy.GetDisplayGrid();

    return *this;
}

bool SwTextGridItem::QueryValue( ::com::sun::star::uno::Any& rVal,
                                 BYTE nMemberId ) const
{
    bool bRet = true;

    switch( nMemberId & ~CONVERT_TWIPS )
    {
    case MID_GRID_COLOR:
        rVal <<= GetColor().GetColor();
        break;
    case MID_GRID_LINES:
        rVal <<= GetLines();
        break;
    case MID_GRID_RUBY_BELOW:
        rVal.setValue( &bRubyTextBelow, ::getBooleanCppuType() );
        break;
    case MID_GRID_PRINT:
        rVal.setValue( &bPrintGrid, ::getBooleanCppuType() );
        break;
    case MID_GRID_DISPLAY:
        rVal.setValue( &bDisplayGrid, ::getBooleanCppuType() );
        break;
    case MID_GRID_BASEHEIGHT:
        DBG_ASSERT( (nMemberId & CONVERT_TWIPS) != 0,
                    "This value needs TWIPS-MM100 conversion" );
        rVal <<= (sal_Int32)TWIP_TO_MM100_UNSIGNED(nBaseHeight);
        break;
    case MID_GRID_RUBYHEIGHT:
        DBG_ASSERT( (nMemberId & CONVERT_TWIPS) != 0,
                    "This value needs TWIPS-MM100 conversion" );
        rVal <<= (sal_Int32)TWIP_TO_MM100_UNSIGNED(nRubyHeight);
        break;
    case MID_GRID_TYPE:
        switch( GetGridType() )
        {
        case GRID_NONE:
            rVal <<= TextGridMode::NONE;
            break;
        case GRID_LINES_ONLY:
            rVal <<= TextGridMode::LINES;
            break;
        case GRID_LINES_CHARS:
            rVal <<= TextGridMode::LINES_AND_CHARS;
            break;
        default:
            OSL_FAIL("unknown SwTextGrid value");
            bRet = false;
            break;
        }
        break;
    default:
        OSL_FAIL("Unknown SwTextGridItem member");
        bRet = false;
        break;
    }

    return bRet;
}

bool SwTextGridItem::PutValue( const ::com::sun::star::uno::Any& rVal,
                               BYTE nMemberId )
{
    bool bRet = true;
    switch( nMemberId & ~CONVERT_TWIPS )
    {
    case MID_GRID_COLOR:
    {
        sal_Int32 nTmp(0);
        bRet = (rVal >>= nTmp);
        if( bRet )
            SetColor( Color(nTmp) );
    }
    break;
    case MID_GRID_LINES:
    {
        sal_Int16 nTmp(0);
        bRet = (rVal >>= nTmp);
        if( bRet && (nTmp >= 0) )
            SetLines( (sal_uInt16)nTmp );
        else
            bRet = false;
    }
    break;
    case MID_GRID_RUBY_BELOW:
        SetRubyTextBelow( *(sal_Bool*)rVal.getValue() );
        break;
    case MID_GRID_PRINT:
        SetPrintGrid( *(sal_Bool*)rVal.getValue() );
        break;
    case MID_GRID_DISPLAY:
        SetDisplayGrid( *(sal_Bool*)rVal.getValue() );
        break;
    case MID_GRID_BASEHEIGHT:
    case MID_GRID_RUBYHEIGHT:
    {
        DBG_ASSERT( (nMemberId & CONVERT_TWIPS) != 0,
                    "This value needs TWIPS-MM100 conversion" );
        sal_Int32 nTmp(0);
        bRet = (rVal >>= nTmp);
        nTmp = MM100_TO_TWIP( nTmp );
        if( bRet && (nTmp >= 0) && ( nTmp <= USHRT_MAX) )
            if( (nMemberId & ~CONVERT_TWIPS) == MID_GRID_BASEHEIGHT )
                SetBaseHeight( (USHORT)nTmp );
            else
                SetRubyHeight( (USHORT)nTmp );
        else
            bRet = false;
    }
    break;
    case MID_GRID_TYPE:
        {
            sal_Int16 nTmp(0);
            bRet = (rVal >>= nTmp);
            if( bRet )
            {
                switch( nTmp )
                {
                case TextGridMode::NONE:
                    SetGridType( GRID_NONE );
                    break;
                case TextGridMode::LINES:
                    SetGridType( GRID_LINES_ONLY );
                    break;
                case TextGridMode::LINES_AND_CHARS:
                    SetGridType( GRID_LINES_CHARS );
                    break;
                default:
                    bRet = false;
                    break;
                }
            }
        }
        break;
    default:
        OSL_FAIL("Unknown SwTextGridItem member");
        bRet = false;
    }

    return bRet;
}

// class SwHeaderAndFooterEatSpacingItem

SfxPoolItem* SwHeaderAndFooterEatSpacingItem::Clone( SfxItemPool* /*pPool*/ ) const
{
    return new SwHeaderAndFooterEatSpacingItem( Which(), GetValue() );
}


//  class SwFrmFmt
//  Implementierung teilweise inline im hxx

void SwFrmFmt::Modify( SfxPoolItem* pOld, SfxPoolItem* pNew )
{
    SwFmtHeader *pH = 0;
    SwFmtFooter *pF = 0;

    sal_uInt16 nWhich = pNew ? pNew->Which() : 0;

    if( RES_ATTRSET_CHG == nWhich )
    {
        ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState(
            RES_HEADER, sal_False, (const SfxPoolItem**)&pH );
        ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState(
            RES_FOOTER, sal_False, (const SfxPoolItem**)&pF );
    }
    else if( RES_HEADER == nWhich )
        pH = (SwFmtHeader*)pNew;
    else if( RES_FOOTER == nWhich )
        pF = (SwFmtFooter*)pNew;

    if( pH && pH->IsActive() && !pH->GetHeaderFmt() )
    {   //Hat er keinen, mach ich ihm einen
        SwFrmFmt *pFmt = GetDoc()->MakeLayoutFmt( RND_STD_HEADER );
        pFmt->Add( pH );
    }

    if( pF && pF->IsActive() && !pF->GetFooterFmt() )
    {   //Hat er keinen, mach ich ihm einen
        SwFrmFmt *pFmt = GetDoc()->MakeLayoutFmt( RND_STD_FOOTER );
        pFmt->Add( pF );
    }

    // MIB 24.3.98: Modify der Basisklasse muss immer gerufen werden, z.B.
    // wegen RESET_FMTWRITTEN.
    // if ( GetDepends() )
    SwFmt::Modify( pOld, pNew );
}

//Vernichtet alle Frms, die in aDepend angemeldet sind.

void SwFrmFmt::DelFrms()
{
    SwClientIter aIter( *this );
    SwClient * pLast = aIter.GoStart();
    if( pLast )
        do {
            if ( pLast->ISA(SwFrm) )
            {
                ((SwFrm*)pLast)->Cut();
                delete pLast;
            }
        } while( 0 != ( pLast = aIter++ ));
}

void SwFrmFmt::MakeFrms()
{
    OSL_ENSURE( !this, "Sorry not implemented." );
}

SwRect SwFrmFmt::FindLayoutRect( const sal_Bool bPrtArea, const Point* pPoint,
                                 const sal_Bool bCalcFrm ) const
{
    SwRect aRet;
    SwFrm *pFrm = 0;
    if( ISA( SwSectionFmt ) )
    {
        // dann den ::com::sun::star::frame::Frame per Node2Layout besorgen
        SwSectionNode* pSectNd = ((SwSectionFmt*)this)->GetSectionNode();
        if( pSectNd )
        {
            SwNode2Layout aTmp( *pSectNd, pSectNd->GetIndex() - 1 );
            pFrm = aTmp.NextFrm();

            if( pFrm && pFrm->GetRegisteredIn() != this )
            {
                // die Section hat keinen eigenen ::com::sun::star::frame::Frame, also falls
                // jemand die tatsaechliche Groe?e braucht, so muss das
                // noch implementier werden, in dem sich vom Ende noch
                // der entsprechende ::com::sun::star::frame::Frame besorgt wird.
                // PROBLEM: was passiert bei SectionFrames, die auf unter-
                //          schiedlichen Seiten stehen??
                if( bPrtArea )
                    aRet = pFrm->Prt();
                else
                {
                    aRet = pFrm->Frm();
                    --aRet.Pos().Y();
                }
                pFrm = 0;       // das Rect ist ja jetzt fertig
            }
        }
    }
    else
    {
        sal_uInt16 nFrmType = RES_FLYFRMFMT == Which() ? FRM_FLY : USHRT_MAX;
        pFrm = ::binfilter::GetFrmOfModify( *(SwModify*)this, nFrmType, pPoint,
                                            0, bCalcFrm );
    }

    if( pFrm )
    {
        if( bPrtArea )
            aRet = pFrm->Prt();
        else
            aRet = pFrm->Frm();
    }
    return aRet;
}

SwContact* SwFrmFmt::FindContactObj()
{
    SwClientIter aIter( *this );
    return (SwContact*)aIter.First( TYPE( SwContact ) );
}

SdrObject* SwFrmFmt::FindSdrObject()
{
    SwClientIter aIter( *this );
    SwClient* pFnd = aIter.First( TYPE( SwContact ) );
    return pFnd ? ((SwContact*)pFnd)->GetMaster() : 0;
}

SdrObject* SwFrmFmt::FindRealSdrObject()
{
    if( RES_FLYFRMFMT == Which() )
    {
        Point aNullPt;
        SwFlyFrm* pFly = (SwFlyFrm*)::binfilter::GetFrmOfModify( *this, FRM_FLY,
                                                                 &aNullPt, 0, sal_False );
        return pFly ? pFly->GetVirtDrawObj() : 0;
    }
    return FindSdrObject();
}


//  class SwFlyFrmFmt
//  Implementierung teilweise inline im hxx

SwFlyFrmFmt::~SwFlyFrmFmt()
{
    SwClientIter aIter( *this );
    SwClient * pLast = aIter.GoStart();
    if( pLast )
        do {
            if ( pLast->ISA( SwFlyFrm ) )
                delete pLast;

        } while( 0 != ( pLast = aIter++ ));

    pLast = aIter.GoStart();
    if( pLast )
        do {
            if ( pLast->ISA( SwFlyDrawContact ) )
                delete pLast;

        } while( 0 != ( pLast = aIter++ ));
}

void SwFlyFrmFmt::MakeFrms()
{
    return;
}

SwFlyFrm* SwFlyFrmFmt::GetFrm( const Point* pPoint, const sal_Bool bCalcFrm ) const
{
    return (SwFlyFrm*)::binfilter::GetFrmOfModify( *(SwModify*)this, FRM_FLY,
                                                   pPoint, 0, bCalcFrm );
}

sal_Bool SwFlyFrmFmt::GetInfo( SfxPoolItem& rInfo ) const
{
    switch( rInfo.Which() )
    {
    case RES_CONTENT_VISIBLE:
    {
        ((SwPtrMsgPoolItem&)rInfo).pObject =
            SwClientIter( *(SwFlyFrmFmt*)this ).First( TYPE(SwFrm) );
    }
    return sal_False;

    default:
        return SwFrmFmt::GetInfo( rInfo );
    }
    return sal_True;
}

/** SwFlyFrmFmt::IsBackgroundTransparent

    overloading virtual method and its default implementation,
    because format of fly frame provides transparent backgrounds.
    Method determines, if background of fly frame is transparent.

    @author OD

    @return true, if background color is transparent, but not "no fill"
    or the transparency of a existing background graphic is set.
*/
sal_Bool SwFlyFrmFmt::IsBackgroundTransparent() const
{
    sal_Bool bReturn = sal_False;

    /// NOTE: If background color is "no fill"/"auto fill" (COL_TRANSPARENT)
    ///     and there is no background graphic, it "inherites" the background
    ///     from its anchor.
    if ( (GetBackground().GetColor().GetTransparency() != 0) &&
         (GetBackground().GetColor() != COL_TRANSPARENT)
        )
    {
        bReturn = sal_True;
    }
    else
    {
        const BfGraphicObject* pTmpGrf =
            static_cast<const BfGraphicObject*>(GetBackground().GetGraphicObject());
        if ( (pTmpGrf) &&
             (pTmpGrf->GetAttr().GetTransparency() != 0)
            )
        {
            bReturn = sal_True;
        }
    }

    return bReturn;
}

/** SwFlyFrmFmt::IsBackgroundBrushInherited

    method to determine, if the brush for drawing the
    background is "inherited" from its parent/grandparent.
    This is the case, if no background graphic is set and the background
    color is "no fill"/"auto fill"
    NOTE: condition is "copied" from method <SwFrm::GetBackgroundBrush(..).

    @author OD

    @return true, if background brush is "inherited" from parent/grandparent
*/


//  class SwDrawFrmFmt
//  Implementierung teilweise inline im hxx

#ifdef _MSC_VER
#pragma optimize( "e", off )
#endif

SwDrawFrmFmt::~SwDrawFrmFmt()
{
    SwContact *pContact = FindContactObj();
    delete pContact;
}

#ifdef _MSC_VER
#pragma optimize( "e", on )
#endif

void SwDrawFrmFmt::MakeFrms()
{
    SwDrawContact *pContact = (SwDrawContact*)FindContactObj();
    if ( pContact )
        pContact->ConnectToLayout();
}

void SwDrawFrmFmt::DelFrms()
{
    SwDrawContact *pContact = (SwDrawContact *)FindContactObj();
    if ( pContact ) //fuer den Reader und andere Unabwaegbarkeiten.
        pContact->DisconnectFromLayout();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
