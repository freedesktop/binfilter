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

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#include <hintids.hxx>

#include <vcl/salbtype.hxx>             // FRound
#include <tools/urlobj.hxx>
#include <bf_svtools/undo.hxx>
#include <bf_svtools/fstathelper.hxx>
#include <bf_svtools/imap.hxx>
#include <bf_svtools/filter.hxx>
#include <bf_so3/svstor.hxx>
#include <bf_sfx2/docinf.hxx>
#include <bf_svx/linkmgr.hxx>
#include <bf_svx/boxitem.hxx>
#include <bf_svx/impgrf.hxx>
#include <sot/formats.hxx>

#include <fmtfsize.hxx>
#include <fmturl.hxx>
#include <frmfmt.hxx>

#include <horiornt.hxx>

#include <doc.hxx>
#include <frmatr.hxx>
#include <grfatr.hxx>
#include <swtypes.hxx>

#include <osl/diagnose.h>

#include <ndgrf.hxx>
#include <fmtcol.hxx>
#include <hints.hxx>
#include <sw3io.hxx>
#include <swbaslnk.hxx>
#include <pagefrm.hxx>
#include <viscrs.hxx>
#include <pam.hxx>
namespace binfilter {

// --------------------
// SwGrfNode
// --------------------
/*N*/ SwGrfNode::SwGrfNode( const SwNodeIndex & rWhere,
/*N*/                 const String& rGrfName, const String& rFltName,
/*N*/                 const Graphic* pGraphic,
/*N*/                 SwGrfFmtColl *pGrfColl,
/*N*/                 SwAttrSet* pAutoAttr )
/*N*/   : SwNoTxtNode( rWhere, ND_GRFNODE, pGrfColl, pAutoAttr )
/*N*/ {
/*N*/   aGrfObj.SetSwapStreamHdl( LINK( this, SwGrfNode, SwapGraphic ) );
/*N*/   bInSwapIn = bChgTwipSize = bChgTwipSizeFromPixel = bLoadLowResGrf =
/*N*/       bFrameInPaint = bScaleImageMap = FALSE;
/*N*/   bGrafikArrived = TRUE;
/*N*/   ReRead( rGrfName, rFltName, pGraphic, 0, FALSE );
/*N*/ }

/*N*/ SwGrfNode::SwGrfNode( const SwNodeIndex & rWhere,
/*N*/                       const BfGraphicObject& rGrfObj,
/*N*/                       SwGrfFmtColl *pGrfColl, SwAttrSet* pAutoAttr )
/*N*/   : SwNoTxtNode( rWhere, ND_GRFNODE, pGrfColl, pAutoAttr )
/*N*/ {
/*N*/   aGrfObj = rGrfObj;
/*N*/   aGrfObj.SetSwapStreamHdl( LINK( this, SwGrfNode, SwapGraphic ) );
/*N*/   if( rGrfObj.HasUserData() && rGrfObj.IsSwappedOut() )
/*?*/       aGrfObj.SetSwapState();
/*N*/   bInSwapIn = bChgTwipSize = bChgTwipSizeFromPixel= bLoadLowResGrf =
/*N*/       bFrameInPaint = bScaleImageMap = FALSE;
/*N*/   bGrafikArrived = TRUE;
/*N*/ }

// Konstruktor fuer den SW/G-Reader. Dieser ctor wird verwendet,
// wenn eine gelinkte Grafik gelesen wird. Sie liest diese NICHT ein.


/*N*/ SwGrfNode::SwGrfNode( const SwNodeIndex & rWhere,
/*N*/                 const String& rGrfName, const String& rFltName,
/*N*/                 SwGrfFmtColl *pGrfColl,
/*N*/                 SwAttrSet* pAutoAttr )
/*N*/   : SwNoTxtNode( rWhere, ND_GRFNODE, pGrfColl, pAutoAttr )
/*N*/ {
/*N*/   aGrfObj.SetSwapStreamHdl( LINK( this, SwGrfNode, SwapGraphic ) );
/*N*/
/*N*/   Graphic aGrf; aGrf.SetDefaultType();
/*N*/   aGrfObj.SetGraphic( aGrf, rGrfName );
/*N*/
/*N*/   bInSwapIn = bChgTwipSize = bChgTwipSizeFromPixel = bLoadLowResGrf =
/*N*/       bFrameInPaint = bScaleImageMap = FALSE;
/*N*/   bGrafikArrived = TRUE;
/*N*/
/*N*/   InsertLink( rGrfName, rFltName );
/*N*/   if( IsLinkedFile() )
/*N*/   {
/*N*/       INetURLObject aUrl( rGrfName );
/*N*/       if( INET_PROT_FILE == aUrl.GetProtocol() &&
/*N*/           ::binfilter::IsDocument( aUrl.GetMainURL( INetURLObject::NO_DECODE ) ))
/*N*/       {
/*N*/           // File vorhanden, Verbindung herstellen ohne ein Update
/*N*/           ((SwBaseLink*)&refLink)->Connect();
/*N*/       }
/*N*/   }
/*N*/ }


// erneutes Einlesen, falls Graphic nicht Ok ist. Die
// aktuelle wird durch die neue ersetzt.

/*N*/ BOOL SwGrfNode::ReRead( const String& rGrfName, const String& rFltName,
/*N*/                       const Graphic* pGraphic, const BfGraphicObject* pGrfObj,
/*N*/                       BOOL bNewGrf )
/*N*/ {
/*N*/   BOOL bReadGrf = FALSE, bSetTwipSize = TRUE;
/*N*/
/*N*/   OSL_ENSURE( pGraphic || pGrfObj || rGrfName.Len(),
/*N*/           "GraphicNode without a name, Graphic or GraphicObject" );
/*N*/
/*N*/   // ReadRead mit Namen
/*N*/   if( refLink.Is() )
/*N*/   {
/*?*/       OSL_ENSURE( !bInSwapIn, "ReRead: stehe noch im SwapIn" );
/*?*/       if( rGrfName.Len() )
/*?*/       {
/*?*/           // Besonderheit: steht im FltNamen DDE, handelt es sich um eine
/*?*/           //                  DDE-gelinkte Grafik
/*?*/           String sCmd( rGrfName );
/*?*/           if( rFltName.Len() )
/*?*/           {
/*?*/               USHORT nNewType;
/*?*/               if( rFltName.EqualsAscii( "DDE" ))
/*?*/                   nNewType = OBJECT_CLIENT_DDE;
/*?*/               else
/*?*/               {
/*?*/                     ::binfilter::MakeLnkName( sCmd, 0, rGrfName, aEmptyStr, &rFltName );
/*?*/                   nNewType = OBJECT_CLIENT_GRF;
/*?*/               }
/*?*/
/*?*/               if( nNewType != refLink->GetObjType() )
/*?*/               {
/*?*/                   refLink->Disconnect();
/*?*/                   ((SwBaseLink*)&refLink)->SetObjType( nNewType );
/*?*/               }
/*?*/           }
/*?*/
/*?*/           refLink->SetLinkSourceName( sCmd );
/*?*/       }
/*?*/       else        // kein Name mehr, Link aufheben
/*?*/       {
/*?*/           GetDoc()->GetLinkManager().Remove( refLink );
/*?*/           refLink.Clear();
/*?*/       }
/*?*/
/*?*/       if( pGraphic )
/*?*/       {
/*?*/           aGrfObj.SetGraphic( *pGraphic, rGrfName );
/*?*/           bReadGrf = TRUE;
/*?*/       }
/*?*/       else if( pGrfObj )
/*?*/       {
/*?*/           aGrfObj = *pGrfObj;
/*?*/           if( pGrfObj->HasUserData() && pGrfObj->IsSwappedOut() )
/*?*/               aGrfObj.SetSwapState();
/*?*/           aGrfObj.SetLink( rGrfName );
/*?*/           bReadGrf = TRUE;
/*?*/       }
/*?*/       else
/*?*/       {
/*?*/           // MIB 25.02.97: Daten der alten Grafik zuruecksetzen, damit
/*?*/           // die korrekte Ersatz-Darstellung erscheint, wenn die
/*?*/           // der neue Link nicht geladen werden konnte.
/*?*/           Graphic aGrf; aGrf.SetDefaultType();
/*?*/           aGrfObj.SetGraphic( aGrf, rGrfName );
/*?*/
/*?*/           if( refLink.Is() )
/*?*/           {
/*?*/               if( GetFrm() )
/*?*/               {
/*?*/                   SwMsgPoolItem aMsgHint( RES_GRF_REREAD_AND_INCACHE );
/*?*/                   Modify( &aMsgHint, &aMsgHint );
/*?*/               }
/*?*/               else
/*?*/                   ((SwBaseLink*)&refLink)->SwapIn();
/*?*/           }
/*?*/           bSetTwipSize = FALSE;
/*?*/       }
/*N*/   }
/*N*/   else if( pGraphic && !rGrfName.Len() )
/*N*/   {
/*N*/       aGrfObj.SetGraphic( *pGraphic );
/*N*/       bReadGrf = TRUE;
/*N*/   }
/*N*/   else if( pGrfObj && !rGrfName.Len() )
/*N*/   {
/*?*/       aGrfObj = *pGrfObj;
/*?*/       if( pGrfObj->HasUserData() && pGrfObj->IsSwappedOut() )
/*?*/           aGrfObj.SetSwapState();
/*?*/       bReadGrf = TRUE;
/*N*/   }
/*N*/       // Import einer Grafik:
/*N*/       // Ist die Grafik bereits geladen?
/*N*/   else if( !bNewGrf && GRAPHIC_NONE != aGrfObj.GetType() )
/*N*/       return TRUE;
/*N*/
/*N*/   else
/*N*/   {
/*N*/       // einen neuen Grafik-Link anlegen
/*N*/       InsertLink( rGrfName, rFltName );
/*N*/
/*N*/       if( GetNodes().IsDocNodes() )
/*N*/       {
/*N*/           if( pGraphic )
/*N*/           {
/*N*/               aGrfObj.SetGraphic( *pGraphic, rGrfName );
/*N*/               bReadGrf = TRUE;
/*N*/               // Verbindung herstellen ohne ein Update; Grafik haben wir!
/*N*/               ((SwBaseLink*)&refLink)->Connect();
/*N*/           }
/*N*/           else if( pGrfObj )
/*N*/           {
/*?*/               aGrfObj = *pGrfObj;
/*?*/               aGrfObj.SetLink( rGrfName );
/*?*/               bReadGrf = TRUE;
/*?*/               // Verbindung herstellen ohne ein Update; Grafik haben wir!
/*?*/               ((SwBaseLink*)&refLink)->Connect();
/*N*/           }
/*N*/           else
/*N*/           {
/*N*/               // MIB 25.02.97: Daten der alten Grafik zuruecksetzen, damit
/*N*/               // die korrekte Ersatz-Darstellung erscheint, wenn die
/*N*/               // der neue Kink nicht geladen werden konnte.
/*N*/               Graphic aGrf; aGrf.SetDefaultType();
/*N*/               aGrfObj.SetGraphic( aGrf, rGrfName );
/*N*/               ((SwBaseLink*)&refLink)->SwapIn();
/*N*/           }
/*N*/       }
/*N*/   }
/*N*/
/*N*/   // Bug 39281: Size nicht sofort loeschen - Events auf ImageMaps
/*N*/   //            sollten nicht beim Austauschen nicht ins "leere greifen"
/*N*/   if( bSetTwipSize )
/*N*/       SetTwipSize( ::binfilter::GetGraphicSizeTwip( aGrfObj.GetGraphic(), 0 ) );
/*N*/
/*N*/   // erzeuge noch einen Update auf die Frames
/*N*/   if( bReadGrf && bNewGrf )
/*N*/   {
/*?*/       SwMsgPoolItem aMsgHint( RES_UPDATE_ATTR );
/*?*/       Modify( &aMsgHint, &aMsgHint );
/*N*/   }
/*N*/
/*N*/   return bReadGrf;
/*N*/ }


/*N*/ SwGrfNode::~SwGrfNode()
/*N*/ {
/*N*/   SwDoc* pDoc = GetDoc();
/*N*/   if( refLink.Is() )
/*N*/   {
/*N*/       OSL_ENSURE( !bInSwapIn, "DTOR: stehe noch im SwapIn" );
/*N*/       pDoc->GetLinkManager().Remove( refLink );
/*N*/       refLink->Disconnect();
/*N*/   }
/*N*/   //#39289# Die Frames muessen hier bereits geloescht weil der DTor der
/*N*/   //Frms die Grafik noch fuer StopAnimation braucht.
/*N*/   if( GetDepends() )
/*?*/       DelFrms();
/*N*/ }


/*N*/ SwCntntNode *SwGrfNode::SplitNode( const SwPosition& /*rPos*/ )
/*N*/ {
/*?*/   return this;
/*N*/ }


/*N*/ SwGrfNode * SwNodes::MakeGrfNode( const SwNodeIndex & rWhere,
/*N*/                               const String& rGrfName,
/*N*/                               const String& rFltName,
/*N*/                               const Graphic* pGraphic,
/*N*/                               SwGrfFmtColl* pGrfColl,
/*N*/                               SwAttrSet* pAutoAttr,
/*N*/                               BOOL bDelayed )
/*N*/ {
/*N*/   OSL_ENSURE( pGrfColl, "MakeGrfNode: Formatpointer ist 0." );
/*N*/   SwGrfNode *pNode;
/*N*/   // Delayed erzeugen nur aus dem SW/G-Reader
/*N*/   if( bDelayed )
/*N*/       pNode = new SwGrfNode( rWhere, rGrfName,
/*N*/                               rFltName, pGrfColl, pAutoAttr );
/*N*/   else
/*N*/       pNode = new SwGrfNode( rWhere, rGrfName,
/*N*/                               rFltName, pGraphic, pGrfColl, pAutoAttr );
/*N*/   return pNode;
/*N*/ }

/*N*/ SwGrfNode * SwNodes::MakeGrfNode( const SwNodeIndex & rWhere,
/*N*/                               const BfGraphicObject& rGrfObj,
/*N*/                               SwGrfFmtColl* pGrfColl,
/*N*/                               SwAttrSet* pAutoAttr )
/*N*/ {
/*N*/   OSL_ENSURE( pGrfColl, "MakeGrfNode: Formatpointer ist 0." );
/*N*/   return new SwGrfNode( rWhere, rGrfObj, pGrfColl, pAutoAttr );
/*N*/ }


/*N*/ Size SwGrfNode::GetTwipSize() const
/*N*/ {
/*N*/   return nGrfSize;
/*N*/ }




// Returnwert:
// -1 : ReRead erfolgreich
//  0 : nicht geladen
//  1 : Einlesen erfolgreich

short SwGrfNode::SwapIn( BOOL bWaitForData )
{
    if( bInSwapIn )                 // nicht rekuriv!!
        return !aGrfObj.IsSwappedOut();

    short nRet = 0;
    bInSwapIn = TRUE;
    SwBaseLink* pLink = (SwBaseLink*)(::binfilter::SvBaseLink*) refLink;
    if( pLink )
    {
        if( GRAPHIC_NONE == aGrfObj.GetType() ||
            GRAPHIC_DEFAULT == aGrfObj.GetType() )
        {
            // noch nicht geladener Link
            if( pLink->SwapIn( bWaitForData ) )
                nRet = -1;
            else if( GRAPHIC_DEFAULT == aGrfObj.GetType() )
            {
                // keine default Bitmap mehr, also neu Painten!
                aGrfObj.SetGraphic( Graphic() );
                SwMsgPoolItem aMsgHint( RES_GRAPHIC_PIECE_ARRIVED );
                Modify( &aMsgHint, &aMsgHint );
            }
        }
        else if( aGrfObj.IsSwappedOut() )
            // nachzuladender Link
            nRet = pLink->SwapIn( bWaitForData ) ? 1 : 0;
        else
            nRet = 1;
    }
    else if( aGrfObj.IsSwappedOut() )
    {
        // Die Grafik ist im Storage oder im TempFile drin
        if( !HasStreamName() )
            nRet = (short)aGrfObj.SwapIn();
        else
        {
            SvStorageRef refRoot = GetDoc()->GetDocStorage();
            OSL_ENSURE( refRoot.Is(), "Kein Storage am Doc" );
            if( refRoot.Is() )
            {
                String aStrmName, aPicStgName;
                BOOL bGraphic = GetStreamStorageNames( aStrmName, aPicStgName );
                SvStorageRef refPics = aPicStgName.Len()
                    ? refRoot->OpenStorage( aPicStgName,
                        STREAM_READ | STREAM_SHARE_DENYWRITE )
                    : &refRoot;
                if( refPics->GetError() == SVSTREAM_OK )
                {
                    SvStorageStreamRef refStrm =
                        refPics->OpenStream( aStrmName,
                        STREAM_READ | STREAM_SHARE_DENYWRITE );
                    if( refStrm->GetError() == SVSTREAM_OK )
                    {
                        refStrm->SetVersion( refRoot->GetVersion() );
                        if( bGraphic ? aGrfObj.SwapIn( refStrm )
                                     : ImportGraphic( *refStrm ) )
                            nRet = 1;
                    }
                }
            }
        }
        if( 1 == nRet )
        {
            SwMsgPoolItem aMsg( RES_GRAPHIC_SWAPIN );
            SwCntntNode::Modify( &aMsg, &aMsg );
        }
    }
    else
        nRet = 1;
    DBG_ASSERTWARNING( nRet, "Grafik kann nicht eingeswapt werden" );

    if( nRet )
    {
        if( !nGrfSize.Width() && !nGrfSize.Height() )
            SetTwipSize( ::binfilter::GetGraphicSizeTwip( aGrfObj.GetGraphic(), 0 ) );
    }
    bInSwapIn = FALSE;
    return nRet;
}

/*N*/ BOOL SwGrfNode::GetFileFilterNms( String* pFileNm, String* pFilterNm ) const
/*N*/ {
/*N*/   BOOL bRet = FALSE;
/*N*/   if( refLink.Is() && refLink->GetLinkManager() )
/*N*/   {
/*N*/       USHORT nType = refLink->GetObjType();
/*N*/       if( OBJECT_CLIENT_GRF == nType )
/*N*/           bRet = refLink->GetLinkManager()->GetDisplayNames(
/*N*/                   refLink, 0, pFileNm, 0, pFilterNm );
/*N*/       else if( OBJECT_CLIENT_DDE == nType && pFileNm && pFilterNm )
/*N*/       {
/*?*/           String sApp, sTopic, sItem;
/*?*/           if( refLink->GetLinkManager()->GetDisplayNames(
/*?*/                   refLink, &sApp, &sTopic, &sItem ) )
/*?*/           {
/*?*/                 ( *pFileNm = sApp ) += ::binfilter::cTokenSeperator;
/*?*/                 ( *pFileNm += sTopic ) += ::binfilter::cTokenSeperator;
/*?*/               *pFileNm += sItem;
/*?*/               pFilterNm->AssignAscii( RTL_CONSTASCII_STRINGPARAM( "DDE" ));
/*?*/               bRet = TRUE;
/*?*/           }
/*N*/       }
/*N*/   }
/*N*/   return bRet;
/*N*/ }

/*N*/ void SwGrfNode::InsertLink( const String& rGrfName, const String& rFltName )
/*N*/ {
/*N*/     refLink = new SwBaseLink( ::binfilter::LINKUPDATE_ONCALL, FORMAT_GDIMETAFILE, this );
/*N*/   SwDoc* pDoc = GetDoc();
/*N*/   if( GetNodes().IsDocNodes() )
/*N*/   {
/*N*/       refLink->SetVisible( pDoc->IsVisibleLinks() );
/*N*/       if( rFltName.EqualsAscii( "DDE" ))
/*N*/       {
/*?*/           USHORT nTmp = 0;
/*?*/           String sApp, sTopic, sItem;
/*?*/             sApp = rGrfName.GetToken( 0, ::binfilter::cTokenSeperator, nTmp );
/*?*/             sTopic = rGrfName.GetToken( 0, ::binfilter::cTokenSeperator, nTmp );
/*?*/           sItem = rGrfName.Copy( nTmp );
/*?*/           pDoc->GetLinkManager().InsertDDELink( refLink,
/*N*/                                           sApp, sTopic, sItem );
/*N*/       }
/*N*/       else
/*N*/       {
/*N*/           BOOL bSync = rFltName.EqualsAscii( "SYNCHRON" );
/*N*/           refLink->SetSynchron( bSync );
/*N*/             refLink->SetContentType( SOT_FORMATSTR_ID_SVXB );
/*N*/
/*N*/           pDoc->GetLinkManager().InsertFileLink( *refLink,
/*N*/                                           OBJECT_CLIENT_GRF, rGrfName,
/*N*/                               (!bSync && rFltName.Len() ? &rFltName : 0) );
/*N*/       }
/*N*/   }
/*N*/   aGrfObj.SetLink( rGrfName );
/*N*/ }




/*N*/ void SwGrfNode::SetTwipSize( const Size& rSz )
/*N*/ {
/*N*/   nGrfSize = rSz;
/*N*/ }



/*N*/ BOOL SwGrfNode::GetStreamStorageNames( String& rStrmName,
/*N*/                                     String& rStorName ) const
/*N*/ {
/*N*/   BOOL bGraphic = FALSE;
/*N*/   rStorName.Erase();
/*N*/   rStrmName.Erase();
/*N*/
/*N*/   String aUserData( aGrfObj.GetUserData() );
/*N*/   if( !aUserData.Len() )
/*N*/       return FALSE;
/*N*/
/*N*/   String aProt( RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.Package:" ) );
/*N*/   if( 0 == aUserData.CompareTo( aProt, aProt.Len() ) )
/*N*/   {
/*N*/       // 6.0 (XML) Package
/*?*/       xub_StrLen nPos = aUserData.Search( '/' );
/*?*/       if( STRING_NOTFOUND == nPos )
/*?*/       {
/*?*/           rStrmName = aUserData.Copy( aProt.Len() );
/*?*/       }
/*?*/       else
/*?*/       {
/*?*/           rStorName = aUserData.Copy( aProt.Len(), nPos-aProt.Len() );
/*?*/           rStrmName = aUserData.Copy( nPos+1 );
/*?*/       }
/*N*/   }
/*N*/   else
/*N*/   {
/*N*/       // 3.1 - 5.2
/*N*/       rStorName = String( RTL_CONSTASCII_USTRINGPARAM( "EmbeddedPictures" ) );
/*N*/       rStrmName = aUserData;
/*N*/       bGraphic = TRUE;
/*N*/   }
/*N*/   OSL_ENSURE( STRING_NOTFOUND == rStrmName.Search( '/' ),
/*N*/           "invalid graphic stream name" );
/*N*/
/*N*/   return bGraphic;
/*N*/ }

/*N*/ SwCntntNode* SwGrfNode::MakeCopy( SwDoc* pDoc, const SwNodeIndex& rIdx ) const
/*N*/ {
/*N*/   // kopiere die Formate in das andere Dokument:
/*N*/   SwGrfFmtColl* pColl = pDoc->CopyGrfColl( *GetGrfColl() );
/*N*/
/*N*/   SwGrfNode* pThis = (SwGrfNode*)this;
/*N*/
/*N*/   Graphic aTmpGrf;
/*N*/   SwBaseLink* pLink = (SwBaseLink*)(::binfilter::SvBaseLink*) refLink;
/*N*/   if( !pLink && HasStreamName() )
/*N*/   {
/*?*/       SvStorageRef refRoot = pThis->GetDoc()->GetDocStorage();
/*?*/       OSL_ENSURE( refRoot.Is(), "Kein Storage am Doc" );
/*?*/       if( refRoot.Is() )
/*?*/       {
/*?*/           String aStrmName, aPicStgName;
/*?*/           BOOL bGraphic = GetStreamStorageNames( aStrmName, aPicStgName );
/*?*/           SvStorageRef refPics = aPicStgName.Len()
/*?*/               ? refRoot->OpenStorage( aPicStgName,
/*?*/                       STREAM_READ | STREAM_SHARE_DENYWRITE )
/*?*/               : &refRoot;
/*?*/           if( refPics->GetError() == SVSTREAM_OK )
/*?*/           {
/*?*/               SvStorageStreamRef refStrm = refPics->OpenStream( aStrmName,
/*?*/                       STREAM_READ | STREAM_SHARE_DENYWRITE );
/*?*/               if( refStrm->GetError() == SVSTREAM_OK )
/*?*/               {
/*?*/                   refStrm->SetVersion( refRoot->GetVersion() );
/*?*/                   if( bGraphic )
/*?*/                       aTmpGrf.SwapIn( refStrm );
/*?*/                   else
/*?*/                       GetGrfFilter()->ImportGraphic( aTmpGrf, String(),
/*?*/                                                      *refStrm );
/*?*/               }
/*?*/           }
/*N*/       }
/*N*/   }
/*N*/   else
/*N*/   {
/*N*/       aTmpGrf = aGrfObj.GetGraphic();
/*N*/   }
/*N*/
/*N*/   const ::binfilter::SvLinkManager& rMgr = GetDoc()->GetLinkManager();
/*N*/   String sFile, sFilter;
/*N*/   if( IsLinkedFile() )
/*N*/       rMgr.GetDisplayNames( refLink, 0, &sFile, 0, &sFilter );
/*N*/   else if( IsLinkedDDE() )
/*N*/   {
/*?*/       String sTmp1, sTmp2;
/*?*/       rMgr.GetDisplayNames( refLink, &sTmp1, &sTmp2, &sFilter );
/*?*/         ::binfilter::MakeLnkName( sFile, &sTmp1, sTmp2, sFilter );
/*?*/       sFilter.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "DDE" ));
/*N*/   }
/*N*/
/*N*/   SwGrfNode* pGrfNd = pDoc->GetNodes().MakeGrfNode( rIdx, sFile, sFilter,
/*N*/                                                   &aTmpGrf, pColl,
/*N*/                                           (SwAttrSet*)GetpSwAttrSet() );
/*N*/   pGrfNd->SetAlternateText( GetAlternateText() );
/*N*/     pGrfNd->SetContour( HasContour(), HasAutomaticContour() );
/*N*/   return pGrfNd;
/*N*/ }

/*M*/ IMPL_LINK( SwGrfNode, SwapGraphic, BfGraphicObject*, pGrfObj )
/*M*/ {
/*M*/   SvStream* pRet = NULL;
/*M*/
/*M*/   // #101174#: Keep graphic while in swap in. That's at least important
/*M*/   // when breaking links, because in this situation a reschedule call and
/*M*/   // a DataChanged call lead to a paint of the graphic.
/*M*/   if( pGrfObj->IsInSwapOut() && (IsSelected() || bInSwapIn) )
/*M*/       pRet = GRFMGR_AUTOSWAPSTREAM_NONE;
/*M*/   else if( !refLink.Is() )
/*M*/   {
/*M*/       pRet = GRFMGR_AUTOSWAPSTREAM_TEMP;
/*M*/
/*M*/       if( HasStreamName() )
/*M*/       {
/*M*/           SvStorageRef refRoot = GetDoc()->GetDocStorage();
/*M*/           OSL_ENSURE( refRoot.Is(), "Kein Storage am Doc" );
/*M*/           if( refRoot.Is() )
/*M*/           {
/*M*/               String aStrmName, aPicStgName;
/*M*/               BOOL bGraphic = GetStreamStorageNames( aStrmName, aPicStgName );
/*M*/               SvStorageRef refPics = aPicStgName.Len()
/*M*/                   ? refRoot->OpenStorage( aPicStgName,
/*M*/                       STREAM_READ | STREAM_SHARE_DENYWRITE )
/*M*/                   : &refRoot;
/*M*/               if( refPics->GetError() == SVSTREAM_OK )
/*M*/               {
/*M*/                   SvStream* pTmp = refPics->OpenStream( aStrmName,
/*M*/                       STREAM_READ | STREAM_SHARE_DENYWRITE );
/*M*/                   BOOL bDelStrm = TRUE;
/*M*/                   if( pTmp->GetError() == SVSTREAM_OK )
/*M*/                   {
/*M*/                       if( pGrfObj->IsInSwapOut() )
/*M*/                           pRet = GRFMGR_AUTOSWAPSTREAM_LINK;
/*M*/                       else
/*M*/                       {
/*M*/                           if( bGraphic )
/*M*/                           {
/*M*/                               pRet = pTmp;
/*M*/                               bDelStrm = FALSE;
/*M*/                               pRet->SetVersion( refRoot->GetVersion() );
/*M*/                           }
/*M*/                           else
/*M*/                           {
/*M*/                               ImportGraphic( *pTmp );
/*M*/                               pRet = GRFMGR_AUTOSWAPSTREAM_LOADED;
/*M*/                           }
/*M*/                       }
/*M*/                   }
/*M*/                   if( bDelStrm )
/*M*/                       delete pTmp;
/*M*/               }
/*M*/           }
/*M*/       }
/*M*/   }
/*M*/
/*M*/   return (long)pRet;
/*M*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
