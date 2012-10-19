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

#ifndef _POREXP_HXX
#define _POREXP_HXX

#include <bf_svtools/bf_solar.h>

#include "portxt.hxx"
namespace binfilter {

/*************************************************************************
 *                      class SwExpandPortion
 *************************************************************************/

class SwExpandPortion : public SwTxtPortion
{
public:
    inline  SwExpandPortion() { SetWhichPor( POR_EXP ); }
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );
    virtual SwPosSize GetTxtSize( const SwTxtSizeInfo &rInfo ) const;

    // Accessibility: pass information about this portion to the PortionHandler

    OUTPUT_OPERATOR
};


/*************************************************************************
 *                      class SwBlankPortion
 *************************************************************************/

class SwBlankPortion : public SwExpandPortion
{
    sal_Unicode cChar;
    BOOL bMulti;        // For multiportion brackets
public:
    inline  SwBlankPortion( sal_Unicode cCh, BOOL bMult = sal_False )
        : cChar( cCh ), bMulti( bMult )
        { cChar = cCh; SetLen(1); SetWhichPor( POR_BLANK ); }

    BOOL IsMulti() const { return bMulti; }
    void SetMulti( BOOL bNew ) { bMulti = bNew; }

    virtual SwLinePortion *Compress();
    virtual sal_Bool GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const;
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );
    MSHORT MayUnderFlow( const SwTxtFormatInfo &rInf, xub_StrLen nIdx,
        sal_Bool bUnderFlow ) const;

    // Accessibility: pass information about this portion to the PortionHandler

    OUTPUT_OPERATOR
};

CLASSIO( SwExpandPortion )
CLASSIO( SwBlankPortion )


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
