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
#ifndef _SXSALITM_HXX
#define _SXSALITM_HXX

#include <bf_svx/svddef.hxx>

#include <bf_svx/sdangitm.hxx>
namespace binfilter {

//------------------------------
// class SdrHorzShearAllItem
//------------------------------
class SdrHorzShearAllItem: public SdrAngleItem {
public:
    SdrHorzShearAllItem(long nAngle=0): SdrAngleItem(SDRATTR_HORZSHEARALL,nAngle) {}
    SdrHorzShearAllItem(SvStream& rIn): SdrAngleItem(SDRATTR_HORZSHEARALL,rIn)    {}
};

//------------------------------
// class SdrVertShearAllItem
//------------------------------
class SdrVertShearAllItem: public SdrAngleItem {
public:
    SdrVertShearAllItem(long nAngle=0): SdrAngleItem(SDRATTR_VERTSHEARALL,nAngle) {}
    SdrVertShearAllItem(SvStream& rIn): SdrAngleItem(SDRATTR_VERTSHEARALL,rIn)    {}
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
