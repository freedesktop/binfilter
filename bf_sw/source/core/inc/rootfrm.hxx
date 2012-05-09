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
#ifndef _ROOTFRM_HXX
#define _ROOTFRM_HXX

#include <bf_svtools/bf_solar.h>

#include "layfrm.hxx"
#include "frmfmt.hxx"
#include "doc.hxx"
class OutputDevice;
namespace binfilter {

class SwCntntFrm;
class ViewShell;
class SdrPage;
class SwPaM;
class SwCursor;
class SwShellCrsr;
class SwTableCursor;

class SwLayVout;
class SwDestroyList;
struct SwPosition;
struct SwCrsrMoveState;

#define HACK_TABLEMODE_INIT         0
#define HACK_TABLEMODE_LOCKLINES    1
#define HACK_TABLEMODE_PAINTLINES   2
#define HACK_TABLEMODE_UNLOCKLINES  3
#define HACK_TABLEMODE_EXIT         4

#define INV_SIZE    1
#define INV_PRTAREA 2
#define INV_POS     4
#define INV_TABLE   8
#define INV_SECTION 16
#define INV_LINENUM 32
#define INV_DIRECTION 64

class SwRootFrm : public SwLayoutFrm
{
    // Fuer das Anlegen und Zerstoeren des virtuellen Outputdevice-Managers
    friend void _FrmInit();     //erzeugt pVout
    friend void _FrmFinit();    //loescht pVout

    static SwLayVout     *pVout;
    static BOOL           bInPaint;     //Schutz gegen doppelte Paints.
    static BOOL           bNoVirDev;    //Bei SystemPaints kein virt. Device

public:
    SwRootFrm( SwFrmFmt *pFmt )
      : SwLayoutFrm( pFmt->GetDoc()->MakeFrmFmt(
          XubString( "Root", RTL_TEXTENCODING_MS_1252 ), pFmt ) ) {}
    static BOOL IsInPaint() { return bInPaint; }
    static void SetNoVirDev( const BOOL bNew ) { bNoVirDev = bNew; }
};

} //namespace binfilter
#endif  //_ROOTFRM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
