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
#ifndef _CRSRSH_HXX
#define _CRSRSH_HXX

#include <bf_svtools/bf_solar.h>

#include <tools/link.hxx>
#include <bf_tools/string.hxx>

#include <viewsh.hxx>           // fuer ViewShell
#include <cshtyp.hxx>           // fuer die CursorShell Typen
#include <crstate.hxx>          // fuer die CursorMove-Staties
#include <tblsel.hxx>               //SwTblSearchType

namespace com { namespace sun { namespace star { namespace util {
    struct SearchOptions;
} } } }
class KeyCode;
class Region;
namespace binfilter {
class SfxItemSet;
class SfxPoolItem;

// einige Forward Deklarationen


class SpellCheck;
class SwBookmark;
class SwCntntFrm;
class SwCrsrShell;
class SwCursor;
class SwField;
class SwFieldType;
class SwFmt;
class SwFmtFld;
class SwIndex;
class SwNodeIndex;
class SwNode;
class SwNodes;
class SwPaM;
class SwSelBoxes;
class SwShellCrsr;
class SwShellTableCrsr;
class SwTableNode;
class SwTxtFmtColl;
class SwVisCrsr;
class SwTxtINetFmt;
class SwFmtINetFmt;
class SwTxtAttr;
class SwTableBox;
class SwCellFrms;
class SwTOXMark;
class SwRedline;
struct SwPosition;



// enum und struktur, um ueber die Doc-Position Informationen zu erfragen

struct SwContentAtPos
{
    enum IsAttrAtPos
    {
        SW_NOTHING      = 0x0000,
        SW_FIELD        = 0x0001,
        SW_CLICKFIELD   = 0x0002,
        SW_FTN          = 0x0004,
        SW_INETATTR     = 0x0008,
        SW_TABLEBOXFML  = 0x0010,
        SW_REDLINE      = 0x0020,
        SW_OUTLINE      = 0x0040,
        SW_TOXMARK      = 0x0080,
        SW_REFMARK      = 0x0100
#ifdef DBG_UTIL
        ,SW_CURR_ATTRS      = 0x4000        // nur zum Debuggen
        ,SW_TABLEBOXVALUE   = 0x8000        // nur zum Debuggen
#endif
    } eCntntAtPos;

    union {
        const SwField* pFld;
        const SfxPoolItem* pAttr;
        const SwRedline* pRedl;
    } aFnd;

    String sStr;
    const SwTxtAttr* pFndTxtAttr;

    SwContentAtPos( int eGetAtPos = 0xffff )
        : eCntntAtPos( (IsAttrAtPos)eGetAtPos )
    {
        aFnd.pFld = 0;
        pFndTxtAttr = 0;
    }

    // befindet sich der Node in einem geschuetzten Bereich?
};


// defines fuers GetCharCount
#define GETCHARCOUNT_NONE       0
#define GETCHARCOUNT_PARA       1
#define GETCHARCOUNT_SECTION    2


// ReturnWerte von SetCrsr (werden verodert)
const int   CRSR_NOERROR =  0x00,
            CRSR_POSOLD =   0x01,   // Cursor bleibt an alter Doc-Position
            CRSR_POSCHG =   0x02;   // Position vom Layout veraendert




// die Cursor - Shell
class SwCrsrShell : public ViewShell, public SwModify
{
    friend class SwCallLink;
    friend class SwVisCrsr;
    friend class SwSelPaintRects;
    friend class SwChgLinkFlag;

    friend BOOL GetAutoSumSel( const SwCrsrShell&, SwCellFrms& );

public:     // public, damit defaultet werden kann !!

    /* ein enum fuer den Aufruf von UpdateCrsr */
    enum CrsrFlag {
        UPDOWN      = (1 << 0),     // Up/Down auf Spalte halten
        SCROLLWIN   = (1 << 1),     // Window Scrollen
        CHKRANGE    = (1 << 2),     // ueberpruefen von ueberlappenden PaM's
        NOCALRECT   = (1 << 3),     // CharRect nicht neu berechnen
        READONLY    = (1 << 4)      // Sichtbar machen trotz Readonly
    };

private:

    SwRect  aCharRect;          // Char-SRectangle auf dem der Cursor steht
    Point   aCrsrHeight;        // Hohe&Offset von sichtbaren Cursor
    Point   aOldRBPos;          // Right/Bottom von letzter VisArea
                                // (wird im Invalidate vom Cursor benutzt)


    // um event. Macro was anhaengt auszufuehren.
    Link aFlyMacroLnk;          // Link will be called, if the Crsr is set
                                // into a fly. A macro can be then becalled
    Link aChgLnk;               // link will be called by every attribut/
                                // format changes at cursor position.
    Link aGrfArrivedLnk;        // Link calls to UI if a grafik is arrived


    SwShellCrsr* pCurCrsr;      // der aktuelle Cursor
    SwShellCrsr* pCrsrStk;      // Stack fuer den Cursor
    SwVisCrsr *pVisCrsr;        // der Sichtbare-Cursor

    SwShellTableCrsr* pTblCrsr; // Tabellen-Crsr; nur in Tabellen, wenn
                                // die Selection ueber 2 Spalten liegt

    SwNodeIndex* pBoxIdx;       // fuers erkennen der veraenderten
    SwTableBox* pBoxPtr;        // Tabellen-Zelle

    long nUpDownX;              // versuche den Cursor bei Up/Down immer in
                                // der gleichen Spalte zu bewegen
    long nLeftFrmPos;
    ULONG nAktNode;             // save CursorPos at Start-Action
    xub_StrLen nAktCntnt;
    USHORT nAktNdTyp;
    bool bAktSelection;

    USHORT nCrsrMove;
    USHORT nBasicActionCnt;     // Actions, die vom Basic geklammert wurden
    CrsrMoveState eMvState;     // Status fuers Crsr-Travelling - GetCrsrOfst

    BOOL bHasFocus : 1;         // Shell ist in einem Window "aktiv"
    BOOL bSVCrsrVis : 1;        // SV-Cursor Un-/Sichtbar
    BOOL bChgCallFlag : 1;      // Attributaenderung innerhalb von
                                // Start- und EndAction
    BOOL bVisPortChgd : 1;      // befindet sich im VisPortChg-Aufruf
                                // (wird im Invalidate vom Cursor benutzt)

    BOOL bCallChgLnk : 1;       // Flag fuer abgeleitete Klassen:
                                // TRUE -> ChgLnk callen
                                // Zugriff nur ueber SwChgLinkFlag
    BOOL bAllProtect : 1;       // Flag fuer Bereiche
                                // TRUE -> alles geschuetzt / versteckt
    BOOL bInCMvVisportChgd : 1; // Flag fuer CrsrMoves
                                // TRUE -> die Sicht wurde verschoben
    BOOL bGCAttr : 1;           // TRUE -> es existieren nichtaufgespannte Attr.
    BOOL bIgnoreReadonly : 1;   // TRUE -> Beim naechsten EndAction trotz
                                // Readonly den Crsr sichtbar machen.
    BOOL bSelTblCells : 1;      // TRUE -> Zellen uebers InputWin selektieren
    BOOL bAutoUpdateCells : 1;  // TRUE -> Zellen werden autoformatiert
    BOOL bBasicHideCrsr : 1;    // TRUE -> HideCrsr vom Basic
    BOOL bSetCrsrInReadOnly : 1;// TRUE -> Cursor darf in ReadOnly-Bereiche
    BOOL bOverwriteCrsr : 1;    // TRUE -> show Overwrite Crsr
    // OD 11.02.2003 #100556# - flag to allow/avoid execution of marcos (default: true)
    bool mbMacroExecAllowed : 1;

    typedef bool (SwCursor:: *FNCrsr)();

protected:
    /*
     * Compare-Methode for the StackCursor and the current Cursor.
     * The Methods return -1, 0, 1 for lower, equal, greater. The enum
     * CrsrCompareType says which position is compared.
     */
    enum CrsrCompareType {
        StackPtStackMk,
        StackPtCurrPt,
        StackPtCurrMk,
        StackMkCurrPt,
        StackMkCurrMk,
        CurrPtCurrMk
    };

    USHORT IncBasicAction()             { return ++nBasicActionCnt; }
    USHORT DecBasicAction()             { return --nBasicActionCnt; }

    // Setzt alle PaMs in OldNode auf NewPos + Offset
    // Setzt alle PaMs im Bereich von [StartNode, EndNode] nach NewPos

public:
    TYPEINFO();
    SwCrsrShell( SwDoc& rDoc, Window *pWin );
    virtual ~SwCrsrShell();

    virtual void Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);

    // loesche den aktuellen Cursor und der folgende wird zum Aktuellen
    // TableCursor in normale Cursor verwandeln, Tablemode aufheben

    SwPaM* GetCrsr( bool bMakeTblCrsr = TRUE ) const;
    // nur den akt. Cursor returnen
          SwShellCrsr* _GetCrsr()                       { return pCurCrsr; }
    const SwShellCrsr* _GetCrsr() const                 { return pCurCrsr; }

    USHORT GetBasicActionCnt() const    { return nBasicActionCnt; }

    // Basiscursortravelling
    long GetUpDownX() const             { return nUpDownX; }

    /**
       Ensure point and mark of the current PaM are in a specific order.

       @param bPointFirst TRUE: If the point is behind the mark then
       swap the PaM. FALSE: If the mark is behind the point then swap
       the PaM.
    */
    void NormalizePam(BOOL bPointFirst = TRUE);

    // Methoden zum Anzeigen bzw. Verstecken der selektierten Bereiche mit
    // dem sichtbaren Cursor

    bool IsOverwriteCrsr() const { return bOverwriteCrsr; }
    void SetOverwriteCrsr( bool bFlag ) { bOverwriteCrsr = bFlag; }

    // darf der Cursor in ReadOnlyBereiche?
    bool IsReadOnlyAvailable() const { return bSetCrsrInReadOnly; }

    // Methoden fuer aFlyMacroLnk
    void        SetFlyMacroLnk( const Link& rLnk ) { aFlyMacroLnk = rLnk; }
    const Link& GetFlyMacroLnk() const             { return aFlyMacroLnk; }

    //ChgLink callen, innerhalb einer Action wird der Ruf verzoegert.
    void CallChgLnk();

    /*
     * liefert das SRectangle, auf dem der Cursor steht.
     */
    const SwRect &GetCharRect() const { return aCharRect; }

        // ist der Crsr in einer Tabelle und ist die Selection ueber
        // zwei Spalten
    bool IsTableMode() const { return 0 != pTblCrsr; }

    // erfrage die selektierte "Region" aller Cursor (fuer D&D auf Mac)
    Region GetCrsrRegion() const;

    // steht der Curor auf einem "Symbol"-Zeichen

    BOOL BasicActionPend() const    { return nBasicActionCnt != nStartAction; }

        // Attribut selelktieren

    bool CheckTblBoxCntnt( const SwPosition* pPos = 0 );
    void ClearTblBoxCntnt();

    // wird gerufen, wenn eine Tabellenselektion im UpdateCrsr erzeugt wird,
    // ohne das die UI davon etaws weiss

    void SetSelTblCells( BOOL bFlag )           { bSelTblCells = bFlag; }
    BOOL IsSelTblCells() const                  { return bSelTblCells; }

    BOOL IsAutoUpdateCells() const              { return bAutoUpdateCells; }
    void SetAutoUpdateCells( BOOL bFlag )       { bAutoUpdateCells = bFlag; }
};


class SwChgLinkFlag
{
    BOOL bOldFlag;
    SwCrsrShell& rCrsrShell;
    long nLeftFrmPos;
public:
    SwChgLinkFlag( SwCrsrShell& rShell );
    ~SwChgLinkFlag();
};

} //namespace binfilter
#endif  // _CRSRSH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
