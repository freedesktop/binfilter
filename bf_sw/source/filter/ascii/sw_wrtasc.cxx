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


#include <tools/stream.hxx>

#include <pam.hxx>

#include <horiornt.hxx>

#include <doc.hxx>
#include <ndtxt.hxx>
#include <mdiexp.hxx>           // ...Percent()
#include <docary.hxx>
#include <fmtcntnt.hxx>
#include <frmfmt.hxx>
#include <wrtasc.hxx>

#include <statstr.hrc>          // ResId fuer Statusleiste
#include <osl/endian.h>
namespace binfilter {

//-----------------------------------------------------------------

/*N*/ SwASCWriter::SwASCWriter( const String& rFltNm )
/*N*/ {
/*N*/   SwAsciiOptions aNewOpts;
/*N*/
/*N*/   switch( 5 <= rFltNm.Len() ? rFltNm.GetChar( 4 ) : 0 )
/*N*/   {
/*?*/   case 'D':
/*?*/ #if !defined(PM2)
/*?*/               aNewOpts.SetCharSet( RTL_TEXTENCODING_IBM_850 );
/*?*/               aNewOpts.SetParaFlags( LINEEND_CRLF );
/*?*/ #endif
/*?*/               if( 5 < rFltNm.Len() )
/*?*/                   switch( rFltNm.Copy( 5 ).ToInt32() )
/*?*/                   {
/*?*/                   case 437: aNewOpts.SetCharSet( RTL_TEXTENCODING_IBM_437 );  break;
/*?*/                   case 850: aNewOpts.SetCharSet( RTL_TEXTENCODING_IBM_850 );  break;
/*?*/                   case 860: aNewOpts.SetCharSet( RTL_TEXTENCODING_IBM_860 );  break;
/*?*/                   case 861: aNewOpts.SetCharSet( RTL_TEXTENCODING_IBM_861 );  break;
/*?*/                   case 863: aNewOpts.SetCharSet( RTL_TEXTENCODING_IBM_863 );  break;
/*?*/                   case 865: aNewOpts.SetCharSet( RTL_TEXTENCODING_IBM_865 );  break;
/*?*/                   }
/*?*/               break;
/*?*/
/*?*/   case 'A':
/*?*/ #if !defined(WIN) && !defined(WNT)
/*?*/               aNewOpts.SetCharSet( RTL_TEXTENCODING_MS_1252 );
/*?*/               aNewOpts.SetParaFlags( LINEEND_CRLF );
/*?*/ #endif
/*?*/               break;
/*?*/
/*?*/   case 'M':
/*?*/ #if !defined(MAC)
/*?*/               aNewOpts.SetCharSet( RTL_TEXTENCODING_APPLE_ROMAN );
/*?*/               aNewOpts.SetParaFlags( LINEEND_CR );
/*?*/ #endif
/*?*/               break;
/*?*/
/*?*/   case 'X':
/*?*/ #if !defined(UNX)
/*?*/               aNewOpts.SetCharSet( RTL_TEXTENCODING_MS_1252 );
/*?*/               aNewOpts.SetParaFlags( LINEEND_LF );
/*?*/ #endif
/*?*/               break;
/*?*/
/*N*/   default:
/*N*/       if( rFltNm.Copy( 4 ).EqualsAscii( "_DLG" ))
/*N*/       {
/*N*/           // use the options
/*N*/           aNewOpts = GetAsciiOptions();
/*N*/       }
/*N*/   }
/*N*/   SetAsciiOptions( aNewOpts );
/*N*/ }


/*N*/ ULONG SwASCWriter::WriteStream()
/*N*/ {
/*N*/ 	sal_Char cLineEnd[ 3 ];
/*N*/ 	sal_Char* pCEnd = cLineEnd;
/*N*/ 	if( bASCII_ParaAsCR )			// falls vorgegeben ist.
/*?*/ 		*pCEnd++ = '\015';
/*N*/ 	else if( bASCII_ParaAsBlanc )
/*?*/ 		*pCEnd++ = ' ';
/*N*/ 	else
/*N*/ 		switch( GetAsciiOptions().GetParaFlags() )
/*N*/ 		{
/*?*/ 		case LINEEND_CR:    *pCEnd++ = '\015'; break;
/*?*/ 		case LINEEND_LF:	*pCEnd++ = '\012'; break;
/*N*/ 		case LINEEND_CRLF:	*pCEnd++ = '\015', *pCEnd++ = '\012'; break;
/*N*/ 		}
/*N*/ 	*pCEnd = 0;
/*N*/
/*N*/ 	sLineEnd.AssignAscii( cLineEnd );
/*N*/
/*N*/ 	long nMaxNode = pDoc->GetNodes().Count();
/*N*/
/*N*/ 	if( bShowProgress )
/*N*/ 		::binfilter::StartProgress( STR_STATSTR_W4WWRITE, 0, nMaxNode, pDoc->GetDocShell() );
/*N*/
/*N*/ 	SwPaM* pPam = pOrigPam;
/*N*/
/*N*/ 	BOOL bWriteSttTag = bUCS2_WithStartChar &&
/*M*/         (RTL_TEXTENCODING_UCS2 == GetAsciiOptions().GetCharSet() ||
/*M*/ 		RTL_TEXTENCODING_UTF8 == GetAsciiOptions().GetCharSet());
/*N*/
/*N*/ 	rtl_TextEncoding eOld = Strm().GetStreamCharSet();
/*N*/ 	Strm().SetStreamCharSet( GetAsciiOptions().GetCharSet() );
/*N*/
/*N*/ 	// gebe alle Bereich des Pams in das ASC-File aus.
/*N*/ 	do {
/*N*/ 		BOOL bTstFly = TRUE;
/*N*/ 		while( pCurPam->GetPoint()->nNode.GetIndex() < pCurPam->GetMark()->nNode.GetIndex() ||
/*N*/ 			  (pCurPam->GetPoint()->nNode.GetIndex() == pCurPam->GetMark()->nNode.GetIndex() &&
/*N*/ 			   pCurPam->GetPoint()->nContent.GetIndex() <= pCurPam->GetMark()->nContent.GetIndex()) )
/*N*/ 		{
/*N*/ 			SwTxtNode* pNd = pCurPam->GetPoint()->nNode.GetNode().GetTxtNode();
/*N*/ 			if( pNd )
/*N*/ 			{
/*N*/ 				// sollten nur Rahmen vorhanden sein?
/*N*/ 				// (Moeglich, wenn Rahmen-Selektion ins Clipboard
/*N*/ 				// gestellt wurde)
/*N*/ 				if( bTstFly && bWriteAll &&
/*N*/ 					// keine Laenge
/*N*/ 					!pNd->GetTxt().Len() &&
/*N*/ 					// Rahmen vorhanden
/*N*/ 					pDoc->GetSpzFrmFmts()->Count() &&
/*N*/ 					// nur ein Node im Array
/*N*/ 					pDoc->GetNodes().GetEndOfExtras().GetIndex() + 3 ==
/*N*/ 					pDoc->GetNodes().GetEndOfContent().GetIndex() &&
/*N*/ 					// und genau der ist selektiert
/*N*/ 					pDoc->GetNodes().GetEndOfContent().GetIndex() - 1 ==
/*N*/ 					pCurPam->GetPoint()->nNode.GetIndex() )
/*N*/ 				{
/*N*/ 					// dann den Inhalt vom Rahmen ausgeben.
/*N*/ 					// dieser steht immer an Position 0 !!
/*?*/ 					SwFrmFmt* pFmt = (*pDoc->GetSpzFrmFmts())[ 0 ];
/*?*/ 					const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
/*?*/ 					if( pIdx )
/*?*/ 					{
/*?*/ 						DBG_BF_ASSERT(0, "STRIP"); //STRIP001 delete pCurPam;
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else
/*M*/ 				{
/*M*/ 					if (bWriteSttTag)
/*M*/ 					{
/*M*/                         switch(GetAsciiOptions().GetCharSet())
/*M*/                         {
/*M*/                             case RTL_TEXTENCODING_UTF8:
/*M*/                                 Strm() << BYTE(0xEF) << BYTE(0xBB) <<
/*M*/                                     BYTE(0xBF);
/*M*/                                 break;
/*M*/                             case RTL_TEXTENCODING_UCS2:
/*M*/ 						        //Strm().StartWritingUnicodeText();
/*M*/                                 Strm().SetEndianSwap(FALSE);
/*M*/ #ifdef OSL_LITENDIAN
/*M*/                                 Strm() << BYTE(0xFF) << BYTE(0xFE);
/*M*/ #else
/*M*/                                 Strm() << BYTE(0xFE) << BYTE(0xFF);
/*M*/ #endif
/*M*/                                 break;
/*M*/
/*M*/                         }
/*?*/ 						bWriteSttTag = FALSE;
/*N*/ 					}
/*N*/ 					Out( aASCNodeFnTab, *pNd, *this );
/*N*/ 				}
/*N*/ 				bTstFly = FALSE;		// eimal Testen reicht
/*N*/ 			}
/*N*/
/*N*/ 			if( !pCurPam->Move( fnMoveForward, fnGoNode ) )
/*N*/ 				break;
/*N*/
/*N*/ 			if( bShowProgress )
/*N*/ 				::binfilter::SetProgressState( pCurPam->GetPoint()->nNode.GetIndex(),
/*N*/ 									pDoc->GetDocShell() );   // Wie weit ?
/*N*/
/*N*/ 		}
/*N*/ 	} while( CopyNextPam( &pPam ) );		// bis alle Pam bearbeitet
/*N*/
/*N*/ 	Strm().SetStreamCharSet( eOld );
/*N*/
/*N*/ 	if( bShowProgress )
/*N*/ 		::binfilter::EndProgress( pDoc->GetDocShell() );
/*N*/
/*N*/ 	return 0;
/*N*/ }


/*N*/ SwASCWriter::~SwASCWriter() {}

/*N*/ WriterRef GetASCWriter( const String& rFltNm )
/*N*/ {
/*N*/   return new SwASCWriter( rFltNm );
/*N*/ }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
