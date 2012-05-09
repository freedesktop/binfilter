/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef _LEGACYBINFILTERSMGR_HXX
#define _LEGACYBINFILTERSMGR_HXX

#include "sal/config.h"

#include "com/sun/star/uno/Reference.hxx"
#include "sal/types.h"

namespace com { namespace sun { namespace star {
    namespace lang { class XMultiServiceFactory; }
    namespace registry { class XRegistryKey; }
} } }

namespace legacy_binfilters
{

::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > const & SAL_CALL getLegacyProcessServiceFactory();

} //namespace legacy_binfilters

extern "C" void * SAL_CALL legacysmgr_component_getFactory(
    char const * implName, com::sun::star::lang::XMultiServiceFactory * smgr,
    com::sun::star::registry::XRegistryKey * key);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
