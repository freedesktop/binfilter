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
#ifndef _SFXMACITEM_HXX
#define _SFXMACITEM_HXX

#include <bf_svtools/bf_solar.h>

// class SvxMacroItem ----------------------------------------------------


#include <bf_svtools/poolitem.hxx>

#include <tools/rtti.hxx>

#include <tools/debug.hxx>

#include <bf_tools/string.hxx>

#include <bf_tools/table.hxx>

class SvStream;

namespace binfilter
{

#define SVX_MACRO_LANGUAGE_JAVASCRIPT "JavaScript"
#define SVX_MACRO_LANGUAGE_STARBASIC "StarBasic"
#define SVX_MACRO_LANGUAGE_SF "Script"

DBG_NAMEEX(SvxMacroItem)

// -----------------------------------------------------------------------

//Ein Macro

enum ScriptType
{
    STARBASIC,
    JAVASCRIPT,
    EXTENDED_STYPE
};

// Basisklasse fuer SjJSbxObject mit virtuellem Destruktor
class SjJSbxObjectBase
{
public:
    virtual ~SjJSbxObjectBase();
    virtual SjJSbxObjectBase* Clone( void );
    //virtual SjJSbxObjectBase& operator=( const SjJSbxObjectBase& rBase );
};

class SjJSbxObject;

class  SvxMacro
{
    String aMacName;
    String aLibName;
    // Fuer JavaScript muss ein Function-Objekt gehalten werden
    SjJSbxObjectBase* pFunctionObject;
    ScriptType eType;

public:

    SvxMacro( const String &rMacName, const String &rLanguage);

    SvxMacro( const String &rMacName, const String &rLibName,
                ScriptType eType); //  = STARBASIC entfernt

    SvxMacro( SjJSbxObjectBase* _pFunctionObject, const String &rSource );
    ~SvxMacro();    // noetig fuer pFunctionObject

    const String &GetLibName() const        { return aLibName; }
    const String &GetMacName() const        { return aMacName; }

    ScriptType GetScriptType() const        { return eType; }

    BOOL HasMacro() const           { return aMacName.Len() ? TRUE : FALSE; }

#ifdef SOLAR_JAVA
    // JavaScript-Function-Objekt holen
    // ACHTUNG: Implementation in SJ, Source/JScript/sjimpl.cxx
    SjJSbxObjectBase* GetFunctionObject( SjJSbxObject* pParent );
#endif

    SvxMacro& operator=( const SvxMacro& rBase );
};

inline SvxMacro::SvxMacro( const String &rMacName, const String &rLibName,
                            ScriptType eTyp )
    : aMacName( rMacName ), aLibName( rLibName ), pFunctionObject(NULL), eType( eTyp )
{}

inline SvxMacro::SvxMacro( SjJSbxObjectBase* _pFunctionObject, const String &rSource )
    : aMacName( rSource ), pFunctionObject( _pFunctionObject ), eType( JAVASCRIPT )
{}

//Macro Table, zerstoert die Pointer im DTor!

DECLARE_TABLE( _SvxMacroTableDtor, SvxMacro* )

#define SVX_MACROTBL_VERSION31  0
#define SVX_MACROTBL_VERSION40  1

#define SVX_MACROTBL_AKTVERSION SVX_MACROTBL_VERSION40

class  SvxMacroTableDtor : public _SvxMacroTableDtor
{
public:
    inline SvxMacroTableDtor( const USHORT nInitSz = 0, const USHORT nReSz = 1 );
    inline SvxMacroTableDtor( const SvxMacroTableDtor &rCpy ) : _SvxMacroTableDtor() { *this = rCpy; }
    inline ~SvxMacroTableDtor() { DelDtor(); }
    SvxMacroTableDtor& operator=( const SvxMacroTableDtor &rCpy );

    // loescht alle Eintraege
    void DelDtor();

    SvStream&   Read( SvStream &, USHORT nVersion = SVX_MACROTBL_AKTVERSION );

    USHORT GetVersion() const       { return SVX_MACROTBL_AKTVERSION; }
};

inline SvxMacroTableDtor::SvxMacroTableDtor( const USHORT nInitSz,
                                             const USHORT nReSz)
    : _SvxMacroTableDtor( nInitSz, nReSz )
{}

/*
[Beschreibung]
Dieses Item beschreibt eine Makro-Tabelle.
*/

class  SvxMacroItem: public SfxPoolItem
{
public:
    TYPEINFO();

    inline SvxMacroItem ( const USHORT nId /*= ITEMID_MACRO*/ );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    XubString &rText,
                                    const ::IntlWrapper * = 0 ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, USHORT) const;
    virtual SvStream& Store(SvStream &rStream, USHORT) const { return rStream; }
    virtual USHORT          GetVersion( USHORT nFileFormatVersion ) const;

    inline const SvxMacroTableDtor& GetMacroTable() const { return aMacroTable;}
    inline void SetMacroTable( const SvxMacroTableDtor& rTbl ) { aMacroTable = rTbl; }

    inline const SvxMacro& GetMacro( USHORT nEvent ) const;
    inline BOOL HasMacro( USHORT nEvent ) const;
           void SetMacro( USHORT nEvent, const SvxMacro& );
    inline BOOL DelMacro( USHORT nEvent );

private:
    SvxMacroTableDtor aMacroTable;

    inline SvxMacroItem( const SvxMacroItem& );
    SvxMacroItem &operator=( const SvxMacroItem & );
};

inline SvxMacroItem::SvxMacroItem( const USHORT nId )
    : SfxPoolItem( nId )
{}
inline SvxMacroItem::SvxMacroItem( const SvxMacroItem &rCpy )
    : SfxPoolItem( rCpy ),
    aMacroTable( rCpy.GetMacroTable() )
{}

inline BOOL SvxMacroItem::HasMacro( USHORT nEvent ) const
{
    return aMacroTable.IsKeyValid( nEvent );
}
inline const SvxMacro& SvxMacroItem::GetMacro( USHORT nEvent ) const
{
    return *(aMacroTable.Get(nEvent));
}
inline BOOL SvxMacroItem::DelMacro( USHORT nEvent )
{
    SvxMacro *pMacro = aMacroTable.Remove( nEvent );
    delete pMacro;
    return ( pMacro != 0 );
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
