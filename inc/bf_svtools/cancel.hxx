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
#ifndef _SFXCANCEL_HXX
#define _SFXCANCEL_HXX

#include <bf_svtools/bf_solar.h>

#include <bf_tools/string.hxx>
#include <tools/ref.hxx>
#include <bf_svtools/brdcst.hxx>
#include <bf_svtools/smplhint.hxx>

namespace binfilter {

class SfxCancellable;

#ifdef _SFX_CANCEL_CXX
#include <bf_svtools/svarray.hxx>
SV_DECL_PTRARR( SfxCancellables_Impl, SfxCancellable*, 0, 4 )
#else
typedef SvPtrarr SfxCancellables_Impl;
#endif

//-------------------------------------------------------------------------

class SvWeakBase;
class SvWeakHdl : public SvRefBase
{
    friend class SvWeakBase;
    SvWeakBase* _pObj;
public:
    void ResetWeakBase( ) { _pObj = 0; }
private:
    SvWeakHdl( SvWeakBase* pObj ) : _pObj( pObj ) {}
public:
    SvWeakBase* GetObj() { return _pObj; }
};

SV_DECL_IMPL_REF( SvWeakHdl )

class SvWeakBase
{
    SvWeakHdlRef _xHdl;
public:
    SvWeakHdl* GetHdl() { return _xHdl; }

    // Wg CompilerWarnung nicht ueber Initializer
    SvWeakBase() { _xHdl = new SvWeakHdl( this ); }
    ~SvWeakBase() { _xHdl->ResetWeakBase(); }
};

#define SV_DECL_WEAK( ClassName )                                   \
class ClassName##Weak                                               \
{                                                                   \
    SvWeakHdlRef _xHdl;                                             \
public:                                                             \
    inline               ClassName##Weak( ) {}                      \
    inline               ClassName##Weak( ClassName* pObj ) {       \
        if( pObj ) _xHdl = pObj->GetHdl(); }                        \
    inline void          Clear() { _xHdl.Clear(); }                 \
    inline ClassName##Weak& operator = ( ClassName * pObj ) {       \
        _xHdl = pObj ? pObj->GetHdl() : 0; return *this; }          \
    inline sal_Bool            Is() const {                         \
        return _xHdl.Is() && _xHdl->GetObj(); }                     \
    inline ClassName *     operator &  () const {                   \
        return (ClassName*) ( _xHdl.Is() ? _xHdl->GetObj() : 0 ); } \
    inline ClassName *     operator -> () const {                   \
        return (ClassName*) ( _xHdl.Is() ? _xHdl->GetObj() : 0 ); } \
    inline ClassName &     operator *  () const {                   \
        return *(ClassName*) _xHdl->GetObj(); }                     \
    inline operator ClassName * () const {                          \
        return (ClassName*) (_xHdl.Is() ? _xHdl->GetObj() : 0 ); }  \
};

class  SfxCancelManager: public SfxBroadcaster
, public SvWeakBase

/*  [Beschreibung]

    An Instanzen dieser Klasse k"onnen nebenl"aufige Prozesse angemeldet
    werden, um vom Benutzer abbrechbar zu sein. Werden abbrechbare
    Prozesse (Instanzen von <SfxCancellable>) an- oder abgemeldet, wird
    dies durch einen <SfxSimpleHint> mit dem Flag SFX_HINT_CANCELLABLE
    gebroadcastet.

    SfxCancelManager k"onnen hierarchisch angeordnet werden, so k"onnen
    z.B. Dokument-lokale Prozesse getrennt gecancelt werden.

    [Beispiel]

    SfxCancelManager *pMgr = new SfxCancelManager;
    StartListening( pMgr );
    pMailSystem->SetCancelManager( pMgr )
*/

{
    SfxCancelManager*       _pParent;
    SfxCancellables_Impl    _aJobs;

public:
                            SfxCancelManager( SfxCancelManager *pParent = 0 );
                            ~SfxCancelManager();

    BOOL                    CanCancel() const;
    void                    Cancel( BOOL bDeep );
    SfxCancelManager*       GetParent() const { return _pParent; }

    void                    InsertCancellable( SfxCancellable *pJob );
    void                    RemoveCancellable( SfxCancellable *pJob );
    USHORT                  GetCancellableCount() const
                            { return _aJobs.Count(); }
    SfxCancellable*         GetCancellable( USHORT nPos ) const
                            { return (SfxCancellable*) _aJobs[nPos]; }
};

SV_DECL_WEAK( SfxCancelManager )
//-------------------------------------------------------------------------

class  SfxCancellable

/*  [Beschreibung]

    Instanzen dieser Klasse werden immer an einem Cancel-Manager angemeldet,
    der dadurch dem Benutzer signalisieren kann, ob abbrechbare Prozesse
    vorhanden sind und der die SfxCancellable-Instanzen auf 'abgebrochen'
    setzen kann.

    Die im Ctor "ubergebene <SfxCancelManger>-Instanz mu\s die Instanz
    dieser Klasse "uberleben!

    [Beispiel]

    {
        SfxCancellable aCancel( pCancelMgr );
        while ( !aCancel && GetData() )
            Reschedule();
    }

*/

{
    SfxCancelManager*       _pMgr;
    BOOL                    _bCancelled;
    String                  _aTitle;

public:
                            SfxCancellable( SfxCancelManager *pMgr,
                                            const String &rTitle )
                            :   _pMgr( pMgr ),
                                _bCancelled( FALSE ),
                                _aTitle( rTitle )
                            { pMgr->InsertCancellable( this ); }

    virtual                 ~SfxCancellable();

    void                    SetManager( SfxCancelManager *pMgr );
    SfxCancelManager*       GetManager() const { return _pMgr; }

    virtual void            Cancel();
    BOOL                    IsCancelled() const { return _bCancelled; }
    operator                BOOL() const { return _bCancelled; }
    const String&           GetTitle() const { return _aTitle; }
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
