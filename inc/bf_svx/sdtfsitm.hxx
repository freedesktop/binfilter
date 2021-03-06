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
#ifndef _SDTFSITM_HXX
#define _SDTFSITM_HXX

#include <bf_svtools/eitem.hxx>

#include <bf_svx/svddef.hxx>
namespace binfilter {

enum SdrFitToSizeType {SDRTEXTFIT_NONE,         // - kein FitToSize
                       SDRTEXTFIT_PROPORTIONAL, // - Alle Buchstaben proportional umgroessern
                       SDRTEXTFIT_ALLLINES,     // - Zus. jede Zeile separat in der Breite stretchen
                       SDRTEXTFIT_RESIZEATTR};  // - Bei Rahmenumgroesserung (ausser Autogrow) wird
                                                //   die Schriftgroesse umattributiert (hart)

// Bei SDRTEXTFIT_PROPORTIONAL und SDRTEXTFIT_ALLLINES gibt es kein AutoGrow und
// keine automatischen Umbrueche.
// Ist SDRTEXTFIT_RESIZEATTR gesetzt, so wird beim umgroessern des Textrahmens
// (ausser bei AutoGrow) die Schrift durch harte Attributierung ebenfalls
// umgegroessert.
// Bei AutoGrowingWidth gibt es ebenfalls keine automatischen Umbrueche (erst bei
// TextMaxFrameWidth).

//--------------------------------
// class SdrTextFitToSizeTypeItem
//--------------------------------
class SdrTextFitToSizeTypeItem: public SfxEnumItem {
public:
    TYPEINFO();
    SdrTextFitToSizeTypeItem(SdrFitToSizeType eFit=SDRTEXTFIT_NONE): SfxEnumItem(SDRATTR_TEXT_FITTOSIZE,eFit) {}
    SdrTextFitToSizeTypeItem(SvStream& rIn)                        : SfxEnumItem(SDRATTR_TEXT_FITTOSIZE,rIn)  {}
    virtual SfxPoolItem*     Clone(SfxItemPool* pPool=NULL) const;
    virtual SfxPoolItem*     Create(SvStream& rIn, USHORT nVer) const;
    virtual USHORT           GetValueCount() const; // { return 4; }
            SdrFitToSizeType GetValue() const      { return (SdrFitToSizeType)SfxEnumItem::GetValue(); }

    virtual bool             QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual bool             PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
