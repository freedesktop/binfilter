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

#ifndef _SVDPNTV_HXX
#define _SVDPNTV_HXX

#include <bf_svtools/bf_solar.h>
#include <list>

#include <vcl/timer.hxx>
#include <vcl/virdev.hxx>
#include <bf_svtools/brdcst.hxx>
#include <bf_svtools/lstner.hxx>
#include <bf_svtools/smplhint.hxx>
#include <bf_svtools/itemset.hxx>
#include <bf_svtools/undo.hxx>
#include <bf_svx/svdlayer.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <com/sun/star/uno/Reference.h>
#include <bf_svtools/colorcfg.hxx>


//************************************************************
//   Pre-Defines
//************************************************************

namespace com { namespace sun { namespace star { namespace awt {
class XControlContainer;
}}}}
class OutputDevice;
class VirtualDevice;
namespace binfilter {
class SfxItemSet;
class SfxStyleSheet;
class SdrGrafObj;
class SdrPage;
class SdrPageViewWinRec;
class SdrView;
class SdrViewUserMarker;
class ExtOutputDevice;
class SdrNamedSubRecord;
class SdrIOHeader;
class SdrOle2Obj;
class ImpMasterBmp;
class SdrObject;
class SdrPageView;

//************************************************************
//   Defines for cache modes
//************************************************************

#define SDR_MASTERPAGECACHE_NONE        0x00000000UL
#define SDR_MASTERPAGECACHE_FULL        0x00000001UL
#define SDR_MASTERPAGECACHE_BACKGROUND  0x00000002UL
#define SDR_MASTERPAGECACHE_DEFAULT     0xFFFFFFFFUL

//************************************************************
//   Defines for AnimationMode
//************************************************************

enum SdrAnimationMode
{
    SDR_ANIMATION_ANIMATE,
    SDR_ANIMATION_DONT_ANIMATE,
    SDR_ANIMATION_DISABLE
};

//************************************************************
//   Typedef's und defines
//************************************************************

typedef unsigned char TRISTATE;
#define FUZZY                   (2)
#define SDR_ANYFORMAT           (0xFFFFFFFF)
#define SDR_ANYITEM             (0xFFFF)
#define SDRVIEWWIN_NOTFOUND     (0xFFFF)

//************************************************************
//   Hilfsklasse RollingRect
//************************************************************

class RollingRect
{
    Point                       aP1;
    Point                       aP2;

    USHORT                      nIdx;
    USHORT                      nLen;

    unsigned                    bStripes : 1;
    unsigned                    bCrossHair : 1;

public:
    RollingRect(USHORT nNewLen=8) { Reset(nNewLen); }
    void Reset(USHORT nNewLen=8) { nIdx=0; nLen=nNewLen; bStripes=FALSE; bCrossHair=FALSE; }
    void IncRollIdx() { nIdx++; if (nIdx>=2*nLen) nIdx=0; }
    const Point& GetP1() const { return aP1; }
    const Point& GetP2() const { return aP2; }
    void SetP1(const Point& rP) const { ((RollingRect*)this)->aP1=rP; }
    void SetP2(const Point& rP) const { ((RollingRect*)this)->aP2=rP; }
    BOOL IsCrossHair() const { return bCrossHair; }
    BOOL IsStripes() const { return bStripes; }
    void SetCrossHair(BOOL bOn) { bCrossHair=bOn; }
    void SetStripes(BOOL bOn) { bStripes=bOn; }
};

//************************************************************
//   Hilfsklasse FrameAnimator
//************************************************************

class FrameAnimator: public RollingRect
{
    OutputDevice*               pOut;
    SdrView&                    rView;
    AutoTimer                   aTim;

public:
    FrameAnimator(SdrView& rNewView);
    DECL_LINK(Hdl,AutoTimer*);
};

//************************************************************
//   Hilfsklasse SdrViewWinRec
//************************************************************

class SdrViewWinRec
{
public:
    OutputDevice*               pWin;
    unsigned                    bXorVisible : 1;

    SdrViewWinRec(OutputDevice* pW);
    ~SdrViewWinRec();
};

//************************************************************
//   Hilfsklasse SdrViewWinList
//************************************************************

class SdrViewWinList
{
    Container                   aList;

    SdrViewWinList(const SdrViewWinList& /*rSource*/)
    :   aList(1024,4,4) {}
    void operator=(const SdrViewWinList& /*rSource*/) {}

protected:
    SdrViewWinRec* GetObject(USHORT i) const { return (SdrViewWinRec*)(aList.GetObject(i)); }

public:
    SdrViewWinList(): aList(1024,4,4) {}
    ~SdrViewWinList() { Clear(); }
    void Clear();
    USHORT GetCount() const { return (USHORT)aList.Count(); }
    void Insert(SdrViewWinRec* pVWR, USHORT nPos=0xFFFF) { aList.Insert(pVWR,nPos); }
    void Delete(USHORT nPos) { delete (SdrViewWinRec*)aList.Remove(nPos); }
    SdrViewWinRec& operator[](USHORT nPos) { return *GetObject(nPos); }
    const SdrViewWinRec& operator[](USHORT nPos) const { return *GetObject(nPos); }
    USHORT Find(OutputDevice* pW) const;
};

#define SVX_HINT_VIEWCHANGED        1

class SvxViewHint : public SfxSimpleHint
{
public:
            TYPEINFO();
            SvxViewHint( ULONG nId );
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// PAINTVIEW
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class SdrPaintView: public SfxListener, public SfxRepeatTarget, public SfxBroadcaster
{
    friend class                SdrPageView;
    friend class                FrameAnimator;
    friend class                SdrViewUserMarker;
    friend class                SdrGrafObj;

protected:
    SdrModel*                   pMod;
    ExtOutputDevice*            pXOut;
    SfxStyleSheet*              pDefaultStyleSheet;

    String                      aAktLayer;     // Aktueller Zeichenlayer
    String                      aMeasureLayer; // Aktueller Layer fuer Bemassung

    Container                   aPagV;         // Liste von SdrPageViews
    Container                   aPagHide;      // Liste von nicht angezeigten SdrPageViews
    SdrViewWinList              aWinList;    // Alle Fenster, in der die View angezeigt ist
    Size                        aGridBig; // muss dann mal raus
    Size                        aGridFin; // muss dann mal raus
    Size                        aGridSubdiv;
    Fraction                    aGridWdtX;
    Fraction                    aGridWdtY;
    FrameAnimator               aAni;
    SfxItemSet                  aDefaultAttr;
    Timer                       aComeBackTimer;
    Timer                       aAfterPaintTimer;
    AutoTimer                   aUserMarkerAnimator;
    Container                   aUserMarkers;

    SdrAnimationMode            eAnimationMode;

    ULONG                       nGraphicManagerDrawMode;

    unsigned                    bLayerSortedRedraw : 1;
    unsigned                    bPageVisible : 1;
    unsigned                    bBordVisible : 1;
    unsigned                    bGridVisible : 1;
    unsigned                    bGridFront : 1;
    unsigned                    bHlplVisible : 1;
    unsigned                    bHlplFront : 1;
    unsigned                    bGlueVisible : 1;    // Persistent. Klebepunkte anzeigen
    unsigned                    bLineDraft : 1;
    unsigned                    bFillDraft : 1;
    unsigned                    bTextDraft : 1;
    unsigned                    bGrafDraft : 1;
    unsigned                    bSomeObjChgdFlag : 1;
    unsigned                    bMasterBmp : 1;         // MasterPages als Bitmap merken?
    unsigned                    bSwapAsynchron : 1;
    unsigned                    bPrintPreview : 1;

    ColorConfig                 maColorConfig;
    Color                       maGridColor;

private:
    DECL_LINK(ImpComeBackHdl,Timer*);
    DECL_LINK(ImpAfterPaintHdl,Timer*);
    DECL_LINK(ImpUserMarkerAnimatorHdl,AutoTimer*);

    void ImpClearVars();
    void ImpInsertUserMarker(SdrViewUserMarker* pMarker);
    void ImpRemoveUserMarker(SdrViewUserMarker* pMarker);
    void ImpCheckMarkerAnimator();
    USHORT ImpGetUserMarkerCount() const { return (USHORT)aUserMarkers.Count(); }
    SdrViewUserMarker* ImpGetUserMarker(USHORT nNum) const { return (SdrViewUserMarker*)aUserMarkers.GetObject(nNum); }

protected:
    virtual void SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType);
    USHORT GetHiddenPV(const SdrPage* pPage) const;


    virtual void WriteRecords(SvStream& rOut) const;
    virtual BOOL ReadRecord(const SdrIOHeader& rViewHead, const SdrNamedSubRecord& rSubHead, SvStream& rIn);
    // ModelHasChanged wird gerufen, sobald nach beliebig vielen HINT_OBJCHG
    // das System wieder idle ist (StarView-Timer). Wer diese Methode ueberlaed,
    // muss unbedingt ModelHasChanged() der Basisklasse rufen.
    virtual void ModelHasChanged();

public:
    TYPEINFO();
    SdrPaintView(SdrModel* pModel1, OutputDevice* pOut=NULL);
    virtual ~SdrPaintView();

    virtual void ClearPageViews();
    virtual void ClearHideViews();
    virtual void ClearAll();
    virtual void Clear(); // PageViews loeschen, Markierungen weg, ...
    SdrModel* GetModel() const { return pMod; }

    virtual BOOL IsAction() const { return FALSE; } // DBG_BF_ASSERT
    virtual void BrkAction() {} // DBG_BF_ASSERT Alle Actions z.B. Draggen abbrechen.

    virtual void ToggleShownXor(OutputDevice*, const Region*) const {} // DBG_BF_ASSERT

    // Anmelden/Abmelden einer PageView an der View.
    // Dieselbe // Seite kann nicht mehrfach angemeldet werden.
    // Methoden mit dem Suffix PgNum erwarten als numerischen Parameter
    // eine Seitennummer (0...). Methoden mit dem Suffix PvNum erwarten
    // degagen als numerischen Parameter die Nummer der PageView an der
    // SdrView (Iterieren ueber alle angemeldeten Pages).
    virtual SdrPageView* ShowPage(SdrPage* pPage, const Point& rOffs);
    virtual void HidePage(SdrPageView* pPV);
    void HidePage(const SdrPage* pPage) { HidePage(GetPageView(pPage)); }
    void HidePagePvNum(USHORT nPvNum) { HidePage(GetPageViewPvNum(nPvNum)); }

    // Iterieren ueber alle angemeldeten PageViews
    USHORT GetPageViewCount() const { return USHORT(aPagV.Count()); }
    SdrPageView* GetPageViewPvNum(USHORT nPvNum) const { return ((SdrPageView*)aPagV.GetObject(nPvNum)); }

    // Pageview einer bestimmten Seite ermitteln
    SdrPageView* GetPageView(const SdrPage* pPage) const;

    // Test, ob eine Seite getroffen

    // Die Seite, die dem Punkt am naechsten ist. Liefert nur NULL,
    // wenn absolut keine Seite angemeldet ist.

    // Falls noetig kann man sich auch die nicht angezeigten Seiten rausholen
    USHORT GetPageHideCount() const { return USHORT(aPagHide.Count()); }
    SdrPageView* GetPageHidePvNum(USHORT nPvNum) const { return ((SdrPageView*)aPagHide.GetObject(nPvNum)); }
    SdrPageView* GetPageHide(const SdrPage* pPage) const { return ((SdrPageView*)aPagHide.GetObject(GetHiddenPV(pPage))); }

    // Eine SdrView kann auf mehreren Fenstern gleichzeitig abgebiltet sein:
    virtual void AddWin(OutputDevice* pWin1);
    virtual void DelWin(OutputDevice* pWin1);

    USHORT GetWinCount() const { return aWinList.GetCount(); }
    OutputDevice* GetWin(USHORT nNum) const { return nNum<aWinList.GetCount() ? aWinList[nNum].pWin : NULL; }

    // Einen Satz von Layern hinzu-sichtbarmachen.
    // bShow=FALSE werden alle Memberlayer des Set ausgeblendet.
    // Bei True dagegen werden alle MemberLayer ein- und alle Excluded-
    // Layer des Sets ausgeblendet.
    // Die View hat anschliessend keine Kenntnis von dem LayerSet,
    // sie merkt sich nur den Sichtbarkeitsstatus der einzeknen Layer.

    // TRUE, wenn alle MemberLayer des Set sichtbar und alle Excluded-Layer
    // des Set unsichtbar.


    BOOL IsGridVisible() const { return bGridVisible; }             // Rastergitter malen oder nicht
    BOOL IsGridFront() const { return bGridFront  ; }               // Rastergitter ueber die Objekte druebermalen oder dahinter
    BOOL IsHlplFront() const { return bHlplFront  ; }               // Hilfslinie ueber die Objekte druebermalen oder dahinter
    void SetPageVisible(BOOL bOn=TRUE) { bPageVisible=bOn; InvalidateAllWin(); }
    void SetBordVisible(BOOL bOn=TRUE) { bBordVisible=bOn; InvalidateAllWin(); }
    void SetGridVisible(BOOL bOn=TRUE) { bGridVisible=bOn; InvalidateAllWin(); }
    void SetGridFront(BOOL bOn=TRUE) { bGridFront  =bOn; InvalidateAllWin(); }
    void SetHlplVisible(BOOL bOn=TRUE) { bHlplVisible=bOn; InvalidateAllWin(); }
    void SetGlueVisible(BOOL bOn=TRUE) { if (bGlueVisible!=bOn) { bGlueVisible=bOn; } }
    void SetGridColor( Color aColor );

    BOOL IsLineDraft() const { return bLineDraft; } // Linien nur andeuten (alle Linien als Haarlinien)
    BOOL IsFillDraft() const { return bFillDraft; } // Ersatzdarstellung fuer Bitmapfuellungen und Farbverlaeufe
    BOOL IsTextDraft() const { return bTextDraft; } // Ersatzdarstellung fuer Text
    BOOL IsGrafDraft() const { return bGrafDraft; } // Ersatzdarstellung fuer Grafiken und OLE
    void SetLineDraft(BOOL bOn) { bLineDraft=bOn; InvalidateAllWin(); }
    void SetFillDraft(BOOL bOn) { bFillDraft=bOn; InvalidateAllWin(); }
    void SetTextDraft(BOOL bOn) { bTextDraft=bOn; InvalidateAllWin(); }
    void SetGrafDraft(BOOL bOn) { bGrafDraft=bOn; InvalidateAllWin(); }

    /*alt*/void SetGridCoarse(const Size& rSiz) { aGridBig=rSiz; SetGridWidth(Fraction(rSiz.Width(),1),Fraction(rSiz.Height(),1)); }
    /*alt*/void SetGridFine(const Size& rSiz) { aGridFin=rSiz; if (aGridFin.Height()==0) aGridFin.Height()=aGridFin.Width(); if (bGridVisible) InvalidateAllWin(); } // #40479#
    /*alt*/const Size& GetGridCoarse() const { return aGridBig; }
    /*alt*/const Size& GetGridFine() const { return aGridFin; }

    void SetGridWidth(const Fraction& rX, const Fraction& rY) { aGridWdtX=rX; aGridWdtY=rY; if (bGridVisible) InvalidateAllWin(); }

    void InvalidateAllWin();
    void InvalidateAllWin(const Rectangle& rRect, BOOL bPlus1Pix=FALSE);

    // Wenn die View kein Invalidate() an den Fenstern durchfuehren soll, muss
    // man diese beiden folgenden Methoden ueberladen und entsprechend anders
    // reagieren.
    virtual void InvalidateOneWin(Window& rWin);
    virtual void InvalidateOneWin(Window& rWin, const Rectangle& rRect);

    void SetActiveLayer(const String& rName) { aAktLayer=rName; }

    // Verlassen einer betretenen Objektgruppe aller sichtbaren Seiten.
    // (wie MsDos chdir ..)

    // Verlassen aller betretenen Objektgruppen aller sichtbaren Seiten.
    // (wie MsDos chdir \)
    void LeaveAllGroup();

    // Wird mit EnterGroup an einer PageView oder an der MarkView eine
    // Gruppe betreten, werden alle zu dieser Zeit nicht erreichbaren
    // Objekte mit diesen Attributen dargestellt. NULL=normale Darstellung.

    // DefaultAttribute an der View: Neu erzeugte Objekte bekommen diese
    // Attribute direkt nach dem Erzeugen erstmal zugewiesen.
    void SetDefaultStyleSheet(SfxStyleSheet* pStyleSheet, BOOL bDontRemoveHardAttr);
    SfxStyleSheet* GetDefaultStyleSheet() const { return pDefaultStyleSheet; }

    // Painten der MasterPage mit 'ner Bitmap
    BOOL IsMasterPagePaintCaching() { return bMasterBmp; }
    void SetMasterPagePaintCaching( BOOL bOn );

    // use this mode as mode to draw all internal GraphicManager objects with
    ULONG GetGraphicManagerDrawMode() const { return nGraphicManagerDrawMode; }
    void SetGraphicManagerDrawMode( ULONG nMode ) { nGraphicManagerDrawMode = nMode; }

    // SwapIn (z.B. von Grafiken) asynchron durchfuehren. Also nicht
    // beim Paint sofort nachladen, sondern dort das Nachladen anstossen.
    // Nach Abschluss des Nachladens wird das Objekt dann angezeigt.
    // Hat nur z.Zt. Wirkung, wenn SwapGraphics am Model eingeschaltet ist.
    // Default=FALSE. Flag ist nicht persistent.
    BOOL IsSwapAsynchron() const { return bSwapAsynchron; }
    void SetSwapAsynchron(BOOL bJa=TRUE) { bSwapAsynchron=bJa; }

    // Verhalten beim Starten von Animation im Paint-Handler:
    // 1. Animation normal starten( SDR_ANIMATION_ANIMATE ) => DEFAULT
    // 2. Nur die Ersatzdarstellung der Animation zeichnen ( SDR_ANIMATION_DONT_ANIMATE )
    // 3. Nicht starten und nichts ersatzweise ausgeben ( SDR_ANIMATION_DISABLE )
    void SetAnimationMode( const SdrAnimationMode eMode ) { eAnimationMode = eMode; }
    SdrAnimationMode GetAnimationMode() const { return eAnimationMode; }

    virtual void InsertControlContainer( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer > /*xCC*/) {}
    virtual void RemoveControlContainer( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer > /*xCC*/) {}

    BOOL IsPrintPreview() const { return bPrintPreview; }
    void SetPrintPreview(BOOL bOn=TRUE) { bPrintPreview=bOn; }


    virtual void onChangeColorConfig();
};

}//end of namespace binfilter
#endif //_SVDPNTV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
