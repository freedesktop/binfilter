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
#ifndef _UNOTXDOC_HXX
#define _UNOTXDOC_HXX

#include <bf_svtools/bf_solar.h>

#include <bf_svtools/svarray.hxx>
#include <bf_sfx2/sfxbasemodel.hxx>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/document/XLinkTargetSupplier.hpp>
#include <com/sun/star/document/XRedlinesSupplier.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XEndnotesSupplier.hpp>
#include <com/sun/star/text/XEndnotesSettingsSupplier.hpp>
#include <com/sun/star/text/XTextSectionsSupplier.hpp>
#include <com/sun/star/text/XLineNumberingProperties.hpp>
#include <com/sun/star/text/XChapterNumberingSupplier.hpp>
#include <com/sun/star/text/XPagePrintable.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextGraphicObjectsSupplier.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XDocumentIndexesSupplier.hpp>
#include <com/sun/star/text/XBookmarksSupplier.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/text/XReferenceMarksSupplier.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/util/XReplaceable.hpp>
#include <com/sun/star/util/XReplaceDescriptor.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/util/XLinkUpdate.hpp>
#include <com/sun/star/view/XRenderable.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/i18n/XForbiddenCharacters.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <bf_svtools/itemprop.hxx>
#include <bf_svx/fmdmod.hxx>
#include <bf_svx/UnoForbiddenCharsTable.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/implbase2.hxx> // helper for implementations
#include <cppuhelper/implbase4.hxx> // helper for implementations
#include <RefreshListenerContainer.hxx>

#define __IFC27 Ifc1, Ifc2, Ifc3, Ifc4, Ifc5, Ifc6, Ifc7, Ifc8, Ifc9, Ifc10, Ifc11, Ifc12, Ifc13, Ifc14, Ifc15, Ifc16, \
Ifc17, Ifc18, Ifc19, Ifc20, Ifc21, Ifc22, Ifc23, Ifc24, Ifc25, Ifc26, Ifc27

#define __CLASS_IFC27 class Ifc1, class Ifc2, class Ifc3, class Ifc4, class Ifc5, class Ifc6, class Ifc7, \
class Ifc8, class Ifc9, class Ifc10, class Ifc11, class Ifc12, class Ifc13, class Ifc14, class Ifc15, class Ifc16, \
class Ifc17, class Ifc18, class Ifc19, class Ifc20, class Ifc21, class Ifc22, class Ifc23, class Ifc24,\
class Ifc25, class Ifc26, class Ifc27

#define __PUBLIC_IFC27 public Ifc1, public Ifc2, public Ifc3, public Ifc4, public Ifc5, public Ifc6, public Ifc7, public Ifc8, public Ifc9, public Ifc10, public Ifc11, public Ifc12, \
public Ifc13, public Ifc14, public Ifc15, public Ifc16, public Ifc17, public Ifc18, \
public Ifc19, public Ifc20, public Ifc21, public Ifc22, public Ifc23 , public Ifc24, \
public Ifc25, public Ifc26, public Ifc27

#include <cppuhelper/implbase_ex.hxx>

#include <cppuhelper/implbase_ex_pre.hxx>
#define __IFC_EX_TYPE_INIT27( class_cast ) \
    __IFC_EX_TYPE_INIT( class_cast, 1 ), __IFC_EX_TYPE_INIT( class_cast, 2 ), \
    __IFC_EX_TYPE_INIT( class_cast, 3 ), __IFC_EX_TYPE_INIT( class_cast, 4 ), \
    __IFC_EX_TYPE_INIT( class_cast, 5 ), __IFC_EX_TYPE_INIT( class_cast, 6 ), \
    __IFC_EX_TYPE_INIT( class_cast, 7 ), __IFC_EX_TYPE_INIT( class_cast, 8 ), \
    __IFC_EX_TYPE_INIT( class_cast, 9 ), __IFC_EX_TYPE_INIT( class_cast, 10 ), \
    __IFC_EX_TYPE_INIT( class_cast, 11 ), __IFC_EX_TYPE_INIT( class_cast, 12 ), \
    __IFC_EX_TYPE_INIT( class_cast, 13 ), __IFC_EX_TYPE_INIT( class_cast, 14 ), \
    __IFC_EX_TYPE_INIT( class_cast, 15 ), __IFC_EX_TYPE_INIT( class_cast, 16 ), \
    __IFC_EX_TYPE_INIT( class_cast, 17 ), __IFC_EX_TYPE_INIT( class_cast, 18 ), \
    __IFC_EX_TYPE_INIT( class_cast, 19 ), __IFC_EX_TYPE_INIT( class_cast, 20 ), \
    __IFC_EX_TYPE_INIT( class_cast, 21 ), __IFC_EX_TYPE_INIT( class_cast, 22 ), \
    __IFC_EX_TYPE_INIT( class_cast, 23 ), __IFC_EX_TYPE_INIT( class_cast, 24 ), \
    __IFC_EX_TYPE_INIT( class_cast, 25 ), __IFC_EX_TYPE_INIT( class_cast, 26 ), \
    __IFC_EX_TYPE_INIT( class_cast, 27 )
#include <cppuhelper/implbase_ex_post.hxx>
__DEF_IMPLHELPER_EX( 27 )

namespace binfilter {

class SwDoc;
class SvxForbiddenCharactersTable;
class SwDocShell;
class UnoActionContext;
class SwXBodyText;
class SwXDrawPage;
class SwUnoCrsr;
class SwXDocumentPropertyHelper;

typedef UnoActionContext* UnoActionContextPtr;
SV_DECL_PTRARR(ActionContextArr, UnoActionContextPtr, 4, 4)

typedef cppu::WeakImplHelper27
<
    ::com::sun::star::text::XTextDocument,
    ::com::sun::star::text::XLineNumberingProperties,
    ::com::sun::star::text::XChapterNumberingSupplier,
    ::com::sun::star::text::XFootnotesSupplier,
    ::com::sun::star::text::XEndnotesSupplier,
    ::com::sun::star::util::XReplaceable,
    ::com::sun::star::text::XPagePrintable,
    ::com::sun::star::text::XReferenceMarksSupplier,
    ::com::sun::star::text::XTextTablesSupplier,
    ::com::sun::star::text::XTextFramesSupplier,
    ::com::sun::star::text::XBookmarksSupplier,
    ::com::sun::star::text::XTextSectionsSupplier,
    ::com::sun::star::text::XTextGraphicObjectsSupplier,
    ::com::sun::star::text::XTextEmbeddedObjectsSupplier,
    ::com::sun::star::text::XTextFieldsSupplier,
    ::com::sun::star::style::XStyleFamiliesSupplier,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::drawing::XDrawPageSupplier,
    ::com::sun::star::text::XDocumentIndexesSupplier,
    ::com::sun::star::beans::XPropertySet,
    ::com::sun::star::beans::XPropertyState,
    ::com::sun::star::document::XLinkTargetSupplier,
    ::com::sun::star::document::XRedlinesSupplier,
    ::com::sun::star::util::XRefreshable,
    ::com::sun::star::util::XLinkUpdate,
    ::com::sun::star::lang::XUnoTunnel,
    ::com::sun::star::view::XRenderable
>
SwXTextDocumentBaseClass;

class SwXTextDocument : public SwXTextDocumentBaseClass,
    public SvxFmMSFactory,
    public SfxBaseModel
{
    ActionContextArr        aActionArr;
    SwRefreshListenerContainer  aRefreshCont;

    SfxItemPropertySet      aPropSet;

    SwDocShell*             pDocShell;
    sal_Bool                    bObjectValid;

    SwXDrawPage*            pDrawPage;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > *          pxXDrawPage;

    ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >               xBodyText;
    SwXBodyText*            pBodyText;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation >             xNumFmtAgg;

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > *     pxXFootnotes;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > *        pxXFootnoteSettings;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > *     pxXEndnotes;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > *        pxXEndnoteSettings;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > *          pxXReferenceMarks;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumerationAccess > *   pxXTextFieldTypes;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > *          pxXTextFieldMasters;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > *          pxXTextSections;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > *          pxXBookmarks;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > *          pxXTextTables;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > *          pxXTextFrames;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > *          pxXGraphicObjects;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > *          pxXEmbeddedObjects;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > *          pxXStyleFamilies;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexReplace > *        pxXChapterNumbering;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > *     pxXDocumentIndexes;

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > *         pxXLineNumberingProperties;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > *          pxLinkTargetSupplier;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumerationAccess >*    pxXRedlines;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>                    xPropertyHelper;
    SwXDocumentPropertyHelper*                                                              pPropertyHelper;

    void                    GetBodyText();
    void                    GetNumberFormatter();

    // used for XRenderable implementation
    SwDoc *         GetRenderDoc( const ::com::sun::star::uno::Any& rSelection );

protected:
    virtual ~SwXTextDocument();
public:
    SwXTextDocument(SwDocShell* pShell);

    inline void notifyRefreshListeners() { aRefreshCont.Refreshed(); }
    virtual     ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  ) throw();
    virtual void SAL_CALL release(  ) throw();

    //XWeak
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAdapter > SAL_CALL queryAdapter(  ) throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    //XTextDocument
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >  SAL_CALL getText(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL reformat(void) throw( ::com::sun::star::uno::RuntimeException );

    //XModel
    virtual sal_Bool SAL_CALL attachResource( const ::rtl::OUString& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getURL(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getArgs(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL connectController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& xController ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL disconnectController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& xController ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL lockControllers(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL unlockControllers(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasControllersLocked(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController > SAL_CALL getCurrentController(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setCurrentController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& xController ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getCurrentSelection(  ) throw(::com::sun::star::uno::RuntimeException);

    //XComponent
    virtual void SAL_CALL dispose(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );

    //XLineNumberingProperties
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL getLineNumberingProperties(void) throw( ::com::sun::star::uno::RuntimeException );

    //XChapterNumberingSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexReplace >  SAL_CALL getChapterNumberingRules(void) throw( ::com::sun::star::uno::RuntimeException );

    //XFootnotesSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >  SAL_CALL getFootnotes(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  SAL_CALL getFootnoteSettings(void) throw( ::com::sun::star::uno::RuntimeException );

    //XEndnotesSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >  SAL_CALL getEndnotes(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  SAL_CALL getEndnoteSettings(void) throw( ::com::sun::star::uno::RuntimeException );

    //XReplaceable
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XReplaceDescriptor >  SAL_CALL createReplaceDescriptor(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Int32 SAL_CALL replaceAll(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XSearchDescriptor > & xDesc) throw( ::com::sun::star::uno::RuntimeException );

    //XSearchable
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XSearchDescriptor >  SAL_CALL createSearchDescriptor(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >  SAL_CALL findAll(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XSearchDescriptor > & xDesc) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SAL_CALL findFirst(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XSearchDescriptor > & xDesc) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SAL_CALL findNext(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & xStartAt, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XSearchDescriptor > & xDesc) throw( ::com::sun::star::uno::RuntimeException );

    //XPagePrintable
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getPagePrintSettings(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setPagePrintSettings(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aSettings) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL printPages(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& xOptions) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

    //XReferenceMarksSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >  SAL_CALL getReferenceMarks(void) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::text::XTextFieldsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumerationAccess >  SAL_CALL getTextFields(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >  SAL_CALL getTextFieldMasters(void) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::text::XTextEmbeddedObjectsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >  SAL_CALL getEmbeddedObjects(void) throw( ::com::sun::star::uno::RuntimeException );

//  // ::com::sun::star::text::XTextShapesSupplier
//    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >  getShapes(void) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::text::XBookmarksSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >  SAL_CALL getBookmarks(void) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::text::XTextSectionsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >  SAL_CALL getTextSections(void) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::text::XTextTablesSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >  SAL_CALL getTextTables(void) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::text::XTextGraphicObjectsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >  SAL_CALL getGraphicObjects(void) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::text::XTextFramesSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >  SAL_CALL getTextFrames(void) throw( ::com::sun::star::uno::RuntimeException );

    //XStyleFamiliesSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >  SAL_CALL getStyleFamilies(void) throw( ::com::sun::star::uno::RuntimeException );

    //XMultiServiceFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SAL_CALL createInstance(const ::rtl::OUString& ServiceSpecifier)
                throw( ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SAL_CALL createInstanceWithArguments(const ::rtl::OUString& ServiceSpecifier,
                const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Arguments)
                throw( ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getAvailableServiceNames(void)
                throw( ::com::sun::star::uno::RuntimeException );

    //XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const ::rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::drawing::XDrawPageSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >  SAL_CALL getDrawPage(void) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::text::XDocumentIndexesSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >  SAL_CALL getDocumentIndexes(void) throw( ::com::sun::star::uno::RuntimeException );

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const ::rtl::OUString& rPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& rPropertyNames ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyToDefault( const ::rtl::OUString& rPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const ::rtl::OUString& rPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XLinkTargetSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >  SAL_CALL getLinks(void) throw( ::com::sun::star::uno::RuntimeException );

    //XRedlinesSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumerationAccess > SAL_CALL getRedlines(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::util::XRefreshable
    virtual void SAL_CALL refresh(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addRefreshListener(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XRefreshListener > & l) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeRefreshListener(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XRefreshListener > & l) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::util::XLinkUpdate,
    virtual void SAL_CALL updateLinks(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::view::XRenderable
    virtual sal_Int32 SAL_CALL getRendererCount( const ::com::sun::star::uno::Any& aSelection, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& xOptions ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getRenderer( sal_Int32 nRenderer, const ::com::sun::star::uno::Any& aSelection, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& xOptions ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL render( sal_Int32 nRenderer, const ::com::sun::star::uno::Any& aSelection, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& xOptions ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    using SfxBaseModel::addEventListener;
    using SfxBaseModel::removeEventListener;

    //
    void                        Invalidate();
    void                        Reactivate(SwDocShell* pNewDocShell);
    SwXDocumentPropertyHelper * GetPropertyHelper ();
    sal_Bool                    IsValid() const {return bObjectValid;}

    void                        InitNewDoc();

    SwUnoCrsr*                  CreateCursorForSearch(::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor > & xCrsr);
    SwUnoCrsr*                  FindAny(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XSearchDescriptor > & xDesc,
                                            ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor > & xCrsr, sal_Bool bAll,
                                            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  xLastResult);

    SwDocShell*                 GetDocShell() {return pDocShell;}

    void * SAL_CALL operator new( size_t ) throw();
    void SAL_CALL operator delete( void * ) throw();

};


class SwXLinkTargetSupplier : public cppu::WeakImplHelper2
<
    ::com::sun::star::container::XNameAccess,
    ::com::sun::star::lang::XServiceInfo
>
{
    SwXTextDocument* pxDoc;
    String sTables;
    String sFrames;
    String sGraphics;
    String sOLEs;
    String sSections;
    String sOutlines;
    String sBookmarks;

public:
    SwXLinkTargetSupplier(SwXTextDocument& rxDoc);
    ~SwXLinkTargetSupplier();

    //XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName(const ::rtl::OUString& Name)  throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL hasByName(const ::rtl::OUString& Name) throw( ::com::sun::star::uno::RuntimeException );

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const ::rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    //
    void    Invalidate() {pxDoc = 0;}
};


class SwXLinkNameAccessWrapper : public cppu::WeakImplHelper4
<
    ::com::sun::star::beans::XPropertySet,
    ::com::sun::star::container::XNameAccess,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::document::XLinkTargetSupplier
>
{
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >            xRealAccess;
    SfxItemPropertySet      aPropSet;
    const String            sLinkSuffix;
    const String            sLinkDisplayName;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextDocument >       xDoc;
    SwXTextDocument*        pxDoc;


public:
    SwXLinkNameAccessWrapper(::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >  xAccess,
            const String& rLinkDisplayName, String sSuffix);
    SwXLinkNameAccessWrapper(SwXTextDocument& rxDoc,
            const String& rLinkDisplayName, String sSuffix);
    ~SwXLinkNameAccessWrapper();

    //XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName(const ::rtl::OUString& Name)  throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL hasByName(const ::rtl::OUString& Name) throw( ::com::sun::star::uno::RuntimeException );

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XLinkTargetSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >  SAL_CALL getLinks(void) throw( ::com::sun::star::uno::RuntimeException );

    //XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const ::rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

};


class SwXOutlineTarget : public cppu::WeakImplHelper2
<
    ::com::sun::star::beans::XPropertySet,
    ::com::sun::star::lang::XServiceInfo
>
{
    SfxItemPropertySet      aPropSet;
    String                  sOutlineText;

public:
    SwXOutlineTarget(const String& rOutlineText);
    ~SwXOutlineTarget();

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const ::rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );
};
} //namespace binfilter
#endif
namespace binfilter {
class SwXDocumentPropertyHelper : public SvxUnoForbiddenCharsTable
{
    ::com::sun::star::uno::Reference < ::com::sun::star::uno::XInterface > xDashTable;
    ::com::sun::star::uno::Reference < ::com::sun::star::uno::XInterface > xGradientTable;
    ::com::sun::star::uno::Reference < ::com::sun::star::uno::XInterface > xHatchTable;
    ::com::sun::star::uno::Reference < ::com::sun::star::uno::XInterface > xBitmapTable;
    ::com::sun::star::uno::Reference < ::com::sun::star::uno::XInterface > xTransGradientTable;
    ::com::sun::star::uno::Reference < ::com::sun::star::uno::XInterface > xMarkerTable;
    ::com::sun::star::uno::Reference < ::com::sun::star::uno::XInterface > xDrawDefaults;

    SwDoc*  m_pDoc;
public:
    SwXDocumentPropertyHelper(SwDoc& rDoc);
    ~SwXDocumentPropertyHelper();
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> GetDrawTable(short nWhich);
    void Invalidate();
};
}//namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
