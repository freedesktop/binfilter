/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdpntv.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 14:44:13 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SVDPNTV_HXX
#define _SVDPNTV_HXX

#include <slist>

#ifndef _TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif

#ifndef _VIRDEV_HXX //autogen
#include <vcl/virdev.hxx>
#endif

#ifndef _SFXBRDCST_HXX
#include <svtools/brdcst.hxx>
#endif

#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif

#ifndef _SFXSMPLHINT_HXX
#include <svtools/smplhint.hxx>
#endif

#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif

#ifndef _UNDO_HXX //autogen
#include <svtools/undo.hxx>
#endif

#ifndef _SVDDRAG_HXX
#include <bf_svx/svddrag.hxx>
#endif

#ifndef _SVDLAYER_HXX
#include <bf_svx/svdlayer.hxx>  // fuer SetOfByte
#endif

#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

#ifndef _WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif


//************************************************************
//   Pre-Defines
//************************************************************

namespace com { namespace sun { namespace star { namespace awt {
class XControlContainer;
}}}}
class OutputDevice;
class VirtualDevice;
class SfxItemSet;
class SfxStyleSheet;
//STRIP012class B2dIAOManager;
namespace binfilter {
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
class SdrItemBrowser;
class SdrObject;

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
//STRIP001  void Invert(OutputDevice& rOut) const { DrawRect(rOut,FALSE); }
//STRIP001  void RollIt(OutputDevice& rOut, BOOL bIncIdx=TRUE) { DrawRect(rOut,TRUE); if (bIncIdx) IncRollIdx(); }
    void IncRollIdx() { nIdx++; if (nIdx>=2*nLen) nIdx=0; }
    const Point& GetP1() const { return aP1; }
    const Point& GetP2() const { return aP2; }
    void SetP1(const Point& rP) const { ((RollingRect*)this)->aP1=rP; }
    void SetP2(const Point& rP) const { ((RollingRect*)this)->aP2=rP; }
    BOOL IsCrossHair() const { return bCrossHair; }
    BOOL IsStripes() const { return bStripes; }
    void SetCrossHair(BOOL bOn) { bCrossHair=bOn; }
    void SetStripes(BOOL bOn) { bStripes=bOn; }

protected:
//STRIP001  void DrawRect(OutputDevice& rOut, BOOL bMov) const;
//STRIP001  void DrawHor(OutputDevice& rOut, long x1, long x2, long y, BOOL bMov) const;
//STRIP001  void DrawVer(OutputDevice& rOut, long x, long y1, long y2, BOOL bMov) const;
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
//STRIP001  void Invert(OutputDevice* pOut) const;
//STRIP001  void Start() const;
//STRIP001  void Stop() const;
};

//************************************************************
//   Hilfsklasse ImpMasterBmp
//************************************************************

class ImpMasterBmp
{
    friend class                SdrPageView;

    Fraction                    aMapX;
    Fraction                    aMapY;
    Rectangle                   aLogBound;
    Rectangle                   aPixBound;
    VirtualDevice               aVD;
    SetOfByte                   aVisLayers;
    ULONG                       nDrawMode;
    USHORT                      nMasterPageNum;

public:
    ImpMasterBmp(): nDrawMode(0), nMasterPageNum(0) {}
    ImpMasterBmp(const OutputDevice& rCompDev): aVD(rCompDev), nDrawMode(0), nMasterPageNum(0) {}
    VirtualDevice& GetVirtDev() { return aVD; }
    const VirtualDevice& GetVirtDev() const { return aVD; }
    USHORT GetMasterPageNum() const { return nMasterPageNum; }
};

//************************************************************
//   Hilfsklasse SdrViewWinRec
//************************************************************

class SdrViewWinRec
{
public:
    OutputDevice*               pWin;
//    VirtualDevice*                pVDev;        // fuer SolidHandles
//STRIP012  B2dIAOManager*              pIAOManager;
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

    SdrViewWinList(const SdrViewWinList& rSource)
    :   aList(1024,4,4) {}
    void operator=(const SdrViewWinList& rSource) {}

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
//  @@@@@   @@@@  @@ @@  @@ @@@@@@  @@ @@ @@ @@@@@ @@   @@
//  @@  @@ @@  @@ @@ @@@ @@   @@    @@ @@ @@ @@    @@   @@
//  @@  @@ @@  @@ @@ @@@@@@   @@    @@ @@ @@ @@    @@ @ @@
//  @@@@@  @@@@@@ @@ @@@@@@   @@    @@@@@ @@ @@@@  @@@@@@@
//  @@     @@  @@ @@ @@ @@@   @@     @@@  @@ @@    @@@@@@@
//  @@     @@  @@ @@ @@  @@   @@     @@@  @@ @@    @@@ @@@
//  @@     @@  @@ @@ @@  @@   @@      @   @@ @@@@@ @@   @@
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
    SdrItemBrowser*             pItemBrowser;
    const OutputDevice*         pActualOutDev; // Nur zum vergleichen
    OutputDevice*               pDragWin;
    SfxStyleSheet*              pDefaultStyleSheet;
    SfxItemSet*                 pDisabledAttr;
    ImpMasterBmp*               pMasterBmp;         // gemerkte MasterPage

    String                      aAktLayer;     // Aktueller Zeichenlayer
    String                      aMeasureLayer; // Aktueller Layer fuer Bemassung

    Container                   aPagV;         // Liste von SdrPageViews
    Container                   aPagHide;      // Liste von nicht angezeigten SdrPageViews
    SdrViewWinList              aWinList;    // Alle Fenster, in der die View angezeigt ist
    MapMode                     aActualMapMode;
    Size                        aGridBig; // muss dann mal raus
    Size                        aGridFin; // muss dann mal raus
    Size                        aGridSubdiv;
    Fraction                    aGridWdtX;
    Fraction                    aGridWdtY;
    FrameAnimator               aAni;
    SdrDragStat                 aDragStat;
    Rectangle                   aMaxWorkArea;
    SfxItemSet                  aDefaultAttr;
    Timer                       aComeBackTimer;
    Timer                       aAfterPaintTimer;
    AutoTimer                   aUserMarkerAnimator;
    Container                   aUserMarkers;
    List                        aAsyncPaintList;
    Timer                       aAsyncPaintTimer;

    SdrAnimationMode            eAnimationMode;

    USHORT                      nHitTolPix;
    USHORT                      nMinMovPix;
    USHORT                      nHitTolLog;
    USHORT                      nMinMovLog;
    ULONG                       nMasterCacheMode;
    ULONG                       nGraphicManagerDrawMode;

    unsigned                    bForeignXOut : 1;
    unsigned                    bLayerSortedRedraw : 1;
    unsigned                    bPageVisible : 1;
    unsigned                    bPageBorderVisible : 1;
    unsigned                    bBordVisible : 1;
    unsigned                    bGridVisible : 1;
    unsigned                    bGridFront : 1;
    unsigned                    bHlplVisible : 1;
    unsigned                    bHlplFront : 1;
    unsigned                    bGlueVisible : 1;    // Persistent. Klebepunkte anzeigen
    unsigned                    bGlueVisible2 : 1;   // Klebepunkte auch bei GluePointEdit anzeigen
    unsigned                    bGlueVisible3 : 1;   // Klebepunkte auch bei EdgeTool anzeigen
    unsigned                    bGlueVisible4 : 1;   // Klebepunkte anzeigen, wenn 1 Edge markiert
    unsigned                    bLineDraft : 1;
    unsigned                    bFillDraft : 1;
    unsigned                    bTextDraft : 1;
    unsigned                    bGrafDraft : 1;
    unsigned                    bLineDraftPrn : 1;
    unsigned                    bFillDraftPrn : 1;
    unsigned                    bTextDraftPrn : 1;
    unsigned                    bGrafDraftPrn : 1;
    unsigned                    bSaveHiddenPages : 1; // den View-Status von nicht angezeigten Seiten merken ?
    unsigned                    bPageTwice : 1;       // TRUE=Dieselbe Page kann mehrfach angezeigt werden.
    unsigned                    bRestoreColors : 1;   // Pens und Brushes werden zurueckgesetzt.
    unsigned                    bObjectPaintIgnoresClipping : 1;
    unsigned                    bEncircle : 1;
    unsigned                    bSomeObjChgdFlag : 1;
    unsigned                    bMasterBmp : 1;         // MasterPages als Bitmap merken?
    unsigned                    bSwapAsynchron : 1;
    unsigned                    bPrintPreview : 1;

    // BOOL fuer die Verwaltung des anzuzeigenden Status
    // Gruppe Betreten/Verlassen. Default ist TRUE, wird aber
    // beispielsweise beim Chart auf FALSE gesetzt, da dort
    // die Ghosted-Effekte zur Darstellug unerwuenscht sind.
    unsigned                    bVisualizeEnteredGroup : 1;
    unsigned                    bHideGrafDraft : 1;
    unsigned                    bAnimationPause : 1;

    ::svtools::ColorConfig            maColorConfig;
    Color                       maGridColor;

private:
    typedef ::std::slist< SdrGrafObj* > GraphicsStack;
    GraphicsStack               maSwappedInGraphicsStack;

    void ImpForceSwapOut();
    void ImpClearVars();
    DECL_LINK(ImpComeBackHdl,Timer*);
    DECL_LINK(ImpAfterPaintHdl,Timer*);
    DECL_LINK(ImpUserMarkerAnimatorHdl,AutoTimer*);
    void ImpInsertUserMarker(SdrViewUserMarker* pMarker);
    void ImpRemoveUserMarker(SdrViewUserMarker* pMarker);
    void ImpCheckMarkerAnimator();
    USHORT ImpGetUserMarkerCount() const { return (USHORT)aUserMarkers.Count(); }
    SdrViewUserMarker* ImpGetUserMarker(USHORT nNum) const { return (SdrViewUserMarker*)aUserMarkers.GetObject(nNum); }
//STRIP001  void ImpAddAsyncObj( const SdrObject* pObj, const OutputDevice* pOut );
    void ImpAsyncPaintDone( const SdrObject* pObj );

protected:
//STRIP001  USHORT ImpGetMinMovLogic(short nMinMov, const OutputDevice* pOut) const;
    USHORT ImpGetHitTolLogic(short nHitTol, const OutputDevice* pOut) const;

    // Wenn man den IdleStatus des Systems nicht abwarten will (auf const geschummelt):
    void FlushComeBackTimer() const;
    void TheresNewMapMode();
    void ImpSetGlueVisible2(BOOL bOn) { if (bGlueVisible2!=bOn) { bGlueVisible2=bOn; if (!bGlueVisible && !bGlueVisible3 && !bGlueVisible4) GlueInvalidate(); } }
    void ImpSetGlueVisible3(BOOL bOn) { if (bGlueVisible3!=bOn) { bGlueVisible3=bOn; if (!bGlueVisible && !bGlueVisible2 && !bGlueVisible4) GlueInvalidate(); } }
    void ImpSetGlueVisible4(BOOL bOn) { if (bGlueVisible4!=bOn) { bGlueVisible4=bOn; if (!bGlueVisible && !bGlueVisible2 && !bGlueVisible3) GlueInvalidate(); } }
    BOOL ImpIsGlueVisible2() const { return bGlueVisible2; }
    BOOL ImpIsGlueVisible3() const { return bGlueVisible3; }
    BOOL ImpIsGlueVisible4() const { return bGlueVisible4; }
    BOOL ImpIsGlueVisible() { return bGlueVisible || bGlueVisible2 || bGlueVisible3 || bGlueVisible4; }

    virtual void SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType);
    BOOL IsShownXorVisibleWinNum(USHORT nNum) const { return aWinList[nNum].bXorVisible; }
    void SetShownXorVisible(USHORT nNum, BOOL bAn) { aWinList[nNum].bXorVisible=bAn; }
    USHORT GetHiddenPV(const SdrPage* pPage) const;
    void GlueInvalidate() const;

//STRIP001  void ShowEncirclement(OutputDevice* pOut);
//STRIP001  void HideEncirclement(OutputDevice* pOut);
//STRIP001  void DrawEncirclement(OutputDevice* pOut) const;

    virtual void WriteRecords(SvStream& rOut) const;
    virtual BOOL ReadRecord(const SdrIOHeader& rViewHead, const SdrNamedSubRecord& rSubHead, SvStream& rIn);
    // ModelHasChanged wird gerufen, sobald nach beliebig vielen HINT_OBJCHG
    // das System wieder idle ist (StarView-Timer). Wer diese Methode ueberlaed,
    // muss unbedingt ModelHasChanged() der Basisklasse rufen.
    virtual void ModelHasChanged();

public:
    TYPEINFO();
    SdrPaintView(SdrModel* pModel1, OutputDevice* pOut=NULL);
    SdrPaintView(SdrModel* pModel1, ExtOutputDevice* pXOut);
    virtual ~SdrPaintView();

    virtual void ClearPageViews();
    virtual void ClearHideViews();
    virtual void ClearAll();
    virtual void Clear(); // PageViews loeschen, Markierungen weg, ...
    SdrModel* GetModel() const { return pMod; }

    virtual BOOL IsAction() const;
//STRIP001  virtual void MovAction(const Point& rPnt);
//STRIP001  virtual void EndAction();
//STRIP001  virtual void BckAction();
    virtual void BrkAction(); // Alle Actions z.B. Draggen abbrechen.
//STRIP001  virtual void TakeActionRect(Rectangle& rRect) const;

    // Muss dann bei jedem Fensterwechsel (wenn die SdrView in mehreren
    // Fenstern gleichzeitig dargestellt wird (->z.B. Splitter)) und bei
    // jedem MapMode(Scaling)-wechsel gerufen werden, damit ich aus meinen
    // Pixelwerten logische Werte berechnen kann.
    void SetActualWin(const OutputDevice* pWin);
    void SetMinMoveDistancePixel(USHORT nVal) { nMinMovPix=nVal; TheresNewMapMode(); }
    USHORT GetMinMoveDistancePixel() const { return (USHORT)nMinMovPix; }
    void SetHitTolerancePixel(USHORT nVal) { nHitTolPix=nVal; TheresNewMapMode(); }
    USHORT GetHitTolerancePixel() const { return (USHORT)nHitTolPix; }

    // Flag zur Visualisierung von Gruppen abfragen/testen
    BOOL DoVisualizeEnteredGroup() const { return bVisualizeEnteredGroup; }
    void SetVisualizeEnteredGroup(BOOL bNew) { bVisualizeEnteredGroup = bNew; }

    virtual void ToggleShownXor(OutputDevice* pOut, const Region* pRegion) const; // Alles togglen was als Xor im Win, nix merken! NULL=alle OutDev
    void ShowShownXor(OutputDevice* pOut, BOOL bShow=TRUE);
    void HideShownXor(OutputDevice* pOut) { ShowShownXor(pOut,FALSE); }
    BOOL IsShownXorVisible(OutputDevice* pOut) const;

    //TRISTATE IsShownXorVisible(OutputDevice* pOut) const;
    void RestartAfterPaintTimer();

    // After Painting this functin needs to be called to make some post-processing
    void PostPaint();

    // Am DragStatus laesst sich beispielsweise erfragen, welche
    // entfernung bereits gedraggd wurde, etc.
    const SdrDragStat& GetDragStat() const { return aDragStat; }

    // Anmelden/Abmelden einer PageView an der View.
    // Dieselbe // Seite kann nicht mehrfach angemeldet werden.
    // Methoden mit dem Suffix PgNum erwarten als numerischen Parameter
    // eine Seitennummer (0...). Methoden mit dem Suffix PvNum erwarten
    // degagen als numerischen Parameter die Nummer der PageView an der
    // SdrView (Iterieren ueber alle angemeldeten Pages).
    virtual SdrPageView* ShowPage(SdrPage* pPage, const Point& rOffs);
    SdrPageView* ShowPagePgNum(USHORT nPgNum, const Point& rOffs);
//STRIP001  SdrPageView* ShowMasterPagePgNum(USHORT nPgNum, const Point& rOffs);
    virtual void HidePage(SdrPageView* pPV);
    void HidePage(const SdrPage* pPage) { HidePage(GetPageView(pPage)); }
//STRIP001  void HidePagePgNum(USHORT nPgNum);
    void HidePagePvNum(USHORT nPvNum) { HidePage(GetPageViewPvNum(nPvNum)); }
    virtual void HideAllPages();
//STRIP001  virtual void SetPagePos(SdrPageView* pPV, const Point& rOffs);
//STRIP001  void SetPagePos(SdrPage* pPage, const Point& rOffs) { SetPagePos(GetPageView(pPage),rOffs); }
//STRIP001  void SetPagePosPgNum(USHORT nPgNum, const Point& rOffs);
//STRIP001  void SetPagePosPvNum(USHORT nPvNum, const Point& rOffs) { SetPagePos(GetPageViewPvNum(nPvNum),rOffs); }

    // Iterieren ueber alle angemeldeten PageViews
    USHORT GetPageViewCount() const { return USHORT(aPagV.Count()); }
    SdrPageView* GetPageViewPvNum(USHORT nPvNum) const { return ((SdrPageView*)aPagV.GetObject(nPvNum)); }

    // Pageview einer bestimmten Seite ermitteln
    SdrPageView* GetPageView(const SdrPage* pPage) const;
//STRIP001  SdrPageView* GetPageViewPgNum(USHORT nPgNum) const;
//STRIP001  USHORT GetPageViewNum(const SdrPageView* pPV) const;

    // Test, ob eine Seite getroffen
//STRIP001  SdrPageView* HitPage(const Point& rPnt) const;

    // Die Seite, die dem Punkt am naechsten ist. Liefert nur NULL,
    // wenn absolut keine Seite angemeldet ist.
//STRIP001  SdrPageView* GetPageView(const Point& rPnt) const;

    // Falls noetig kann man sich auch die nicht angezeigten Seiten rausholen
    USHORT GetPageHideCount() const { return USHORT(aPagHide.Count()); }
    SdrPageView* GetPageHidePvNum(USHORT nPvNum) const { return ((SdrPageView*)aPagHide.GetObject(nPvNum)); }
    SdrPageView* GetPageHide(const SdrPage* pPage) const { return ((SdrPageView*)aPagHide.GetObject(GetHiddenPV(pPage))); }
//STRIP001  USHORT GetPageHideNum(const SdrPageView* pPV) const;

    // Eine SdrView kann auf mehreren Fenstern gleichzeitig abgebiltet sein:
    virtual void AddWin(OutputDevice* pWin1);
    virtual void DelWin(OutputDevice* pWin1);

    USHORT GetWinCount() const { return aWinList.GetCount(); }
    OutputDevice* GetWin(USHORT nNum) const { return nNum<aWinList.GetCount() ? aWinList[nNum].pWin : NULL; }
    USHORT FindWin( OutputDevice* pWin ) { return aWinList.Find( pWin ); }

    const SdrViewWinRec& GetWinRec(USHORT nNum) const { return aWinList[nNum]; }
    SdrViewWinRec& GetWinRec(USHORT nNum) { return aWinList[nNum]; }

    /** Query visible area of the view in the given window

        @param nNum
        The window number (same as in the GetWin() method)

        @return the visible area in logical coordinates
     */
    Rectangle GetVisibleArea(USHORT nNum);

    // Einen Satz von Layern hinzu-sichtbarmachen.
    // bShow=FALSE werden alle Memberlayer des Set ausgeblendet.
    // Bei True dagegen werden alle MemberLayer ein- und alle Excluded-
    // Layer des Sets ausgeblendet.
    // Die View hat anschliessend keine Kenntnis von dem LayerSet,
    // sie merkt sich nur den Sichtbarkeitsstatus der einzeknen Layer.
//STRIP001  void ShowLayerSet(const String& rName, BOOL bShow=TRUE);

    // TRUE, wenn alle MemberLayer des Set sichtbar und alle Excluded-Layer
    // des Set unsichtbar.
//STRIP001  TRISTATE IsLayerSetVisible(const String& rName) const;

//STRIP001  void SetLayerVisible(const String& rName, BOOL bShow=TRUE);
//STRIP001  TRISTATE IsLayerVisible(const String& rName) const;
//STRIP001  void SetAllLayersVisible(BOOL bShow=TRUE);

    void SetLayerLocked(const String& rName, BOOL bLock=TRUE);
//STRIP001  TRISTATE IsLayerLocked(const String& rName) const;
//STRIP001  void SetAllLayersLocked(BOOL bLock=TRUE);

//STRIP001  void SetLayerPrintable(const String& rName, BOOL bPrn=TRUE);
//STRIP001  TRISTATE IsLayerPrintable(const String& rName) const;
//STRIP001  void SetAllLayersPrintable(BOOL bPrn=TRUE);

    virtual void InitRedraw(OutputDevice* pOut, const Region& rReg, USHORT nPaintMode=0);
//STRIP001  virtual void InitRedraw(USHORT nWinNum, const Region& rReg, USHORT nPaintMode=0);
//STRIP001  BOOL IsRedrawReady() const;
//STRIP001  BOOL RedrawOne(USHORT nBrkEvent=INPUT_MOUSEANDKEYBOARD);
//STRIP001  BOOL RedrawUntilInput(USHORT nBrkEvent=INPUT_MOUSEANDKEYBOARD);

    BOOL IsPageVisible() const { return bPageVisible; }             // Seite (weisse Flaeche) malen oder nicht
    BOOL IsPageBorderVisible() const { return bPageBorderVisible; } // Seite (weisse Flaeche) malen oder nicht
    BOOL IsBordVisible() const { return bBordVisible; }             // Seitenrandlinie malen oder nicht
    BOOL IsGridVisible() const { return bGridVisible; }             // Rastergitter malen oder nicht
    BOOL IsGridFront() const { return bGridFront  ; }               // Rastergitter ueber die Objekte druebermalen oder dahinter
    BOOL IsHlplVisible() const { return bHlplVisible; }             // Hilfslinien der Seiten malen oder nicht
    BOOL IsHlplFront() const { return bHlplFront  ; }               // Hilfslinie ueber die Objekte druebermalen oder dahinter
    BOOL IsGlueVisible() const { return bGlueVisible; }             // Konnektoren der objekte sichtbar oder nicht
//STRIP001  Color GetGridColor() const;
    void SetPageVisible(BOOL bOn=TRUE) { bPageVisible=bOn; InvalidateAllWin(); }
    void SetPageBorderVisible(BOOL bOn=TRUE) { bPageBorderVisible=bOn; InvalidateAllWin(); }
    void SetBordVisible(BOOL bOn=TRUE) { bBordVisible=bOn; InvalidateAllWin(); }
    void SetGridVisible(BOOL bOn=TRUE) { bGridVisible=bOn; InvalidateAllWin(); }
    void SetGridFront(BOOL bOn=TRUE) { bGridFront  =bOn; InvalidateAllWin(); }
    void SetHlplVisible(BOOL bOn=TRUE) { bHlplVisible=bOn; InvalidateAllWin(); }
    void SetHlplFront(BOOL bOn=TRUE) { bHlplFront  =bOn; InvalidateAllWin(); }
    void SetGlueVisible(BOOL bOn=TRUE) { if (bGlueVisible!=bOn) { bGlueVisible=bOn; if (!bGlueVisible2 && !bGlueVisible3 && !bGlueVisible4) GlueInvalidate(); } }
    void SetGridColor( Color aColor );

    BOOL IsLineDraft() const { return bLineDraft; } // Linien nur andeuten (alle Linien als Haarlinien)
    BOOL IsFillDraft() const { return bFillDraft; } // Ersatzdarstellung fuer Bitmapfuellungen und Farbverlaeufe
    BOOL IsTextDraft() const { return bTextDraft; } // Ersatzdarstellung fuer Text
    BOOL IsGrafDraft() const { return bGrafDraft; } // Ersatzdarstellung fuer Grafiken und OLE
    BOOL IsHideGrafDraft() const { return bHideGrafDraft; } // Ersatzdarstellung fuer Grafiken und OLE nicht anzeigen
    BOOL IsLineDraftPrn() const { return bLineDraftPrn; } // Beim drucken: Linien nur andeuten (alle Linien als Haarlinien)
    BOOL IsFillDraftPrn() const { return bFillDraftPrn; } // Beim drucken: Ersatzdarstellung fuer Bitmapfuellungen und Farbverlaeufe
    BOOL IsTextDraftPrn() const { return bTextDraftPrn; } // Beim drucken: Ersatzdarstellung fuer Text
    BOOL IsGrafDraftPrn() const { return bGrafDraftPrn; } // Beim drucken: Ersatzdarstellung fuer Grafiken und OLE
    void SetLineDraft(BOOL bOn) { bLineDraft=bOn; InvalidateAllWin(); }
    void SetFillDraft(BOOL bOn) { bFillDraft=bOn; InvalidateAllWin(); }
    void SetTextDraft(BOOL bOn) { bTextDraft=bOn; InvalidateAllWin(); }
    void SetGrafDraft(BOOL bOn) { bGrafDraft=bOn; InvalidateAllWin(); }
    void SetHideGrafDraft(BOOL bOn) { bHideGrafDraft=bOn; InvalidateAllWin(); }
    void SetLineDraftPrn(BOOL bOn) { bLineDraftPrn=bOn; }
    void SetFillDraftPrn(BOOL bOn) { bFillDraftPrn=bOn; }
    void SetTextDraftPrn(BOOL bOn) { bTextDraftPrn=bOn; }
    void SetGrafDraftPrn(BOOL bOn) { bGrafDraftPrn=bOn; }

    /*alt*/void SetGridCoarse(const Size& rSiz) { aGridBig=rSiz; SetGridWidth(Fraction(rSiz.Width(),1),Fraction(rSiz.Height(),1)); }
    /*alt*/void SetGridFine(const Size& rSiz) { aGridFin=rSiz; if (aGridFin.Height()==0) aGridFin.Height()=aGridFin.Width(); if (bGridVisible) InvalidateAllWin(); } // #40479#
    /*alt*/const Size& GetGridCoarse() const { return aGridBig; }
    /*alt*/const Size& GetGridFine() const { return aGridFin; }

    // SetGridSubdivision(): Werte fuer X und Y sind die Anzahl der
    // Zwischenraeume, also = Anzahl der Zwischenpunkte+1
    void SetGridWidth(const Fraction& rX, const Fraction& rY) { aGridWdtX=rX; aGridWdtY=rY; if (bGridVisible) InvalidateAllWin(); }
    void SetGridSubdivision(const Size& rSubdiv) { aGridSubdiv=rSubdiv; if (bGridVisible) InvalidateAllWin(); }
    const Fraction& GetGridWidthX() const { return aGridWdtX; }
    const Fraction& GetGridWidthY() const { return aGridWdtY; }
    const Size& GetGridSubdivision() const { return aGridSubdiv; }

    // Status der nicht angezeigten Seiten merken?
    // (LayerVisible-Settings)
    BOOL IsHiddenPageSaveMode() const { return bSaveHiddenPages; }
    void SetHiddenPageSaveMode(BOOL bOn=TRUE) { bSaveHiddenPages=bOn; }

    void InvalidateAllWin();
    void InvalidateAllWin(const Rectangle& rRect, BOOL bPlus1Pix=FALSE);

    // Wenn die View kein Invalidate() an den Fenstern durchfuehren soll, muss
    // man diese beiden folgenden Methoden ueberladen und entsprechend anders
    // reagieren.
    virtual void InvalidateOneWin(Window& rWin);
    virtual void InvalidateOneWin(Window& rWin, const Rectangle& rRect);

    void SetActiveLayer(const String& rName) { aAktLayer=rName; }
    const String&  GetActiveLayer() const { return aAktLayer; }

    // Normalerweise ist der Redraw nicht nach Layer sortiert. Die Objekte
    // werden also ihrer Order nach (SdrObject::GetObjOrdNum()) ausgegeben;
    // Objekte mit niedriger Ordnungszahl werden zuerst, solche mit hoher
    // Ordnungszahl zuletzt ausgegeben. Layer werden dabei nicht
    // beruecksichtigt, dass heisst:
    // - die Objekte werden aus Layersicht bunt durcheinander gemalt
    //   (also nicht der Folieneffekt)
    // - Falls vorhanden, werden auch Objekte gepainted, fuer die kein
    //   Layer definiert ist (Objekt besitzt undefinierte LayerID).
    // Setzt man den Modus "LayerSortedRedraw", ergibt sich eine andere
    // Reihenfolge zur Ausgabe der Zeichenobjekte. Diese richtet sich zunaechst
    // nach der Reihenfolge der Layer in den LayerAdmins der Page und des Model.
    // Zuerst werden die Layer des Model ausgegeben, anschliessend die Layer
    // der Page. Innerhalb der Layer gilt schliesslich wieder die ObjOrder.
    // Applikationen die die Zeichenobjekte mit der Methode RedrawOneLayer
    // ausgeben (also immer layerorientiert painten) muessen diesen Modus
    // stets gesetzt halten, damit der HitTest, etc. entsprechend reagieren
    // kann.
    // !!!noch nicht implementiert!!!
    void SetLayerSortedRedraw(BOOL bOn) { bLayerSortedRedraw=TRUE; InvalidateAllWin(); }
    BOOL IsLayerSortedRedraw() const { return bLayerSortedRedraw; }

    // zu beruecksichtigen:
    // Redraw, HitTest, ToTop/Btm (inkl. IsPossible) DrawMarkedObj
    // Wenn Objekte gepainted werden, die auf das Clipping sch...
    // (-> Carsten's Dialogeditor, in dem er echte Controls malt)
    void SetObjectPaintIgnoresClipping(BOOL bOn=TRUE) { bObjectPaintIgnoresClipping=bOn; }
    BOOL IsObjectPaintIgnoresClipping() const { return bObjectPaintIgnoresClipping; }

    // Verlassen einer betretenen Objektgruppe aller sichtbaren Seiten.
    // (wie MsDos chdir ..)
//STRIP001  void LeaveOneGroup();

    // Verlassen aller betretenen Objektgruppen aller sichtbaren Seiten.
    // (wie MsDos chdir \)
    void LeaveAllGroup();

    // Feststellen, ob Leave sinnvoll ist.
    BOOL IsGroupEntered() const;

    // Wird mit EnterGroup an einer PageView oder an der MarkView eine
    // Gruppe betreten, werden alle zu dieser Zeit nicht erreichbaren
    // Objekte mit diesen Attributen dargestellt. NULL=normale Darstellung.
//STRIP001  void SetDisabledAttr(const SfxItemSet* pNewDisabledAttr);

    // DefaultAttribute an der View: Neu erzeugte Objekte bekommen diese
    // Attribute direkt nach dem Erzeugen erstmal zugewiesen.
//STRIP001  void SetDefaultAttr(const SfxItemSet& rAttr, BOOL bReplaceAll);
    const SfxItemSet& GetDefaultAttr() const { return aDefaultAttr; }
    void SetDefaultStyleSheet(SfxStyleSheet* pStyleSheet, BOOL bDontRemoveHardAttr);
    SfxStyleSheet* GetDefaultStyleSheet() const { return pDefaultStyleSheet; }

//STRIP001  void SetNotPersistDefaultAttr(const SfxItemSet& rAttr, BOOL bReplaceAll);
//STRIP001  void MergeNotPersistDefaultAttr(SfxItemSet& rAttr, BOOL bOnlyHardAttr) const;

    // Aufziehen eines animierten Rechtecks fuer Applikationsspeziefische
    // Verwendung. Alle Positionsangaben in logischen View-Koordinaten.
    // pOut bezeichnet das OutputDevice, in das animierte Rechteck dargestellt
    // werden soll. Wird NULL uebergeben, wird es in allen an der View
    // angemeldeten OutputDevices gleichzeitig dargestellt.
//STRIP001  void BegEncirclement(const Point& rPnt, OutputDevice* pOut=NULL, short nMinMov=-2);
//STRIP001  void MovEncirclement(const Point& rPnt);
//STRIP001  Rectangle EndEncirclement(BOOL bNoJustify=FALSE);
    void BrkEncirclement();
    BOOL IsEncirclement() const { return bEncircle; }

    // Painten der MasterPage mit 'ner Bitmap
    BOOL IsMasterPagePaintCaching() { return bMasterBmp; }
    ULONG GetMasterPagePaintCacheMode() const { return nMasterCacheMode; }
    void SetMasterPagePaintCaching( BOOL bOn, ULONG nCacheMode = SDR_MASTERPAGECACHE_DEFAULT );
    BOOL HasMasterPagePaintCache() { return pMasterBmp!=NULL; }
    void ReleaseMasterPagePaintCache();

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

    // get the InteractionObjectManager for a specified window
//STRIP001  B2dIAOManager* GetIAOManager(OutputDevice* pOut);
//STRIP001  B2dIAOManager* GetFirstIAOManager();
//STRIP012  void RefreshAllIAOManagers();
//STRIP001  virtual BOOL KeyInput(const KeyEvent& rKEvt, Window* pWin);

    BOOL MouseButtonDown(const MouseEvent& rMEvt, Window* pWin) { return FALSE; }
    BOOL MouseButtonUp(const MouseEvent& rMEvt, Window* pWin) { return FALSE; }
    BOOL MouseMove(const MouseEvent& rMEvt, Window* pWin) { return FALSE; }
    BOOL Command(const CommandEvent& rCEvt, Window* pWin) { return FALSE; }
    BOOL Cut(ULONG nFormat=SDR_ANYFORMAT) { return FALSE; }
    BOOL Yank(ULONG nFormat=SDR_ANYFORMAT) { return FALSE; }
    BOOL Paste(Window* pWin=NULL, ULONG nFormat=SDR_ANYFORMAT) { return FALSE; }

    /* new interface src537 */
//STRIP001  BOOL GetAttributes(SfxItemSet& rTargetSet, BOOL bOnlyHardAttr=FALSE) const;

//STRIP001  BOOL SetAttributes(const SfxItemSet& rSet, BOOL bReplaceAll);
//STRIP001  SfxStyleSheet* GetStyleSheet(BOOL& rOk) const;
    BOOL SetStyleSheet(SfxStyleSheet* pStyleSheet, BOOL bDontRemoveHardAttr);

//STRIP001  virtual void MakeVisible(const Rectangle& rRect, Window& rWin);

    // Fuer PlugIn. Wird vom Paint des OLE-Obj gerufen.
//STRIP001  virtual void DoConnect(SdrOle2Obj* pOleObj);

    // Animation aktivieren/deaktivieren fuer ::Paint
    // wird z.Zt. ausgewertet von SdrGrafObj, wenn in dem eine Animation steckt
    // Das Unterbinden der automatischen Animation wird z.B. fuer die Dia-Show benoetigt
    BOOL IsAnimationEnabled() const { return ( SDR_ANIMATION_ANIMATE == eAnimationMode ); }
    void SetAnimationEnabled( BOOL bEnable=TRUE ) { SetAnimationMode( bEnable ? SDR_ANIMATION_ANIMATE : SDR_ANIMATION_DISABLE ); }

    // set/unset pause state for animations
    BOOL IsAnimationPause() const { return bAnimationPause; }
    void SetAnimationPause( BOOL bSet ) { bAnimationPause = bSet; }

    // Verhalten beim Starten von Animation im Paint-Handler:
    // 1. Animation normal starten( SDR_ANIMATION_ANIMATE ) => DEFAULT
    // 2. Nur die Ersatzdarstellung der Animation zeichnen ( SDR_ANIMATION_DONT_ANIMATE )
    // 3. Nicht starten und nichts ersatzweise ausgeben ( SDR_ANIMATION_DISABLE )
    void SetAnimationMode( const SdrAnimationMode eMode ) { eAnimationMode = eMode; }
    SdrAnimationMode GetAnimationMode() const { return eAnimationMode; }

    // bei bShow=FALSE wird der Browser destruiert
//STRIP001  void ShowItemBrowser(BOOL bShow=TRUE);
    BOOL IsItemBrowserVisible() const { return pItemBrowser!=NULL && ((Window*)pItemBrowser)->IsVisible(); }
    Window* GetItemBrowser() const { return (Window*)pItemBrowser; }

    // Muss von App beim Scrollen usw. gerufen werden, damit ein u.U.
    // aktives FormularControl mitverschoben werden kann
    void VisAreaChanged(const OutputDevice* pOut=NULL);
    void VisAreaChanged(const SdrPageViewWinRec& rPVWR);

    virtual void InsertControlContainer( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer > xCC) {}
    virtual void RemoveControlContainer( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer > xCC) {}

    BOOL IsPrintPreview() const { return bPrintPreview; }
    void SetPrintPreview(BOOL bOn=TRUE) { bPrintPreview=bOn; }

//STRIP001     const svtools::ColorConfig& getColorConfig() const;

    virtual void onChangeColorConfig();

    // #103834# Set background color for svx at SdrPageViews
    void SetApplicationBackgroundColor(Color aBackgroundColor);

    // #103911# Set document color for svx at SdrPageViews
    void SetApplicationDocumentColor(Color aDocumentColor);

    /** Determine typical background color for given area on given page

        This method calculates a single representative color for the
        given rectangular area. This is accomplished by sampling from
        various points within that area, and calculating a weighted
        mean value of those samples. High contrast settings are
        respected by this method, i.e. in high contrast mode,
        svtools::DOCCOLOR is used as the background color.

        @attention For some cases, such as highly structured
        background or huge color ranges spanned by the background,
        this method cannot yield meaningful results, as the background
        simply cannot be represented by a single color. Beware.

        @param rArea
        Rectangular area to determine representative background color from

        @param rVisibleLayers
        Set of layers visible when calculating the background color

        @param rCurrPage
        Current page for which to determine background color on

        @return a representative background color.
     */
    Color CalcBackgroundColor( const Rectangle&     rArea,
                               const SetOfByte&     rVisibleLayers,
                               const SdrPage&   rCurrPage ) const;
};

}//end of namespace binfilter
#endif //_SVDPNTV_HXX

