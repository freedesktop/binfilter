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

#include "bindetect.hxx"

#include <bf_svtools/bf_solar.h>

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XContainerQuery.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/NamedValue.hpp>

#include <tools/urlobj.hxx>
#include <bf_svtools/pathoptions.hxx>
#include <rtl/ustring.h>
#include <osl/mutex.hxx>
#include <sot/storage.hxx>
#include <sot/exchange.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/svapp.hxx>
#include <osl/module.hxx>
#include <osl/file.hxx>

using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::uno;

namespace binfilter {

struct TypesInfo
{
    const char* Type;
    sal_Int32   Length;
};

// the binfilter module is responsible for
// - see /filter/source/config/fragments/filters/
static const sal_uInt8 nFileTypeCount = 34;
static const TypesInfo aFileTypeList[ nFileTypeCount ] =
{
    {RTL_CONSTASCII_STRINGPARAM("writer_StarWriter_10")},
    {RTL_CONSTASCII_STRINGPARAM("writer_StarWriter_20")},
    {RTL_CONSTASCII_STRINGPARAM("writer_StarWriter_DOS")},
    {RTL_CONSTASCII_STRINGPARAM("writer_Lotus_1_2_3_10_DOS_StarWriter")},
    {RTL_CONSTASCII_STRINGPARAM("writer_Lotus_1_2_3_10_WIN_StarWriter")},
    {RTL_CONSTASCII_STRINGPARAM("calc_StarCalc_30")},
    {RTL_CONSTASCII_STRINGPARAM("calc_StarCalc_30_VorlageTemplate")},
    {RTL_CONSTASCII_STRINGPARAM("calc_StarCalc_40")},
    {RTL_CONSTASCII_STRINGPARAM("calc_StarCalc_40_VorlageTemplate")},
    {RTL_CONSTASCII_STRINGPARAM("calc_StarCalc_50")},
    {RTL_CONSTASCII_STRINGPARAM("calc_StarCalc_50_VorlageTemplate")},
    {RTL_CONSTASCII_STRINGPARAM("chart_StarChart_30")},
    {RTL_CONSTASCII_STRINGPARAM("chart_StarChart_40")},
    {RTL_CONSTASCII_STRINGPARAM("chart_StarChart_50")},
    {RTL_CONSTASCII_STRINGPARAM("draw_StarDraw_30")},
    {RTL_CONSTASCII_STRINGPARAM("draw_StarDraw_30_Vorlage")},
    {RTL_CONSTASCII_STRINGPARAM("draw_StarDraw_50")},
    {RTL_CONSTASCII_STRINGPARAM("draw_StarDraw_50_Vorlage")},
    {RTL_CONSTASCII_STRINGPARAM("impress_StarImpress_40")},
    {RTL_CONSTASCII_STRINGPARAM("impress_StarImpress_40_Vorlage")},
    {RTL_CONSTASCII_STRINGPARAM("impress_StarImpress_50")},
    {RTL_CONSTASCII_STRINGPARAM("impress_StarImpress_50_Vorlage")},
    {RTL_CONSTASCII_STRINGPARAM("impress_StarImpress_50_packed")},
    {RTL_CONSTASCII_STRINGPARAM("math_StarMath_30")},
    {RTL_CONSTASCII_STRINGPARAM("math_StarMath_40")},
    {RTL_CONSTASCII_STRINGPARAM("math_StarMath_50")},
    {RTL_CONSTASCII_STRINGPARAM("writer_StarWriter_30")},
    {RTL_CONSTASCII_STRINGPARAM("writer_StarWriter_30_VorlageTemplate")},
    {RTL_CONSTASCII_STRINGPARAM("writer_StarWriter_40")},
    {RTL_CONSTASCII_STRINGPARAM("writer_StarWriter_40_VorlageTemplate")},
    {RTL_CONSTASCII_STRINGPARAM("writer_globaldocument_StarWriter_40GlobalDocument")},
    {RTL_CONSTASCII_STRINGPARAM("writer_StarWriter_50")},
    {RTL_CONSTASCII_STRINGPARAM("writer_StarWriter_50_VorlageTemplate")},
    {RTL_CONSTASCII_STRINGPARAM("writer_globaldocument_StarWriter_50GlobalDocument")}
};

bool checkFormat( SvStream* pStream, ::rtl::OUString& rTypeName, bool bCheckAll )
{
    sal_Char aBuffer[4098];
    if( pStream && !pStream->GetError() )
    {
        const ULONG nMaxRead = sizeof(aBuffer) - 2;
        pStream->Seek(STREAM_SEEK_TO_BEGIN);
        ULONG nBytesRead = pStream->Read(aBuffer, nMaxRead);
        pStream->Seek(STREAM_SEEK_TO_BEGIN);
        if (nBytesRead <= nMaxRead)
        {
            aBuffer[nBytesRead] = '\0';
            aBuffer[nBytesRead+1] = '\0';
            if (nBytesRead & 0x00000001)
                aBuffer[nBytesRead+2] = '\0';
        }
    }
    else
        return false;
    // clear type name in case nothing gets detected
    ::rtl::OUString aName( rTypeName );
    rTypeName = ::rtl::OUString();

    const sal_Char FILTER_SWG[]    = "SWG";
    if ( bCheckAll || aName.equalsAsciiL( aFileTypeList[0].Type, aFileTypeList[0].Length ) )
    {
        // StarWriter 1.0
        if ( 0 == strncmp( FILTER_SWG, aBuffer, 3 ) &&
                '1' == *(aBuffer + 3) )
        {
            rTypeName = ::rtl::OUString::createFromAscii( aFileTypeList[0].Type );
        }
    }
    if ( bCheckAll || aName.equalsAsciiL( aFileTypeList[1].Type, aFileTypeList[1].Length ) )
    {
        // StarWriter 2.0
        if( 0 == strncmp( FILTER_SWG, aBuffer, 3 ) &&
                '1' != *(aBuffer + 3) )
        {
            rTypeName = ::rtl::OUString::createFromAscii( aFileTypeList[1].Type );
        }
    }
    if ( bCheckAll || aName.equalsAsciiL( aFileTypeList[2].Type, aFileTypeList[2].Length ) )
    {
        // StarWriter DOS
        sal_Char const sSw6_FormatStt[] =         ".\\\\\\ WRITER ";
        sal_Char const sSw6_FormatEnd[] =         " \\\\\\";


        if ( 0 == strncmp( sSw6_FormatStt, aBuffer, 12 ) &&
               0 == strncmp( sSw6_FormatEnd, aBuffer + 12 + 1, 4 ))
        {
            rTypeName = ::rtl::OUString::createFromAscii( aFileTypeList[2].Type );
        }
    }
    if ( bCheckAll || aName.equalsAsciiL( aFileTypeList[3].Type, aFileTypeList[3].Length ) )
    {
        // writer_Lotus_1_2_3_10_DOS_StarWriter
        if ( 0 == aBuffer[0] && 0 == aBuffer[1] &&
                2 == aBuffer[2] && 0 == aBuffer[3] &&
                ( 4 == aBuffer[4] || 6 == aBuffer[4] ) && 4 == aBuffer[5])
        {
            rTypeName = ::rtl::OUString::createFromAscii( aFileTypeList[3].Type );
        }
    }
    if ( bCheckAll || aName.equalsAsciiL( aFileTypeList[4].Type, aFileTypeList[4].Length ) )
    {
        // writer_Lotus_1_2_3_10_WIN_StarWriter
        if ( 0 == aBuffer[0] && 0 == aBuffer[1] &&
                2 == aBuffer[2] && 0 == aBuffer[3] &&
                ( 4 == aBuffer[4] || 6 == aBuffer[4] ) && 4 == aBuffer[5])
        {
            rTypeName = ::rtl::OUString::createFromAscii( aFileTypeList[4].Type );
        }
    }
    return (rTypeName.getLength() !=  0);
}

BinFilterDetect::BinFilterDetect( const Reference < ::com::sun::star::lang::XMultiServiceFactory >& xFactory )
    : m_xFact( xFactory )
{
}

BinFilterDetect::~BinFilterDetect()
{
}

::rtl::OUString SAL_CALL BinFilterDetect::detect( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lDescriptor ) throw( ::com::sun::star::uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    // create MediaDescriptor
    comphelper::MediaDescriptor aMedium( lDescriptor );

    // extract preselected type or filter
    ::rtl::OUString aTypeName, aPreselectedFilterName, aFilterName, aFileName;
    aTypeName   = aMedium.getUnpackedValueOrDefault(comphelper::MediaDescriptor::PROP_TYPENAME()  , ::rtl::OUString() );
    aFilterName = aMedium.getUnpackedValueOrDefault(comphelper::MediaDescriptor::PROP_FILTERNAME(), ::rtl::OUString() );
    aFileName   = aMedium.getUnpackedValueOrDefault(comphelper::MediaDescriptor::PROP_URL()       , ::rtl::OUString() );

    // open stream
    aMedium.addInputStream();
    Reference < com::sun::star::io::XInputStream > xInputStream = aMedium.getUnpackedValueOrDefault(comphelper::MediaDescriptor::PROP_INPUTSTREAM(), Reference < com::sun::star::io::XInputStream >() );

    // no detection without stream.
    // URL was used already to open stream ... so no further chance to get one.
    if ( ! xInputStream.is() )
        return ::rtl::OUString();

    try
    {
        // get type for preselected filter (will overwrite a possible TypeName)
        Reference< ::com::sun::star::container::XNameAccess> xFilters ( m_xFact->createInstance( DEFINE_CONST_UNICODE( "com.sun.star.document.FilterFactory" ) ), UNO_QUERY_THROW );
        if ( aPreselectedFilterName.getLength() )
        {
            try
            {
                ::comphelper::SequenceAsHashMap( xFilters->getByName( aPreselectedFilterName ) )[DEFINE_CONST_UNICODE("Type")] >>= aTypeName;
            }
            catch ( com::sun::star::uno::Exception& )
            {
                // filter name might be wrong
                aPreselectedFilterName = ::rtl::OUString();
            }
        }

        // get ClipboardFormat from Storage
        SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( xInputStream );
        SotStorageRef aStor = new SotStorage( pStream, FALSE );
        if ( !aStor->GetError() )
        {
            // check for the OLE-storage based formats
            sal_Int32 nClipId = aStor->GetFormat();
            ::rtl::OUString aFormatName = SotExchange::GetFormatName( nClipId );

            // check against preselected type
            ::rtl::OUString aSelectedFormatName;
            Reference< ::com::sun::star::container::XNameAccess > xTypes ( m_xFact->createInstance( DEFINE_CONST_UNICODE( "com.sun.star.document.TypeDetection" ) ), UNO_QUERY_THROW );
            if ( aTypeName.getLength() )
            {
                try
                {
                    ::comphelper::SequenceAsHashMap( xTypes->getByName( aTypeName ) )[DEFINE_CONST_UNICODE("ClipboardFormat")] >>= aSelectedFormatName;
                }
                catch ( com::sun::star::uno::Exception& )
                {
                    // type name might be wrong
                    aTypeName = ::rtl::OUString();
                }
            }

            // if preselected format doesn't fit check the other ones
            if ( aSelectedFormatName != aFormatName )
            {
                // no type detected so far because preselected one didn't match
                aTypeName = ::rtl::OUString();

                // check if template type is wanted (template and "normal" document share the MediaType)
                // a template type will be detected only if the file extension is "vor" (how else?!)
                INetURLObject aObj( aFileName );
                sal_Bool bIsTemplate = ( aObj.getExtension().equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("vor")) );

                // get possible types
                Reference< ::com::sun::star::container::XContainerQuery > xTypeCFG ( xTypes, UNO_QUERY_THROW );
                Sequence < com::sun::star::beans::NamedValue > aSeq(1);
                aSeq[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ClipboardFormat" ));
                aSeq[0].Value <<= aFormatName;
                Reference < com::sun::star::container::XEnumeration > xEnum = xTypeCFG->createSubSetEnumerationByProperties( aSeq );
                while ( xEnum->hasMoreElements() )
                {
                    Sequence < ::rtl::OUString > aExtensions;
                    ::comphelper::SequenceAsHashMap aHashedProps( xEnum->nextElement() );
                    aHashedProps[DEFINE_CONST_UNICODE("ClipboardFormat")] >>= aSelectedFormatName;
                    aHashedProps[DEFINE_CONST_UNICODE("Extensions")] >>= aExtensions;
                    if ( aSelectedFormatName == aFormatName )
                    {
                        // check wether this type specifies a template
                        sal_Bool bIsTemplateType = sal_False;
                        for ( sal_Int32 n=0; n<aExtensions.getLength(); n++ )
                            if ( aExtensions[n].equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("vor")) )
                                bIsTemplateType = sal_True;

                        if ( bIsTemplateType != bIsTemplate )
                            // return it only if the file has the template extension (see comment above)
                            continue;

                        // get the type name from the properties
                        aHashedProps[DEFINE_CONST_UNICODE("Name")] >>= aTypeName;
                        break;
                    }
                }
            }
            else
                aFilterName = aPreselectedFilterName;
        }
        else
        {
            // check for the stream based formats
            if ( checkFormat( pStream, aTypeName, FALSE ) )
                aFilterName = aPreselectedFilterName;
            else
                checkFormat( pStream, aTypeName, TRUE );
        }

        DELETEZ( pStream );
    }
    catch ( com::sun::star::uno::Exception &)
    {
        // in case something goes wrong with type detection or filter configuration
       aTypeName = ::rtl::OUString();
    }

    /* #i71829# Dont search for any suitable filter during deep detection.
                A detection service is used to find a suitable type - not filter.
                Search for a filter is done outside ... recognizing preferred flags;
                user preselection and some other things too.
    */

    // only return a type name if it matches to the valid ones
    // Because stripped binfilter detect EVERY format, which was known before
    // code was changed ... it can happen that typename represent a format not realy
    // supported by binfilter component.
    sal_Bool bSupportedType = sal_False;
    if ( aTypeName.getLength() )
    {
        for (sal_Int32 i=0; i < nFileTypeCount; ++i)
        {
            if ( aTypeName.equalsAsciiL( aFileTypeList[i].Type, aFileTypeList[i].Length ) )
            {
                bSupportedType = true;
                break;
            }
        }
    }

    if ( bSupportedType )
        aMedium[comphelper::MediaDescriptor::PROP_TYPENAME()] <<= aTypeName;
    else
        aTypeName = ::rtl::OUString();

    lDescriptor = aMedium.getAsConstPropertyValueList();

    return aTypeName;
}

SFX_IMPL_SINGLEFACTORY( BinFilterDetect )

/* XServiceInfo */
UNOOUSTRING SAL_CALL BinFilterDetect::getImplementationName() throw( UNORUNTIMEEXCEPTION )
{
    return impl_getStaticImplementationName();
}
                                                                                                                                \
/* XServiceInfo */
sal_Bool SAL_CALL BinFilterDetect::supportsService( const UNOOUSTRING& sServiceName ) throw( UNORUNTIMEEXCEPTION )
{
    UNOSEQUENCE< UNOOUSTRING >  seqServiceNames =   getSupportedServiceNames();
    const UNOOUSTRING*          pArray          =   seqServiceNames.getConstArray();
    for ( sal_Int32 nCounter=0; nCounter<seqServiceNames.getLength(); nCounter++ )
    {
        if ( pArray[nCounter] == sServiceName )
        {
            return sal_True ;
        }
    }
    return sal_False ;
}

/* XServiceInfo */
UNOSEQUENCE< UNOOUSTRING > SAL_CALL BinFilterDetect::getSupportedServiceNames() throw( UNORUNTIMEEXCEPTION )
{
    return impl_getStaticSupportedServiceNames();
}

/* Helper for XServiceInfo */
UNOSEQUENCE< UNOOUSTRING > BinFilterDetect::impl_getStaticSupportedServiceNames()
{
    UNOMUTEXGUARD aGuard( UNOMUTEX::getGlobalMutex() );
    UNOSEQUENCE< UNOOUSTRING > seqServiceNames( 1 );
    seqServiceNames.getArray() [0] = UNOOUSTRING( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.ExtendedTypeDetection" ));
    return seqServiceNames ;
}

/* Helper for XServiceInfo */
UNOOUSTRING BinFilterDetect::impl_getStaticImplementationName()
{
    return UNOOUSTRING( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.sfx2.BinaryFormatDetector" ));
}

/* Helper for registry */
UNOREFERENCE< UNOXINTERFACE > SAL_CALL BinFilterDetect::impl_createInstance( const UNOREFERENCE< UNOXMULTISERVICEFACTORY >& xServiceManager ) throw( UNOEXCEPTION )
{
    return UNOREFERENCE< UNOXINTERFACE >( *new BinFilterDetect( xServiceManager ) );
}

}
extern "C" {

SAL_DLLPUBLIC_EXPORT void* SAL_CALL component_getFactory(   const   sal_Char*   pImplementationName ,
                                                void*       pServiceManager     ,
                                                void*       /*pRegistryKey*/        )
{
    // Set default return value for this operation - if it failed.
    void* pReturn = NULL ;

    if  (
            ( pImplementationName   !=  NULL ) &&
            ( pServiceManager       !=  NULL )
        )
    {
        // Define variables which are used in following macros.
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory >   xFactory                                                                                                ;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    xServiceManager( reinterpret_cast< ::com::sun::star::lang::XMultiServiceFactory* >( pServiceManager ) ) ;

        //=============================================================================
        //  Add new macro line to handle new service.
        //
        //  !!! ATTENTION !!!
        //      Write no ";" at end of line and dont forget "else" ! (see macro)
        //=============================================================================
        IF_NAME_CREATECOMPONENTFACTORY( ::binfilter::BinFilterDetect )

        // Factory is valid - service was found.
        if ( xFactory.is() )
        {
            xFactory->acquire();
            pReturn = xFactory.get();
        }
    }
    // Return with result of this operation.
    return pReturn ;
}
} // extern "C"


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
