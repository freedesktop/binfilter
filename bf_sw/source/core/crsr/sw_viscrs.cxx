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

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_USHORTS
#endif


#include <viewopt.hxx>
#include <frmtool.hxx>
#include <viscrs.hxx>

#include <horiornt.hxx>

#include <crsrsh.hxx>
#include <dview.hxx>
#include <txtfrm.hxx>   // SwTxtFrm
#include <ndtxt.hxx>
#include <drawfont.hxx>

#include <comcore.hrc>          // ResId fuer Abfrage wenn zu Search & Replaces
namespace binfilter {

extern void SwCalcPixStatics( OutputDevice *pOut );


//Damit beim ShowCrsr nicht immer wieder die gleiche Size teuer ermittelt
//werden muss, hier statische Member, die beim Wechsel des MapModes
// angepasst werden

long SwSelPaintRects::nPixPtX = 0;
long SwSelPaintRects::nPixPtY = 0;
MapMode* SwSelPaintRects::pMapMode = 0;


#ifdef SHOW_BOOKMARKS
#define SHOWBOOKMARKS1( nAct )          ShowBookmarks( GetShell(),nAct );
#else
#define SHOWBOOKMARKS1( nAct )
#endif

#ifdef SHOW_REDLINES
#define SHOWREDLINES1( nAct )           ShowRedlines( GetShell(),nAct );
#else
#define SHOWREDLINES1( nAct )
#endif

/*N*/ SwVisCrsr::~SwVisCrsr()
/*N*/ {
/*N*/ #ifdef SW_CRSR_TIMER
/*N*/   if( bTimerOn )
/*N*/       Stop();     // Timer stoppen
/*N*/ #endif
/*N*/
/*N*/   if( bIsVisible && aTxtCrsr.IsVisible() )
/*?*/       aTxtCrsr.Hide();
/*N*/
/*N*/   pCrsrShell->GetWin()->SetCursor( 0 );
/*N*/ }

// ------ Ab hier Klassen / Methoden fuer die Selectionen -------

/*N*/ SwSelPaintRects::~SwSelPaintRects()
/*N*/ {
/*N*/   Hide();
/*N*/ }

/*N*/ void SwSelPaintRects::Hide()
/*N*/ {
/*N*/   SwRects::Remove( 0, Count() );
/*N*/ }

/*N*/ SwShellCrsr::~SwShellCrsr() {}

/*N*/ SwShellCrsr::operator SwShellCrsr* () { return this; }

/*N*/ SwShellTableCrsr::~SwShellTableCrsr() {}

/*N*/ void SwShellTableCrsr::SetMark()              { SwShellCrsr::SetMark(); }
/*N*/ SwShellTableCrsr::operator SwShellCrsr* ()        { return this; }
/*N*/ SwShellTableCrsr::operator SwTableCursor* ()  { return this; }
/*N*/ SwShellTableCrsr::operator SwShellTableCrsr* ()   { return this; }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
