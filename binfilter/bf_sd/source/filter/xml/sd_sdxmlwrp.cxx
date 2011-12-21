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

#include <rtl/logfile.hxx>

#include <com/sun/star/xml/sax/SAXParseException.hdl>
#include <bf_sfx2/docfile.hxx>
#include "drawdoc.hxx"
#include <unotools/streamwrap.hxx>
#include <bf_svx/xmlgrhlp.hxx>

#include "bf_sd/docshell.hxx"
#include "sdxmlwrp.hxx"
#include "strmname.h"

#include <bf_svx/xmleohlp.hxx>

#include <bf_sfx2/appuno.hxx>

#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>

#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>

#include <comphelper/genericpropertyset.hxx>

#include <bf_svtools/saveopt.hxx>

// #80365# include necessary for XML progress bar at load time
#include <bf_svtools/itemset.hxx>

#include <bf_svtools/sfxecode.hxx>

#include "sderror.hxx"
#include "sdresid.hxx"
#include "glob.hrc"

#include <legacysmgr/legacy_binfilters_smgr.hxx>
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::document;
using namespace comphelper;

using rtl::OUString;

#define SD_XML_READERROR 1234

//////////////////////////////////////////////////////////////////////////////

#ifndef SEQTYPE
 #if defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)
  #define SEQTYPE(x) (new ::com::sun::star::uno::Type( x ))
 #else
  #define SEQTYPE(x) &(x)
 #endif
#endif

#define MAP_LEN(x) x, sizeof(x) - 1

#define XML_STRING(i, x) sal_Char const i[sizeof(x)] = x

XML_STRING( sXML_metaStreamName, "meta.xml");
XML_STRING( sXML_styleStreamName, "styles.xml" );
XML_STRING( sXML_contentStreamName, "content.xml" );
XML_STRING( sXML_oldContentStreamName, "Content.xml" );
XML_STRING( sXML_settingsStreamName, "settings.xml" );

XML_STRING( sXML_export_impress_meta_service, "com.sun.star.comp.Impress.XMLMetaExporter" );
XML_STRING( sXML_export_impress_styles_service, "com.sun.star.comp.Impress.XMLStylesExporter" );
XML_STRING( sXML_export_impress_content_service, "com.sun.star.comp.Impress.XMLContentExporter" );
XML_STRING( sXML_export_impress_settings_service, "com.sun.star.comp.Impress.XMLSettingsExporter" );

XML_STRING( sXML_export_draw_meta_service, "com.sun.star.comp.Draw.XMLMetaExporter" );
XML_STRING( sXML_export_draw_styles_service, "com.sun.star.comp.Draw.XMLStylesExporter" );
XML_STRING( sXML_export_draw_content_service, "com.sun.star.comp.Draw.XMLContentExporter" );
XML_STRING( sXML_export_draw_settings_service, "com.sun.star.comp.Draw.XMLSettingsExporter" );

XML_STRING( sXML_import_impress_service, "com.sun.star.comp.Impress.XMLImporter" );
XML_STRING( sXML_import_impress_meta_service, "com.sun.star.comp.Impress.XMLMetaImporter" );
XML_STRING( sXML_import_impress_styles_service, "com.sun.star.comp.Impress.XMLStylesImporter" );
XML_STRING( sXML_import_impress_content_service, "com.sun.star.comp.Impress.XMLContentImporter" );
XML_STRING( sXML_import_impress_settings_service, "com.sun.star.comp.Impress.XMLSettingsImporter" );

XML_STRING( sXML_import_draw_service, "com.sun.star.comp.Draw.XMLImporter" );
XML_STRING( sXML_import_draw_meta_service, "com.sun.star.comp.Draw.XMLMetaImporter" );
XML_STRING( sXML_import_draw_styles_service, "com.sun.star.comp.Draw.XMLStylesImporter" );
XML_STRING( sXML_import_draw_content_service, "com.sun.star.comp.Draw.XMLContentImporter" );
XML_STRING( sXML_import_draw_settings_service, "com.sun.star.comp.Draw.XMLSettingsImporter" );

struct XML_SERVICEMAP
{
    const sal_Char* mpService;
    const sal_Char* mpStream;
    sal_Bool mbPlain;
};


// ----------------
// - SdXMLWrapper -
// ----------------

SdXMLFilter::SdXMLFilter( SfxMedium& rMedium, SdDrawDocShell& rDocShell, sal_Bool bShowProgress, SdXMLFilterMode eFilterMode ) :
    SdFilter( rMedium, rDocShell, bShowProgress ), meFilterMode( eFilterMode )
{
}

SdXMLFilter::~SdXMLFilter()
{
}

// -----------------------------------------------------------------------------

sal_Bool SdXMLFilter::Import()
{
    OSL_ASSERT("XML import removed");
  sal_uInt32  nRet = 0;
  return nRet == 0;
}

// -----------------------------------------------------------------------------

sal_Bool SdXMLFilter::Export()
{
#ifdef TIMELOG
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sd", "cl93746", "SdXMLFilter::Export" );
    ByteString aFile( mrMedium.GetName(), RTL_TEXTENCODING_ASCII_US );
    RTL_LOGFILE_CONTEXT_TRACE1( aLog, "exporting %s", aFile.GetBuffer() );
#endif

    SvXMLEmbeddedObjectHelper*  pObjectHelper = NULL;
    SvXMLGraphicHelper*         pGraphicHelper = NULL;
    sal_Bool                    bDocRet = FALSE;

    try
    {
        if( !mxModel.is() )
        {
            OSL_FAIL("Got NO Model in XMLExport");
            return FALSE;
        }

        uno::Reference< lang::XServiceInfo > xServiceInfo( mxModel, uno::UNO_QUERY );

        if( !xServiceInfo.is() || !xServiceInfo->supportsService( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.GenericDrawingDocument" ) ) ) )
        {
            OSL_FAIL( "Model is no DrawingDocument in XMLExport" );
            return FALSE;
        }

        uno::Reference< lang::XMultiServiceFactory> xServiceFactory( ::legacy_binfilters::getLegacyProcessServiceFactory() );

        if( !xServiceFactory.is() )
        {
            OSL_FAIL( "got no service manager" );
            return FALSE;
        }

        uno::Reference< uno::XInterface > xWriter( xServiceFactory->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.Writer" ) ) ) );

        if( !xWriter.is() )
        {
            OSL_FAIL( "com.sun.star.xml.sax.Writer service missing" );
            return FALSE;
        }

        uno::Reference<xml::sax::XDocumentHandler>  xHandler( xWriter, uno::UNO_QUERY );

        /** property map for export info set */
        PropertyMapEntry aExportInfoMap[] =
        {
            // #82003#
            { MAP_LEN( "ProgressRange" ),   0, &::getCppuType((const sal_Int32*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
            { MAP_LEN( "ProgressMax" ),     0, &::getCppuType((const sal_Int32*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
            { MAP_LEN( "ProgressCurrent" ), 0, &::getCppuType((const sal_Int32*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
            { MAP_LEN( "UsePrettyPrinting"),0, &::getBooleanCppuType(),             ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},

            { MAP_LEN( "PageLayoutNames" ), 0, SEQTYPE(::getCppuType((const OUString*)0)),  ::com::sun::star::beans::PropertyAttribute::MAYBEVOID,     0},
            { NULL, 0, 0, NULL, 0, 0 }
        };

        uno::Reference< beans::XPropertySet > xInfoSet( GenericPropertySet_CreateInstance( new PropertySetInfo( aExportInfoMap ) ) );


        SvtSaveOptions aSaveOpt;
        OUString sUsePrettyPrinting(RTL_CONSTASCII_USTRINGPARAM("UsePrettyPrinting"));
        sal_Bool bUsePrettyPrinting( aSaveOpt.IsPrettyPrinting() );
        xInfoSet->setPropertyValue( sUsePrettyPrinting, makeAny( bUsePrettyPrinting ) );

        SvStorage* pStorage = mrMedium.GetOutputStorage( sal_True );

        // initialize descriptor
        uno::Sequence< beans::PropertyValue > aDescriptor( 1 );
        beans::PropertyValue* pProps = aDescriptor.getArray();

        pProps[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "FileName" ) );
        pProps[0].Value <<= OUString( mrMedium.GetName() );

        {
            uno::Reference< document::XEmbeddedObjectResolver > xObjectResolver;
            uno::Reference< document::XGraphicObjectResolver >  xGrfResolver;

            // create helper for graphic and ole export if we have a storage
            if( pStorage )
            {
                SvPersist *pPersist = mrDocShell.GetDoc()->GetPersist();
                if( pPersist )
                {
                    pObjectHelper = SvXMLEmbeddedObjectHelper::Create( *pStorage, *pPersist, EMBEDDEDOBJECTHELPER_MODE_WRITE, sal_False );
                    xObjectResolver = pObjectHelper;
                }

                pGraphicHelper = SvXMLGraphicHelper::Create( *pStorage, GRAPHICHELPER_MODE_WRITE, FALSE );
                xGrfResolver = pGraphicHelper;
            }

            // #82003#
            if(mbShowProgress)
            {
                CreateStatusIndicator();
                if(mxStatusIndicator.is())
                {
                    sal_Int32 nProgressRange(1000000);
                    sal_Int32 nProgressCurrent(0);
                    OUString aMsg = String( SdResId( STR_SAVE_DOC ) );
                    mxStatusIndicator->start(aMsg, nProgressRange);

                    // set ProgressRange
                    uno::Any aProgRange;
                    aProgRange <<= nProgressRange;
                    xInfoSet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ProgressRange")), aProgRange);

                    // set ProgressCurrent
                    uno::Any aProgCurrent;
                    aProgCurrent <<= nProgressCurrent;
                    xInfoSet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ProgressCurrent")), aProgCurrent);
                }
            }

            uno::Reference< lang::XComponent > xComponent( mxModel, uno::UNO_QUERY );

            XML_SERVICEMAP aServices[5]; sal_uInt16 i = 0;
            aServices[i  ].mpService = IsDraw() ? sXML_export_draw_styles_service : sXML_export_impress_styles_service;
            aServices[i  ].mpStream  = sXML_styleStreamName;
            aServices[i++].mbPlain = sal_False;

            aServices[i  ].mpService = IsDraw() ? sXML_export_draw_content_service : sXML_export_impress_content_service;
            aServices[i  ].mpStream  = sXML_contentStreamName;
            aServices[i++].mbPlain = sal_False;

            aServices[i  ].mpService = IsDraw() ? sXML_export_draw_settings_service : sXML_export_impress_settings_service;
            aServices[i  ].mpStream  = sXML_settingsStreamName;
            aServices[i++].mbPlain = sal_False;

            if( mrDocShell.GetCreateMode() != SFX_CREATE_MODE_EMBEDDED )
            {
                aServices[i  ].mpService = IsDraw() ? sXML_export_draw_meta_service : sXML_export_impress_meta_service;
                aServices[i  ].mpStream  = sXML_metaStreamName;
                aServices[i++].mbPlain = sal_True;
            };

            aServices[i].mpService = NULL;
            aServices[i].mpStream  = NULL;

            XML_SERVICEMAP* pServices = aServices;

            // doc export
            do
            {
                RTL_LOGFILE_CONTEXT_TRACE1( aLog, "exporting substream %s", pServices->mpStream );

                uno::Reference<io::XOutputStream> xDocOut;
                SvStorageStreamRef xDocStream;

                if( pStorage )
                {
                    const OUString sDocName( OUString::createFromAscii( pServices->mpStream ) );
                    xDocStream = pStorage->OpenStream( sDocName, STREAM_WRITE | STREAM_SHARE_DENYWRITE | STREAM_TRUNC  );
                    DBG_ASSERT(xDocStream.Is(), "Can't create output stream in package!");
                    if( !xDocStream.Is() )
                        return sal_False;

                    xDocStream->SetVersion( pStorage->GetVersion() );
//                  xDocStream->SetKey( pStorage->GetKey() );
                    xDocStream->SetBufferSize( 16*1024 );
                    xDocOut = new ::utl::OOutputStreamWrapper( *xDocStream );

                    uno::Any aAny; aAny <<= OUString( RTL_CONSTASCII_USTRINGPARAM("text/xml") );
                    xDocStream->SetProperty(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MediaType")), aAny);

                    if( pServices->mbPlain )
                    {
                        xDocStream->SetProperty( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Compressed") ), uno::makeAny( (sal_Bool) sal_False ) );
                    }
                    else
                    {
                        xDocStream->SetProperty( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Encrypted") ), uno::makeAny( (sal_Bool)sal_True ) );
                    }

                }

                uno::Reference< io::XActiveDataSource > xDocSrc( xWriter, uno::UNO_QUERY );
                xDocSrc->setOutputStream( xDocOut );

                uno::Sequence< uno::Any > aArgs( 2 + ( mxStatusIndicator.is() ? 1 : 0 ) + ( xGrfResolver.is() ? 1 : 0 ) + ( xObjectResolver.is() ? 1 : 0 ) );
                uno::Any* pArgs = aArgs.getArray();
                if( xGrfResolver.is() )         *pArgs++ <<= xGrfResolver;
                if( xObjectResolver.is() )      *pArgs++ <<= xObjectResolver;
                if( mxStatusIndicator.is() )    *pArgs++ <<= mxStatusIndicator;

                *pArgs++ <<= xInfoSet;
                *pArgs   <<= xHandler;

                uno::Reference< document::XFilter > xFilter( xServiceFactory->createInstanceWithArguments( OUString::createFromAscii( pServices->mpService ), aArgs ), uno::UNO_QUERY );
                if( xFilter.is() )
                {
                    uno::Reference< document::XExporter > xExporter( xFilter, uno::UNO_QUERY );
                    if( xExporter.is() )
                    {
                        xExporter->setSourceDocument( xComponent );

                        bDocRet = xFilter->filter( aDescriptor );

                        if(bDocRet && xDocStream.Is())
                            xDocStream->Commit();
                    }
                }

                pServices++;
            }
            while( bDocRet && pServices->mpService );

            // #82003#
            if(mbShowProgress)
            {
                if(mxStatusIndicator.is())
                    mxStatusIndicator->end();
            }
        }
    }
    catch(uno::Exception e)
    {
#if OSL_DEBUG_LEVEL > 1
        ByteString aError( "uno Exception caught while exporting:\n" );
        aError += ByteString( String( e.Message), RTL_TEXTENCODING_ASCII_US );
        OSL_FAIL( aError.GetBuffer() );
#endif
        bDocRet = sal_False;
    }

    if( pGraphicHelper )
        SvXMLGraphicHelper::Destroy( pGraphicHelper );

    if( pObjectHelper )
        SvXMLEmbeddedObjectHelper::Destroy( pObjectHelper );


    return bDocRet;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
