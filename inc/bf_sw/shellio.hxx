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
#ifndef _SHELLIO_HXX
#define _SHELLIO_HXX

#include <bf_svtools/bf_solar.h>
#include <bf_tools/string.hxx>

#include <tools/datetime.hxx>
#include <tools/ref.hxx>
#include <sot/formats.hxx>

#include <docfac.hxx>   // SwDocFac
#include <tools/debug.hxx>
class SvStream;
namespace binfilter {
class SvStorage;
class SvStorageStreamRef;
class SvPtrarr;
class SfxItemPool;
class SfxItemSet;
class SvStrings;
class SvStringsSortDtor;
class SvxMacroTableDtor;

// einige Forward - Deklarationen
class SfxFactoryFilterContainer;
class SfxFilter;
class SfxMedium;
class SvxFontItem;

class Sw3Io;
class SwCntntNode;
class SwCrsrShell;
class SwDoc;
class SwPaM;
struct SwPosition;

// ab so vielen chars wird ein mit einem ASCII/W4W-Reader eingelesener
// Absatz zwangsweise umgebrochen. Muss immer groesser als 200 sein !!!
#define MAX_ASCII_PARA 10000


class SwAsciiOptions
{
    String sFont;
    rtl_TextEncoding eCharSet;
    USHORT nLanguage;
    LineEnd eCRLF_Flag;

public:

    const String& GetFontName() const { return sFont; }
    void SetFontName( const String& rFont ) { sFont = rFont; }

    rtl_TextEncoding GetCharSet() const { return eCharSet; }
    void SetCharSet( rtl_TextEncoding nVal ) { eCharSet = nVal; }

    USHORT GetLanguage() const { return nLanguage; }
    void SetLanguage( USHORT nVal ) { nLanguage = nVal; }

    LineEnd GetParaFlags() const { return eCRLF_Flag; }
    void SetParaFlags( LineEnd eVal ) { eCRLF_Flag = eVal; }

    void Reset()
    {
        sFont.Erase();
        eCRLF_Flag = GetSystemLineEnd();
        eCharSet = ::osl_getThreadTextEncoding();
        nLanguage = 0;
    }
    // for the automatic conversion (mail/news/...)
    void ReadUserData( const String& );

    SwAsciiOptions() { Reset(); }
};


/* Basis-Klasse aller Writer */

struct Writer_Impl;

class Writer : public SvRefBase
{
    SwAsciiOptions aAscOpts;

protected:
    Writer_Impl* pImpl;

    SvStream* pStrm;
    SwPaM* pOrigPam;            // der letze zu bearbeitende Pam

    void ResetWriter();
    BOOL CopyNextPam( SwPaM ** );

    virtual ULONG WriteStream() = 0;

public:
    SwDoc* pDoc;
    SwPaM* pCurPam;
    BOOL bWriteAll : 1;
    BOOL bShowProgress : 1;
    BOOL bWriteClipboardDoc : 1;
    BOOL bWriteOnlyFirstTable : 1;
    BOOL bASCII_ParaAsCR : 1;
    BOOL bASCII_ParaAsBlanc : 1;
    BOOL bASCII_NoLastLineEnd : 1;
    BOOL bUCS2_WithStartChar : 1;

    BOOL bBlock : 1;
    BOOL bOrganizerMode : 1;

    Writer();
    virtual ~Writer();

    ULONG Write( SwPaM&, SvStream& );

    BOOL ShowProgress() const                   { return bShowProgress; }
    void SetShowProgress( BOOL bFlag = FALSE )  { bShowProgress = bFlag; }

    const SwAsciiOptions& GetAsciiOptions() const { return aAscOpts; }
    void SetAsciiOptions( const SwAsciiOptions& rOpt ) { aAscOpts = rOpt; }

    void SetStrm( SvStream& rStrm ) { pStrm = &rStrm; }
#ifndef DBG_UTIL
    SvStream& Strm() { return *pStrm; }
#else
    SvStream& Strm();
#endif

    BOOL IsOrganizerMode() const { return bOrganizerMode; }
    void SetOrganizerMode( BOOL bSet ) { bOrganizerMode = bSet; }
};

#ifndef SW_DECL_WRITER_DEFINED
#define SW_DECL_WRITER_DEFINED
SV_DECL_REF(Writer)
#endif
SV_IMPL_REF(Writer)

/**************** SwReader/Reader ************************/
// Basisklasse der moeglichen Optionen fuer einen speziellen Reader
class Reader;
// Ruft den Reader mit seinen Optionen, Dokument, Cursor etc.
class SwReader;
// SwRead ist der Pointer auf die Read-Optionen-Basisklasse
typedef Reader *SwRead;

class SwgReaderOption
{
    SwAsciiOptions aASCIIOpts;
    union
    {
        BOOL bFmtsOnly;
        struct
        {
            BOOL bFrmFmts: 1;
            BOOL bPageDescs: 1;
            BOOL bTxtFmts: 1;
            BOOL bNumRules: 1;
            BOOL bMerge:1;
        }  Fmts;
    } What;

public:
    void ResetAllFmtsOnly() { What.bFmtsOnly = 0; }
    void SetAllFmtsOnly() { What.bFmtsOnly = (BOOL)0xff; }
    BOOL IsFmtsOnly() const { return What.bFmtsOnly; }

    BOOL IsFrmFmts() const { return What.Fmts.bFrmFmts; }
    void SetFrmFmts( const BOOL bNew) { What.Fmts.bFrmFmts = bNew; }

    BOOL IsPageDescs() const { return What.Fmts.bPageDescs; }
    void SetPageDescs( const BOOL bNew) { What.Fmts.bPageDescs = bNew; }

    BOOL IsTxtFmts() const { return What.Fmts.bTxtFmts; }
    void SetTxtFmts( const BOOL bNew) { What.Fmts.bTxtFmts = bNew; }

    BOOL IsNumRules() const { return What.Fmts.bNumRules; }
    void SetNumRules( const BOOL bNew) { What.Fmts.bNumRules = bNew; }

    BOOL IsMerge() const { return What.Fmts.bMerge; }
    void SetMerge( const BOOL bNew ) { What.Fmts.bMerge = bNew; }

    const SwAsciiOptions& GetASCIIOpts() const { return aASCIIOpts; }
    void SetASCIIOpts( const SwAsciiOptions& rOpts ) { aASCIIOpts = rOpts; }
    void ResetASCIIOpts() { aASCIIOpts.Reset(); }

    SwgReaderOption()
    {   ResetAllFmtsOnly(); aASCIIOpts.Reset(); }
};

class SwReader: public SwDocFac
{
    SvStream* pStrm;
    SvStorage* pStg;
    SfxMedium* pMedium;     // wer ein Medium haben will (W4W)

    SwPaM* pCrsr;
    String aFileName;

public:
    /*
     * Initiales Einlesen. Dokument wird erst beim Read(..) angelegt.
     * JP 25.04.95: oder falls es mitgegeben wird, in dieses.
     *              Sonderfall fuer Load mit Sw3Reader
     */
    SwReader( SvStorage&, const String& rFilename, SwDoc *pDoc = 0 );
    SwReader( SfxMedium&, const String& rFilename, SwDoc *pDoc = 0 );
    /*
     * In ein existierendes Dokument einlesen, Dokument und
     * Position im Dokument werden aus dem SwPaM uebernommen.
     */
    SwReader( SfxMedium&, const String& rFilename, SwPaM& );

    /*
     * Nur SwReader::Read(...) ist die Export-Schnittstelle!!!
     */
    BOOL NeedsPasswd( const Reader& );
    BOOL CheckPasswd( const String&, const Reader& );
    ULONG Read( const Reader& );

    // ask for glossaries
};


/****************  SPEZIELLE Reader ************************/

// spezielle - Reader koennen beides sein !! (Excel, W4W, .. )
#define SW_STREAM_READER    1
#define SW_STORAGE_READER   2

class Reader
{
    friend class SwReader;
    SwDoc* pTemplate;
    String aTemplateNm;
    Date aDStamp;
    Time aTStamp;
    DateTime aChkDateTime;

protected:
    SvStream* pStrm;
    SvStorage* pStg;
    SfxMedium* pMedium;     // wer ein Medium haben will (W4W)

    SwgReaderOption aOpt;
    BOOL bInsertMode : 1;
    BOOL bTmplBrowseMode : 1;
    BOOL bReadUTF8: 1;      // Stream als UTF-8 interpretieren
    BOOL bBlockMode: 1;
    BOOL bOrganizerMode : 1;
    BOOL bHasAskTemplateName : 1;
    BOOL bIgnoreHTMLComments : 1;

    virtual String GetTemplateName() const;

public:
    Reader();
    virtual ~Reader();

    virtual int GetReaderType();
    SwgReaderOption& GetReaderOpt() { return aOpt; }

    virtual void SetFltName( const String& rFltNm );
    static void SetNoOutlineNum( SwDoc& rDoc );

    // Die Filter-Vorlage laden, setzen und wieder freigeben
    SwDoc* GetTemplateDoc();
    BOOL SetTemplate( SwDoc& rDoc );
    void ClearTemplate();
    void SetTemplateName( const String& rDir );

    BOOL IsReadUTF8() const { return bReadUTF8; }
    void SetReadUTF8( BOOL bSet ) { bReadUTF8 = bSet; }

    BOOL IsBlockMode() const { return bBlockMode; }
    void SetBlockMode( BOOL bSet ) { bBlockMode = bSet; }

    BOOL IsOrganizerMode() const { return bOrganizerMode; }
    void SetOrganizerMode( BOOL bSet ) { bOrganizerMode = bSet; }

    BOOL IsIgnoreHTMLComments() const { return bIgnoreHTMLComments; }
    void SetIgnoreHTMLComments( BOOL bSet ) { bIgnoreHTMLComments = bSet; }


    // read the sections of the document, which is equal to the medium.
    // returns the count of it
private:
    virtual ULONG Read(SwDoc &,SwPaM &,const String &)=0;

    // alle die die Streams / Storages nicht geoeffnet brauchen,
    // muessen die Methode ueberladen (W4W!!)
    virtual int SetStrmStgPtr();
};



class AsciiReader: public Reader
{
    friend class SwReader;
    virtual ULONG Read( SwDoc &,SwPaM &,const String &){DBG_BF_ASSERT(0, "STRIP"); return 0;}
public:
    AsciiReader(): Reader() {}
};

class SwgReader: public Reader
{
    virtual ULONG Read( SwDoc &,SwPaM &,const String &);
};

class StgReader : public Reader
{
    String aFltName;

protected:
    ULONG OpenMainStream( SvStorageStreamRef& rRef, USHORT& rBuffSize );

public:
    const String& GetFltName() { return aFltName; }
};


class Sw3Reader : public StgReader
{
    Sw3Io* pIO;
    virtual ULONG Read( SwDoc &,SwPaM &,const String &);
public:
    Sw3Reader() : pIO( 0 ) {}

          Sw3Io* GetSw3Io()                 { return pIO; }
    const Sw3Io* GetSw3Io() const           { return pIO; }
          void   SetSw3Io( Sw3Io* pIo )     { pIO = pIo; }

    // read the sections of the document, which is equal to the medium.
    // returns the count of it
};

// Der uebergebene Stream muss dynamisch angelegt werden und
// vor dem Loeschen der Instanz per Stream() angefordert
// und geloescht werden!

class SwImpBlocks;

class SwTextBlocks
{
    friend class Sw2TextBlocks;
    friend class Sw3IoImp;
    SwImpBlocks* pImp;
    ULONG        nErr;

public:
    void Flush(){}

    ULONG GetError() const { return nErr; }
};


extern void _InitFilter();
extern void _FinitFilter();
extern SwRead ReadRtf, ReadAscii, ReadSwg, ReadSw3, ReadHTML;


/*
 * Schreiben, Writer
 */


// Die folgende Klasse ist ein Wrappe fuer die Basic-I/O-Funktionen
// des Writer 3.0. Alles ist statisch. Alle u.a. Filternamen sind die
// Writer-internen Namen, d.h. die namen, die in INSTALL.INI vor dem
// Gleichheitszeichen stehen, z.b. SWG oder ASCII.

class SwIoSystem
{
public:
        // suche ueber den Filtertext den Filtereintrag
    static const SfxFilter* GetFilterOfFilterTxt( const String& rFilterNm,
                                const SfxFactoryFilterContainer* pCnt = 0 );
        // suche ueber den internen FormatNamen den Filtereintrag
    static const SfxFilter* GetFilterOfFormat( const String& rFormat,
                                const SfxFactoryFilterContainer* pCnt = 0 );

    // Feststellen des zu verwendenden Filters fuer die uebergebene
    // Datei. Der Filtername wird zurueckgeliefert. Konnte kein Filter
    // zurueckgeliefert werden, wird der Name des ASCII-Filters geliefert!
    static const SfxFilter* GetFileFilter( const String& rFileName,
                                            SfxMedium* pMedium = 0 );

        // Feststellen ob das File in dem vorgegebenen Format vorliegt.
        // Z.z werden nur unsere eigene Filter unterstuetzt!!
    static bool IsFileFilter( SfxMedium& rMedium, const String& rFmtName,
                                    const SfxFilter** ppFlt = 0 );

    static bool IsValidStgFilter( SvStorage& , const SfxFilter& );

        static bool IsDetectableText(const sal_Char* pBuf, ULONG &rLen,
        CharSet *pCharSet=0, bool *pSwap=0, LineEnd *pLineEnd=0);
    static bool IsDetectableW4W(const String& rFileName);

    static const SfxFilter* GetTextFilter(const sal_Char* pBuf, ULONG nLen);
    // gebe einen bestimmten Reader zurueck
     static Reader* GetReader( const String& rFltName );

    static const String GetSubStorageName( const SfxFilter& rFltr );
};


// Schnittstellenklasse fuer den allgemeinen Zugriff auf die
// speziellen Writer

class SwWriter
{
    SvStream* pStrm;

    SwPaM* pOutPam;
    SwCrsrShell *pShell;
    SwDoc &rDoc;
    BOOL bWriteAll;

public:
    ULONG Write( WriterRef& rxWriter );
    SwWriter( SvStream&, SwPaM &, BOOL bWriteAll = FALSE );
};

WriterRef GetASCWriter( const String& rFltNm );


// diese Filter sind immer vorhanden und koennen ueber die
// Formatnamen gesucht werden. Alle anderen Filter sind nur intern
// bekannt. Die UI-Seite benutzt die GetReader()/GetWriter() -Funktionen,
// um die speziellen zu erhalten.

extern sal_Char FILTER_SWG[];   // SWG-Filter
extern sal_Char FILTER_RTF[];   // RTF-Filter
extern sal_Char FILTER_TEXT[];  // Text-Filter mit Default-CodeSet
extern sal_Char FILTER_BAS[];   // StarBasic (identisch mit ANSI)
extern sal_Char FILTER_WW8[];   // WinWord 97-Filter
extern sal_Char FILTER_SW3[];   // SW3-Storage Filter
extern sal_Char FILTER_SW4[];   // SW4-Storage Filter
extern sal_Char FILTER_SW4[];   // SW4-Storage Filter
extern sal_Char FILTER_SW5[];   // SW5-Storage Filter
extern sal_Char FILTER_SWGV[];  // SWG-Vorlagen Filter
extern sal_Char FILTER_SW3V[];  // SW3-Storage Vorlagen Filter
extern sal_Char FILTER_SW4V[];  // SW4-Storage Vorlagen Filter
extern sal_Char FILTER_SW5V[];  // SW5-Storage Vorlagen Filter
extern sal_Char FILTER_SWW4V[]; // SW/Web Storage Vorlagen Filter
extern sal_Char FILTER_SWW5V[]; // SW/Web Storage Vorlagen Filter
extern sal_Char FILTER_TEXT_DLG[];  // text filter with encoding dialog
extern sal_Char FILTER_XML[];   // XML filter
extern sal_Char FILTER_XMLV[];  // XML filter
extern sal_Char FILTER_XMLVW[]; // XML filter


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
