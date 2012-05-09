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

#define _SVSTDARR_STRINGSSORTDTOR

 #ifndef _STREAM_HXX //autogen
 #include <tools/stream.hxx>
 #endif
#include <bf_svx/fontitem.hxx>
#include <bf_svx/eeitem.hxx>

#include <shellio.hxx>

#include <horiornt.hxx>

#include <pam.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <bookmrk.hxx>          // fuer SwBookmark ...
#include <numrule.hxx>

#include <swerror.h>
namespace binfilter {

/*N*/ DECLARE_TABLE( SwBookmarkNodeTable, SvPtrarr* )

/*N*/ struct Writer_Impl
/*N*/ {
/*N*/ 	SvStringsSortDtor *pSrcArr, *pDestArr;
/*N*/ 	SvPtrarr* pFontRemoveLst, *pBkmkArr;
/*N*/ 	SwBookmarkNodeTable* pBkmkNodePos;
/*N*/
/*N*/ 	Writer_Impl();
/*N*/ 	~Writer_Impl();
/*N*/
/*N*/ 	void RemoveFontList( SwDoc& rDoc );
/*N*/ };

/*N*/ Writer_Impl::Writer_Impl()
/*N*/ 	: pSrcArr( 0 ), pDestArr( 0 ), pFontRemoveLst( 0 ), pBkmkNodePos( 0 )
/*N*/ {
/*N*/ }

/*N*/ Writer_Impl::~Writer_Impl()
/*N*/ {
/*N*/ 	delete pSrcArr;
/*N*/ 	delete pDestArr;
/*N*/ 	delete pFontRemoveLst;
/*N*/
/*N*/ 	if( pBkmkNodePos )
/*N*/ 	{
/*?*/ 		for( SvPtrarr* p = pBkmkNodePos->First(); p; p = pBkmkNodePos->Next() )
/*?*/ 			delete p;
/*?*/ 		delete pBkmkNodePos;
/*N*/ 	}
/*N*/ }

/*N*/ void Writer_Impl::RemoveFontList( SwDoc& rDoc )
/*N*/ {
/*N*/ 	OSL_ENSURE( pFontRemoveLst, "wo ist die FontListe?" );
/*N*/ 	for( USHORT i = pFontRemoveLst->Count(); i; )
/*N*/ 	{
/*N*/ 		SvxFontItem* pItem = (SvxFontItem*)(*pFontRemoveLst)[ --i ];
/*N*/ 		rDoc.GetAttrPool().Remove( *pItem );
/*N*/ 	}
/*N*/ }

/*
 * Dieses Modul ist die Zentrale-Sammelstelle fuer alle Write-Filter
 * und ist eine DLL !
 *
 * Damit der Writer mit den unterschiedlichen Writern arbeiten kann,
 * muessen fuer diese die Ausgabe-Funktionen der Inhalts tragenden
 * Objecte auf die verschiedenen Ausgabe-Funktionen gemappt werden.
 *
 * Dazu kann fuer jedes Object ueber den Which-Wert in einen Tabelle ge-
 * griffen werden, um seine Ausgabe-Funktion zu erfragen.
 * Diese Funktionen stehen in den entsprechenden Writer-DLL's.
 */

/*N*/ Writer::Writer()
/*N*/   : pImpl( 0 ), pStrm( 0 ), pOrigPam( 0 ),
/*N*/   pDoc(0), pCurPam(0)
/*N*/ {
/*N*/   bWriteAll = bShowProgress = bUCS2_WithStartChar = TRUE;
/*N*/   bASCII_NoLastLineEnd = bASCII_ParaAsBlanc = bASCII_ParaAsCR =
/*N*/       bWriteClipboardDoc = bWriteOnlyFirstTable = bBlock =
/*N*/       bOrganizerMode = FALSE;
/*N*/ }

/*N*/ Writer::~Writer()
/*N*/ {
/*N*/ }

/*N*/ void Writer::ResetWriter()
/*N*/ {
/*N*/   if( pImpl && pImpl->pFontRemoveLst )
/*N*/       pImpl->RemoveFontList( *pDoc );
/*N*/   delete pImpl, pImpl = 0;
/*N*/
/*N*/   if( pCurPam )
/*N*/   {
/*N*/       while( pCurPam->GetNext() != pCurPam )
/*?*/           delete pCurPam->GetNext();
/*N*/       delete pCurPam;
/*N*/   }
/*N*/   pCurPam = 0;
/*N*/   pDoc = 0;
/*N*/   pStrm = 0;
/*N*/
/*N*/   bShowProgress = bUCS2_WithStartChar = TRUE;
/*N*/   bASCII_NoLastLineEnd = bASCII_ParaAsBlanc = bASCII_ParaAsCR =
/*N*/       bWriteClipboardDoc = bWriteOnlyFirstTable = bBlock =
/*N*/       bOrganizerMode = FALSE;
/*N*/ }

/*N*/ BOOL Writer::CopyNextPam( SwPaM ** ppPam )
/*N*/ {
/*N*/   if( (*ppPam)->GetNext() == pOrigPam )
/*N*/   {
/*N*/       *ppPam = pOrigPam;          // wieder auf den Anfangs-Pam setzen
/*N*/       return FALSE;               // Ende vom Ring
/*N*/   }
/*N*/
/*N*/   // ansonsten kopiere den die Werte aus dem naechsten Pam
/*?*/   *ppPam = ((SwPaM*)(*ppPam)->GetNext() );
/*?*/
/*?*/   *pCurPam->GetPoint() = *(*ppPam)->Start();
/*?*/   *pCurPam->GetMark() = *(*ppPam)->End();
/*?*/
/*?*/   return TRUE;
/*N*/ }

// Stream-spezifisches
/*N*/ #ifdef DBG_UTIL
/*N*/ SvStream& Writer::Strm()
/*N*/ {
/*N*/   OSL_ENSURE( pStrm, "Oh-oh. Dies ist ein Storage-Writer. Gleich knallts!" );
/*N*/   return *pStrm;
/*N*/ }
/*N*/ #endif


/*N*/ ULONG Writer::Write( SwPaM& rPaM, SvStream& rStrm )
/*N*/ {
/*N*/ 	pStrm = &rStrm;
/*N*/ 	pDoc = rPaM.GetDoc();
/*N*/ 	pImpl = new Writer_Impl;
/*N*/
/*N*/ 	// PaM kopieren, damit er veraendert werden kann
/*N*/ 	pCurPam = new SwPaM( *rPaM.End(), *rPaM.Start() );
/*N*/ 	// zum Vergleich auf den akt. Pam sichern
/*N*/ 	pOrigPam = &rPaM;
/*N*/
/*N*/ 	ULONG nRet = WriteStream();
/*N*/
/*N*/ 	ResetWriter();
/*N*/
/*N*/ 	return nRet;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
