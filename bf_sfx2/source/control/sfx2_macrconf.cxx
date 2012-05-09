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

#include "macrconf.hxx"
#include "sfx.hrc"
#include "app.hxx"
#include "objshimp.hxx"

namespace binfilter {

static const sal_uInt16 nCompatVersion = 2;
static const sal_uInt16 nVersion = 3;

// Static member
SfxMacroConfig* SfxMacroConfig::pMacroConfig = NULL;

/*N*/ void SfxMacroConfig::Release_Impl()
/*N*/ {
/*N*/   ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
/*N*/   DELETEZ( pMacroConfig );
/*N*/ }

//==========================================================================

/*?*/ struct SfxMacroConfig_Impl
/*?*/ {
/*?*/   SfxMacroInfoArr_Impl    aArr;
/*?*/   sal_uInt32                  nEventId;
/*?*/   sal_Bool                    bWaitingForCallback;
/*?*/
/*?*/                           SfxMacroConfig_Impl()
/*?*/                           : nEventId( 0 )
/*?*/                           , bWaitingForCallback( sal_False )
/*?*/                           {}
/*?*/ };

//==========================================================================

/*?*/ SfxMacroInfo::SfxMacroInfo( SfxObjectShell *pDoc ) :
/*?*/   pHelpText(0),
/*?*/   nRefCnt(0),
/*?*/   bAppBasic(pDoc == NULL),
/*?*/   nSlotId(0)
/*?*/ {}

//==========================================================================

/*?*/ SfxMacroInfo::~SfxMacroInfo()
/*?*/ {
/*?*/   delete pHelpText;
/*?*/ }

//==========================================================================

/*?*/ SvStream& operator >> (SvStream& rStream, SfxMacroInfo& rInfo)
/*?*/ {
/*?*/   sal_uInt16 nAppBasic, nFileVersion;
/*?*/   String aDocName;
/*?*/
/*?*/   rStream >> nFileVersion;
/*?*/   if ( nVersion < nCompatVersion )
/*?*/   {
/*?*/       // In der 1.Version ohne Versionskennung
/*?*/       nAppBasic = nVersion;
/*?*/       nFileVersion = 1;
/*?*/       aDocName = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rStream, RTL_TEXTENCODING_UTF8);
/*?*/       rInfo.aLibName = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rStream, RTL_TEXTENCODING_UTF8);
/*?*/       rInfo.aModuleName = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rStream, RTL_TEXTENCODING_UTF8);
/*?*/       rInfo.aMethodName = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rStream, RTL_TEXTENCODING_UTF8);
/*?*/   }
/*?*/   else
/*?*/   {
/*?*/       String aInput;
/*?*/       rStream >> nAppBasic;
/*?*/       aDocName = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rStream, RTL_TEXTENCODING_UTF8);                 // Vorsicht: kann bei AppName Unsinn sein!
/*?*/       rInfo.aLibName = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rStream, RTL_TEXTENCODING_UTF8);
/*?*/       rInfo.aModuleName = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rStream, RTL_TEXTENCODING_UTF8);
/*?*/       aInput = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rStream, RTL_TEXTENCODING_UTF8);
/*?*/
/*?*/       if ( nFileVersion == nCompatVersion )
/*?*/           rInfo.aMethodName = aInput;
/*?*/       else
/*?*/       {
/*?*/           sal_uInt16 nCount = aInput.GetTokenCount('.');
/*?*/           rInfo.aMethodName = aInput.GetToken( nCount-1, '.' );
/*?*/           if ( nCount > 1 )
/*?*/               rInfo.aModuleName = aInput.GetToken( nCount-2, '.' );
/*?*/           if ( nCount > 2 )
/*?*/               rInfo.aLibName = aInput.GetToken( 0, '.' );
/*?*/       }
/*?*/   }
/*?*/
/*?*/   rInfo.bAppBasic = (sal_Bool) nAppBasic;
/*?*/   return rStream;
/*?*/ }

//==========================================================================

/*?*/ SvStream& operator << (SvStream& rStream, const SfxMacroInfo& rInfo)
/*?*/ {
/*?*/   if ( rInfo.bAppBasic )
/*?*/   {
/*?*/       rStream << nVersion
/*?*/               << (sal_uInt16) rInfo.bAppBasic;
/*?*/       write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rStream, rInfo.aLibName, RTL_TEXTENCODING_UTF8);
/*?*/       write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rStream, rInfo.aModuleName, RTL_TEXTENCODING_UTF8);
/*?*/       write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rStream, rInfo.aMethodName, RTL_TEXTENCODING_UTF8);
/*?*/   }
/*?*/   else
/*?*/   {
/*?*/       rStream << nVersion
/*?*/               << (sal_uInt16) rInfo.bAppBasic;
/*?*/       write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rStream, SFX_APP()->GetName(), RTL_TEXTENCODING_UTF8);
/*?*/       write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rStream, rInfo.aLibName, RTL_TEXTENCODING_UTF8);
/*?*/       write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rStream, rInfo.aModuleName, RTL_TEXTENCODING_UTF8);
/*?*/       write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rStream, rInfo.aMethodName, RTL_TEXTENCODING_UTF8);
/*?*/   }
/*?*/
/*?*/   return rStream;
/*?*/ }

//==========================================================================

/*?*/ SfxMacroConfig::~SfxMacroConfig()
/*?*/ {
/*?*/   if ( pImp->nEventId )
/*?*/       Application::RemoveUserEvent( pImp->nEventId );
/*?*/   delete pImp;
/*?*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
