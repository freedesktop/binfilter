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

#ifndef _SB_SBPROPERTY_HXX
#define _SB_SBPROPERTY_HXX

#include <bf_svtools/bf_solar.h>

#include "sbxprop.hxx"
#include "sbdef.hxx"

namespace binfilter {

class SbModule;

class SbProperty : public SbxProperty
{
    friend class SbiFactory;
    friend class SbModule;
    friend class SbProcedureProperty;
    SbModule* pMod;
    BOOL     bInvalid;
    SbProperty( const String&, SbxDataType, SbModule* );
    virtual ~SbProperty();
public:
    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_BASICPROP,1);
    TYPEINFO();
    SbModule* GetModule() { return pMod; }
};

#ifndef __SB_SBPROPERTYREF_HXX
#define __SB_SBPROPERTYREF_HXX
SV_DECL_IMPL_REF(SbProperty)
#endif


class SbProcedureProperty : public SbxProperty
{
    bool mbSet;     // Flag for set command

    virtual ~SbProcedureProperty();

public:
    SbProcedureProperty( const String& r, SbxDataType t )
            : SbxProperty( r, t ) // , pMod( p )
            , mbSet( false )
    {}
    TYPEINFO();

    bool isSet( void )
        { return mbSet; }
    void setSet( bool bSet )
        { mbSet = bSet; }
};

#ifndef __SB_SBPROCEDUREPROPERTYREF_HXX
#define __SB_SBPROCEDUREPROPERTYREF_HXX
SV_DECL_IMPL_REF(SbProcedureProperty)
#endif

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
