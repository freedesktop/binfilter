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

#ifndef _RTL_USTRING_
#include <rtl/ustring.h>
#endif



#include "macrconf.hxx"
#include "objsh.hxx"
#include "eventsupplier.hxx"

#include "sfxsids.hrc"
#include "app.hxx"
#include <vector>

#include <legacysmgr/legacy_binfilters_smgr.hxx>
namespace binfilter {

// -----------------------------------------------------------------------

#define PROPERTYVALUE           ::com::sun::star::beans::PropertyValue
#define XNAMEREPLACE            ::com::sun::star::container::XNameReplace
#define XEVENTSSUPPLIER         ::com::sun::star::document::XEventsSupplier
#define ANY                     ::com::sun::star::uno::Any
#define REFERENCE               ::com::sun::star::uno::Reference
#define SEQUENCE                ::com::sun::star::uno::Sequence
#define UNO_QUERY               ::com::sun::star::uno::UNO_QUERY

#define OUSTRING                ::rtl::OUString

// -----------------------------------------------------------------------

static const USHORT nCompatVersion = 3;
static const USHORT nOldVersion = 4;
static const USHORT nVersion = 5;

/*N*/ TYPEINIT1(SfxEventHint, SfxHint);

// class SfxEventList_Impl -----------------------------------------------

/*N*/ struct EventNames_Impl
/*N*/ {
/*N*/     USHORT  mnId;
/*N*/     String  maEventName;
/*N*/     String  maUIName;
/*N*/
/*N*/             EventNames_Impl( USHORT nId,
/*N*/                              const String& rEventName,
/*N*/                              const String& rUIName )
/*N*/                 : mnId( nId )
/*N*/                 , maEventName( rEventName )
/*N*/                 , maUIName( rUIName ) {}
/*N*/ };

typedef ::std::vector< EventNames_Impl* > SfxEventList_Impl;
SfxEventList_Impl   *gp_Id_SortList = NULL;
SfxEventList_Impl   *gp_Name_SortList = NULL;

//==========================================================================

/*N*/ SfxEventConfiguration::SfxEventConfiguration()
/*N*/  : pAppEventConfig( NULL )
/*N*/  , pDocEventConfig( NULL )
/*N*/ {
/*N*/     bIgnoreConfigure = sal_False;
/*N*/
/*N*/     // Array zum Ermitteln der Bindungen
/*N*/     pEventArr = new SfxEventArr_Impl;
/*N*/
/*N*/     // Einen default entry eingf"ugen
/*N*/     const SfxEvent_Impl *pEvent = new SfxEvent_Impl(String(), 0);
/*N*/     pEventArr->Insert(pEvent, 0);
/*N*/ }

//==========================================================================

SfxEventConfiguration::~SfxEventConfiguration()
{
    for (USHORT n=0; n<pEventArr->Count(); n++)
        delete (*pEventArr)[n];

    delete pEventArr;
    delete pAppEventConfig;

    if ( gp_Id_SortList )
    {
        for ( size_t i = 0, n = gp_Id_SortList->size(); i < n; ++i )
            delete (*gp_Id_SortList)[ i ];
        gp_Id_SortList->clear();

        delete gp_Id_SortList;
        delete gp_Name_SortList;

        gp_Id_SortList = NULL;
        gp_Name_SortList = NULL;
    }
}

//==========================================================================

void SfxEventConfiguration::RegisterEvent(USHORT nId, const String& rName)
{
    USHORT nCount = pEventArr->Count();
    const SfxEvent_Impl *pEvent = new SfxEvent_Impl(rName, nId);
    pEventArr->Insert(pEvent, nCount);
}

//==========================================================================

/*N*/ SfxEventConfigItem_Impl::SfxEventConfigItem_Impl( USHORT nConfigId,
/*N*/     SfxEventConfiguration *pCfg,
/*N*/     SfxObjectShell *pObjSh)
/*N*/     : SfxConfigItem( nConfigId, pObjSh ? NULL : SFX_APP()->GetConfigManager_Impl() )
/*N*/     , aMacroTable( 2, 2 )
/*N*/     , pEvConfig( pCfg )
/*N*/     , pObjShell( pObjSh )
/*N*/   , bInitialized( FALSE )
/*N*/ {
/*N*/   bInitialized = TRUE;
/*N*/ }

//==========================================================================

/*N*/ int SfxEventConfigItem_Impl::Load(SvStream& rStream)
/*N*/ {
/*N*/     USHORT nFileVersion;
/*N*/     rStream >> nFileVersion;
/*N*/     if ( nFileVersion < nCompatVersion || nFileVersion > nVersion )
/*?*/         return SfxConfigItem::WARNING_VERSION;
/*N*/
/*N*/     SvxMacroTableDtor aLocalMacroTable;
/*N*/     if ( nFileVersion <= nOldVersion )
/*N*/     {
/*?*/         if ( nFileVersion > nCompatVersion )
/*?*/         {
/*?*/             USHORT nWarn;
/*?*/             rStream >> nWarn;
/*?*/             bWarning = ( nWarn & 0x01 ) != 0;
/*?*/             bAlwaysWarning = ( nWarn & 0x02 ) != 0;
/*?*/         }
/*?*/         else
/*?*/             bWarning = bAlwaysWarning = FALSE;
/*?*/
/*?*/         USHORT nCount, nEventCount = pEvConfig->GetEventCount();
/*?*/         rStream >> nCount;
/*?*/
/*?*/         USHORT i;
/*?*/         for (i=0; i<nCount; i++)
/*?*/         {
/*?*/           USHORT nId;
/*?*/             SfxMacroInfo aInfo( pObjShell );
/*?*/             rStream >> nId >> aInfo;
/*?*/
/*?*/             for (USHORT n=0; n<nEventCount; n++)
/*?*/             {
/*?*/               USHORT nEventId = (*pEvConfig->pEventArr)[n+1]->nEventId;
/*?*/                 if ( nEventId == nId )
/*?*/                 {
/*?*/                   SvxMacro *pMacro = new SvxMacro( String(), String(), STARBASIC );
/*?*/                   aLocalMacroTable.Insert( nEventId, pMacro );
/*?*/                     break;
/*?*/                 }
/*?*/             }
/*?*/         }
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         USHORT nWarn;
/*N*/         rStream >> nWarn;
/*N*/         bWarning = ( nWarn & 0x01 ) != 0;
/*N*/         bAlwaysWarning = ( nWarn & 0x02 ) != 0;
/*N*/         aLocalMacroTable.Read( rStream );
/*N*/     }
/*N*/
/*N*/     if ( pObjShell && pEvConfig )
/*N*/         pEvConfig->PropagateEvents_Impl( pObjShell, aLocalMacroTable );
/*N*/
/*N*/     return SfxConfigItem::ERR_OK;
/*N*/ }

//==========================================================================

/*?*/ String SfxEventConfigItem_Impl::GetStreamName() const
/*?*/ {
/*?*/     return SfxConfigItem::GetStreamName( GetType() );
/*?*/ }

//==========================================================================

/*N*/ void SfxEventConfigItem_Impl::UseDefault()
/*N*/ {
/*N*/     bWarning = TRUE;
/*N*/     bAlwaysWarning = FALSE;
/*N*/
/*N*/     aMacroTable.DelDtor();
/*N*/     SetDefault( TRUE );
/*N*/     pEvConfig->PropagateEvents_Impl( pObjShell, aMacroTable );
/*N*/ }

int SfxEventConfigItem_Impl::Load( SotStorage& /*rStorage*/ )
{
    DBG_BF_ASSERT(0, "STRIP");  // VIRTUAL
    return 0;
}

//==========================================================================

/*N*/ void SfxEventConfigItem_Impl::ConfigureEvent( USHORT nId, SvxMacro *pMacro )
/*N*/ {
/*N*/     if ( aMacroTable.Seek( nId ) )
/*N*/     {
/*?*/         if ( pMacro )
/*?*/             aMacroTable.Replace( nId, pMacro );
/*?*/         else
/*?*/             aMacroTable.Remove( nId );
/*N*/     }
/*N*/     else if ( pMacro )
/*N*/         aMacroTable.Insert( nId, pMacro );
/*N*/     SetDefault(FALSE);
/*N*/ }

//--------------------------------------------------------------------------
/*N*/ void SfxEventConfiguration::PropagateEvents_Impl( SfxObjectShell *pDoc,
/*N*/     const SvxMacroTableDtor& rTable )
/*N*/ {
/*N*/     REFERENCE< XEVENTSSUPPLIER > xSupplier;
/*N*/   if ( pDoc )
/*N*/   {
/*N*/       xSupplier = REFERENCE< XEVENTSSUPPLIER >( pDoc->GetModel(), UNO_QUERY );
/*N*/   }
/*N*/   else
/*N*/   {
/*N*/       xSupplier = REFERENCE< XEVENTSSUPPLIER >
/*N*/                 ( ::legacy_binfilters::getLegacyProcessServiceFactory()->createInstance(
/*N*/                         ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.GlobalEventBroadcaster" ))), UNO_QUERY );
/*N*/   }
/*N*/
/*N*/     if ( xSupplier.is() )
/*N*/     {
/*N*/         SvxMacro   *pMacro;
/*N*/         ULONG       nCount;
/*N*/         ULONG       nID, i;
/*N*/
/*N*/         REFERENCE< XNAMEREPLACE > xEvents = xSupplier->getEvents();
/*N*/
/*N*/         bIgnoreConfigure = sal_True;
/*N*/
/*N*/         // Erase old values first, because we don't know anything about the
/*N*/         // changes here
/*N*/
/*N*/         SEQUENCE < PROPERTYVALUE > aProperties;
/*N*/         SEQUENCE < OUSTRING > aEventNames = xEvents->getElementNames();
/*N*/         OUSTRING*   pNames  = aEventNames.getArray();
/*N*/         ANY aEmpty;
/*N*/
/*N*/         aEmpty <<= aProperties;
/*N*/         nCount  = aEventNames.getLength();
/*N*/
/*N*/         for ( i=0; i<nCount; i++ )
/*N*/         {
/*N*/             try
/*N*/             {
/*N*/                 xEvents->replaceByName( pNames[i], aEmpty );
/*N*/             }
/*N*/             catch( ::com::sun::star::lang::IllegalArgumentException )
/*N*/             { DBG_ERRORFILE( "PropagateEvents_Impl: caught IllegalArgumentException" ); }
/*N*/             catch( ::com::sun::star::container::NoSuchElementException )
/*N*/             { DBG_ERRORFILE( "PropagateEvents_Impl: caught NoSuchElementException" ); }
/*N*/         }
/*N*/
/*N*/         // now set the new values
/*N*/
/*N*/         nCount = rTable.Count();
/*N*/
/*N*/         for ( i=0; i<nCount; i++ )
/*N*/         {
/*N*/             pMacro = rTable.GetObject( i );
/*N*/             nID = rTable.GetObjectKey( i );
/*N*/             OUSTRING aEventName = GetEventName_Impl( nID );
/*N*/
/*N*/             if ( aEventName.getLength() )
/*N*/             {
/*N*/                 ANY aEventData = CreateEventData_Impl( pMacro );
/*N*/                 try
/*N*/                 {
/*N*/                     xEvents->replaceByName( aEventName, aEventData );
/*N*/                 }
/*N*/                 catch( ::com::sun::star::lang::IllegalArgumentException )
/*N*/                 { DBG_ERRORFILE( "PropagateEvents_Impl: caught IllegalArgumentException" ); }
/*N*/                 catch( ::com::sun::star::container::NoSuchElementException )
/*N*/                 { DBG_ERRORFILE( "PropagateEvents_Impl: caught NoSuchElementException" ); }
/*N*/             }
/*N*/             else
/*N*/                 DBG_WARNING( "PropagateEvents_Impl: Got unkown event" );
/*N*/         }
/*N*/
/*N*/         bIgnoreConfigure = sal_False;
/*N*/     }
/*N*/ }

//--------------------------------------------------------------------------

/*N*/ ANY SfxEventConfiguration::CreateEventData_Impl( const SvxMacro *pMacro )
/*N*/ {
/*
    This function converts a SvxMacro into an Any containing three
    properties. These properties are EventType and Script. Possible
    values for EventType ar StarBasic, JavaScript, ...
    The Script property should contain the URL to the macro and looks
    like "macro://./standard.module1.main()"
*/
/*N*/     ANY aEventData;
/*N*/
/*N*/     if ( pMacro )
/*N*/     {
/*N*/         if ( pMacro->GetScriptType() == STARBASIC )
/*N*/         {
/*N*/             SEQUENCE < PROPERTYVALUE > aProperties(3);
/*N*/             PROPERTYVALUE  *pValues = aProperties.getArray();
/*N*/
/*N*/             OUSTRING    aType   (RTL_CONSTASCII_USTRINGPARAM( STAR_BASIC ));
/*N*/             OUSTRING    aLib    = pMacro->GetLibName();
/*N*/             OUSTRING    aMacro  = pMacro->GetMacName();
/*N*/
/*N*/             pValues[ 0 ].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( PROP_EVENT_TYPE ));
/*N*/             pValues[ 0 ].Value <<= aType;
/*N*/
/*N*/             pValues[ 1 ].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( PROP_LIBRARY ));
/*N*/             pValues[ 1 ].Value <<= aLib;
/*N*/
/*N*/             pValues[ 2 ].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( PROP_MACRO_NAME ));
/*N*/             pValues[ 2 ].Value <<= aMacro;
/*N*/
/*N*/             aEventData <<= aProperties;
/*N*/         }
/*N*/         else if ( pMacro->GetScriptType() == EXTENDED_STYPE )
/*N*/         {
/*?*/             SEQUENCE < PROPERTYVALUE > aProperties(2);
/*?*/             PROPERTYVALUE  *pValues = aProperties.getArray();
/*?*/
/*?*/             OUSTRING    aLib    = pMacro->GetLibName();
/*?*/             OUSTRING    aMacro  = pMacro->GetMacName();
/*?*/
/*?*/             pValues[ 0 ].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( PROP_EVENT_TYPE ));
/*?*/             pValues[ 0 ].Value <<= aLib;
/*?*/
/*?*/             pValues[ 1 ].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( PROP_SCRIPT ));
/*?*/             pValues[ 1 ].Value <<= aMacro;
/*?*/
/*?*/             aEventData <<= aProperties;
/*N*/       }
/*N*/         else if ( pMacro->GetScriptType() == JAVASCRIPT )
/*N*/         {
/*?*/             SEQUENCE < PROPERTYVALUE > aProperties(2);
/*?*/             PROPERTYVALUE  *pValues = aProperties.getArray();
/*?*/
/*?*/             OUSTRING    aMacro  = pMacro->GetMacName();
/*?*/
/*?*/             pValues[ 0 ].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( PROP_EVENT_TYPE ));
/*?*/             pValues[ 0 ].Value <<= ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SVX_MACRO_LANGUAGE_JAVASCRIPT));
/*?*/
/*?*/             pValues[ 1 ].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( PROP_MACRO_NAME ));
/*?*/             pValues[ 1 ].Value <<= aMacro;
/*?*/
/*?*/             aEventData <<= aProperties;
/*N*/       }
/*N*/         else
/*N*/         {
/*N*/             DBG_ERRORFILE( "CreateEventData_Impl(): ScriptType not supported!");
/*N*/         }
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         SEQUENCE < PROPERTYVALUE > aProperties;
/*N*/         aEventData <<= aProperties;
/*N*/     }
/*N*/
/*N*/     return aEventData;
/*N*/ }

// -------------------------------------------------------------------------------------------------------
/*N*/ ULONG SfxEventConfiguration::GetPos_Impl( USHORT nId, sal_Bool &rFound )
/*N*/ {
/*N*/     rFound = sal_False;
/*N*/
/*N*/     if ( gp_Id_SortList->empty() )
/*N*/         return 0;
/*N*/
/*N*/     // use binary search to find the correct position
/*N*/     // in the list
/*N*/
/*N*/     int     nCompVal = 1;
/*N*/     long    nStart = 0;
/*N*/     long    nEnd = gp_Id_SortList->size() - 1;
/*N*/     long    nMid(0);
/*N*/
/*N*/     EventNames_Impl* pMid;
/*N*/
/*N*/     rFound = sal_False;
/*N*/
/*N*/     while ( nCompVal && ( nStart <= nEnd ) )
/*N*/     {
/*N*/         nMid = ( nEnd - nStart ) / 2 + nStart;
/*N*/         pMid = (*gp_Id_SortList)[ nMid ];
/*N*/
/*N*/         nCompVal = pMid->mnId - nId;
/*N*/
/*N*/         if ( nCompVal < 0 )     // pMid < pData
/*N*/             nStart = nMid + 1;
/*N*/         else
/*N*/             nEnd = nMid - 1;
/*N*/     }
/*N*/
/*N*/     if ( nCompVal == 0 )
/*N*/     {
/*N*/         rFound = sal_True;
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         if ( nCompVal < 0 )     // pMid < pData
/*N*/             nMid++;
/*N*/     }
/*N*/
/*N*/     return (ULONG) nMid;
/*N*/ }

// -------------------------------------------------------------------------------------------------------
/*N*/ ULONG SfxEventConfiguration::GetPos_Impl( const String& rName, sal_Bool &rFound )
/*N*/ {
/*N*/     rFound = sal_False;
/*N*/
/*N*/     if ( gp_Name_SortList->empty() )
/*N*/         return 0;
/*N*/
/*N*/     // use binary search to find the correct position
/*N*/     // in the list
/*N*/
/*N*/     int     nCompVal = 1;
/*N*/     long    nStart = 0;
/*N*/     long    nEnd = gp_Name_SortList->size() - 1;
/*N*/     long    nMid(0);
/*N*/
/*N*/     EventNames_Impl* pMid;
/*N*/
/*N*/     rFound = sal_False;
/*N*/
/*N*/     while ( nCompVal && ( nStart <= nEnd ) )
/*N*/     {
/*N*/         nMid = ( nEnd - nStart ) / 2 + nStart;
/*N*/         pMid = (*gp_Name_SortList)[ nMid ];
/*N*/
/*N*/         nCompVal = rName.CompareTo( pMid->maEventName );
/*N*/
/*N*/         if ( nCompVal < 0 )     // pMid < pData
/*N*/             nStart = nMid + 1;
/*N*/         else
/*N*/             nEnd = nMid - 1;
/*N*/     }
/*N*/
/*N*/     if ( nCompVal == 0 )
/*N*/     {
/*N*/         rFound = sal_True;
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         if ( nCompVal < 0 )     // pMid < pData
/*N*/             nMid++;
/*N*/     }
/*N*/
/*N*/     return (ULONG) nMid;
/*N*/ }

//--------------------------------------------------------------------------------------------------------
/*N*/ OUSTRING SfxEventConfiguration::GetEventName_Impl( ULONG nID )
/*N*/ {
/*N*/     OUSTRING    aRet;
/*N*/
/*N*/     if ( gp_Id_SortList )
/*N*/     {
/*N*/         sal_Bool    bFound;
/*N*/         ULONG       nPos = GetPos_Impl( (USHORT) nID, bFound );
/*N*/
/*N*/         if ( bFound )
/*N*/         {
/*N*/             EventNames_Impl *pData = (*gp_Id_SortList)[  nPos ];
/*N*/             aRet = pData->maEventName;
/*N*/         }
/*N*/     }
/*N*/
/*N*/     return aRet;
/*N*/ }

//--------------------------------------------------------------------------------------------------------
/*N*/ ULONG SfxEventConfiguration::GetEventId_Impl( const OUSTRING& rEventName )
/*N*/ {
/*N*/     ULONG nRet = 0;
/*N*/
/*N*/     if ( gp_Name_SortList )
/*N*/     {
/*N*/         sal_Bool    bFound;
/*N*/         ULONG       nPos = GetPos_Impl( rEventName, bFound );
/*N*/
/*N*/         if ( bFound )
/*N*/         {
/*N*/             EventNames_Impl *pData = (*gp_Name_SortList)[ nPos ];
/*N*/             nRet = pData->mnId;
/*N*/         }
/*N*/     }
/*N*/
/*N*/     return nRet;
/*N*/ }

// -------------------------------------------------------------------------------------------------------
/*N*/ void SfxEventConfiguration::RegisterEvent( USHORT nId,
/*N*/                                            const String& rUIName,
/*N*/                                            const String& rMacroName )
/*N*/ {
/*N*/     if ( ! gp_Id_SortList )
/*N*/     {
/*N*/         gp_Id_SortList = new SfxEventList_Impl;
/*N*/         gp_Name_SortList = new SfxEventList_Impl;
/*N*/     }
/*N*/
/*N*/     sal_Bool    bFound = sal_False;
/*N*/     ULONG       nPos = GetPos_Impl( nId, bFound );
/*N*/
/*N*/     if ( bFound )
/*N*/     {
/*?*/         DBG_ERRORFILE( "RegisterEvent: Event already registered?" );
/*?*/         return;
/*N*/     }
/*N*/
/*N*/     EventNames_Impl *pData;
/*N*/
/*N*/     pData = new EventNames_Impl( nId, rMacroName, rUIName );
          SfxEventList_Impl::iterator itId = gp_Id_SortList->begin();
          ::std::advance( itId, nPos );
          gp_Id_SortList->insert( itId, pData );
/*N*/
/*N*/     nPos = GetPos_Impl( rMacroName, bFound );
/*N*/     DBG_ASSERT( !bFound, "RegisterEvent: Name in List, but ID not?" );
/*N*/
          SfxEventList_Impl::iterator itName = gp_Name_SortList->begin();
          ::std::advance( itName, nPos );
          gp_Name_SortList->insert( itName, pData );
/*N*/
/*N*/     SFX_APP()->GetEventConfig()->RegisterEvent( nId, rUIName );
/*N*/ }

/*N*/ BOOL SfxEventConfiguration::Import( SvStream& rInStream, SvStream* pOutStream, SfxObjectShell* pDoc )
/*N*/ {
/*N*/     if ( pDoc )
/*N*/     {
/*N*/         // load events, they are automatically propagated to the document
/*N*/         DBG_ASSERT( !pOutStream, "DocEventConfig must not be converted!" );
/*N*/         (void)pOutStream;
/*N*/         SfxEventConfigItem_Impl* pCfg = pDoc->GetEventConfig_Impl( TRUE );
/*N*/       if ( pCfg )
/*N*/           return ( pCfg->Load( rInStream ) == SfxConfigItem::ERR_OK );
/*N*/       OSL_FAIL("Couldn't create EventConfiguration!");
/*N*/       return FALSE;
/*N*/     }
/*?*/
/*?*/     OSL_FAIL( "No OutStream!" );
/*?*/     return FALSE;
/*N*/ }



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
