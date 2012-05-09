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

#define _SVSTDARR_USHORTS
#include <bf_svtools/macitem.hxx>
#include <bf_svtools/urihelper.hxx>

#include <horiornt.hxx>

#include <doc.hxx>
#include <osl/diagnose.h>
#include <swtable.hxx>

#include <osl/diagnose.h>

#include <pam.hxx>
#include <cntfrm.hxx>
#include <pagefrm.hxx>
#include <txtftn.hxx>
#include <fmtftn.hxx>
#include <fmtfld.hxx>
#include <fmtanchr.hxx>
#include <fmthdft.hxx>
#include <fmtcntnt.hxx>
#include <fmtfsize.hxx>
#include <fmtpdsc.hxx>
#include <fmtflcnt.hxx>
#include <fmtrfmrk.hxx>
#include <fchrfmt.hxx>
#include <fmtinfmt.hxx>
#include <fmtcnct.hxx>
#include <charfmt.hxx>
#include <ftninfo.hxx>
#include <sw3imp.hxx>
#include <cellatr.hxx>
#include <tox.hxx>
#include <pagedesc.hxx>
#include <fmtruby.hxx>
#include <tgrditem.hxx>

#include "bf_so3/staticbaseurl.hxx"
namespace binfilter {


#define URL_DECODE  \
    , INetURLObject::WAS_ENCODED, INetURLObject::DECODE_UNAMBIGUOUS


// Diese Datei hat einige Besonderheiten:
// Die aktuelle Sw3IoImp-Instanz wird ueber Sw3IoImp::GetCurrentIo()
// angefordert. Da diese die Variable pStrm mit dem aktuellen I/O-
// Stream besetzt hat, muss dieser auf den uebergebenen Stream umgesetzt
// werden. Dabei wird angenommen, dass alle uebergebenen Streams
// SvStorageStreams sind.

////////////////////////////// Frame-Attribute ////////////////////////////

/*N*/ SfxPoolItem* SwFmtAnchor::Create( SvStream& rStrm, USHORT nIVer ) const
/*N*/ {
/*N*/   BYTE cType;
/*N*/   USHORT nIndex;
/*N*/   rStrm >> cType;
/*N*/   if( nIVer < IVER_FMTANCHOR_LONGIDX )
/*N*/   {
/*N*/       rStrm >> nIndex;
/*N*/   }
/*N*/   else
/*N*/   {
/*N*/       UINT32 nIndex32 = Sw3IoImp::InULong( rStrm );
/*N*/       OSL_ENSURE( nIndex32 <= USHRT_MAX,
/*N*/               "SwFmtAnchor: Index/Seitenzahl > USHRT_MAX" );
/*N*/       nIndex = (USHORT)nIndex32;
/*N*/   }
/*N*/   // Dieser Anker ist fuer FLY_AT_CNTNT und FLY_IN_CNTNT
/*N*/   // unvollstaendig und muss noch angepasst werden. Dies
/*N*/   // geschieht nach dem Einlesen des FrmFmts in InTxtNode().
/*N*/   return new SwFmtAnchor( (RndStdIds) cType, nIndex );
/*N*/ }

/*N*/ USHORT SwFmtAnchor::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/   OSL_ENSURE( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/           "SwFmtAnchor: Gibt es ein neues Fileformat?" );
/*N*/   return ( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/            SOFFICE_FILEFORMAT_40==nFFVer ) ? 0 : IVER_FMTANCHOR_LONGIDX;
/*N*/ }

/*N*/ SfxPoolItem* SwFmtHeader::Create( SvStream& rStrm, USHORT ) const
/*N*/ {
/*N*/   BYTE bActive2;
/*N*/   rStrm >> bActive2;
/*N*/   Sw3IoImp* pIo = Sw3IoImp::GetCurrentIo();
/*N*/   SvStream* p = pIo->pStrm;
/*N*/   pIo->pStrm = (SvStorageStream*) &rStrm;
/*N*/
/*N*/   USHORT eSave_StartNodeType = pIo->eStartNodeType;
/*N*/   pIo->eStartNodeType = SwHeaderStartNode;
/*N*/
/*N*/   SwFmtHeader* pAttr = NULL;
/*N*/   if( pIo->Peek() == SWG_FREEFMT )
/*N*/   {
/*N*/       SwFrmFmt* pFmt = (SwFrmFmt*) pIo->InFormat( SWG_FREEFMT, NULL );
/*N*/       if( pFmt )
/*N*/       {
/*N*/           pAttr = new SwFmtHeader( pFmt );
/*N*/           pAttr->SetActive( BOOL( bActive2 ) );
/*N*/       }
/*N*/       else
/*?*/           pIo->Error();
/*N*/   }
/*N*/   // Ohne Format ist das Attribut immer inaktiv!
/*N*/   else pAttr = new SwFmtHeader( BOOL( FALSE ) );
/*N*/   pIo->pStrm = p;
/*N*/   pIo->eStartNodeType = eSave_StartNodeType;
/*N*/
/*N*/   return pAttr;
/*N*/ }

/*N*/ SfxPoolItem* SwFmtFooter::Create( SvStream& rStrm, USHORT ) const
/*N*/ {
/*N*/   BYTE bActive3;
/*N*/   rStrm >> bActive3;
/*N*/   Sw3IoImp* pIo = Sw3IoImp::GetCurrentIo();
/*N*/   SvStream* p = pIo->pStrm;
/*N*/   pIo->pStrm = (SvStorageStream*) &rStrm;
/*N*/
/*N*/   USHORT eSave_StartNodeType = pIo->eStartNodeType;
/*N*/   pIo->eStartNodeType = SwFooterStartNode;
/*N*/
/*N*/   SwFmtFooter* pAttr = NULL;
/*N*/   if( pIo->Peek() == SWG_FREEFMT )
/*N*/   {
/*N*/       SwFrmFmt* pFmt = (SwFrmFmt*) pIo->InFormat( SWG_FREEFMT, NULL );
/*N*/       if( pFmt )
/*N*/       {
/*N*/           // Emulate 5.2 footer size bug:
/*N*/           if ( pIo->IsVersion( SWG_MAJORVERSION_50 ) )
/*N*/           {
/*N*/               SwFmtFrmSize aSize = pFmt->GetFrmSize( FALSE );
/*N*/               if ( ATT_MIN_SIZE == aSize.GetSizeType() )
/*N*/               {
/*N*/                   aSize.SetHeight( 0 );
/*N*/                   pFmt->SetAttr( aSize );
/*N*/               }
/*N*/           }
/*N*/           pAttr = new SwFmtFooter( pFmt );
/*N*/           pAttr->SetActive( BOOL( bActive3 ) );
/*N*/       }
/*N*/       else
/*?*/           pIo->Error();
/*N*/   }
/*N*/   // Ohne Format ist das Attribut immer inaktiv!
/*N*/   else pAttr = new SwFmtFooter( BOOL( FALSE ) );
/*N*/   pIo->pStrm = p;
/*N*/   pIo->eStartNodeType = eSave_StartNodeType;
/*N*/
/*N*/   return pAttr;
/*N*/ }

/*N*/ SfxPoolItem* SwFmtCntnt::Create( SvStream& rStrm, USHORT ) const
/*N*/ {
/*N*/   Sw3IoImp* pIo = Sw3IoImp::GetCurrentIo();
/*N*/   SvStream* p = pIo->pStrm;
/*N*/   pIo->pStrm = (SvStorageStream*) &rStrm;
/*N*/   if( pIo->bInsert )
/*N*/   {
/*?*/       if( !pIo->pSectionDepths )
/*?*/           pIo->pSectionDepths = new SvUShorts;
/*?*/       pIo->pSectionDepths->Insert( (USHORT)0U, pIo->pSectionDepths->Count() );
/*N*/   }
/*N*/   SwStartNode* pSttNd = &pIo->InContents();
/*N*/   if( pIo->bInsert )
/*N*/   {
/*?*/       OSL_ENSURE( pIo->pSectionDepths, "There is the section depth stack?" );
/*?*/       OSL_ENSURE( pIo->pSectionDepths->Count() > 0U,
/*?*/               "section depth stack is empty" );
/*?*/       pIo->pSectionDepths->Remove( pIo->pSectionDepths->Count() - 1U );
/*N*/   }
/*N*/   pIo->pStrm = p;
/*N*/   return new SwFmtCntnt( pSttNd );
/*N*/ }

/*N*/ SfxPoolItem* SwFmtPageDesc::Create( SvStream& rStrm, USHORT nVersion ) const
/*N*/ {
/*N*/   // Hier wird nur der Name im Attribut gesetzt. Spaeter wird
/*N*/   // mit ConnectPageDescAttrs() die eigentliche Verbindung
/*N*/   // hergestellt.
/*N*/   BYTE bAuto;
/*N*/   UINT16 nOff, nIdx;
/*N*/   if( nVersion < IVER_FMTPAGEDESC_LONGPAGE )
/*N*/   {
/*N*/       if( nVersion < IVER_FMTPAGEDESC_NOAUTO )
/*N*/           rStrm >> bAuto;
/*N*/       rStrm >> nOff >> nIdx;
/*N*/   }
/*N*/   else
/*N*/   {
/*N*/       ULONG nOff32 = Sw3IoImp::InULong( rStrm );
/*N*/       OSL_ENSURE( nOff32 <= USHRT_MAX, "SwFmtPageDesc: Seitenzahl > USHRT_MAX" );
/*N*/       nOff = (USHORT)nOff32;
/*N*/       rStrm >> nIdx;
/*N*/   }
/*N*/   SwFmtPageDesc* pAttr = new SwFmtPageDesc( 0 );
/*N*/   pAttr->SetNumOffset( nOff );
/*N*/   pAttr->SetDescNameIdx( nIdx );
/*N*/   return pAttr;
/*N*/ }

/*N*/ USHORT SwFmtPageDesc::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/   OSL_ENSURE( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/           "SwFmtPageDesc: Gibt es ein neues Fileformat?" );
/*N*/   return ( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/            SOFFICE_FILEFORMAT_40==nFFVer ) ? 0 : IVER_FMTPAGEDESC_LONGPAGE;
/*N*/ }

/*N*/ SfxPoolItem* SwFmtFlyCnt::Create( SvStream& rStrm, USHORT ) const
/*N*/ {
/*N*/   Sw3IoImp* pIo = Sw3IoImp::GetCurrentIo();
/*N*/   SvStream* p = pIo->pStrm;
/*N*/   pIo->pStrm = (SvStorageStream*) &rStrm;
/*N*/
/*N*/   USHORT eSave_StartNodeType = pIo->eStartNodeType;
/*N*/   pIo->eStartNodeType = SwFlyStartNode;
/*N*/
/*N*/   SwFrmFmt* pTmpFmt = NULL;
/*N*/   BYTE cKind = pIo->Peek();
/*N*/   if( SWG_SDRFMT==cKind )
/*N*/   {
/*N*/       if( pIo->bInsIntoHdrFtr )
/*N*/       {
/*?*/           pIo->SkipRec();
/*?*/           pIo->bDrawFmtSkipped = TRUE;
/*N*/       }
/*N*/       else
/*N*/       {
/*N*/           pTmpFmt = (SwFrmFmt*) pIo->InFormat( SWG_SDRFMT, NULL );
/*N*/       }
/*N*/   }
/*N*/   else
/*N*/   {
/*N*/       pTmpFmt = (SwFrmFmt*) pIo->InFormat( SWG_FLYFMT, NULL );
/*N*/   }
/*N*/
/*N*/   pIo->pStrm = p;
/*N*/   pIo->eStartNodeType = eSave_StartNodeType;
/*N*/
/*N*/   if( pTmpFmt )
/*N*/       return new SwFmtFlyCnt( pTmpFmt );
/*N*/
/*?*/   if( !pIo->bInsIntoHdrFtr || SWG_SDRFMT!=cKind )
/*?*/       pIo->Error();
/*?*/   return NULL;
/*N*/ }

//////////////////////////////// Text-Attribute ////////////////////////////

/*N*/ SfxPoolItem* SwFmtRefMark::Create( SvStream& rStrm, USHORT ) const
/*N*/ {
/*N*/   String aName = rStrm.ReadUniOrByteString( rStrm.GetStreamCharSet() );
/*N*/   return new SwFmtRefMark( aName );
/*N*/ }

/*N*/ SfxPoolItem* SwFmtCharFmt::Create( SvStream& rStrm, USHORT ) const
/*N*/ {
/*N*/   UINT16 nIdx;
/*N*/   rStrm >> nIdx;
/*N*/   if( nIdx == IDX_NO_VALUE )
/*N*/       return NULL;
/*N*/   Sw3IoImp* pIo = Sw3IoImp::GetCurrentIo();
/*N*/   SwCharFmt* pChFmt = (SwCharFmt*) pIo->FindFmt( nIdx, SWG_CHARFMT );
/*N*/   return new SwFmtCharFmt( pChFmt );
/*N*/ }

/*N*/ SfxPoolItem* SwFmtINetFmt::Create( SvStream& rStrm, USHORT nIVer ) const
/*N*/ {
/*N*/   UINT16 nId1, nId2;
/*N*/   String aURL1 = rStrm.ReadUniOrByteString( rStrm.GetStreamCharSet() );
/*N*/   String aTarget = rStrm.ReadUniOrByteString( rStrm.GetStreamCharSet() );
/*N*/   rStrm >> nId1 >> nId2;
/*N*/
/*N*/   aURL1 = ::binfilter::StaticBaseUrl::SmartRelToAbs( aURL1 );
/*N*/   SwFmtINetFmt *pNew = new SwFmtINetFmt( aURL1, aTarget );
/*N*/   Sw3IoImp* pIo = Sw3IoImp::GetCurrentIo();
/*N*/   if( nId1 != IDX_NO_VALUE )
/*N*/   {
/*?*/       SwCharFmt* pChFmt = (SwCharFmt*) pIo->FindFmt( nId1, SWG_CHARFMT );
/*?*/       if( pChFmt )
/*?*/       {
/*?*/           pNew->aINetFmt = pChFmt->GetName();
/*?*/           pNew->nINetId = pChFmt->GetPoolFmtId();
/*?*/       }
/*N*/   }
/*N*/   if( nId2 != IDX_NO_VALUE )
/*N*/   {
/*?*/       SwCharFmt* pChFmt = (SwCharFmt*) pIo->FindFmt( nId2, SWG_CHARFMT );
/*?*/       if( pChFmt )
/*?*/       {
/*?*/           pNew->aVisitedFmt = pChFmt->GetName();
/*?*/           pNew->nVisitedId = pChFmt->GetPoolFmtId();
/*?*/       }
/*N*/   }
/*N*/   USHORT nCnt;
/*N*/   rStrm >> nCnt;
/*N*/   while( nCnt-- )
/*N*/   {
/*N #i27164#*/      USHORT nCurKey;
/*N #i27164#*/      String aLibName, aMacName;
/*N #i27164#*/      rStrm >> nCurKey;
/*N #i27164#*/      aLibName = rStrm.ReadUniOrByteString( rStrm.GetStreamCharSet() );
/*N #i27164#*/      aMacName = rStrm.ReadUniOrByteString( rStrm.GetStreamCharSet() );
/*N #i27164#*/      pNew->SetMacro( nCurKey, SvxMacro( aMacName, aLibName, STARBASIC ) );
/*N*/   }
/*N*/   if( nIVer >= 1 )
/*N*/   {
/*N*/       String aName1 = rStrm.ReadUniOrByteString( rStrm.GetStreamCharSet() );;
/*N*/       pNew->SetName( aName1 );
/*N*/   }
/*N*/   if( nIVer >= 2 )
/*N*/   {
/*N*/       rStrm >> nCnt;
/*N*/       while( nCnt-- )
/*N*/       {
/*N #i27164#*/          USHORT nCurKey, nScriptType;
/*N #i27164#*/          rStrm >> nCurKey;
/*N #i27164#*/          String aLibName = rStrm.ReadUniOrByteString( rStrm.GetStreamCharSet() );
/*N #i27164#*/          String aMacName = rStrm.ReadUniOrByteString( rStrm.GetStreamCharSet() );
/*N #i27164#*/          rStrm >> nScriptType;
/*N #i27164#*/          pNew->SetMacro( nCurKey, SvxMacro( aMacName, aLibName,
/*N #i27164#*/                                      (ScriptType)nScriptType ) );
/*N*/       }
/*N*/   }
/*N*/
/*N*/   return pNew;
/*N*/ }

/*N*/ USHORT SwFmtINetFmt::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/   OSL_ENSURE( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/           "SwFmtINetFmr: Gibt es ein neues Fileformat?" );
/*N*/   return SOFFICE_FILEFORMAT_31==nFFVer ? USHRT_MAX : 2;
/*N*/ }


/*N*/ SfxPoolItem* SwFmtFtn::Create( SvStream& rStrm, USHORT nIVer ) const
/*N*/ {
/*N*/   UINT16 nNumber1 = 0;
/*N*/   rStrm >> nNumber1;
/*N*/   String aNumber1 = rStrm.ReadUniOrByteString( rStrm.GetStreamCharSet() );
/*N*/
/*N*/   // Die Section fuer den Text erzeugen
/*N*/   Sw3IoImp* pIo = Sw3IoImp::GetCurrentIo();
/*N*/   SwNodes& rNodes = pIo->pDoc->GetNodes();
/*N*/   SwNodeIndex aStart( rNodes.GetEndOfInserts() );
/*N*/   SwStartNode* pSttNd = rNodes.MakeEmptySection( aStart,SwFootnoteStartNode );
/*N*/   aStart = *pSttNd->EndOfSectionNode();
/*N*/
/*N*/   if( pIo->bInsert )
/*N*/   {
/*?*/       if( !pIo->pSectionDepths )
/*?*/           pIo->pSectionDepths = new SvUShorts;
/*?*/       pIo->pSectionDepths->Insert( (USHORT)0U, pIo->pSectionDepths->Count() );
/*N*/   }
/*N*/   pIo->InContents( aStart, 0, FALSE );
/*N*/   if( pIo->bInsert )
/*N*/   {
/*?*/       OSL_ENSURE( pIo->pSectionDepths, "There is the section depth stack?" );
/*?*/       OSL_ENSURE( pIo->pSectionDepths->Count() > 0U,
/*?*/               "section depth stack is empty" );
/*?*/       pIo->pSectionDepths->Remove( pIo->pSectionDepths->Count() - 1U );
/*N*/   }
/*N*/
/*N*/   // die Seq-Nummer einlesen - fuer die Querverweise auf Fussnoten
/*N*/   USHORT nSeqNo = 0;
/*N*/   BOOL bEndNote1 = FALSE;
/*N*/   if( 1 <= nIVer )
/*N*/   {
/*N*/       rStrm >> nSeqNo;
/*N*/   }
/*N*/   if( 2 <= nIVer )
/*N*/   {
/*N*/       BYTE nFlags = 0;
/*N*/       rStrm >> nFlags;
/*N*/       bEndNote1 = (nFlags & 0x01) != 0;
/*N*/   }
/*N*/
/*N*/   SwFmtFtn aFtn( bEndNote1 );
/*N*/   aFtn.SetNumStr( aNumber1 );
/*N*/   aFtn.SetNumber( nNumber1 );
/*N*/
/*N*/   // Das Fussnoten-Attribut liest seine Section "auf der Wiese" ein.
/*N*/   // Hier muss also der Start errechnet und eingetragen werden.
/*N*/   SwFmtFtn& rNew = (SwFmtFtn&)pIo->pDoc->GetAttrPool().Put( aFtn );
/*N*/   SwTxtFtn* pAttr = new SwTxtFtn( rNew, 0 );
/*N*/   aStart = *pSttNd;
/*N*/   pAttr->SetStartNode( &aStart );
/*N*/   if( 1 <= nIVer )
/*N*/       pAttr->SetSeqNo( nSeqNo );
/*N*/
/*N*/   return &rNew;
/*N*/ }

/*N*/ USHORT SwFmtFtn::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/   OSL_ENSURE( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/           "SwFmtINetFmr: Gibt es ein neues Fileformat?" );
/*N*/   return SOFFICE_FILEFORMAT_31 == nFFVer ? 0 :
/*N*/          (SOFFICE_FILEFORMAT_40 == nFFVer ? 1 : 2);
/*N*/ }


/*N*/ SfxPoolItem* SwFmtFld::Create( SvStream& rStrm, USHORT ) const
/*N*/ {
/*N*/   Sw3IoImp* pIo = Sw3IoImp::GetCurrentIo();
/*N*/   SvStream* p = pIo->pStrm;
/*N*/   pIo->pStrm = (SvStorageStream*) &rStrm;
/*N*/   SwField* pFld = pIo->InField();
/*N*/   pIo->pStrm = p;
/*N*/   if( !pFld )
/*N*/       return NULL;
/*N*/
/*N*/   SwFmtFld* pAttr = new SwFmtFld;
/*N*/   pAttr->pField = pFld;
/*N*/   return pAttr;
/*N*/ }

/*N*/ SfxPoolItem* SwTOXMark::Create( SvStream& rStrm, USHORT nIVer ) const
/*N*/ {
/*N*/   BYTE cType = 0;
/*N*/   UINT16 nLevel1 = 0, nStrIdx = IDX_NO_VALUE;
/*N*/   String aTypeName, aAltText1, aPrimKey, aSecKey;
/*N*/   Sw3IoImp* pIo = Sw3IoImp::GetCurrentIo();
/*N*/   rStrm >> cType
/*N*/         >> nLevel1;
/*N*/
/*N*/   if( nIVer < IVER_TOXMARK_STRPOOL )
/*N*/       aTypeName = rStrm.ReadUniOrByteString( rStrm.GetStreamCharSet() );
/*N*/   else
/*N*/       rStrm >> nStrIdx;
/*N*/
/*N*/   aAltText1 = rStrm.ReadUniOrByteString( rStrm.GetStreamCharSet() );
/*N*/   aPrimKey = rStrm.ReadUniOrByteString( rStrm.GetStreamCharSet() );
/*N*/   aSecKey = rStrm.ReadUniOrByteString( rStrm.GetStreamCharSet() );
/*N*/
/*N*/   BYTE cFlags = 0;
/*N*/   // With the 5.2, there are new tox types.
/*N*/   if( nIVer >= IVER_TOXMARK_NEWTOX )
/*N*/   {
/*N*/       rStrm >> cType >> nStrIdx >> cFlags;
/*N*/   }
/*N*/
/*N*/   TOXTypes eType = (TOXTypes)cType;
/*N*/   if( nIVer >= IVER_TOXMARK_STRPOOL )
/*N*/   {
/*N*/       if( nStrIdx != IDX_NO_VALUE )
/*?*/           aTypeName = pIo->aStringPool.Find( nStrIdx );
/*N*/       else
/*N*/           aTypeName = SwTOXBase::GetTOXName( eType );
/*N*/   }
/*N*/
/*N*/   // Search tox type
/*N*/   const SwTOXType *pType = NULL;
/*N*/   USHORT n = pIo->pDoc->GetTOXTypeCount( eType );
/*N*/   for( USHORT i = 0; i < n; i++ )
/*N*/   {
/*N*/       const SwTOXType *pTmp = pIo->pDoc->GetTOXType( eType, i );
/*N*/       if( pTmp && pTmp->GetTypeName() == aTypeName )
/*N*/       {
/*N*/           pType = pTmp;
/*N*/           break;
/*N*/       }
/*N*/   }
/*N*/
/*N*/   // If the tox type is unknown, a new one is created.
/*N*/   if( !pType )
/*N*/   {
/*N*/       pIo->pDoc->InsertTOXType( SwTOXType( eType, aTypeName ) );
/*N*/       pType = pIo->pDoc->GetTOXType( eType, n );
/*N*/   }
/*N*/   OSL_ENSURE( pType, "unknown tox type" );
/*N*/   if( !pType )
/*N*/   {
/*?*/       pIo->Error();
/*N*/       return NULL;
/*N*/   }
/*N*/
/*N*/   SwTOXMark* pMark = new SwTOXMark( pType );
/*N*/   pMark->SetAlternativeText( aAltText1 );
/*N*/   switch( eType )
/*N*/   {
/*N*/       case TOX_INDEX:
/*N*/           if( aPrimKey.Len() )
/*N*/               pMark->SetPrimaryKey( aPrimKey );
/*N*/           if( aSecKey.Len() )
/*?*/               pMark->SetSecondaryKey( aSecKey );
/*N*/           break;
/*N*/       case TOX_USER:
/*N*/       case TOX_CONTENT:
/*N*/       case TOX_ILLUSTRATIONS:
/*N*/       case TOX_OBJECTS:
/*N*/       case TOX_TABLES:
/*N*/       case TOX_AUTHORITIES:
/*N*/           pMark->SetLevel( nLevel1 );
/*N*/           break;
/*N*/       default:
/*?*/           pIo->Error();
/*?*/           delete pMark;
/*?*/           return 0;
/*N*/   }
/*N*/
/*N*/   pMark->SetAutoGenerated( 0 != (cFlags & 0x01) );
/*N*/   pMark->SetMainEntry( 0 != (cFlags & 0x02) );
/*N*/
/*N*/   return pMark;
/*N*/ }

/*N*/ USHORT SwTOXMark::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/   OSL_ENSURE( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/           "SwTOXMark: Gibt es ein neues Fileformat?" );
/*N*/   return ( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/            SOFFICE_FILEFORMAT_40==nFFVer ) ? 0 : IVER_TOXMARK_NEWTOX;
/*N*/ }

SfxPoolItem* SwFmtRuby::Create(SvStream & rStrm, USHORT /*nVer*/) const
{
    String sRubyTxt1;
    SwFmtRuby* pRet = new SwFmtRuby( sRubyTxt1 );

    BOOL bVal = FALSE;
    rStrm >> bVal;

    return pRet;
}

/*N*/ USHORT SwFmtRuby::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/   OSL_ENSURE( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/           "SwFmtRuby: Gibt es ein neues Fileformat?" );
/*N*/
/*N*/   return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
/*N*/ }


SfxPoolItem* SwTblBoxFormula::Create( SvStream & rStrm, USHORT ) const
{
    String sStr = rStrm.ReadUniOrByteString( rStrm.GetStreamCharSet() );
    return new SwTblBoxFormula( sStr );
}

/*N*/ USHORT SwTblBoxFormula::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/   OSL_ENSURE( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/           "SwTblBoxFormula: Gibt es ein neues Fileformat?" );
/*N*/   return SOFFICE_FILEFORMAT_31==nFFVer ? USHRT_MAX : 0;
/*N*/ }

SfxPoolItem* SwFmtChain::Create(SvStream& rStrm, USHORT nIVer) const
{
    SwFmtChain *pChain = new SwFmtChain;

    UINT16 nPrevIdx, nNextIdx;
    if( nIVer>0 )
    {
        rStrm   >> nPrevIdx
                >> nNextIdx;

        Sw3IoImp* pIo = Sw3IoImp::GetCurrentIo();
        OSL_ENSURE( pIo || nPrevIdx != IDX_NO_VALUE || nNextIdx != IDX_NO_VALUE,
                "SwFmtChain: kein sw3io: Verkettung nicht moeglich" );
        if( pIo )
        {
            // Wenn hier ein Format schon gelesen wurde, erfolgt die
            // Verkettung hier in der einen Richtung und in InFormat
            // in der anderen Richtung.
            // Wenn das Format noch nicht gefunden wurde, erfolgt die
            // Verkettung wenn das Ziel-Format gelesen wird.
            if( nPrevIdx != IDX_NO_VALUE )
            {
                SwFlyFrmFmt *pPrevFlyFmt =
                    (SwFlyFrmFmt *)pIo->aStringPool.FindCachedFmt( nPrevIdx );
                OSL_ENSURE( pIo->bInsert ||
                        (SwFlyFrmFmt *)pIo->pDoc->FindSpzFrmFmtByName(
                        pIo->aStringPool.Find( nPrevIdx ) ) == pPrevFlyFmt,
                        "falsches Prev-Format gechached?" );
                pChain->SetPrev( pPrevFlyFmt );
            }
            if( nNextIdx != IDX_NO_VALUE )
            {
                SwFlyFrmFmt *pNextFlyFmt =
                    (SwFlyFrmFmt *)pIo->aStringPool.FindCachedFmt( nNextIdx );
                OSL_ENSURE( pIo->bInsert ||
                        (SwFlyFrmFmt *)pIo->pDoc->FindSpzFrmFmtByName(
                        pIo->aStringPool.Find( nNextIdx ) ) == pNextFlyFmt,
                        "falsches Prev-Format gechached?" );
                pChain->SetNext( pNextFlyFmt );
            }
        }
    }

    return pChain;
}

/*N*/ USHORT SwFmtChain::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/   OSL_ENSURE( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/           "SwFmtChain: Gibt es ein neues Fileformat?" );
/*N*/   return SOFFICE_FILEFORMAT_40 < nFFVer ? 1 : USHRT_MAX;
/*N*/ }

SfxPoolItem* SwTextGridItem::Create(SvStream& rStrm, USHORT /*nIVer*/) const
{
    SwTextGridItem* pRet = new SwTextGridItem;
    BOOL bVal;
    rStrm >> bVal;

    return pRet;
}

/*N*/ USHORT SwTextGridItem::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/   OSL_ENSURE( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/             "SwTextGridItem: Gibt es ein neues Fileformat?" );
        (void) nFFVer; // avoid warnings
/*N*/
/*N*/     return USHRT_MAX;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
