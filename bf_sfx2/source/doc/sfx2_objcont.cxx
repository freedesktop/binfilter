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

#include <com/sun/star/uno/Reference.hxx>

#include <com/sun/star/document/UpdateDocMode.hpp>

#include <tools/cachestr.hxx>
#include <bf_svtools/style.hxx>

#include <bf_sfx2/app.hxx>

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <bf_svtools/stritem.hxx>
#include <bf_svtools/intitem.hxx>
#include <bf_svtools/rectitem.hxx>
#include <bf_svtools/urihelper.hxx>
#include <comphelper/processfactory.hxx>

#include <bf_svtools/sfxecode.hxx>
#include <bf_svtools/ehdl.hxx>
#include <tools/datetime.hxx>
#include <math.h>

#include <bf_svtools/saveopt.hxx>
#include <bf_svtools/useroptions.hxx>
#include <unotools/localfilehelper.hxx>
#include <bf_so3/svstor.hxx>

#include "fltfnc.hxx"
#include "docfac.hxx"
#include "cfgmgr.hxx"
#include "objsh.hxx"
#include "objshimp.hxx"
#include "cfgitem.hxx"
#include "evntconf.hxx"
#include "interno.hxx"
#include "printer.hxx"
#include "doctempl.hxx"
#include "appdata.hxx"
#include "sfxbasemodel.hxx"

#include <bf_svtools/imgdef.hxx>

#include "docfile.hxx"
#include "objuno.hxx"
#include "request.hxx"

#include <legacysmgr/legacy_binfilters_smgr.hxx>

#include "bf_basic/basmgr.hxx"

#include "bf_so3/staticbaseurl.hxx"
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

//====================================================================

#define SFX_WINDOWS_STREAM "SfxWindows"
#define SFX_PREVIEW_STREAM "SfxPreview"

//====================================================================

/*?*/ GDIMetaFile* SfxObjectShell::GetPreviewMetaFile( sal_Bool /*bFullContent*/ ) const
/*?*/ { return NULL;
/*?*/ }

//====================================================================

/*N*/ void SfxObjectShell::UpdateDocInfoForSave()
/*N*/ {
/*N*/   if( !pImp->bDoNotTouchDocInfo )
/*N*/   {
/*N*/       SfxDocumentInfo &rDocInfo = GetDocInfo();
/*N*/       rDocInfo.SetTemplateConfig( HasTemplateConfig() );
/*N*/
/*N*/       if ( IsModified() )
/*N*/       {
/*N*/           // Keine Unterschiede mehr zwischen Save, SaveAs
/*N*/           String aUserName = SvtUserOptions().GetFullName();
/*N*/           if ( !rDocInfo.IsUseUserData() )
/*N*/           {
/*N*/               SfxStamp aCreated = rDocInfo.GetCreated();
/*N*/               if ( aUserName == aCreated.GetName() )
/*N*/               {
/*N*/                   aCreated.SetName( String() );
/*N*/                   rDocInfo.SetCreated( aCreated );
/*N*/               }
/*N*/
/*N*/               SfxStamp aPrinted = rDocInfo.GetPrinted();
/*N*/               if ( aUserName == aPrinted.GetName() )
/*N*/               {
/*N*/                   aPrinted.SetName( String() );
/*N*/                   rDocInfo.SetPrinted( aPrinted );
/*N*/               }
/*N*/
/*N*/               aUserName.Erase();
/*N*/           }
/*N*/
/*N*/           rDocInfo.SetChanged( aUserName );
/*N*/           if ( !HasName() || pImp->bIsSaving )
/*N*/               UpdateTime_Impl( rDocInfo );
/*N*/       }
/*N*/
/*N*/       if ( !pImp->bIsSaving )
/*N*/           rDocInfo.SetPasswd( pImp->bPasswd );
/*N*/
/*N*/       Broadcast( SfxDocumentInfoHint( &rDocInfo ) );
/*N*/   }
/*N*/ }

//--------------------------------------------------------------------

// Bearbeitungszeit aktualisieren
/*N*/ SfxDocumentInfo& SfxObjectShell::UpdateTime_Impl(SfxDocumentInfo &rInfo)
/*N*/ {
/*N*/   // Get old time from documentinfo
/*N*/   Time aOldTime(rInfo.GetTime());
/*N*/
/*N*/   // Initialize some local member! Its neccessary for wollow operations!
/*N*/   DateTime    aNow( DateTime::SYSTEM );   // Date and time at current moment
/*N*/   Time        n24Time     (24,0,0,0)  ;   // Time-value for 24 hours - see follow calculation
/*N*/   ULONG       nDays       = 0         ;   // Count of days between now and last editing
/*N*/   Time        nAddTime    (0)         ;   // Value to add on aOldTime
/*N*/
/*N*/   // Safe impossible cases!
/*N*/   // User has changed time to the past between last editing and now ... its not possible!!!
/*N*/   DBG_ASSERT( !(aNow.GetDate()<pImp->nTime.GetDate()), "Timestamp of last change is in the past ?!..." );
/*N*/
/*N*/   // Do the follow only, if user has NOT changed time to the past.
/*N*/   // Else add a time of 0 to aOldTime ... !!!
/*N*/   if (aNow.GetDate()>=pImp->nTime.GetDate())
/*N*/   {
/*N*/       // Get count of days last editing.
/*N*/       nDays = aNow.GetSecFromDateTime(pImp->nTime.GetDate())/86400 ;
/*N*/
/*N*/       if (nDays==0)
/*N*/       {
/*N*/           // If no day between now and last editing - calculate time directly.
/*N*/             nAddTime    =   (const Time&)aNow - (const Time&)pImp->nTime ;
/*N*/       }
/*N*/       else
/*N*/       // If time of working without save greater then 1 month (!) ....
/*N*/       // we add 0 to aOldTime!
/*N*/       if (nDays<=31)
/*N*/       {
/*N*/           // If 1 or up to 31 days between now and last editing - calculate time indirectly.
/*N*/           // nAddTime = (24h - nTime) + (nDays * 24h) + aNow
/*?*/           --nDays;
/*?*/           nAddTime     =  nDays*n24Time.GetTime() ;
/*?*/           nAddTime    +=  n24Time-(const Time&)pImp->nTime        ;
/*?*/           nAddTime    +=  aNow                    ;
/*N*/       }
/*N*/
/*N*/       aOldTime += nAddTime;
/*N*/   }
/*N*/
/*N*/   rInfo.SetTime(aOldTime.GetTime());
/*N*/   pImp->nTime = aNow;
/*N*/   rInfo.IncDocumentNumber();
/*N*/   //! DocumentNummer
/*N*/   return rInfo;
/*N*/ }

//--------------------------------------------------------------------

/*?*/ SvEmbeddedInfoObject* SfxObjectShell::InsertObject
/*?*/ (
/*?*/   SvEmbeddedObject*   /*pObj*/,
/*?*/   const String&       /*rName*/
/*?*/ )
/*?*/ { return NULL;
/*?*/ }

//-------------------------------------------------------------------------

/*N*/ SfxConfigManager* SfxObjectShell::GetConfigManager( BOOL bForceCreation )
/*N*/ {
/*N*/     if ( !pImp->pCfgMgr )
/*N*/     {
/*N*/         if ( bForceCreation || (HasStorage() && SfxConfigManager::HasConfiguration( *GetStorage() )) )
/*N*/       {
/*N*/             pImp->pCfgMgr = new SfxConfigManager( *this );
/*N*/       }
/*N*/     }
/*N*/
/*N*/   return pImp->pCfgMgr;
/*N*/ }

//-------------------------------------------------------------------------

/*N*/ void SfxObjectShell::SetConfigManager(SfxConfigManager *pMgr)
/*N*/ {
/*N*/     if ( pImp->pCfgMgr && pImp->pCfgMgr != pMgr )
/*?*/       delete pImp->pCfgMgr;
/*N*/
/*N*/   pImp->pCfgMgr = pMgr;
/*N*/ }

//-------------------------------------------------------------------------

/*N*/ BOOL SfxObjectShell::HasTemplateConfig() const
/*N*/ {
/*N*/     return FALSE;
/*N*/ }

//--------------------------------------------------------------------

/*N*/ SfxStyleSheetBasePool* SfxObjectShell::GetStyleSheetPool()
/*N*/ {
/*N*/   return 0;
/*N*/ }


//--------------------------------------------------------------------

/*N*/ USHORT SfxObjectShell::GetContentCount(USHORT /*nIdx1*/,
/*N*/                                        USHORT /*nIdx2*/)
/*N*/ { return 0;
/*N*/ }

//--------------------------------------------------------------------

/*N*/ void SfxObjectShell::GetContent(String & /*rText*/,
/*N*/                               Bitmap & /*rClosedBitmap*/,
/*N*/                               Bitmap & /*rOpenedBitmap*/,
/*N*/                               BOOL & /*bCanDel*/,
/*N*/                               USHORT /*i*/,
/*N*/                               USHORT /*nIdx1*/,
/*N*/                               USHORT /*nIdx2*/ )
/*N*/ {
/*N*/ }

//--------------------------------------------------------------------

/*N*/ void SfxObjectShell::LoadStyles
/*N*/ (
    SfxObjectShell &/*rSource*/         /*  die Dokument-Vorlage, aus der
                                            die Styles geladen werden sollen */
/*N*/ )

/*  [Beschreibung]

    Diese Methode wird vom SFx gerufen, wenn aus einer Dokument-Vorlage
    Styles nachgeladen werden sollen. Bestehende Styles soll dabei
    "uberschrieben werden. Das Dokument mu"s daher neu formatiert werden.
    Daher werden die Applikationen in der Regel diese Methode "uberladen
    und in ihrer Implementierung die Implementierung der Basisklasse
    rufen.
*/

/*N*/ {
/*N*/ }

//--------------------------------------------------------------------

/*N*/ SfxEventConfigItem_Impl* SfxObjectShell::GetEventConfig_Impl( BOOL bForce )
/*N*/ {
/*N*/     if ( bForce && !pImp->pEventConfig )
/*N*/     {
/*N*/         pImp->pEventConfig = new SfxEventConfigItem_Impl( SFX_ITEMTYPE_DOCEVENTCONFIG,
/*N*/                     SFX_APP()->GetEventConfig(), this );
/*N*/       if (pImp->pCfgMgr)
/*N*/           pImp->pEventConfig->Connect( pImp->pCfgMgr );
/*N*/       pImp->pEventConfig->Initialize();
/*N*/   }
/*N*/
/*N*/   return pImp->pEventConfig;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
