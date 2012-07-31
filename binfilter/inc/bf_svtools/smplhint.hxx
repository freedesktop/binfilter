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
#ifndef _SFXSMPLHINT_HXX
#define _SFXSMPLHINT_HXX

#include <bf_svtools/hint.hxx>

#include <tools/rtti.hxx>

namespace binfilter
{


#define SFX_HINT_DYING              0x00000001
#define SFX_HINT_NAMECHANGED        0x00000002
#define SFX_HINT_TITLECHANGED       0x00000004
#define SFX_HINT_DATACHANGED        0x00000008
#define SFX_HINT_DOCCHANGED         0x00000010
#define SFX_HINT_UPDATEDONE         0x00000020
#define SFX_HINT_DEINITIALIZING     0x00000040
#define SFX_HINT_MODECHANGED        0x00000080
#define SFX_HINT_CANCELLABLE        0x00000100
#define SFX_HINT_DATAAVAILABLE      0x00000200
#define SFX_HINT_SAVECOMPLETED      0x00000400
#define SFX_HINT_RELEASEREF         0x00000800
#define SFX_HINT_COLORS_CHANGED     0x00001000
#define SFX_HINT_CTL_SETTINGS_CHANGED   0x00002000
#define SFX_HINT_ACCESSIBILITY_CHANGED  0x00004000
#define SFX_HINT_VIEWCREATED        0x00008000
#define SFX_HINT_USER00             0x00010000
#define SFX_HINT_USER01             0x00020000
#define SFX_HINT_USER02             0x00040000
#define SFX_HINT_USER03             0x00080000
#define SFX_HINT_USER04             0x00100000
#define SFX_HINT_USER05             0x00200000
#define SFX_HINT_USER06             0x00400000
#define SFX_HINT_USER07             0x00800000
#define SFX_HINT_USER08             0x01000000
#define SFX_HINT_USER09             0x02000000
#define SFX_HINT_USER10             0x04000000
#define SFX_HINT_USER11             0x08000000
#define SFX_HINT_USER12             0x10000000
#define SFX_HINT_USER13             0x20000000
#define SFX_HINT_UNDO_OPTIONS_CHANGED 0x40000000
#define SFX_HINT_USER_OPTIONS_CHANGED 0x80000000
#define SFX_HINT_ALL                0xFFFFFFFF

class  SfxSimpleHint: public SfxHint
{
private:
    ULONG nId;
public:
    TYPEINFO();
    SfxSimpleHint( ULONG nId );
    ULONG GetId() const { return nId; }
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
