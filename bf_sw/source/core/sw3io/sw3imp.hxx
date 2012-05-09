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
#ifndef _SW3IMP_HXX
#define _SW3IMP_HXX

#include <bf_svtools/bf_solar.h>

#include <bf_tools/string.hxx>
#include <bf_svtools/svarray.hxx>

#ifndef _SVSTDARR_BYTES_DECL
#define _SVSTDARR_BYTES
#include <bf_svtools/svstdarr.hxx>
#endif

#include <bf_so3/svstor.hxx>
#include <bf_so3/persist.hxx>
#include <tools/poly.hxx>
#include <unotools/fontcvt.hxx>

#include <docary.hxx>
#include <sw3ids.hxx>
#include <docstat.hxx>
#include <list>

class SvStream;
class SvXub_StrLens;
class SvxMacroTableDtor;

namespace binfilter {

class ImageMap;
class SvStringsDtor;
class SvStorageInfoList;
class SvStrings;

class Crypter;
class SvUShorts;

class SfxItemSet;
class SfxPoolItem;


class Sw3Io;
class SwAttrSet;
class SwBlockNames;
class SwBookmark;
class SwCntntNode;
class SwDoc;
class SwEndNoteInfo;
class SwField;
class SwFieldType;
class SwFmt;
class SwFmtFld;
class SwFmtINetFmt;
class SwFrm;
class SwFrmFmt;
class SwIndex;
class SwNoTxtNode;
class SwNodeIndex;
class SwNodeNum;
class SwNumFmt;
class SwNumRule;
class SwOLENode;
class SwPaM;
class SwPageDesc;
class SwPageFtnInfo;
class SwRedline;
class SwSectionNode;
class SwStartNode;
class SwTable;
class SwTableBox;
class SwTableBoxes;
class SwTableLine;
class SwTableLines;
class SwTableNode;
class SwTxtFmtColl;
class SwTxtNode;
class SwgReaderOption;
class SwpHints;
class SwFlyFrm;

class Sw3RecordSizeTable;
class SvxTabStopItem;
class Sw3TOXBase;
class SwInsHardBlankSoftHyph;
class SvxFontItem;
class SdrObject;


extern sal_Char SW3HEADER[];        // 3.0/3.1 Header
extern sal_Char SW4HEADER[];        // 4.0 Header
extern sal_Char SW5HEADER[];        // 5.0 Header

extern sal_Char sSW3IO_FixedField[];
extern sal_Char sSW3IO_AuthorityField[];
extern sal_Char sSW3IO_DropDownField[]; // #108791#

// die Block-Groessen fuer die einzelnen Stream (Lesen/Schreiben)
#define SW3_BSR_STYLES 16384
#define SW3_BSW_STYLES 16384
#define SW3_BSR_NUMRULES 16384
#define SW3_BSW_NUMRULES 16384
#define SW3_BSR_PAGESTYLES 16384
#define SW3_BSW_PAGESTYLES 16384
#define SW3_BSR_DRAWING 16384
#define SW3_BSW_DRAWING 16384
#define SW3_BSR_CONTENTS 32768
#define SW3_BSW_CONTENTS 32768
// Contents-Stream, wenn nur der Header gelesen wird oder es ein
// Textbaustein-Stream ohne Formatierung ist
#define SW3_BSR_CONTENTS_HEADER 120
#define SW3_BSR_CONTENTS_FLAT 2048
#define SW3_BSW_CONTENTS_FLAT 2048
#define SW3_BSR_BLKDIR 8192
#define SW3_BSW_BLKDIR 8192

#define STRING_MAXLEN52 STRING_MAXLEN

typedef ::std::list< SdrObject * > SwHiddenDrawObjList_Impl;

class Sw3String : public String
{
    USHORT nPoolId;

// OPT: Cache fuer Formate im StringPool
    SwFmt *pCachedFmt;  // Opt. fuer Lesen: Id zugeordnetes Format
// /OPT: Cache fuer Formate im StringPool

public:
    Sw3String( const String& r, USHORT n = 0 ) :
        String( r ), nPoolId( n ), pCachedFmt( 0 ) {}
    USHORT GetPoolId() const { return nPoolId; }

// OPT: Cache fuer Formate im StringPool
    void SetCachedFmt( SwFmt *pFmt ) { pCachedFmt = pFmt; }
    SwFmt *GetCachedFmt() const { return pCachedFmt; }
// /OPT: Cache fuer Formate im StringPool

};

class Sw3NumRuleInfo
{
    String aOrigName;   // Name der Rule im File.
    String aNewName;    // (ggf. neuer ) Name der Rule im Dokument

    BOOL bUsed;

public:

    Sw3NumRuleInfo( const String& rOrigName, const String& rNewName ) :
        aOrigName( rOrigName ), aNewName( rNewName ), bUsed( FALSE ) {}

    Sw3NumRuleInfo( const String& rOrigName ) :
        aOrigName( rOrigName ), aNewName( rOrigName ), bUsed( FALSE ) {}

    void SetUsed() { bUsed = TRUE; }
    BOOL IsUsed() const { return bUsed; }

    const String& GetOrigName() const { return aOrigName; }
    const String& GetNewName() const { return aNewName; }

    inline int operator==( const Sw3NumRuleInfo& rInfo );
    inline int operator<( const Sw3NumRuleInfo& rInfo );
};

inline int Sw3NumRuleInfo::operator==( const Sw3NumRuleInfo& rInfo )
{
    return aOrigName == rInfo.aOrigName;
}

inline int Sw3NumRuleInfo::operator<( const Sw3NumRuleInfo& rInfo )
{
    return aOrigName < rInfo.aOrigName;
}

SV_DECL_PTRARR_DEL(Sw3TOXs,Sw3TOXBase*,16,16)
typedef SwBookmark* SwBookmarkPtr;
SV_DECL_PTRARR(Sw3Bookmarks,SwBookmarkPtr,16,16)
typedef Sw3String* Sw3StringPtr;
SV_DECL_PTRARR_DEL(Sw3Strings,Sw3StringPtr,16,16)
typedef Sw3NumRuleInfo *Sw3NumRuleInfoPtr;
SV_DECL_PTRARR_SORT(Sw3NumRuleInfos,Sw3NumRuleInfoPtr,16,16)
typedef SwFrmFmt *SwFrmFmtPtr;
SV_DECL_PTRARR(Sw3FrmFmts, SwFrmFmtPtr,16,16)
typedef SwRedline *SwRedlinePtr;
SV_DECL_PTRARR(Sw3Redlines,SwRedlinePtr,16,16)
SV_DECL_VARARR(Sw3ULongs,UINT32,32,32)
SV_DECL_VARARR(Sw3Bytes,BYTE,32,32)

typedef SwFmt *SwFmtPtr;
SV_DECL_PTRARR_SORT(Sw3SortFmts,SwFmtPtr,16,16)

#define SW3IO_CONV_FROM_MATH 1
#define SW3IO_CONV_FROM_BATS 2
#define SW3IO_CONV_TO_SYMBOL 4
class Sw3Fmts
{
    Sw3SortFmts aFmts;
    SvBytes     aFlags;

public:

    Sw3Fmts() {}

    void Insert( SwFmt *pFmt, BYTE nFlags )
    {
        USHORT i;
        aFmts.Insert( pFmt, i );
        aFlags.Insert( nFlags, i );
    }

    SwFmt *GetFmt( USHORT i ) { return aFmts[i]; }
    BYTE GetFlags( USHORT i ) { return aFlags[i]; }

    BYTE GetFlags( const SwFmt *pFmt )
    {
        BYTE nFlags = 0;
        USHORT nPos;
        if( aFmts.Seek_Entry( (SwFmt *)pFmt, &nPos ) )
            nFlags = aFlags[nPos];

        return nFlags;
    }


    USHORT Count() const { return aFmts.Count(); }
};

struct Sw3ExportInfo;
class Sw3Marks;

class Sw3StringPool
{
    String aEmpty;
    Sw3Strings aPool;
    long nExpFFVersion; // Export: FF-Version (SOFFICE_FILEFORMAT_??)

    BOOL bFixed;
    void RemoveExtension( SwFmt& );

public:
    Sw3StringPool();
    USHORT Count() { return aPool.Count(); }
    void   Clear();
    void   RemoveExtensions( SwDoc& );
    static void RemoveExtension( String& );
    const  String& Find( USHORT );
    USHORT FindPoolId( USHORT );

// OPT: Cache fuer Formate im StringPool
    void   SetCachedFmt( USHORT, SwFmt* );
    SwFmt *FindCachedFmt( USHORT ) const;
// /OPT: Cache fuer Formate im StringPool

    void   LoadOld( SvStream& );
    void   Load( SvStream&, USHORT nVersion );
    void   Store( SvStream& ) {}

    static USHORT ConvertFromOldPoolId( USHORT nId, USHORT nVersion );
    static USHORT ConvertToOldPoolId( USHORT nId, sal_uInt32 nFFVersion );
};

// Informationen, die nur beim Export eines Doks benoetigt werden
struct Sw3ExportInfo
{
    // Wenn nichts anderes angegeben ist, werden die Infos beim Export
    // immer gefuellt.

    const SfxItemSet    *pItemSet;  // der gerade exportierte Item-Set
    const SwFlyFrm      *pFlyFrm;   // der aktuelle Fly-Frame

    // Namen der Line- und Box-Formate im StrPool (nur fuer 3.1-/4.0-Export)
    SvStringsDtor       *pTblLineBoxFmtNames40;
    Sw3FrmFmts          *pTblLineBoxFmts40;

    BOOL bFlyFrmFmt : 1;            // es wird ein Fly-Frame-Format exportiert
    BOOL bDrwFrmFmt31 : 1;          // es wird ein zeichen-geb. Zeichenobjekt
                                    // als absatz-gebundenes Objekt exportiert
                                    // (nur 3.1-Export)

    Sw3ExportInfo() : pItemSet( 0 ), pFlyFrm( 0 ),
                      pTblLineBoxFmtNames40( 0 ), pTblLineBoxFmts40( 0 ),
                      bFlyFrmFmt( FALSE ), bDrwFrmFmt31( FALSE ) {}
    ~Sw3ExportInfo();
};

class Sw3IoImp
{                                   // I/O fuer Records:
    Sw3Bytes           aRecTypes;   // Satztyp-Stack
    Sw3ULongs          aRecSizes;   // Satzanfang/Satzlaengen-Stack
    Sw3ULongs          aValPositions;// Stack fuer UINT16-Werte
    ULONG              nFlagRecEnd; // Ende eines Flag-gesteuerten Datenbereichs
    BOOL               bOut;        // TRUE: Output Mode fuer Records
    String             aDefWordDelim;   // Word Delimiter vom SwModul
    Sw3RecordSizeTable *pRecSizes;  // Tabelle fuer Records > 16MB
    FontToSubsFontConverter hBatsFontConv;
    FontToSubsFontConverter hMathFontConv;

    void               SetDBName();
    void               MakeBlockText( const ByteString& rText );
    sal_Char ConvStarSymbolCharToStarBats( sal_Unicode c );
    sal_Unicode ConvStarBatsCharToStarSymbol( sal_Char c );
    sal_Unicode ConvStarMathCharToStarSymbol( sal_Char c );
    sal_Bool ConvertText( ByteString& rText8, String& rText,
                    xub_StrLen nStart, xub_StrLen nEnd,
                    xub_StrLen nOffset, const SwTxtNode& rNd,
                    rtl_TextEncoding eEnc,
                    const SvxFontItem& rFontItem,
                    SwInsHardBlankSoftHyph* pHBSH, BOOL bTo8 );
    void ConvertText( ByteString& rText8, String& rText,
                      xub_StrLen nOffset, SwTxtNode& rNd,
                      rtl_TextEncoding eEnc, const SvxFontItem& rFontItem,
                      SwInsHardBlankSoftHyph* pHBSH, BOOL bTo8 );

public:
    const String N_DOC;             // Name des Dokument-Streams
    const String N_PAGESTYLES;      // Die Seitenvorlagen
    const String N_NUMRULES;        // Name des NumRules-Streams
    const String N_DRAWING;         // Der Drawing Layer
    const String N_PICTURES;        // Name des Grafiken-Storages
    const String N_BLOCKDIR;        // Name des Block-Directories
    const String sStarSymbol;
    const String sOpenSymbol;
    const String sStarBats;
    const String sStarMath;

    Sw3Io&             rIo;         // kann spaeter mal wech
    SwDoc*             pDoc;        // das Dokument
    Sw3ExportInfo*     pExportInfo; // Infos fuer SW3.1-Export
    SvStorageRef       pOldRoot;    // zwischengespeicherter Storage
    SvStorageRef       pBlkRoot;    // Root-Storage fuer alle Textbausteine
    SvStorageRef       pRoot;       // logischer Root-Storage
    SvStorageStreamRef pStyles;     // Styles-Stream
    SvStorageStreamRef pPageStyles; // Seitenvorlagen-Stream
    SvStorageStreamRef pNumRules;   // NumRules-Stream
    SvStorageStreamRef pDrawing;    // Drawing Layer
    SvStorageStreamRef pContents;   // Inhalt
    SvStorageStreamRef pBlkDir;     // Block-Directory
    SvStream*          pStrm;       // der aktuelle Stream
    SvStorageInfoList* pBlkList;    // Liste aller Textbausteine
    SvPersistRef       pPersist;    // SvPersist fuer Textbausteine
    short              nCurBlk;     // Enumerierungswert
    short              nCurMark;    // Enum-Wert fuer Bookmarks
    Sw3StringPool      aStringPool; // der String-Pool eines Dokuments
    Sw3NumRuleInfos    aNumRuleInfos; // Namen der NumRules eines Doks.
    rtl_TextEncoding   eSrcSet;     // CharSet des Quell-Streams
    rtl_TextEncoding   eBlkDirSet;  // CharSet des Block-Directories
    SwDocStat          aStat;       // wird beim Schreiben generiert
    String             aBlkName;    // Name eines Textbausteins
    String             aINetFldText;// Text eines INet-Feldes
    ULONG              nRes;        // Result Code
    ULONG              nWarn;       // Result-Code fuer Warnungen
    USHORT             nGblFlags;   // globale Flags
#define SW3F_NOHDRFMT  0x0001       // Header-Format nicht ausgeben
#define SW3F_NOFTRFMT  0x0002       // Footer-Format nicht ausgeben
#define SW3F_SHAREDFMT 0x0004       // ein Shared-Fmt wird eingelesen
#define SW3F_UPDEXPR   0x0008       // nach dem Einlesen UpdateExpr() rufen
#define SW3F_NODRAWING 0x0010       // keine Zeichenobjekte laden
#define SW3F_RDTABLE   0x0020       // es wird eine Tabelle gelesen
#define SW3F_CONVBLOCK 0x0040       // Konversion SW2-Textbausteine zu SW3
#define SW3F_NOROOTCOMMIT 0x0100    // no commit on Block-Root

    USHORT             nFileFlags;  // dateispezifische Flags
    USHORT             nVersion;    // Versionsnummer
    BYTE               cPasswd[16]; // Passwort (codiert)
    USHORT             nFlyLevel;   // != 0, falls ein FlyFmt eingelesen wird
    ULONG              nZOrderOff;  // Offset fuer Z-Order bei Insert Drawings
    ULONG              nHiddenDrawObjs; // Anzahl versteckter Zeichen-Objekte
    SwTable*           pCurTbl;     // aktuell ausgegebene Tabelle
    SwNumRule*         pCurNumRule; // aktuelle Numerierungsregel
    SwPaM*             pCurNumRange;// aktueller Numerierungsbereich
    SwPaM*             pCurPaM;     // aktueller PaM
    Sw3TOXs*           pTOXs;       // I: Liste aller TOX-Bereiche
    Sw3Bookmarks*      pBookmarks;  // I: Liste aller Bookmarks
    Sw3Redlines*       pRedlines;   // I: Liste alle Bookmarks
    Sw3Marks*          pRedlineMarks; // I: Positionen der Redlines
    Sw3Marks*          pMarks;      // alle TOXe und Bookmarks (Positionen)
    Crypter*           pCrypter;    // Encrypter, falls mit Passwort
    SwOLENodes*        p30OLENodes; // OLE-Nodes, die vom SW3.0 angelegt wurd.
    SwFmtINetFmt*      pFmtINetFmt; // aus INetFld hervorgeganges Attribut
    Sw3FrmFmts*        pTblLineBoxFmts; // shared Line-/Box-Formate
    SvUShorts*         pAuthorityMap;
    SvUShorts*         pSectionDepths;
    Sw3Fmts*           pConvToSymbolFmts;
    SwHiddenDrawObjList_Impl *pHiddenDrawObjs;
    sal_uInt32         nCurPercent; // Aktueller Stand der Prozentanzeige
    sal_uInt32         nEndPercent; // Maximalwert der Prozentanzeige
    UINT32             nDate,nTime; // Zeitpunkt der Speicherung
    long               nSizeDivFac; // Divisionsfaktor fuer FRMSIZE-Attribs

    ULONG              nStatStart;  // Start der DocStatistik
    USHORT             eStartNodeType;  // fuers erzeugen von StartNodes
                                        // Fly/Footer/Header!
    USHORT             nCntntBkmkStart;
    USHORT             nCntntRedlineStart;


    // Was bedeuten die Modus-Flags, oder: nie wieder Nachdenken muessen
    // bNormal    - ist gesetzt, wenn
    //                  - Dokumente geladen, oder
    //                  - Dokumente eingefuegt werden, oder
    //                  - Vorlagen ueber den Organizer (LoadStyles) geladen
    //                    werden,
    //              aber nicht gesetzt, wenn Vorlagen (ueber Load) geladen
    //              werden. Wenn zusaetzlich noch bInsert gesetzt ist,
    //              wird eingefuegt. Wenn bOrganizer gesetzt ist, werden
    //              Vorlagen ueber den Organizer (LoadStyles) geladen .
    // bInsert    - ist gesetzt, wenn Dokumente eingefuegt werden.
    //              Gleichzeitig ist immer auch bAdditive gesetzt.
    // bAdditive  - ist gesetzt, wenn Dokumente eingefuegt werden oder wenn
    //              Vorlagen (ueber Load) geladen werden, ohne bestehende zu
    //              ueberschreiben.
    // bTxtColls, - sind gesetzt, wenn der entsprechene Vorlagen-Typ (ueber
    // bCharFmts,   Load) geladen wird. bNormal ist nicht gesetzt, wenn eines
    // bFrmFmts,    dieser Falgs gesetzt ist. bAdditive gibt an, ob nur nicht
    // bPageDescs,  vorhandene (TRUE) Vorlagen gelesen werden sollen oder ob
    // bNumRules    vorhandene Vorlagen ueberschrieben werden sollen (FALSE).
    // bOrganizer - ist gesetzt, wenn Vorlagen ueber LoadStyles (Organizer)
    //              geladen werden. In diesem Fall ist auch bNormal gesetzt.

    BOOL               bInsert;     // TRUE: in Doc einfuegen
    BOOL               bNormal;     // TRUE: normales Einlesen
    BOOL               bTxtColls;   // TRUE: Absatzvorlagen
    BOOL               bCharFmts;   // TRUE: Zeichenvorlagen
    BOOL               bFrmFmts;    // TRUE: Rahmenvorlagen
    BOOL               bPageDescs;  // TRUE: Seitenvorlagen
    BOOL               bNumRules;   // TRUE: Numerierungsvorlagen
    BOOL               bAdditive;   // TRUE: vorhandene Vorlagen nicht ersetzen
    BOOL               bNoDrawings; // TRUE: drwing layer is corrupt
    BOOL               bBlock;      // TRUE: Textbausteine
    BOOL               bSw31Export; // TRUE: Sw31-Export
    BOOL               bOrganizer;  // TRUE: es wurde ist ein LoadStyle
    BOOL               bInsIntoHdrFtr; // TRUE: Es wird in Kopf/Fusszeilen
                                       // eingefuegt
    BOOL               bDrawFmtSkipped; // TRUE: Es wurde ein DrawFmt
                                        // uebersprungen
    BOOL               bConvertNoNum; // TRUE: NO_NUM in NO_NUMLEVEL konvert.

    BOOL               bSaveAll;    // TRUE: das ganze Doc wird gespeichert

    BOOL               bSpellAllAgain;      // TRUE: set all TxtNode as dirty
    BOOL               bSpellWrongAgain;    // TRUE: set all WrongList as dirty

#ifdef DBG_UTIL
    BYTE              *pRefSdrObjects;
#endif

    Sw3IoImp( Sw3Io& );
   ~Sw3IoImp();
    static Sw3IoImp* GetCurrentIo();
    void Reset();
    void Reset2();
    void SetReadOptions( const SwgReaderOption&, BOOL );
    void SetSw31Export( BOOL b31 ) { bSw31Export = b31; }
    BOOL IsSw31Export() const { return bSw31Export; }
#ifndef DBG_UTIL
    inline BOOL IsSw31Or40Export() const;
#else
    BOOL IsSw31Or40Export() const;
#endif
    void Error( ULONG = 0 );
    void Warning( ULONG = 0 );

    BOOL OpenStreams( BOOL bRdWr = TRUE, BOOL bUseDrawStream = TRUE );
    void CloseStreams();
    ULONG OpenStreamsForScan( SvStorage *pStor, BOOL bPageStyles );
    void CloseStreamsForScan();
    BOOL CheckStreams();
    BOOL CheckHeader( sal_Char *pHeader );
    BOOL Good()     { return BOOL( pStrm->GetError() == SVSTREAM_OK ); }
    BYTE Peek();                    // 1 Byte peeken
    void OutputMode( BOOL b )       { bOut = b; }
    BOOL OpenRec( BYTE cType );     // Record oeffnen
    void CloseRec( BYTE cType );    // Record schliessen
    void SkipRec();                 // Record uebergehen
    void InsertRecordSize( sal_uInt32 nPos, sal_uInt32 nSize );
    sal_uInt32 GetRecordSize( sal_uInt32 nPos );
    BOOL HasRecSizes() const { return pRecSizes != 0; }
    void FlushRecSizes();
    void InRecSizes( ULONG nRecPos );
    BYTE OpenFlagRec();             // Endeposition eines Flag-Records merken
    void CloseFlagRec();            // Flag-Record schliessen
    ULONG BytesLeft();              // wie viele Bytes hat der Record noch?
    void CheckIoError( SvStream*);  // korrekten E/A-Fehlercode setzen
    static String ConvertStringNoDbDelim( const ByteString& rStr,
                                          rtl_TextEncoding eSource );
    static String ConvertStringNoDelim( const ByteString& rStr,
                                        sal_Char cSrcDelim,
                                        sal_Unicode cDelim,
                                        rtl_TextEncoding eSource );
    static sal_uInt32 InULong( SvStream& );            // ULONG komprimiert lesen
    inline SvStream& InString( SvStream& rStrm, String& rStr );
    inline SvStream& OutString( SvStream& rStrm, const String& rStr );

    void AddTblLineBoxFmt( SwFrmFmt *pFmt );
    SwFrmFmt *GetTblLineBoxFmt( USHORT nIdx );

    // Ist eingelene Datei neuer oder gleich nMinVers und aelter
    // (und ungleeich) nMaxVers?
    inline BOOL IsVersion( USHORT nMinVers ) const;
    inline BOOL IsVersion( USHORT nMinVers, USHORT nMaxVers ) const;
    inline BOOL IsVersion( USHORT nMinVers1, USHORT nMaxVers1,
                           USHORT nMinVers2 ) const;
    inline BOOL IsVersion( USHORT nMinVers1, USHORT nMaxVers1,
                           USHORT nMinVers2, USHORT nMaxVers2 ) const;

    void OpenPercentBar( sal_uInt32, sal_uInt32 );
    void ClosePercentBar();
    void SetPercentBar( sal_uInt32 );

    void Cleanup( BOOL bConnectPageDescs = TRUE );  // Nach Einlesen aufraeumen
    void ChangeFontItemCharSet();
    void ConvertFmtsToStarSymbol();

    void LoadDrawingLayer();
    void SaveDrawingLayer();

    void LoadNumRules();
    void SaveNumRules( BOOL = FALSE );

    void LoadPageStyles();
    void SavePageStyles( BOOL = FALSE );

    void LoadContents( SwPaM* );
    void SaveContents( SwPaM&, const String* = NULL );

    void LoadMacros();
    void SaveMacros();

    void RemoveUnusedObjects();             // unbenutzte Objekte entfernen

    // Passwort-Checks
    BOOL  CheckPasswd();                    // I: Passwort testen
    void  SetPasswd();                      // O: Passwort setzen

                                            // SW3STYLE.CXX
    void LoadStyleSheets( BOOL bNew );      // I: StyleSheets

                                            // SW3BLOCK.CXX
    void DetectAndSetFFVersion( SvStorage *pRoot );
    void InitBlockMode( SvStorage*, BOOL );
    void ResetBlockMode();
    ULONG FirstBlockName( String& rShort, String& rLong );
    ULONG NextBlockName( String& rShort, String& rLong );
    ULONG GetBlock( const String& rShort, sal_Bool bConvertMode = sal_False );
    ULONG GetBlockText( const String& rShort, String& rText );
    BOOL   CheckPersist();                  // Doc mit SvPersist versehen
    void   ClearPersist();                  // SvPersist freigeben
    ULONG GetBlockMacroTable( const String& rShort,
                          SvxMacroTableDtor& rMacroTbl );

                                            // SW3DOC.CXX
    void   LoadDocContents( SwPaM* );       // I: Dokumentinhalt
    USHORT GetStreamFlags();                // I: Stream-Flags laden
    void   InHeader( BOOL bReadRecSizes=FALSE); // I: Dateikopf
    BOOL   InHeaderForScan( BOOL bReadRecSizes=FALSE );

                                            // SW3FIELD.CXX
    SwField* InField();                     // I: Feld
    SwFieldType* InFieldType();             // I: ein Feldtyp

                                            // SW3FMTS.CXX
    SfxPoolItem* InAttr( xub_StrLen&, xub_StrLen&,
                         const SwTxtNode *pTxtNd=0 );// I: Attribut
    void   InAttrSet( SwAttrSet& rSet );    // I: AttrSet
    SwFmt* InFormat( BYTE cKind, SwFmt* );  // I: Format
    void   InFlyFrames();                   // I: globale FlyFrames

                                            // SW3MISC.CXX
    SwFmt* FindFmt( USHORT nIdx, BYTE c );  // Suchen eines Formats per StringId
    SwFmt* FindNamedFmt( USHORT nIdx, BYTE);// Suchen einer Formatvorlage
    SwTxtFmtColl* FindTxtColl( USHORT n );  // Suchen einer Absatzvorlage
    SwPageDesc* FindPageDesc( USHORT nIdx );// Suchen einer Seitenvorlage
    void   InMacroTbl();                    // I: globale Makros
    void   InDictionary();                  // I: Woerterbuecher (Online-Spl.)
    void   InJobSetup();                    // I: Job Setup
    void   InDBName();                      // I: Datenbankname
    void   InStringPool( BYTE, Sw3StringPool& );
    void   InPasswd();                      // I: Passwort
    void   SetPasswd( const String& rPass );// IO: Passwort setzen
                                            // O: TOX- und Bookmark-Bereiche suchen
    void   InNodeMark( const SwNodeIndex&, xub_StrLen nOffset );    // I: Markierung
    void   InBookmarks();                   // I: Bookmarks
    void   InTOXs51();                      // I: TOX-Bereiche
    void   InTOXs();                        // I: TOX-Bereiche
    void   CleanupMarks();                  // I: nicht ben. Marks loeschen
    void   ConnectTOXs();                   // I: insert TOXs

                                            // SW3NODES.CXX
    void   ConvertText( SwTxtNode& rNd, const ByteString& rText8,
                        xub_StrLen, SvUShorts*, SvXub_StrLens* );   // I: Zeichensatz-Konversion
    void   InTxtNode( SwTxtNode*, SwNodeIndex&, xub_StrLen, BYTE = 0 );
                                            // I: hartes Attribut
    void   InTxtAttr( SwTxtNode&, const ByteString& rText8,
                      xub_StrLen, SvStringsDtor**, SvXub_StrLens**,
                      SvXub_StrLens**, SvUShorts**, SvXub_StrLens** );
    void   InGrfNode( SwNodeIndex& rPos );      // I: Grafik-Node
    void   InOLENode( SwNodeIndex& rPos );      // I: OLE-Node
    void   InRepTxtNode( SwNodeIndex& );        // I: Textwiederholung

                                            // I/O: ImageMap-Infos
    ImageMap *InImageMap( String& rURL, String& rTarget, BOOL& rIsMap );

                                            // I/O: Contour
    PolyPolygon *InContour();

                                            // SW3NUM.CXX
    void   InNumFmt( SwNumFmt& rFmt );      // I: Numerierungs-Format
    SwNumRule* InNumRule( BYTE );           // I: Numerierungs-Regelwerk
    void   InNumRules();                    // I: NumRules
    void   InOutlineExt();
    void   InNodeNum( SwNodeNum& );         // I: Absatz-Numerierung

    void   InEndNoteInfo( SwEndNoteInfo &rENInf ); // I: globale Endnoten-Info
    void   InEndNoteInfo();                 // I: globale Endnoten-Info
    void   InFtnInfo();                     // I: globale Fussnoten-Info
    void   InFtnInfo40();                   // I: globale Fussn.-Info 3.1/4.0
    void   OpenNumRange40( const SwNodeIndex& ); // I: Numerierungsregel-Beginn
    void   CloseNumRange40( const SwNodeIndex& ); // I: Numerierungsregel-Ende

                                            // SW3PAGE.CXX
    void   InPageFtnInfo( SwPageFtnInfo& ); // I: Fussnoten-Info
    void   InPageDescs();                   // I: alle Seitenvorlagen
    SwPageDesc* InPageDesc( USHORT& );      // I: Seitenvorlage
    void   ConnectPageDescAttrs();          // I: PageDesc-Referenzen aufloesen

                                            // SW3SECTN.CXX
    void   InContents( SwNodeIndex&, xub_StrLen=0, BOOL=TRUE, BYTE=0, BOOL=FALSE );
    SwStartNode& InContents();              // I: neue Section anlegen
    void   InSection( SwNodeIndex& );           // I: SwSection einlesen
                                            // SW3TABLE.CXX
    void   InTable( SwNodeIndex& rPos );        // I: Tabelle
    void   InTableLine( SwTableLines&, SwTableBox*, USHORT, SwNodeIndex& );
    void   InTableBox( SwTableBoxes&, USHORT, SwTableLine*, SwNodeIndex& );

                                            // SW3REDLIN.CXX
/*N*/   void InRedline(); //SW50.SDW                        // I: Redline
/*N*/   void InRedlines(); //SW50.SDW
/*N*/   void InNodeRedline( const SwNodeIndex& rNodeIdx, INT32& nOffset, BYTE=0 ); //SW50.SDW
    void CleanupRedlines();

    void InDocStat();

    void InNumberFormatter();

    void InLineNumberInfo();

    void InDocDummies();

    void InPagePreViewPrintData();

    // die folgenden Methoden sind zum Suchen von Sections
    void GetSectionList( SvStrings& rSectionList, SvStringsDtor& rBookmarks );
    void ScanContents( SvStrings& rSectionList, SvStringsDtor& rBookmarks );
    void ScanTableLine( SvStrings& rSectionList, SvStringsDtor& rBookmarks );

    void GetMacroTable( SvxMacroTableDtor& rMacroTbl );
    void ScanMacroTbl( SvxMacroTableDtor& rMacroTbl );
};

inline BOOL Sw3IoImp::IsVersion( USHORT nMinVers ) const
{
    return nVersion >= nMinVers;
}

inline BOOL Sw3IoImp::IsVersion( USHORT nMinVers, USHORT nMaxVers ) const
{
    return nVersion >= nMinVers && nVersion < nMaxVers;
}

inline BOOL Sw3IoImp::IsVersion( USHORT nMinVers1, USHORT nMaxVers1,
                                 USHORT nMinVers2 ) const
{
    return (nVersion >= nMinVers1 && nVersion < nMaxVers1) ||
            nVersion >= nMinVers2;
}

inline BOOL Sw3IoImp::IsVersion( USHORT nMinVers1, USHORT nMaxVers1,
                                 USHORT nMinVers2, USHORT nMaxVers2 ) const
{
    return (nVersion >= nMinVers1 && nVersion < nMaxVers1) ||
           (nVersion >= nMinVers2 && nVersion < nMaxVers2);
}

#ifndef DBG_UTIL
inline BOOL Sw3IoImp::IsSw31Or40Export() const
{
    return pRoot->GetVersion() <= SOFFICE_FILEFORMAT_40;
}
#endif

inline SvStream& Sw3IoImp::InString( SvStream& rStrm, String& rStr )
{
    rStr = rStrm.ReadUniOrByteString( eSrcSet );
    return rStrm;
}

inline SvStream& Sw3IoImp::OutString( SvStream& rStrm, const String& rStr )
{
    return rStrm.WriteUniOrByteString( rStr, eSrcSet );
}

void lcl_sw3io__ConvertNumLRSpace( SwTxtNode& rTxtNd, const SwNumRule& rNumRule,
                                   BYTE nLevel, BOOL bTabStop );
void lcl_sw3io__ConvertNumTabStop( SwTxtNode& rTxtNd, long nOffset,
                                   BOOL bDeep );
void lcl_sw3io__ConvertNumTabStop( SvxTabStopItem& rTStop, long nOffset );


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
