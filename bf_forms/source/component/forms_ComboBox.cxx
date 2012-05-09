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

#include <comphelper/proparrhlp.hxx>

#include "ComboBox.hxx"
#include "property.hrc"
#include "services.hxx"
#include "frm_resource.hxx"
#include "frm_resource.hrc"
#include "BaseListBox.hxx"
#include <comphelper/numbers.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/dbconversion.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>

#include <com/sun/star/form/FormComponentType.hpp>

#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <comphelper/property.hxx>

#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>

#include <limits.h>

namespace binfilter {

    using namespace ::dbtools;

//.........................................................................
namespace frm
{
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;

//========================================================================
// class OComboBoxModel
//========================================================================
sal_Int32 OComboBoxModel::nTextHandle = -1;

//------------------------------------------------------------------
InterfaceRef SAL_CALL OComboBoxModel_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory) throw (RuntimeException)
{
    return (*new OComboBoxModel(_rxFactory));
}

//------------------------------------------------------------------------------
Sequence<Type> OComboBoxModel::_getTypes()
{
    return ::comphelper::concatSequences(
        OBoundControlModel::_getTypes(),
        OErrorBroadcaster::getTypes()
    );
}

// XServiceInfo
//------------------------------------------------------------------------------
StringSequence SAL_CALL OComboBoxModel::getSupportedServiceNames() throw(RuntimeException)
{
    StringSequence aSupported = OBoundControlModel::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 2);

    ::rtl::OUString* pArray = aSupported.getArray();
    pArray[aSupported.getLength()-2] = FRM_SUN_COMPONENT_DATABASE_COMBOBOX;
    pArray[aSupported.getLength()-1] = FRM_SUN_COMPONENT_COMBOBOX;
    return aSupported;
}

//------------------------------------------------------------------------------
Any SAL_CALL OComboBoxModel::queryAggregation(const Type& _rType) throw (RuntimeException)
{
    Any aReturn = OBoundControlModel::queryAggregation( _rType );
    return aReturn.hasValue() ? aReturn : OErrorBroadcaster::queryInterface( _rType );
}

//------------------------------------------------------------------
DBG_NAME( OComboBoxModel )
//------------------------------------------------------------------
OComboBoxModel::OComboBoxModel(const Reference<XMultiServiceFactory>& _rxFactory)
    :OBoundControlModel(_rxFactory, VCL_CONTROLMODEL_COMBOBOX, FRM_CONTROL_COMBOBOX)
                    // use the old control name for compytibility reasons
    ,OErrorBroadcaster( OComponentHelper::rBHelper )
    ,m_eListSourceType(ListSourceType_TABLE)
    ,m_aNullDate(DBTypeConversion::getStandardDate())
    ,m_nFormatKey(0)
    ,m_nFieldType(DataType::OTHER)
    ,m_nKeyType(NumberFormat::UNDEFINED)
    ,m_bEmptyIsNull(sal_True)
{
    DBG_CTOR( OComboBoxModel, NULL );

    m_nClassId = FormComponentType::COMBOBOX;
    m_sDataFieldConnectivityProperty = PROPERTY_TEXT;
    if (OComboBoxModel::nTextHandle == -1)
        OComboBoxModel::nTextHandle = getOriginalHandle(PROPERTY_ID_TEXT);
}

//------------------------------------------------------------------
OComboBoxModel::OComboBoxModel( const OComboBoxModel* _pOriginal, const Reference<XMultiServiceFactory>& _rxFactory )
    :OBoundControlModel( _pOriginal, _rxFactory )
    ,OErrorBroadcaster( OComponentHelper::rBHelper )
    ,m_aNullDate(DBTypeConversion::getStandardDate())
    ,m_nFormatKey(0)
    ,m_nFieldType(DataType::OTHER)
    ,m_nKeyType(NumberFormat::UNDEFINED)
{
    DBG_CTOR( OComboBoxModel, NULL );

    m_eListSourceType = _pOriginal->m_eListSourceType;
    m_bEmptyIsNull = _pOriginal->m_bEmptyIsNull;
    m_aListSource = _pOriginal->m_aListSource;
    m_aDefaultText = _pOriginal->m_aDefaultText;
}

//------------------------------------------------------------------
OComboBoxModel::~OComboBoxModel()
{
    if (!OComponentHelper::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }

    DBG_DTOR( OComboBoxModel, NULL );
}

// XCloneable
//------------------------------------------------------------------------------
IMPLEMENT_DEFAULT_CLONING( OComboBoxModel )

//------------------------------------------------------------------------------
void OComboBoxModel::disposing()
{
    OBoundControlModel::disposing();
    OErrorBroadcaster::disposing();
    m_xFormatter = NULL;
}

//------------------------------------------------------------------------------
void OComboBoxModel::getFastPropertyValue(Any& _rValue, sal_Int32 _nHandle) const
{
    switch (_nHandle)
    {
        case PROPERTY_ID_LISTSOURCETYPE : _rValue <<= m_eListSourceType; break;
        case PROPERTY_ID_LISTSOURCE     : _rValue <<= m_aListSource; break;
        case PROPERTY_ID_EMPTY_IS_NULL  : _rValue <<= m_bEmptyIsNull; break;
        case PROPERTY_ID_DEFAULT_TEXT   : _rValue <<= m_aDefaultText; break;
        default:
            OBoundControlModel::getFastPropertyValue(_rValue, _nHandle);
    }
}

//------------------------------------------------------------------------------
void OComboBoxModel::setFastPropertyValue_NoBroadcast(sal_Int32 _nHandle, const Any& _rValue)
                        throw (Exception)
{
    switch (_nHandle)
    {
        case PROPERTY_ID_LISTSOURCETYPE :
            DBG_ASSERT(_rValue.getValueType().equals(::getCppuType(reinterpret_cast<ListSourceType*>(NULL))),
                "OComboBoxModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            _rValue >>= m_eListSourceType;
            break;

        case PROPERTY_ID_LISTSOURCE :
                        DBG_ASSERT(_rValue.getValueType().getTypeClass() == TypeClass_STRING,
                "OComboBoxModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            _rValue >>= m_aListSource;
            // die ListSource hat sich geaendert -> neu laden
            if (ListSourceType_VALUELIST != m_eListSourceType)
            {
                if (m_xCursor.is() && !getField().is()) // combobox bereits mit Datenbank verbunden ?
                    // neu laden
                    loadData();
            }
            break;

        case PROPERTY_ID_EMPTY_IS_NULL :
                        DBG_ASSERT(_rValue.getValueType().getTypeClass() == TypeClass_BOOLEAN,
                "OComboBoxModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            _rValue >>= m_bEmptyIsNull;
            break;

        case PROPERTY_ID_DEFAULT_TEXT :
                        DBG_ASSERT(_rValue.getValueType().getTypeClass() == TypeClass_STRING,
                "OComboBoxModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            _rValue >>= m_aDefaultText;
            //_reset();
            break;

        default:
            OBoundControlModel::setFastPropertyValue_NoBroadcast(_nHandle, _rValue);
    }
}

//------------------------------------------------------------------------------
sal_Bool OComboBoxModel::convertFastPropertyValue(
                        Any& _rConvertedValue, Any& _rOldValue, sal_Int32 _nHandle, const Any& _rValue)
                        throw (IllegalArgumentException)
{
    sal_Bool bModified(sal_False);
    switch (_nHandle)
    {
        case PROPERTY_ID_LISTSOURCETYPE :
            bModified = tryPropertyValueEnum(_rConvertedValue, _rOldValue, _rValue, m_eListSourceType);
            break;

        case PROPERTY_ID_LISTSOURCE :
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_aListSource);
            break;

        case PROPERTY_ID_EMPTY_IS_NULL :
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_bEmptyIsNull);
            break;

        case PROPERTY_ID_DEFAULT_TEXT :
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_aDefaultText);
            break;

        default:
            bModified = OBoundControlModel::convertFastPropertyValue(_rConvertedValue, _rOldValue, _nHandle, _rValue);
            break;
    }
    return bModified;
}

//------------------------------------------------------------------------------
Reference<XPropertySetInfo> SAL_CALL OComboBoxModel::getPropertySetInfo() throw(RuntimeException)
{
        Reference<XPropertySetInfo> xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//------------------------------------------------------------------------------
cppu::IPropertyArrayHelper& OComboBoxModel::getInfoHelper()
{
    return *const_cast<OComboBoxModel*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
void OComboBoxModel::fillProperties(
                Sequence< Property >& _rProps,
                Sequence< Property >& _rAggregateProps ) const
{
    FRM_BEGIN_PROP_HELPER(12)
        DECL_PROP2(CLASSID,             sal_Int16,                  READONLY, TRANSIENT);
        DECL_PROP1(NAME,                ::rtl::OUString,            BOUND);
        DECL_PROP1(TAG,                 ::rtl::OUString,            BOUND);
        DECL_PROP1(TABINDEX,            sal_Int16,                  BOUND);
        DECL_PROP1(LISTSOURCETYPE,      ListSourceType, BOUND);
        DECL_PROP1(LISTSOURCE,          ::rtl::OUString,            BOUND);
        DECL_BOOL_PROP1(EMPTY_IS_NULL,                              BOUND);
        DECL_PROP1(DEFAULT_TEXT,        ::rtl::OUString,            BOUND);
        DECL_PROP1(CONTROLSOURCE,       ::rtl::OUString,            BOUND);
        DECL_IFACE_PROP3(BOUNDFIELD,    XPropertySet,               BOUND,READONLY, TRANSIENT);
        DECL_IFACE_PROP2(CONTROLLABEL,  XPropertySet,               BOUND, MAYBEVOID);
        DECL_PROP2(CONTROLSOURCEPROPERTY,   rtl::OUString,          READONLY, TRANSIENT);
    FRM_END_PROP_HELPER();
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OComboBoxModel::getServiceName() throw(RuntimeException)
{
    return FRM_COMPONENT_COMBOBOX;  // old (non-sun) name for compatibility !
}

//------------------------------------------------------------------------------
void SAL_CALL OComboBoxModel::write(const Reference<stario::XObjectOutputStream>& _rxOutStream)
        throw(stario::IOException, RuntimeException)
{
    OBoundControlModel::write(_rxOutStream);

    // Version
    // Version 0x0002:  EmptyIsNull
    // Version 0x0003:  ListSource->Seq
    // Version 0x0004:  DefaultText
    // Version 0x0005:  HelpText
    _rxOutStream->writeShort(0x0006);

    // Maskierung fuer any
    sal_uInt16 nAnyMask = 0;
    if (m_aBoundColumn.getValueType().getTypeClass() == TypeClass_SHORT)
        nAnyMask |= BOUNDCOLUMN;
    _rxOutStream << nAnyMask;

    StringSequence aListSourceSeq(&m_aListSource, 1);
    _rxOutStream << aListSourceSeq;
    _rxOutStream << (sal_Int16)m_eListSourceType;

    if ((nAnyMask & BOUNDCOLUMN) == BOUNDCOLUMN)
    {
        sal_Int16 nBoundColumn(0);
        m_aBoundColumn >>= nBoundColumn;
        _rxOutStream << nBoundColumn;
    }

    _rxOutStream << (sal_Bool)m_bEmptyIsNull;
    _rxOutStream << m_aDefaultText;
    writeHelpTextCompatibly(_rxOutStream);

    // from version 0x0006 : common properties
    writeCommonProperties(_rxOutStream);
}

//------------------------------------------------------------------------------
void SAL_CALL OComboBoxModel::read(const Reference<stario::XObjectInputStream>& _rxInStream) throw(stario::IOException, RuntimeException)
{
    OBoundControlModel::read(_rxInStream);
    ::osl::MutexGuard aGuard(m_aMutex);

    // Version
    sal_uInt16 nVersion = _rxInStream->readShort();
    DBG_ASSERT(nVersion > 0, "OComboBoxModel::read : version 0 ? this should never have been written !");

    if (nVersion > 0x0006)
    {
        OSL_FAIL("OComboBoxModel::read : invalid (means unknown) version !");
        m_aListSource = ::rtl::OUString();
        m_aBoundColumn <<= (sal_Int16)0;
        m_aDefaultText = ::rtl::OUString();
        m_eListSourceType = ListSourceType_TABLE;
        m_bEmptyIsNull = sal_True;
        defaultCommonProperties();
        return;
    }

    // Maskierung fuer any
    sal_uInt16 nAnyMask;
    _rxInStream >> nAnyMask;

    // ListSource
    if (nVersion < 0x0003)
    {
        ::rtl::OUString sListSource;
        _rxInStream >> m_aListSource;
    }
    else // nVersion == 4
    {
        m_aListSource = ::rtl::OUString();
        StringSequence aListSource;
        _rxInStream >> aListSource;
        const ::rtl::OUString* pToken = aListSource.getConstArray();
        sal_Int32 nLen = aListSource.getLength();
        for (sal_Int32 i = 0; i < nLen; ++i, ++pToken)
            m_aListSource += *pToken;
    }

    sal_Int16 nValue;
    _rxInStream >> nValue;
    m_eListSourceType = (ListSourceType)nValue;

    if ((nAnyMask & BOUNDCOLUMN) == BOUNDCOLUMN)
    {
        sal_Int16 nLclValue;
        _rxInStream >> nLclValue;
        m_aBoundColumn <<= nLclValue;
    }

    if (nVersion > 0x0001)
    {
        sal_Bool bNull;
        _rxInStream >> bNull;
        m_bEmptyIsNull = bNull;
    }

    if (nVersion > 0x0003)  // nVersion == 4
        _rxInStream >> m_aDefaultText;

    // Stringliste muss geleert werden, wenn eine Listenquelle gesetzt ist
    // dieses kann der Fall sein wenn im alive modus gespeichert wird
    if (m_aListSource.getLength() && m_xAggregateSet.is())
    {
        StringSequence aSequence;
        m_xAggregateSet->setPropertyValue(PROPERTY_STRINGITEMLIST, makeAny(aSequence));
    }

    if (nVersion > 0x0004)
        readHelpTextCompatibly(_rxInStream);

    if (nVersion > 0x0005)
        readCommonProperties(_rxInStream);

    // Nach dem Lesen die Defaultwerte anzeigen
    if (m_aControlSource.getLength())
    {
        // (not if we don't have a control source - the "State" property acts like it is persistent, then
        //_reset();
    }
}

//------------------------------------------------------------------------------
void OComboBoxModel::loadData()
{
    OSL_ENSURE( false, "OComboBoxModel::loadData: dead code!?" );
}

//------------------------------------------------------------------------------
void SAL_CALL OComboBoxModel::reloaded( const EventObject& /*rEvent*/ ) throw(RuntimeException)
{
    OSL_ENSURE( false, "OComboBoxModel::reloaded: dead code!?" );
}

//========================================================================
//= OComboBoxControl
//========================================================================

//------------------------------------------------------------------
InterfaceRef SAL_CALL OComboBoxControl_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory) throw (RuntimeException)
{
    return *(new OComboBoxControl(_rxFactory));
}

//------------------------------------------------------------------------------
OComboBoxControl::OComboBoxControl(const Reference<XMultiServiceFactory>& _rxFactory)
    :OBoundControl(_rxFactory, VCL_CONTROL_COMBOBOX)
{
}

//------------------------------------------------------------------------------
StringSequence SAL_CALL OComboBoxControl::getSupportedServiceNames() throw(RuntimeException)
{
    StringSequence aSupported = OBoundControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    ::rtl::OUString* pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_CONTROL_COMBOBOX;
    return aSupported;
}

//.........................................................................
}
//.........................................................................

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
