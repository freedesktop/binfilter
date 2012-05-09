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

#include <bf_svtools/eitem.hxx>
#include <bf_svtools/stritem.hxx>
#include <bf_so3/svstor.hxx>
#include <tools/zcodec.hxx>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/FilterOptionsRequest.hpp>
#include <com/sun/star/document/XInteractionFilterOptions.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <comphelper/processfactory.hxx>

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <bf_svtools/itemset.hxx>
#include <bf_svtools/sfxecode.hxx>
#include <cppuhelper/weak.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/classids.hxx>
#include <tools/cachestr.hxx>
#include <bf_svtools/addxmltostorageoptions.hxx>
#include <unotools/streamwrap.hxx>
#include <bf_svtools/saveopt.hxx>
#include <bf_svtools/useroptions.hxx>
#include <bf_svtools/pathoptions.hxx>
#include <tools/urlobj.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/ucbhelper.hxx>
#include <unotools/tempfile.hxx>
#include <ucbhelper/content.hxx>
#include <sot/storinfo.hxx>

#include "objsh.hxx"
#include "request.hxx"
#include "docfile.hxx"
#include "fltfnc.hxx"
#include "docfilt.hxx"
#include "docfac.hxx"
#include "cfgmgr.hxx"
#include "objshimp.hxx"
#include "sfxtypes.hxx"
#include "appdata.hxx"
#include "sfxsids.hrc"
#include "interno.hxx"
#include "module.hxx"
#include "app.hxx"
#include "openflag.hxx"
#include "helper.hxx"
#include "dlgcont.hxx"
#include "appuno.hxx"

#include "bf_basic/basmgr.hxx"

#include "scriptcont.hxx"
#include "event.hxx"
#include "fltoptint.hxx"

#include <legacysmgr/legacy_binfilters_smgr.hxx>
#include "bf_so3/staticbaseurl.hxx"
namespace binfilter {

#define S2BS(s) rtl::OUStringToOString(s, RTL_TEXTENCODING_MS_1252)


extern sal_uInt32 CheckPasswd_Impl( SfxObjectShell*, SfxItemPool&, SfxMedium* );


using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::document;
using namespace ::rtl;
using namespace ::cppu;

sal_Bool ShallSetBaseURL_Impl( SfxMedium &rMed )
{
    SvtSaveOptions aOpt;
    sal_Bool bIsRemote = rMed.IsRemote();
    return  (aOpt.IsSaveRelINet() && bIsRemote) || (aOpt.IsSaveRelFSys() && !bIsRemote);
}

sal_Bool GetPasswd_Impl( const SfxItemSet* pSet, String& rPasswd )
{
    const SfxPoolItem* pItem = NULL;
    if ( pSet && SFX_ITEM_SET == pSet->GetItemState( SID_PASSWORD, sal_True, &pItem ) )
    {
        DBG_ASSERT( pItem->IsA( TYPE(SfxStringItem) ), "wrong item type" );
        rPasswd = ( (const SfxStringItem*)pItem )->GetValue();
        return sal_True;
    }
    return sal_False;
}

sal_Bool SfxObjectShell::DoInitNew( SvStorage * pStor )
/*  [Beschreibung]

Diese von SvPersist geerbte virtuelle Methode wird gerufen, um
die SfxObjectShell-Instanz aus einem Storage (pStor != 0) bzw.
(pStor == 0) ganz neu zu initialisieren.

Wie alle Do...-Methoden liegt hier eine Steuerung vor, die eigentliche
Implementierung erfolgt, indem die ebenfalls virtuellen Methode
InitNew(SvStorate*) von der SfxObjectShell-Subclass implementiert wird.

F"ur pStor == 0 wird ein die SfxObjectShell-Instanz mit einem leeren
SfxMedium verbunden, sonst mit einem SfxMedium, welches auf den
als Parameter "ubergeben SvStorage verweist.

Erst nach InitNew() oder Load() ist das Objekt korrekt initialisiert.

[R"uckgabewert]
sal_True            Das Objekt wurde initialisiert.
sal_False           Das Objekt konnte nicht initialisiert werden
*/

{
    ModifyBlocker_Impl aBlock( this );
    if ( pStor )
        pMedium = new SfxMedium( pStor );
    else
    {
        bIsTmp = sal_True;
        pMedium = new SfxMedium;
    }

    if ( InitNew( pStor ) )
    {
        // empty documents always get their macros from the user, so there is no reason to restrict access
        pImp->nMacroMode = MacroExecMode::ALWAYS_EXECUTE_NO_WARN;

        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >  xModel ( GetModel(), ::com::sun::star::uno::UNO_QUERY );
        if ( xModel.is() )
        {
            SfxItemSet *pSet = GetMedium()->GetItemSet();
            pSet->Put( SfxStringItem( SID_FILTER_NAME, GetFactory().GetFilter(0)->GetFilterName() ) );
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aArgs;
            TransformItems( SID_OPENDOC, *pSet, aArgs );
            sal_Int32 nLength = aArgs.getLength();
            aArgs.realloc( nLength + 1 );
            aArgs[nLength].Name = DEFINE_CONST_UNICODE("Title");
            aArgs[nLength].Value <<= ::rtl::OUString( GetTitle( SFX_TITLE_DETECT ) );
            xModel->attachResource( ::rtl::OUString(), aArgs );
        }

        SetActivateEvent_Impl( SFX_EVENT_CREATEDOC );
        return sal_True;
    }
    return sal_False;
}

void SfxObjectShell::DoHandsOffNoMediumClose()
{
    const SfxFilter *pFilter = pMedium->GetFilter();
    if( !pFilter || pFilter->IsOwnFormat() || ( pFilter->GetFilterFlags() & SFX_FILTER_PACKED ) )
        HandsOff();

    // Force document library containers to release storage
    SotStorageRef xDummyStorage;
    SfxDialogLibraryContainer* pDialogCont = pImp->pDialogLibContainer;
    if( pDialogCont )
        pDialogCont->setStorage( xDummyStorage );

    SfxScriptLibraryContainer* pBasicCont = pImp->pBasicLibContainer;
    if( pBasicCont )
        pBasicCont->setStorage( xDummyStorage );
}

void SfxObjectShell::DoHandsOff()

/*  [Beschreibung]

Diese von SvPersist geerbte virtuelle Methode wird gerufen, um
das Objekt aufzufordern, den ihm zugeteilten SvStorage freizugeben,
insbesondere Substorages und Streams zu schlie"sen.

Als Do...-Methode liegt hier nur die Steuerung. Der Implementierer
von Subclasses kann die ebenfalls virtuelle Methode HandsOff()
implementieren, um seine Substorages und Streams zu schlie"sen.

Nach dem Aufruf dieser Methode, ist dem Objekt kein SfxMedium mehr
zugeordnet, bis SaveCompleted() durchlaufen ist.
*/

{
    DoHandsOffNoMediumClose();
    pMedium->Close();
}

sal_Bool SfxObjectShell::DoLoad(
    const String& /*rFileName*/, StreamMode /*nStreamMode*/, StorageMode /*nStorageMode*/)
{
    DBG_BF_ASSERT(0, "STRIP");  // VIRTUAL
    return sal_False;
}

sal_Bool SfxObjectShell::DoLoad( SvStorage * pStor )
/*  [Beschreibung]

Diese von SvPersist geerbte virtuelle Methode steuert das Laden
des Objektes aus einem Storage. Dabei wird der SvStorage zun"achst
in einem SfxMedium verpackt und SfxObjectShell::DoLoad(SfxMedium*)
mit diesem gerufen.

[R"uckgabewert]
sal_True                Das Objekt wurde initialisiert.
sal_False               Das Objekt konnte nicht initialisiert werden
*/

{
    pMedium = new SfxMedium( pStor );
    if ( DoLoad(pMedium) )
    {
        if ( SFX_CREATE_MODE_EMBEDDED == eCreateMode )
        {
            ModifyBlocker_Impl aBlock( this );
            SetTitle( pStor->GetName() );
        }
        return sal_True;
    }

    return sal_False;
}

sal_Bool SfxObjectShell::DoLoad( SfxMedium *pMed )

/*  [Beschreibung]

Diese Methode steuert das Laden des Objektes aus dem von dem
"ubergebenen SfxMedium beschriebenen Medium. Hinterher ist das Objekt
mit diesem SfxMedium verbunden.

Handelt es sich bei dem SfxMedium um einen Storage im eigenen Format,
wird die virtuelle Methode SvPersit::Load(SvStorage*) gerufen, welche
die Implementierer von Subclasses "uberladen m"ussen, um das Objekt
aus einem eigenen Storage zu laden (=> Swapping m"oeglich).

Handelt es sich bei dem SfxMedium um einen Storage in einem fremden
Format, oder um ein Flat-File, dann wird die virtuelle Methode
<SfxObjectShell::ConvertFrom(SfxMedium*)> gerufen, welche die
Implementierer von Subclasses "uberladen m"ussen, um das Objekt
aus dem SfxMedium zu konvertieren. W"ahrend der Bearbeitung ist
das Objekt dann mit einem tempor"aren SvStorage verbunden.

Erst nach InitNew() oder Load() ist das Objekt korrekt
initialisiert.

[R"uckgabewert]
sal_True                Das Objekt wurde geladen.
sal_False           Das Objekt konnte nicht geladen werden
*/

{
    /*SfxApplication *pSfxApp =*/ SFX_APP();
    ModifyBlocker_Impl aBlock( this );

    pMedium = pMed;
    sal_Bool bOk = sal_False;
    SfxItemSet* pSet = pMedium->GetItemSet();
    if( !pImp->nEventId )
    {
        SFX_ITEMSET_ARG(
            pSet, pTemplateItem, SfxBoolItem,
            SID_TEMPLATE, sal_False);
        SetActivateEvent_Impl(
            ( pTemplateItem && pTemplateItem->GetValue() )
            ? SFX_EVENT_CREATEDOC : SFX_EVENT_OPENDOC );
    }


    SFX_ITEMSET_ARG( pSet, pBaseItem, SfxStringItem,
                    SID_BASEURL, sal_False);
    rtl::OUString aBaseURL;
    SFX_ITEMSET_ARG( pMedium->GetItemSet(), pSalvageItem, SfxStringItem, SID_DOC_SALVAGE, sal_False);
    if( pBaseItem )
        aBaseURL = pBaseItem->GetValue();
    else
    {
        if( GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
        {
            aBaseURL = ::binfilter::StaticBaseUrl::GetBaseURL();
            SetBaseURL( aBaseURL );
        }
        else if ( pSalvageItem )
        {
            rtl::OUString aLclName( pMed->GetPhysicalName() );
            ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aLclName, aBaseURL );
        }
        else
            aBaseURL = pMed->GetBaseURL();
    }

    /*SfxApplication* pApp =*/ SFX_APP();
    pImp->nLoadedFlags = 0;

    if ( pMedium->GetFilter() )
    {
        sal_uInt32 nError = HandleFilter( pMedium, this );
        if ( nError != ERRCODE_NONE )
            SetError( nError );
    }

    if ( GetError() == ERRCODE_NONE && pMed->IsStorage() )
    {
        SvStorageRef xStor( pMed->GetStorage() );
        if( pMed->GetLastStorageCreationState() == ERRCODE_NONE )
        {
            if( xStor.Is() && !xStor->GetError() && pMed->GetFilter() && pMed->GetFilter()->GetVersion() < SOFFICE_FILEFORMAT_60 )
            {
                // Undoobjekte aufraeumen, muss vor dem eigentlichen Laden erfolgen
                SvEmbeddedObjectRef xThis = this;
                SvPersistRef xPer;
                if ( xThis.Is() )
                    xPer = new SvEmbeddedObject;
                else
                    xPer = new SvPersist;

                xPer->DoOwnerLoad(xStor);
                xPer->CleanUp();
            }

            if ( xStor.Is() )
            {
                SvStorageInfoList aList;
                xStor->FillInfoList( &aList );
                if ( aList.empty() && !xStor->IsOLEStorage() )
                    SetError( ERRCODE_IO_BROKENPACKAGE );
                else
                {
                    BOOL bHasMacros = FALSE;
                    if ( xStor->IsOLEStorage() )
                        bHasMacros = BasicManager::HasBasicWithModules(
                        *xStor,
                        ::binfilter::StaticBaseUrl::GetBaseURL(
                            INetURLObject::NO_DECODE) );
                    else
                        bHasMacros = xStor->IsStorage( String::CreateFromAscii("Basic") );

                    if ( bHasMacros )
                        AdjustMacroMode( String() );
                    else
                    {
                        // if macros will be added by the user later, the security check is obsolete
                        pImp->nMacroMode = MacroExecMode::ALWAYS_EXECUTE_NO_WARN;
                    }
                }
            }

            // Load
            if ( !GetError() )
            {
                const String aOldURL( ::binfilter::StaticBaseUrl::GetBaseURL() );
                if( aBaseURL.getLength() ) ::binfilter::StaticBaseUrl::SetBaseURL( aBaseURL );
                pImp->nLoadedFlags = 0;
                bOk = xStor.Is() && LoadOwnFormat( *pMed );
                ::binfilter::StaticBaseUrl::SetBaseURL( aOldURL );
                if ( bOk )
                {
                    GetDocInfo().Load(xStor);
                    bHasName = sal_True;
                }
                else
                    SetError( ERRCODE_ABORT );
            }
        }
        else
            SetError( pMed->GetLastStorageCreationState() );
    }
    else if ( GetError() == ERRCODE_NONE && InitNew(0) )
    {
        // Name vor ConvertFrom setzen, damit GetSbxObject() schon funktioniert
        bHasName = sal_True;
        // Importieren
        const String aOldURL( ::binfilter::StaticBaseUrl::GetBaseURL() );
        if( aBaseURL.getLength() )
            ::binfilter::StaticBaseUrl::SetBaseURL( aBaseURL );
        if( !pMedium->GetFilter()->UsesStorage() )
            pMedium->GetInStream();
        else
            pMedium->GetStorage();

        pImp->nLoadedFlags = 0;
        bOk = ConvertFrom(*pMedium);
        ::binfilter::StaticBaseUrl::SetBaseURL( aOldURL );

        if( bOk && pMedium->GetOpenMode() & STREAM_WRITE )
        //Medium offen halten um andere Zugriffe zu verhindern
        {
            if(pMedium->GetFilter() && pMedium->GetFilter()->UsesStorage())
            {
                pMedium->GetStorage();
                if( pMedium->GetLastStorageCreationState() != ERRCODE_NONE )
                    pMedium->SetError( pMedium->GetLastStorageCreationState() );
            }
            else
                pMedium->GetInStream();
            if(pMedium->GetError())
                bOk = sal_False;
        }
    }

    if ( bOk )
    {
        try
        {
            ::ucbhelper::Content aContent( pMedium->GetName(), ::com::sun::star::uno::Reference < XCommandEnvironment >() );
            ::com::sun::star::uno::Reference < XPropertySetInfo > xProps = aContent.getProperties();
            if ( xProps.is() )
            {
                ::rtl::OUString aAuthor( RTL_CONSTASCII_USTRINGPARAM("Author") );
                ::rtl::OUString aKeywords( RTL_CONSTASCII_USTRINGPARAM("Keywords") );
                ::rtl::OUString aSubject( RTL_CONSTASCII_USTRINGPARAM("Subject") );
                Any aAny;
                ::rtl::OUString aValue;
                SfxDocumentInfo& rInfo = GetDocInfo();
                if ( xProps->hasPropertyByName( aAuthor ) )
                {
                    aAny = aContent.getPropertyValue( aAuthor );
                    if ( ( aAny >>= aValue ) )
                        rInfo.SetCreated( SfxStamp( String( aValue ) ) );
                }
                if ( xProps->hasPropertyByName( aKeywords ) )
                {
                    aAny = aContent.getPropertyValue( aKeywords );
                    if ( ( aAny >>= aValue ) )
                        rInfo.SetKeywords( aValue );
                }
                if ( xProps->hasPropertyByName( aSubject ) )
                {
                    aAny = aContent.getPropertyValue( aSubject );
                    if ( ( aAny >>= aValue ) )
                        rInfo.SetTheme( aValue );
                }
            }
        }
        catch( Exception& )
        {
        }

        ::rtl::OUString aTitle = GetTitle( SFX_TITLE_DETECT );

        // Falls nicht asynchron geladen wird selbst FinishedLoading aufrufen
        if ( !( pImp->nLoadedFlags & SFX_LOADED_MAINDOCUMENT ) &&
            ( !pMedium->GetFilter() ||
            pMedium->GetFilter()->UsesStorage() ||
            !( pMedium->GetFilter()->GetFilterFlags() & SFX_FILTER_ASYNC ) ) )
            FinishedLoading( SFX_LOADED_MAINDOCUMENT );

        if ( pSalvageItem )
        {
            pImp->aTempName = pMedium->GetPhysicalName();
            pMedium->GetItemSet()->ClearItem( SID_DOC_SALVAGE );
            pMedium->GetItemSet()->ClearItem( SID_FILE_NAME );
        }
        else
        {
            pMedium->GetItemSet()->ClearItem( SID_PROGRESS_STATUSBAR_CONTROL );
            pMedium->GetItemSet()->ClearItem( SID_DOCUMENT );
        }

        pMedium->GetItemSet()->ClearItem( SID_REFERER );
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >  xModel ( GetModel(), ::com::sun::star::uno::UNO_QUERY );
        if ( xModel.is() )
        {
            ::rtl::OUString aURL = GetMedium()->GetOrigURL();
            SfxItemSet *pLclSet = GetMedium()->GetItemSet();
            if ( !GetMedium()->IsReadOnly() )
                pLclSet->ClearItem( SID_INPUTSTREAM );
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aArgs;
            TransformItems( SID_OPENDOC, *pLclSet, aArgs );
            xModel->attachResource( aURL, aArgs );
        }

        if( IsOwnStorageFormat_Impl(*pMed) && pMed->GetFilter() )
        {
        }
        Broadcast( SfxSimpleHint(SFX_HINT_NAMECHANGED) );
    }

    if ( SFX_CREATE_MODE_EMBEDDED != eCreateMode )
    {
        GetpApp()->HideStatusText();
    }

    return bOk;
}

sal_uInt32 SfxObjectShell::HandleFilter( SfxMedium* pMedium, SfxObjectShell* pDoc )
{
    sal_uInt32 nError = ERRCODE_NONE;
    SfxItemSet* pSet = pMedium->GetItemSet();
    SFX_ITEMSET_ARG( pSet, pOptions_A, SfxStringItem, SID_FILE_FILTEROPTIONS, sal_False );
    SFX_ITEMSET_ARG( pSet, pData_A, SfxUsrAnyItem, SID_FILTER_DATA, sal_False );
    if ( !pData_A && !pOptions_A )
    {
        ::com::sun::star::uno::Reference< XMultiServiceFactory > xServiceManager = ::legacy_binfilters::getLegacyProcessServiceFactory();
        ::com::sun::star::uno::Reference< XNameAccess > xFilterCFG;
        if( xServiceManager.is() )
        {
            xFilterCFG = ::com::sun::star::uno::Reference< XNameAccess >(
                xServiceManager->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.FilterFactory" )) ),
                UNO_QUERY );
        }

        if( xFilterCFG.is() )
        {
            BOOL bAbort = FALSE;
            try {
                const SfxFilter* pFilter = pMedium->GetFilter();
                Sequence < PropertyValue > aProps;
                Any aAny = xFilterCFG->getByName( pFilter->GetName() );
                if ( aAny >>= aProps )
                {
                    sal_Int32 nPropertyCount = aProps.getLength();
                    for( sal_Int32 nProperty=0; nProperty < nPropertyCount; ++nProperty )
                        if( aProps[nProperty].Name.equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "UIComponent" ))) )
                        {
                            ::rtl::OUString aServiceName;
                            aProps[nProperty].Value >>= aServiceName;
                            if( aServiceName.getLength() )
                            {
                                ::com::sun::star::uno::Reference< XInteractionHandler > rHandler = pMedium->GetInteractionHandler();
                                if( rHandler.is() )
                                {
                                    // we need some properties in the media descriptor, so we have to make sure that they are in
                                    Any aLclAny;
                                    aLclAny <<= pMedium->GetInputStream();
                                    if ( pSet->GetItemState( SID_INPUTSTREAM ) < SFX_ITEM_SET )
                                    pSet->Put( SfxUsrAnyItem( SID_INPUTSTREAM, aLclAny ) );
                                    if ( pSet->GetItemState( SID_FILE_NAME ) < SFX_ITEM_SET )
                                        pSet->Put( SfxStringItem( SID_FILE_NAME, pMedium->GetName() ) );
                                    if ( pSet->GetItemState( SID_FILTER_NAME ) < SFX_ITEM_SET )
                                        pSet->Put( SfxStringItem( SID_FILTER_NAME, pFilter->GetName() ) );

                                    Sequence< PropertyValue > rProperties;
                                    TransformItems( SID_OPENDOC, *pSet, rProperties );
                                    RequestFilterOptions* pFORequest = new RequestFilterOptions( pDoc->GetModel(), rProperties );
                                    ::com::sun::star::uno::Reference< XInteractionRequest > rRequest( pFORequest );
                                    rHandler->handle( rRequest );

                                    if ( !pFORequest->isAbort() )
                                    {
                                        SfxAllItemSet aNewParams( pDoc->GetPool() );
                                        TransformParameters( SID_OPENDOC,
                                                            pFORequest->getFilterOptions(),
                                                            aNewParams );
                                        SFX_ITEMSET_ARG( &aNewParams,
                                                        pOptions,
                                                        SfxStringItem,
                                                        SID_FILE_FILTEROPTIONS,
                                                        sal_False );
                                        if ( pOptions )
                                            pSet->Put( *pOptions );

                                        SFX_ITEMSET_ARG( &aNewParams,
                                                        pData,
                                                        SfxUsrAnyItem,
                                                        SID_FILTER_DATA,
                                                        sal_False );
                                        if ( pData )
                                            pSet->Put( *pData );
                                    }
                                    else
                                        bAbort = TRUE;
                                }
                            }
                            break;
                        }
                }
                if( bAbort )
                {
                    // filter options were not entered
                    nError = ERRCODE_ABORT;
                }
            }
            catch( NoSuchElementException& )
            {
                // the filter name is unknown
                nError = ERRCODE_IO_INVALIDPARAMETER;
            }
            catch( Exception& )
            {
                nError = ERRCODE_ABORT;
            }
        }
    }
    return nError;
}

sal_Bool SfxObjectShell::IsOwnStorageFormat_Impl(const SfxMedium &rMedium) const
{
    return !rMedium.GetFilter() || // Embedded
        ( rMedium.GetFilter()->IsOwnFormat() &&
            rMedium.GetFilter()->UsesStorage() );
}

void Lock_Impl( SfxObjectShell* /*pDoc*/, BOOL /*bLock*/ )
{
}

sal_Bool SfxObjectShell::SaveTo_Impl
(
    SfxMedium &rMedium, // Medium, in das gespeichert werden soll
    const SfxItemSet* /*pSet*/,
    BOOL bPrepareForDirectAccess
)

/*  [Beschreibung]

    Schreibt den aktuellen Inhalt in das Medium rMedium.
    Ist das Zielmedium kein Storage, so wird ueber ein temporaeres
    Medium gespeichert, sonst direkt, da das Medium transacted
    geschaltet ist, wenn wir es selbst geoeffnet haben und falls wir
    Server sind entweder der Container einen transacted Storage zur
    Verfuegung stellt oder selbst einen temporaeren Storage erzeugt hat.
*/

{
    SfxForceLinkTimer_Impl aFLT( this );
    EnableSetModified( FALSE );

    const SfxFilter *pFilter = rMedium.GetFilter();
    if ( !pFilter )
    {
        // if no filter was set, use the default filter
        // this should be changed in the feature, it should be an error!
        pFilter = GetFactory().GetFilter(0);
        rMedium.SetFilter(pFilter);
    }

    if( pFilter->UsesStorage() )
        // create an output storage in the correct format
        rMedium.GetOutputStorage( SOFFICE_FILEFORMAT_60 <= pFilter->GetVersion() );
    else
        rMedium.GetOutStream();

    if( rMedium.GetErrorCode() )
        return sal_False;

    sal_Bool bOldStat = pImp->bForbidReload;
    pImp->bForbidReload = sal_True;

    // lock user interface while saving the document
    Lock_Impl( this, sal_True );

    sal_Bool bOk = sal_False;
    if( sal_True )
    {
        SvStorageRef aMedRef = rMedium.GetStorage();
        if ( !aMedRef.Is() )
        {
            // no saving without storage, unlock UI and return
            Lock_Impl( this, sal_False );
            return sal_False;
        }

        // transfer password from the parameters to the storage
        String aPasswd;
        if ( GetPasswd_Impl( rMedium.GetItemSet(), aPasswd ) )
            aMedRef->SetKey( S2BS( aPasswd ) ); //!!! (pb) needs new implementation

        const SfxFilter* pLclFilter = rMedium.GetFilter();
        if(  ((SvStorage *)aMedRef) == ((SvStorage *)GetStorage() ) )
        {
            // target storage and object storage are identical, should never happen here
            OSL_FAIL( "Saving storage without copy!");
            aMedRef->SetVersion( pLclFilter->GetVersion() );
            bOk = Save();
        }
        else
            // save to target
            bOk = false;
    }
    // SetModified must be enabled when SaveCompleted is called, otherwise the modified flag of child objects will not be cleared
    EnableSetModified( sal_True );

    sal_Bool bCopyTo = sal_False;
    SfxItemSet *pMedSet = rMedium.GetItemSet();
    if( pMedSet )
    {
        SFX_ITEMSET_ARG( pMedSet, pSaveToItem, SfxBoolItem, SID_SAVETO, sal_False );
        bCopyTo =   GetCreateMode() == SFX_CREATE_MODE_EMBEDDED ||
                    (pSaveToItem && pSaveToItem->GetValue());
    }

    if( bOk )
    {
        // remember new object storage, if it is a temporary one, because we will need it for a "SaveCompleted" later
        SvStorageRef xNewTempRef;
        if ( bOk && bPrepareForDirectAccess )
        {
            // if the target medium is an alien format and the "old" medium was an own format, the object storage
            // must be exchanged, because now we need a new temporary storage as object storage
            BOOL bNeedsStorage = !bCopyTo && IsOwnStorageFormat_Impl(*pMedium) && !IsOwnStorageFormat_Impl(rMedium);
            if ( bNeedsStorage )
            {
                if( !pMedium->GetName().Len() )
                    // if the old object storage was a temporary one too, we can continue with it
                    xNewTempRef = GetStorage();
                else
                {
                    // copy storage of old medium to new temporary storage and take this over
                    if( ConnectTmpStorage_Impl( pMedium->GetStorage() ) )
                        xNewTempRef = GetStorage();
                    else
                        bOk = sal_False;
                }
            }

            // When the new medium ( rMedium ) has the same name as the current one,
            // we need to call DoHandsOff() so Commit() can overwrite the old version
            if ( bOk && pMedium && ( rMedium.GetName().EqualsIgnoreCaseAscii( pMedium->GetName() ) )
            && rMedium.GetName().CompareIgnoreCaseToAscii( "private:stream", 14 ) != COMPARE_EQUAL )
                DoHandsOff();
        }

        if ( bOk )
        {
            // transfer data to its destinated location
            EnableSetModified( sal_False );
            RegisterTransfer( rMedium );
            bOk = rMedium.Commit();
            EnableSetModified( sal_True );
        }
    }

    // unlock user interface
    Lock_Impl( this, sal_False );
    pImp->bForbidReload = bOldStat;

    if ( bOk )
    {
        DBG_ASSERT( pFilter, "No filter after successful save?!" );
        if( pFilter )
        {
            if( !bCopyTo && pFilter->IsAlienFormat() )
                // set flag, that the user will be warned for possible data loss on closing this document
                pImp->bDidDangerousSave=sal_True;
            else
                pImp->bDidDangerousSave=sal_False;
        }

        try
        {
            ::ucbhelper::Content aContent( rMedium.GetName(), ::com::sun::star::uno::Reference < XCommandEnvironment >() );
            ::com::sun::star::uno::Reference < XPropertySetInfo > xProps = aContent.getProperties();
            if ( xProps.is() )
            {
                ::rtl::OUString aAuthor( RTL_CONSTASCII_USTRINGPARAM("Author") );
                ::rtl::OUString aKeywords( RTL_CONSTASCII_USTRINGPARAM("Keywords") );
                ::rtl::OUString aSubject( RTL_CONSTASCII_USTRINGPARAM("Subject") );
                Any aAny;
                if ( xProps->hasPropertyByName( aAuthor ) )
                {
                    aAny <<= ::rtl::OUString( GetDocInfo().GetCreated().GetName() );
                    aContent.setPropertyValue( aAuthor, aAny );
                }
                if ( xProps->hasPropertyByName( aKeywords ) )
                {
                    aAny <<= ::rtl::OUString( GetDocInfo().GetKeywords() );
                    aContent.setPropertyValue( aKeywords, aAny );
                }
                if ( xProps->hasPropertyByName( aSubject ) )
                {
                    aAny <<= ::rtl::OUString( GetDocInfo().GetTheme() );
                    aContent.setPropertyValue( aSubject, aAny );
                }
            }
        }
        catch( Exception& )
        {
        }
    }

    return bOk;
}

sal_Bool SfxObjectShell::ConnectTmpStorage_Impl( SvStorage* pStg)

/*   [Beschreibung]

Arbeitet die Applikation auf einem temporaeren Storage,
so darf der temporaere Storage nicht aus dem SaveCompleted
genommen werden. Daher wird in diesem Fall schon hier an
den neuen Storage connected. SaveCompleted tut dann nichts.

*/

{
    // wenn es kein temp. Storage ist, einen anlegen
    SvStorageRef aTmpMed = new SvStorage( (pStg->GetVersion() >= SOFFICE_FILEFORMAT_60), String() );

    // nach HandsOff muss der alte Storage wieder eingesetzt werden
    if ( !pStg->CopyTo(aTmpMed) )
    {
        SetError(aTmpMed->GetErrorCode());
        aTmpMed.Clear();
        return sal_False;
    }

    SetError(GetMedium()->GetErrorCode());
    //SaveCompleted(aTmpMed); // neuer temp. Storage; gibt alten frei
    return sal_True;
}

sal_Bool SfxObjectShell::ConvertFrom
(
    SfxMedium&  /*rMedium*/     /*  <SfxMedium>, welches die Quell-Datei beschreibt
                        (z.B. Dateiname, <SfxFilter>, Open-Modi etc.) */
)

/*  [Beschreibung]

Diese Methode wird zum Laden von Dokumenten "uber alle Filter gerufen,
die nicht SFX_FILTER_OWN sind oder f"ur die kein Clipboard-Format
registriert wurde (also kein Storage-Format benutzen). Mit anderen Worten:
mit dieser Methode wird importiert.

Das hier zu "offende File sollte "uber 'rMedium' ge"offnet werden,
um die richtigen Open-Modi zu gew"ahrleisten. Insbesondere wenn das
Format beibehalten wird (nur m"oglich bei SFX_FILTER_SIMULATE oder
SFX_FILTER_ONW) mu\s die Datei STREAM_SHARE_DENYWRITE ge"offnet werden.


[R"uckgabewert]

sal_Bool                sal_True
                Das Dokument konnte geladen werden.

                sal_False
                Das Dokument konnte nicht geladen werden, ein
                Fehlercode ist mit <SvMedium::GetError()const> zu
                erhalten.


[Beispiel]

sal_Bool DocSh::ConvertFrom( SfxMedium &rMedium )
{
    SvStreamRef xStream = rMedium.GetInStream();
    if( xStream.is() )
    {
        xStream->SetBufferSize(4096);
        *xStream >> ...;

        // NICHT 'rMedium.CloseInStream()' rufen! File gelockt halten!
        return SVSTREAM_OK == rMedium.GetError();
    }

    return sal_False;
}

[Querverweise]

<SfxObjectShell::ConvertTo(SfxMedium&)>
<SFX_FILTER_REGISTRATION>
*/
{
    return sal_False;
}

sal_Bool SfxObjectShell::ConvertTo
(
    SfxMedium&  /*rMedium*/     /*  <SfxMedium>, welches die Ziel-Datei beschreibt
                        (z.B. Dateiname, <SfxFilter>, Open-Modi etc.) */
)

/*  [Beschreibung]

Diese Methode wird zum Speichern von Dokumenten "uber alle Filter gerufen,
die nicht SFX_FILTER_OWN sind oder f"ur die kein Clipboard-Format
registriert wurde (also kein Storage-Format benutzen). Mit anderen Worten:
mit dieser Methode wird exportiert.

Das hier zu "offende File sollte "uber 'rMedium' ge"offnet werden,
um die richtigen Open-Modi zu gew"ahrleisten. Insbesondere wenn das
Format beibehalten wird (nur m"oglich bei SFX_FILTER_SIMULATE oder
SFX_FILTER_ONW) mu\s die Datei auch nach dem Speichern im Modus
STREAM_SHARE_DENYWRITE ge"offnet bleiben.


[R"uckgabewert]

sal_Bool                sal_True
                Das Dokument konnte gespeichert werden.

                sal_False
                Das Dokument konnte nicht gespeichert werden, ein
                Fehlercode ist mit <SvMedium::GetError()const> zu
                erhalten.


[Beispiel]

sal_Bool DocSh::ConvertTo( SfxMedium &rMedium )
{
    SvStreamRef xStream = rMedium.GetOutStream();
    if ( xStream.is() )
    {
        xStream->SetBufferSize(4096);
        *xStream << ...;

        rMedium.CloseOutStream(); // "offnet automatisch wieder den InStream
        return SVSTREAM_OK == rMedium.GetError();
    }
    return sal_False ;
}


[Querverweise]

<SfxObjectShell::ConvertFrom(SfxMedium&)>
<SFX_FILTER_REGISTRATION>
*/

/*?*/ {
/*?*/   return sal_False;
/*?*/ }

//-------------------------------------------------------------------------


/*N*/ sal_Bool SfxObjectShell::CommonSaveAs_Impl
/*N*/ (
/*N*/   const INetURLObject&   aURL,
/*N*/   const String&   aFilterName,
/*N*/   SfxItemSet*     aParams
/*N*/ )
/*N*/ {
/*N*/   SFX_APP()->NotifyEvent(SfxEventHint(SFX_EVENT_SAVEASDOC,this));
/*N*/   BOOL bWasReadonly = IsReadOnly();
/*N*/
/*N*/   if( aURL.HasError() )
/*N*/   {
/*N*/       SetError( ERRCODE_IO_INVALIDPARAMETER );
/*N*/       return sal_False;
/*N*/   }
/*N*/
/*N*/     DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "Illegal URL!" );
/*N*/     DBG_ASSERT( aParams->Count() != 0, "fehlerhafte Parameter");
/*N*/
/*N*/     SFX_ITEMSET_ARG( aParams, pSaveToItem, SfxBoolItem, SID_SAVETO, sal_False );
/*N*/     sal_Bool bSaveTo = pSaveToItem ? pSaveToItem->GetValue() : sal_False;
/*N*/
/*N*/     const SfxFilter* pFilter = GetFactory().GetFilterContainer()->GetFilter( aFilterName );
/*N*/     if ( !pFilter
/*N*/       || !pFilter->CanExport()
/*N*/       || (!bSaveTo && !pFilter->CanImport()) )
/*N*/     {
/*N*/         SetError( ERRCODE_IO_INVALIDPARAMETER );
/*N*/         return sal_False;
/*N*/     }
/*N*/
/*N*/   pImp->bPasswd = aParams && SFX_ITEM_SET == aParams->GetItemState(SID_PASSWORD);
/*N*/
/*N*/   SfxMedium *pActMed = GetMedium();
/*N*/   const INetURLObject aActName(pActMed->GetName());
/*N*/
/*N*/   if ( aURL == aActName
/*N*/       && aURL != INetURLObject( OUString( RTL_CONSTASCII_USTRINGPARAM( "private:stream" )) ) )
/*N*/   {
/*?*/       if ( IsReadOnly() )
/*?*/       {
/*?*/           SetError(ERRCODE_SFX_DOCUMENTREADONLY);
/*?*/           return sal_False;
/*?*/       }
/*?*/       // gleicher Filter? -> Save()
/*?*/       const SfxFilter *pLclFilter = pActMed->GetFilter();
/*?*/         if ( pLclFilter && pLclFilter->GetFilterName() == aFilterName )
/*?*/       {
/*?*/           pImp->bIsSaving=sal_False;
/*?*/           if ( aParams )
/*?*/           {
/*?*/               SfxItemSet* pSet = pMedium->GetItemSet();
/*?*/               pSet->ClearItem( SID_PASSWORD );
/*?*/               pSet->Put( *aParams );
/*?*/           }
/*?*/           return sal_False;
/*?*/       }
/*N*/   }

/*N*/     if( SFX_ITEM_SET != aParams->GetItemState(SID_UNPACK) && SvtSaveOptions().IsSaveUnpacked() )
/*N*/         aParams->Put( SfxBoolItem( SID_UNPACK, sal_False ) );
/*N*/
/*N*/   if ( PreDoSaveAs_Impl(aURL.GetMainURL( INetURLObject::NO_DECODE ),aFilterName,aParams))
/*N*/   {
/*N*/       pImp->bWaitingForPicklist = sal_True;
/*N*/       if (!pImp->bSetStandardName)
/*N*/           pImp->bDidWarnFormat=sal_False;
/*N*/
/*N*/       // Daten am Medium updaten
/*N*/       SfxItemSet *pSet = GetMedium()->GetItemSet();
/*N*/       pSet->ClearItem( SID_INTERACTIONHANDLER );
/*N*/
/*N*/       if ( !bSaveTo )
/*N*/       {
/*N*/           pSet->ClearItem( SID_REFERER );
/*N*/           pSet->ClearItem( SID_POSTDATA );
/*N*/           pSet->ClearItem( SID_TEMPLATE );
/*N*/           pSet->ClearItem( SID_DOC_READONLY );
/*N*/           pSet->ClearItem( SID_CONTENTTYPE );
/*N*/           pSet->ClearItem( SID_CHARSET );
/*N*/           pSet->ClearItem( SID_FILTER_NAME );
/*N*/           pSet->ClearItem( SID_OPTIONS );
/*N*/           //pSet->ClearItem( SID_FILE_FILTEROPTIONS );
/*N*/           pSet->ClearItem( SID_VERSION );
/*N*/           //pSet->ClearItem( SID_USE_FILTEROPTIONS );
/*N*/           pSet->ClearItem( SID_EDITDOC );
/*N*/
/*N*/           SFX_ITEMSET_GET( (*aParams), pFilterItem, SfxStringItem, SID_FILTER_NAME, sal_False );
/*N*/           if ( pFilterItem )
/*N*/               pSet->Put( *pFilterItem );
/*N*/
/*N*/           SFX_ITEMSET_GET( (*aParams), pOptionsItem, SfxStringItem, SID_OPTIONS, sal_False );
/*N*/           if ( pOptionsItem )
/*N*/               pSet->Put( *pOptionsItem );
/*N*/
/*N*/           SFX_ITEMSET_GET( (*aParams), pFilterOptItem, SfxStringItem, SID_FILE_FILTEROPTIONS, sal_False );
/*N*/           if ( pFilterOptItem )
/*N*/               pSet->Put( *pFilterOptItem );
/*N*/       }
/*N*/
/*N*/       SFX_APP()->NotifyEvent(SfxEventHint(SFX_EVENT_SAVEASDOCDONE,this));
/*N*/
/*N*/       if ( bWasReadonly && !bSaveTo )
/*N*/           Broadcast( SfxSimpleHint(SFX_HINT_MODECHANGED) );
/*N*/
/*N*/       return sal_True;
/*N*/   }
/*N*/   else
/*N*/       return sal_False;
/*N*/ }

//-------------------------------------------------------------------------

/*N*/ sal_Bool SfxObjectShell::PreDoSaveAs_Impl
/*N*/ (
/*N*/   const String&   rFileName,
/*N*/   const String&   aFilterName,
/*N*/   SfxItemSet*     pParams
/*N*/ )
/*N*/ {
/*N*/     // copy all items stored in the itemset of the current medium
/*N*/     SfxAllItemSet* pMergedParams = new SfxAllItemSet( *pMedium->GetItemSet() );
/*N*/
/*N*/     // in "SaveAs" title and password will be cleared ( maybe the new itemset contains new values, otherwise they will be empty )
/*N*/   pMergedParams->ClearItem( SID_PASSWORD );
/*N*/   pMergedParams->ClearItem( SID_DOCINFO_TITLE );
/*N*/
/*N*/   pMergedParams->ClearItem( SID_INPUTSTREAM );
/*N*/   pMergedParams->ClearItem( SID_CONTENT );
/*N*/
/*N*/   pMergedParams->ClearItem( SID_REPAIRPACKAGE );
/*N*/
/*N*/     // "SaveAs" will never store any version information - it's a complete new file !
/*N*/     pMergedParams->ClearItem( SID_VERSION );
/*N*/
/*N*/     // merge the new parameters into the copy
/*N*/     // all values present in both itemsets will be overwritten by the new parameters
/*N*/     if( pParams )
/*N*/       pMergedParams->Put( *pParams );
/*N*/     //DELETEZ( pParams );
/*N*/
/*N*/ #ifdef DBG_UTIL
/*N*/     if ( pMergedParams->GetItemState( SID_DOC_SALVAGE) >= SFX_ITEM_SET )
/*N*/         OSL_FAIL("Salvage item present in Itemset, check the parameters!");
/*N*/ #endif
/*N*/
/*N*/     // should be unneccessary - too hot to handle!
/*N*/   pMergedParams->ClearItem( SID_DOC_SALVAGE );
/*N*/
/*N*/     // take over the new merged itemset
/*N*/   pParams = pMergedParams;
/*N*/
/*N*/     // create a medium for the target URL
/*N*/     SfxMedium *pNewFile = new SfxMedium( rFileName, STREAM_READWRITE | STREAM_SHARE_DENYWRITE, sal_False, 0, pParams );
/*N*/
/*N*/     // set filter; if no filter is given, take the default filter of the factory
/*N*/     if ( aFilterName.Len() )
/*N*/         pNewFile->SetFilter( GetFactory(), aFilterName );
/*N*/   else
/*N*/         pNewFile->SetFilter( GetFactory().GetFilterContainer()->GetFilter(0) );
/*N*/
/*N*/     // saving is alway done using a temporary file
/*N*/     pNewFile->CreateTempFileNoCopy();
/*N*/     if ( pNewFile->GetErrorCode() != ERRCODE_NONE )
/*N*/     {
/*N*/         // creating temporary file failed ( f.e. floppy disk not inserted! )
/*N*/         SetError( pNewFile->GetError() );
/*N*/         delete pNewFile;
/*N*/         return sal_False;
/*N*/     }
/*N*/
/*N*/     // check if a "SaveTo" is wanted, no "SaveAs"
/*N*/     SFX_ITEMSET_ARG( pParams, pSaveToItem, SfxBoolItem, SID_SAVETO, sal_False );
/*N*/     sal_Bool bCopyTo = GetCreateMode() == SFX_CREATE_MODE_EMBEDDED || (pSaveToItem && pSaveToItem->GetValue());
/*N*/
/*N*/     // some base URL stuff ( awful, but not avoidable ... )
/*N*/     const String aOldURL( ::binfilter::StaticBaseUrl::GetBaseURL() );
/*N*/   if( GetCreateMode() != SFX_CREATE_MODE_EMBEDDED )
/*N*/   {
/*N*/       if ( ShallSetBaseURL_Impl(*pNewFile) )
/*N*/           ::binfilter::StaticBaseUrl::SetBaseURL( pNewFile->GetBaseURL() );
/*N*/       else
/*N*/           ::binfilter::StaticBaseUrl::SetBaseURL( String() );
/*N*/   }
/*N*/
/*N*/     // distinguish between "Save" and "SaveAs"
/*N*/     pImp-> bIsSaving = sal_False;
/*N*/
/*N*/   sal_Bool bToOwnFormat = sal_True;
/*N*/   if ( bToOwnFormat )
/*N*/   {
/*N*/         // If the filter is a "cross export" filter ( f.e. a filter for exporting an impress document from
/*N*/         // a draw document ), the ClassId of the destination storage is different from the ClassId of this
/*N*/         // document. It can be retrieved from the default filter for the desired target format
/*N*/         long nFormat = pNewFile->GetFilter()->GetFormat();
/*N*/       SfxFilterMatcher& rMatcher = SFX_APP()->GetFilterMatcher();
/*N*/       const SfxFilter *pFilt = rMatcher.GetFilter4ClipBoardId( nFormat );
/*N*/       if ( pFilt )
/*N*/       {
/*N*/             if ( pFilt->GetFilterContainer() != pNewFile->GetFilter()->GetFilterContainer() )
/*N*/                 pNewFile->GetStorage()->SetClass( SvFactory::GetServerName( nFormat ), nFormat, pFilt->GetTypeName() );
/*N*/       }
/*N*/   }
/*N*/
/*N*/     // Save the document ( first as temporary file, then transfer to the target URL by committing the medium )
/*N*/     sal_Bool bOk = sal_False;
/*N*/     if ( !pNewFile->GetErrorCode() && SaveTo_Impl( *pNewFile, NULL, sal_True ) )
/*N*/   {
/*N*/       bOk = sal_True;
/*N*/
/*N*/         // restore old BaseURL
/*N*/       ::binfilter::StaticBaseUrl::SetBaseURL( aOldURL );
/*N*/
/*N*/         // transfer a possible error from the medium to the document
/*N*/         SetError( pNewFile->GetErrorCode() );
/*N*/
/*N*/         // notify the document that saving was done successfully
/*N*/         if ( bCopyTo )
/*N*/       {
/*N*/           if ( IsHandsOff() )
/*N*/               bOk = false;
/*N*/       }
/*N*/       else
/*N*/       {
/*N*/           // Muss !!!
/*N*/           if ( bToOwnFormat )
/*N*/               SetFileName( pNewFile->GetPhysicalName() );
/*N*/
/*N*/           bOk = false;
/*N*/       }
/*N*/
/*N*/       if( bOk )
/*N*/       {
/*N*/             if( !bCopyTo )
/*N*/                 SetModified( sal_False );
/*N*/       }
/*N*/       else
/*N*/       {
/*N*/             DBG_ASSERT( !bCopyTo, "Error while reconnecting to medium, can't be handled!");
/*N*/           SetError( pNewFile->GetErrorCode() );
/*N*/
/*N*/             if ( !bCopyTo )
/*N*/             {
/*N*/                 // reconnect to the old medium
/*?*/                 BOOL bRet = false;
/*?*/                 DBG_ASSERT( bRet, "Error in DoSaveCompleted, can't be handled!");
/*?*/                 (void)bRet;
/*N*/             }
/*N*/
/*N*/           DELETEZ( pNewFile );
/*N*/       }
/*N*/
/*N*/       String aPasswd;
/*N*/       if ( IsOwnStorageFormat_Impl( *GetMedium() ) && GetPasswd_Impl( GetMedium()->GetItemSet(), aPasswd ) )
/*?*/           GetMedium()->GetStorage()->SetKey( S2BS( aPasswd ) );   //!!! (pb) needs new implementation
/*N*/   }
/*N*/   else
/*N*/   {
/*?*/       ::binfilter::StaticBaseUrl::SetBaseURL( aOldURL );
/*?*/       SetError( pNewFile->GetErrorCode() );
/*?*/
/*?*/       DELETEZ( pNewFile );
/*N*/   }
/*N*/
/*N*/     if( !bOk )
/*?*/         SetModified( sal_True );
/*N*/
/*N*/   if ( bCopyTo )
/*N*/         DELETEZ( pNewFile );
/*N*/
/*N*/   return bOk;
/*N*/ }

//------------------------------------------------------------------------

/*?*/ sal_Bool SfxObjectShell::LoadFrom( SvStorage *pStor )
/*?*/ {
/*?*/   GetConfigManager();
/*?*/   GetDocInfo().Load(pStor);
/*?*/   return sal_True;
/*?*/ }

//-------------------------------------------------------------------------

/*N*/ sal_Bool SfxObjectShell::IsInformationLost()
/*N*/ {
/*N*/   const SfxFilter *pFilt = GetMedium()->GetFilter();
/*N*/     if ( pFilt == GetFactory().GetFilterContainer()->GetFilter(0) )
/*N*/         return sal_False;
/*N*/     return pFilt && pFilt->IsAlienFormat() && pImp->bDidDangerousSave;
/*N*/ }

/*N*/ sal_Bool SfxObjectShell::LoadOwnFormat( SfxMedium& rMedium )
/*N*/ {
/*N*/   SvStorageRef xStor = rMedium.GetStorage();
/*N*/   if ( xStor.Is() )
/*N*/   {
/*N*/       if ( rMedium.GetFileVersion() )
/*N*/           xStor->SetVersion( rMedium.GetFileVersion() );
/*N*/
/*N*/       // Password
/*N*/         SFX_ITEMSET_ARG( rMedium.GetItemSet(), pPasswdItem, SfxStringItem, SID_PASSWORD, sal_False );
/*N*/         if ( pPasswdItem || ERRCODE_IO_ABORT != CheckPasswd_Impl( this, SFX_APP()->GetPool(), pMedium ) )
/*N*/       {
/*N*/           String aPasswd;
/*N*/           if ( GetPasswd_Impl(pMedium->GetItemSet(), aPasswd) )
/*N*/               xStor->SetKey( S2BS( aPasswd ) ); //!!! (pb) needs new implementation
/*N*/
/*N*/           // load document
                sal_Bool bRet = Load( xStor );
                if ( bRet )
                    GetConfigManager( TRUE );
                return bRet;
/*N*/       }
/*N*/       return sal_False;
/*N*/   }
/*N*/   else
/*N*/       return sal_False;
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
