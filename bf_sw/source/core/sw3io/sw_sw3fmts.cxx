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
#include <bf_svtools/itemiter.hxx>
#include <bf_svx/svdmodel.hxx>
#include <bf_svx/svdpage.hxx>

#include <fmtfsize.hxx>

#include <horiornt.hxx>

#include <fmtornt.hxx>
#include <fmtcntnt.hxx>
#include <fmtsrnd.hxx>
#include <fmtanchr.hxx>
#include <fmtcnct.hxx>
#include <frmatr.hxx>
#include <swtblfmt.hxx>

#include <fmtflcnt.hxx>

#include <osl/diagnose.h>

#include <ndtxt.hxx>
#include "swerror.h"
#include "doc.hxx"
#include "pagefrm.hxx"
#include "cntfrm.hxx"
#include "pam.hxx"
#include "section.hxx"
#include "sw3imp.hxx"
#include "poolfmt.hxx"
#include "dcontact.hxx"
#include "dflyobj.hxx"
#include "flyfrm.hxx"
#include "hints.hxx"
#include "ndnotxt.hxx"
namespace binfilter {

//////////////////////////////////////////////////////////////////////////////

// Finden eines AttrSets nach Index

SwFmt* lcl_sw3io__GetUserPoolFmt( USHORT nId, const SvPtrarr* pFmtArr )
{
    SwFmt* pFmt;
    for( USHORT n = 0; n < pFmtArr->Count(); ++n )
        if( nId == (pFmt = (SwFmt*)(*pFmtArr)[n])->GetPoolFmtId() )
            return pFmt;
    return 0;
}

// Umwandeln eines gespreizten Whichwertes in den normalen Wert

/*N*/ USHORT lcl_sw3io__CompressWhich( USHORT nWhich, USHORT nVersion )
/*N*/ {
/*N*/   if( nWhich >= 0x6000 )
/*N*/   {
/*N*/       // #95500#: Files written with the SWG_NEWGRFATTR version have the
/*N*/       // extended grapic attribute range, but didn't correct it while
/*N*/       // saving: File before that version didn't have the extended graphic
/*N*/       // attribute range and need a correction. Files written with later
/*N*/       // version did correct the range while saving and therfor need a
/*N*/       // correction while loading, too.that version didn't have the extended graphic
/*N*/       // attribute range and need a correction. Files written with later
/*N*/       // version did correct the range while saving and therfor need a
/*N*/       // correction while loading, too.
/*N*/
/*N*/       (nWhich -= 0x6000 ) += RES_GRFATR_BEGIN;
/*N*/       if( SWG_NEWGRFATTR != nVersion && nWhich > RES_GRFATR_CROPGRF )
/*N*/           nWhich += RES_GRFATR_END - RES_GRFATR_CROPGRF - 1;
/*N*/       return nWhich;
/*N*/   }
/*N*/   if( nWhich >= 0x5000 )
/*N*/       return nWhich - 0x5000 + RES_FRMATR_BEGIN;
/*N*/   if( nWhich >= 0x4000 )
/*N*/       return nWhich - 0x4000 + RES_PARATR_BEGIN;
/*N*/   if( nWhich >= 0x3000 )
/*N*/       return nWhich - 0x3000 + RES_TXTATR_NOEND_BEGIN;
/*N*/   if( nWhich >= 0x2000 )
/*N*/       return nWhich - 0x2000 + RES_TXTATR_WITHEND_BEGIN;
/*N*/   if( nWhich >= 0x1000 )
/*N*/       return nWhich - 0x1000 + RES_CHRATR_BEGIN;
/*N*/   // Alte Dokumente vor dem 21.04.95
/*N*/   return nWhich;
/*N*/ }

sal_Bool lcl_sw3io_insFtn( const SwTxtNode *pTxtNd )
{
    OSL_ENSURE( pTxtNd, "There is the text node?" );
    if( !pTxtNd )
        return FALSE;

    const SwStartNode *pSttNd = pTxtNd->FindStartNode();
    while( pSttNd &&
           (pSttNd->IsTableNode() || pSttNd->IsSectionNode() ||
            SwTableBoxStartNode == pSttNd->GetStartNodeType() ) )
        pSttNd = pSttNd->FindStartNode();
    return !pSttNd || SwNormalStartNode == pSttNd->GetStartNodeType();
}

//////////////////////////////////////////////////////////////////////////////

// Einlesen eines Attributs
// Falls Bgn und End nicht besetzt sind, wird STRING_LEN zurueckgeliefert.
// Fall nur Bgn besetzt ist, ist End == Bgn

// BYTE     Flags
//          0x10 - Beginn ist gueltig
//          0x20 - Ende ist gueltig
// UINT16   Which
// UINT16   Attributversion
// UINT16   Beginn (opt)
// UINT16   Ende (opt)
// Attribut

/*N*/ SfxPoolItem* Sw3IoImp::InAttr( xub_StrLen& nBgn, xub_StrLen& nEnd,
/*N*/                              const SwTxtNode *pTxtNd )
/*N*/ {
/*N*/   nBgn = nEnd = STRING_LEN;
/*N*/   OpenRec( SWG_ATTRIBUTE );
/*N*/   BYTE cFlags = OpenFlagRec();
/*N*/   UINT16 nWhich, nVer;
/*N*/   *pStrm >> nWhich >> nVer;
/*N*/   nWhich = lcl_sw3io__CompressWhich( nWhich, nVersion );
/*N*/   if( cFlags & 0x10 )
/*N*/   {
/*N*/       USHORT nBgn16;
/*N*/       *pStrm >> nBgn16;
/*N*/       nBgn = nBgn16;
/*N*/   }
/*N*/   if( cFlags & 0x20 )
/*N*/   {
/*N*/       USHORT nEnd16;
/*N*/       *pStrm >> nEnd16;
/*N*/       nEnd = nEnd16;
/*N*/   }
/*N*/   else
/*N*/       nEnd = nBgn;
/*N*/   CloseFlagRec();
/*N*/
/*N*/   SfxPoolItem* pItem = 0;
/*N*/   // MIB 1.9.97: In der 3.0 geschriebene Crop-Attribute sind inkompatibel,
/*N*/   // also ignorieren wir sie.
/*N*/   if( POOLATTR_BEGIN <= nWhich && nWhich < POOLATTR_END &&
/*N*/       ( nWhich!=RES_GRFATR_CROPGRF || IsVersion(SWG_NONUMLEVEL) )  )
/*N*/   {
/*N*/       if( nWhich != RES_TXTATR_FTN ||
/*N*/           (nFlyLevel == 0 && (!bInsert || lcl_sw3io_insFtn(pTxtNd)) ) )
/*N*/       {
/*N*/           const SfxPoolItem& rDflt = pDoc->GetAttrPool().GetDefaultItem( nWhich );
/*N*/           pItem = rDflt.Create( *pStrm, nVer );
/*N*/
/*N*/           // Don't insert charset color items that have the system charset
/*N*/ //            if( RES_CHRATR_CHARSETCOLOR == nWhich &&
/*N*/ //                ((SvxCharSetColorItem *)pItem)->GetCharSet() == eSysSet )
/*N*/ //            {
/*N*/ //                delete pItem;
/*N*/ //                pItem = 0;
/*N*/ //            }
/*N*/       }
/*N*/       else
/*N*/       {
/*N*/           OSL_ENSURE( nFlyLevel == 0, "Fussnoten im Fly sind nicht erlaubt" );
/*N*/       }
/*N*/   }
/*N*/   CloseRec( SWG_ATTRIBUTE );
/*N*/   return pItem;
/*N*/ }

// Einlesen eines AttrSets

/*N*/ void Sw3IoImp::InAttrSet( SwAttrSet& rSet )
/*N*/ {
/*N*/   OpenRec( SWG_ATTRSET );
/*N*/   while( BytesLeft() )
/*N*/   {
/*N*/       xub_StrLen n1, n2;
/*N*/       SfxPoolItem* pItem = InAttr( n1, n2 );
/*N*/       if( pItem )
/*N*/           rSet.Put( *pItem ), delete pItem;
/*N*/   }
/*N*/   CloseRec( SWG_ATTRSET );
/*N*/ }

// Ein Format einlesen
// Hier wird das Format auch erzeugt, falls es nicht uebergeben wurde.

/*N*/ SwFmt* Sw3IoImp::InFormat( BYTE cKind, SwFmt* pFmt )
/*N*/ {
/*N*/   if( !OpenRec( cKind ) )
/*N*/   {
/*N*/       CloseRec( SWG_EOF );
/*N*/       return NULL;
/*N*/   }
/*N*/
/*N*/   // Ist der Drawing Layer korrekt geladen?
/*N*/   if( cKind == SWG_SDRFMT && ( nGblFlags & SW3F_NODRAWING ) )
/*N*/   {
/*?*/       Warning();
/*?*/       CloseRec( cKind );
/*?*/       return NULL;
/*N*/   }
/*N*/   if( cKind == SWG_FLYFMT || cKind == SWG_SDRFMT ) nFlyLevel++;
/*N*/   BYTE cFlags = OpenFlagRec();
/*N*/   BYTE cMoreFlags = 0;
/*N*/   USHORT nDerived, nPoolId, nStrIdx = IDX_NO_VALUE;
/*N*/   SdrObject* pSdrObj = NULL;
/*N*/   *pStrm >> nDerived >> nPoolId;
/*N*/   // 0x0L - Laenge der Daten
/*N*/   // 0x10 - Namensindex des Formats folgt
/*N*/   // 0x20 - SdrObject-Referenz folgt
/*N*/   if( cFlags & 0x10 )
/*N*/       *pStrm >> nStrIdx;
/*N*/   if( cFlags & 0x20 )
/*N*/   {
/*N*/       // Sdr-Objekt aus SdrModel auslesen
/*N*/       UINT32 nObjRef;
/*N*/       *pStrm >> nObjRef;
/*N*/       nObjRef += nZOrderOff;
/*N*/       SdrPage* pPg = pDoc->MakeDrawModel()->GetPage( 0 );
/*N*/       switch( cKind )
/*N*/       {
/*N*/           case SWG_FLYFMT:
/*N*/               if( ( nVersion < SWG_ZORDER )
/*N*/                || ( nGblFlags & SW3F_NODRAWING ) )
/*N*/               {
/*N*/                   pSdrObj = new SwFlyDrawObj;
/*N*/                   pPg->InsertObject( pSdrObj, nObjRef );
/*N*/                   break;
/*N*/               }
/*N*/               // sonst kann durchgerauscht werden, da
/*N*/               // LoadDrawingLayer() die Objekte schon angelegt hat
/*N*/           case SWG_SDRFMT:
/*N*/               if( nObjRef >= pPg->GetObjCount() )
/*N*/               {
/*?*/                   OSL_ENSURE( !this, "Ungueltige SdrObject-Nummer" );
/*?*/                   nObjRef = 0;
/*?*/                   Error();
/*N*/               }
/*N*/               pSdrObj = pPg->GetObj( nObjRef );
/*N*/               // All sdr object have to be in the hidden layer initially.
/*N*/               // They will be moved in the visible one when the layout
/*N*/               // will be created.
/*N*/               if( SWG_SDRFMT==cKind )
/*N*/                   pSdrObj->NbcSetLayer( pDoc->GetInvisibleLayerIdByVisibleOne( pSdrObj->GetLayer() ) );
/*N*/               break;
/*N*/       }
/*N*/   }
/*N*/
/*N*/   BOOL bMoreFlags;
/*N*/   if( IsVersion(SWG_LONGIDX) )
/*N*/       bMoreFlags = (cFlags & 0x80) != 0;
/*N*/   else
/*N*/       bMoreFlags = (cFlags & 0x40) != 0;
/*N*/
/*N*/   if( bMoreFlags )
/*N*/       *pStrm >> cMoreFlags;
/*N*/   CloseFlagRec();
/*N*/
/*N*/   BOOL bReadName;
/*N*/   if( IsVersion(SWG_LONGIDX) )
/*N*/       bReadName = (cMoreFlags & 0x20) != 0;
/*N*/   else
/*N*/       bReadName = nStrIdx == IDX_NO_VALUE;
/*N*/
/*N*/   String aName;
/*N*/   if( bReadName )
/*N*/   {
/*N*/       InString( *pStrm, aName );
/*N*/   }
/*N*/   else
/*N*/   {
/*N*/       aName = aStringPool.Find( nStrIdx );
/*N*/   }
/*N*/
/*N*/   // Line und Box-Formate habe keinen Namen mehr.
/*N*/   if( !IsVersion(SWG_LONGIDX) && pFmt &&
/*N*/       ( pFmt->IsA(TYPE(SwTableLineFmt)) || pFmt->IsA(TYPE(SwTableBoxFmt)) ) )
/*N*/   {
/*N*/       aName = aEmptyStr;
/*N*/   }
/*N*/
/*N*/   // Format erzeugen
/*N*/   if( !pFmt )
/*N*/   {
/*N*/       switch( cKind )
/*N*/       {
/*N*/           case SWG_FREEFMT:
/*N*/               // Freifliegende Frameformate werden von Headers
/*N*/               // und Footers verwendet.
/*N*/               pFmt = new SwFrmFmt( pDoc->GetAttrPool(), aName,
/*N*/                                    pDoc->GetDfltFrmFmt() );
/*N*/               break;
/*?*/           case SWG_FRAMEFMT:
/*?*/               if( IDX_NO_VALUE != nPoolId )
/*?*/               {
/*?*/                   if( IsPoolUserFmt( nPoolId ) )
/*?*/                   {
/*?*/                       // wir suchen uns das richtige Format
                            if( 0 == ( pFmt = lcl_sw3io__GetUserPoolFmt( nPoolId,
/*?*/                                               pDoc->GetFrmFmts() )) )
/*?*/                           pFmt = lcl_sw3io__GetUserPoolFmt( nPoolId,
/*?*/                                               pDoc->GetSpzFrmFmts() );
/*?*/                       OSL_ENSURE( pFmt, "Format not found." );
/*?*/                   }
/*?*/                   else
/*?*/                       pFmt = pDoc->GetFrmFmtFromPool( nPoolId );
/*?*/               }
/*?*/               if( !pFmt )
/*?*/                   pFmt = pDoc->MakeFrmFmt( aName,
/*?*/                       (SwFrmFmt*) FindFmt( nDerived, cKind ) );
/*?*/               else
/*?*/                   pFmt->ResetAllAttr();
/*?*/
/*?*/               break;
/*N*/           case SWG_FLYFMT:
/*N*/               if( pSdrObj && pSdrObj->GetUserCall() )
/*N*/               {
/*N*/                   //s.u., und auch bugdoc 28336.sdw beim schliessen nach laden.
/*?*/                   OSL_ENSURE( !this, "More than one Format" );
/*?*/                   CloseRec( cKind );
/*?*/                   nFlyLevel--;
/*?*/                   return NULL;
/*N*/               }
/*N*/               pFmt = pDoc->MakeFlyFrmFmt( aName,
/*N*/                       (SwFlyFrmFmt*) FindFmt( nDerived, cKind ) );
/*N*/               if ( pSdrObj )
/*N*/                   new SwFlyDrawContact( (SwFrmFmt*) pFmt, pSdrObj );
/*N*/               break;
/*N*/           case SWG_SDRFMT:
/*N*/               if ( pSdrObj && (pSdrObj->ISA(SwFlyDrawObj) ||
/*N*/                                pSdrObj->GetUserCall()) )
/*N*/               {
/*N*/                   // Pruefen, ob das Draw-Objekt nicht etwa zu einem
/*N*/                   // Fly-Frame-Format gehoert oder schon verwendet wurde.
/*N*/                   // Kann passieren, weil sich manche Draw-Objekte selber
/*N*/                   // zerstoeren, ohne dass das Format geloescht wird. Leider
/*N*/                   // wurden solche Format bisher auch rausgeschreiben.
/*N*/                   // (bug fix #25354# fuer alte Doks)
/*?*/                   CloseRec( cKind );
/*?*/                   nFlyLevel--;
/*?*/                   return NULL;
/*N*/               }
/*N*/               pFmt = pDoc->MakeDrawFrmFmt( aName,
/*N*/                       (SwFrmFmt*) FindFmt( nDerived, cKind ) );
/*N*/               if( pSdrObj )
/*N*/                   new SwDrawContact( (SwFrmFmt*) pFmt, pSdrObj );
/*N*/               break;
/*N*/           case SWG_SECTFMT:
/*N*/               pFmt = pDoc->MakeSectionFmt(
/*N*/                       (SwSectionFmt*) FindFmt( nDerived, cKind ) );
/*N*/               break;
/*N*/       }
/*N*/   }
/*N*/   else
/*N*/   {
/*N*/       // Fuellen eines vorhandenen Formats
/*N*/       pFmt->ResetAllAttr();
/*N*/       if( nDerived != IDX_NO_VALUE )
/*N*/           pFmt->SetDerivedFrom( FindFmt( nDerived, cKind ) );
/*N*/   }
/*N*/
/*N*/   if( nPoolId != IDX_NO_VALUE )
/*N*/       pFmt->SetPoolFmtId( nPoolId );
/*N*/
/*N*/   if( cMoreFlags & 0x01 )
/*?*/       pFmt->SetAutoUpdateFmt();
/*N*/
/*N*/   while( BytesLeft() )
/*N*/   {
/*N*/       BYTE cSubtype = Peek();
/*N*/       switch( cSubtype )
/*N*/       {
/*N*/           case SWG_ATTRSET:
/*N*/               InAttrSet( (SwAttrSet&) pFmt->GetAttrSet() );
/*N*/               ((SwAttrSet&)pFmt->GetAttrSet()).SetModifyAtAttr( pFmt );
/*N*/
/*N*/               if( cKind == SWG_FLYFMT &&
/*N*/                   !IsVersion( SWG_URLANDMAP, SWG_EXPORT31, SWG_DESKTOP40 ) )
/*N*/               {
/*N*/                   // In 31-FFohne URL-Attribut muss das Attribut noch vom
/*N*/                   // Node in das Format verschoben werden
/*N*/                   SwNoTxtNode *pNd = pDoc->GetNodes()[
/*N*/                       pFmt->GetCntnt().GetCntntIdx()->GetIndex()+1 ]->GetNoTxtNode();
/*N*/
/*N*/                   const SfxPoolItem *pItem;
/*N*/                   if ( pNd && SFX_ITEM_SET == pNd->GetSwAttrSet().
/*N*/                                       GetItemState( RES_URL, FALSE, &pItem))
/*N*/                   {
/*N*/                       pFmt->SetAttr( *pItem );
/*N*/                       pNd->ResetAttr( RES_URL );
/*N*/                   }
/*N*/               }
/*N*/
/*N*/               if( nVersion < SWG_DESKTOP40 && cKind == SWG_FLYFMT )
/*N*/               {
/*N*/                   // In SW31-FlyFrm-Formaten muessen ein par Attrs
/*N*/                   // korregiert werden. Sie werden immer gesetzt, da sie
/*N*/                   // in den Vorlagen nicht korregiert wurden.
/*N*/                   const SvxLRSpaceItem& rLRSpace = pFmt->GetLRSpace();
/*N*/                   const SvxULSpaceItem& rULSpace = pFmt->GetULSpace();
/*N*/
/*N*/                   const SwFmtFrmSize& rFrmSize = pFmt->GetFrmSize();
/*N*/                   Size aConvSize(
/*N*/                       rFrmSize.GetSizeConvertedFromSw31( &rLRSpace,
/*N*/                                                          &rULSpace ) );
/*N*/                   if( rFrmSize.GetSize() != aConvSize )
/*N*/                   {
/*?*/                       SwFmtFrmSize aFrmSize( rFrmSize );
/*?*/                       aFrmSize.SetSize( aConvSize );
/*?*/                       pFmt->SetAttr( aFrmSize );
/*N*/                   }
/*N*/
/*N*/                   const SwFmtHoriOrient& rHori = pFmt->GetHoriOrient();
/*N*/                   SwTwips nConvPos =
/*N*/                       rHori.GetPosConvertedFromSw31( &rLRSpace );
/*N*/                   if( rHori.GetPos() != nConvPos )
/*N*/                   {
/*?*/                       SwFmtHoriOrient aHori( rHori );
/*?*/                       aHori.SetPos( nConvPos );
/*?*/                       pFmt->SetAttr( aHori );
/*N*/                   }
/*N*/
/*N*/                   const SwFmtVertOrient& rVert = pFmt->GetVertOrient();
/*N*/                   nConvPos = rVert.GetPosConvertedFromSw31( &rULSpace );
/*N*/                   if( rVert.GetPos() != nConvPos )
/*N*/                   {
/*N*/                       SwFmtVertOrient aVert( rVert );
/*N*/                       aVert.SetPos( nConvPos );
/*N*/                       pFmt->SetAttr( aVert );
/*N*/                   }
/*N*/               }
/*N*/
/*N*/               if( cKind == SWG_FLYFMT && IsVersion( SWG_LONGIDX ) )
/*N*/               {
/*N*/                   const SfxPoolItem *pItem;
/*N*/                   if( SFX_ITEM_SET == pFmt->GetAttrSet().
/*N*/                               GetItemState( RES_CHAIN, FALSE, &pItem) )
/*N*/                   {
/*?*/                       const SwFmtChain *pChain =
/*?*/                           (const SwFmtChain *)pItem;
/*?*/
/*?*/                       // Den Next des Prev richtig setzen.
/*?*/                       SwFlyFrmFmt *pFlyFmt = pChain->GetPrev();
/*?*/                       if( pFlyFmt )
/*?*/                       {
/*?*/                            SwFmtChain aChain( pFlyFmt->GetChain() );
/*?*/                           OSL_ENSURE( !aChain.GetNext(),
/*?*/                                   "Next ist bereits verkettet" );
/*?*/                           aChain.SetNext( (SwFlyFrmFmt *)pFmt );
/*?*/                           pFlyFmt->SetAttr( aChain );
/*?*/                       }
/*?*/
/*?*/                       // Den Prev des Next richtig setzen.
/*?*/                       pFlyFmt = pChain->GetNext();
/*?*/                       if( pFlyFmt )
/*?*/                       {
/*?*/                            SwFmtChain aChain( pFlyFmt->GetChain() );
/*?*/                           OSL_ENSURE( !aChain.GetPrev(),
/*?*/                                   "Prev ist bereits verkettet" );
/*?*/                           aChain.SetPrev( (SwFlyFrmFmt *)pFmt );
/*?*/                           pFlyFmt->SetAttr( aChain );
/*?*/                       }
/*N*/                   }
/*N*/               }
/*N*/
/*N*/               if( bInsert )
/*N*/               {
/*N*/                   // das Format hat sich geaendert!
/*?*/                   SwFmtChg aHint( pFmt );
/*?*/                   pFmt->Modify( &aHint, &aHint );
/*N*/               }
/*N*/               break;
/*N*/           default:
/*?*/               SkipRec();
/*N*/       }
/*N*/   }
/*N*/   CloseRec( cKind );
/*N*/
/*N*/   if( cKind == SWG_SDRFMT &&
/*N*/       !IsVersion( SWG_WRAPDRAWOBJ, SWG_EXPORT31, SWG_DESKTOP40 ) )
/*N*/       pFmt->SetAttr( SwFmtSurround( SURROUND_THROUGHT ) );
/*N*/
/*N*/   // beim Einfuegen eines Dokuments muessen Rahmen, Grafiken und
/*N*/   // OLE-Nodes noch einen eindeutigen Namen bekommen
/*N*/   // This must happen always, because there are lots of documents
/*N*/   // that contain frames with the same name.
/*N*/   if( cKind == SWG_FLYFMT )
/*N*/   {
/*N*/       const SwNode* pNd =
/*N*/           pDoc->GetNodes()[ pFmt->GetCntnt().GetCntntIdx()->GetIndex()+1 ];
/*N*/       BYTE nNdTyp = !pNd->IsNoTxtNode() ? ND_TEXTNODE : pNd->GetNodeType();
/*N*/
/*N*/       pFmt->SetName( aEmptyStr );
/*N*/
/*N*/       // If the name is not changed, its extension must be kept to be able
/*N*/       // to load the layout.
/*N*/       String aSrchName( aName );
/*N*/       aStringPool.RemoveExtension( aSrchName );
/*N*/       if( bInsert )
/*N*/           aName = aSrchName;
/*N*/
/*N*/       if( pDoc->FindFlyByName( aSrchName, nNdTyp ) )
/*N*/       {
/*?*/           switch( nNdTyp )
/*?*/           {
/*?*/           case ND_TEXTNODE:
/*?*/               aName = pDoc->GetUniqueFrameName();
/*?*/               break;
/*?*/           case ND_GRFNODE:
/*?*/               aName = pDoc->GetUniqueGrfName();
/*?*/               break;
/*?*/           case ND_OLENODE:
/*?*/               aName = pDoc->GetUniqueOLEName();
/*?*/               break;
/*?*/           }
/*N*/       }
/*N*/   }
/*N*/
/*N*/   pFmt->SetName( aName );
/*N*/   if( cKind == SWG_FLYFMT || cKind == SWG_SDRFMT )
/*N*/       nFlyLevel--;
/*N*/
/*N*/ // OPT: Cache fuer Formate im StringPool
/*N*/   if( nStrIdx != IDX_NO_VALUE )
/*N*/       aStringPool.SetCachedFmt( nStrIdx, pFmt );
/*N*/ // /OPT: Cache fuer Formate im StringPool
/*N*/
/*N*/   return pFmt;
/*N*/ }

// globale FlyFrames

/*N*/ void Sw3IoImp::InFlyFrames()
/*N*/ {
/*N*/   BOOL bInsIntoHdrFtrSave = bInsIntoHdrFtr;   // In seitengebundenen sind
/*N*/   bInsIntoHdrFtr = FALSE;                     // Draw-Objekte immer erlaubt
/*N*/
/*N*/   USHORT nPageNumOff = 0;
/*N*/   if( bInsert )       // im einfuege Modus?
/*N*/   {
/*N*/       // dann binde die Seitengebundenen an die akt. Seite + Nummer
/*N*/       // Dazu stelle erstmal fest, ob es ueberhaupt einen entsprechen
/*N*/       // Offset gibt.
/*N*/       SwFrm* pFrm;
/*?*/       SwCntntNode* pCNd = pDoc->GetNodes()[ pCurPaM->GetPoint()->nNode ]->GetCntntNode();
/*?*/       if( pCNd && 0 != ( pFrm = pCNd->GetFrm() ))
/*?*/           nPageNumOff = pFrm->FindPageFrm()->GetPhyPageNum() - 1;
/*N*/   }
/*N*/
/*N*/   USHORT eSave_StartNodeType = eStartNodeType;
/*N*/   eStartNodeType = SwFlyStartNode;
/*N*/
/*N*/   OpenRec( SWG_FLYFRAMES );
/*N*/   while( BytesLeft() )
/*N*/   {
/*N*/       BYTE cType = Peek();
/*N*/       if( cType == SWG_FLYFMT || cType == SWG_SDRFMT )
/*N*/       {
/*N*/           SwFmt* pFmt = InFormat( cType, NULL );
/*N*/           OSL_ENSURE( !pFmt || FLY_PAGE >= pFmt->GetAnchor().GetAnchorId(),
/*N*/                       "Rahmen ist ungueltig gebunden" );
/*N*/           if( pFmt && nPageNumOff )
/*N*/           {
/*?*/               const SwFmtAnchor& rOld = pFmt->GetAnchor();
/*?*/               if( FLY_PAGE == rOld.GetAnchorId() )
/*?*/               {
/*?*/                   pFmt->SetAttr( SwFmtAnchor( FLY_PAGE,
/*?*/                               nPageNumOff + rOld.GetPageNum() ) );
/*?*/               }
/*N*/           }
/*N*/       }
/*N*/       else
/*?*/           Error();
/*N*/   }
/*N*/   CloseRec( SWG_FLYFRAMES );
/*N*/   eStartNodeType = eSave_StartNodeType;
/*N*/   bInsIntoHdrFtr = bInsIntoHdrFtrSave;
/*N*/ }

////////////////////////////////////////////////////////////////////////////

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
