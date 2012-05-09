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
#ifndef _VIEWIMP_HXX
#define _VIEWIMP_HXX

#include <bf_svtools/bf_solar.h>


#include <vcl/timer.hxx>
#include <tools/color.hxx>
#include <bf_svx/svdtypes.hxx>

#include "swtypes.hxx"
#include "swrect.hxx"
#include <tools/debug.hxx> //for strip
#ifdef ACCESSIBLE_LAYOUT
class Fraction;
#endif
namespace binfilter {
class ViewShell;
class SwFlyFrm;
class SwViewOption;
class SwScrollAreas;
class SwScrollColumn;
class SwFrm;
class SwDrawView;
class SdrPageView;
class SwPageFrm;
class ExtOutputDevice;
class SdrPaintInfoRec;
#ifdef ACCESSIBLE_LAYOUT
class SwAccessibleMap;
class SdrObject;
#endif
class SwPagePreviewLayout;
} //namespace binfilter

#include <prevwpage.hxx>
#include <vector>
namespace binfilter {
class SwViewImp
{
    friend class ViewShell;

    // for paint of page preview
    friend class SwPagePreviewLayout;

    ViewShell *pSh;             //Falls jemand einen Imp durchreicht und doch
                                //mal eine ViewShell braucht hier die
                                //Rueckwaertsverkettung.

    SdrPageView *pSdrPageView;  //Genau eine Seite fuer unsere DrawView

    BOOL bFirstPageInvalid  :1; //Pointer auf erste Seite ungueltig?
    BOOL bNextScroll        :1; //Scroll in der folgenden EndAction erlaubt?
    BOOL bScroll            :1; //Scroll in der aktuellen EndAction erlaubt?
    BOOL bScrolled          :1; //Wurde gescrolled? Dann im Idle aufraeumen.

    BOOL bResetXorVisibility:1; //StartAction/EndAction
    BOOL bShowHdlPaint      :1; //LockPaint/UnlockPaint
    BOOL bResetHdlHiddenPaint:1;//  -- "" --
    BOOL bPaintInScroll     :1; //Paint (Update() im ScrollHdl der ViewShell

    BOOL bSmoothUpdate      :1; //Meber fuer SmoothScroll
    BOOL bStopSmooth        :1;
    BOOL bStopPrt           :1; // Stop Printing

    USHORT nRestoreActions  ; //Die Anzahl der zu restaurierenden Actions (UNO)
    SwRect aSmoothRect;


    /**
        Signal whether to stop printing.

        @param _useless just to fit macro
    */
    DECL_LINK(SetStopPrt, void * _useless = NULL);
public:
    ~SwViewImp();

    const ViewShell *GetShell() const { return pSh; }
          ViewShell *GetShell()       { return pSh; }

    void SetFirstVisPageInvalid() { bFirstPageInvalid = TRUE; }

    //Wird vom Layout ggf. waehrend einer Action gerufen, wenn der
    //Verdacht besteht, dass es etwas drunter und drueber geht.
    void ResetScroll()        { bScroll = FALSE; }

    // neues Interface fuer StarView Drawing
          SdrPageView*GetPageView()       { return pSdrPageView; }
    const SdrPageView*GetPageView() const { return pSdrPageView; }
};
} //namespace binfilter
#endif //_VIEWIMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
