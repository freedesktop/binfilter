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

#include "fmdmod.hxx"

#include "fmservs.hxx"
#include <fmobj.hxx>
#include <unoshape.hxx>
#include "fmglob.hxx"
#include <legacysmgr/legacy_binfilters_smgr.hxx>
namespace binfilter {

using namespace ::binfilter::svxform;

//-----------------------------------------------------------------------------
/*N*/ ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SAL_CALL SvxFmMSFactory::createInstance(const ::rtl::OUString& ServiceSpecifier) throw( ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException )
/*N*/ {
/*N*/   ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  xRet;
/*N*/   if ( ServiceSpecifier.indexOf( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.component." )) ) == 0 )
/*N*/   {
/*?*/       xRet = ::legacy_binfilters::getLegacyProcessServiceFactory()->createInstance(ServiceSpecifier);
/*N*/   }
/*N*/   else if ( ServiceSpecifier == ::rtl::OUString( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.ControlShape" )) ) )
/*N*/   {
/*N*/       SdrObject* pObj = new FmFormObj(OBJ_FM_CONTROL);
/*N*/       xRet = *new SvxShapeControl(pObj);
/*N*/   }
/*N*/   if (!xRet.is())
/*N*/       xRet = SvxUnoDrawMSFactory::createInstance(ServiceSpecifier);
/*N*/   return xRet;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
