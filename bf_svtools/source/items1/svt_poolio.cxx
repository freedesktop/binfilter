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

#include <string.h>
#include <stdio.h>

#include <sal/log.hxx>
#include <bf_svtools/bf_solar.h>
#include <bf_svtools/itempool.hxx>
#include "whassert.hxx"
#include <bf_svtools/brdcst.hxx>
#include <bf_svtools/filerec.hxx>
#include <bf_svtools/svtdata.hxx>
#include "poolio.hxx"

namespace binfilter
{

// STATIC DATA -----------------------------------------------------------

DBG_NAME(SfxItemPool)

void SfxItemPool::LoadCompleted()

/*  [Beschreibung]

    Wurde der SfxItemPool mit 'bRefCounts' == FALSE geladen, mu\s das
    Laden der Dokumentinhalte mit einem Aufruf dieser Methode beendet
    werden. Ansonsten hat der Aufruf dieser Methode keine Funktion.


    [Anmerkung]

    Beim Laden ohne Ref-Counts werden diese tats"achlich auf 1 gesetzt,
    damit nicht w"ahrend des Ladevorgangs SfxPoolItems gel"oscht werden,
    die danach, aber auch noch beim Ladevorgang, ben"otigt werden. Diese
    Methode setzt den Ref-Count wieder zur"uck und entfernt dabei
    gleichzeitig alle nicht mehr ben"otigten Items.


    [Querverweise]

    <SfxItemPool::Load()>
*/

{
    // wurden keine Ref-Counts mitgeladen?
    if ( pImp->nInitRefCount > 1 )
    {

        // "uber alle Which-Werte iterieren
        SfxPoolItemArray_Impl** ppItemArr = pImp->ppPoolItems;
        for( USHORT nArrCnt = GetSize_Impl(); nArrCnt; --nArrCnt, ++ppItemArr )
        {
            // ist "uberhaupt ein Item mit dem Which-Wert da?
            if ( *ppItemArr )
            {
                // "uber alle Items mit dieser Which-Id iterieren
                SfxPoolItem** ppHtArr = (SfxPoolItem**)(*ppItemArr)->GetData();
                for( USHORT n = (*ppItemArr)->Count(); n; --n, ++ppHtArr )
                    if (*ppHtArr)
                    {
                        #ifdef DBG_UTIL
                        const SfxPoolItem &rItem = **ppHtArr;
                        DBG_ASSERT( !rItem.ISA(SfxSetItem) ||
                                    0 != &((const SfxSetItem&)rItem).GetItemSet(),
                                    "SetItem without ItemSet" );
                        #endif

                        if ( !ReleaseRef( **ppHtArr, 1 ) )
                            DELETEZ( *ppHtArr );
                    }
            }
        }

        // from now on normal initial ref count
        pImp->nInitRefCount = 1;
    }

    // notify secondary pool
    if ( pSecondary )
        pSecondary->LoadCompleted();
}

//============================================================================
// This had to be moved to a method of its own to keep Solaris GCC happy:
void SfxItemPool::readTheItems (
    SvStream & rStream, USHORT nItemCount, USHORT nVersion,
    SfxPoolItem * pDefItem, SfxPoolItemArray_Impl ** ppArr)
{
    SfxMultiRecordReader aItemsRec( &rStream, SFX_ITEMPOOL_REC_ITEMS );

    SfxPoolItemArray_Impl *pNewArr = new SfxPoolItemArray_Impl( nItemCount );
    SfxPoolItem *pItem = 0;

    USHORT n, nLastSurrogate = USHORT(-1);
    while (aItemsRec.GetContent())
    {
        // n"achstes Surrogat holen
        USHORT nSurrogate = aItemsRec.GetContentTag();
        DBG_ASSERT( aItemsRec.GetContentVersion() == 'X',
                    "not an item content" );

        // fehlende auff"ullen
        for ( pItem = 0, n = nLastSurrogate+1; n < nSurrogate; ++n )
            pNewArr->C40_INSERT(SfxPoolItem, pItem, n);
        nLastSurrogate = nSurrogate;

        // Ref-Count und Item laden
        USHORT nRef(0);
        rStream >> nRef;

        pItem = pDefItem->Create(rStream, nVersion);
        pNewArr->C40_INSERT(SfxPoolItem, pItem, nSurrogate);

        if ( !bPersistentRefCounts )
            // bis <SfxItemPool::LoadCompleted()> festhalten
            AddRef(*pItem, 1);
        else
        {
            if ( nRef > SFX_ITEMS_OLD_MAXREF )
                pItem->SetKind( nRef );
            else
                AddRef(*pItem, nRef);
        }
    }

    // fehlende auff"ullen
    for ( pItem = 0, n = nLastSurrogate+1; n < nItemCount; ++n )
        pNewArr->C40_INSERT(SfxPoolItem, pItem, n);

    SfxPoolItemArray_Impl *pOldArr = *ppArr;
    *ppArr = pNewArr;

    // die Items merken, die schon im Pool sind
    int bEmpty = TRUE;
    if ( 0 != pOldArr )
        for ( n = 0; bEmpty && n < pOldArr->Count(); ++n )
            bEmpty = pOldArr->GetObject(n) == 0;
    DBG_ASSERTWARNING( bEmpty, "loading non-empty pool" );
    if ( !bEmpty )
    {
        // f"ur alle alten suchen, ob ein gleiches neues existiert
        for ( USHORT nOld = 0; nOld < pOldArr->Count(); ++nOld )
        {
            SfxPoolItem *pOldItem = (*pOldArr)[nOld];
            if ( pOldItem )
            {
                USHORT nFree = USHRT_MAX;
                int bFound = FALSE;
                USHORT nCount = (*ppArr)->Count();
                for ( USHORT nNew = nCount; !bFound && nNew--; )
                {
                    // geladenes Item
                    SfxPoolItem *&rpNewItem =
                        (SfxPoolItem*&)(*ppArr)->GetData()[nNew];

                    // surrogat unbenutzt?
                    if ( !rpNewItem )
                        nFree = nNew;

                    // gefunden?
                    else if ( *rpNewItem == *pOldItem )
                    {
                        // wiederverwenden
                        AddRef( *pOldItem, rpNewItem->GetRefCount() );
                        SetRefCount( *rpNewItem, 0 );
                        delete rpNewItem;
                        rpNewItem = pOldItem;
                        bFound = TRUE;
                    }
                }

                // vorhervorhandene, nicht geladene uebernehmen
                if ( !bFound )
                {
                    if ( nFree != USHRT_MAX )
                        (SfxPoolItem*&)(*ppArr)->GetData()[nFree] = pOldItem;
                    else
                        (*ppArr)->C40_INSERT( SfxPoolItem, pOldItem, nCount );
                }
            }
        }
    }
    delete pOldArr;
}

// -----------------------------------------------------------------------

SvStream &SfxItemPool::Load(SvStream &rStream)
{
    DBG_CHKTHIS(SfxItemPool, 0);
    DBG_ASSERT(ppStaticDefaults, "kein DefaultArray");

    // protect items by increasing ref count
    if ( !bPersistentRefCounts )
    {

        // "uber alle Which-Werte iterieren
        SfxPoolItemArray_Impl** ppItemArr = pImp->ppPoolItems;
        for( USHORT nArrCnt = GetSize_Impl(); nArrCnt; --nArrCnt, ++ppItemArr )
        {
            // ist "uberhaupt ein Item mit dem Which-Wert da?
            if ( *ppItemArr )
            {
                // "uber alle Items mit dieser Which-Id iterieren
                SfxPoolItem** ppHtArr = (SfxPoolItem**)(*ppItemArr)->GetData();
                for( USHORT n = (*ppItemArr)->Count(); n; --n, ++ppHtArr )
                    if (*ppHtArr)
                    {
                        #ifdef DBG_UTIL
                        const SfxPoolItem &rItem = **ppHtArr;
                        DBG_ASSERT( !rItem.ISA(SfxSetItem) ||
                                    0 != &((const SfxSetItem&)rItem).GetItemSet(),
                                    "SetItem without ItemSet" );
                        DBG_WARNING( "loading non-empty ItemPool" );
                        #endif

                        AddRef( **ppHtArr, 1 );
                    }
            }
        }

        // during loading (until LoadCompleted()) protect all items
        pImp->nInitRefCount = 2;
    }

    // Load-Master finden
    SfxItemPool *pLoadMaster = pMaster != this ? pMaster : 0;
    while ( pLoadMaster && !pLoadMaster->pImp->bStreaming )
        pLoadMaster = pLoadMaster->pSecondary;

    // Gesamt Header einlesen
    pImp->bStreaming = TRUE;
    if ( !pLoadMaster )
    {
        // Format-Version laden
        CHECK_FILEFORMAT2( rStream,
                SFX_ITEMPOOL_TAG_STARTPOOL_5, SFX_ITEMPOOL_TAG_STARTPOOL_4 );
        rStream >> pImp->nMajorVer >> pImp->nMinorVer;

        // Format-Version in Master-Pool "ubertragen
        pMaster->pImp->nMajorVer = pImp->nMajorVer;
        pMaster->pImp->nMinorVer = pImp->nMinorVer;

        // altes Format?
        if ( pImp->nMajorVer < 2 )
            // pImp->bStreaming wird von Load1_Impl() zur"uckgesetzt
            return Load1_Impl( rStream );

        // zu neues Format?
        if ( pImp->nMajorVer > SFX_ITEMPOOL_VER_MAJOR )
        {
            rStream.SetError(SVSTREAM_FILEFORMAT_ERROR);
            pImp->bStreaming = FALSE;
            return rStream;
        }

        // Version 1.2-Trick-Daten "uberspringen
        CHECK_FILEFORMAT( rStream, SFX_ITEMPOOL_TAG_TRICK4OLD );
        rStream.SeekRel( 4 ); // Hack-Daten wegen SfxStyleSheetPool-Bug  skippen
    }

    // neues Record-orientiertes Format
    SfxMiniRecordReader aPoolRec( &rStream, SFX_ITEMPOOL_REC );
    if ( rStream.GetError() )
    {
        pImp->bStreaming = FALSE;
        return rStream;
    }

    // Einzel-Header
    int bOwnPool = TRUE;
    UniString aExternName;
    {
        // Header-Record suchen
        SfxMiniRecordReader aPoolHeaderRec( &rStream, SFX_ITEMPOOL_REC_HEADER );
        if ( rStream.GetError() )
        {
            pImp->bStreaming = FALSE;
            return rStream;
        }

        // Header-lesen
        rStream >> pImp->nLoadingVersion;
        SfxPoolItem::readByteString(rStream, aExternName);
        bOwnPool = aExternName == aName;

        //! solange wir keine fremden Pools laden k"onnen
        if ( !bOwnPool )
        {
            rStream.SetError(SVSTREAM_FILEFORMAT_ERROR);
            aPoolRec.Skip();
            pImp->bStreaming = FALSE;
            return rStream;
        }
    }

    // Version-Maps
    {
        SfxMultiRecordReader aVerRec( &rStream, SFX_ITEMPOOL_REC_VERSIONMAP );
        if ( rStream.GetError() )
        {
            pImp->bStreaming = FALSE;
            return rStream;
        }

        // Versions-Maps einlesen
        USHORT nOwnVersion = pImp->nVersion;
        for ( USHORT nVerNo = 0; aVerRec.GetContent(); ++nVerNo )
        {
            // Header f"ur einzelne Version einlesen
            USHORT nVersion(0), nHStart(0), nHEnd(0);
            rStream >> nVersion >> nHStart >> nHEnd;
            USHORT nCount = nHEnd - nHStart + 1;

            // Version neuer als bekannt?
            if ( nVerNo >= pImp->aVersions.Count() )
            {
                // neue Version hinzufuegen
                USHORT *pMap = new USHORT[nCount];
                memset(pMap, 0, nCount * sizeof(USHORT));
                for ( USHORT n = 0; n < nCount; ++n )
                    rStream >> pMap[n];
                SetVersionMap( nVersion, nHStart, nHEnd, pMap );
            }
        }
        pImp->nVersion = nOwnVersion;
    }

    // Items laden
    bool bSecondaryLoaded = FALSE;
    long nSecondaryEnd = 0;
    {
        SfxMultiRecordReader aWhichIdsRec( &rStream, SFX_ITEMPOOL_REC_WHICHIDS);
        while ( aWhichIdsRec.GetContent() )
        {
            // SlotId, Which-Id und Item-Version besorgen
            USHORT nCount(0), nVersion(0), nWhich(0);
            rStream >> nWhich;
            if ( pImp->nLoadingVersion != pImp->nVersion )
                // Which-Id aus File-Version in Pool-Version verschieben
                nWhich = GetNewWhich( nWhich );

            // unbekanntes Item aus neuerer Version
            if ( !IsInRange(nWhich) )
                continue;

            rStream >> nVersion;
            rStream >> nCount;

            USHORT nIndex = GetIndex_Impl(nWhich);
            SfxPoolItemArray_Impl **ppArr = pImp->ppPoolItems + nIndex;

            // SfxSetItems k"onnten Items aus Sekund"arpools beinhalten
            SfxPoolItem *pDefItem = *(ppStaticDefaults + nIndex);
            pImp->bInSetItem = pDefItem->ISA(SfxSetItem);
            if ( !bSecondaryLoaded && pSecondary && pImp->bInSetItem )
            {
                // an das Ende des eigenen Pools seeken
                ULONG nLastPos = rStream.Tell();
                aPoolRec.Skip();

                // Sekund"arpool einlesen
                pSecondary->Load( rStream );
                bSecondaryLoaded = TRUE;
                nSecondaryEnd = rStream.Tell();

                // zur"uck zu unseren eigenen Items
                rStream.Seek(nLastPos);
            }

            // Items an sich lesen
            readTheItems(rStream, nCount, nVersion, pDefItem, ppArr);

            pImp->bInSetItem = FALSE;
        }
    }

    // Pool-Defaults lesen
    {
        SfxMultiRecordReader aDefsRec( &rStream, SFX_ITEMPOOL_REC_DEFAULTS );

        while ( aDefsRec.GetContent() )
        {
            // SlotId, Which-Id und Item-Version besorgen
            USHORT nVersion(0), nWhich(0);
            rStream >> nWhich;
            if ( pImp->nLoadingVersion != pImp->nVersion )
                // Which-Id aus File-Version in Pool-Version verschieben
                nWhich = GetNewWhich( nWhich );

            // unbekanntes Item aus neuerer Version
            if ( !IsInRange(nWhich) )
                continue;

            rStream >> nVersion;

            // Pool-Default-Item selbst laden
            SfxPoolItem *pItem =
                    ( *( ppStaticDefaults + GetIndex_Impl(nWhich) ) )
                    ->Create( rStream, nVersion );
            pItem->SetKind( SFX_ITEMS_POOLDEFAULT );
            *( ppPoolDefaults + GetIndex_Impl(nWhich) ) = pItem;
        }
    }

    // ggf. Secondary-Pool laden
    aPoolRec.Skip();
    if ( pSecondary )
    {
        if ( !bSecondaryLoaded )
            pSecondary->Load( rStream );
        else
            rStream.Seek( nSecondaryEnd );
    }

    // wenn nicht own-Pool, dann kein Name
    if ( aExternName != aName )
        aName.Erase();

    pImp->bStreaming = FALSE;
    return rStream;
};

// -----------------------------------------------------------------------

SvStream &SfxItemPool::Load1_Impl(SvStream &rStream)
{
    // beim Master ist der Header schon von <Load()> geladen worden
    if ( !pImp->bStreaming )
    {
        // Header des Secondary lesen
        CHECK_FILEFORMAT( rStream, SFX_ITEMPOOL_TAG_STARTPOOL_4 );
        rStream >> pImp->nMajorVer >> pImp->nMinorVer;
    }
    sal_uInt32 nAttribSize(0);
    int bOwnPool = TRUE;
    UniString aExternName;
    if ( pImp->nMajorVer > 1 || pImp->nMinorVer >= 2 )
        rStream >> pImp->nLoadingVersion;
    SfxPoolItem::readByteString(rStream, aExternName);
    bOwnPool = aExternName == aName;
    pImp->bStreaming = TRUE;

    //! solange wir keine fremden laden k"onnen
    if ( !bOwnPool )
    {
        rStream.SetError(SVSTREAM_FILEFORMAT_ERROR);
        pImp->bStreaming = FALSE;
        return rStream;
    }

    // Versionen bis 1.3 k"onnen noch keine Which-Verschiebungen lesen
    if ( pImp->nMajorVer == 1 && pImp->nMinorVer <= 2 &&
         pImp->nVersion < pImp->nLoadingVersion )
    {
        rStream.SetError(ERRCODE_IO_WRONGVERSION);
        pImp->bStreaming = FALSE;
        return rStream;
    }

    // Size-Table liegt hinter den eigentlichen Attributen
    rStream >> nAttribSize;

    // Size-Table einlesen
    ULONG nStartPos = rStream.Tell();
    rStream.SeekRel( nAttribSize );
    CHECK_FILEFORMAT( rStream, SFX_ITEMPOOL_TAG_SIZES );
    sal_uInt32 nSizeTableLen(0);
    rStream >> nSizeTableLen;
    sal_Char *pBuf = new sal_Char[nSizeTableLen];
    rStream.Read( pBuf, nSizeTableLen );
    ULONG nEndOfSizes = rStream.Tell();
    SvMemoryStream aSizeTable( pBuf, nSizeTableLen, STREAM_READ );

    // ab Version 1.3 steht in der Size-Table eine Versions-Map
    if ( pImp->nMajorVer > 1 || pImp->nMinorVer >= 3 )
    {
        // Version-Map finden (letztes ULONG der Size-Table gibt Pos an)
        rStream.Seek( nEndOfSizes - sizeof(sal_uInt32) );
        sal_uInt32 nVersionMapPos(0);
        rStream >> nVersionMapPos;
        rStream.Seek( nVersionMapPos );

        // Versions-Maps einlesen
        CHECK_FILEFORMAT( rStream, SFX_ITEMPOOL_TAG_VERSIONMAP );
        USHORT nVerCount(0);
        rStream >> nVerCount;
        for ( USHORT nVerNo = 0; nVerNo < nVerCount; ++nVerNo )
        {
            // Header f"ur einzelne Version einlesen
            USHORT nVersion(0), nHStart(0), nHEnd(0);
            rStream >> nVersion >> nHStart >> nHEnd;
            USHORT nCount = nHEnd - nHStart + 1;
            USHORT nBytes = (nCount)*sizeof(USHORT);

            // Version neuer als bekannt?
            if ( nVerNo >= pImp->aVersions.Count() )
            {
                // neue Version hinzufuegen
                USHORT *pMap = new USHORT[nCount];
                memset(pMap, 0, nCount * sizeof(USHORT));
                for ( USHORT n = 0; n < nCount; ++n )
                    rStream >> pMap[n];
                SetVersionMap( nVersion, nHStart, nHEnd, pMap );
            }
            else
                // Version schon bekannt => "uberspringen
                rStream.SeekRel( nBytes );
        }
    }

    // Items laden
    rStream.Seek( nStartPos );
    CHECK_FILEFORMAT( rStream, SFX_ITEMPOOL_TAG_ITEMS );
    bool bSecondaryLoaded = FALSE;
    long nSecondaryEnd = 0;
    USHORT nWhich(0), nSlot(0);
    while ( rStream >> nWhich, nWhich )
    {
        // ggf. Which-Id aus alter Version verschieben?
        if ( pImp->nLoadingVersion != pImp->nVersion )
            nWhich = GetNewWhich( nWhich );

        rStream >> nSlot;
        USHORT nMappedWhich = GetWhich(nSlot, FALSE);
        int bKnownItem = bOwnPool || IsWhich(nMappedWhich);

        USHORT nRef(0), nCount(0), nVersion(0);
        sal_uInt32 nAttrSize(0);
        rStream >> nVersion >> nCount;

        SfxPoolItemArray_Impl **ppArr = 0;
        SfxPoolItemArray_Impl *pNewArr = 0;
        SfxPoolItem *pDefItem = 0;
        if ( bKnownItem )
        {
            if ( !bOwnPool )
                nWhich = nMappedWhich;

            USHORT nIndex = GetIndex_Impl(nWhich);
            ppArr = pImp->ppPoolItems + nIndex;
            pNewArr = new SfxPoolItemArray_Impl( nCount );
            pDefItem = *(ppStaticDefaults + nIndex);
        }

        // Position vor ersten Item merken
        ULONG nLastPos = rStream.Tell();

        // SfxSetItems k"onnten Items aus Sekund"arpools beinhalten
        if ( !bSecondaryLoaded && pSecondary && pDefItem->ISA(SfxSetItem) )
        {
            // an das Ende des eigenen Pools seeken
            rStream.Seek(nEndOfSizes);
            CHECK_FILEFORMAT_RELEASE( rStream, SFX_ITEMPOOL_TAG_ENDPOOL, pNewArr );
            CHECK_FILEFORMAT_RELEASE( rStream, SFX_ITEMPOOL_TAG_ENDPOOL, pNewArr );

            // Sekund"arpool einlesen
            pSecondary->Load1_Impl( rStream );
            bSecondaryLoaded = TRUE;
            nSecondaryEnd = rStream.Tell();

            // zur"uck zu unseren eigenen Items
            rStream.Seek(nLastPos);
        }

        // Items an sich lesen
        for ( USHORT j = 0; j < nCount; ++j )
        {
            ULONG nPos = nLastPos;
            rStream >> nRef;

            if ( bKnownItem )
            {
                SfxPoolItem *pItem = 0;
                if ( nRef )
                {
                    pItem = pDefItem->Create(rStream, nVersion);

                    if ( !bPersistentRefCounts )
                        // bis <SfxItemPool::LoadCompleted()> festhalten
                        AddRef(*pItem, 1);
                    else
                    {
                        if ( nRef > SFX_ITEMS_OLD_MAXREF )
                            pItem->SetKind( nRef );
                        else
                            AddRef(*pItem, nRef);
                    }
                }

                pNewArr->C40_INSERT( SfxPoolItem, pItem, j);

                // restliche gespeicherte Laenge skippen (neueres Format)
                nLastPos = rStream.Tell();
            }

            aSizeTable >> nAttrSize;
            SFX_ASSERT( !bKnownItem || ( nPos + nAttrSize) >= nLastPos,
                        nPos,
                        "too many bytes read - version mismatch?" );

            if ( !bKnownItem || ( nLastPos < (nPos + nAttrSize) ) )
            {
                nLastPos = nPos + nAttrSize;
                rStream.Seek( nLastPos );
            }
        }

        if ( bKnownItem )
        {
            SfxPoolItemArray_Impl *pOldArr = *ppArr;
            *ppArr = pNewArr;

            // die Items merken, die schon im Pool sind
            int bEmpty = TRUE;
            if ( 0 != pOldArr )
                for ( USHORT n = 0; bEmpty && n < pOldArr->Count(); ++n )
                    bEmpty = pOldArr->GetObject(n) == 0;
            DBG_ASSERTWARNING( bEmpty, "loading non-empty pool" );
            if ( !bEmpty )
            {
                // f"ur alle alten suchen, ob ein gleiches neues existiert
                for ( USHORT nOld = 0; nOld < pOldArr->Count(); ++nOld )
                {
                    SfxPoolItem *pOldItem = (*pOldArr)[nOld];
                    if ( pOldItem )
                    {
                        int bFound = FALSE;
                        for ( USHORT nNew = 0;
                              !bFound && nNew < (*ppArr)->Count();
                              ++nNew )
                        {
                            SfxPoolItem *&rpNewItem =
                                (SfxPoolItem*&)(*ppArr)->GetData()[nNew];

                            if ( rpNewItem && *rpNewItem == *pOldItem )
                            {
                                AddRef( *pOldItem, rpNewItem->GetRefCount() );
                                SetRefCount( *rpNewItem, 0 );
                                delete rpNewItem;
                                rpNewItem = pOldItem;
                                bFound = TRUE;
                                SAL_INFO(
                                    "binfilter", "reusing item" << pOldItem);
                            }
                        }
                        SAL_INFO_IF(
                            !bFound, "binfilter",
                            "item not found: " << pOldItem);
                    }
                }
            }
            delete pOldArr; /* @@@ */
        }
    }

    // Pool-Defaults lesen
    if ( pImp->nMajorVer > 1 || pImp->nMinorVer > 0 )
        CHECK_FILEFORMAT( rStream, SFX_ITEMPOOL_TAG_DEFAULTS );

    ULONG nLastPos = rStream.Tell();
    while ( rStream >> nWhich, nWhich )
    {
        // ggf. Which-Id aus alter Version verschieben?
        if ( pImp->nLoadingVersion != pImp->nVersion )
            nWhich = GetNewWhich( nWhich );

        rStream >> nSlot;
        USHORT nMappedWhich = GetWhich(nSlot, FALSE);
        int bKnownItem = bOwnPool || IsWhich(nMappedWhich);

        ULONG nPos = nLastPos;
        sal_uInt32 nSize(0);
        USHORT nVersion(0);
        rStream >> nVersion;

        if ( bKnownItem )
        {
            if ( !bOwnPool )
                nWhich = nMappedWhich;
            SfxPoolItem *pItem =
                ( *( ppStaticDefaults + GetIndex_Impl(nWhich) ) )
                ->Create( rStream, nVersion );
            pItem->SetKind( SFX_ITEMS_POOLDEFAULT );
            *( ppPoolDefaults + GetIndex_Impl(nWhich) ) = pItem;
        }

        nLastPos = rStream.Tell();
        aSizeTable >> nSize;
        SFX_ASSERT( ( nPos + nSize) >= nLastPos, nPos,
                    "too many bytes read - version mismatch?" );
        if ( nLastPos < (nPos + nSize) )
            rStream.Seek( nPos + nSize );
    }

    delete[] pBuf;
    rStream.Seek(nEndOfSizes);
    CHECK_FILEFORMAT( rStream, SFX_ITEMPOOL_TAG_ENDPOOL );
    CHECK_FILEFORMAT( rStream, SFX_ITEMPOOL_TAG_ENDPOOL );

    if ( pSecondary )
    {
        if ( !bSecondaryLoaded )
            pSecondary->Load1_Impl( rStream );
        else
            rStream.Seek( nSecondaryEnd );
    }

    if ( aExternName != aName )
        aName.Erase();

    pImp->bStreaming = FALSE;
    return rStream;
}

// -----------------------------------------------------------------------

const SfxPoolItem* SfxItemPool::LoadSurrogate
(
    SvStream&           rStream,    // vor einem Surrogat positionierter Stream
    USHORT&             rWhich,     // Which-Id des zu ladenden <SfxPoolItem>s
    USHORT              nSlotId,    // Slot-Id des zu ladenden <SfxPoolItem>s
    const SfxItemPool*  pRefPool    // <SfxItemPool> in dem das Surrogat gilt
)

/*  [Beschreibung]

    L"adt Surrogat aus 'rStream' und liefert das dadurch in 'rRefPool'
    repr"asentierte SfxPoolItem zu"ruck. Ist das im Stream befindliche
    Surrogat == SFX_ITEMS_DIRECT (!SFX_ITEM_POOLABLE) wird 0 zur"uckgegeben,
    das Item ist direkt aus dem Stream zu laden. Bei 0xfff0 (SFX_ITEMS_NULL)
    wird auch 0 zurueckgegeben und rWhich auf 0 gesetzt, das Item ist nicht
    verfuegbar.

    Ansonsten wird ber"ucksichtigt, ob der betroffene Pool ohne Ref-Counts
    geladen wird, ob aus einem neuen Pool nachgeladen wird (&rRefPool != this)
    oder ob aus einem g"anzlich anders aufgebauten Pool geladen wird.

    Wird aus einem anders aufgebauten Pool geladen und die 'nSlotId' kann
    nicht in eine Which-Id dieses Pools gemappt werden, wird ebenfalls 0
    zur"uckgeliefert.

    Preconditions:  - Pool mu\s geladen sein
                    - LoadCompleted darf noch nicht gerufen worden sein
                    - 'rStream' steht genau an der Position, an der ein
                      Surrogat f"ur ein Item mit der SlotId 'nSlotId' und
                      der WhichId 'rWhichId' mit StoreSurrogate gepeichert
                      wurde

    Postconditions: - 'rStream' ist so positioniert, wie auch StoreSurrogate
                      sein speichern beendet hatte
                    - konnte ein Item geladen werden, befindet es sich
                      in diesem SfxItemPool
                    - 'rWhichId' enth"alt die ggf. gemappte Which-Id
    Laufzeit:       Tiefe des Ziel Sekund"arpools * 10 + 10
*/

{
    // erstmal das Surrogat lesen
    USHORT nSurrogat(0);
    rStream >> nSurrogat;

    // direkt gespeichertes Item?
    if ( SFX_ITEMS_DIRECT == nSurrogat )
        return 0;

    // nicht vorhandenes Item?
    if ( SFX_ITEMS_NULL == nSurrogat )
    {
        rWhich = 0;
        return 0;
    }

    // Bei einem identisch aufgebauten Pool (im Stream) kann das Surrogat
    // auf jeden Fall aufgel"ost werden.
    if ( !pRefPool )
        pRefPool = this;
    bool bResolvable = pRefPool->GetName().Len() > 0;
    if ( !bResolvable )
    {
        // Bei einem anders aufgebauten Pool im Stream, mu\s die SlotId
        // aus dem Stream in eine Which-Id gemappt werden k"onnen.
        USHORT nMappedWhich = nSlotId ? GetWhich(nSlotId, TRUE) : 0;
        if ( IsWhich(nMappedWhich) )
        {
            // gemappte SlotId kann "ubernommen werden
            rWhich = nMappedWhich;
            bResolvable = TRUE;
        }
    }

    // kann Surrogat aufgel"ost werden?
    const SfxPoolItem *pItem = 0;
    if ( bResolvable )
    {
        for ( SfxItemPool *pTarget = this; pTarget; pTarget = pTarget->pSecondary )
        {
            // richtigen (Folge-) Pool gefunden?
            if ( pTarget->IsInRange(rWhich) )
            {
                // dflt-Attribut?
                if ( SFX_ITEMS_STATICDEFAULT == nSurrogat )
                    return *(pTarget->ppStaticDefaults +
                            pTarget->GetIndex_Impl(rWhich));

                SfxPoolItemArray_Impl* pItemArr = *(pTarget->pImp->ppPoolItems +
                        pTarget->GetIndex_Impl(rWhich));
                pItem = pItemArr && nSurrogat < pItemArr->Count()
                            ? (*pItemArr)[nSurrogat]
                            : 0;
                if ( !pItem )
                {
                    OSL_FAIL( "can't resolve surrogate" );
                    rWhich = 0; // nur zur Sicherheit fuer richtige Stream-Pos
                    return 0;
                }

                // Nachladen aus Ref-Pool?
                if ( pRefPool != pMaster )
                    return &pTarget->Put( *pItem );

                // Referenzen sind NICHT schon mit Pool geladen worden?
                if ( !pTarget->HasPersistentRefCounts() )
                    AddRef( *pItem, 1 );
                else
                    return pItem;

                return pItem;
            }
        }

        SFX_ASSERT( FALSE, rWhich, "can't resolve Which-Id in LoadSurrogate" );
    }

    return 0;
}

// -----------------------------------------------------------------------

USHORT SfxItemPool::GetSurrogate(const SfxPoolItem *pItem) const
{
    DBG_CHKTHIS(SfxItemPool, 0);
    DBG_ASSERT( pItem, "no 0-Pointer Surrogate" );
    DBG_ASSERT( !IsInvalidItem(pItem), "no Invalid-Item Surrogate" );
    DBG_ASSERT( !IsPoolDefaultItem(pItem), "no Pool-Default-Item Surrogate" );

    if ( !IsInRange(pItem->Which()) )
    {
        if ( pSecondary )
            return pSecondary->GetSurrogate( pItem );
        SFX_ASSERT( 0, pItem->Which(), "unknown Which-Id - dont ask me for surrogates" );
    }

    // Pointer auf static- oder pool-dflt-Attribut?
    if( IsStaticDefaultItem(pItem) || IsPoolDefaultItem(pItem) )
        return SFX_ITEMS_STATICDEFAULT;

    SfxPoolItemArray_Impl* pItemArr = *(pImp->ppPoolItems + GetIndex_Impl(pItem->Which()));
    DBG_ASSERT(pItemArr, "ItemArr nicht vorhanden");
    const USHORT nCount = pItemArr->Count();
    for ( USHORT i = 0; i < nCount; ++i )
    {
        const SfxPoolItem *p = (*pItemArr)[i];
        if ( p == pItem )
            return i;
    }
    SFX_ASSERT( 0, pItem->Which(), "Item nicht im Pool");
    return SFX_ITEMS_NULL;
}

// -----------------------------------------------------------------------

void SfxItemPool::SetVersionMap
(
    USHORT  nVer,               /*  neue Versionsnummer */
    USHORT  nOldStart,          /*  alte erste Which-Id */
    USHORT  nOldEnd,            /*  alte letzte Which-Id */
    USHORT* pOldWhichIdTab      /*  Array mit genau dem Aufbau der Which-Ids
                                    der vorhergehenden Version, in denen
                                    die jeweils neue Which-Id steht. */
)

/*  [Beschreibung]

    Mit dieser Methode k"onnen neue, inkompatible Which-Id-Folgen oder
    Verteilungen realisiert werden. Pools, die noch mit alten Versionen
    gespeichert wurden, werden dann "uber die angegebene Tabelle solange
    gemappt, bis die aktuelle Version erreicht ist. Neuere Pools k"onnen
    unter Verlust neuer Attribute geladen werden, da die Map mit dem Pool
    gespeichert wird.

    Precondition:   Pool darf noch nicht geladen sein
    Postcondition:  Which-Ids aus fr"uheren Versionen k"onnen bei Laden auf
                    Version 'nVer' gemappt werden
    Laufzeit:       1.5 * new + 10

    [Anmerkung]

    F"ur neue Which-Ranges (nStart,nEnd) m"ssen im Vergleich zur Vorg"anger-
    Version (nOldStart,nOldEnd) immer gelten, da\s (nOldStart,nOldEnd)
    vollst"andig in (nStart,nEnd) enthalten ist. Es ist also zul"assig, den
    Which-Range in beide Richtungen zu erweitern, auch durch Einf"ugung
    von Which-Ids, nicht aber ihn zu beschneiden.

    Diese Methode sollte nur im oder direkt nach Aufruf des Konstruktors
    gerufen werden.

    Das Array mu\s statisch sein, da es nicht kopiert wird und au\serdem
    im Copy-Ctor des SfxItemPool wiederverwendet wird.


    [Beispiel]

    Urspr"unglich (Version 0) hatte der Pool folgende Which-Ids:

        1:A, 2:B, 3:C, 4:D

    Nun soll eine neue Version (Version 1) zwei zus"atzliche Ids X und Y
    zwischen B und C erhalten, also wie folgt aussehen:

        1:A, 2:B, 3:X, 4:Y, 5:C, 6:D

    Dabei haben sich also die Ids 3 und 4 ge"andert. F"ur die neue Version
    m"u\ste am Pool folgendes gesetzt werden:

        static USHORT nVersion1Map = { 1, 2, 5, 6 };
        pPool->SetVersionMap( 1, 1, 4, &nVersion1Map );


    [Querverweise]

    <SfxItemPool::IsLoadingVersionCurrent()const>
    <SfxItemPool::GetNewWhich(USHORT)>
    <SfxItemPool::GetLoadingVersion()const>
*/

{
    // neuen Map-Eintrag erzeugen und einf"ugen
    const SfxPoolVersion_Impl *pVerMap = new SfxPoolVersion_Impl(
                nVer, nOldStart, nOldEnd, pOldWhichIdTab );
    pImp->aVersions.Insert( pVerMap, pImp->aVersions.Count() );

    DBG_ASSERT( nVer > pImp->nVersion, "Versions not sorted" );
    pImp->nVersion = nVer;

    // Versions-Range anpassen
    for ( USHORT n = 0; n < nOldEnd-nOldStart+1; ++n )
    {
        USHORT nWhich = pOldWhichIdTab[n];
        if ( nWhich < pImp->nVerStart )
        {
            if ( !nWhich )
                nWhich = 0;
            pImp->nVerStart = nWhich;
        }
        else if ( nWhich > pImp->nVerEnd )
            pImp->nVerEnd = nWhich;
    }
}

// -----------------------------------------------------------------------

USHORT SfxItemPool::GetNewWhich
(
    USHORT  nFileWhich      // die aus dem Stream geladene Which-Id
)   const

/*  [Beschreibung]

    Diese Methoden rechnet Which-Ids aus einem File-Format in die der
    aktuellen Pool-Version um. Ist das File-Format "alter, werden die vom
    Pool-Entwickler mit SetVersion() gesetzten Tabellen verwendet,
    ist das File-Format neuer, dann die aus dem File geladenen Tabellen.
    Im letzteren Fall kann ggf. nicht jede Which-Id gemappt werden,
    so da\s 0 zur"uckgeliefert wird.

    Die Berechnung ist nur f"ur Which-Ids definiert, die in der betreffenden
    File-Version unterst"utzt wurden. Dies ist per Assertion abgesichert.

    Precondition:   Pool mu\s geladen sein
    Postcondition:  unver"andert
    Laufzeit:       linear(Anzahl der Sekund"arpools) +
                    linear(Differenz zwischen alter und neuer Version)


    [Querverweise]

    <SfxItemPool::IsLoadingVersionCurrent()const>
    <SfxItemPool::SetVersionMap(USHORT,USHORT,USHORT,USHORT*)>
    <SfxItemPool::GetLoadingVersion()const>
*/

{
    // (Sekund"ar-) Pool bestimmen
    if ( !IsInVersionsRange(nFileWhich) )
    {
        if ( pSecondary )
            return pSecondary->GetNewWhich( nFileWhich );
        SFX_ASSERT( 0, nFileWhich, "unknown which in GetNewWhich()" );
    }

    // Version neuer/gleich/"alter?
    short nDiff = (short)pImp->nLoadingVersion - (short)pImp->nVersion;

    // Which-Id einer neueren Version?
    if ( nDiff > 0 )
    {
        // von der Top-Version bis runter zur File-Version stufenweise mappen
        for ( USHORT nMap = pImp->aVersions.Count(); nMap > 0; --nMap )
        {
            SfxPoolVersion_Impl *pVerInfo = pImp->aVersions[nMap-1];
            if ( pVerInfo->_nVer > pImp->nVersion )
            {   USHORT nOfs;
                USHORT nCount = pVerInfo->_nEnd - pVerInfo->_nStart + 1;
                for ( nOfs = 0;
                      nOfs <= nCount &&
                        pVerInfo->_pMap[nOfs] != nFileWhich;
                      ++nOfs )
                    continue;

                if ( pVerInfo->_pMap[nOfs] == nFileWhich )
                    nFileWhich = pVerInfo->_nStart + nOfs;
                else
                    return 0;
            }
            else
                break;
        }
    }

    // Which-Id einer neueren Version?
    else if ( nDiff < 0 )
    {
        // von der File-Version bis zur aktuellen Version stufenweise mappen
        for ( USHORT nMap = 0; nMap < pImp->aVersions.Count(); ++nMap )
        {
            SfxPoolVersion_Impl *pVerInfo = pImp->aVersions[nMap];
            if ( pVerInfo->_nVer > pImp->nLoadingVersion )
            {
                DBG_ASSERT( nFileWhich >= pVerInfo->_nStart &&
                            nFileWhich <= pVerInfo->_nEnd,
                            "which-id unknown in version" );
                nFileWhich = pVerInfo->_pMap[nFileWhich - pVerInfo->_nStart];
            }
        }
    }

    // originale (nDiff==0) bzw. gemappte (nDiff!=0) Id zur"uckliefern
    return nFileWhich;
}

// -----------------------------------------------------------------------


bool SfxItemPool::IsInVersionsRange( USHORT nWhich ) const
{
    return nWhich >= pImp->nVerStart && nWhich <= pImp->nVerEnd;
}

// -----------------------------------------------------------------------

bool SfxItemPool::IsCurrentVersionLoading() const

/*  [Beschreibung]

    Mit dieser Methode kann festgestellt werden, ob die geladene Pool-Version
    dem aktuellen Pool-Aufbau entspricht.

    Precondition:   Pool mu\s geladen sein
    Postcondition:  unver"andert
    Laufzeit:       linear(Anzahl der Sekund"arpools)


    [Querverweise]

    <SfxItemPool::SetVersionMap(USHORT,USHORT,USHORT,USHORT*)>
    <SfxItemPool::GetNewWhich(USHORT)const>
    <SfxItemPool::GetLoadingVersion()const>
*/

{
    return ( pImp->nVersion == pImp->nLoadingVersion ) &&
           ( !pSecondary || pSecondary->IsCurrentVersionLoading() );
}

// -----------------------------------------------------------------------

USHORT SfxItemPool::GetLoadingVersion() const

/*  [Beschreibung]

    Diese Methode liefert die Versionsnummer des SfxItemPool-Aufbaus
    (also des Which-Bereichs), die bei Laden vorgefunden wurde.

    Precondition:   Pool mu\s geladen sein
    Postcondition:  unver"andert
    Laufzeit:       2


    [Anmerkung]

    Achtung: Es mu\s ggf. die Versionsnummer von Sekund"arpools
    ber"ucksichtigt werden.


    [Querverweise]

    <SfxItemPool::IsLoadingVersionCurrent()const>
    <SfxItemPool::SetVersionMap(USHORT,USHORT,USHORT,USHORT*)>
    <SfxItemPool::GetNewWhich(USHORT)const>
*/

{
    return pImp->nLoadingVersion;
}

//-------------------------------------------------------------------------

bool SfxItemPool::IsVer2_Impl() const
{
    return pMaster->pImp->nMajorVer >= 2;
}

//-------------------------------------------------------------------------

const SfxPoolItem* SfxItemPool::LoadItem( SvStream &rStream, bool bDirect,
                                          const SfxItemPool *pRefPool )

// pRefPool==-1 => nicht putten!

{
    USHORT nWhich=0, nSlot=0; // nSurrogate;
    rStream >> nWhich >> nSlot;

    BOOL bDontPut = (SfxItemPool*)-1 == pRefPool;
    if ( bDontPut || !pRefPool )
        pRefPool = this;

    // richtigen Sekund"ar-Pool finden
    while ( !pRefPool->IsInVersionsRange(nWhich) )
    {
        if ( pRefPool->pSecondary )
            pRefPool = pRefPool->pSecondary;
        else
        {
            // WID in der Version nicht vorhanden => ueberspringen
            USHORT nSurro(0), nVersion(0), nLen(0);
            rStream >> nSurro;
            if ( SFX_ITEMS_DIRECT == nSurro )
            {
                rStream >> nVersion >> nLen;
                rStream.SeekRel( nLen );
            }
            return 0;
        }
    }

    // wird eine andere Version geladen?
    bool bCurVersion = pRefPool->IsCurrentVersionLoading();
    if ( !bCurVersion )
        // Which-Id auf neue Version mappen
        nWhich = pRefPool->GetNewWhich( nWhich );

    DBG_ASSERT( !nWhich || !pImp->bInSetItem ||
                !pRefPool->ppStaticDefaults[pRefPool->GetIndex_Impl(nWhich)]->ISA(SfxSetItem),
                "loading SetItem in ItemSet of SetItem" );

    // soll "uber Surrogat geladen werden?
    const SfxPoolItem *pItem = 0;
    if ( !bDirect )
    {
        // Which-Id in dieser Version bekannt?
        if ( nWhich )
            // Surrogat laden, reagieren falls keins vorhanden
            pItem = LoadSurrogate( rStream, nWhich, nSlot, pRefPool );
        else
            // sonst "uberspringen
            rStream.SeekRel( sizeof(USHORT) );
    }

    // wird direkt, also nicht "uber Surrogat geladen?
    if ( bDirect || ( nWhich && !pItem ) )
    {
        // bDirekt bzw. nicht IsPoolable() => Item direkt laden
        USHORT nVersion(0);
        sal_uInt32 nLen(0);
        rStream >> nVersion >> nLen;
        ULONG nIStart = rStream.Tell();

        // Which-Id in dieser Version bekannt?
        if ( nWhich )
        {
            // Item direkt laden
            SfxPoolItem *pNewItem =
                    pRefPool->GetDefaultItem(nWhich).Create(rStream, nVersion);
            if ( bDontPut )
                pItem = pNewItem;
            else
                if ( pNewItem )
                {
                    pItem = &Put(*pNewItem);
                    delete pNewItem;
                }
                else
                    pItem = 0;
            ULONG nIEnd = rStream.Tell();
            DBG_ASSERT( nIEnd <= (nIStart+nLen), "read past end of item" );
            if ( (nIStart+nLen) != nIEnd )
                rStream.Seek( nIStart+nLen );
        }
        else
            // Item "uberspringen
            rStream.Seek( nIStart+nLen );
    }

    return pItem;
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
