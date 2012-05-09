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

// INCLUDE ---------------------------------------------------------------

#include <bf_svtools/zforlist.hxx>

#include "ddelink.hxx"
#include "document.hxx"
#include "scmatrix.hxx"
#include "rechead.hxx"
#include "bf_sc.hrc"
#include "hints.hxx"
namespace binfilter {

/*N*/ TYPEINIT2(ScDdeLink,::binfilter::SvBaseLink,SfxBroadcaster);

#ifdef PM2
#define DDE_TXT_ENCODING    RTL_TEXTENCODING_IBM_850
#else
#define DDE_TXT_ENCODING    RTL_TEXTENCODING_MS_1252
#endif

/*N*/ BOOL ScDdeLink::bIsInUpdate = FALSE;

//------------------------------------------------------------------------

/*N*/ ScDdeLink::ScDdeLink( ScDocument* pD, const String& rA, const String& rT, const String& rI,
/*N*/                       BYTE nM ) :
/*N*/   ::binfilter::SvBaseLink(::binfilter::LINKUPDATE_ALWAYS,FORMAT_STRING),
/*N*/   pDoc( pD ),
/*N*/   aAppl( rA ),
/*N*/   aTopic( rT ),
/*N*/   aItem( rI ),
/*N*/   nMode( nM ),
/*N*/   bNeedUpdate( FALSE ),
/*N*/   pResult( NULL )
/*N*/ {
/*N*/ }

/*N*/ ScDdeLink::~ScDdeLink()
/*N*/ {
/*N*/   // Verbindung aufheben
/*N*/
/*N*/   delete pResult;
/*N*/ }

/*N*/ ScDdeLink::ScDdeLink( ScDocument* pD, SvStream& rStream, ScMultipleReadHeader& rHdr ) :
/*N*/   ::binfilter::SvBaseLink(::binfilter::LINKUPDATE_ALWAYS,FORMAT_STRING),
/*N*/   pDoc( pD ),
/*N*/   bNeedUpdate( FALSE ),
/*N*/   pResult( NULL )
/*N*/ {
/*N*/   rHdr.StartEntry();
/*N*/
/*N*/   rtl_TextEncoding eCharSet = rStream.GetStreamCharSet();
/*N*/   aAppl = rStream.ReadUniOrByteString( eCharSet );
/*N*/   aTopic = rStream.ReadUniOrByteString( eCharSet );
/*N*/   aItem = rStream.ReadUniOrByteString( eCharSet );
/*N*/
/*N*/   BOOL bHasValue;
/*N*/   rStream >> bHasValue;
/*N*/   if ( bHasValue )
/*N*/       pResult = new ScMatrix( rStream );
/*N*/
/*N*/   if (rHdr.BytesLeft())       // neu in 388b und der 364w (RealTime-Client) Version
/*N*/       rStream >> nMode;
/*N*/   else
/*N*/       nMode = SC_DDE_DEFAULT;
/*N*/
/*N*/   rHdr.EndEntry();
/*N*/ }

/*N*/ void ScDdeLink::DataChanged( const String& /*rMimeType*/,
/*N*/                               const ::com::sun::star::uno::Any & /*rValue*/ )
/*N*/ {
/*?*/   DBG_BF_ASSERT(0, "STRIP");  // VIRTUAL
/*N*/ }




/*N*/ void ScDdeLink::TryUpdate()
/*N*/ {
/*N*/   if (bIsInUpdate)
/*N*/       bNeedUpdate = TRUE;         // kann jetzt nicht ausgefuehrt werden
/*N*/   else
/*N*/   {
/*N*/       bIsInUpdate = TRUE;
/*N*/       //Application::Reschedule();    //! OS/2-Simulation
/*N*/         pDoc->IncInDdeLinkUpdate();
/*N*/       Update();
/*N*/         pDoc->DecInDdeLinkUpdate();
/*N*/       bIsInUpdate = FALSE;
/*N*/       bNeedUpdate = FALSE;
/*N*/   }
/*N*/ }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
