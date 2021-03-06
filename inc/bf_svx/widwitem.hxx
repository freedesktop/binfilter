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
#ifndef _SVX_WIDWITEM_HXX
#define _SVX_WIDWITEM_HXX

#include <bf_svtools/intitem.hxx>

#include <bf_svx/svxids.hrc>

namespace rtl
{
    class OUString;
}
namespace binfilter {
class SvXMLUnitConverter;

// class SvxWidowsItem ---------------------------------------------------

/*
[Beschreibung]
Dieses Item beschreibt die Anzahl der Zeilen fuer die Hurenkinderregelung.
*/

class SvxWidowsItem: public SfxByteItem
{
    friend SvStream & operator<<( SvStream & aS, SvxWidowsItem & );
public:
    TYPEINFO();

    SvxWidowsItem( const BYTE nL=0, const USHORT nId = ITEMID_WIDOWS );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create( SvStream &, USHORT ) const;


    inline SvxWidowsItem& operator=( const SvxWidowsItem& rWidows )
    {
        SetValue( rWidows.GetValue() );
        return *this;
    }
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
