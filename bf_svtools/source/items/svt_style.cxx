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

#define _SVSTDARR_STRINGS
#define _SVSTDARR_STRINGSSORTDTOR
#define _SVSTDARR_BYTESTRINGS
#define _SVSTDARR_BYTESTRINGSSORTDTOR

#include <tools/tenccvt.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/intlwrapper.hxx>
#include <bf_svtools/smplhint.hxx>
#include <bf_svtools/poolitem.hxx>
#include <bf_svtools/itemset.hxx>
#include <bf_svtools/itempool.hxx>
#include <poolio.hxx>
#include <bf_svtools/filerec.hxx>
#include <bf_svtools/itemiter.hxx>
#include <bf_svtools/style.hxx>
#include <bf_svtools/svstdarr.hxx>
#include <vcl/svapp.hxx>

#define STYLESTREAM             "SfxStyleSheets"
#define STYLESTREAM_VERSION     USHORT(50)

namespace binfilter
{


TYPEINIT0(SfxStyleSheetBase)

TYPEINIT3(SfxStyleSheet, SfxStyleSheetBase, SfxListener, SfxBroadcaster)


//=========================================================================

TYPEINIT1(SfxStyleSheetHint, SfxHint);
TYPEINIT1(SfxStyleSheetHintExtended, SfxStyleSheetHint);
TYPEINIT1(SfxStyleSheetPoolHint, SfxHint);

SfxStyleSheetHintExtended::SfxStyleSheetHintExtended
(
    USHORT              nAction,        // SFX_STYLESHEET_... (s.o.)
    const String&       rOldName,
    SfxStyleSheetBase&  rStyleSheet     // geh"ort weiterhin dem Aufrufer
)
:   SfxStyleSheetHint( nAction, rStyleSheet ),
    aName( rOldName )
{}

//-------------------------------------------------------------------------

SfxStyleSheetHint::SfxStyleSheetHint
(
    USHORT              nAction,        // SFX_STYLESHEET_... (s.o.)
    SfxStyleSheetBase&  rStyleSheet     // geh"ort weiterhin dem Aufrufer
)
:   pStyleSh( &rStyleSheet ),
    nHint( nAction )
{}

//=========================================================================

class SfxStyleSheetBasePool_Impl
{
  public:
    SfxStyles aStyles;
    SfxStyleSheetIterator *pIter;
    SfxStyleSheetBasePool_Impl() : pIter(0){}
    ~SfxStyleSheetBasePool_Impl(){delete pIter;}
};


//////////////////////////// SfxStyleSheetBase ///////////////////////////////

// Konstruktoren

SfxStyleSheetBase::SfxStyleSheetBase
    ( const XubString& rName, SfxStyleSheetBasePool& r, SfxStyleFamily eFam, USHORT mask )
    : rPool( r )
    , nFamily( eFam )
    , aName( rName )
    , aParent()
    , aFollow( rName )
    , pSet( NULL )
    , nMask(mask)
    , nHelpId( 0 )
    , bMySet( FALSE )
{
}

SfxStyleSheetBase::SfxStyleSheetBase( const SfxStyleSheetBase& r )
    : rPool( r.rPool )
    , nFamily( r.nFamily )
    , aName( r.aName )
    , aParent( r.aParent )
    , aFollow( r.aFollow )
    , aHelpFile( r.aHelpFile )
    , nMask( r.nMask )
    , nHelpId( r.nHelpId )
    , bMySet( r.bMySet )
{
    if( r.pSet )
        pSet = bMySet ? new SfxItemSet( *r.pSet ) : r.pSet;
    else
        pSet = NULL;
}

SfxStyleSheetBase::~SfxStyleSheetBase()
{
    if( bMySet )
    {
        delete pSet;
        pSet = 0;
    }
}

USHORT SfxStyleSheetBase::GetVersion() const
{
    return 0x0000;
}

// Namen aendern

const XubString& SfxStyleSheetBase::GetName() const
{
    return aName;
}

BOOL SfxStyleSheetBase::SetName( const XubString& rName )
{
    if(rName.Len() == 0)
        return FALSE;
    if( aName != rName )
    {
        String aOldName = aName;
        SfxStyleSheetBase *pOther = rPool.Find( rName, nFamily ) ;
        if ( pOther && pOther != this )
            return FALSE;

        SfxStyleFamily eTmpFam=rPool.GetSearchFamily();
        USHORT nTmpMask=rPool.GetSearchMask();

        rPool.SetSearchMask(nFamily);

        if ( aName.Len() )
            rPool.ChangeParent( aName, rName, FALSE );
        if ( aFollow.Equals( aName ) )
            aFollow = rName;
        aName = rName;
        rPool.SetSearchMask(eTmpFam, nTmpMask);
        rPool.Broadcast( SfxStyleSheetHintExtended(
            SFX_STYLESHEET_MODIFIED, aOldName, *this ) );
    }
    return TRUE;
}

// Parent aendern

const XubString& SfxStyleSheetBase::GetParent() const
{
    return aParent;
}

BOOL SfxStyleSheetBase::SetParent( const XubString& rName )
{
    if ( rName == aName )
        return FALSE;

    if( aParent != rName )
    {
        SfxStyleSheetBase* pIter = rPool.Find(rName, nFamily);
        if( rName.Len() && !pIter )
        {
            OSL_FAIL( "StyleSheet-Parent nicht gefunden" );
            return FALSE;
        }
        // rekursive Verknuepfungen verhindern
        if( aName.Len() )
            while(pIter)
            {
                if(pIter->GetName() == aName && aName != rName)
                    return FALSE;
                pIter = rPool.Find(pIter->GetParent(), nFamily);
            }
        aParent = rName;
    }
    rPool.Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_MODIFIED, *this ) );
    return TRUE;
}

// Follow aendern

const XubString& SfxStyleSheetBase::GetFollow() const
{
    return aFollow;
}

BOOL SfxStyleSheetBase::SetFollow( const XubString& rName )
{
    if( aFollow != rName )
    {
        if( !rPool.Find( rName, nFamily ) )
        {
            OSL_FAIL( "StyleSheet-Follow nicht gefunden" );
            return FALSE;
        }
        aFollow = rName;
    }
    rPool.Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_MODIFIED, *this ) );
    return TRUE;
}

// Itemset setzen. Die Dflt-Implementation legt ein neues Set an.

SfxItemSet& SfxStyleSheetBase::GetItemSet()
{
    if( !pSet )
    {
        pSet = new SfxItemSet( rPool.GetPool() );
        bMySet = TRUE;
    }
    return *pSet;
}

// Hilfe-Datei und -ID setzen und abfragen

ULONG SfxStyleSheetBase::GetHelpId( String& rFile )
{
    rFile = aHelpFile;
    return nHelpId;
}

void SfxStyleSheetBase::SetHelpId( const String& rFile, ULONG nId )
{
    aHelpFile = rFile;
    nHelpId = nId;
}

// Folgevorlage m"oglich? Default: Ja

BOOL SfxStyleSheetBase::HasFollowSupport() const
{
    return TRUE;
}

// Basisvorlage m"oglich? Default: Ja

BOOL SfxStyleSheetBase::HasParentSupport() const
{
    return TRUE;
}

// Basisvorlage uf NULL setzen m"oglich? Default: Nein

BOOL SfxStyleSheetBase::HasClearParentSupport() const
{
    return FALSE;
}

// Defaultmaessig sind alle StyleSheets Used

BOOL SfxStyleSheetBase::IsUsed() const
{
    return TRUE;
}

// eingestellte Attribute ausgeben


XubString SfxStyleSheetBase::GetDescription()
{
    return GetDescription( SFX_MAPUNIT_CM );
}

// eingestellte Attribute ausgeben

XubString SfxStyleSheetBase::GetDescription( SfxMapUnit )
{
    XubString aEmpty;
    return aEmpty;
}

/////////////////////////// SfxStyleSheetIterator ///////////////////////////////

SfxStyleFamily SfxStyleSheetIterator::GetSearchFamily() const
{
    return nSearchFamily;
}

inline BOOL SfxStyleSheetIterator::IsTrivialSearch()
{
    return nMask == 0xFFFF && GetSearchFamily() == SFX_STYLE_FAMILY_ALL;
}

BOOL SfxStyleSheetIterator::DoesStyleMatch(SfxStyleSheetBase *pStyle)
{
    return ((GetSearchFamily() == SFX_STYLE_FAMILY_ALL) ||
            ( pStyle->GetFamily() == GetSearchFamily() ))
        && (( pStyle->GetMask() & ( GetSearchMask() & ~SFXSTYLEBIT_USED )) ||
            ( bSearchUsed ? pStyle->IsUsed() : FALSE ) ||
            GetSearchMask() == SFXSTYLEBIT_ALL );
}


SfxStyleSheetIterator::SfxStyleSheetIterator(SfxStyleSheetBasePool *pBase,
                                             SfxStyleFamily eFam, USHORT n)
{
    pBasePool=pBase;
    nSearchFamily=eFam;
    bSearchUsed=FALSE;
        if((n != SFXSTYLEBIT_ALL ) && ((n & SFXSTYLEBIT_USED) == SFXSTYLEBIT_USED))
    {
        bSearchUsed = TRUE;
        n &= ~SFXSTYLEBIT_USED;
    }
    nMask=n;
}

SfxStyleSheetIterator::~SfxStyleSheetIterator()
{
}


USHORT SfxStyleSheetIterator::Count()
{
    USHORT n = 0;
    if( IsTrivialSearch())
        n = (USHORT) pBasePool->aStyles.size();
    else
        for(USHORT i=0; i<pBasePool->aStyles.size(); i++)
        {
            SfxStyleSheetBase* pStyle = pBasePool->aStyles[ i ];
            if(DoesStyleMatch(pStyle))
                n++;
        }
    return n;
}

SfxStyleSheetBase* SfxStyleSheetIterator::operator[](USHORT nIdx)
{
    if( IsTrivialSearch())
        return ( nIdx < pBasePool->aStyles.size() ) ? pBasePool->aStyles[ nIdx ] : NULL;

    USHORT z = 0;
    for( size_t n = 0; n < pBasePool->aStyles.size(); n++ )
    {
        SfxStyleSheetBase* pStyle = pBasePool->aStyles[ n ];
        if( DoesStyleMatch(pStyle) )
        {
            if(z == nIdx)
            {
                nAktPosition=n;
                return pAktStyle=pStyle;
            }
            ++z;
        }
    }
    OSL_FAIL("falscher Index");
    return 0;
}

SfxStyleSheetBase* SfxStyleSheetIterator::First()
{
    INT32 nIdx = -1;

    if ( IsTrivialSearch() && pBasePool->aStyles.size() )
        nIdx = 0;
    else
        for( size_t n = 0; n < pBasePool->aStyles.size(); n++ )
        {
            SfxStyleSheetBase* pStyle = pBasePool->aStyles[ n ];

            if ( DoesStyleMatch( pStyle ) )
            {
                nIdx = n;
                break;
            }
        }

    if ( nIdx != -1 )
    {
        nAktPosition = (USHORT)nIdx;
        return pAktStyle = pBasePool->aStyles[ nIdx ];
    }
    return 0;
}


SfxStyleSheetBase* SfxStyleSheetIterator::Next()
{
    INT32 nIdx = -1;

    if ( IsTrivialSearch() &&
         (USHORT)pBasePool->aStyles.size() > nAktPosition + 1 )
        nIdx = nAktPosition + 1;
    else
        for( size_t n = nAktPosition + 1; n < pBasePool->aStyles.size(); n++ )
        {
            SfxStyleSheetBase* pStyle = pBasePool->aStyles[ n ];

            if ( DoesStyleMatch( pStyle ) )
            {
                nIdx = n;
                break;
            }
        }

    if ( nIdx != -1 )
    {
        nAktPosition = (USHORT)nIdx;
        return pAktStyle = pBasePool->aStyles[ nIdx ];
    }
    return 0;
}


SfxStyleSheetBase* SfxStyleSheetIterator::Find(const XubString& rStr)
{
    for ( size_t n = 0; n < pBasePool->aStyles.size(); n++ )
    {
        SfxStyleSheetBase* pStyle = pBasePool->aStyles[ n ];

        // #98454# performance: in case of bSearchUsed==TRUE it may be
        // significant to first compare the name and only if it matches to call
        // the style sheet IsUsed() method in DoesStyleMatch().
        if ( pStyle->GetName().Equals( rStr ) && DoesStyleMatch( pStyle ) )
        {
            nAktPosition = n;
            return pAktStyle = pStyle;
        }
    }
    return 0;
}


USHORT SfxStyleSheetIterator::GetSearchMask() const
{
    USHORT mask = nMask;

    if ( bSearchUsed )
        mask |= SFXSTYLEBIT_USED;
    return mask;
}

/////////////////////////// SfxStyleSheetBasePool ///////////////////////////////


void SfxStyleSheetBasePool::Replace(
    SfxStyleSheetBase& rSource, SfxStyleSheetBase& rTarget )
{
    rTarget.SetFollow( rSource.GetFollow() );
    rTarget.SetParent( rSource.GetParent() );
    SfxItemSet& rSourceSet = rSource.GetItemSet();
    SfxItemSet& rTargetSet = rTarget.GetItemSet();
    rTargetSet.Intersect( rSourceSet );
    rTargetSet.Put( rSourceSet );
}

SfxStyleSheetIterator& SfxStyleSheetBasePool::GetIterator_Impl()
{
    SfxStyleSheetIterator*& rpIter = pImp->pIter;
    if( !rpIter || rpIter->GetSearchMask() != nMask ||
        rpIter->GetSearchFamily() != nSearchFamily )
    {
        delete rpIter;
        rpIter = CreateIterator( nSearchFamily, nMask );
    }
    return *rpIter;
}


SfxStyleSheetBasePool::SfxStyleSheetBasePool( SfxItemPool& r )
    : aAppName(r.GetName())
    , rPool(r)
    , nSearchFamily(SFX_STYLE_FAMILY_PARA)
    , nMask(0xFFFF)
{
    pImp = new SfxStyleSheetBasePool_Impl;
}

SfxStyleSheetBasePool::SfxStyleSheetBasePool( const SfxStyleSheetBasePool& r )
    : SfxBroadcaster( r )
    , aAppName(r.aAppName)
    , rPool(r.rPool)
    , nSearchFamily(r.nSearchFamily)
    , nMask( r.nMask )
{
    pImp = new SfxStyleSheetBasePool_Impl;
    *this += r;
}

SfxStyleSheetBasePool::~SfxStyleSheetBasePool()
{
    Broadcast( SfxSimpleHint(SFX_HINT_DYING) );
    Clear();
    delete pImp;
}

BOOL SfxStyleSheetBasePool::SetParent(SfxStyleFamily eFam, const XubString& rStyle, const XubString& rParent)
{
    SfxStyleSheetIterator aIter(this,eFam,SFXSTYLEBIT_ALL);
    SfxStyleSheetBase *pStyle =
        aIter.Find(rStyle);
    DBG_ASSERT(pStyle, "Vorlage nicht gefunden. Writer mit Solar <2541??");
    if(pStyle)
        return pStyle->SetParent(rParent);
    else
        return FALSE;
}


void SfxStyleSheetBasePool::SetSearchMask(SfxStyleFamily eFam, USHORT n)
{
    nSearchFamily = eFam; nMask = n;
}

USHORT SfxStyleSheetBasePool::GetSearchMask() const
{
    return nMask;
}


// Der Name des Streams

String SfxStyleSheetBasePool::GetStreamName()
{
    return String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(STYLESTREAM));
}

/////////////////////////////////// Factory ////////////////////////////////



SfxStyleSheetIterator* SfxStyleSheetBasePool::CreateIterator
(
 SfxStyleFamily eFam,
 USHORT mask
)
{
    return new SfxStyleSheetIterator(this,eFam,mask);
}


SfxStyleSheetBase* SfxStyleSheetBasePool::Create
(
    const XubString& rName,
    SfxStyleFamily eFam,
    USHORT mask
)
{
    return new SfxStyleSheetBase( rName, *this, eFam, mask );
}

SfxStyleSheetBase* SfxStyleSheetBasePool::Create( const SfxStyleSheetBase& r )
{
    return new SfxStyleSheetBase( r );
}

SfxStyleSheetBase& SfxStyleSheetBasePool::Make( const XubString& rName,
                            SfxStyleFamily eFam, USHORT mask, USHORT nPos)
{
    DBG_ASSERT( eFam != SFX_STYLE_FAMILY_ALL, "FamilyAll als Familie nicht erlaubt" );

    SfxStyleSheetIterator aIter(this, eFam, mask);
    SfxStyleSheetBase* p = aIter.Find( rName );
    DBG_ASSERT( !p, "StyleSheet bereits vorhanden" );
    SfxStyleSheetIterator& rIter = GetIterator_Impl();

    if( !p )
    {
        p = Create( rName, eFam, mask );
        if(0xffff == nPos || nPos == aStyles.size() ||
           nPos == rIter.Count())
            aStyles.push_back( p );
        else
        {
            rIter[nPos];
            size_t i = rIter.GetPos();
            if ( i < aStyles.size() )
            {
                SfxStyles::iterator it = aStyles.begin();
                ::std::advance( it, i );
                aStyles.insert( it, p );
            }
            else
            {
                aStyles.push_back( p );
            }
        }
        Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_CREATED, *p ) );
    }
    return *p;
}

/////////////////////////////// Kopieren ///////////////////////////////////

// Hilfsroutine: Falls eine Vorlage dieses Namens existiert, wird
// sie neu erzeugt. Alle Vorlagen, die diese Vorlage zum Parent haben,
// werden umgehaengt.

SfxStyleSheetBase& SfxStyleSheetBasePool::Add( SfxStyleSheetBase& rSheet )
{
    SfxStyleSheetIterator aIter(this, rSheet.GetFamily(), nMask);
    SfxStyleSheetBase* pOld = aIter.Find( rSheet.GetName() );
    Erase( pOld );
    SfxStyleSheetBase* pNew = Create( rSheet );
    aStyles.push_back( pNew );
    Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_CHANGED, *pNew ) );
    return *pNew;
}

SfxStyleSheetBasePool& SfxStyleSheetBasePool::operator=( const SfxStyleSheetBasePool& r )
{
    if( &r != this )
    {
        Clear();
        *this += r;
    }
    return *this;
}

SfxStyleSheetBasePool& SfxStyleSheetBasePool::operator+=( const SfxStyleSheetBasePool& rP )
{
    if( &rP != this )
    {
        // kopieren
        SfxStyleSheetBasePool& r = (SfxStyleSheetBasePool&) rP;
        for ( size_t i = 0, n = r.aStyles.size(); i < n; ++i )
            Add( *r.aStyles[ i ] );
    }
    return *this;
}

//////////////////////////////// Suchen ////////////////////////////////////

USHORT SfxStyleSheetBasePool::Count()
{
    return GetIterator_Impl().Count();
}

SfxStyleSheetBase *SfxStyleSheetBasePool::operator[](USHORT nIdx)
{
    return GetIterator_Impl()[nIdx];
}

SfxStyleSheetBase* SfxStyleSheetBasePool::Find(const XubString& rName,
                                               SfxStyleFamily eFam,
                                               USHORT mask)
{
    SfxStyleSheetIterator aIter(this,eFam,mask);
    return aIter.Find(rName);
}

SfxStyleSheetBase* SfxStyleSheetBasePool::First()
{
    return GetIterator_Impl().First();
}

SfxStyleSheetBase* SfxStyleSheetBasePool::Next()
{
    return GetIterator_Impl().Next();
}

//////////////////////////////// Loeschen /////////////////////////////////

SfxStyleSheetBase* SfxStyleSheetBasePool::Remove( SfxStyleSheetBase* p )
{
    if( p )
    {
        // Alle Styles umsetzen, deren Parent dieser hier ist
        ChangeParent( p->GetName(), p->GetParent() );
        for ( SfxStyles::iterator it = aStyles.begin(); it < aStyles.end(); ++it )
        {
            if ( *it == p )
            {
                aStyles.erase( it );
                break;
            }
        }
        Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_ERASED, *p ) );
    }
    return p;
}

void SfxStyleSheetBasePool::Erase( SfxStyleSheetBase* p )
{
    if( p )
    {
        for ( SfxStyles::iterator it = aStyles.begin(); it < aStyles.end(); ++it )
        {
            if ( *it == p )
            {
                aStyles.erase( it );
                break;
            }
        }
        delete p;
    }
}

void SfxStyleSheetBasePool::Insert( SfxStyleSheetBase* p )
{
    DBG_ASSERT( p, "Kein StyleSheet?" );
    SfxStyleSheetIterator aIter(
        this, p->GetFamily(), p->GetMask());
    SfxStyleSheetBase* pOld = aIter.Find( p->GetName() );
    DBG_ASSERT( !pOld, "StyleSheet bereits vorhanden" );
    if( p->GetParent().Len() )
    {
        pOld = aIter.Find( p->GetParent() );
        DBG_ASSERT( pOld, "Parent nicht mehr vorhanden" );
    }
    (void)pOld;
    aStyles.push_back( p );
    Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_CREATED, *p ) );
}

void SfxStyleSheetBasePool::Clear()
{
    while( aStyles.size() )
    {
        SfxStyleSheetBase* p = aStyles[ 0 ];
        aStyles.erase( aStyles.begin() );
        Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_ERASED, *p ) );
        delete p;
    }
}

/////////////////////////// Parents umsetzen ////////////////////////////////

void SfxStyleSheetBasePool::ChangeParent(const XubString& rOld,
                                         const XubString& rNew,
                                         BOOL bVirtual)
{
    const USHORT nTmpMask = GetSearchMask();
    SetSearchMask(GetSearchFamily(), 0xffff);
    for( SfxStyleSheetBase* p = First(); p; p = Next() )
    {
        if( p->GetParent().Equals( rOld ) )
        {
            if(bVirtual)
                p->SetParent( rNew );
            else
                p->aParent = rNew;
        }
    }
    SetSearchMask(GetSearchFamily(), nTmpMask);
}

/////////////////////////// Laden/Speichern /////////////////////////////////

void SfxStyleSheetBase::Load( SvStream&, USHORT )
{
}

BOOL SfxStyleSheetBasePool::Load( SvStream& rStream )
{
    // alte Version?
    if ( !rPool.IsVer2_Impl() )
        return Load1_Impl( rStream );

    // gesamten StyleSheetPool in neuer Version aus einem MiniRecord lesen
    SfxMiniRecordReader aPoolRec( &rStream, SFX_STYLES_REC );

    // Header-Record lesen
    short nCharSet = 0;
    if ( !rStream.GetError() )
    {
        SfxSingleRecordReader aHeaderRec( &rStream, SFX_STYLES_REC_HEADER );
        if ( !aHeaderRec.IsValid() )
            return FALSE;

        aAppName = rPool.GetName();
        rStream >> nCharSet;
    }

    // Styles-Record lesen
    if ( !rStream.GetError() )
    {
        SfxMultiRecordReader aStylesRec( &rStream, SFX_STYLES_REC_STYLES );
        if ( !aStylesRec.IsValid() )
            return FALSE;

        rtl_TextEncoding eEnc = GetSOLoadTextEncoding(
            (rtl_TextEncoding)nCharSet );
        rtl_TextEncoding eOldEnc = rStream.GetStreamCharSet();
        rStream.SetStreamCharSet( eEnc );

        USHORT nStyles;
        for ( nStyles = 0; aStylesRec.GetContent(); nStyles++ )
        {
            // kann nicht mehr weiterlesen?
            if ( rStream.GetError() )
                break;

            // Globale Teile
            XubString aName, aParent, aFollow;
            String aHelpFile;
            USHORT nFamily, nStyleMask,nCount;
            sal_uInt32 nHelpId;
            aName = rStream.ReadUniOrByteString(eEnc );
            aParent = rStream.ReadUniOrByteString(eEnc );
            aFollow = rStream.ReadUniOrByteString(eEnc );
            rStream >> nFamily >> nStyleMask;
            SfxPoolItem::readByteString(rStream, aHelpFile);
            rStream >> nHelpId;

            SfxStyleSheetBase& rSheet = Make( aName, (SfxStyleFamily)nFamily , nStyleMask);
            rSheet.SetHelpId( aHelpFile, nHelpId );
            // Hier erst einmal Parent und Follow zwischenspeichern
            rSheet.aParent = aParent;
            rSheet.aFollow = aFollow;
            UINT32 nPos = rStream.Tell();
            rStream >> nCount;
            if(nCount)
            {
                rStream.Seek( nPos );
                // Das Laden des ItemSets bedient sich der Methode GetItemSet(),
                // damit eigene ItemSets untergeschoben werden koennen
                SfxItemSet& rSet = rSheet.GetItemSet();
                rSet.ClearItem();
    //!         SfxItemSet aTmpSet( *pTmpPool );
                /*!aTmpSet*/ rSet.Load( rStream );
                //! rSet.Put( aTmpSet );
            }
            // Lokale Teile
            UINT32 nSize;
            USHORT nVer;
            rStream >> nVer >> nSize;
            nPos = rStream.Tell() + nSize;
            rSheet.Load( rStream, nVer );
            rStream.Seek( nPos );
        }

        //  #72939# only loop through the styles that were really inserted
        size_t n = aStyles.size();

        //! delete pTmpPool;
        // Jetzt Parent und Follow setzen. Alle Sheets sind geladen.
        // Mit Setxxx() noch einmal den String eintragen, da diese
        // virtuellen Methoden evtl. ueberlagert sind.
        for ( size_t i = 0; i < n; i++ )
        {
            SfxStyleSheetBase* p = aStyles[ i ];
            XubString aText = p->aParent;
            p->aParent.Erase();
            p->SetParent( aText );
            aText = p->aFollow;
            p->aFollow.Erase();
            p->SetFollow( aText );
        }

        rStream.SetStreamCharSet( eOldEnc );
    }

    // alles klar?
    return BOOL( rStream.GetError() == SVSTREAM_OK );
}

BOOL SfxStyleSheetBasePool::Load1_Impl( SvStream& rStream )
{
    aAppName = rPool.GetName();
    USHORT nVersion;
    short nCharSet;
    rStream >> nVersion;

    if(nVersion!=STYLESTREAM_VERSION)
        nCharSet=nVersion;
    else
        rStream >> nCharSet;

    rtl_TextEncoding eEnc = GetSOLoadTextEncoding(
        (rtl_TextEncoding)nCharSet );
    rtl_TextEncoding eOldEnc = rStream.GetStreamCharSet();
    rStream.SetStreamCharSet( eEnc );

    USHORT nStyles;
    rStream >> nStyles;
    USHORT i;
    for ( i = 0; i < nStyles; i++ )
    {
        // kann nicht mehr weiterlesen?
        if ( rStream.GetError() )
        {
            nStyles = i;
            break;
        }

        // Globale Teile
        XubString aName, aParent, aFollow;
        String aHelpFile;
        USHORT nFamily, nStyleMask,nCount;
        sal_uInt32 nHelpId;
        aName = rStream.ReadUniOrByteString(eEnc );
        aParent = rStream.ReadUniOrByteString(eEnc );
        aFollow = rStream.ReadUniOrByteString(eEnc );
        rStream >> nFamily >> nStyleMask;
        SfxPoolItem::readByteString(rStream, aHelpFile);
        if(nVersion!=STYLESTREAM_VERSION)
        {
            USHORT nTmpHelpId;
            rStream >> nTmpHelpId;
            nHelpId=nTmpHelpId;
        }
        else
            rStream >> nHelpId;

        SfxStyleSheetBase& rSheet = Make( aName, (SfxStyleFamily)nFamily , nStyleMask);
        rSheet.SetHelpId( aHelpFile, nHelpId );
        // Hier erst einmal Parent und Follow zwischenspeichern
        rSheet.aParent = aParent;
        rSheet.aFollow = aFollow;
        UINT32 nPos = rStream.Tell();
        rStream >> nCount;
        if(nCount) {
            rStream.Seek( nPos );
            // Das Laden des ItemSets bedient sich der Methode GetItemSet(),
            // damit eigene ItemSets untergeschoben werden koennen
            SfxItemSet& rSet = rSheet.GetItemSet();
            rSet.ClearItem();
//!         SfxItemSet aTmpSet( *pTmpPool );
            /*!aTmpSet*/ rSet.Load( rStream );
            //! rSet.Put( aTmpSet );
        }
        // Lokale Teile
        UINT32 nSize;
        USHORT nVer;
        rStream >> nVer >> nSize;
        nPos = rStream.Tell() + nSize;
        rSheet.Load( rStream, nVer );
        rStream.Seek( nPos );
    }

    //! delete pTmpPool;
    // Jetzt Parent und Follow setzen. Alle Sheets sind geladen.
    // Mit Setxxx() noch einmal den String eintragen, da diese
    // virtuellen Methoden evtl. ueberlagert sind.
    for ( i = 0; i < nStyles; i++ )
    {
        SfxStyleSheetBase* p = aStyles[ i ];
        XubString aText = p->aParent;
        p->aParent.Erase();
        p->SetParent( aText );
        aText = p->aFollow;
        p->aFollow.Erase();
        p->SetFollow( aText );
    }

    rStream.SetStreamCharSet( eOldEnc );

    return BOOL( rStream.GetError() == SVSTREAM_OK );
}

SfxItemPool& SfxStyleSheetBasePool::GetPool()
{
    return rPool;
}

const SfxItemPool& SfxStyleSheetBasePool::GetPool() const
{
    return rPool;
}

/////////////////////// SfxStyleSheet /////////////////////////////////

SfxStyleSheet::SfxStyleSheet(const XubString &rName,
                             SfxStyleSheetBasePool& r_Pool,
                             SfxStyleFamily eFam,
                             USHORT mask ):
    SfxStyleSheetBase(rName, r_Pool, eFam, mask)
{}

SfxStyleSheet::SfxStyleSheet(const SfxStyleSheet& rStyle) :
    SfxStyleSheetBase(rStyle),
    SfxListener( rStyle ),
    SfxBroadcaster( rStyle )
{}

SfxStyleSheet::~SfxStyleSheet()
{
    Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_INDESTRUCTION, *this ) );
}


BOOL SfxStyleSheet::SetParent( const XubString& rName )
{
    if(aParent == rName)
        return TRUE;
    const XubString aOldParent(aParent);
    if(SfxStyleSheetBase::SetParent(rName)) {
            // aus der Benachrichtigungskette des alten
            // Parents gfs. austragen
        if(aOldParent.Len()) {
            SfxStyleSheet *pParent = (SfxStyleSheet *)rPool.Find(aOldParent, nFamily, 0xffff);
            if(pParent)
                EndListening(*pParent);
        }
            // in die Benachrichtigungskette des neuen
            // Parents eintragen
        if(aParent.Len()) {
            SfxStyleSheet *pParent = (SfxStyleSheet *)rPool.Find(aParent, nFamily, 0xffff);
            if(pParent)
                StartListening(*pParent);
        }
        return TRUE;
    }
    return FALSE;
}

// alle Zuhoerer benachtichtigen

void SfxStyleSheet::SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType,
                           const SfxHint& rHint, const TypeId& rHintType )
{
    SFX_FORWARD(rBC, rBCType, rHint, rHintType);
}

//////////////////////// SfxStyleSheetPool ///////////////////////////////

SfxStyleSheetPool::SfxStyleSheetPool( SfxItemPool& rSet):
    SfxStyleSheetBasePool(rSet)
{}

/////////////////////////////////// Factory ////////////////////////////////

SfxStyleSheetBase* SfxStyleSheetPool::Create( const XubString& rName,
                                    SfxStyleFamily eFam, USHORT mask )
{
    return new SfxStyleSheet( rName, *this, eFam, mask );
}

SfxStyleSheetBase* SfxStyleSheetPool::Create( const SfxStyleSheet& r )
{
    return new SfxStyleSheet( r );
}
/*
BOOL SfxStyleSheetPool::CopyTo(SfxStyleSheetPool &, const String &)
{
    return FALSE;
}
*/

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
