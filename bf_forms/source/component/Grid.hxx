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

#include "FormComponent.hxx"

#include <com/sun/star/form/XGridColumnFactory.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/form/XLoadable.hpp>

#include <tools/link.hxx>

#include "InterfaceContainer.hxx"

#include <comphelper/proparrhlp.hxx>
#include <cppuhelper/implbase6.hxx>
#include "errorbroadcaster.hxx"
namespace binfilter {

//.........................................................................
namespace frm
{
//.........................................................................

class OGridColumn;

//==================================================================
// ColumnDescription
//==================================================================

    struct ColumnDescription : public ElementDescription
    {
    public:
        OGridColumn*                pColumn;        // not owned by this instance! only to prevent duplicate XUnoTunnel usage
    };

//==================================================================
// OGridControlModel
//==================================================================
typedef ::cppu::ImplHelper6 <   ::com::sun::star::awt::XControlModel
                            ,   ::com::sun::star::form::XGridColumnFactory
                            ,   ::com::sun::star::form::XLoadListener
                            ,   ::com::sun::star::form::XReset
                            ,   ::com::sun::star::view::XSelectionSupplier
                            ,   ::com::sun::star::sdb::XSQLErrorListener
                            >   OGridControlModel_BASE;

class OGridControlModel :public OControlModel
                        ,public OInterfaceContainer
                        ,public OErrorBroadcaster
                        ,public OAggregationArrayUsageHelper< OGridControlModel >
                            // though we don't use aggregation, we're derived from an OPropertySetAggregationHelper,
                            // which expects that we use an OPropertyArrayAggregationHelper, which we ensure
                            // with deriving from OAggregationArrayUsageHelper
                        ,public OGridControlModel_BASE
{
    ::cppu::OInterfaceContainerHelper       m_aSelectListeners,
                                            m_aResetListeners;

// [properties]
    ::com::sun::star::uno::Any              m_aRowHeight;           // Zeilenhoehe
    ::com::sun::star::uno::Any              m_aTabStop;
    ::com::sun::star::uno::Any              m_aTextColor;
    ::com::sun::star::uno::Any              m_aBackgroundColor;
    ::com::sun::star::uno::Any              m_aCursorColor;             // transient
    ::com::sun::star::awt::FontDescriptor   m_aFont;
    ::com::sun::star::awt::FontDescriptor   m_aOldFont;
    ::rtl::OUString                         m_aDefaultControl;
    ::rtl::OUString                         m_sHelpText;
    ::com::sun::star::uno::Any              m_aTextLineColor;
    sal_Int16                               m_nFontRelief;
    sal_Int16                               m_nFontEmphasis;
// [properties]

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       m_xSelection;
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XLoadable >           m_xParentFormLoadable;
    sal_uInt32                  m_nFontEvent;

// [properties]
    ::rtl::OUString             m_sHelpURL;                 // URL
    sal_Int16                   m_nBorder;
    sal_Bool                    m_bEnable           : 1;
    sal_Bool                    m_bNavigation       : 1;
    sal_Bool                    m_bRecordMarker     : 1;
    sal_Bool                    m_bPrintable        : 1;
    sal_Bool                    m_bAlwaysShowCursor : 1;    // transient
    sal_Bool                    m_bDisplaySynchron  : 1;        // transient
// [properties]

public:
    DECLARE_DEFAULT_LEAF_XTOR( OGridControlModel );

// UNO Anbindung
    DECLARE_UNO3_AGG_DEFAULTS(OGridControlModel, OControlModel);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException);

// starcontainer::XChild
    virtual void SAL_CALL setParent(const InterfaceRef& Parent) throw(::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::lang::XServiceInfo
    IMPLEMENTATION_NAME(OGridControlModel);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

// XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);

// OComponentHelper
    virtual void SAL_CALL disposing();

// ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& _rSource) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::form::XLoadListener
    virtual void SAL_CALL loaded(const ::com::sun::star::lang::EventObject& aEvent) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL unloaded(const ::com::sun::star::lang::EventObject& aEvent) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL unloading(const ::com::sun::star::lang::EventObject& aEvent) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL reloading(const ::com::sun::star::lang::EventObject& aEvent) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL reloaded(const ::com::sun::star::lang::EventObject& aEvent) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::form::XReset
    virtual void SAL_CALL reset() throw ( ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addResetListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XResetListener>& _rxListener) throw ( ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeResetListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XResetListener>& _rxListener) throw ( ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::view::XSelectionSupplier
    virtual sal_Bool SAL_CALL select(const ::com::sun::star::uno::Any& aElement) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getSelection() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addSelectionChangeListener(const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener >& xListener) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeSelectionChangeListener(const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener >& xListener) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::form::XGridColumnFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> SAL_CALL createColumn(const ::rtl::OUString& ColumnType) throw ( :: com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual StringSequence SAL_CALL getColumnTypes() throw ( ::com::sun::star::uno::RuntimeException);

// stario::XPersistObject
    virtual ::rtl::OUString SAL_CALL getServiceName() throw ( ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL write(const ::com::sun::star::uno::Reference<stario::XObjectOutputStream>& _rxOutStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL read(const ::com::sun::star::uno::Reference<stario::XObjectInputStream>& _rxInStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException);
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
    virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const;
    virtual sal_Bool SAL_CALL convertFastPropertyValue(::com::sun::star::uno::Any& rConvertedValue, ::com::sun::star::uno::Any& rOldValue,
                                          sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
                                        throw(::com::sun::star::lang::IllegalArgumentException);
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue) throw ( ::com::sun::star::uno::Exception);

// ::com::sun::star::beans::XPropertyState
    virtual ::com::sun::star::uno::Any getPropertyDefaultByHandle( sal_Int32 nHandle ) const;

// ::com::sun::star::sdb::XSQLErrorListener
    virtual void SAL_CALL errorOccured( const ::com::sun::star::sdb::SQLErrorEvent& _rEvent ) throw (::com::sun::star::uno::RuntimeException);

// OAggregationArrayUsageHelper
    virtual void fillProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps,
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rAggregateProps
        ) const;

    // prevent method hiding
    using OControlModel::disposing;
    using OControlModel::getFastPropertyValue;

    IMPLEMENT_INFO_SERVICE()

protected:
    DECLARE_XCLONEABLE();

protected:
    virtual void approveNewElement(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxObject,
            ElementDescription* _pElement
        );

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>  createColumn(sal_Int32 nTypeId) const;

    OGridColumn* getColumnImplementation(const InterfaceRef& _rxIFace) const;

    virtual ElementDescription* createElementMetaData( );

protected:
    virtual void implRemoved(const InterfaceRef& _rxObject);
    virtual void implInserted( const ElementDescription* _pElement );
    virtual void implReplaced( const InterfaceRef& _rxReplacedObject, const ElementDescription* _pElement );

    void gotColumn(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxColumn);
    void lostColumn(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxColumn);

    void cloneColumns( const OGridControlModel* _pOriginalContainer );

private:
    DECL_LINK( OnFontChanged, void* );
};

//.........................................................................
}   // namespace frm
//.........................................................................
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
