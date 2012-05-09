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

#ifndef _SFXFILEREC_HXX
#define _SFXFILEREC_HXX

#include <bf_svtools/bf_solar.h>

#include <tools/debug.hxx>

#include <tools/stream.hxx>

#include <bf_svtools/svarray.hxx>

namespace binfilter
{

SV_DECL_VARARR( SfxUINT32s, UINT32, 8, 8 )

//------------------------------------------------------------------------

#define SFX_REC_PRETAG_EXT              BYTE(0x00)  // Pre-Tag f"ur Extended-Records
#define SFX_REC_PRETAG_EOR              BYTE(0xFF)  // Pre-Tag f"ur End-Of-Records

#define SFX_REC_TYPE_NONE               BYTE(0x00)  // unbekannter Record-Typ
#define SFX_REC_TYPE_FIRST              BYTE(0x01)
#define SFX_REC_TYPE_SINGLE             BYTE(0x01)  // Single-Content-Record
#define SFX_REC_TYPE_FIXSIZE            BYTE(0x02)  // Fix-Size-Multi-Content-Record
#define SFX_REC_TYPE_VARSIZE_RELOC      BYTE(0x03)  // variable Rec-Size
#define SFX_REC_TYPE_VARSIZE            BYTE(0x04)  // alt (nicht verschiebbar)
#define SFX_REC_TYPE_MIXTAGS_RELOC      BYTE(0x07)  // Mixed Tag Content-Record
#define SFX_REC_TYPE_MIXTAGS            BYTE(0x08)  // alt (nicht verschiebbar)
#define SFX_REC_TYPE_LAST               BYTE(0x08)
#define SFX_REC_TYPE_MINI                   0x100   // Mini-Record
#define SFX_REC_TYPE_DRAWENG                0x400   // Drawing-Engine-Record
#define SFX_REC_TYPE_EOR                    0xF00   // End-Of-Records

//------------------------------------------------------------------------

#define SFX_REC_HEADERSIZE_MINI     4   // Gr"o\se des Mini-Record-Headers
#define SFX_REC_HEADERSIZE_SINGLE   4   // zzgl. HEADERSIZE_MINI => 8
#define SFX_REC_HEADERSIZE_MULTI    6   // zzgl. HEADERSIZE_SINGLE => 14

//------------------------------------------------------------------------

#ifndef DBG
#ifdef DBG_UTIL
#define DBG(x) x
#else
#define DBG(x)
#endif
#endif

//------------------------------------------------------------------------

class  SfxMiniRecordReader

/*  [Beschreibung]

    Mit Instanzen dieser Klasse kann ein einfacher Record aus einem Stream
    gelesen werden, der mit der Klasse <SfxRecordWriter> geschrieben wurde.

    Es ist auch m"oglich, den Record zu "uberspringen, ohne sein internes
    Format zu kennen.

    [Beispiel]

    {
        SfxMiniRecordReader aRecord( pStream );
        switch ( aRecord.GetTag() )
        {
            case MY_TAG_X:
                *aRecord >> aMember1;
                *aRecord >> aMember2;
                break;

            ...
        }
    }
*/

{
protected:
    SvStream*           _pStream;   //  <SvStream>, aus dem gelesen wird
    UINT32              _nEofRec;   //  Position direkt hinter dem Record
    bool            _bSkipped;  //  TRUE: der Record wurde explizit geskippt
    BYTE                _nPreTag;   //  aus dem Header gelesenes Pre-Tag

                        // Drei-Phasen-Ctor f"ur Subklassen
                        SfxMiniRecordReader() {}
   void                 Construct_Impl( SvStream *pStream, BYTE nTag )
                        {
                            _pStream = pStream;
                            _bSkipped = FALSE;
                            _nPreTag = nTag;
                        }
    inline bool     SetHeader_Impl( UINT32 nHeader );

                        // als ung"ultig markieren und zur"uck-seeken
    void                SetInvalid_Impl( UINT32 nRecordStartPos )
                        {
                            _nPreTag = SFX_REC_PRETAG_EOR;
                            _pStream->Seek( nRecordStartPos );
                        }

public:
                        SfxMiniRecordReader( SvStream *pStream, BYTE nTag );
    inline              ~SfxMiniRecordReader();

    inline BYTE         GetTag() const;
    inline bool     IsValid() const;

    inline SvStream&    operator*() const;

    inline void         Skip();

private:
                        // not implementend, not allowed
                        SfxMiniRecordReader( const SfxMiniRecordReader& );
    SfxMiniRecordReader& operator=(const SfxMiniRecordReader&);
};

//------------------------------------------------------------------------

class  SfxSingleRecordReader: public SfxMiniRecordReader

/*  [Beschreibung]

    Es ist auch m"oglich, den Record zu "uberspringen, ohne sein internes
    Format zu kennen.

    [Beispiel]

    {
        SfxSingleRecordReader aRecord( pStream );
        switch ( aRecord.GetTag() )
        {
            case MY_TAG_X:
                aRecord >> aMember1;
                if ( aRecord.HasVersion(2) )
                    *aRecord >> aMember2;
                break;

            ...
        }
    }
*/

{
protected:
    UINT16              _nRecordTag;    // Art des Gesamt-Inhalts
    BYTE                _nRecordVer;    // Version des Gesamt-Inhalts
    BYTE                _nRecordType;   // Record Type aus dem Header

                        // Drei-Phasen-Ctor f"ur Subklassen
                        SfxSingleRecordReader() {}
    void                Construct_Impl( SvStream *pStream )
                        {
                            SfxMiniRecordReader::Construct_Impl(
                                    pStream, SFX_REC_PRETAG_EXT );
                        }
    bool            FindHeader_Impl( UINT16 nTypes, UINT16 nTag );
    bool            ReadHeader_Impl( USHORT nTypes );

public:
                        SfxSingleRecordReader( SvStream *pStream, USHORT nTag );

    inline UINT16       GetTag() const;

    inline BYTE         GetVersion() const;
    inline bool     HasVersion( USHORT nVersion ) const;
};

//------------------------------------------------------------------------

class  SfxMultiRecordReader: public SfxSingleRecordReader

/*  [Beschreibung]

    Es ist auch m"oglich, den Record oder einzelne Contents zu "uberspringen,
    ohne das jeweilis interne Format zu kennen.

    [Beispiel]

    {
        SfxMultiRecordReader aRecord( pStream );
        for ( USHORT nRecNo = 0; aRecord.GetContent(); ++nRecNo )
        {
            switch ( aRecord.GetTag() )
            {
                case MY_TAG_X:
                    X *pObj = new X;
                    *aRecord >> pObj.>aMember1;
                    if ( aRecord.HasVersion(2) )
                        *aRecord >> pObj->aMember2;
                    Append( pObj );
                    break;

                ...
            }
        }
    }
*/

{
    UINT32              _nStartPos;     //  Start-Position des Records
    UINT32*             _pContentOfs;   //  Offsets der Startpositionen
    UINT32              _nContentSize;  //  Size jedes einzelnen / Tabellen-Pos
    UINT16              _nContentCount; //  Anzahl der Contents im Record
    UINT16              _nContentNo;    /*  der Index des aktuellen Contents
                                            enth"alt jeweils den Index des
                                            Contents, der beim n"achsten
                                            GetContent() geholt wird */
    UINT16              _nContentTag;   //  Art-Kennung des aktuellen Contents
    BYTE                _nContentVer;   //  Versions-Kennung des akt. Contents

    bool            ReadHeader_Impl();

public:
                        SfxMultiRecordReader( SvStream *pStream, UINT16 nTag );
                        ~SfxMultiRecordReader();

    bool            GetContent();
    inline UINT16       GetContentTag();
    inline BYTE         GetContentVersion() const;
    inline bool     HasContentVersion( USHORT nVersion ) const;

    inline UINT32       ContentCount() const;
};

//=========================================================================

inline SfxMiniRecordReader::~SfxMiniRecordReader()

/*  [Beschreibung]

    Der Dtor der Klasse <SfxMiniRecordReader> positioniert den Stream
    automatisch auf die Position direkt hinter dem Record, falls nicht
    <SfxMiniRecordReader::Skip()> bereits explizit gerufen wurde.
*/

{
    // noch nicht explizit ans Ende gesprungen?
    if ( !_bSkipped )
        Skip();
}

//-------------------------------------------------------------------------

inline void SfxMiniRecordReader::Skip()

/*  [Beschreibung]

    Mit dieser Methode wird der Stream direkt hinter das Ende des Records
    positioniert.
*/

{
    _pStream->Seek(_nEofRec);
    _bSkipped = TRUE;
}

//-------------------------------------------------------------------------

inline BYTE SfxMiniRecordReader::GetTag() const

/*  [Beschreibung]

    Liefert des aus dem Header gelesene Pre-Tag des Records. Dieses kann
    auch SFX_REC_PRETAG_EXT oder SFX_REC_PRETAG_EOR sein, im
    letzteren Fall ist am Stream der Fehlercode ERRCODE_IO_WRONGFORMAT
    gesetzt. SFX_REC_PRETAG_EXT ist g"ultig, da diese extended-Records
    nur eine Erweiterung des SfxMiniRecord darstellen.
*/

{
    return _nPreTag;
}

//-------------------------------------------------------------------------

inline bool SfxMiniRecordReader::IsValid() const

/*  [Beschreibung]

    Hiermit kann abgefragt werden, ob der Record erfolgreich aus dem
    Stream konstruiert werden konnte, der Header also f"ur diesen Record-Typ
    passend war.
*/

{
    return _nPreTag != SFX_REC_PRETAG_EOR;
}

//-------------------------------------------------------------------------

inline SvStream& SfxMiniRecordReader::operator*() const

/*  [Beschreibung]

    Dieser Operator liefert den Stream in dem der Record liegt.
    Die aktuelle Position des Streams mu\s innerhalb des Records liegen.
*/

{
    DBG_ASSERT( _pStream->Tell() < _nEofRec, "read behind record" );
    return *_pStream;
}

//=========================================================================

inline UINT16 SfxSingleRecordReader::GetTag() const

/*  [Beschreibung]

    Liefert des aus dem Header gelesene Tag f"ur den Gesamt-Record.
*/

{
    return _nRecordTag;
}

//-------------------------------------------------------------------------

inline BYTE SfxSingleRecordReader::GetVersion() const

/*  [Beschreibung]

    Liefert die Version des aus dem Stream gelesenen Records.
*/

{
    return _nRecordVer;
}

//-------------------------------------------------------------------------

inline bool SfxSingleRecordReader::HasVersion( USHORT nVersion ) const

/*  [Beschreibung]

    Stellt fest, ob der aus dem Stream gelese Record in der Version
    'nVersion' oder h"oher vorliegt.
*/

{
    return _nRecordVer >= nVersion;
}

//=========================================================================

inline UINT16 SfxMultiRecordReader::GetContentTag()

/*  [Beschreibung]

    Diese Methode liefert die Art-Kennung des zuletzt mit der Methode
    <SfxMultiRecordReder::GetContent()> ge"offneten Contents.
*/

{
    return _nContentTag;
}

//-------------------------------------------------------------------------

inline BYTE SfxMultiRecordReader::GetContentVersion() const

/*  [Beschreibung]

    Diese Methode liefert die Version-Kennung des zuletzt mit der Methode
    <SfxMultiRecordReder::GetContent()> ge"offneten Contents.
*/

{
    return _nContentVer;
}

//-------------------------------------------------------------------------

inline bool SfxMultiRecordReader::HasContentVersion( USHORT nVersion ) const

/*  [Beschreibung]

    Diese Methode stellt fest, ob die Version 'nVersion' in der Version des
    zuletzt mit der Methode <SfxMultiRecordReder::GetContent()> ge"offneten
    Contents enthalten ist.
*/

{
    return _nContentVer >= nVersion;
}

//-------------------------------------------------------------------------

inline UINT32 SfxMultiRecordReader::ContentCount() const

/*  [Beschreibung]

    Diese Methode liefert die Anzahl im Record befindlichen Contents.
*/

{
    return _nContentCount;
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
