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

#include <tools/debug.hxx>

#include <com/sun/star/io/XActiveDataControl.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/io/XSeekable.hdl>

#include <unotools/streamwrap.hxx>
#include <bf_sfx2/docfile.hxx>
#include <bf_xmloff/xmluconv.hxx>

#include "bf_xmloff/xmlnmspe.hxx"
#include "bf_xmloff/nmspmap.hxx"
#include "bf_xmloff/xmlkywd.hxx"
#include "bf_xmloff/DashStyle.hxx"
#include "bf_xmloff/GradientStyle.hxx"
#include "bf_xmloff/HatchStyle.hxx"
#include "bf_xmloff/ImageStyle.hxx"
#include "bf_xmloff/MarkerStyle.hxx"

#include "xmlgrhlp.hxx"

#include "xmlxtimp.hxx"
#include <legacysmgr/legacy_binfilters_smgr.hxx>

namespace binfilter {
using namespace ::com::sun::star;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::xml::sax;
using namespace cppu;

using rtl::OUString;

///////////////////////////////////////////////////////////////////////

enum SvxXMLTableImportContextEnum { stice_unknown, stice_color, stice_marker, stice_dash, stice_hatch, stice_gradient, stice_bitmap };

///////////////////////////////////////////////////////////////////////

class SvxXMLTableImportContext : public SvXMLImportContext
{
public:
    SvxXMLTableImportContext( SvXMLImport& rImport, USHORT nPrfx, const OUString& rLName, const Reference< XAttributeList >& xAttrList, SvxXMLTableImportContextEnum eContext, const Reference< XNameContainer >& xTable );
    virtual ~SvxXMLTableImportContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList );

    SvxXMLXTableImport& getImport() const { return *(SvxXMLXTableImport*)&GetImport(); }

protected:
    void importColor( USHORT nPrfx, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName );
    void importMarker( USHORT nPrfx, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName );
    void importDash( USHORT nPrfx, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName );
    void importHatch( USHORT nPrfx, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName );
    void importGradient( USHORT nPrfx, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName );
    void importBitmap( USHORT nPrfx, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName );

private:
    Reference< XNameContainer > mxTable;
    SvxXMLTableImportContextEnum meContext;
};

///////////////////////////////////////////////////////////////////////

/*N*/ SvxXMLTableImportContext::SvxXMLTableImportContext( SvXMLImport& rInImport, USHORT nPrfx, const OUString& rLName, const Reference< XAttributeList >& /*xAttrList*/, SvxXMLTableImportContextEnum eContext, const Reference< XNameContainer >& xTable )
/*N*/ : SvXMLImportContext( rInImport, nPrfx, rLName )
/*N*/ , mxTable( xTable )
/*N*/ , meContext( eContext )
/*N*/ {
/*N*/ }

/*N*/ SvxXMLTableImportContext::~SvxXMLTableImportContext()
/*N*/ {
/*N*/ }

/*N*/ SvXMLImportContext *SvxXMLTableImportContext::CreateChildContext( USHORT nInPrefix, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList )
/*N*/ {
/*N*/   if( XML_NAMESPACE_DRAW == nInPrefix )
/*N*/   {
/*N*/       try
/*N*/       {
/*N*/           Any aAny;
/*N*/           OUString aName;
/*N*/
/*N*/           switch( meContext )
/*N*/           {
/*N*/           case stice_color:
/*N*/               importColor( nInPrefix, rLocalName, xAttrList, aAny, aName );
/*N*/               break;
/*N*/           case stice_marker:
/*N*/               importMarker( nInPrefix, rLocalName, xAttrList, aAny, aName  );
/*N*/               break;
/*N*/           case stice_dash:
/*N*/               importDash( nInPrefix, rLocalName, xAttrList, aAny, aName  );
/*N*/               break;
/*N*/           case stice_hatch:
/*N*/               importHatch( nInPrefix, rLocalName, xAttrList, aAny, aName  );
/*N*/               break;
/*N*/           case stice_gradient:
/*N*/               importGradient( nInPrefix, rLocalName, xAttrList, aAny, aName  );
/*N*/               break;
/*N*/           case stice_bitmap:
/*N*/               importBitmap( nInPrefix, rLocalName, xAttrList, aAny, aName  );
/*N*/               break;
/*N*/           default:
/*N*/               break;
/*N*/           }
/*N*/
/*N*/           if( aName.getLength() && aAny.hasValue() )
/*N*/           {
/*N*/               if( mxTable->hasByName( aName ) )
/*N*/               {
/*?*/                   mxTable->replaceByName( aName, aAny );
/*N*/               }
/*N*/               else
/*N*/               {
/*N*/                   mxTable->insertByName( aName, aAny );
/*N*/               }
/*N*/           }
/*N*/       }
/*N*/       catch (const uno::Exception&)
/*N*/       {
/*N*/       }
/*N*/   }
/*N*/
/*N*/   return new SvXMLImportContext( GetImport(), nInPrefix, rLocalName );
/*N*/ }

/*N*/ void SvxXMLTableImportContext::importColor( USHORT /*nPrfx*/, const OUString& /*rLocalName*/, const Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName )
/*N*/ {
/*N*/   const sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
/*N*/   for( sal_Int16 i=0; i < nAttrCount; i++ )
/*N*/   {
/*N*/       const OUString& rFullAttrName = xAttrList->getNameByIndex( i );
/*N*/       OUString aLclLocalName;
/*N*/       sal_uInt16 nLclPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( rFullAttrName, &aLclLocalName );
/*N*/
/*N*/
/*N*/       if( XML_NAMESPACE_DRAW == nLclPrefix )
/*N*/       {
/*N*/           if( aLclLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_name ) ) )
/*N*/           {
/*N*/               rName = xAttrList->getValueByIndex( i );
/*N*/           }
/*N*/           else if( aLclLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_color ) ) )
/*N*/           {
/*N*/               Color aColor;
/*N*/               SvXMLUnitConverter::convertColor(aColor, xAttrList->getValueByIndex( i ));
/*N*/               rAny <<= (sal_Int32)aColor.GetColor();
/*N*/           }
/*N*/       }
/*N*/   }
/*N*/ }

/*N*/ void SvxXMLTableImportContext::importMarker( USHORT /*nPrfx*/, const OUString& /*rLocalName*/, const Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName )
/*N*/ {
/*N*/   XMLMarkerStyleImport aMarkerStyle( GetImport() );
/*N*/   aMarkerStyle.importXML( xAttrList, rAny, rName );
/*N*/ }

/*N*/ void SvxXMLTableImportContext::importDash( USHORT /*nPrfx*/, const OUString& /*rLocalName*/, const Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName )
/*N*/ {
/*N*/   XMLDashStyleImport aDashStyle( GetImport() );
/*N*/   aDashStyle.importXML( xAttrList, rAny, rName );
/*N*/ }

/*N*/ void SvxXMLTableImportContext::importHatch( USHORT /*nPrfx*/, const OUString& /*rLocalName*/, const Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName )
/*N*/ {
/*N*/   XMLHatchStyleImport aHatchStyle( GetImport() );
/*N*/   aHatchStyle.importXML( xAttrList, rAny, rName );
/*N*/ }

/*N*/ void SvxXMLTableImportContext::importGradient( USHORT /*nPrfx*/, const OUString& /*rLocalName*/, const Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName )
/*N*/ {
/*N*/   XMLGradientStyleImport aGradientStyle( GetImport() );
/*N*/   aGradientStyle.importXML( xAttrList, rAny, rName );
/*N*/ }

/*N*/ void SvxXMLTableImportContext::importBitmap( USHORT /*nPrfx*/, const OUString& /*rLocalName*/, const Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName )
/*N*/ {
/*N*/   XMLImageStyle aImageStyle;
/*N*/   aImageStyle.importXML( xAttrList, rAny, rName, GetImport() );
/*N*/ }

///////////////////////////////////////////////////////////////////////

/*N*/ // #110680#
/*N*/ SvxXMLXTableImport::SvxXMLXTableImport(
/*N*/   const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
/*N*/   const Reference< XNameContainer > & rTable,
/*N*/   Reference< XGraphicObjectResolver >& xGrfResolver )
/*N*/ : SvXMLImport(xServiceFactory),
/*N*/   mrTable( rTable )
/*N*/ {
/*N*/   SetGraphicResolver( xGrfResolver );
/*N*/ }

/*N*/ SvxXMLXTableImport::~SvxXMLXTableImport() throw ()
/*N*/ {
/*N*/ }

/*N*/ sal_Bool SvxXMLXTableImport::load( const OUString& rUrl, const Reference< XNameContainer >& xTable ) throw()
/*N*/ {
/*N*/   sal_Bool bRet = sal_True;
/*N*/
/*N*/   Reference< XGraphicObjectResolver > xGrfResolver;
/*N*/   SvXMLGraphicHelper* pGraphicHelper = 0;
/*N*/
/*N*/   try
/*N*/   {
/*N*/       do
/*N*/       {
/*N*/           SfxMedium aMedium( rUrl, STREAM_READ | STREAM_NOCREATE, TRUE );
/*N*/
/*N*/           uno::Reference<lang::XMultiServiceFactory> xServiceFactory( ::legacy_binfilters::getLegacyProcessServiceFactory() );
/*N*/           if( !xServiceFactory.is() )
/*N*/           {
/*N*/               OSL_FAIL( "SvxXMLXTableImport::load: got no service manager" );
/*N*/               break;
/*N*/           }
/*N*/
/*N*/           uno::Reference< xml::sax::XParser > xParser( xServiceFactory->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.Parser" ) ) ), uno::UNO_QUERY );
/*N*/           if( !xParser.is() )
/*N*/           {
/*N*/               OSL_FAIL( "com.sun.star.xml.sax.Parser service missing" );
/*N*/               break;
/*N*/           }
/*N*/
/*N*/           SvStorageStreamRef                      xIStm;
/*N*/           uno::Reference< io::XActiveDataSource > xSource;
/*N*/
/*N*/           SvStorage*                              pStorage = aMedium.GetStorage();
/*N*/
/*N*/           xml::sax::InputSource                   aParserInput;
/*N*/           aParserInput.sSystemId = aMedium.GetName();
/*N*/
/*N*/           if( pStorage )
/*N*/           {
/*N*/               const String aContentStmName( RTL_CONSTASCII_USTRINGPARAM( "Content.xml" ) );
/*N*/
/*N*/               xIStm = pStorage->OpenStream( aContentStmName, STREAM_READ | STREAM_NOCREATE );
/*N*/               if( !xIStm.Is() )
/*N*/               {
/*N*/                   OSL_FAIL( "could not open Content stream" );
/*N*/                   break;
/*N*/               }
/*N*/
/*N*/               xIStm->SetBufferSize( 16 * 1024 );
/*N*/               aParserInput.aInputStream = new ::utl::OInputStreamWrapper( *xIStm );
/*N*/
/*N*/               pGraphicHelper = SvXMLGraphicHelper::Create( *pStorage, GRAPHICHELPER_MODE_READ );
/*N*/               xGrfResolver = pGraphicHelper;
/*N*/           }
/*N*/           else
/*N*/           {
/*N*/                 aParserInput.aInputStream = aMedium.GetInputStream();
/*N*/               Reference< io::XSeekable > xSeek( aParserInput.aInputStream, UNO_QUERY );
/*N*/               if ( xSeek.is() )
/*N*/                   xSeek->seek( 0 );
/*N*/           }
/*N*/
/*N*/           if( xSource.is() )
/*N*/           {
/*?*/               Reference< io::XActiveDataControl > xSourceControl( xSource, UNO_QUERY );
/*?*/               xSourceControl->start();
/*N*/           }
/*N*/
/*N*/           // #110680#
/*N*/           // Reference< XDocumentHandler > xHandler( new SvxXMLXTableImport( xTable, xGrfResolver ) );
/*N*/           Reference< XDocumentHandler > xHandler( new SvxXMLXTableImport( xServiceFactory, xTable, xGrfResolver ) );
/*N*/
/*N*/           xParser->setDocumentHandler( xHandler );
/*N*/           xParser->parseStream( aParserInput );
/*N*/       }
/*N*/       while(0);
/*N*/
/*N*/       if( pGraphicHelper )
/*N*/           SvXMLGraphicHelper::Destroy( pGraphicHelper );
/*N*/   }
/*N*/   catch (const uno::Exception&)
/*N*/   {
/*N*/       bRet = sal_False;
/*N*/   }
/*N*/
/*N*/   return bRet;
/*N*/ }

/*N*/ SvXMLImportContext *SvxXMLXTableImport::CreateContext( sal_uInt16 nInPrefix, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList )
/*N*/ {
/*N*/   if( XML_NAMESPACE_OFFICE == nInPrefix )
/*N*/   {
/*N*/       Type aType = mrTable->getElementType();
/*N*/
/*N*/       if( rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "color-table" ) ) )
/*N*/       {
/*N*/           if( aType == ::getCppuType((const sal_Int32*)0) )
/*N*/               return new SvxXMLTableImportContext( *this, nInPrefix, rLocalName, xAttrList, stice_color, mrTable );
/*N*/       }
/*N*/       else if( rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "marker-table" ) ) )
/*N*/       {
/*N*/           if( aType == ::getCppuType((const drawing::PolyPolygonBezierCoords*)0) )
/*N*/               return new SvxXMLTableImportContext( *this, nInPrefix, rLocalName, xAttrList, stice_marker, mrTable );
/*N*/       }
/*N*/       else if( rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "dash-table" ) ) )
/*N*/       {
/*N*/           if( aType == ::getCppuType((const drawing::LineDash*)0) )
/*N*/               return new SvxXMLTableImportContext( *this, nInPrefix, rLocalName, xAttrList, stice_dash, mrTable );
/*N*/       }
/*N*/       else if( rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "hatch-table" ) ) )
/*N*/       {
/*N*/           if( aType == ::getCppuType((const drawing::Hatch*)0) )
/*N*/               return new SvxXMLTableImportContext( *this, nInPrefix, rLocalName, xAttrList, stice_hatch, mrTable );
/*N*/       }
/*N*/       else if( rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "gradient-table" ) ) )
/*N*/       {
/*N*/           if( aType == ::getCppuType((const awt::Gradient*)0))
/*N*/               return new SvxXMLTableImportContext( *this, nInPrefix, rLocalName, xAttrList, stice_gradient, mrTable );
/*N*/       }
/*N*/       else if( rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "bitmap-table" ) ) )
/*N*/       {
/*N*/           if( aType == ::getCppuType((const OUString*)0))
/*N*/               return new SvxXMLTableImportContext( *this, nInPrefix, rLocalName, xAttrList, stice_bitmap, mrTable );
/*N*/       }
/*N*/   }
/*N*/
/*?*/   return new SvXMLImportContext( *this, nInPrefix, rLocalName );
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
