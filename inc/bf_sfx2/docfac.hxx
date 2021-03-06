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
#ifndef _SFX_OBJFAC_HXX
#define _SFX_OBJFAC_HXX

#include <bf_svtools/bf_solar.h>

#include <tools/rtti.hxx>
#include <bf_so3/factory.hxx>

// SFX_IMPL_MODULE_LIB
#include <osl/module.hxx>
#include <rtl/ustring.hxx>

#include <bf_sfx2/objsh.hxx>
#include <bf_sfx2/sfxdefs.hxx>
class SvFactory;
namespace binfilter {

class SfxObjectFactoryArr_Impl;
class SfxMedium;
class SfxFilter;
class SfxViewFactory;
struct SfxObjectFactory_Impl;
class SfxFilterContainer;
class SfxFactoryFilterContainer;
class SfxMenuBarManager;
class SfxBindings;

typedef SfxObjectShell *(*SfxObjectCtor)(SfxObjectCreateMode);

//====================================================================

typedef void (*SfxVoidFunc)();

#define SDT_SDT_DOCFACTPRIO     10      // Explorer
#define SDT_SW_DOCFACTPRIO      20      // Text-Dokument
#define SDT_SWW_DOCFACTPRIO     21      // Web-Dokument
#define SDT_SC_DOCFACTPRIO      30      // Tabellen-Dokument
#define SDT_SI_DOCFACTPRIO      40      // Impress-Dokument
#define SDT_SD_DOCFACTPRIO      41      // Draw-Dokument
#define SDT_SCH_DOCFACTPRIO     50      // Chart-Dokument
#define SDT_SMA_DOCFACTPRIO     60      // Math-Dokument
#define SDT_SIM_DOCFACTPRIO     70      // Image-Dokument
#define SDT_FRM_DOCFACTPRIO    100      // Rahmen-Dokument
#define SDT_MSG_DOCFACTPRIO    110      // Nachrichten-Dokument
#define SDT_SDB_DOCFACTPRIO    200      // Datenbank-Dokument

//====================================================================

class SfxObjectFactory: public SvFactory
{
private:
    USHORT                  nId;            // Factory-Id
    SfxObjectCtor           fnCreate;       // static Methode, die Ctor ruft
    const char*             pShortName;     // Objekt-Kurzname
    SfxObjectFactory_Impl*  pImpl;          // Zusatzdaten
    SfxObjectShellFlags     nFlags;

public:
                    TYPEINFO();

    SfxObjectFactory( const SvGlobalName &rName, const String &rClassName, CreateInstanceType funcCIT );
    ~SfxObjectFactory();

    void            Construct( USHORT nId, SfxObjectCtor fnCreateFnc,
                               SfxObjectShellFlags, const char *pShortName = 0 );
    SfxObjectShell* CreateObject(SfxObjectCreateMode = SFX_CREATE_MODE_STANDARD) const;

    SfxObjectShellFlags GetFlags() { return nFlags; }
    USHORT          GetOrdinal() const { return nId; }
    const char*     GetShortName() const { return pShortName; }
    void            RegisterMenuBar( const ResId &rId );
    void            RegisterPluginMenuBar( const ResId &rId );
    void            RegisterAccel( const ResId &rId );
    const ResId*    GetMenuBarId() const;
    const ResId*    GetPluginMenuBarId() const;
    const ResId*    GetAccelId() const;
    void            RegisterHelpFile( const String& );
    void            SetExplorerImageId( USHORT nImageId );

    void            RegisterHelpPIFile( const String& );
    SfxFactoryFilterContainer *GetFilterContainer( BOOL bForceLoad = TRUE) const;

    // Views
    void            RegisterViewFactory(SfxViewFactory &rFactory);
    USHORT          GetViewFactoryCount() const;
    SfxViewFactory& GetViewFactory(USHORT i = 0) const;

    // Filter
    void            RegisterInitFactory( SfxVoidFunc pFunc );
    void            DoInitFactory();

    USHORT           GetFilterCount() const;
    const SfxFilter* GetFilter(USHORT i) const;

    static const SfxObjectFactory*  GetFactory( const String& rFactoryURL );

        void                                    SetDocumentServiceName( const ::rtl::OUString& rServiceName );
    const ::rtl::OUString&  GetDocumentServiceName() const;

    DECL_LINK( InitFactoryHdl, void* );

    static const SfxObjectFactory&     GetDefaultFactory();

#if _SOLAR__PRIVATE
    static void                        RemoveAll_Impl();
    static void                        ClearAll_Impl();
    static SfxObjectFactoryArr_Impl&   GetObjFacArray_Impl();
    static void                        RegisterObjectFactory_Impl(SfxObjectFactory &rObjFac);
    static USHORT                      GetObjectFactoryCount_Impl();
    static const SfxObjectFactory&     GetObjectFactory_Impl(USHORT i);
    SfxMenuBarManager*                 GetMenuBar_Impl();
    void                               SetModule_Impl( SfxModule* );
#endif

private:
    // Kopieren verboten
    SfxObjectFactory(const SfxObjectFactory&);
    const SfxObjectFactory& operator=(const SfxObjectFactory &);
};

//=========================================================================

/*  SFX_DECL_OBJECTFACTORY_DLL

    [Description]

    This macro declares an SfxObjectFactory for subclasses of SfxObjectShell
    which are derived too from SfxInPlaceObject and are linked to a DLL that
    is loaded on demand.
    The macro must be used in the public-area of the declaration of the
    SfxObjectShell subclass.
*/

#define SFX_DECL_OBJECTFACTORY_DLL(Class,FactoryPtr)                        \
private:                                                                    \
    static SfxObjectFactory*    pObjectFactory;                             \
protected:                                                                  \
    virtual BOOL                Close();                                    \
public:                                                                     \
                                SO2_DECL_BASIC_CLASS_DLL(Class,FactoryPtr)  \
    virtual BOOL                DoClose();                                  \
    static SfxObjectShell*      CreateObject(SfxObjectCreateMode eMode = SFX_CREATE_MODE_STANDARD); \
    static void                 InitFactory();                              \
    static SfxObjectFactory&    Factory() { return *(SfxObjectFactory*)ClassFactory(); } \
    static void                 RegisterFactory( USHORT nPrio = USHRT_MAX );\
    virtual BOOL                DoInitNew( SvStorage * );                   \
    virtual void                ModifyChanged();                            \
    virtual SfxObjectFactory&   GetFactory() const

//=========================================================================

/*  SFX_IMPL_MODULE_LIB

    [Beschreibung]

    Dieses Makro implementiert exportierte Funktionen als Schnittstelle,
    die zum Laden und Initialisieren sowie zum Freigeben und Deinitialisieren
    einer DLL dienen sowie Funktionsaddressen in dieser DLL ermitteln
    k"onnen.

    Parameter:
        LibName     - "logischer" Name der DLL ( z.B. "SIM" )
        LibString   - Dateiname der DLL

    Damit auch mehrere solcherma\sen definierte Schnittstellen importiert
    werden k"onnen, wird der "LibName" mit in die Namen der definierten
    Funktionen einbezogen.
*/

extern "C"
{
    typedef void  (__LOADONCALLAPI *So2VoidFunc_TYPE) ();
}

#define SFX_IMPL_MODULE_LIB( LibName, LibString )                   \
    void * GetFunc##LibName( const char * pFuncName );              \
    static ::osl::Module* pLibHandle##LibName = NULL;               \
    BOOL LoadLib##LibName()                                 \
    {                                                               \
        if( pLibHandle##LibName == NULL )                           \
        {                                                           \
            pLibHandle##LibName = new ::osl::Module();              \
            So2VoidFunc_TYPE fFunc;                                 \
            if (!pLibHandle##LibName->loadRelative( &thisModule, LibString )) \
                return FALSE;                                       \
            fFunc = (So2VoidFunc_TYPE)GetFunc##LibName( "Init" #LibName "Dll");\
            if( fFunc )                                             \
                (*fFunc)();                                         \
        }                                                           \
        return pLibHandle##LibName->is();                           \
    }                                                               \
    void FreeLib##LibName()                             \
    {                                                               \
        if( pLibHandle##LibName && pLibHandle##LibName->is() )      \
        {                                                           \
            So2VoidFunc_TYPE fFunc;                                 \
            fFunc = (So2VoidFunc_TYPE)GetFunc##LibName("DeInit" #LibName "Dll");\
            if( fFunc )                                             \
                (*fFunc)();                                         \
        }                                                           \
    }                                                               \
    void* GetFunc##LibName( const char * pFuncName )        \
    {                                                               \
        if( LoadLib##LibName() )                                    \
            return pLibHandle##LibName->getSymbol( ::rtl::OUString::createFromAscii(pFuncName) );\
        return NULL;                                                \
    }

//---------------------------------------------------------------------------
/*  SFX_IMPL_MODULE_DLL

    [Beschreibung]

    Dieses Makro implementiert C-Funktionen zum Initialisieren und Deinitialisieren
    einer DLL, die wiederum statische Methoden der DLL-Klasse aufrufen.

    Parameter:
        LibName     - "logischer" Name der DLL ( z.B. "SIM" )
*/

#define SFX_IMPL_MODULE_DLL( LibName )                                      \
                                                                            \
        extern "C" void __LOADONCALLAPI Init##LibName##Dll()                \
        {                                                                   \
            LibName##DLL::Init();                                           \
        }                                                                   \
        extern "C" void __LOADONCALLAPI DeInit##LibName##Dll()              \
        {                                                                   \
            LibName##DLL::Exit();                                           \
        }

//---------------------------------------------------------------------------
 /* SFX_IMPL_OBJECTFACTORY_DLL

    [Beschreibung]

    Die folgenden Makros implementieren eine Factory f"ur SfxObjectShell-Subklassen,
    die auch von SfxInPlaceObject abgeleitet sind und die in einer DLL definiert sind.
    Damit die DLL auch Load-On-Demand geladen werden kann, wird eine einfach
    exportierbare C-Funktion definiert, die die eigentliche Factory-Methode
    aufruft.
    Diese Funktion kann dann in eine andere, exportierte Factory-Methode
    eingebettet werden, so da\s sowohl in der DLL als auch im exportierten Interface
    jeweils eine Factory-Methode definiert sind.
    Erstere erzeugt direkt das Objekt, letztere ruft erstere "uber die
    erw"ahnte exportierte C-Funktion auf. Die Funktionsadresse wird "uber die
    Funktionen des Macros SFX_IMPL_MODULE_LIB besorgt, so da\s jedes Modul, das
    nachladbar sein soll, in seinem Interface dieses Macro einbauem mu\s.
    Dann kann es beliebig viele Interfaces zu SvObjekten mit dem Macro

                SFX_IMPL_OBJECTFACTORY_MOD

    implementieren. Der mit diesem Makro generierte Code wird zum Executable bzw.
    einer per Implib geladenen DLL gelinkt ( daher auch __EXPORT !! ).
    Die Objekte selbst werden mit dem Macro

                SFX_IMPL_OBJECTFACTORY_DLL

    in der nachzuladenden DLL implementiert.
    Das Macro

                SFX_IMPL_OBJECTFACTORY_GEN

    definiert einige Methoden, die in beiden Macros ben"otigt werden.
*/

// Typedefs f"ur die exportierten Funktionen
extern "C"
{
    typedef void* (__LOADONCALLAPI *So2CreateExtern_TYPE) ();
    typedef void* (__LOADONCALLAPI *SfxCreateExtern_TYPE) (int);
}

// Gemeinsame Funktionen in DLL und LIB
#define SFX_IMPL_OBJECTFACTORY_GEN( Class, LibType )                        \
                                                                            \
        SfxObjectShell* Class::CreateObject(SfxObjectCreateMode eMode) \
        {                                                                   \
            SfxObjectShell* pDoc = CreateObj##Class##LibType((int)eMode); \
            return pDoc;                                                    \
        }                                                                   \
        SfxObjectFactory& Class::GetFactory() const             \
        {                                                                   \
            return Factory();                                               \
        }

// Funktionen und Methoden in der LIB
#define SFX_IMPL_OBJECTFACTORY_MOD( ClassName, nFlags, ShortName, aGlobName, LibName, LibString ) \
        SfxObjectFactory* ClassName::pObjectFactory = 0;                        \
        extern "C" ClassName * __LOADONCALLAPI Create##ClassName##Lib()           \
        {                                                                         \
            So2CreateExtern_TYPE fCreate = (So2CreateExtern_TYPE)                 \
                GetFunc##LibName( "Create" #ClassName "Dll");                     \
            if( fCreate )                                                         \
                return (ClassName *)(*fCreate)();                                 \
            return NULL;                                                          \
        }                                                                         \
        SotFactory* ClassName::ClassFactory()                                     \
        {                                                                         \
            if( !pObjectFactory )                                                     \
            {                                                                     \
                pObjectFactory = new SfxObjectFactory( aGlobName,                \
                    String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM(#ClassName) ), ClassName::CreateInstance );  \
                pObjectFactory->PutSuperClass( SfxInPlaceObject::ClassFactory() );  \
            }                                                                     \
            return pObjectFactory;                                                    \
        }                                                                         \
        void* ClassName::CreateInstance( SotObject ** ppObj )            \
        {                                                                         \
            ClassName * p = Create##ClassName##Lib();                             \
            SfxInPlaceObject* pSuper1 = p;                                        \
            SotObject* pBasicObj = pSuper1;                                       \
            if( ppObj )                                                           \
                *ppObj = pBasicObj;                                               \
            return p;                                                             \
        }                                                                         \
        void ClassName::RegisterFactory( USHORT nPrio )         \
        {                                                                   \
            Factory().Construct(                                            \
                nPrio,                                                      \
                &ClassName::CreateObject, nFlags | SFXOBJECTSHELL_HASMENU,  \
                #ShortName );                                               \
            Factory().RegisterInitFactory( &InitFactory );                  \
            Factory().Register();                                           \
        }                                                                   \
        ClassName* CreateObj##ClassName##Lib(int nMode)             \
        {                                                                   \
            SfxCreateExtern_TYPE fCreate = (SfxCreateExtern_TYPE)           \
                GetFunc##LibName( "CreateObj" #ClassName "Dll");            \
            if( fCreate )                                                   \
                return (ClassName*)(*fCreate)(nMode);                       \
            return NULL;                                                    \
        }                                                                   \
                                                                            \
        SFX_IMPL_OBJECTFACTORY_GEN( ClassName, Lib )                        \
                                                                            \
        void ClassName::InitFactory()

// Funktionen und Methoden in der DLL
#define SFX_IMPL_OBJECTFACTORY_LOD( Class, ShortName, aGlobName, LibName ) \
        SO2_IMPL_BASIC_CLASS1_DLL( Class, SfxObjectFactory, SfxInPlaceObject, aGlobName ) \
        extern "C" Class* __LOADONCALLAPI Create##Class##Dll()              \
        {                                                                   \
            return new Class();                                             \
        }                                                                   \
        extern "C" Class* __LOADONCALLAPI CreateObj##Class##Dll(int nMode)  \
        {                                                                   \
            SfxObjectCreateMode eMode = (SfxObjectCreateMode) nMode;        \
            return new Class(eMode);                                        \
        }                                                                   \
                                                                            \
        BOOL Class::DoInitNew( SvStorage *pStor )                           \
        { return SfxObjectShell::DoInitNew(pStor); }                        \
                                                                            \
        BOOL Class::DoClose()                                               \
        { return SfxInPlaceObject::DoClose(); }                             \
                                                                            \
        BOOL Class::Close()                                                 \
        {   SvObjectRef aRef(this);                                         \
            SfxInPlaceObject::Close();                                      \
            return SfxObjectShell::Close(); }                               \
                                                                            \
        void Class::ModifyChanged()                                         \
        { SfxObjectShell::ModifyChanged(); }                                \
                                                                            \
        SFX_IMPL_OBJECTFACTORY_GEN( Class, Dll )

// Wenn nur eine Factory pro Module vorhanden ist
#define SFX_IMPL_OBJECTFACTORY_LIB( Class, nFlags, ShortName, aGlobName, LibName,LibString ) \
        SFX_IMPL_MODULE_LIB( LibName, LibString )                                                                                                   \
        SFX_IMPL_OBJECTFACTORY_MOD( Class, nFlags, ShortName, aGlobName, LibName,LibString )

#define SFX_IMPL_OBJECTFACTORY_DLL( Class, ShortName, aGlobName, LibName ) \
        SFX_IMPL_MODULE_DLL( LibName )                                          \
        SFX_IMPL_OBJECTFACTORY_LOD( Class, ShortName, aGlobName, LibName )

//---------------------------------------------------------------------------

#define SFX_FILTER_REGISTRATION(aName, aWild, ePurpose, lClipboardFormat, \
    pMacType, pOS2Type, nOS2DocIcon, pUserData)

}//end of namespace binfilter
#endif // #ifndef _SFX_OBJFAC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
