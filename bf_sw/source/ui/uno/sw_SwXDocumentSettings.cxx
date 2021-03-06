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
#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <osl/mutex.hxx>

#include <SwXDocumentSettings.hxx>
#include <SwXPrintPreviewSettings.hxx>
#include <comphelper/MasterPropertySetInfo.hxx>
#include <com/sun/star/i18n/XForbiddenCharacters.hpp>
#include <com/sun/star/document/PrinterIndependentLayout.hpp>

#include <horiornt.hxx>

#include <rtl/uuid.h>

#include <doc.hxx>
#include <docsh.hxx>
#include <chcmprse.hxx>
#include <fldupde.hxx>
#include <linkenum.hxx>
#include <bf_sfx2/printer.hxx>
#include <bf_sfx2/docinf.hxx>
#include <drawdoc.hxx>
#include <bf_svtools/zforlist.hxx>
#include <unotxdoc.hxx>
#include <cmdid.h>
#include <bf_svx/zoomitem.hxx>
#include <comphelper/TypeGeneration.hxx>
#include <unomod.hxx>
#include <vcl/svapp.hxx>
namespace binfilter {


using namespace comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;

using rtl::OUString;

enum SwDocumentSettingsPropertyHandles
{
    HANDLE_FORBIDDEN_CHARS,
    HANDLE_LINK_UPDATE_MODE,
    HANDLE_FIELD_AUTO_UPDATE,
    HANDLE_CHART_AUTO_UPDATE,
    HANDLE_ADD_PARA_TABLE_SPACING,
    HANDLE_ADD_PARA_TABLE_SPACING_AT_START,
    HANDLE_ALIGN_TAB_STOP_POSITION,
    HANDLE_PRINTER_NAME,
    HANDLE_PRINTER_SETUP,
    HANDLE_IS_KERN_ASIAN_PUNCTUATION,
    HANDLE_CHARACTER_COMPRESSION_TYPE,
    HANDLE_APPLY_USER_DATA,
    HANDLE_SAVE_GLOBAL_DOCUMENT_LINKS,
    HANDLE_CURRENT_DATABASE_DATA_SOURCE,
    HANDLE_CURRENT_DATABASE_COMMAND,
    HANDLE_CURRENT_DATABASE_COMMAND_TYPE,
    HANDLE_SAVE_VERSION_ON_CLOSE,
    HANDLE_IS_GRID_VISIBLE,
    HANDLE_IS_SNAP_TO_GRID,
    HANDLE_IS_SYNCHRONISE_AXES,
    HANDLE_HORIZONTAL_GRID_RESOLUTION,
    HANDLE_HORIZONTAL_GRID_SUBDIVISION,
    HANDLE_VERTICAL_GRID_RESOLUTION,
    HANDLE_VERTICAL_GRID_SUBDIVISION,
    HANDLE_UPDATE_FROM_TEMPLATE,
    HANDLE_PRINTER_INDEPENDENT_LAYOUT,
    HANDLE_IS_LABEL_DOC,
    HANDLE_IS_ADD_FLY_OFFSET
};

MasterPropertySetInfo * lcl_createSettingsInfo()
{
    static PropertyInfo aWriterSettingsInfoMap[] =
    {
        { RTL_CONSTASCII_STRINGPARAM("ForbiddenCharacters"),        HANDLE_FORBIDDEN_CHARS,                 CPPUTYPE_REFFORBCHARS,      0,   0},
        { RTL_CONSTASCII_STRINGPARAM("LinkUpdateMode"),             HANDLE_LINK_UPDATE_MODE,                CPPUTYPE_INT16,             0,   0},
        { RTL_CONSTASCII_STRINGPARAM("FieldAutoUpdate"),            HANDLE_FIELD_AUTO_UPDATE,               CPPUTYPE_BOOLEAN,           0,   0},
        { RTL_CONSTASCII_STRINGPARAM("ChartAutoUpdate"),            HANDLE_CHART_AUTO_UPDATE,               CPPUTYPE_BOOLEAN,           0,   0},
        { RTL_CONSTASCII_STRINGPARAM("AddParaTableSpacing"),        HANDLE_ADD_PARA_TABLE_SPACING,          CPPUTYPE_BOOLEAN,           0,   0},
        { RTL_CONSTASCII_STRINGPARAM("AddParaTableSpacingAtStart"), HANDLE_ADD_PARA_TABLE_SPACING_AT_START, CPPUTYPE_BOOLEAN,           0,   0},
        { RTL_CONSTASCII_STRINGPARAM("AlignTabStopPosition"),       HANDLE_ALIGN_TAB_STOP_POSITION,         CPPUTYPE_BOOLEAN,           0,   0},
        { RTL_CONSTASCII_STRINGPARAM("PrinterName"),                HANDLE_PRINTER_NAME,                    CPPUTYPE_OUSTRING,          0,   0},
        { RTL_CONSTASCII_STRINGPARAM("PrinterSetup"),               HANDLE_PRINTER_SETUP,                   CPPUTYPE_SEQINT8,           0,   0},
        { RTL_CONSTASCII_STRINGPARAM("IsKernAsianPunctuation"),     HANDLE_IS_KERN_ASIAN_PUNCTUATION,       CPPUTYPE_BOOLEAN,           0,   0},
        { RTL_CONSTASCII_STRINGPARAM("CharacterCompressionType"),   HANDLE_CHARACTER_COMPRESSION_TYPE,      CPPUTYPE_INT16,             0,   0},
        { RTL_CONSTASCII_STRINGPARAM("ApplyUserData"),              HANDLE_APPLY_USER_DATA,                 CPPUTYPE_BOOLEAN,           0,   0},
        { RTL_CONSTASCII_STRINGPARAM("SaveGlobalDocumentLinks"),    HANDLE_SAVE_GLOBAL_DOCUMENT_LINKS,      CPPUTYPE_BOOLEAN,           0,   0},
        { RTL_CONSTASCII_STRINGPARAM("CurrentDatabaseDataSource"),  HANDLE_CURRENT_DATABASE_DATA_SOURCE,    CPPUTYPE_OUSTRING,          0,   0},
        { RTL_CONSTASCII_STRINGPARAM("CurrentDatabaseCommand"),     HANDLE_CURRENT_DATABASE_COMMAND,        CPPUTYPE_OUSTRING,          0,   0},
        { RTL_CONSTASCII_STRINGPARAM("CurrentDatabaseCommandType"), HANDLE_CURRENT_DATABASE_COMMAND_TYPE,   CPPUTYPE_INT32,             0,   0},
        { RTL_CONSTASCII_STRINGPARAM("SaveVersionOnClose"),         HANDLE_SAVE_VERSION_ON_CLOSE,           CPPUTYPE_BOOLEAN,           0,   0},
        { RTL_CONSTASCII_STRINGPARAM("UpdateFromTemplate"),         HANDLE_UPDATE_FROM_TEMPLATE,            CPPUTYPE_BOOLEAN,           0,   0},

        { RTL_CONSTASCII_STRINGPARAM("PrinterIndependentLayout"),   HANDLE_PRINTER_INDEPENDENT_LAYOUT,      CPPUTYPE_INT16,             0,   0},
        { RTL_CONSTASCII_STRINGPARAM("IsLabelDocument"),            HANDLE_IS_LABEL_DOC,                    CPPUTYPE_BOOLEAN,           0,   0},
        { RTL_CONSTASCII_STRINGPARAM("AddFrameOffsets"),            HANDLE_IS_ADD_FLY_OFFSET,               CPPUTYPE_BOOLEAN,           0,   0},
        { NULL, 0, 0, CPPUTYPE_UNKNOWN, 0, 0 }
    };
    return new MasterPropertySetInfo ( aWriterSettingsInfoMap );
}

SwXDocumentSettings::SwXDocumentSettings ( SwXTextDocument * pModel )
: MasterPropertySet ( lcl_createSettingsInfo (),
                      &Application::GetSolarMutex () )
, mxModel ( pModel )
, mpModel ( pModel )
, mpDocSh ( NULL )
, mpDoc ( NULL )
, mpPrinter( NULL )
{
    registerSlave ( new SwXPrintSettings ( PRINT_SETTINGS_DOCUMENT, mpModel->GetDocShell()->GetDoc() ) );
    registerSlave ( new SwXPrintPreviewSettings ( mpModel->GetDocShell()->GetDoc() ) );
}

SwXDocumentSettings::~SwXDocumentSettings()
    throw()
{
}

Any SAL_CALL SwXDocumentSettings::queryInterface( const Type& rType )
    throw(RuntimeException)
{
        return ::cppu::queryInterface ( rType,
                                        // OWeakObject interfaces
                                        reinterpret_cast< XInterface* > ( this ),
                                        static_cast< XWeak* > ( this ),
                                        // my own interfaces
                                        static_cast< XPropertySet*  > ( this ),
                                        static_cast< XPropertyState* > ( this ),
                                        static_cast< XMultiPropertySet* > ( this ),
                                        static_cast< XServiceInfo* > ( this ),
                                        static_cast< XTypeProvider* > ( this ) );
}
void SwXDocumentSettings::acquire ()
    throw ()
{
    OWeakObject::acquire();
}
void SwXDocumentSettings::release ()
    throw ()
{
    OWeakObject::release();
}

uno::Sequence< uno::Type > SAL_CALL SwXDocumentSettings::getTypes(  )
    throw (RuntimeException)
{
    SolarMutexGuard aGuard;

    uno::Sequence< uno::Type > aBaseTypes( 5 );
    uno::Type* pBaseTypes = aBaseTypes.getArray();

    // from MasterPropertySet
    pBaseTypes[0] = ::getCppuType((Reference< XPropertySet >*)0);
    pBaseTypes[1] = ::getCppuType((Reference< XPropertyState >*)0);
    pBaseTypes[2] = ::getCppuType((Reference< XMultiPropertySet >*)0);
    //
    pBaseTypes[3] = ::getCppuType((Reference< XServiceInfo >*)0);
    pBaseTypes[4] = ::getCppuType((Reference< XTypeProvider >*)0);

    return aBaseTypes;
}

uno::Sequence< sal_Int8 > SAL_CALL SwXDocumentSettings::getImplementationId(  )
    throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    static Sequence< sal_Int8 > aId( 16 );
    static sal_Bool bInit = sal_False;
    if(!bInit)
    {
        rtl_createUuid( (sal_uInt8 *)(aId.getArray() ), 0, sal_True );
        bInit = sal_True;
    }
    return aId;
}

void SwXDocumentSettings::_preSetValues ()
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException )
{
    mpDocSh = mpModel->GetDocShell();
    mpDoc = mpDocSh->GetDoc();

    if( NULL == mpDoc || NULL == mpDocSh )
        throw UnknownPropertyException();

}
void SwXDocumentSettings::_setSingleValue( const comphelper::PropertyInfo & rInfo, const ::com::sun::star::uno::Any &rValue )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException )
{
    switch( rInfo.mnHandle )
    {
        case HANDLE_FORBIDDEN_CHARS:
            break;
        case HANDLE_LINK_UPDATE_MODE:
        {
            sal_Int16 nMode = 0;
            rValue >>= nMode;
            switch (nMode)
            {
                case NEVER:
                case MANUAL:
                case AUTOMATIC:
                case GLOBALSETTING:
                    break;
                default:
                    throw IllegalArgumentException();
            }
            mpDoc->SetLinkUpdMode(nMode);
        }
        break;
        case HANDLE_FIELD_AUTO_UPDATE:
        {
            sal_Bool bUpdateField = *(sal_Bool*)rValue.getValue();
            sal_Int16 nFlag = mpDoc->GetFldUpdateFlags();
            mpDoc->SetFldUpdateFlags( bUpdateField ?
                    nFlag == AUTOUPD_FIELD_AND_CHARTS ? AUTOUPD_FIELD_AND_CHARTS
                    : AUTOUPD_FIELD_ONLY : AUTOUPD_OFF );
        }
        break;
        case HANDLE_CHART_AUTO_UPDATE:
        {
            sal_Bool bUpdateChart = *(sal_Bool*)rValue.getValue();
            sal_Int16 nFlag = mpDoc->GetFldUpdateFlags();
            mpDoc->SetFldUpdateFlags ( (nFlag == AUTOUPD_FIELD_ONLY || nFlag == AUTOUPD_FIELD_AND_CHARTS )
                    ? bUpdateChart ? AUTOUPD_FIELD_AND_CHARTS : AUTOUPD_FIELD_ONLY : AUTOUPD_OFF );
        }
        break;
        case HANDLE_ADD_PARA_TABLE_SPACING:
        {
            sal_Bool bParaSpace = sal_False;
            rValue >>= bParaSpace;
            mpDoc->SetParaSpaceMax( bParaSpace, mpDoc->IsParaSpaceMaxAtPages());
        }
        break;
        case HANDLE_ADD_PARA_TABLE_SPACING_AT_START:
        {
            sal_Bool bParaSpacePage = sal_False;
            rValue >>= bParaSpacePage;
            mpDoc->SetParaSpaceMax( mpDoc->IsParaSpaceMax(), bParaSpacePage);
        }
        break;
        case HANDLE_ALIGN_TAB_STOP_POSITION:
        {
            sal_Bool bAlignTab = *(sal_Bool*)rValue.getValue();
            mpDoc->SetTabCompat( bAlignTab );
        }
        break;
        case HANDLE_PRINTER_NAME:
        {
            //the printer must be created
            OUString sPrinterName;
            if( rValue >>= sPrinterName  )
            {
                if( !mpPrinter && sPrinterName.getLength() > 0 )
                {
                    SfxPrinter* pPrinter = mpDoc->GetPrt( sal_True );
                    if ( OUString ( pPrinter->GetName()) != sPrinterName )
                    {
                        SfxPrinter *pNewPrinter = new SfxPrinter ( pPrinter->GetOptions().Clone(), sPrinterName );
                        if( pNewPrinter->IsKnown() )
                        {
                            // set printer only once; in _postSetValues
                            mpPrinter = pNewPrinter;
                        }
                        else
                        {
                            delete pNewPrinter;
                        }
                    }
                }
            }
            else
                throw IllegalArgumentException();
        }
        break;
        case HANDLE_PRINTER_SETUP:
        {
            Sequence < sal_Int8 > aSequence;
            if ( rValue >>= aSequence )
            {
                sal_uInt32 nSize = aSequence.getLength();
                if( nSize > 0 )
                {
                    SvMemoryStream aStream (aSequence.getArray(), nSize,
                                            STREAM_READ );
                    aStream.Seek ( STREAM_SEEK_TO_BEGIN );
                    static sal_uInt16 const nRange[] =
                    {
                        FN_PARAM_ADDPRINTER, FN_PARAM_ADDPRINTER,
                        SID_HTML_MODE,  SID_HTML_MODE,
                        SID_PRINTER_NOTFOUND_WARN, SID_PRINTER_NOTFOUND_WARN,
                        SID_PRINTER_CHANGESTODOC, SID_PRINTER_CHANGESTODOC,
                        0
                    };
                    SfxItemSet *pItemSet = new SfxItemSet( mpDoc->GetAttrPool(), nRange );
                    SfxPrinter *pPrinter = SfxPrinter::Create ( aStream, pItemSet );

                    // set printer only once; in _postSetValues
                    delete mpPrinter;
                    mpPrinter = pPrinter;
                }
            }
            else
                throw IllegalArgumentException();
        }
        break;
        case HANDLE_IS_KERN_ASIAN_PUNCTUATION:
        {
            sal_Bool bIsKern = *(sal_Bool*)(rValue).getValue();
            mpDoc->SetKernAsianPunctuation( bIsKern );
        }
        break;
        case HANDLE_CHARACTER_COMPRESSION_TYPE:
        {
            sal_Int16 nMode = 0;
            rValue >>= nMode;
            switch (nMode)
            {
                case CHARCOMPRESS_NONE:
                case CHARCOMPRESS_PUNCTUATION:
                case CHARCOMPRESS_PUNCTUATION_KANA:
                    break;
                default:
                    throw IllegalArgumentException();
            }
            mpDoc->SetCharCompressType(static_cast < SwCharCompressType > (nMode) );
        }
        break;
        case HANDLE_APPLY_USER_DATA:
        {
            SfxDocumentInfo& rLclInfo = mpDocSh->GetDocInfo();
            sal_Bool bUseUserData = *(sal_Bool*)rValue.getValue();
            rLclInfo.SetUseUserData(bUseUserData);
        }
        break;
        case HANDLE_SAVE_GLOBAL_DOCUMENT_LINKS:
        {
            sal_Bool bSaveGlobal = *(sal_Bool*)rValue.getValue();
            mpDoc->SetGlblDocSaveLinks( bSaveGlobal );
        }
        break;
        case HANDLE_CURRENT_DATABASE_DATA_SOURCE:
        {
            SwDBData aData = mpDoc->GetDBData();
            if ( rValue >>= aData.sDataSource )
                mpDoc->ChgDBData( aData );
        }
        break;
        case HANDLE_CURRENT_DATABASE_COMMAND:
        {
            SwDBData aData = mpDoc->GetDBData();
            if ( rValue >>= aData.sCommand )
                mpDoc->ChgDBData( aData );
        }
        break;
        case HANDLE_CURRENT_DATABASE_COMMAND_TYPE:
        {
            SwDBData aData = mpDoc->GetDBData();
            if ( rValue >>= aData.nCommandType )
                mpDoc->ChgDBData( aData );
        }
        break;
        case HANDLE_SAVE_VERSION_ON_CLOSE:
        {
            SfxDocumentInfo& rLclInfo = mpDocSh->GetDocInfo();
            sal_Bool bSaveVersion = *(sal_Bool*)rValue.getValue();
            rLclInfo.SetSaveVersionOnClose ( bSaveVersion );
        }
        break;
        case HANDLE_UPDATE_FROM_TEMPLATE:
        {
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDocSh->GetDocInfo().SetQueryLoadTemplate( bTmp );
        }
        break;
        case HANDLE_PRINTER_INDEPENDENT_LAYOUT:
        {
            sal_Int16 nTmp = 0;
            rValue >>= nTmp;
            mpDoc->SetUseVirtualDevice( nTmp != document::PrinterIndependentLayout::DISABLED  );
        }
        break;
        case HANDLE_IS_LABEL_DOC :
        {
            sal_Bool bSet = sal_False;
            if(!(rValue >>= bSet))
                throw IllegalArgumentException();
            mpDoc->SetLabelDoc(bSet);
        }
        break;
        case HANDLE_IS_ADD_FLY_OFFSET:
        {
            sal_Bool bTmp = *(sal_Bool*)rValue.getValue();
            mpDoc->SetAddFlyOffsets( bTmp );
        }
        break;
        default:
            throw UnknownPropertyException();
    }
}

void SwXDocumentSettings::_postSetValues ()
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException )
{
    // set printer only once, namely here!
    if( mpPrinter != NULL )
        mpDoc->SetPrt( mpPrinter, sal_True );

    mpPrinter = 0;
    mpDocSh = 0;
    mpDoc = 0;
}

void SwXDocumentSettings::_preGetValues ()
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException )
{
    mpDocSh = mpModel->GetDocShell();
    mpDoc = mpDocSh->GetDoc();
    if( NULL == mpDoc || NULL == mpDocSh )
        throw UnknownPropertyException();
}

void SwXDocumentSettings::_getSingleValue( const comphelper::PropertyInfo & rInfo, ::com::sun::star::uno::Any & rValue )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException )
{
    switch( rInfo.mnHandle )
    {
        case HANDLE_FORBIDDEN_CHARS:
        {
            Reference<XForbiddenCharacters> xRet(*mpModel->GetPropertyHelper(), UNO_QUERY);
            rValue <<= xRet;
        }
        break;
        case HANDLE_LINK_UPDATE_MODE:
        {
            rValue <<= static_cast < sal_Int16 > ( mpDoc->GetLinkUpdMode() );
        }
        break;
        case HANDLE_FIELD_AUTO_UPDATE:
        {
            sal_uInt16 nFlags = mpDoc->GetFldUpdateFlags();
            BOOL bFieldUpd = (nFlags == AUTOUPD_FIELD_ONLY || nFlags == AUTOUPD_FIELD_AND_CHARTS );
            rValue.setValue(&bFieldUpd, ::getBooleanCppuType());
        }
        break;
        case HANDLE_CHART_AUTO_UPDATE:
        {
            sal_uInt16 nFlags = mpDoc->GetFldUpdateFlags();
            BOOL bChartUpd = nFlags == AUTOUPD_FIELD_AND_CHARTS;
            rValue.setValue(&bChartUpd, ::getBooleanCppuType());
        }
        break;
        case HANDLE_ADD_PARA_TABLE_SPACING:
        {
            sal_Bool bParaSpace = mpDoc->IsParaSpaceMax();
            rValue.setValue(&bParaSpace, ::getBooleanCppuType());
        }
        break;
        case HANDLE_ADD_PARA_TABLE_SPACING_AT_START:
        {
            sal_Bool bParaSpace = mpDoc->IsParaSpaceMaxAtPages();
            rValue.setValue(&bParaSpace, ::getBooleanCppuType());
        }
        break;
        case HANDLE_ALIGN_TAB_STOP_POSITION:
        {
            sal_Bool bAlignTab = mpDoc->IsTabCompat();
            rValue.setValue(&bAlignTab, ::getBooleanCppuType());
        }
        break;
        case HANDLE_PRINTER_NAME:
        {
            SfxPrinter *pPrinter = mpDoc->GetPrt ( sal_False );
            rValue <<= pPrinter ? OUString ( pPrinter->GetName()) : OUString();
        }
        break;
        case HANDLE_PRINTER_SETUP:
        {
            SfxPrinter *pPrinter = mpDocSh->GetDoc()->GetPrt ( sal_False );
            if (pPrinter)
            {
                SvMemoryStream aStream;
                pPrinter->Store( aStream );
                aStream.Seek ( STREAM_SEEK_TO_END );
                sal_uInt32 nSize = aStream.Tell();
                aStream.Seek ( STREAM_SEEK_TO_BEGIN );
                Sequence < sal_Int8 > aSequence( nSize );
                aStream.Read ( aSequence.getArray(), nSize );
                rValue <<= aSequence;
            }
            else
            {
                Sequence < sal_Int8 > aSequence ( 0 );
                rValue <<= aSequence;
            }
        }
        break;
        case HANDLE_IS_KERN_ASIAN_PUNCTUATION:
        {
            sal_Bool bParaSpace = mpDoc->IsKernAsianPunctuation();
            rValue.setValue(&bParaSpace, ::getBooleanCppuType());
        }
        break;
        case HANDLE_APPLY_USER_DATA:
        {
            SfxDocumentInfo &rLclInfo = mpDocSh->GetDocInfo();
            sal_Bool bUseUserInfo = rLclInfo.IsUseUserData();
            rValue.setValue(&bUseUserInfo, ::getBooleanCppuType());
        }
        break;
        case HANDLE_CHARACTER_COMPRESSION_TYPE:
        {
            rValue <<= static_cast < sal_Int16 > (mpDoc->GetCharCompressType());
        }
        break;
        case HANDLE_SAVE_GLOBAL_DOCUMENT_LINKS:
        {
            sal_Bool bSaveGlobal = mpDoc->IsGlblDocSaveLinks();
            rValue.setValue(&bSaveGlobal, ::getBooleanCppuType());
        }
        break;
        case HANDLE_CURRENT_DATABASE_DATA_SOURCE:
        {
            const SwDBData& rData = mpDoc->GetDBDesc();
            rValue <<= rData.sDataSource;
        }
        break;
        case HANDLE_CURRENT_DATABASE_COMMAND:
        {
            const SwDBData& rData = mpDoc->GetDBDesc();
            rValue <<= rData.sCommand;
        }
        break;
        case HANDLE_CURRENT_DATABASE_COMMAND_TYPE:
        {
            const SwDBData& rData = mpDoc->GetDBDesc();
            rValue <<= rData.nCommandType;
        }
        break;
        case HANDLE_SAVE_VERSION_ON_CLOSE:
        {
            SfxDocumentInfo& rLclInfo = mpDocSh->GetDocInfo();
            sal_Bool bSaveVersion = rLclInfo.IsSaveVersionOnClose();
            rValue.setValue(&bSaveVersion, ::getBooleanCppuType());
        }
        break;
        case HANDLE_UPDATE_FROM_TEMPLATE:
        {
            sal_Bool bTmp = mpDocSh->GetDocInfo().IsQueryLoadTemplate();
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
        }
        break;
        case HANDLE_PRINTER_INDEPENDENT_LAYOUT:
        {
            sal_Int16 nTmp = mpDoc->IsUseVirtualDevice()
                ? document::PrinterIndependentLayout::ENABLED
                : document::PrinterIndependentLayout::DISABLED;
            rValue <<= nTmp;
        }
        break;
        case HANDLE_IS_LABEL_DOC:
        {
            sal_Bool bLabel = mpDoc->IsLabelDoc();
            rValue <<= bLabel;
        }
        break;
        case HANDLE_IS_ADD_FLY_OFFSET:
        {
            sal_Bool bTmp = mpDoc->IsAddFlyOffsets();
            rValue.setValue( &bTmp, ::getBooleanCppuType() );
        }
        break;
        default:
            throw UnknownPropertyException();
    }
}

void SwXDocumentSettings::_postGetValues ()
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException )
{
    mpDocSh = 0;
    mpDoc = 0;
}

// XServiceInfo
OUString SAL_CALL SwXDocumentSettings::getImplementationName(  )
    throw(RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Writer.DocumentSettings"));
}

sal_Bool SAL_CALL SwXDocumentSettings::supportsService( const OUString& ServiceName )
    throw(RuntimeException)
{
    const Sequence< OUString > aSeq( getSupportedServiceNames() );
    sal_Int32 nCount = aSeq.getLength();
    const OUString* pServices = aSeq.getConstArray();
    while( nCount-- )
    {
        if( *pServices++ == ServiceName )
            return sal_True;
    }
    return sal_True;
}

Sequence< OUString > SAL_CALL SwXDocumentSettings::getSupportedServiceNames(  )
    throw(RuntimeException)
{
    Sequence< OUString > aSeq( 3 );
    aSeq[0] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.Settings") );
    aSeq[1] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Writer.Settings") );
    aSeq[2] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.DocumentSettings") );
    return aSeq;
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
