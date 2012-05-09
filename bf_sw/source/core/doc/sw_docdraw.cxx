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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <hintids.hxx>

#include <rtl/logfile.hxx>
#include <bf_sfx2/printer.hxx>
#include <bf_svx/eeitem.hxx>

#include <bf_svx/itemdata.hxx>
#include <bf_svtools/poolitem.hxx>

#ifndef _SVX_FLDITEM_HXX //autogen
#define ITEMID_FIELD EE_FEATURE_FIELD
#endif
#include <bf_svx/editeng.hxx>
#include <bf_svx/svdoutl.hxx>
#include <bf_svx/svdpage.hxx>


#include <bf_svx/svdpool.hxx>

#include <frmfmt.hxx>
#include <swhints.hxx>

#include <horiornt.hxx>

#include <doc.hxx>
#include <docsh.hxx>
#include <rootfrm.hxx>  //Damit der RootDtor gerufen wird.
#include <viewsh.hxx>           // fuer MakeDrawView
#include <drawdoc.hxx>
#include <dcontact.hxx>
#include <dview.hxx>
#include <mvsave.hxx>
#include <flyfrm.hxx>
#include <dflyobj.hxx>

#include <bf_svx/fhgtitem.hxx>

#include <bf_svx/svdpagv.hxx>
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::linguistic2;


/*N*/ SV_IMPL_VARARR_SORT( _ZSortFlys, _ZSortFly )

/*N*/ _ZSortFly::_ZSortFly( const SwFrmFmt* pFrmFmt, const SwFmtAnchor* pFlyAn,
/*N*/                     UINT32 nArrOrdNum )
/*N*/   : pFmt( pFrmFmt ), pAnchor( pFlyAn ), nOrdNum( nArrOrdNum )
/*N*/ {
/*N*/       // #i11176#
/*N*/   // This also needs to work when no layout exists. Thus, for
/*N*/   // FlyFrames an alternative method is used now in that case.
/*N*/   SwClientIter aIter( (SwFmt&)*pFmt );
/*N*/
/*N*/   if( RES_FLYFRMFMT == pFmt->Which() )
/*N*/   {
/*N*/       // Schauen, ob es ein SdrObject dafuer gibt
/*N*/       if( aIter.First( TYPE(SwFlyDrawContact) ) )
/*N*/           nOrdNum = ((SwFlyDrawContact*)aIter())->GetMaster()->GetOrdNum();
/*N*/   }
/*N*/   else if( RES_DRAWFRMFMT == pFmt->Which() )
/*N*/   {
/*N*/           // Schauen, ob es ein SdrObject dafuer gibt
/*N*/           if( aIter.First( TYPE(SwDrawContact) ) )
/*N*/               nOrdNum = ((SwDrawContact*)aIter())->GetMaster()->GetOrdNum();
/*N*/   }
/*N*/   else
        {
/*N*/           OSL_ENSURE( !this, "was ist das fuer ein Format?" );
        }
/*N*/ }
} //namespace binfilter

/*************************************************************************/
// Wird auch vom Sw3-Reader gerufen, wenn ein Fehler beim Einlesen
// des Drawing Layers auftrat. In diesem Fall wird der Layer komplett
// neu aufgebaut.

// #75371#
#include <bf_svx/sxenditm.hxx>

namespace binfilter {
/*N*/ void SwDoc::InitDrawModel()
/*N*/ {
/*N*/   RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDoc::InitDrawModel" );
/*N*/
/*N*/   //!!Achtung im sw3-Reader (sw3imp.cxx) gibt es aehnlichen Code, der
/*N*/   //mitgepfelgt werden muss.
/*N*/   if ( pDrawModel )
/*?*/       ReleaseDrawModel();
/*N*/
/*N*/   //DrawPool und EditEnginePool anlegen, diese gehoeren uns und werden
/*N*/   //dem Drawing nur mitgegeben. Im ReleaseDrawModel werden die Pools wieder
/*N*/   //zerstoert.
/*N*/   // 17.2.99: for Bug 73110 - for loading the drawing items. This must
/*N*/   //                          be loaded without RefCounts!
/*N*/   SfxItemPool *pSdrPool = new SdrItemPool( &aAttrPool, SDRATTR_START,
/*N*/                                           SDRATTR_END, FALSE );
/*N*/   // #75371# change DefaultItems for the SdrEdgeObj distance items
/*N*/   // to TWIPS.
/*N*/   if(pSdrPool)
/*N*/   {
/*N*/       const long nDefEdgeDist = ((500 * 72) / 127); // 1/100th mm in twips
/*N*/       pSdrPool->SetPoolDefaultItem(SdrEdgeNode1HorzDistItem(nDefEdgeDist));
/*N*/       pSdrPool->SetPoolDefaultItem(SdrEdgeNode1VertDistItem(nDefEdgeDist));
/*N*/       pSdrPool->SetPoolDefaultItem(SdrEdgeNode2HorzDistItem(nDefEdgeDist));
/*N*/       pSdrPool->SetPoolDefaultItem(SdrEdgeNode2VertDistItem(nDefEdgeDist));
/*N*/   }
/*N*/   SfxItemPool *pEEgPool = EditEngine::CreatePool( FALSE );
/*N*/   pSdrPool->SetSecondaryPool( pEEgPool );
/*N*/   if ( !aAttrPool.GetFrozenIdRanges () )
/*N*/       aAttrPool.FreezeIdRanges();
/*N*/   else
/*?*/       pSdrPool->FreezeIdRanges();
/*N*/
/*N*/     // set FontHeight pool defaults without changing static SdrEngineDefaults
/*N*/   aAttrPool.SetPoolDefaultItem(SvxFontHeightItem( 240, 100, EE_CHAR_FONTHEIGHT ));
/*N*/
/*N*/   RTL_LOGFILE_CONTEXT_TRACE( aLog, "before create DrawDocument" );
/*N*/   //Das SdrModel gehoert dem Dokument, wir haben immer zwei Layer und eine
/*N*/   //Seite.
/*N*/   pDrawModel = new SwDrawDocument( this );
/*N*/
/*N*/   String sLayerNm;
/*N*/   sLayerNm.AssignAscii(RTL_CONSTASCII_STRINGPARAM("Hell" ));
/*N*/   nHell   = pDrawModel->GetLayerAdmin().NewLayer( sLayerNm )->GetID();
/*N*/
/*N*/   sLayerNm.AssignAscii(RTL_CONSTASCII_STRINGPARAM("Heaven" ));
/*N*/   nHeaven = pDrawModel->GetLayerAdmin().NewLayer( sLayerNm )->GetID();
/*N*/
/*N*/   sLayerNm.AssignAscii(RTL_CONSTASCII_STRINGPARAM("Controls" ));
/*N*/   nControls = pDrawModel->GetLayerAdmin().NewLayer( sLayerNm )->GetID();
/*N*/
/*N*/     // add invisible layers corresponding to the visible ones.
/*N*/     {
/*N*/         sLayerNm.AssignAscii(RTL_CONSTASCII_STRINGPARAM("InvisibleHell" ));
/*N*/         nInvisibleHell   = pDrawModel->GetLayerAdmin().NewLayer( sLayerNm )->GetID();
/*N*/
/*N*/         sLayerNm.AssignAscii(RTL_CONSTASCII_STRINGPARAM("InvisibleHeaven" ));
/*N*/         nInvisibleHeaven = pDrawModel->GetLayerAdmin().NewLayer( sLayerNm )->GetID();
/*N*/
/*N*/         sLayerNm.AssignAscii(RTL_CONSTASCII_STRINGPARAM("InvisibleControls" ));
/*N*/         nInvisibleControls = pDrawModel->GetLayerAdmin().NewLayer( sLayerNm )->GetID();
/*N*/     }
/*N*/
/*N*/   pDrawModel->InsertPage( pDrawModel->AllocPage( FALSE ) );
/*N*/   RTL_LOGFILE_CONTEXT_TRACE( aLog, "after create DrawDocument" );
/*N*/
/*N*/   SdrOutliner& rOutliner = pDrawModel->GetDrawOutliner();
/*N*/   RTL_LOGFILE_CONTEXT_TRACE( aLog, "after create Spellchecker/Hyphenator" );
/*N*/
/*N*/   SetCalcFieldValueHdl(&rOutliner);
/*N*/   SetCalcFieldValueHdl(&pDrawModel->GetHitTestOutliner());
/*N*/
/*N*/   //JP 16.07.98: Bug 50193 - Linkmanager am Model setzen, damit
/*N*/   //          dort ggfs. verlinkte Grafiken eingefuegt werden koennen
/*N*/   //JP 28.01.99: der WinWord Import benoetigt ihn auch
/*N*/   pDrawModel->SetLinkManager( &GetLinkManager() );
/*N*/
/*N*/       OutputDevice* pRefDev = _GetRefDev();
/*N*/     if ( pRefDev )
/*N*/         pDrawModel->SetRefDevice( pRefDev );
/*N*/ }

/** method to determine, if a layer ID belongs to the visible ones.
    Note: If given layer ID is unknown, method asserts and returns <false>.

    @author OD
*/
bool SwDoc::IsVisibleLayerId( const SdrLayerID& _nLayerId )
{
    bool bRetVal;

    if ( _nLayerId == GetHeavenId() ||
         _nLayerId == GetHellId() ||
         _nLayerId == GetControlsId() )
    {
        bRetVal = true;
    }
    else if ( _nLayerId == GetInvisibleHeavenId() ||
              _nLayerId == GetInvisibleHellId() ||
              _nLayerId == GetInvisibleControlsId() )
    {
        bRetVal = false;
    }
    else
    {
        OSL_FAIL( "<SwDoc::IsVisibleLayerId(..)> - unknown layer ID." );
        bRetVal = false;
    }

    return bRetVal;
}

/** method to determine, if the corresponding visible layer ID for a invisible one.
    Note: If given layer ID is a visible one, method returns given layer ID.
    Note: If given layer ID is unknown, method returns given layer ID.

    @author OD
*/
SdrLayerID SwDoc::GetVisibleLayerIdByInvisibleOne( const SdrLayerID& _nInvisibleLayerId )
{
    SdrLayerID nVisibleLayerId;

    if ( _nInvisibleLayerId == GetInvisibleHeavenId() )
    {
        nVisibleLayerId = GetHeavenId();
    }
    else if ( _nInvisibleLayerId == GetInvisibleHellId() )
    {
        nVisibleLayerId = GetHellId();
    }
    else if ( _nInvisibleLayerId == GetInvisibleControlsId() )
    {
        nVisibleLayerId = GetControlsId();
    }
    else if ( _nInvisibleLayerId == GetHeavenId() ||
              _nInvisibleLayerId == GetHellId() ||
              _nInvisibleLayerId == GetControlsId() )
    {
        OSL_FAIL( "<SwDoc::GetVisibleLayerIdByInvisibleOne(..)> - given layer ID already an invisible one." );
        nVisibleLayerId = _nInvisibleLayerId;
    }
    else
    {
        OSL_FAIL( "<SwDoc::GetVisibleLayerIdByInvisibleOne(..)> - given layer ID is unknown." );
        nVisibleLayerId = _nInvisibleLayerId;
    }

    return nVisibleLayerId;
}

/** method to determine, if the corresponding invisible layer ID for a visible one.
    Note: If given layer ID is a invisible one, method returns given layer ID.
    Note: If given layer ID is unknown, method returns given layer ID.

    @author OD
*/
SdrLayerID SwDoc::GetInvisibleLayerIdByVisibleOne( const SdrLayerID& _nVisibleLayerId )
{
    SdrLayerID nInvisibleLayerId;

    if ( _nVisibleLayerId == GetHeavenId() )
    {
        nInvisibleLayerId = GetInvisibleHeavenId();
    }
    else if ( _nVisibleLayerId == GetHellId() )
    {
        nInvisibleLayerId = GetInvisibleHellId();
    }
    else if ( _nVisibleLayerId == GetControlsId() )
    {
        nInvisibleLayerId = GetInvisibleControlsId();
    }
    else if ( _nVisibleLayerId == GetInvisibleHeavenId() ||
              _nVisibleLayerId == GetInvisibleHellId() ||
              _nVisibleLayerId == GetInvisibleControlsId() )
    {
        OSL_FAIL( "<SwDoc::GetInvisibleLayerIdByVisibleOne(..)> - given layer ID already an invisible one." );
        nInvisibleLayerId = _nVisibleLayerId;
    }
    else
    {
        OSL_FAIL( "<SwDoc::GetInvisibleLayerIdByVisibleOne(..)> - given layer ID is unknown." );
        nInvisibleLayerId = _nVisibleLayerId;
    }

    return nInvisibleLayerId;
}

/*************************************************************************/


/*N*/ void SwDoc::ReleaseDrawModel()
/*N*/ {
/*N*/   if ( pDrawModel )
/*N*/   {
/*N*/       //!!Den code im sw3io fuer Einfuegen Dokument mitpflegen!!
/*N*/
/*N*/       delete pDrawModel; pDrawModel = 0;
/*N*/       SfxItemPool *pSdrPool = aAttrPool.GetSecondaryPool();
/*N*/
/*N*/       OSL_ENSURE( pSdrPool, "missing Pool" );
/*N*/       SfxItemPool *pEEgPool = pSdrPool->GetSecondaryPool();
/*N*/       OSL_ENSURE( !pEEgPool->GetSecondaryPool(), "i don't accept additional pools");
/*N*/       pSdrPool->Delete();                 //Erst die Items vernichten lassen,
/*N*/                                           //dann erst die Verkettung loesen
/*N*/       aAttrPool.SetSecondaryPool( 0 );    //Der ist ein muss!
/*N*/       pSdrPool->SetSecondaryPool( 0 );    //Der ist sicherer
/*N*/       delete pSdrPool;
/*N*/       delete pEEgPool;
/*N*/   }
/*N*/ }

/*************************************************************************/


/*N*/ SdrModel* SwDoc::_MakeDrawModel()
/*N*/ {
/*N*/   OSL_ENSURE( !pDrawModel, "_MakeDrawModel: Why?" );
/*N*/   InitDrawModel();
/*N*/   return pDrawModel;
/*N*/ }


/*************************************************************************
|*
|* Am Outliner Link auf Methode fuer Felddarstellung in Editobjekten setzen
|*
\*************************************************************************/

/*N*/ void SwDoc::SetCalcFieldValueHdl(Outliner* pOutliner)
/*N*/ {
/*N*/   pOutliner->SetCalcFieldValueHdl(LINK(this, SwDoc, CalcFieldValueHdl));
/*N*/ }

/*************************************************************************
|*
|* Felder bzw URLs im Outliner erkennen und Darstellung festlegen
|*
\************************************************************************/

IMPL_LINK(SwDoc, CalcFieldValueHdl, EditFieldInfo*, /*pInfo*/)
{
    DBG_BF_ASSERT(0, "STRIP");  // IMPL_LINK()
    return(0);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
