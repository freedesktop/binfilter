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

#include <bf_svtools/filerec.hxx>
#include <osl/endian.h>

namespace binfilter
{

//========================================================================

SV_IMPL_VARARR( SfxUINT32s, UINT32 );

//========================================================================

/*  Die folgenden Makros extrahieren Teilbereiche aus einem UINT32 Wert.
    Diese UINT32-Werte werden anstelle der einzelnen Werte gestreamt,
    um Calls zu sparen.
*/

#define SFX_REC_PRE(n) ( ((n) & 0x000000FF) )
#define SFX_REC_OFS(n) ( ((n) & 0xFFFFFF00) >> 8 )
#define SFX_REC_TYP(n) ( ((n) & 0x000000FF) )
#define SFX_REC_VER(n) ( ((n) & 0x0000FF00) >> 8 )
#define SFX_REC_TAG(n) ( ((n) & 0xFFFF0000) >> 16 )

#define SFX_REC_CONTENT_VER(n) ( ((n) & 0x000000FF) )
#define SFX_REC_CONTENT_OFS(n) ( ((n) & 0xFFFFFF00) >> 8 )

//-------------------------------------------------------------------------

/*  Die folgenden Makros setzen Teilbereiche zu einem UINT32 Wert zusammen.
    Diese UINT32-Werte werden anstelle der einzelnen Werte gestreamt,
    um Calls zu sparen.
*/

#define SFX_REC_HEADER(nRecType,nContentTag,nContentVer) \
                    ( UINT32(nRecType) | \
                      ( UINT32(nContentVer) << 8 ) | \
                      ( UINT32(nContentTag) << 16 ) )

//=========================================================================

bool SfxMiniRecordReader::SetHeader_Impl( UINT32 nHeader )

/*  [Beschreibung]

    Interne Methode zum nachtr"aglichen Verarbeiten eines extern gelesenen
    Headers. Falls der Header eine End-Of-Records-Kennung darstellt,
    wird am Stream ein Errorcode gesetzt und FALSE zur"uckgeliefert. Im
    Fehlerfall wird der Stream jedoch nicht auf den Record-Anfang zur"uck-
    gesetzt.
*/

{
    bool bRet = true;

    // Record-Ende und Pre-Tag aus dem Header ermitteln
    _nEofRec = _pStream->Tell() + SFX_REC_OFS(nHeader);
    _nPreTag = sal::static_int_cast< BYTE >(SFX_REC_PRE(nHeader));

    // wenn End-Of-Record-Kennung, dann Fehler
    if ( _nPreTag == SFX_REC_PRETAG_EOR )
    {
        _pStream->SetError( ERRCODE_IO_WRONGFORMAT );
        bRet = false;
    }
    return bRet;
}

//-------------------------------------------------------------------------

SfxMiniRecordReader::SfxMiniRecordReader
(
    SvStream*       pStream,        /*  <SvStream>, an dessen aktueller
                                        Position sich ein <SfxMiniRecord>
                                        befindet.
                                    */
    BYTE            nTag            //  Pre-Tag des gew"unschten Records
)

/*  [Beschreibung]

    Dieser Ctor interpretiert 'pStream' ab der aktuellen Position als
    eine l"uckenlose Folge von, von dieser Klassen-Gruppe interpretierbaren,
    Records. Der in dieser Folge erste als <SfxMiniRecord> interpretierbare
    (also ggf. auch ein extended-Record) mit dem PreTag 'nTag' wird ge"offnet
    und durch diese Instanz repr"asentiert.

    Wird das Ende des Streams oder die Kennung SFX_REC_PRETAG_EOR
    erreicht, bevor ein Record mit dem ge"unschten Pre-Tag gefunden wird,
    ist die erzeugte Instanz ung"ultig ('IsValid() == FALSE'). Ein ent-
    sprechender Error-Code (ERRCODE_IO_EOF bzw. ERRCODE_IO_WRONGFORMAT)
    ist dann am Stream gesetzt, dessen Position ist dann au\serdem unver-
    "andert.

    Bei 'nTag==SFX_FILEREC_PRETAG_EOR' wird nicht versucht, einen Record
    zu lesen, es wird sofort 'IsValid()' auf FALSE gesetzt und kein Error-Code
    am Stream gesetzt. Dies ist dauzu gedacht, ohne 'new' und 'delete'
    abw"rtskompatibel SfxMiniRecords einbauen zu k"onnen. Siehe dazu
    <SfxItemSet::Load()>.


    [Anwendungsvorschlag]

    Wird dieser Ctor in einer bereits ausgelieferten Programmversion
    verwendet, k"onnen in das File-Format jeweils davor kompatibel neue
    Records mit einer anderen Kennung eingef"ugt werden. Diese werden
    schlie\slich automatisch "uberlesen. Erkauft wird diese M"oglichkeit
    allerdings mit etwas schlechterem Laufzeitverhalten im Vergleich mit
    direktem 'drauf-los-lesen', der sich jedoch auf einen Vergleich zweier
    Bytes reduziert, falls der gesuchte Record der erste in der Folge ist.
*/

:   _pStream( pStream ),
    _bSkipped( nTag == SFX_REC_PRETAG_EOR )
{
    // ggf. ignorieren (s.o.)
    if ( _bSkipped )
    {
        _nPreTag = nTag;
        return;
    }

    // StartPos merken, um im Fehlerfall zur"uck-seeken zu k"onnen
    UINT32 nStartPos = pStream->Tell();

    // passenden Record suchen
    while(TRUE)
    {
        // Header lesen
        DBG( DbgOutf( "SfxFileRec: searching record at %ul", pStream->Tell() ) );
        UINT32 nHeader(0);
        *pStream >> nHeader;

        // Headerdaten von Basisklasse extrahieren lassen
        SetHeader_Impl( nHeader );

        // ggf. Fehler behandeln
        if ( !pStream->good() )
            _nPreTag = SFX_REC_PRETAG_EOR;
        else if ( _nPreTag == SFX_REC_PRETAG_EOR )
            pStream->SetError( ERRCODE_IO_WRONGFORMAT );
        else
        {
            // wenn gefunden, dann Schleife abbrechen
            if ( _nPreTag == nTag )
                break;

            // sonst skippen und weitersuchen
            pStream->Seek( _nEofRec );
            continue;
        }

        // Fehler => zur"uck-seeken
        pStream->Seek( nStartPos );
        break;
    }
}

//=========================================================================

inline bool SfxSingleRecordReader::ReadHeader_Impl( USHORT nTypes )

/*  [Beschreibung]

    Interne Methode zum Einlesen eines SfxMultiRecord-Headers, nachdem
    die Basisklasse bereits initialisiert und deren Header gelesen ist.
    Ggf. ist ein Error-Code am Stream gesetzt, im Fehlerfall wird jedoch
    nicht zur"uckge-seekt.
*/

{
    bool bRet;

    // Basisklassen-Header einlesen
    UINT32 nHeader(0);
    *_pStream >> nHeader;
    if ( !SetHeader_Impl( nHeader ) )
        bRet = FALSE;
    else
    {
        // eigenen Header einlesen
        *_pStream >> nHeader;
        _nRecordVer = sal::static_int_cast< BYTE >(SFX_REC_VER(nHeader));
        _nRecordTag = sal::static_int_cast< UINT16 >(SFX_REC_TAG(nHeader));

        // falscher Record-Typ?
        _nRecordType = sal::static_int_cast< BYTE >(SFX_REC_TYP(nHeader));
        bRet = 0 != ( nTypes & _nRecordType);
    }
    return bRet;
}

//-------------------------------------------------------------------------

SfxSingleRecordReader::SfxSingleRecordReader( SvStream *pStream, USHORT nTag )
{
    // StartPos merken, um im Fehlerfall zur"uck-seeken zu k"onnen
    UINT32 nStartPos = pStream->Tell();

    // richtigen Record suchen, ggf. Error-Code setzen und zur"uck-seeken
    Construct_Impl( pStream );
    if ( !FindHeader_Impl( SFX_REC_TYPE_SINGLE, nTag ) )
    {
        // Error-Code setzen und zur"uck-seeken
        pStream->Seek( nStartPos );
        pStream->SetError( ERRCODE_IO_WRONGFORMAT );
    }
}

//-------------------------------------------------------------------------

bool SfxSingleRecordReader::FindHeader_Impl
(
    UINT16      nTypes,     // arithm. Veroderung erlaubter Record-Typen
    UINT16      nTag        // zu findende Record-Art-Kennung
)

/*  [Beschreibung]

    Interne Methode zum lesen des Headers des ersten Record, der einem
    der Typen in 'nTypes' entspricht und mit der Art-Kennung 'nTag'
    gekennzeichnet ist.

    Kann ein solcher Record nicht gefunden werden, wird am Stream ein
    Errorcode gesetzt, zur"uck-geseekt und FALSE zur"uckgeliefert.
*/

{
    // StartPos merken, um im Fehlerfall zur"uck-seeken zu k"onnen
    UINT32 nStartPos = _pStream->Tell();

    // richtigen Record suchen
    while ( _pStream->good() )
    {
        // Header lesen
        UINT32 nHeader(0);
        DBG( DbgOutf( "SfxFileRec: searching record at %ul", _pStream->Tell() ) );
        *_pStream >> nHeader;
        if ( !_pStream->good() || !SetHeader_Impl( nHeader ) )
            // EOR => Such-Schleife abbreichen
            break;

        // Extended Record gefunden?
        if ( _nPreTag == SFX_REC_PRETAG_EXT )
        {
            // Extended Header lesen
            *_pStream >> nHeader;
            if ( !_pStream->good() )
                break;
            _nRecordTag = sal::static_int_cast< UINT16 >(SFX_REC_TAG(nHeader));

            // richtigen Record gefunden?
            if ( _nRecordTag == nTag )
            {
                // gefundener Record-Typ passend?
                _nRecordType = sal::static_int_cast< BYTE >(
                    SFX_REC_TYP(nHeader));
                if ( nTypes & _nRecordType )
                    // ==> gefunden
                    return TRUE;

                // error => Such-Schleife abbrechen
                break;
            }
        }

        // sonst skippen
        if ( _pStream->good() )
            _pStream->Seek( _nEofRec );
    }

    // Fehler setzen und zur"uck-seeken
    _pStream->SetError( ERRCODE_IO_WRONGFORMAT );
    _pStream->Seek( nStartPos );
    return FALSE;
}

//=========================================================================

bool SfxMultiRecordReader::ReadHeader_Impl()

/*  [Beschreibung]

    Interne Methode zum Einlesen eines SfxMultiRecord-Headers, nachdem
    die Basisklasse bereits initialisiert und deren Header gelesen ist.
    Ggf. ist ein Error-Code am Stream gesetzt, im Fehlerfall wird jedoch
    nicht zur"uckge-seekt.
*/

{
    // eigenen Header lesen
    *_pStream >> _nContentCount;
    *_pStream >> _nContentSize; // Fix: jedes einzelnen, Var|Mix: Tabellen-Pos.

    // mu\s noch eine Tabelle mit Content-Offsets geladen werden?
    if ( _nRecordType != SFX_REC_TYPE_FIXSIZE )
    {
        // Tabelle aus dem Stream einlesen
        UINT32 nContentPos = _pStream->Tell();
        if ( _nRecordType == SFX_REC_TYPE_VARSIZE_RELOC ||
             _nRecordType == SFX_REC_TYPE_MIXTAGS_RELOC )
            _pStream->SeekRel( + _nContentSize );
        else
            _pStream->Seek( _nContentSize );
        _pContentOfs = new UINT32[_nContentCount];
        memset(_pContentOfs, 0, _nContentCount*sizeof(UINT32));
        //! darf man jetzt so einr"ucken
        #if defined(OSL_LITENDIAN)
        _pStream->Read( _pContentOfs, sizeof(UINT32)*_nContentCount );
        #else
        for ( USHORT n = 0; n < _nContentCount; ++n )
            *_pStream >> _pContentOfs[n];
        #endif
        _pStream->Seek( nContentPos );
    }

    // Header konnte gelesen werden, wenn am Stream kein Error gesetzt ist
    return !_pStream->GetError();
}

//-------------------------------------------------------------------------

SfxMultiRecordReader::SfxMultiRecordReader( SvStream *pStream, UINT16 nTag )
    : _pContentOfs(0)
    , _nContentSize(0)
    , _nContentCount(0)
    , _nContentNo(0)
{
    // Position im Stream merken, um im Fehlerfall zur"uck-seeken zu k"onnen
    _nStartPos = pStream->Tell();

    // passenden Record suchen und Basisklasse initialisieren
    SfxSingleRecordReader::Construct_Impl( pStream );
    if ( SfxSingleRecordReader::FindHeader_Impl( SFX_REC_TYPE_FIXSIZE |
            SFX_REC_TYPE_VARSIZE | SFX_REC_TYPE_VARSIZE_RELOC |
            SFX_REC_TYPE_MIXTAGS | SFX_REC_TYPE_MIXTAGS_RELOC,
            nTag ) )
    {
        // eigenen Header dazu-lesen
        if ( !ReadHeader_Impl() )
            // nicht lesbar => als ung"ultig markieren und zur"uck-seeken
            SetInvalid_Impl( _nStartPos);
    }
}

//-------------------------------------------------------------------------

SfxMultiRecordReader::~SfxMultiRecordReader()
{
    delete[] _pContentOfs;
}

//-------------------------------------------------------------------------

bool SfxMultiRecordReader::GetContent()

/*  [Beschreibung]

    Positioniert den Stream an den Anfang des n"chsten bzw. beim 1. Aufruf
    auf den Anfang des ersten Contents im Record und liest ggf. dessen
    Header ein.

    Liegt laut Record-Header kein Content mehr vor, wird FALSE zur"uck-
    gegeben. Trotz einem TRUE-Returnwert kann am Stream ein Fehlercode
    gesetzt sein, z.B. falls er unvorhergesehenerweise (kaputtes File)
    zuende ist.
*/

{
    // noch ein Content vorhanden?
    if ( _nContentNo < _nContentCount )
    {
        // den Stream an den Anfang des Contents positionieren
        UINT32 nOffset = _nRecordType == SFX_REC_TYPE_FIXSIZE
                    ? _nContentNo * _nContentSize
                    : SFX_REC_CONTENT_OFS(_pContentOfs[_nContentNo]);
        UINT32 nNewPos = _nStartPos + nOffset;
        DBG_ASSERT( nNewPos >= _pStream->Tell(), "SfxMultiRecordReader::GetContent() - New position before current, to much data red!" );

        // correct stream pos in every case;
        // the if clause was added by MT  a long time ago,
        // maybe to 'repair' other corrupt documents; but this
        // gives errors when writing with 5.1 and reading with current
        // versions, so we decided to remove the if clause
        // if ( nNewPos > _pStream->Tell() )
        _pStream->Seek( nNewPos );

        // ggf. Content-Header lesen
        if ( _nRecordType == SFX_REC_TYPE_MIXTAGS ||
             _nRecordType == SFX_REC_TYPE_MIXTAGS_RELOC )
        {
            _nContentVer = sal::static_int_cast< BYTE >(
                SFX_REC_CONTENT_VER(_pContentOfs[_nContentNo]));
            *_pStream >> _nContentTag;
        }

        // ContentNo weiterz"ahlen
        ++_nContentNo;
        return TRUE;
    }

    return FALSE;
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
