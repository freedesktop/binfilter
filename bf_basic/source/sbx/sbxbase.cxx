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

#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <tools/stream.hxx>

#include "sbx.hxx"
#include "sbxfac.hxx"
#include "sbxbase.hxx"

namespace binfilter {

// AppData-Struktur fuer SBX:

SV_IMPL_PTRARR(SbxParams,SbxParamInfo*);
SV_IMPL_PTRARR(SbxFacs,SbxFactory*);

TYPEINIT0(SbxBase)

// SBX-Daten anfordern oder ggf. anlegen
// wir legen den Bereich einfach an und verzichten auf die Freigabe!

SbxAppData* GetSbxData_Impl()
{
    DBG_TESTSOLARMUTEX();
    static SbxAppData* pAppData = new SbxAppData;
    return pAppData;
}

//////////////////////////////// SbxBase /////////////////////////////////

DBG_NAME(SbxBase);

SbxBase::SbxBase()
{
    DBG_CTOR( SbxBase, 0 );
    nFlags  = SBX_READWRITE;
}

SbxBase::SbxBase( const SbxBase& r )
    : SvRefBase( r )
{
    DBG_CTOR( SbxBase, 0 );
    nFlags  = r.nFlags;
}

SbxBase::~SbxBase()
{
    DBG_DTOR(SbxBase,0);
}

SbxBase& SbxBase::operator=( const SbxBase& r )
{
    DBG_CHKTHIS( SbxBase, 0 );
    nFlags = r.nFlags;
    return *this;
}

SbxDataType SbxBase::GetType() const
{
    DBG_CHKTHIS( SbxBase, 0 );
    return SbxEMPTY;
}

SbxClassType SbxBase::GetClass() const
{
    DBG_CHKTHIS( SbxBase, 0 );
    return SbxCLASS_DONTCARE;
}

void SbxBase::Clear()
{
    DBG_CHKTHIS( SbxBase, 0 );
}

BOOL SbxBase::IsFixed() const
{
    DBG_CHKTHIS( SbxBase, 0 );
    return IsSet( SBX_FIXED );
}

void SbxBase::SetModified( BOOL b )
{
    DBG_CHKTHIS( SbxBase, 0 );
    if( IsSet( SBX_NO_MODIFY ) )
        return;
    if( b )
        SetFlag( SBX_MODIFIED );
    else
        ResetFlag( SBX_MODIFIED );
}

SbxError SbxBase::GetError()
{
    return GetSbxData_Impl()->eSbxError;
}

void SbxBase::SetError( SbxError e )
{
    SbxAppData* p = GetSbxData_Impl();
    if( e && p->eSbxError == SbxERR_OK )
        p->eSbxError = e;
}

BOOL SbxBase::IsError()
{
    return BOOL( GetSbxData_Impl()->eSbxError != SbxERR_OK );
}

void SbxBase::ResetError()
{
    GetSbxData_Impl()->eSbxError = SbxERR_OK;
}

void SbxBase::AddFactory( SbxFactory* pFac )
{
    SbxAppData* p = GetSbxData_Impl();
    const SbxFactory* pTemp = pFac;

    // AB, 6.3.96: HandleLast-Flag beruecksichtigen
    USHORT nPos = p->aFacs.Count();     // Einfuege-Position
    if( !pFac->IsHandleLast() )         // Nur, wenn nicht selbst HandleLast
    {
        // Neue Factory vor Factories mit HandleLast einordnen
        while( nPos > 0 &&
                (static_cast<SbxFactory*>(p->aFacs.GetObject( nPos-1 )))->IsHandleLast() )
            nPos--;
    }
    p->aFacs.Insert( pTemp, nPos );
}

void SbxBase::RemoveFactory( SbxFactory* pFac )
{
    SbxAppData* p = GetSbxData_Impl();
    for( USHORT i = 0; i < p->aFacs.Count(); i++ )
    {
        if( p->aFacs.GetObject( i ) == pFac )
        {
            p->aFacs.Remove( i, 1 ); break;
        }
    }
}


SbxBase* SbxBase::Create( UINT16 nSbxId, UINT32 nCreator )
{
    // #91626: Hack to skip old Basic dialogs
    // Problem: There does not exist a factory any more,
    // so we have to create a dummy SbxVariable instead
    if( nSbxId == 0x65 )    // Dialog Id
        return new SbxVariable;

    XubString aEmptyStr;
    if( nCreator == SBXCR_SBX )
      switch( nSbxId )
    {
        case SBXID_VALUE:       return new SbxValue;
        case SBXID_VARIABLE:    return new SbxVariable;
        case SBXID_ARRAY:       return new SbxArray;
        case SBXID_DIMARRAY:    return new SbxDimArray;
        case SBXID_OBJECT:      return new SbxObject( aEmptyStr );
        case SBXID_COLLECTION:  return new SbxCollection( aEmptyStr );
        case SBXID_FIXCOLLECTION:
                                return new SbxStdCollection( aEmptyStr, aEmptyStr );
        case SBXID_METHOD:      return new SbxMethod( aEmptyStr, SbxEMPTY );
        case SBXID_PROPERTY:    return new SbxProperty( aEmptyStr, SbxEMPTY );
    }
    // Unbekanter Typ: ?ber die Factories gehen!
    SbxAppData* p = GetSbxData_Impl();
    SbxBase* pNew = NULL;
    for( USHORT i = 0; i < p->aFacs.Count(); i++ )
    {
        SbxFactory* pFac = p->aFacs.GetObject( i );
        pNew = pFac->Create( nSbxId, nCreator );
        if( pNew )
            break;
    }
    return pNew;
}

SbxObject* SbxBase::CreateObject( const rtl::OUString& rClass )
{
    SbxAppData* p = GetSbxData_Impl();
    SbxObject* pNew = NULL;
    for( USHORT i = 0; i < p->aFacs.Count(); i++ )
    {
        pNew = p->aFacs.GetObject( i )->CreateObject( rClass );
        if( pNew )
            break;
    }
    SAL_WARN_IF(!pNew, "binfilter", "No factory for object class " << rClass);
    return pNew;
}

static BOOL bStaticEnableBroadcasting = TRUE;

// Sbx-Loesung als Ersatz fuer SfxBroadcaster::Enable()
BOOL SbxBase::StaticIsEnabledBroadcasting( void )
{
    return bStaticEnableBroadcasting;
}

SbxBase* SbxBase::Load( SvStream& rStrm )
{
    UINT16 nSbxId, nFlags, nVer;
    UINT32 nCreator, nSize;
    rStrm >> nCreator >> nSbxId >> nFlags >> nVer;

    // Eine Dummheit meinerseits korrigieren:
    if( nFlags & SBX_RESERVED )
        nFlags = ( nFlags & ~SBX_RESERVED ) | SBX_GBLSEARCH;

    ULONG nOldPos = rStrm.Tell();
    rStrm >> nSize;
    SbxBase* p = Create( nSbxId, nCreator );
    if( p )
    {
        p->nFlags = nFlags;
        if( p->LoadData( rStrm, nVer ) )
        {
            ULONG nNewPos = rStrm.Tell();
            nOldPos += nSize;
            DBG_ASSERT( nOldPos >= nNewPos, "SBX: Zu viele Daten eingelesen" );
            if( nOldPos != nNewPos )
                rStrm.Seek( nOldPos );
            if( !p->LoadCompleted() )
            {
                // Loeschen des Objekts
                SbxBaseRef aRef( p );
                p = NULL;
            }
        }
        else
        {
            rStrm.SetError( SVSTREAM_FILEFORMAT_ERROR );
            // Loeschen des Objekts
            SbxBaseRef aRef( p );
            p = NULL;
        }
    }
    else
        rStrm.SetError( SVSTREAM_FILEFORMAT_ERROR );
    return p;
}

// Sbx-Objekt im Stream ueberspringen
void SbxBase::Skip( SvStream& rStrm )
{
    UINT16 nSbxId, nFlags, nVer;
    UINT32 nCreator, nSize;
    rStrm >> nCreator >> nSbxId >> nFlags >> nVer;

    ULONG nStartPos = rStrm.Tell();
    rStrm >> nSize;

    rStrm.Seek( nStartPos + nSize );
}

BOOL SbxBase::LoadData( SvStream&, USHORT )
{
    DBG_CHKTHIS( SbxBase, 0 );
    return FALSE;
}

BOOL SbxBase::LoadPrivateData( SvStream&, USHORT )
{
    DBG_CHKTHIS( SbxBase, 0 );
    return TRUE;
}

BOOL SbxBase::StorePrivateData( SvStream& ) const
{
    DBG_CHKTHIS( SbxBase, 0 );
    return TRUE;
}

BOOL SbxBase::LoadCompleted()
{
    DBG_CHKTHIS( SbxBase, 0 );
    return TRUE;
}

BOOL SbxBase::StoreCompleted()
{
    DBG_CHKTHIS( SbxBase, 0 );
    return TRUE;
}

//////////////////////////////// SbxFactory ////////////////////////////////

SbxBase* SbxFactory::Create( UINT16, UINT32 )
{
    return NULL;
}

SbxObject* SbxFactory::CreateObject( const XubString& )
{
    return NULL;
}

///////////////////////////////// SbxInfo //////////////////////////////////

SbxInfo::~SbxInfo()
{}

void SbxInfo::AddParam
        ( const XubString& rName, SbxDataType eType, USHORT nFlags )
{
    const SbxParamInfo* p = new SbxParamInfo( rName, eType, nFlags );
    aParams.Insert( p, aParams.Count() );
}

BOOL SbxInfo::LoadData( SvStream& rStrm, USHORT nVer )
{
    aParams.Remove( 0, aParams.Count() );
    UINT16 nParam;
    aComment = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rStrm,
        RTL_TEXTENCODING_ASCII_US);
    aHelpFile = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rStrm,
        RTL_TEXTENCODING_ASCII_US);
    rStrm >> nHelpId >> nParam;
    while( nParam-- )
    {
        XubString aName;
        UINT16 nType, nFlags;
        UINT32 nUserData = 0;
        aName = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rStrm,
            RTL_TEXTENCODING_ASCII_US);
        rStrm >> nType >> nFlags;
        if( nVer > 1 )
            rStrm >> nUserData;
        AddParam( aName, (SbxDataType) nType, nFlags );
        SbxParamInfo* p = aParams.GetObject( aParams.Count() - 1 );
        p->nUserData = nUserData;
    }
    return TRUE;
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
