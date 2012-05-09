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

#include "hintids.hxx"          //Damit "unsere" Attribute angezogen werden.

#include <bf_svtools/sfxecode.hxx>
#include <bf_svx/lrspitem.hxx>
#include <tools/tenccvt.hxx>
#include <swmodule.hxx>

#include <fmtcntnt.hxx>
#include <frmatr.hxx>

#include <horiornt.hxx>

#include "doc.hxx"
#include "swrect.hxx"
#include "swerror.h"
#include "pam.hxx"
#include "rootfrm.hxx"
#include "pagedesc.hxx"
#include "ndtxt.hxx"
#include "ndole.hxx"
#include "sw3imp.hxx"
namespace binfilter {

// Wird von der GUI-Seite gesetzt, wenn Layouts ignoriert werden sollen

//////////////////////////////////////////////////////////////////////////////


void lcl_sw3doc_ChgChartName( SwDoc* pDoc )
{
    // bei "alten" Dokumenten muss das an eine Tabelle gebundene
    // Chart-Object sonderbehandelt werden. Die Verbindung von
    // Tabelle zum ChartObject erfolgt weiterhin ueber den Namen,
    // aber der wird nicht mehr im FlyFormat, sondern direkt
    // beim OLE-Node gespeichert. FlyFrames koennen jetzt benannt
    // werden (Name steht im FlyFormat!)

    // durch alle Tabellen
    for( USHORT n = pDoc->GetTblFrmFmts()->Count(); n; )
    {
        const String& rTblNm = (*pDoc->GetTblFrmFmts())[ --n ]->GetName();
        for( USHORT i = pDoc->GetSpzFrmFmts()->Count(); i; )
        {
            // und durch alle Flys
            SwFmt *pFmt = (*pDoc->GetSpzFrmFmts())[ --i ];
            if( RES_FLYFRMFMT == pFmt->Which() && pFmt->GetName() == rTblNm )
            {
                const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
                SwOLENode* pNd;
                if( pIdx && 0 != ( pNd =
                    pDoc->GetNodes()[ pIdx->GetIndex() + 1 ]->GetOLENode() ))
                {
                    pNd->SetChartTblName( rTblNm );
                    pFmt->SetName( pDoc->GetUniqueOLEName() );
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

// Laden eines Dokumentinhalts. Das kann ein Textbaustein oder ein
// normales Dokument sein. Die Vorlagen sind bereits eingelesen.


/*N*/ void Sw3IoImp::LoadDocContents( SwPaM* pPaM )
/*N*/ {
/*N*/   // Wenn ein PaM angegeben ist, wird an einer bestimmten
/*N*/   // Position eingefuegt, also ist Insert = TRUE
/*N*/   short nDiff = 0;
/*N*/   BYTE nInsFirstPara = 0;
/*N*/   if( pPaM )
/*N*/   {
/*?*/       bInsert = TRUE;
/*?*/       SwTxtNode *pTxtNode=pDoc->GetNodes()[pPaM->GetPoint()->nNode]->GetTxtNode();
/*?*/       // Ist das ein SwPaM auf einen TextNode?
/*?*/       if( !pTxtNode )
/*?*/       {
/*?*/           // Ist keiner, dann machen wir uns einen
/*?*/           pTxtNode=pDoc->GetNodes().MakeTxtNode(pPaM->GetPoint()->nNode,
/*?*/                    (SwTxtFmtColl*) pDoc->GetDfltTxtFmtColl() );
/*?*/           // Da zeigt jetzt auch der StartIdx hin
/*?*/           nDiff = -1;
/*?*/       }
/*?*/       else if( pTxtNode->GetTxt().Len() )
/*?*/           nInsFirstPara = 1;
/*N*/   }
/*N*/
/*N*/   // aktuelles Passwort retten
/*N*/   sal_Char cOldPass[ 16 ];
/*N*/   memcpy( cOldPass, cPasswd, 16 );
/*N*/   InHeader( TRUE );
/*N*/   if( nFileFlags & SWGF_BAD_FILE )
/*N*/   {
/*?*/       Error( ERR_SWG_READ_ERROR );
/*?*/       return;
/*N*/   }
/*N*/
/*N*/   // Auf gehts:
/*N*/   BOOL bNode1 = TRUE;
/*N*/   BOOL bDone = BOOL( !Good() );
/*N*/   xub_StrLen nOffset = 0;
/*N*/   SwNodeIndex aPos( pDoc->GetNodes().GetEndOfContent(), -1 );
/*N*/   if( pPaM )
/*?*/       aPos = pPaM->GetPoint()->nNode.GetIndex() + nDiff,
/*?*/       nOffset = pPaM->GetPoint()->nContent.GetIndex();
/*N*/   pCurPaM = pPaM;
/*N*/
/*N*/   SwDBData aOldData(pDoc->_GetDBDesc());
/*N*/
/*N*/   // Normales Lesen?
/*N*/   while( !bDone )
/*N*/   {
/*N*/       BYTE cType = Peek();
/*N*/       if( !Good() || pStrm->IsEof() )
/*N*/           bDone = TRUE;
/*N*/       else switch( cType )
/*N*/       {
/*N*/           case SWG_EOF:
/*N*/               bDone = TRUE; break;
/*N*/           case SWG_DBNAME:
/*N*/               // der DatenbankName
/*N*/               if( bNormal && !bInsert )
/*N*/                   InDBName();
/*N*/               else if( bInsert &&
/*N*/                        !IsVersion( SWG_MULTIDB, SWG_EXPORT31, SWG_DESKTOP40 ) )
/*N*/               {
/*?*/                   // in Versionen ohne mehrfache Datenbanken wird hier
/*?*/                   // der DB-Name gelesen. Warum nicht auch das
/*?*/                   // SQL-Statemement?
/*?*/                   OpenRec( SWG_DBNAME );
/*?*/
/*?*/                   // TODO: unicode: what about 0xff?
/*?*/                   String sStr;
/*?*/                   InString( *pStrm, sStr );
/*?*/                   SwDBData aData;
/*?*/                   aData.sDataSource = sStr.GetToken(0, DB_DELIM);
/*?*/                   aData.sCommand = sStr.GetToken(1, DB_DELIM);
/*?*/                   pDoc->ChgDBData( aData );
/*?*/
/*?*/                   ULONG nSaveWarn = nWarn;
/*?*/                   CloseRec( SWG_DBNAME );
/*?*/                   nWarn = nSaveWarn;
/*N*/               }
/*N*/               else
/*?*/                   SkipRec();
/*N*/               break;
/*N*/           case SWG_STRINGPOOL:
/*N*/               InStringPool( cType, aStringPool );
/*N*/               break;
/*N*/           case SWG_PASSWORD:
                    InPasswd(); break;
/*N*/           case SWG_JOBSETUP:
/*N*/               if( bNormal && !bInsert ) InJobSetup();
/*N*/               else SkipRec(); break;
/*N*/           case SWG_CONFIG:
/*N*/               SkipRec();
/*N*/               break;
/*?*/           case SWG_PGPREVIEWPRTDATA:
/*?*/               if( bNormal && !bInsert )
                        InPagePreViewPrintData();
/*?*/               else
/*?*/                   SkipRec();
/*?*/               break;
/*N*/
/*N*/ //            case SWG_NUMRULE:
/*N*/               // Der 4.0-SW kommt hier auch schon mit NumRules
/*N*/               // zurecht, ignoriert sie aber. Es weiss zwar keiner
/*N*/               // warum das so ist, aber wir behalten es mal lieber bei.
/*N*/           case SWG_OUTLINE:
/*N*/               if( bNormal && !(bInsert || bBlock) )
/*N*/               {
/*N*/                   // Dieser Record steht nur in der 3.1 und 4.0 hier.
/*N*/                   // Ab der 5.0 steht er in einem eigenen Stream.
/*N*/                   SwNumRule* pRule = InNumRule( cType );
/*N*/                   if( pRule )
/*N*/                   {
/*N*/                       pDoc->SetOutlineNumRule( *pRule );
/*N*/                   }
/*N*/                   delete pRule;
/*N*/               }
/*N*/               else
/*N*/               {
/*?*/                   SkipRec();
/*N*/               }
/*N*/               break;
/*?*/           case SWG_MACROTBL:
                    if( bNormal ) InMacroTbl(); else SkipRec();
/*?*/               break;
/*N*/           case SWG_DICTIONARY:
/*N*/               if( bNormal && IsVersion( SWG_DESKTOP40 ) )
/*N*/                   InDictionary();
/*N*/               else
/*?*/                   SkipRec();
/*N*/               break;
/*N*/           case SWG_FIELDTYPE:
/*N*/               if( bNormal ) InFieldType(); else SkipRec();
/*N*/               break;
/*N*/           case SWG_FLYFRAMES:
/*N*/               if( bNormal ) InFlyFrames(); else SkipRec();
/*N*/               break;
/*N*/           case SWG_FOOTINFO:
/*N*/               // globale Fussnoten-Info
/*N*/               if( bNormal && !(bInsert || bBlock) )
/*N*/                   InFtnInfo();
/*N*/               else
/*?*/                   SkipRec();
/*N*/               break;
/*N*/           case SWG_ENDNOTEINFO:
/*N*/               // globale Endnoten-Info
/*N*/               if( bNormal && !(bInsert || bBlock) )
/*N*/                   InEndNoteInfo();
/*N*/               else
/*?*/                   SkipRec();
/*N*/               break;
/*N*/           case SWG_LINENUMBERINFO:
/*N*/               // Zeilennumer-Info
/*N*/               if( bNormal && !(bInsert || bBlock) )
/*N*/                   InLineNumberInfo();
/*N*/               else
/*N*/                   SkipRec();
/*N*/               break;
/*N*/           case SWG_TOXDESCS51:
/*N*/               if( !IsVersion(SWG_NEWTOX) )
/*N*/                   InTOXs51();
/*N*/               else
/*N*/                   SkipRec();
/*N*/               break;
/*N*/           case SWG_TOXDESCS:
/*N*/               OSL_ENSURE( IsVersion(SWG_NEWTOX),
/*N*/                       "new tox descs within a old doc" );
/*N*/               InTOXs();
/*N*/               break;
/*N*/           case SWG_BOOKMARKS:
/*N*/               InBookmarks();
/*N*/               break;
/*N*/           case SWG_REDLINES:
                    InRedlines(); //SW50.SDW
/*?*/               break;
/*N*/           case SWG_CONTENTS:
/*N*/               if( bNormal )
/*N*/                   InContents( aPos, nOffset, bNode1, nInsFirstPara );
/*N*/               else
/*?*/                   SkipRec();
/*N*/               bNode1 = FALSE; nOffset = 0;
/*N*/               nInsFirstPara = 0;
/*N*/               break;
/*N*/           case SWG_DOCSTAT:
/*N*/               if( bNormal && !bInsert )
/*N*/                   InDocStat();
/*N*/               else
/*?*/                   SkipRec();
/*N*/               break;
/*N*/           case SWG_NUMBERFORMATTER:
/*?*/               InNumberFormatter();
/*?*/               break;
/*N*/           case SWG_DOCDUMMIES:
/*N*/               if( bNormal && !bInsert )
/*N*/                   InDocDummies();
/*N*/               else
/*?*/                   SkipRec();
/*N*/               break;
/*N*/           case SWG_LAYOUTINFO:
/*N*/               SkipRec();
/*N*/               break;
/*N*/
/*N*/ #ifdef TEST_HUGE_DOCS
/*N*/           case SWG_TESTHUGEDOCS:
/*N*/               InHugeRecord();
/*N*/ #endif
/*N*/           default:
/*N*/               SkipRec();
/*N*/       }
/*N*/   }
/*N*/
/*N*/   // Insert TOXs if not done this already.
/*N*/   ConnectPageDescAttrs();
/*N*/   ConnectTOXs();
/*N*/
/*N*/   Cleanup( FALSE );
/*N*/
/*N*/   if( !IsVersion(SWG_LONGIDX) && pCurNumRange )
/*?*/       CloseNumRange40( aPos );
/*N*/
/*N*/   if( bInsert && !IsVersion( SWG_MULTIDB, SWG_EXPORT31, SWG_DESKTOP40 ) )
/*?*/       pDoc->ChgDBData( aOldData );
/*N*/
/*N*/   if( !nRes )
/*N*/   {
/*N*/       if( bNormal )
/*N*/       {
/*N*/           // Neues Doc gelesen
/*N*/           if( !bInsert )
/*N*/           {
/*N*/               // bei "alten" Dokumenten muss das an eine Tabelle gebundene
/*N*/               // Chart-Object sonderbehandelt werden. Die Verbindung von
/*N*/               // Tabelle zum ChartObject erfolgt weiterhin ueber den Namen,
/*N*/               // aber der wird nicht mehr im FlyFormat, sondern direkt
/*N*/               // beim OLE-Node gespeichert. FlyFrames koennen jetzt benannt
/*N*/               // werden (Name steht im FlyFormat!)
/*N*/               if( nVersion < SWG_OLENAME && pDoc->GetTblFrmFmts()->Count() &&
/*N*/                   pDoc->GetSpzFrmFmts()->Count() )
                        lcl_sw3doc_ChgChartName( pDoc );
/*N*/
/*N*/               pDoc->SetLoaded( TRUE );
/*N*/           }
/*N*/       }
/*N*/       else
/*N*/       {
/*N*/           // Nachladen von Vorlagen etc.
/*N*/           // falls wir die OutlineLevel veraendert haben (Vorlagen geladen),
/*N*/           // dann muss am Doc ein Update auf die neuen Nummern erfolgen !!
/*?*/           if( !bAdditive && pDoc->GetOutlineNumRule() )
/*?*/               pDoc->SetOutlineNumRule( *pDoc->GetOutlineNumRule() );
/*?*/           pDoc->SetModified();
/*N*/       }
/*N*/   }
/*N*/   pDoc->SetNewDoc( FALSE );
/*N*/   if ( nVersion < SWG_OLEVIS2PAGE )
/*N*/   {
/*N*/       //Bei alten Dokumenten schaetzen wir: Wenn die Standardpage
/*N*/       //Seitenraender von 0 hat, so wirds wohl ein Ole sein.
/*N*/       //MA: Ole wird jetzt per Browse dargestellt.
/*?*/       SwPageDesc &rDesc = pDoc->_GetPageDesc( 0 );
/*?*/       const SvxLRSpaceItem &rLR = rDesc.GetMaster().GetLRSpace();
/*?*/       if ( !rLR.GetLeft() && !rLR.GetRight() )
/*?*/           pDoc->SetBrowseMode( TRUE );
/*N*/   }
/*N*/ }

//////////////////////////////////////////////////////////////////////////////

// Laden des Doc-Headers.

// BYTE[6]      Signatur fuer den harten Patch
// BYTE         0
// BYTE         Laenge der globalen Daten (z.Zt. 48 oder 112)
// INT16        Format-Version
// INT16        Globale Flags
// INT32        Persistente Flags vom Document
//              0x0001 == bOleVis2Page, MA 20. Mar. 98 gibt es nicht mehr, wird
//                                      auf BrowseMode gemappt.
// INT32 x 3    Dummys
// BYTE x 16    Passwort
// BYTE         CharSet letzter Save
// BYTE         GUIType letzer Save (Info)
// INT32        Datum letzter Save (Info)
// INT32        Uhrzeit letzter Save (Info)
// BYTE[64]     Langer Name des Textbausteins, falls SWGF_BLOCKNAME aktiv

/*N*/ BOOL Sw3IoImp::CheckHeader( sal_Char *sHeader )
/*N*/ {
/*N*/   return strcmp( sHeader, SW5HEADER ) == 0 ||
/*N*/          strcmp( sHeader, SW4HEADER ) == 0 ||
/*N*/          strcmp( sHeader, SW3HEADER ) == 0;
/*N*/ }

/*N*/ void Sw3IoImp::InHeader( BOOL bReadRecSizes )
/*N*/ {
/*N*/   BYTE cLen, cSet, cGUI;
/*N*/   sal_Char cHdrSign[ 8 ];
/*N*/   Reset2();
/*N*/   OutputMode( FALSE );
/*N*/   pStrm->Seek( 0L );
/*N*/   if( pStrm->Read( cHdrSign, 7 ) == 7 )
/*N*/   {
/*N*/       if( !CheckHeader(cHdrSign) )
/*N*/       {
/*N*/           Error( ERR_SW6_NOWRITER_FILE );
/*N*/           return;
/*N*/       }
/*N*/   }
/*N*/   else
/*N*/       return;
/*N*/
/*N*/   *pStrm >> cLen;
/*N*/   ULONG nOld = pStrm->Tell();
/*N*/
/*N*/   UINT32 nRecSzPos;
/*N*/   INT32 nDocFlags,
/*N*/         nDummy;
/*N*/   BYTE cRedlineMode;
/*N*/   INT8 nCompatVer,
/*N*/        nDummy8;
/*N*/
/*N*/   *pStrm >> nVersion >> nFileFlags >> nDocFlags >> nRecSzPos >> nDummy
/*N*/          >> nDummy8 >> nDummy8 >> cRedlineMode >> nCompatVer;
/*N*/   // Ist die daeti von einem zu neuen SW und laesst sich nicht mehr lesen?
/*N*/   // Die Kompatibilitaets-Version von aelteren File-Formaten muss
/*N*/   // natuerlich ignoriert werden.
/*N*/   if( IsVersion(SWG_MAJORVERSION) && nCompatVer > SWG_CVERSION )
/*N*/   {
/*N*/       Error( ERR_SWG_NEW_VERSION ); return; // Datei ist von neuerem SW
/*N*/   }
/*N*/   pStrm->Read( cPasswd, 16L );
/*N*/   *pStrm >> cSet >> cGUI >> nDate >> nTime;
/*N*/   eSrcSet = GetSOLoadTextEncoding( (rtl_TextEncoding) cSet );
/*N*/   pStrm->SetStreamCharSet( eSrcSet );
/*N*/   if( nFileFlags & SWGF_BLOCKNAME )
/*N*/   {
/*?*/       sal_Char cBuf[ 64 ];
/*?*/       if( pStrm->Read( cBuf, 64 ) != 64 )
/*?*/           pStrm->SetError( SVSTREAM_FILEFORMAT_ERROR );
/*?*/       aBlkName = String( cBuf, eSrcSet );
/*N*/   }
/*N*/   ULONG nNew = pStrm->Tell();
/*N*/   nOld += cLen;
/*N*/   if( nOld != nNew )
/*?*/       pStrm->Seek( nOld );
/*N*/
/*N*/ #ifdef MAC
/*N*/   if( pStrm != &pPageStyles && pStrm != &pNumRules )
/*N*/ #else
/*N*/   if( pStrm != pPageStyles && pStrm != &pNumRules )
/*N*/ #endif
/*N*/   {
/*N*/       // Flags werden nur gesetzt, wenn der
/*N*/       // Contents-Stream eingelesen wird.
/*N*/       if( nFileFlags & SWGF_HAS_PGNUMS )
/*?*/           pDoc->SetPageNums();
/*N*/       if( !bInsert )
/*N*/       {
/*N*/           if( IsVersion(SWG_OLEVIS2PAGE) )
/*N*/               pDoc->SetBrowseMode ( 0 != (nDocFlags & 0x0001) ||
/*N*/                                     0 != (nDocFlags & 0x0002) );
/*N*/
/*N*/           pDoc->SetHTMLMode( 0 != (nDocFlags & 0x0004) );
/*N*/           pDoc->SetHeadInBrowse( 0 != (nDocFlags & 0x0008) );
/*N*/           pDoc->SetFootInBrowse( 0 != (nDocFlags & 0x0010) );
/*N*/           pDoc->SetGlobalDoc( 0 != (nDocFlags & 0x0020) );
/*N*/           pDoc->SetGlblDocSaveLinks( 0 != (nDocFlags & 0x0040) );
/*N*/           pDoc->SetLabelDoc( 0 != (nDocFlags & 0x0080) );
/*N*/       }
/*N*/   }
/*N*/   if( bNormal && !bInsert && IsVersion(SWG_LONGIDX) )
/*N*/       pDoc->SetRedlineMode_intern( (SwRedlineMode)cRedlineMode );
/*N*/   if( !CheckPasswd() )
/*?*/       Error( ERRCODE_SFX_WRONGPASSWORD );
/*N*/
/*N*/   // MIB: Das <= fuer das 40-FF ist korrekt, denn es gab eine Weile
/*N*/   // 4.0-Storages mit 5.0-Streams.
/*N*/ #ifdef DBG_UTIL
/*N*/   if( !bBlock )
/*N*/       OSL_ENSURE( ( pRoot->GetVersion() == SOFFICE_FILEFORMAT_31 &&
/*N*/                 nVersion >= SWG_MAJORVERSION_30 &&
/*N*/                 nVersion < SWG_MAJORVERSION_40 ) ||
/*N*/               ( pRoot->GetVersion() == SOFFICE_FILEFORMAT_40 &&
/*N*/                 nVersion >= SWG_MAJORVERSION_40 &&
/*N*/                 nVersion < SWG_MAJORVERSION_50 ) ||
/*N*/               ( pRoot->GetVersion() == SOFFICE_FILEFORMAT_50 &&
/*N*/                 nVersion >= SWG_MAJORVERSION_50 ),
/*N*/               "Beim Lesen stimmt die FF-Version am Storage nicht!" );
/*N*/ #endif
/*N*/
/*N*/   if( nRecSzPos!= 0 && bReadRecSizes && !nRes && IsVersion(SWG_RECSIZES) )
                    InRecSizes( nRecSzPos );
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
