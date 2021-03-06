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

#include <bf_svtools/inetoptions.hxx>

#include "rtl/instance.hxx"

#include <tools/urlobj.hxx>
#include <tools/wldcrd.hxx>

#include <algorithm>
#include <map>
#include <set>
#include <vector>
#include <utility>

#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/system/XProxySettings.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <osl/mutex.hxx>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <unotools/configitem.hxx>
#include <osl/diagnose.h>
#include <salhelper/refobj.hxx>
#include <rtl/logfile.hxx>
#include "itemholder1.hxx"

using namespace com::sun;

//============================================================================
//
//  takeAny
//
//============================================================================

namespace {

template< typename T > inline T takeAny(star::uno::Any const & rAny)
{
    T aValue = T();
    rAny >>= aValue;
    return aValue;
}

}

namespace binfilter
{

//============================================================================
//
//  SvtInetOptions::Impl
//
//============================================================================

class SvtInetOptions::Impl: public salhelper::ReferenceObject,
                            public utl::ConfigItem
{
public:
    enum Index
    {
        INDEX_NO_PROXY,
        INDEX_PROXY_TYPE,
        INDEX_FTP_PROXY_NAME,
        INDEX_FTP_PROXY_PORT,
        INDEX_HTTP_PROXY_NAME,
        INDEX_HTTP_PROXY_PORT
    };

    Impl();

    inline void flush() { Commit(); }

private:
    enum { ENTRY_COUNT = INDEX_HTTP_PROXY_PORT + 1 };

    struct Entry
    {
        enum State { UNKNOWN, KNOWN, MODIFIED };

        inline Entry(): m_eState(UNKNOWN) {}

        rtl::OUString m_aName;
        star::uno::Any m_aValue;
        State m_eState;
    };

    // MSVC has problems with the below Map type when
    // star::uno::Reference< star::beans::XPropertiesChangeListener > is not
    // wrapped in class Listener:
    class Listener:
        public star::uno::Reference< star::beans::XPropertiesChangeListener >
    {
    public:
        Listener(star::uno::Reference<
                         star::beans::XPropertiesChangeListener > const &
                     rListener):
            star::uno::Reference< star::beans::XPropertiesChangeListener >(
                rListener)
        {}
    };

    typedef std::map< Listener, std::set< rtl::OUString > > Map;

    osl::Mutex m_aMutex;
    Entry m_aEntries[ENTRY_COUNT];
    Map m_aListeners;

    virtual inline ~Impl() { Commit(); }

    virtual void Notify(star::uno::Sequence< rtl::OUString > const & rKeys);

    virtual void Commit();

    void notifyListeners(star::uno::Sequence< rtl::OUString > const & rKeys);
};

//============================================================================
// virtual
void
SvtInetOptions::Impl::Notify(star::uno::Sequence< rtl::OUString > const &
                                 rKeys)
{
    {
        osl::MutexGuard aGuard(m_aMutex);
        for (sal_Int32 i = 0; i < rKeys.getLength(); ++i)
            for (sal_Int32 j = 0; j < ENTRY_COUNT; ++j)
                if (rKeys[i] == m_aEntries[j].m_aName)
                {
                    m_aEntries[j].m_eState = Entry::UNKNOWN;
                    break;
                }
    }
    notifyListeners(rKeys);
}

//============================================================================
// virtual
void SvtInetOptions::Impl::Commit()
{
    star::uno::Sequence< rtl::OUString > aKeys(ENTRY_COUNT);
    star::uno::Sequence< star::uno::Any > aValues(ENTRY_COUNT);
    sal_Int32 nCount = 0;
    {
        osl::MutexGuard aGuard(m_aMutex);
        for (sal_Int32 i = 0; i < ENTRY_COUNT; ++i)
            if (m_aEntries[i].m_eState == Entry::MODIFIED)
            {
                aKeys[nCount] = m_aEntries[i].m_aName;
                aValues[nCount] = m_aEntries[i].m_aValue;
                ++nCount;
                m_aEntries[i].m_eState = Entry::KNOWN;
            }
    }
    if (nCount > 0)
    {
        aKeys.realloc(nCount);
        aValues.realloc(nCount);
        PutProperties(aKeys, aValues);
    }
}

//============================================================================
void
SvtInetOptions::Impl::notifyListeners(
    star::uno::Sequence< rtl::OUString > const & rKeys)
{
    typedef
        std::vector< std::pair< star::uno::Reference<
                                    star::beans::XPropertiesChangeListener >,
                                star::uno::Sequence<
                                    star::beans::PropertyChangeEvent > > >
    List;
    List aNotifications;
    {
        osl::MutexGuard aGuard(m_aMutex);
        aNotifications.reserve(m_aListeners.size());
        Map::const_iterator aMapEnd(m_aListeners.end());
        for (Map::const_iterator aIt(m_aListeners.begin()); aIt != aMapEnd;
             ++aIt)
        {
            const Map::mapped_type &rSet = aIt->second;
            Map::mapped_type::const_iterator aSetEnd(rSet.end());
            star::uno::Sequence< star::beans::PropertyChangeEvent >
                aEvents(rKeys.getLength());
            sal_Int32 nCount = 0;
            for (sal_Int32 i = 0; i < rKeys.getLength(); ++i)
            {
                rtl::OUString
                    aTheKey(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                              "Inet/")));
                aTheKey += rKeys[i];
                if (rSet.find(aTheKey) != aSetEnd)
                {
                    aEvents[nCount].PropertyName = aTheKey;
                    aEvents[nCount].PropertyHandle = -1;
                    ++nCount;
                }
            }
            if (nCount > 0)
            {
                aEvents.realloc(nCount);
                aNotifications.
                    push_back(std::pair< List::value_type::first_type,
                                              List::value_type::second_type >(
                                  aIt->first, aEvents));
            }
        }
    }
    for (List::size_type i = 0; i < aNotifications.size(); ++i)
        if (aNotifications[i].first.is())
            aNotifications[i].first->
                propertiesChange(aNotifications[i].second);
}

//============================================================================
SvtInetOptions::Impl::Impl():
    ConfigItem(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Inet/Settings")))
{
    m_aEntries[INDEX_NO_PROXY].m_aName
        = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ooInetNoProxy"));
    m_aEntries[INDEX_PROXY_TYPE].m_aName
        = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ooInetProxyType"));
    m_aEntries[INDEX_FTP_PROXY_NAME].m_aName
        = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ooInetFTPProxyName"));
    m_aEntries[INDEX_FTP_PROXY_PORT].m_aName
        = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ooInetFTPProxyPort"));
    m_aEntries[INDEX_HTTP_PROXY_NAME].m_aName
        = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ooInetHTTPProxyName"));
    m_aEntries[INDEX_HTTP_PROXY_PORT].m_aName
        = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ooInetHTTPProxyPort"));

    star::uno::Sequence< rtl::OUString > aKeys(ENTRY_COUNT);
    for (sal_Int32 i = 0; i < ENTRY_COUNT; ++i)
        aKeys[i] = m_aEntries[i].m_aName;
    if (!EnableNotification(aKeys))
        OSL_FAIL("SvtInetOptions::Impl::Impl(): Bad EnableNotifications()");
}

//============================================================================
//
//  SvtInetOptions
//
//============================================================================

namespace
{
    class LocalSingleton : public rtl::Static< osl::Mutex, LocalSingleton >
    {
    };
}

// static
SvtInetOptions::Impl * SvtInetOptions::m_pImpl = 0;

//============================================================================
SvtInetOptions::SvtInetOptions()
{
    osl::MutexGuard aGuard(LocalSingleton::get());
    if (!m_pImpl)
    {
        RTL_LOGFILE_CONTEXT(aLog, "svtools ( ??? ) ::SvtInetOptions_Impl::ctor()");
        m_pImpl = new Impl;

        ItemHolder1::holdConfigItem(E_INETOPTIONS);
    }
    m_pImpl->acquire();
}

//============================================================================
SvtInetOptions::~SvtInetOptions()
{
    osl::MutexGuard aGuard(LocalSingleton::get());
    if (m_pImpl->release() == 0)
        m_pImpl = 0;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
