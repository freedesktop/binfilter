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

#include <sfxbasemodel.hxx>
#include <bf_sfx2/app.hxx>
#include <com/sun/star/view/XPrintJob.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/frame/IllegalArgumentIOException.hpp>
#include <com/sun/star/view/PaperFormat.hpp>
#include <com/sun/star/view/PaperOrientation.hpp>
#include <cppuhelper/interfacecontainer.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <uno/mapping.hxx>
#include <bf_svtools/itemset.hxx>
#include <bf_svtools/stritem.hxx>
#include <bf_svtools/intitem.hxx>
#include <bf_svtools/eitem.hxx>
#include "bf_basic/sbx.hxx"
#include <osl/file.hxx>
#include <osl/thread.hxx>
#include <tools/urlobj.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/localfilehelper.hxx>
#include <ucbhelper/content.hxx>
#include <osl/mutex.hxx>
#include <vcl/salctype.hxx>

#include "appuno.hxx"
#include <objshimp.hxx>
#include <docfile.hxx>
#include <request.hxx>
#include <objuno.hxx>
#include <printer.hxx>
#include <event.hxx>
#include <eventsupplier.hxx>
#include <evntconf.hxx>
#include <interno.hxx>

#include "sfx.hrc"

#include "openflag.hxx"
#include "appdata.hxx"
#include "docfac.hxx"
#include "fcontnr.hxx"

#include <legacysmgr/legacy_binfilters_smgr.hxx>
#include <cppuhelper/typeprovider.hxx>

#include "bf_basic/basmgr.hxx"

namespace binfilter {

#define    XFRAME                                    ::com::sun::star::frame::XFrame
#define    XINTERFACE                                ::com::sun::star::uno::XInterface
#define    OMULTITYPEINTERFACECONTAINERHELPER        ::cppu::OMultiTypeInterfaceContainerHelper
#define    UNO_QUERY                                ::com::sun::star::uno::UNO_QUERY
#define    DISPOSEDEXCEPTION                        ::com::sun::star::lang::DisposedException
#define    MAPPING                                    ::com::sun::star::uno::Mapping
#define    XSELECTIONSUPPLIER                        ::com::sun::star::view::XSelectionSupplier
#define    ANY                                        ::com::sun::star::uno::Any
#define    ILLEGALARGUMENTEXCEPTION                ::com::sun::star::lang::IllegalArgumentException
#define    ILLEGALARGUMENTIOEXCEPTION                ::com::sun::star::frame::IllegalArgumentIOException
#define    DOUBLEINITIALIZATIONEXCEPTION            ::com::sun::star::frame::DoubleInitializationException
#define    OINTERFACECONTAINERHELPER               ::cppu::OInterfaceContainerHelper
#define    OINTERFACEITERATORHELPER                ::cppu::OInterfaceIteratorHelper
#define    SIZE                                    ::com::sun::star::awt::Size
#define    PAPERFORMAT                                ::com::sun::star::view::PaperFormat
#define    PAPERORIENTATION                        ::com::sun::star::view::PaperOrientation
#define    OTYPECOLLECTION                            ::cppu::OTypeCollection
#define    OIMPLEMENTATIONID                        ::cppu::OImplementationId
#define    MUTEXGUARD                                ::osl::MutexGuard
#define    XINDEXCONTAINER                            ::com::sun::star::container::XIndexContainer
#define    UNSUPPORTEDFLAVOREXCEPTION                ::com::sun::star::datatransfer::UnsupportedFlavorException
#define    XPRINTJOBLISTENER                        ::com::sun::star::view::XPrintJobListener


// Don't use using ... here, because there are at least two classes with the same name in use
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

struct IMPL_SfxBaseModel_DataContainer
{
    SfxObjectShellRef                                m_pObjectShell            ;
    OUSTRING                                        m_sURL                    ;
    sal_uInt16                                        m_nControllerLockCount    ;
    OMULTITYPEINTERFACECONTAINERHELPER                m_aInterfaceContainer    ;
    REFERENCE< XINTERFACE >                            m_xParent                ;
    REFERENCE< XCONTROLLER >                        m_xCurrent                ;
    REFERENCE< XDOCUMENTINFO >                        m_xDocumentInfo            ;
    REFERENCE< XSTARBASICACCESS >                    m_xStarBasicAccess        ;
    REFERENCE< XNAMEREPLACE >                        m_xEvents                ;
    SEQUENCE< PROPERTYVALUE>                        m_seqArguments            ;
    SEQUENCE< REFERENCE< XCONTROLLER > >            m_seqControllers        ;
    REFERENCE< XINDEXACCESS >                        m_contViewData            ;
    sal_Bool                                        m_bLoadDone                ;
    sal_Bool                                        m_bLoadState            ;
    sal_Bool                                        m_bClosed                ;
    sal_Bool                                        m_bClosing                ;
    REFERENCE< ::com::sun::star::view::XPrintJob>     m_xPrintJob                ;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > m_aPrintOptions;

    IMPL_SfxBaseModel_DataContainer( MUTEX& aMutex, SfxObjectShell* pObjectShell)
            :    m_pObjectShell            ( pObjectShell    )
            ,    m_sURL                    ( String()        )
            ,    m_nControllerLockCount    ( 0                )
            ,    m_aInterfaceContainer    ( aMutex        )
            ,    m_bLoadDone                ( sal_False        )
            ,    m_bLoadState            ( sal_False        )
            ,    m_bClosed                ( sal_False        )
            ,    m_bClosing                ( sal_False        )
    {
    }
} ;

extern void* getEnhMetaFileFromGDI_Impl( const GDIMetaFile* pGDIMeta );
extern void* getWinMetaFileFromGDI_Impl( const GDIMetaFile* pGDIMeta, const Size& aMetaSize );
extern SvMemoryStream* getMetaMemStrFromGDI_Impl( const GDIMetaFile* pGDIMeta, sal_uInt32 nFormat );
extern sal_Bool supportsMetaFileHandle_Impl();

SfxBaseModel::SfxBaseModel( SfxObjectShell *pObjectShell )
: IMPL_SfxBaseModel_MutexContainer() , m_pData( new IMPL_SfxBaseModel_DataContainer( m_aMutex, pObjectShell ) )
{
    if ( pObjectShell != NULL )
    {
        StartListening( *pObjectShell ) ;
    }
}

SfxBaseModel::~SfxBaseModel()
{
}

//    XInterface
ANY SAL_CALL SfxBaseModel::queryInterface( const UNOTYPE& rType ) throw( RUNTIMEEXCEPTION )
{
    // Attention:
    //    Don't use mutex or guard in this method!!! Is a method of XInterface.
    // Ask for my own supported interfaces ...
    ANY aReturn( ::cppu::queryInterface( rType,
                                                static_cast< XTYPEPROVIDER*            > ( this )    ,
                                                static_cast< XCHILD*                > ( this )    ,
                                                static_cast< XDOCUMENTINFOSUPPLIER*    > ( this )    ,
                                                static_cast< XEVENTLISTENER*        > ( this )    ,
                                                static_cast< XMODEL*                > ( this )    ,
                                                static_cast< XMODIFIABLE*            > ( this )    ,
                                            static_cast< XCOMPONENT*            > ( this )  ,
                                                static_cast< XPRINTABLE*            > ( this )    ,
                                                static_cast< XSTARBASICACCESS*        > ( this )    ,
                                            static_cast< XSTORABLE*             > ( this )  ,
                                            static_cast< XLOADABLE*             > ( this )  ,
                                            static_cast< XCLOSEABLE*            > ( this )  ) );

    if ( aReturn.hasValue() == sal_False )
    {
        aReturn = ::cppu::queryInterface(    rType                                            ,
                                            static_cast< XMODIFYBROADCASTER*    > ( this )  ,
                                            static_cast< XTRANSFERABLE*    > ( this )  ,
                                                static_cast< XPRINTJOBBROADCASTER*    > ( this )    ,
                                                static_cast< XCLOSEBROADCASTER*        > ( this )    ,
                                            static_cast< XVIEWDATASUPPLIER*     > ( this )  ,
                                                static_cast< XEVENTBROADCASTER*        > ( this )    ,
                                                static_cast< XEVENTSSUPPLIER*        > ( this )    ) ;
    }
    // If searched interface supported by this class ...
    if ( aReturn.hasValue() == sal_True )
    {
        // ... return this information.
        return aReturn ;
    }
    else
    {
        // Else; ... ask baseclass for interfaces!
        return OWeakObject::queryInterface( rType ) ;
    }
}

//    XInterface
void SAL_CALL SfxBaseModel::acquire() throw( )
{
    // Attention:
    //    Don't use mutex or guard in this method!!! Is a method of XInterface.
    // Forward to baseclass
    OWeakObject::acquire() ;
}

//    XInterface
void SAL_CALL SfxBaseModel::release() throw( )
{
    // Attention:
    //    Don't use mutex or guard in this method!!! Is a method of XInterface.
    // Forward to baseclass
    OWeakObject::release() ;
}

//    XTypeProvider
SEQUENCE< UNOTYPE > SAL_CALL SfxBaseModel::getTypes() throw( RUNTIMEEXCEPTION )
{
    // Optimize this method !
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pTypeCollection is NULL - for the second call pTypeCollection is different from NULL!
    static OTYPECOLLECTION* pTypeCollection = NULL ;

    if ( pTypeCollection == NULL )
    {
        // Ready for multithreading; get global mutex for first call of this method only! see before
        MUTEXGUARD aGuard( MUTEX::getGlobalMutex() ) ;

        // Control these pointer again ... it can be, that another instance will be faster then these!
        if ( pTypeCollection == NULL )
        {
            // Create a static typecollection ...
            static OTYPECOLLECTION aTypeCollectionFirst( ::getCppuType(( const REFERENCE< XTYPEPROVIDER          >*)NULL ) ,
                                                        ::getCppuType(( const REFERENCE< XCHILD                 >*)NULL ) ,
                                                        ::getCppuType(( const REFERENCE< XDOCUMENTINFOSUPPLIER  >*)NULL ) ,
                                                        ::getCppuType(( const REFERENCE< XEVENTLISTENER         >*)NULL ) ,
                                                        ::getCppuType(( const REFERENCE< XMODEL                 >*)NULL ) ,
                                                        ::getCppuType(( const REFERENCE< XMODIFIABLE            >*)NULL ) ,
                                                        ::getCppuType(( const REFERENCE< XPRINTABLE             >*)NULL ) ,
                                                        ::getCppuType(( const REFERENCE< XSTORABLE              >*)NULL ) ,
                                                        ::getCppuType(( const REFERENCE< XLOADABLE              >*)NULL ) ,
                                                        ::getCppuType(( const REFERENCE< XCLOSEABLE             >*)NULL ) ,
                                                        ::getCppuType(( const REFERENCE< XSTARBASICACCESS       >*)NULL ) ,
                                                        ::getCppuType(( const REFERENCE< XEVENTBROADCASTER      >*)NULL ) );

            static OTYPECOLLECTION aTypeCollection     ( ::getCppuType(( const REFERENCE< XVIEWDATASUPPLIER      >*)NULL ) ,
                                                        ::getCppuType(( const REFERENCE< XTRANSFERABLE          >*)NULL ) ,
                                                        ::getCppuType(( const REFERENCE< XPRINTJOBBROADCASTER   >*)NULL ) ,
                                                        ::getCppuType(( const REFERENCE< XEVENTSSUPPLIER        >*)NULL ) ,
                                                        ::getCppuType(( const REFERENCE< XCLOSEBROADCASTER      >*)NULL ) ,
                                                        aTypeCollectionFirst.getTypes()                                   );

            // ... and set his address to static pointer!
            pTypeCollection = &aTypeCollection ;
        }
    }

    return pTypeCollection->getTypes() ;
}

//    XTypeProvider
SEQUENCE< sal_Int8 > SAL_CALL SfxBaseModel::getImplementationId() throw( RUNTIMEEXCEPTION )
{
    // Create one Id for all instances of this class.
    // Use ethernet address to do this! (sal_True)
    // Optimize this method
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pID is NULL - for the second call pID is different from NULL!
    static OIMPLEMENTATIONID* pID = NULL ;

    if ( pID == NULL )
    {
        // Ready for multithreading; get global mutex for first call of this method only! see before
        MUTEXGUARD aGuard( MUTEX::getGlobalMutex() ) ;

        // Control these pointer again ... it can be, that another instance will be faster then these!
        if ( pID == NULL )
        {
            // Create a new static ID ...
            static OIMPLEMENTATIONID aID( sal_False ) ;
            // ... and set his address to static pointer!
            pID = &aID ;
        }
    }

    return pID->getImplementationId() ;
}

//    XStarBasicAccess
REFERENCE< XSTARBASICACCESS > implGetStarBasicAccess( SfxObjectShell* pObjectShell )
{
    REFERENCE< XSTARBASICACCESS > xRet;
    if( pObjectShell )
    {
        BasicManager* pMgr = pObjectShell->GetBasicManager();
        xRet = getStarBasicAccess( pMgr );
    }
    return xRet;
}

REFERENCE< XNAMECONTAINER > SAL_CALL SfxBaseModel::getLibraryContainer() throw( RUNTIMEEXCEPTION )
{
    SolarMutexGuard aGuard;
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    REFERENCE< XSTARBASICACCESS >& rxAccess = m_pData->m_xStarBasicAccess;
    if( !rxAccess.is() )
        rxAccess = implGetStarBasicAccess( m_pData->m_pObjectShell );

    REFERENCE< XNAMECONTAINER > xRet;
    if( rxAccess.is() )
        xRet = rxAccess->getLibraryContainer();
    return xRet;
}

/**___________________________________________________________________________________________________
    @seealso    XStarBasicAccess
*/
void SAL_CALL SfxBaseModel::createLibrary( const OUSTRING& LibName, const OUSTRING& Password,
    const OUSTRING& ExternalSourceURL, const OUSTRING& LinkTargetURL )
        throw(ELEMENTEXISTEXCEPTION, RUNTIMEEXCEPTION)
{
    SolarMutexGuard aGuard;
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    REFERENCE< XSTARBASICACCESS >& rxAccess = m_pData->m_xStarBasicAccess;
    if( !rxAccess.is() )
        rxAccess = implGetStarBasicAccess( m_pData->m_pObjectShell );

    if( rxAccess.is() )
        rxAccess->createLibrary( LibName, Password, ExternalSourceURL, LinkTargetURL );
}

/**___________________________________________________________________________________________________
    @seealso    XStarBasicAccess
*/
void SAL_CALL SfxBaseModel::addModule( const OUSTRING& LibraryName, const OUSTRING& ModuleName,
    const OUSTRING& Language, const OUSTRING& Source )
        throw( NOSUCHELEMENTEXCEPTION, RUNTIMEEXCEPTION)
{
    SolarMutexGuard aGuard;
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    REFERENCE< XSTARBASICACCESS >& rxAccess = m_pData->m_xStarBasicAccess;
    if( !rxAccess.is() )
        rxAccess = implGetStarBasicAccess( m_pData->m_pObjectShell );

    if( rxAccess.is() )
        rxAccess->addModule( LibraryName, ModuleName, Language, Source );
}

/**___________________________________________________________________________________________________
    @seealso    XStarBasicAccess
*/
void SAL_CALL SfxBaseModel::addDialog( const OUSTRING& LibraryName, const OUSTRING& DialogName,
    const ::com::sun::star::uno::Sequence< sal_Int8 >& Data )
        throw(NOSUCHELEMENTEXCEPTION, RUNTIMEEXCEPTION)
{
    SolarMutexGuard aGuard;
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    REFERENCE< XSTARBASICACCESS >& rxAccess = m_pData->m_xStarBasicAccess;
    if( !rxAccess.is() )
        rxAccess = implGetStarBasicAccess( m_pData->m_pObjectShell );

    if( rxAccess.is() )
        rxAccess->addDialog( LibraryName, DialogName, Data );
}

//    XChild
REFERENCE< XINTERFACE > SAL_CALL SfxBaseModel::getParent() throw( RUNTIMEEXCEPTION )
{
    SolarMutexGuard aGuard;
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    return m_pData->m_xParent;
}

//    XChild
void SAL_CALL SfxBaseModel::setParent(const REFERENCE< XINTERFACE >& Parent) throw(NOSUPPORTEXCEPTION, RUNTIMEEXCEPTION)
{
    SolarMutexGuard aGuard;
    if ( Parent.is() && getParent().is() )
        // only set parent when no parent is available
        throw NOSUPPORTEXCEPTION();

    m_pData->m_xParent = Parent;
}

//    XChild
long SfxObjectShellClose_Impl( void* pObj, void* pArg );

void SAL_CALL SfxBaseModel::dispose() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    // object already disposed?
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    if  ( !m_pData->m_bClosed )
    {
        // gracefully accept wrong dispose calls instead of close call
        // and try to make it work (may be really disposed later!)
        try
        {
            close( sal_True );
        }
        catch ( ::com::sun::star::util::CloseVetoException& )
        {
        }

        return;
    }

    EVENTOBJECT aEvent( (XMODEL *)this );
    m_pData->m_aInterfaceContainer.disposeAndClear( aEvent );

    // is an object shell assigned?
    if ( m_pData->m_pObjectShell.Is() )
    {
        // Rekursion vermeiden
        SfxObjectShellRef pShell;
        //SfxObjectShellLock pShellLock;
        {
            // am I "ThisComponent" in AppBasic?
            StarBASIC* pBas = SFX_APP()->GetBasic_Impl();
            if ( pBas && SFX_APP()->Get_Impl()->pThisDocument == m_pData->m_pObjectShell )
            {
                OSL_FAIL( "SfxBaseModel::dispose: dead code!" );
            }

            pShell = m_pData->m_pObjectShell;
            EndListening( *pShell );
            m_pData->m_pObjectShell = SfxObjectShellRef();
        }
        // Bei dispose keine Speichern-R"uckfrage
        pShell->Get_Impl()->bDisposing = TRUE;
        SfxObjectShellClose_Impl( 0, (void*) pShell );
    }

    m_pData->m_xCurrent = REFERENCE< XCONTROLLER > ();
    m_pData->m_seqControllers = SEQUENCE< REFERENCE< XCONTROLLER > > () ;
    DELETEZ(m_pData);
}

//    XChild
void SAL_CALL SfxBaseModel::addEventListener( const REFERENCE< XEVENTLISTENER >& aListener )
    throw(::com::sun::star::uno::RuntimeException)
{
    // object already disposed?
    SolarMutexGuard aGuard;
    if ( impl_isDisposed() )
        return;

    m_pData->m_aInterfaceContainer.addInterface( ::getCppuType((const REFERENCE< XEVENTLISTENER >*)0), aListener );
}

//    XChild
void SAL_CALL SfxBaseModel::removeEventListener( const REFERENCE< XEVENTLISTENER >& aListener )
    throw(::com::sun::star::uno::RuntimeException)
{
    // object already disposed?
    SolarMutexGuard aGuard;
    if ( impl_isDisposed() )
        return;

    m_pData->m_aInterfaceContainer.removeInterface( ::getCppuType((const REFERENCE< XEVENTLISTENER >*)0), aListener );
}

//    XDOCUMENTINFOSupplier
REFERENCE< XDOCUMENTINFO > SAL_CALL SfxBaseModel::getDocumentInfo() throw(::com::sun::star::uno::RuntimeException)
{
    // object already disposed?
    SolarMutexGuard aGuard;
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    if ( !m_pData->m_xDocumentInfo.is() && m_pData->m_pObjectShell.Is() )
        ((SfxBaseModel*)this)->m_pData->m_xDocumentInfo = new SfxDocumentInfoObject( m_pData->m_pObjectShell ) ;

    return m_pData->m_xDocumentInfo;
}

//    XEVENTLISTENER
void SAL_CALL SfxBaseModel::disposing( const EVENTOBJECT& aObject )
      throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if ( impl_isDisposed() )
        return;

    REFERENCE< XMODIFYLISTENER >  xMod( aObject.Source, UNO_QUERY );
    REFERENCE< XEVENTLISTENER >  xListener( aObject.Source, UNO_QUERY );
    REFERENCE< XDOCEVENTLISTENER >  xDocListener( aObject.Source, UNO_QUERY );

    if ( xMod.is() )
        m_pData->m_aInterfaceContainer.removeInterface( ::getCppuType((const REFERENCE< XMODIFYLISTENER >*)0), xMod );
    else if ( xListener.is() )
        m_pData->m_aInterfaceContainer.removeInterface( ::getCppuType((const REFERENCE< XEVENTLISTENER >*)0), xListener );
    else if ( xDocListener.is() )
        m_pData->m_aInterfaceContainer.removeInterface( ::getCppuType((const REFERENCE< XDOCEVENTLISTENER >*)0), xListener );
}

//    XMODEL
sal_Bool SAL_CALL SfxBaseModel::attachResource(    const    OUSTRING&                    rURL    ,
                                                const    SEQUENCE< PROPERTYVALUE >&    rArgs    )
    throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    if ( rURL.getLength() == 0 && rArgs.getLength() == 1 && rArgs[0].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "SetEmbedded" ) ) )
    {
        // allows to set a windowless document to EMBEDDED state
        // but _only_ before load() or initNew() methods
        if ( m_pData->m_pObjectShell.Is() && !m_pData->m_pObjectShell->GetMedium() )
        {
            sal_Bool bEmb;
            rArgs[0].Value >>= bEmb;
        }

        return sal_True;
    }

    if ( m_pData->m_pObjectShell.Is() )
    {
        m_pData->m_sURL = rURL;
        m_pData->m_seqArguments = rArgs;

        for ( sal_Int32 nInd = 0; nInd < rArgs.getLength(); nInd++ )
            if ( rArgs[nInd].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "WinExtent" ) ) )
            {
                // the document should be resized
                SfxInPlaceObject* pInPlaceObj = m_pData->m_pObjectShell->GetInPlaceObject();
                if ( pInPlaceObj )
                {
                    Sequence< sal_Int32 > aSize;
                    if ( ( rArgs[nInd].Value >>= aSize ) && aSize.getLength() == 4 )
                    {
                        Rectangle aTmpRect( aSize[0], aSize[1], aSize[2], aSize[3] );
                        aTmpRect = OutputDevice::LogicToLogic( aTmpRect, MAP_100TH_MM, pInPlaceObj->GetMapUnit() );

                        pInPlaceObj->SetVisArea( aTmpRect );
                    }
                }
            }

        if( m_pData->m_pObjectShell->GetMedium() )
        {
            SfxAllItemSet aSet( m_pData->m_pObjectShell->GetPool() );
            TransformParameters( SID_OPENDOC, rArgs, aSet );
            m_pData->m_pObjectShell->GetMedium()->GetItemSet()->Put( aSet );
            SFX_ITEMSET_ARG( &aSet, pItem, SfxStringItem, SID_FILTER_NAME, sal_False );
            if ( pItem )
                m_pData->m_pObjectShell->GetMedium()->SetFilter(
                    m_pData->m_pObjectShell->GetFactory().GetFilterContainer()->GetFilter( pItem->GetValue() ) );
        }
    }

    return sal_True ;
}

//    XMODEL
OUSTRING SAL_CALL SfxBaseModel::getURL() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    return m_pData->m_sURL ;
}

//    XMODEL
SEQUENCE< PROPERTYVALUE > SAL_CALL SfxBaseModel::getArgs() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    if ( m_pData->m_pObjectShell.Is() )
    {
        SEQUENCE< PROPERTYVALUE > seqArgsNew;
        SEQUENCE< PROPERTYVALUE > seqArgsOld;
        SfxAllItemSet aSet( m_pData->m_pObjectShell->GetPool() );

        // we need to know which properties are supported by the transformer
        // hopefully it is a temporary solution, I guess nonconvertable properties
        // should not be supported so then there will be only ItemSet from medium

        TransformItems( SID_OPENDOC, *(m_pData->m_pObjectShell->GetMedium()->GetItemSet()), seqArgsNew );
        TransformParameters( SID_OPENDOC, m_pData->m_seqArguments, aSet );
        TransformItems( SID_OPENDOC, aSet, seqArgsOld );

        sal_Int32 nOrgLength = m_pData->m_seqArguments.getLength();
        sal_Int32 nOldLength = seqArgsOld.getLength();
        sal_Int32 nNewLength = seqArgsNew.getLength();

        // "WinExtent" property should be updated always.
        // We can store it now to overwrite an old value
        // since it is not from ItemSet
        SfxInPlaceObject* pInPlaceObj = m_pData->m_pObjectShell->GetInPlaceObject();
        if ( pInPlaceObj )
        {
            Rectangle aTmpRect = pInPlaceObj->GetVisArea( ASPECT_CONTENT );
            aTmpRect = OutputDevice::LogicToLogic( aTmpRect, pInPlaceObj->GetMapUnit(), MAP_100TH_MM );

            Sequence< sal_Int32 > aSize(4);
            aSize[0] = aTmpRect.Left();
            aSize[1] = aTmpRect.Top();
            aSize[2] = aTmpRect.Right();
            aSize[3] = aTmpRect.Bottom();
            seqArgsNew.realloc( ++nNewLength );
            seqArgsNew[ nNewLength - 1 ].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "WinExtent" ));
            seqArgsNew[ nNewLength - 1 ].Value <<= aSize;
        }

        for ( sal_Int32 nOrg = 0; nOrg < nOrgLength; nOrg++ )
        {
            sal_Int32 nOldInd = 0;
            while ( nOldInd < nOldLength )
            {
                if ( m_pData->m_seqArguments[nOrg].Name.equals( seqArgsOld[nOldInd].Name ) )
                    break;
                nOldInd++;
            }

            if ( nOldInd == nOldLength )
            {
                // the entity with this name should be new for seqArgsNew
                // since it is not supported by transformer
                seqArgsNew.realloc( ++nNewLength );
                seqArgsNew[ nNewLength - 1 ].Name = m_pData->m_seqArguments[nOrg].Name;
                seqArgsNew[ nNewLength - 1 ].Value = m_pData->m_seqArguments[nOrg].Value;
            }

        }
        m_pData->m_seqArguments = seqArgsNew;
    }

    return m_pData->m_seqArguments ;
}

//    XMODEL
void SAL_CALL SfxBaseModel::connectController( const REFERENCE< XCONTROLLER >& xController )
    throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    sal_uInt32 nOldCount = m_pData->m_seqControllers.getLength();
    SEQUENCE< REFERENCE< XCONTROLLER > > aNewSeq( nOldCount + 1 );
    for ( sal_uInt32 n = 0; n < nOldCount; n++ )
        aNewSeq.getArray()[n] = m_pData->m_seqControllers.getConstArray()[n];
    aNewSeq.getArray()[nOldCount] = xController;
    m_pData->m_seqControllers = aNewSeq;
}

//    XMODEL
void SAL_CALL SfxBaseModel::disconnectController( const REFERENCE< XCONTROLLER >& xController ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    sal_uInt32 nOldCount = m_pData->m_seqControllers.getLength();
    if ( !nOldCount )
        return;

    SEQUENCE< REFERENCE< XCONTROLLER > > aNewSeq( nOldCount - 1 );
    for ( sal_uInt32 nOld = 0, nNew = 0; nOld < nOldCount; ++nOld )
    {
        if ( xController != m_pData->m_seqControllers.getConstArray()[nOld] )
        {
            aNewSeq.getArray()[nNew] = m_pData->m_seqControllers.getConstArray()[nOld];
            ++nNew;
        }
    }

    m_pData->m_seqControllers = aNewSeq;

    if ( xController == m_pData->m_xCurrent )
        m_pData->m_xCurrent = REFERENCE< XCONTROLLER > ();
}

//    XMODEL
void SAL_CALL SfxBaseModel::lockControllers() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();
    ++m_pData->m_nControllerLockCount ;
}

//    XMODEL
void SAL_CALL SfxBaseModel::unlockControllers() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();
    --m_pData->m_nControllerLockCount ;
}

//    XMODEL
sal_Bool SAL_CALL SfxBaseModel::hasControllersLocked() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();
    return ( m_pData->m_nControllerLockCount != 0 ) ;
}

//    XMODEL
REFERENCE< XCONTROLLER > SAL_CALL SfxBaseModel::getCurrentController() throw(::com::sun::star::uno::RuntimeException)
{
    // object already disposed?
    SolarMutexGuard aGuard;
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    // get the last active controller of this model
    if ( m_pData->m_xCurrent.is() )
        return m_pData->m_xCurrent;

    // get the first controller of this model
    return m_pData->m_seqControllers.getLength() ? m_pData->m_seqControllers.getConstArray()[0] : m_pData->m_xCurrent;
}

//    XMODEL
void SAL_CALL SfxBaseModel::setCurrentController( const REFERENCE< XCONTROLLER >& xCurrentController )
        throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException)
{
    // object already disposed?
    SolarMutexGuard aGuard;
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    m_pData->m_xCurrent = xCurrentController;
}

//    XMODEL
REFERENCE< XINTERFACE > SAL_CALL SfxBaseModel::getCurrentSelection() throw(::com::sun::star::uno::RuntimeException)
{
    // object already disposed?
    SolarMutexGuard aGuard;
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    REFERENCE< XINTERFACE >     xReturn;
    REFERENCE< XCONTROLLER >    xController    =    getCurrentController()        ;

    if ( xController.is() )
    {
        REFERENCE< XSELECTIONSUPPLIER >  xDocView( xController, UNO_QUERY );
        if ( xDocView.is() )
        {
            ANY xSel = xDocView->getSelection();
            xSel >>= xReturn ;
        }
    }

    return xReturn ;
}

//    XModifiable
sal_Bool SAL_CALL SfxBaseModel::isModified() throw(::com::sun::star::uno::RuntimeException)
{
    // object already disposed?
    SolarMutexGuard aGuard;
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    return m_pData->m_pObjectShell.Is() ? m_pData->m_pObjectShell->IsModified() : sal_False;
}

//    XModifiable
void SAL_CALL SfxBaseModel::setModified( sal_Bool bModified )
        throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException)
{
    // object already disposed?
    SolarMutexGuard aGuard;
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    if ( m_pData->m_pObjectShell.Is() )
        m_pData->m_pObjectShell->SetModified(bModified);
}

//    XModifiable
void SAL_CALL SfxBaseModel::addModifyListener(const REFERENCE< XMODIFYLISTENER >& xListener) throw( RUNTIMEEXCEPTION )
{
    // object already disposed?
    SolarMutexGuard aGuard;
    if ( impl_isDisposed() )
        return;

    m_pData->m_aInterfaceContainer.addInterface( ::getCppuType((const REFERENCE< XMODIFYLISTENER >*)0),xListener );
}

//    XModifiable
void SAL_CALL SfxBaseModel::removeModifyListener(const REFERENCE< XMODIFYLISTENER >& xListener) throw( RUNTIMEEXCEPTION )
{
    // object already disposed?
    SolarMutexGuard aGuard;
    if ( impl_isDisposed() )
        return;

    m_pData->m_aInterfaceContainer.removeInterface( ::getCppuType((const REFERENCE< XMODIFYLISTENER >*)0), xListener );
}

//  XCloseable
void SAL_CALL SfxBaseModel::close( sal_Bool bDeliverOwnership ) throw (CLOSEVETOEXCEPTION, RUNTIMEEXCEPTION)
{
    SolarMutexGuard aGuard;
    if ( !m_pData || m_pData->m_bClosed || m_pData->m_bClosing )
        return;

    uno::Reference< uno::XInterface > xSelfHold( static_cast< ::cppu::OWeakObject* >(this) );
    lang::EventObject             aSource    (static_cast< ::cppu::OWeakObject*>(this));
    ::cppu::OInterfaceContainerHelper* pContainer = m_pData->m_aInterfaceContainer.getContainer( ::getCppuType( ( const uno::Reference< util::XCloseListener >*) NULL ) );
    if (pContainer!=NULL)
    {
        ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
        while (pIterator.hasMoreElements())
        {
            try
            {
                ((util::XCloseListener*)pIterator.next())->queryClosing( aSource, bDeliverOwnership );
            }
            catch( uno::RuntimeException& )
            {
                pIterator.remove();
            }
        }
    }

    // no own objections against closing!
    m_pData->m_bClosing = sal_True;
    m_pData->m_pObjectShell->Broadcast( SfxSimpleHint(SFX_HINT_DEINITIALIZING) );
    pContainer = m_pData->m_aInterfaceContainer.getContainer( ::getCppuType( ( const uno::Reference< util::XCloseListener >*) NULL ) );
    if (pContainer!=NULL)
    {
        ::cppu::OInterfaceIteratorHelper pCloseIterator(*pContainer);
    }

    m_pData->m_bClosed = sal_True;
    m_pData->m_bClosing = sal_False;

    dispose();
}

//  XCloseBroadcaster
void SAL_CALL SfxBaseModel::addCloseListener( const REFERENCE< XCLOSELISTENER >& xListener ) throw (RUNTIMEEXCEPTION)
{
    // object already disposed?
    SolarMutexGuard aGuard;
    if ( impl_isDisposed() )
        return;

    m_pData->m_aInterfaceContainer.addInterface( ::getCppuType((const REFERENCE< XCLOSELISTENER >*)0), xListener );
}

//  XCloseBroadcaster
void SAL_CALL SfxBaseModel::removeCloseListener( const REFERENCE< XCLOSELISTENER >& xListener ) throw (RUNTIMEEXCEPTION)
{
    // object already disposed?
    SolarMutexGuard aGuard;
    if ( impl_isDisposed() )
        return;

    m_pData->m_aInterfaceContainer.removeInterface( ::getCppuType((const REFERENCE< XCLOSELISTENER >*)0), xListener );
}

//  XPrintable
//________________________________________________________________________________________________________
void SAL_CALL SfxBaseModel::print(const SEQUENCE< PROPERTYVALUE >& /*rOptions*/)
    throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    DBG_BF_ASSERT(0, "STRIP");  // VIRTUAL
}

//________________________________________________________________________________________________________
//  XStorable
//________________________________________________________________________________________________________

/*N*/ sal_Bool SAL_CALL SfxBaseModel::hasLocation() throw(::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/   // object already disposed?
/*N*/     SolarMutexGuard aGuard;
/*N*/   if ( impl_isDisposed() )
/*N*/       throw DISPOSEDEXCEPTION();
/*N*/
/*N*/   return m_pData->m_pObjectShell.Is() ? m_pData->m_pObjectShell->HasName() : sal_False;
/*N*/ }

//________________________________________________________________________________________________________
//  XStorable
//________________________________________________________________________________________________________

/*N*/ OUSTRING SAL_CALL SfxBaseModel::getLocation() throw(::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/   // object already disposed?
/*N*/     SolarMutexGuard aGuard;
/*N*/   if ( impl_isDisposed() )
/*N*/       throw DISPOSEDEXCEPTION();
/*N*/
/*N*/   return m_pData->m_pObjectShell.Is() ? OUSTRING(m_pData->m_pObjectShell->GetMedium()->GetName()) : m_pData->m_sURL;
/*N*/ }

//________________________________________________________________________________________________________
//  XStorable
//________________________________________________________________________________________________________

/*N*/ sal_Bool SAL_CALL SfxBaseModel::isReadonly() throw(::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/   // object already disposed?
/*N*/     SolarMutexGuard aGuard;
/*N*/   if ( impl_isDisposed() )
/*N*/       throw DISPOSEDEXCEPTION();
/*N*/
/*N*/   return m_pData->m_pObjectShell.Is() ? m_pData->m_pObjectShell->IsReadOnly() : sal_True;
/*N*/ }

//________________________________________________________________________________________________________
//  XStorable
//________________________________________________________________________________________________________

/*N*/ void SAL_CALL SfxBaseModel::store() throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/   // object already disposed?
/*N*/     SolarMutexGuard aGuard;
/*N*/   if ( impl_isDisposed() )
/*N*/       throw DISPOSEDEXCEPTION();
/*N*/
/*N*/   if ( m_pData->m_pObjectShell.Is() )
/*N*/     {
/*N*/       if ( ! ( m_pData->m_pObjectShell->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED ) )
/*N*/       {}
/*N*/     }
/*N*/ }

//________________________________________________________________________________________________________
//  XStorable
//________________________________________________________________________________________________________

/*N*/ void SAL_CALL SfxBaseModel::storeAsURL(   const   OUSTRING&                   rURL    ,
/*N*/                                       const   SEQUENCE< PROPERTYVALUE >&  rArgs   )
/*N*/         throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/   // object already disposed?
/*N*/     SolarMutexGuard aGuard;
/*N*/   if ( impl_isDisposed() )
/*N*/       throw DISPOSEDEXCEPTION();
/*N*/
/*N*/    if ( m_pData->m_pObjectShell.Is() )
/*N*/  {
/*N*/      impl_store( m_pData->m_pObjectShell, rURL, rArgs, sal_False );
/*N*/
/*N*/      SEQUENCE< PROPERTYVALUE > aSequence ;
/*N*/      TransformItems( SID_OPENDOC, *m_pData->m_pObjectShell->GetMedium()->GetItemSet(), aSequence );
/*N*/      attachResource( rURL, aSequence );
/*N*/  }
/*N*/ }

//________________________________________________________________________________________________________
//  XStorable
//________________________________________________________________________________________________________

/*N*/ void SAL_CALL SfxBaseModel::storeToURL(   const   OUSTRING&                   rURL    ,
/*N*/                                       const   SEQUENCE< PROPERTYVALUE >&  rArgs   )
/*N*/         throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/   // object already disposed?
/*N*/     SolarMutexGuard aGuard;
/*N*/   if ( impl_isDisposed() )
/*N*/       throw DISPOSEDEXCEPTION();
/*N*/
/*N*/   if ( m_pData->m_pObjectShell.Is() )
/*N*/     {
/*N*/       impl_store( m_pData->m_pObjectShell, rURL, rArgs, sal_True );
/*N*/     }
/*N*/ }

//________________________________________________________________________________________________________
// XLoadable
void SAL_CALL SfxBaseModel::initNew()
        throw (::com::sun::star::frame::DoubleInitializationException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::uno::RuntimeException,
                ::com::sun::star::uno::Exception)
{
    // object already disposed?
    SolarMutexGuard aGuard;
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    // the object shell should exist always
    DBG_ASSERT( m_pData->m_pObjectShell.Is(), "Model is useless without an ObjectShell" );
    if ( m_pData->m_pObjectShell.Is() )
    {
        if( m_pData->m_pObjectShell->GetMedium() )
            throw DOUBLEINITIALIZATIONEXCEPTION();

        sal_Bool bRes = m_pData->m_pObjectShell->DoInitNew( NULL );
        m_pData->m_pObjectShell->ResetError();

        if ( !bRes )
        {
            throw ::com::sun::star::io::IOException();
        }
    }
}

// XLoadable
void SAL_CALL SfxBaseModel::load(    const SEQUENCE< PROPERTYVALUE >& seqArguments )
          throw (::com::sun::star::frame::DoubleInitializationException,
                 ::com::sun::star::io::IOException,
                 ::com::sun::star::uno::RuntimeException,
                 ::com::sun::star::uno::Exception)
{
      // object already disposed?
      SolarMutexGuard aGuard;
      if ( impl_isDisposed() )
          throw DISPOSEDEXCEPTION();

      // the object shell should exist always
      DBG_ASSERT( m_pData->m_pObjectShell.Is(), "Model is useless without an ObjectShell" );
    if ( m_pData->m_pObjectShell.Is() )
     {
         if( m_pData->m_pObjectShell->GetMedium() )
             throw DOUBLEINITIALIZATIONEXCEPTION();

         SfxAllItemSet *pParams = new SfxAllItemSet( SFX_APP()->GetPool() );
         TransformParameters( SID_OPENDOC, seqArguments, *pParams );

         ::rtl::OUString aFilterName;
         SFX_ITEMSET_ARG( pParams, pFilterNameItem, SfxStringItem, SID_FILTER_NAME, sal_False );
         if( pFilterNameItem )
             aFilterName = pFilterNameItem->GetValue();

         if( !aFilterName.getLength() )
             throw ILLEGALARGUMENTIOEXCEPTION();

        const SfxFilter* pFilter = SFX_APP()->GetFilterMatcher().GetFilter4FilterName( aFilterName );
        BOOL bReadOnly = FALSE;
        SFX_ITEMSET_ARG( pParams, pReadOnlyItem, SfxBoolItem, SID_DOC_READONLY, FALSE );
        if ( pReadOnlyItem && pReadOnlyItem->GetValue() )
            bReadOnly = TRUE;
        SFX_ITEMSET_ARG( pParams, pFileNameItem, SfxStringItem, SID_FILE_NAME, FALSE );
        SfxMedium* pMedium = new SfxMedium( pFileNameItem->GetValue(), bReadOnly ? SFX_STREAM_READONLY : SFX_STREAM_READWRITE, FALSE, pFilter, pParams );

        // allow to use an interactionhandler (if there is one)
        pMedium->UseInteractionHandler( TRUE );

        // load document
        sal_uInt32 nError = ERRCODE_NONE;
        BOOL bOK = m_pData->m_pObjectShell->DoLoad(pMedium);
        m_pData->m_pObjectShell->ResetError();
        nError = pMedium->GetError();
        if ( !nError && !bOK )
            nError = ERRCODE_IO_GENERAL;

        if ( nError )
        {
            if ( m_pData->m_pObjectShell->GetMedium() != pMedium )
            {
                // for whatever reason document now has another medium
                OSL_FAIL("Document has rejected the medium?!");
                delete pMedium;
            }

            throw ::com::sun::star::io::IOException();
        }
    }
}

// XTransferable
ANY SAL_CALL SfxBaseModel::getTransferData( const DATAFLAVOR& /*aFlavor*/ )
        throw (::com::sun::star::datatransfer::UnsupportedFlavorException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::uno::RuntimeException)
{
    DBG_BF_ASSERT(0, "STRIP");  // VIRTUAL
    ANY aAny;
    return aAny;
}

// XTransferable
SEQUENCE< DATAFLAVOR > SAL_CALL SfxBaseModel::getTransferDataFlavors()
        throw (::com::sun::star::uno::RuntimeException)
{
    DBG_BF_ASSERT(0, "STRIP");  // VIRTUAL
    SEQUENCE< DATAFLAVOR > aDATAFLAVOR(0);
    return aDATAFLAVOR;
}

//    XEventsSupplier
REFERENCE< XNAMEREPLACE > SAL_CALL SfxBaseModel::getEvents() throw( RUNTIMEEXCEPTION )
{
    // object already disposed?
    SolarMutexGuard aGuard;
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    if ( ! m_pData->m_xEvents.is() )
    {
        m_pData->m_xEvents = new SfxEvents_Impl( m_pData->m_pObjectShell, this );
    }

    return m_pData->m_xEvents;
}

//    XEventBroadcaster
void SAL_CALL SfxBaseModel::addEventListener( const REFERENCE< XDOCEVENTLISTENER >& aListener ) throw( RUNTIMEEXCEPTION )
{
    // object already disposed?
    SolarMutexGuard aGuard;
    if ( impl_isDisposed() )
        return;

    m_pData->m_aInterfaceContainer.addInterface( ::getCppuType((const REFERENCE< XDOCEVENTLISTENER >*)0), aListener );
}

//    XEventBroadcaster
void SAL_CALL SfxBaseModel::removeEventListener( const REFERENCE< XDOCEVENTLISTENER >& aListener ) throw( RUNTIMEEXCEPTION )
{
    // object already disposed?
    SolarMutexGuard aGuard;
    if ( impl_isDisposed() )
        return;

    m_pData->m_aInterfaceContainer.removeInterface( ::getCppuType((const REFERENCE< XDOCEVENTLISTENER >*)0), aListener );
}

//    SfxListener
void addTitle_Impl( Sequence < ::com::sun::star::beans::PropertyValue >& rSeq, const ::rtl::OUString& rTitle )
{
    sal_Int32 nCount = rSeq.getLength();
    sal_Int32 nArg; for ( nArg=0; nArg<nCount; nArg++ )
    {
        ::com::sun::star::beans::PropertyValue& rProp = rSeq[nArg];
        if ( rProp.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Title")) )
        {
            rProp.Value <<= rTitle;
            break;
        }
    }

    if ( nArg == nCount )
    {
        rSeq.realloc( nCount+1 );
        rSeq[nCount].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Title") );
        rSeq[nCount].Value <<= rTitle;
    }
}

void SfxBaseModel::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    if ( !m_pData )
        return;

    if ( &rBC == m_pData->m_pObjectShell )
    {
        SfxSimpleHint* pSimpleHint = PTR_CAST( SfxSimpleHint, &rHint );
        if ( pSimpleHint && pSimpleHint->GetId() == SFX_HINT_DOCCHANGED )
            changing();

        SfxEventHint* pNamedHint = PTR_CAST( SfxEventHint, &rHint );
        if ( pNamedHint )
        {
            if ( SFX_EVENT_SAVEASDOCDONE == pNamedHint->GetEventId() )
            {
                m_pData->m_sURL = m_pData->m_pObjectShell->GetMedium()->GetName();
                SfxItemSet *pSet = m_pData->m_pObjectShell->GetMedium()->GetItemSet();
                ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aArgs;
                ::rtl::OUString aTitle = m_pData->m_pObjectShell->GetTitle();
                TransformItems( SID_SAVEASDOC, *pSet, aArgs );
                addTitle_Impl( aArgs, aTitle );
                attachResource( m_pData->m_pObjectShell->GetMedium()->GetName(), aArgs );
            }

            postEvent_Impl( *pNamedHint );
        }

        if ( pSimpleHint )
        {
            if ( pSimpleHint->GetId() == SFX_HINT_TITLECHANGED )
            {
                ::rtl::OUString aTitle = m_pData->m_pObjectShell->GetTitle();
                addTitle_Impl( m_pData->m_seqArguments, aTitle );
            }
        }

    }
}

//    public impl.
void SfxBaseModel::changing()
{
    // object already disposed?
    // object already disposed?
    SolarMutexGuard aGuard;
    if ( impl_isDisposed() )
        return;

    OINTERFACECONTAINERHELPER* pIC = m_pData->m_aInterfaceContainer.getContainer( ::getCppuType((const REFERENCE< XMODIFYLISTENER >*)0) );
    if( pIC )

    {
        EVENTOBJECT aEvent( (XMODEL *)this );
        OINTERFACEITERATORHELPER aIt( *pIC );
        while( aIt.hasMoreElements() )
        {
            try
            {
                ((XMODIFYLISTENER *)aIt.next())->modified( aEvent );
            }
            catch( RUNTIMEEXCEPTION& )
            {
                aIt.remove();
            }
        }
    }
}

//    public impl.
SfxObjectShell* SfxBaseModel::GetObjectShell() const
{
    return m_pData ? (SfxObjectShell*) m_pData->m_pObjectShell : 0;
}

//    public impl.
sal_Bool SfxBaseModel::impl_isDisposed() const
{
    return ( m_pData == NULL ) ;
}

//    private impl.
void SfxBaseModel::impl_store(SfxObjectShell* pObjectShell,const OUSTRING& sURL,
                                const SEQUENCE < PROPERTYVALUE >& seqArguments,
                                sal_Bool bSaveTo )
{
    if( !sURL.getLength() )
        throw ILLEGALARGUMENTIOEXCEPTION();

    SfxAllItemSet *aParams = new SfxAllItemSet( SFX_APP()->GetPool() );
    aParams->Put( SfxStringItem( SID_FILE_NAME, String(sURL) ) );
    if ( bSaveTo )
        aParams->Put( SfxBoolItem( SID_SAVETO, sal_True ) );

    TransformParameters( SID_SAVEASDOC, seqArguments, *aParams );
    sal_Bool aRet = pObjectShell->APISaveAs_Impl( sURL, aParams );
    DELETEZ( aParams );

    pObjectShell->ResetError();

    if ( !aRet )
    {
        throw ::com::sun::star::io::IOException();
    }
}

void SfxBaseModel::postEvent_Impl( const SfxEventHint& rHint )
{
    // object already disposed?
    if ( impl_isDisposed() )
        return;

    OINTERFACECONTAINERHELPER* pIC = m_pData->m_aInterfaceContainer.getContainer(
                                        ::getCppuType((const REFERENCE< XDOCEVENTLISTENER >*)0) );
    if( pIC )

    {
        OUSTRING aName = SfxEventConfiguration::GetEventName_Impl( rHint.GetEventId() );
        DOCEVENTOBJECT aEvent( (XMODEL *)this, aName );
        OINTERFACEITERATORHELPER aIt( *pIC );
        while( aIt.hasMoreElements() )
        {
            try
            {
                ((XDOCEVENTLISTENER *)aIt.next())->notifyEvent( aEvent );
            }
            catch( RUNTIMEEXCEPTION& )
            {
                aIt.remove();
            }
        }
    }
}

REFERENCE < XINDEXACCESS > SAL_CALL SfxBaseModel::getViewData() throw(::com::sun::star::uno::RuntimeException)
{
    // object already disposed?
    SolarMutexGuard aGuard;
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

            return REFERENCE < XINDEXACCESS >();

}

void SAL_CALL SfxBaseModel::setViewData( const REFERENCE < XINDEXACCESS >& aData ) throw(::com::sun::star::uno::RuntimeException)
{
    // object already disposed?
    SolarMutexGuard aGuard;
    if ( impl_isDisposed() )
        throw DISPOSEDEXCEPTION();

    m_pData->m_contViewData = aData;
}

/** calls all XEventListeners */
void SfxBaseModel::notifyEvent( const ::com::sun::star::document::EventObject& aEvent ) const
{
    // object already disposed?
    if ( impl_isDisposed() )
        return;

    OINTERFACECONTAINERHELPER* pIC = m_pData->m_aInterfaceContainer.getContainer(
                                        ::getCppuType((const REFERENCE< XDOCEVENTLISTENER >*)0) );
    if( pIC )

    {
        OINTERFACEITERATORHELPER aIt( *pIC );
        while( aIt.hasMoreElements() )
        {
            try
            {
                ((XDOCEVENTLISTENER *)aIt.next())->notifyEvent( aEvent );
            }
            catch( RUNTIMEEXCEPTION& )
            {
                aIt.remove();
            }
        }
    }
}

/** returns true if someone added a XEventListener to this XEventBroadcaster */
sal_Bool SfxBaseModel::hasEventListeners() const
{
    return !impl_isDisposed() && (NULL != m_pData->m_aInterfaceContainer.getContainer( ::getCppuType((const REFERENCE< XDOCEVENTLISTENER >*)0) ) );
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
