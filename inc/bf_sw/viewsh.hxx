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
#ifndef _VIEWSH_HXX
#define _VIEWSH_HXX

#include <bf_svtools/bf_solar.h>

#include <tools/rtti.hxx>

#include <ring.hxx>
#include <swrect.hxx>
/*N*/ #include <tools/debug.hxx> //for stripping
#ifdef ACCESSIBLE_LAYOUT
namespace com { namespace sun { namespace star { namespace accessibility {
           class XAccessible; } } } }
#endif
class VirtualDevice;
class Window;
class OutputDevice;
class SvtAccessibilityOptions;
class Fraction;
class SvEmbeddedObjectRef;

namespace binfilter {

class SfxItemPool;
class SwDoc;
class SfxPrinter;

class SfxProgress;
class SwNodes;
class SdrView;

class SfxViewShell;
class SwViewOption;
class SwViewImp;
class SwPrtOptions;
class SwPagePreViewPrtData;
struct ShellResource;
class SwFrm;
struct SwPrintData;
class SwPagePreviewLayout;

struct SwAccessibilityOptions;

class ViewShell : public Ring
{
    friend class SwViewImp;

    // OD 12.12.2002 #103492# - for setting visible area for page preview paint
    friend class SwPagePreviewLayout;

    Point         aPrtOffst;         //Ofst fuer den Printer,
                                     //nicht bedruckbarer Rand.
     Size         aBrowseBorder;    //Rand fuer Framedokumente
    SwRect        aInvalidRect;

    SfxViewShell *pSfxViewShell;
    SwViewImp    *pImp;             //Core-Interna der ViewShell.
                                    //Der Pointer ist niemals 0.

    Window       *pWin;              // = 0 during printing or pdf export
    OutputDevice *pOut;              // Window, Printer, VirtDev, ...
    OutputDevice* mpTmpRef;           // Temporariy reference device. Is used
                                     // during (printer depending) prospect
                                     // and page preview printing
                                     // (because a scaling has to be set at
                                     // the original printer)

    SwViewOption *pOpt;
    SwAccessibilityOptions* pAccOptions;


    sal_Bool  bDocSizeChgd     :1;  //Fuer DocChgNotify(): Neue DocGroesse bei
                                //EndAction an das DocMDI melden.
    sal_Bool  bPaintWorks      :1;  //Normal Painten wenn sal_True,
                                //Paint merken wenn sal_False
    sal_Bool  bPaintInProgress :1;  //Kein zweifaches Paint durchlassen.
    sal_Bool  bViewLocked      :1;  //Lockt den sichtbaren Bereich,
                                //MakeVisible laeuft dann in's leere.
    sal_Bool  bInEndAction     :1;  //Fiese unstaende vermeiden, siehe viewsh.cxx
    sal_Bool  bPreView         :1;  //Ist sal_True wenns eine PreView-ViewShell ist.
    sal_Bool  bFrameView       :1;  //sal_True wenn es ein (HTML-)Frame ist.
    sal_Bool  bEnableSmooth    :1;  //Disable des SmoothScroll z.B. fuer
                                //Drag der Scrollbars.
    sal_Bool  bEndActionByVirDev:1; //Paints aus der EndAction immer ueber virtuelles

                                               // Malen der Wiese und rufen
    // PaintDesktop gesplittet, dieser Teil wird auch von PreViewPage benutzt

    sal_Bool CheckInvalidForPaint( const SwRect & );//Direkt Paint oder lieber
                                                //eine Aktion ausloesen.
protected:
    static ShellResource*   pShellRes;      // Resourcen fuer die Shell
    static Window*          pCareWindow;    // diesem Fenster ausweichen

    SwRect                  aVisArea;       //Die moderne Ausfuerung der VisArea
    SwDoc                   *pDoc;          //Das Dokument, niemals 0

    sal_uInt16 nStartAction; //ist != 0 wenn mindestens eine ::com::sun::star::chaos::Action laeuft
    sal_uInt16 nLockPaint;   //ist != 0 wenn das Paint gelocked ist.

public:
    TYPEINFO();

          SwViewImp *Imp() { return pImp; }
    const SwViewImp *Imp() const { return pImp; }

    const SwRect &VisArea() const { return aVisArea; }

    inline SwDoc *GetDoc()  const { return pDoc; }  //niemals 0.

    // 1. GetPrt:      The printer at the document
    // 3. GetRefDev:   Either the printer or the virtual device from the doc
    // 2. GetWin:      Available if we not printing
    // 4. GetOut:      Printer, Window or Virtual device
    VirtualDevice* GetVirDev( sal_Bool bCreate = sal_False ) const;
    OutputDevice& GetRefDev() const;
    inline Window* GetWin()    const { return pWin; }
    inline OutputDevice* GetOut()     const { return pOut; }

    // Setzt Drucker fuer ALLE Sichten im Ring; einschl. Invalidierungen
    void SetVirDev( VirtualDevice* );

    //Andern alle PageDescriptoren

    // creates temporary doc with selected text for PDF export
    SwDoc * CreatePrtDoc( SfxPrinter* pPrt, SvEmbeddedObjectRef &rDocShellRef );

    //Wird intern fuer die Shell gerufen die Druckt. Formatiert die Seiten.

    //All about fields.
    // update all charts, for that exists any table

    // Sollen Absatzabstaende addiert oder maximiert werden?

    // compatible behaviour of tabs

    //formatting by virtual device or printer
    sal_Bool IsUseVirtualDevice()const;
    void SetUseVirtualDevice(sal_Bool bSet);

    inline const SwViewOption *GetViewOptions() const { return pOpt; }

    //static void           SetShellRes( ShellResource* pRes ) { pShellRes = pRes; }
    static ShellResource* GetShellRes();

    // Selektion der Draw ::com::sun::star::script::Engine geaendert
    virtual void DrawSelChanged(SdrView*);

    // OD 12.12.2002 #103492#
    SwPagePreviewLayout* PagePreviewLayout();

    // print page/print preview
    void PrintPreViewPage( SwPrtOptions& rOptions, sal_uInt16 nRowCol,
                           SfxProgress& rProgress,
                           const SwPagePreViewPrtData* = 0 );

    //sorge dafuer, das auf jedenfall die MarkListe aktuell ist (Bug 57153)

    sal_Bool IsPreView() const { return bPreView; }

    //Damit in der UI nicht ueberall das dochxx includet werden muss
    sal_Bool IsBrowseMode() const;

    const Size& GetBrowseBorder() const{ return aBrowseBorder; }

    virtual ~ViewShell();

};

//---- class CurrShell verwaltet den globalen ShellPointer -------------------

class CurrShell
{
public:
    ViewShell *pPrev;

    CurrShell( ViewShell *pNew );
    ~CurrShell();
};

} //namespace binfilter
#endif //_VIEWSH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
