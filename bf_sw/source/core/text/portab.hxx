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
#ifndef _PORTAB_HXX
#define _PORTAB_HXX

#include "porglue.hxx"
namespace binfilter {

/*************************************************************************
 *                      class SwTabPortion
 *************************************************************************/

class SwTabPortion : public SwFixPortion
{
    const KSHORT nTabPos;
    const sal_Unicode cFill;

    // Das Format() verzweigt entweder in Pre- oder PostFormat()
    sal_Bool PreFormat( SwTxtFormatInfo &rInf );
public:
    SwTabPortion( const KSHORT nTabPos, const sal_Unicode cFill = '\0' );
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );
    virtual void FormatEOL( SwTxtFormatInfo &rInf );
    sal_Bool PostFormat( SwTxtFormatInfo &rInf );
    inline  sal_Bool IsFilled() const { return 0 != cFill; }
    inline  KSHORT GetTabPos() const { return nTabPos; }

    // Accessibility: pass information about this portion to the PortionHandler

    OUTPUT_OPERATOR
};

/*************************************************************************
 *                  class SwTabLeftPortion
 *************************************************************************/

class SwTabLeftPortion : public SwTabPortion
{
public:
    inline SwTabLeftPortion( const KSHORT nTabPos1, const sal_Unicode cFill1='\0' )
         : SwTabPortion( nTabPos1, cFill1 )
    { SetWhichPor( POR_TABLEFT ); }
    OUTPUT_OPERATOR
};

/*************************************************************************
 *                  class SwTabRightPortion
 *************************************************************************/

class SwTabRightPortion : public SwTabPortion
{
public:
    inline SwTabRightPortion( const KSHORT nTabPos2, const sal_Unicode cFill2='\0' )
         : SwTabPortion( nTabPos2, cFill2 )
    { SetWhichPor( POR_TABRIGHT ); }
    OUTPUT_OPERATOR
};

/*************************************************************************
 *                  class SwTabCenterPortion
 *************************************************************************/

class SwTabCenterPortion : public SwTabPortion
{
public:
    inline SwTabCenterPortion( const KSHORT nTabPos3, const sal_Unicode cFill3='\0' )
         : SwTabPortion( nTabPos3, cFill3 )
    { SetWhichPor( POR_TABCENTER ); }
    OUTPUT_OPERATOR
};

/*************************************************************************
 *                  class SwTabDecimalPortion
 *************************************************************************/

class SwTabDecimalPortion : public SwTabPortion
{
    const sal_Unicode cTab;
public:
    inline SwTabDecimalPortion( const KSHORT nTabPos4, const sal_Unicode cInTab,
                                const sal_Unicode cFill4 = '\0' )
         : SwTabPortion( nTabPos4, cFill4 ), cTab(cInTab)
    { SetWhichPor( POR_TABDECIMAL ); }
    inline sal_Unicode GetTabDecimal() const { return cTab; }
    OUTPUT_OPERATOR
};

CLASSIO( SwTabPortion )
CLASSIO( SwTabLeftPortion )
CLASSIO( SwTabRightPortion )
CLASSIO( SwTabCenterPortion )
CLASSIO( SwTabDecimalPortion )


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
