/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 Red Hat, Inc., Caolán McNamara <caolanm@redhat.com>
 *  (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <unotest/filters-test.hxx>
#include <test/bootstrapfixture.hxx>
#include <com/sun/star/document/XFilter.hpp>

#include <osl/file.hxx>
#include <osl/process.h>

using namespace ::com::sun::star;

namespace
{
    class BinFilterTest
        : public test::FiltersTest
        , public test::BootstrapFixture
    {
    public:
        BinFilterTest() : BootstrapFixture(true, true) {}

        virtual void setUp();

        virtual bool load(const rtl::OUString &,
            const rtl::OUString &rURL, const rtl::OUString &);

        void test();

        CPPUNIT_TEST_SUITE(BinFilterTest);
        CPPUNIT_TEST(test);
        CPPUNIT_TEST_SUITE_END();
    private:
        uno::Reference<document::XFilter> m_xFilter;
    };

    void BinFilterTest::setUp()
    {
        test::BootstrapFixture::setUp();

        m_xFilter = uno::Reference< document::XFilter >(m_xSFactory->createInstance(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.comp.office.BF_MigrateFilter"))),
            uno::UNO_QUERY_THROW);
    }

    bool BinFilterTest::load(const rtl::OUString &rFilter,
        const rtl::OUString &rURL, const rtl::OUString &rUserData)
    {
        uno::Sequence< beans::PropertyValue > aDescriptor(3);
        aDescriptor[0].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("URL"));
        aDescriptor[0].Value <<= rURL;
        aDescriptor[1].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UserData"));
        uno::Sequence< rtl::OUString > aUserData(1);
        aUserData[0] = rUserData;
        aDescriptor[1].Value <<= aUserData;
        aDescriptor[2].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FilterName"));
        aDescriptor[2].Value <<= rFilter;
        return m_xFilter->filter(aDescriptor);
    }

    void BinFilterTest::test()
    {
        testDir(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StarWriter 5.0")),
            getURLFromSrc("/binfilter/qa/cppunit/data/sdw/"),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CSW5")));
    }

    CPPUNIT_TEST_SUITE_REGISTRATION(BinFilterTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
