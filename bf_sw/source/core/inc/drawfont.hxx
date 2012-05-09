/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
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
#ifndef _DRAWFONT_HXX
#define _DRAWFONT_HXX

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_SHORTS
#define _SVSTDARR_BYTES
#define _SVSTDARR_USHORTS
#define _SVSTDARR_XUB_STRLEN
#include <bf_svtools/svstdarr.hxx>
#endif
#include <tools/gen.hxx>
#include <bf_tools/string.hxx>
#include <osl/diagnose.h>
#include <tools/fract.hxx>

#include <i18npool/lang.h>
class OutputDevice;
class Point;
class Size;
class Font;
namespace binfilter {

class SwWrongList;
class SwFont;

class ViewShell;
class SwTxtNode;
class SwAttrHandler;
class SwTxtFrm;
class SwUnderlineFont;

/*************************************************************************
 *                class SwScanner
 * Hilfsklasse, die beim Spellen die Worte im gewuenschten Bereich
 * nacheinander zur Verfuegung stellt.
 *************************************************************************/

class SwScanner
{
    XubString aWord;
    const SwWrongList* pWrong;
    const SwTxtNode& rNode;
    xub_StrLen nEndPos;
    xub_StrLen nBegin;
    xub_StrLen nLen;
    LanguageType aCurrLang;
    USHORT nWordType;
    BOOL bReverse;
    BOOL bStart;
    BOOL bIsOnlineSpell;

public:
    SwScanner( const SwTxtNode& rNd, const SwWrongList* pWrng, USHORT nWordType,
               xub_StrLen nStart, xub_StrLen nEnde, BOOL bRev, BOOL bOS );

    // This next word function tries to find the language for the next word
    // It should currently _not_ be used for spell checking, and works only for
    // ! bReverse
    BOOL NextWord();

    const XubString& GetWord() const    { return aWord; }

    xub_StrLen GetBegin() const         { return nBegin; }
    xub_StrLen GetEnd() const           { return nBegin + nLen; }
    xub_StrLen GetLen() const           { return nLen; }
};

/*************************************************************************
 *                      class SwScriptInfo
 *
 * encapsultes information about script changes
 *************************************************************************/

class SwScriptInfo
{
private:
    SvXub_StrLens aScriptChg;
    SvBytes aScriptType;
    SvXub_StrLens aDirChg;
    SvBytes aDirType;
    SvXub_StrLens aKashida;
    SvXub_StrLens aCompChg;
    SvXub_StrLens aCompLen;
    SvBytes aCompType;
    xub_StrLen nInvalidityPos;
    BYTE nDefaultDir;

    void UpdateBidiInfo( const String& rTxt );

public:
    enum CompType { KANA, SPECIAL_LEFT, SPECIAL_RIGHT, NONE };

    inline SwScriptInfo() : nInvalidityPos( 0 ), nDefaultDir( 0 ) {};

    // determines script changes
    void InitScriptInfo( const SwTxtNode& rNode, sal_Bool bRTL );

    // set/get position from which data is invalid
    inline void SetInvalidity( const xub_StrLen nPos );
    inline xub_StrLen GetInvalidity() const { return nInvalidityPos; };

    // get default direction for paragraph
    inline BYTE GetDefaultDir() const { return nDefaultDir; };

    // array operations, nCnt refers to array position
    inline USHORT CountScriptChg() const;
    inline xub_StrLen GetScriptChg( const USHORT nCnt ) const;
    inline BYTE GetScriptType( const USHORT nCnt ) const;

    inline USHORT CountDirChg() const;
    inline xub_StrLen GetDirChg( const USHORT nCnt ) const;
    inline BYTE GetDirType( const USHORT nCnt ) const;

    inline USHORT CountKashida() const;
    inline xub_StrLen GetKashida( const USHORT nCnt ) const;

    inline USHORT CountCompChg() const;
    inline xub_StrLen GetCompStart( const USHORT nCnt ) const;
    inline xub_StrLen GetCompLen( const USHORT nCnt ) const;
    inline BYTE GetCompType( const USHORT nCnt ) const;

    // "high" level operations, nPos refers to string position
    xub_StrLen NextScriptChg( const xub_StrLen nPos ) const;
    BYTE ScriptType( const xub_StrLen nPos ) const;

    // Returns the position of the next direction level change.
    // If bLevel is set, the position of the next level which is smaller
    // than the level at position nPos is returned. This is required to
    // obtain the end of a SwBidiPortion
    xub_StrLen NextDirChg( const xub_StrLen nPos,
                           const BYTE* pLevel = 0 ) const;
};

inline void SwScriptInfo::SetInvalidity( const xub_StrLen nPos )
{
    if ( nPos < nInvalidityPos )
        nInvalidityPos = nPos;
};
inline USHORT SwScriptInfo::CountScriptChg() const { return aScriptChg.Count(); }
inline xub_StrLen SwScriptInfo::GetScriptChg( const USHORT nCnt ) const
{
    OSL_ENSURE( nCnt < aScriptChg.Count(),"No ScriptChange today!");
    return aScriptChg[ nCnt ];
}
inline BYTE SwScriptInfo::GetScriptType( const xub_StrLen nCnt ) const
{
    OSL_ENSURE( nCnt < aScriptChg.Count(),"No ScriptType today!");
    return aScriptType[ nCnt ];
}

inline USHORT SwScriptInfo::CountDirChg() const { return aDirChg.Count(); }
inline xub_StrLen SwScriptInfo::GetDirChg( const USHORT nCnt ) const
{
    OSL_ENSURE( nCnt < aDirChg.Count(),"No DirChange today!");
    return aDirChg[ nCnt ];
}
inline BYTE SwScriptInfo::GetDirType( const xub_StrLen nCnt ) const
{
    OSL_ENSURE( nCnt < aDirChg.Count(),"No DirType today!");
    return aDirType[ nCnt ];
}

inline USHORT SwScriptInfo::CountKashida() const { return aKashida.Count(); }
inline xub_StrLen SwScriptInfo::GetKashida( const USHORT nCnt ) const
{
    OSL_ENSURE( nCnt < aKashida.Count(),"No Kashidas today!");
    return aKashida[ nCnt ];
}

inline USHORT SwScriptInfo::CountCompChg() const { return aCompChg.Count(); };
inline xub_StrLen SwScriptInfo::GetCompStart( const USHORT nCnt ) const
{
    OSL_ENSURE( nCnt < aCompChg.Count(),"No CompressionStart today!");
    return aCompChg[ nCnt ];
}
inline xub_StrLen SwScriptInfo::GetCompLen( const USHORT nCnt ) const
{
    OSL_ENSURE( nCnt < aCompChg.Count(),"No CompressionLen today!");
    return aCompLen[ nCnt ];
}

inline BYTE SwScriptInfo::GetCompType( const USHORT nCnt ) const
{
    OSL_ENSURE( nCnt < aCompChg.Count(),"No CompressionType today!");
    return aCompType[ nCnt ];
}

/*************************************************************************
 *                      class SwDrawTextInfo
 *
 * encapsulates information for drawing text
 *************************************************************************/

class SwDrawTextInfo
{
    const SwTxtFrm* pFrm;
    OutputDevice* pOut;
    ViewShell* pSh;
    const SwScriptInfo* pScriptInfo;
    const Point* pPos;
    const XubString* pText;
    const SwWrongList* pWrong;
    const Size* pSize;
    SwFont *pFnt;
    SwUnderlineFont* pUnderFnt;
    xub_StrLen* pHyphPos;
//   Fraction aZoom;
    long nLeft;
    long nRight;
    long nKanaDiff;
    xub_StrLen nIdx;
    xub_StrLen nLen;
    xub_StrLen nOfst;
    USHORT nWidth;
    USHORT nAscent;
    USHORT nCompress;
    short nSperren;
    short nKern;
    short nSpace;
    BYTE nCursorBidiLevel;
    BOOL bBullet : 1;
    BOOL bUpper : 1;        // Fuer Kapitaelchen: Grossbuchstaben-Flag
    BOOL bDrawSpace : 1;    // Fuer Kapitaelchen: Unter/Durchstreichung
    BOOL bGreyWave  : 1;    // Graue Wellenlinie beim extended TextInput
    BOOL bSpaceStop : 1;    // For underlining we need to know, if a portion
                            // is right in front of a hole portion or a
                            // fix margin portion.
    BOOL bSnapToGrid : 1;   // Does paragraph snap to grid?
    BOOL bIgnoreFrmRTL : 1; // Paint text as if text has LTR direction, used for
                            // line numbering
    BOOL bPosMatchesBounds :1;  // GetCrsrOfst should not return the next
                                // position if screen position is inside second
                                // have of bound rect, used for Accessibility

    SwDrawTextInfo();       // nicht zulaessig
public:
#ifdef DBG_UTIL
    BOOL bOut   : 1;    // In der Non-Product wird der Zugriff auf die Member
    BOOL bPos   : 1;    // durch diese Flags ueberwacht.
    BOOL bText  : 1;    // Wird ein Member gesetzt, so wird das entsprechende
    BOOL bWrong : 1;    // Flag gesetzt.
    BOOL bSize  : 1;    // Wird ein Member ausgelesen, so wird ASSERTet, dass
    BOOL bFnt   : 1;    // dieser zuvor gesetzt worden ist.
    BOOL bIdx   : 1;
    BOOL bLen   : 1;
    BOOL bWidth : 1;
    BOOL bAscent: 1;
    BOOL bSperr : 1;
    BOOL bKern  : 1;
    BOOL bSpace : 1;
    BOOL bBull  : 1;
    BOOL bSpec  : 1;
    BOOL bUppr  : 1;
    BOOL bDrawSp: 1;
    BOOL bGreyWv: 1;
    BOOL bLeft  : 1;
    BOOL bRight : 1;
    BOOL bKana  : 1;
    BOOL bOfst  : 1;
    BOOL bHyph  : 1;
#endif

    SwDrawTextInfo( ViewShell *pS, OutputDevice &rO, const SwScriptInfo* pSI,
                    const XubString &rSt, xub_StrLen nI, xub_StrLen nL,
                    USHORT nW = 0, BOOL bB = FALSE )
    {
        pFrm = NULL;
        pSh = pS;
        pOut = &rO;
        pScriptInfo = pSI;
        pText = &rSt;
        nIdx = nI;
        nLen = nL;
        nKern = 0;
        nCompress = 0;
        nWidth = nW;
        nCursorBidiLevel = 0;
        bBullet = bB;
        pUnderFnt = 0;
        bGreyWave = FALSE;
        bSpaceStop = FALSE;
        bSnapToGrid = FALSE;
        bIgnoreFrmRTL = FALSE;
        bPosMatchesBounds = FALSE;

#ifdef DBG_UTIL
        bOut = bText = bIdx = bLen = bWidth = bKern = bBull = bSpec =
        bGreyWv = TRUE;

        bPos = bWrong = bSize = bFnt = bAscent = bSpace = bUppr =
        bDrawSp = bLeft = bRight = bKana = bOfst = bHyph = FALSE;
#endif
    }

    const SwTxtFrm* GetFrm() const
    {
        return pFrm;
    }

    void SetFrm( const SwTxtFrm* pNewFrm )
    {
        pFrm = pNewFrm;
    }

    ViewShell *GetShell() const
    {
        return pSh;
    }

    OutputDevice& GetOut() const
    {
#if OSL_DEBUG_LEVEL > 1
        OSL_ENSURE( bOut, "DrawTextInfo: Undefined Outputdevice" );
#endif
        return *pOut;
    }

    OutputDevice *GetpOut() const
    {
#if OSL_DEBUG_LEVEL > 1
        OSL_ENSURE( bOut, "DrawTextInfo: Undefined Outputdevice" );
#endif
        return pOut;
    }

    const SwScriptInfo* GetScriptInfo() const
    {
        return pScriptInfo;
    }

    const Point &GetPos() const
    {
#if OSL_DEBUG_LEVEL > 1
        OSL_ENSURE( bPos, "DrawTextInfo: Undefined Position" );
#endif
        return *pPos;
    }

    xub_StrLen *GetHyphPos() const
    {
#if OSL_DEBUG_LEVEL > 1
        OSL_ENSURE( bHyph, "DrawTextInfo: Undefined Hyph Position" );
#endif
        return pHyphPos;
    }

    const XubString &GetText() const
    {
#if OSL_DEBUG_LEVEL > 1
        OSL_ENSURE( bText, "DrawTextInfo: Undefined String" );
#endif
        return *pText;
    }

    const SwWrongList* GetWrong() const
    {
#if OSL_DEBUG_LEVEL > 1
        OSL_ENSURE( bWrong, "DrawTextInfo: Undefined WrongList" );
#endif
        return pWrong;
    }

    const Size &GetSize() const
    {
#if OSL_DEBUG_LEVEL > 1
        OSL_ENSURE( bSize, "DrawTextInfo: Undefined Size" );
#endif
        return *pSize;
    }

    SwFont* GetFont() const
    {
#if OSL_DEBUG_LEVEL > 1
        OSL_ENSURE( bFnt, "DrawTextInfo: Undefined Font" );
#endif
        return pFnt;
    }

    SwUnderlineFont* GetUnderFnt() const
    {
#if OSL_DEBUG_LEVEL > 1
        OSL_ENSURE( bSpec, "DrawTextInfo: Undefined Underlinefont" );
#endif
        return pUnderFnt;
    }

    xub_StrLen GetIdx() const
    {
#if OSL_DEBUG_LEVEL > 1
        OSL_ENSURE( bIdx, "DrawTextInfo: Undefined Index" );
#endif
        return nIdx;
    }

    xub_StrLen GetLen() const
    {
#if OSL_DEBUG_LEVEL > 1
        OSL_ENSURE( bLen, "DrawTextInfo: Undefined Length" );
#endif
        return nLen;
    }

    xub_StrLen GetOfst() const
    {
#if OSL_DEBUG_LEVEL > 1
        OSL_ENSURE( bOfst, "DrawTextInfo: Undefined Offset" );
#endif
        return nOfst;
    }

    xub_StrLen GetEnd() const
    {
#if OSL_DEBUG_LEVEL > 1
        OSL_ENSURE( bIdx, "DrawTextInfo: Undefined Index" );
        OSL_ENSURE( bLen, "DrawTextInfo: Undefined Length" );
#endif
        return nIdx + nLen;
    }

    long GetLeft() const
    {
#if OSL_DEBUG_LEVEL > 1
        OSL_ENSURE( bLeft, "DrawTextInfo: Undefined left range" );
#endif
        return nLeft;
    }

    long GetRight() const
    {
#if OSL_DEBUG_LEVEL > 1
        OSL_ENSURE( bRight, "DrawTextInfo: Undefined right range" );
#endif
        return nRight;
    }

    long GetKanaDiff() const
    {
#if OSL_DEBUG_LEVEL > 1
        OSL_ENSURE( bKana, "DrawTextInfo: Undefined kana difference" );
#endif
        return nKanaDiff;
    }

    USHORT GetWidth() const
    {
#if OSL_DEBUG_LEVEL > 1
        OSL_ENSURE( bWidth, "DrawTextInfo: Undefined Width" );
#endif
        return nWidth;
    }

    USHORT GetAscent() const
    {
#if OSL_DEBUG_LEVEL > 1
        OSL_ENSURE( bAscent, "DrawTextInfo: Undefined Ascent" );
#endif
        return nAscent;
    }

    USHORT GetKanaComp() const
    {
        return nCompress;
    }

    short GetSperren() const
    {
#if OSL_DEBUG_LEVEL > 1
        OSL_ENSURE( bSperr, "DrawTextInfo: Undefined >Sperren<" );
#endif
        return nSperren;
    }

    short GetKern() const
    {
#if OSL_DEBUG_LEVEL > 1
        OSL_ENSURE( bKern, "DrawTextInfo: Undefined Kerning" );
#endif
        return nKern;
    }

    short GetSpace() const
    {
#if OSL_DEBUG_LEVEL > 1
        OSL_ENSURE( bSpace, "DrawTextInfo: Undefined Spacing" );
#endif
        return nSpace;
    }

    BYTE GetCursorBidiLevel() const
    {
        return nCursorBidiLevel;
    }

    BOOL GetBullet() const
    {
#if OSL_DEBUG_LEVEL > 1
        OSL_ENSURE( bBull, "DrawTextInfo: Undefined Bulletflag" );
#endif
        return bBullet;
    }

    BOOL GetUpper() const
    {
#if OSL_DEBUG_LEVEL > 1
        OSL_ENSURE( bUppr, "DrawTextInfo: Undefined Upperflag" );
#endif
        return bUpper;
    }

    BOOL GetDrawSpace() const
    {
#if OSL_DEBUG_LEVEL > 1
        OSL_ENSURE( bDrawSp, "DrawTextInfo: Undefined DrawSpaceflag" );
#endif
        return bDrawSpace;
    }

    BOOL GetGreyWave() const
    {
#if OSL_DEBUG_LEVEL > 1
        OSL_ENSURE( bGreyWv, "DrawTextInfo: Undefined GreyWave" );
#endif
        return bGreyWave;
    }

    BOOL IsSpaceStop() const
    {
        return bSpaceStop;
    }

    BOOL SnapToGrid() const
    {
        return bSnapToGrid;
    }

    BOOL IsIgnoreFrmRTL() const
    {
        return bIgnoreFrmRTL;
    }

    BOOL IsPosMatchesBounds() const
    {
        return bPosMatchesBounds;
    }

    void SetOut( OutputDevice &rNew )
    {
        pOut = &rNew;
#ifdef DBG_UTIL
        bOut = TRUE;
#endif
    }

    void SetPos( const Point &rNew )
    {
        pPos = &rNew;
#ifdef DBG_UTIL
        bPos = TRUE;
#endif
    }

    void SetHyphPos( xub_StrLen *pNew )
    {
        pHyphPos = pNew;
#ifdef DBG_UTIL
        bHyph = TRUE;
#endif
    }

    void SetText( const XubString &rNew )
    {
        pText = &rNew;
#ifdef DBG_UTIL
        bText = TRUE;
#endif
    }

    void SetWrong( const SwWrongList* pNew )
    {
        pWrong = pNew;
#ifdef DBG_UTIL
        bWrong = TRUE;
#endif
    }

    void SetSize( const Size &rNew )
    {
        pSize = &rNew;
#ifdef DBG_UTIL
        bSize = TRUE;
#endif
    }

    void SetFont( SwFont* pNew )
    {
        pFnt = pNew;
#ifdef DBG_UTIL
        bFnt = TRUE;
#endif
    }

    void SetIdx( xub_StrLen nNew )
    {
        nIdx = nNew;
#ifdef DBG_UTIL
        bIdx = TRUE;
#endif
    }

    void SetLen( xub_StrLen nNew )
    {
        nLen = nNew;
#ifdef DBG_UTIL
        bLen = TRUE;
#endif
    }

    void SetOfst( xub_StrLen nNew )
    {
        nOfst = nNew;
#ifdef DBG_UTIL
        bOfst = TRUE;
#endif
    }

    void SetLeft( long nNew )
    {
        nLeft = nNew;
#ifdef DBG_UTIL
        bLeft = TRUE;
#endif
    }

    void SetRight( long nNew )
    {
        nRight = nNew;
#ifdef DBG_UTIL
        bRight = TRUE;
#endif
    }

    void SetKanaDiff( long nNew )
    {
        nKanaDiff = nNew;
#ifdef DBG_UTIL
        bKana = TRUE;
#endif
    }

    void SetWidth( USHORT nNew )
    {
        nWidth = nNew;
#ifdef DBG_UTIL
        bWidth = TRUE;
#endif
    }

    void SetAscent( USHORT nNew )
    {
        nAscent = nNew;
#ifdef DBG_UTIL
        bAscent = TRUE;
#endif
    }

    void SetKern( short nNew )
    {
        nKern = nNew;
#ifdef DBG_UTIL
        bKern = TRUE;
#endif
    }

    void SetSperren( short nNew )
    {
        nSperren = nNew;
#ifdef DBG_UTIL
        bSperr = TRUE;
#endif
    }

    void SetSpace( short nNew )
    {
        if( nNew < 0 )
        {
            SetSperren( -nNew );
            nSpace = 0;
        }
        else
        {
            nSpace = nNew;
            SetSperren( 0 );
        }
#ifdef DBG_UTIL
        bSpace = TRUE;
#endif
    }

    void SetCursorBidiLevel( BYTE nNew )
    {
        nCursorBidiLevel = nNew;
    }

    void SetKanaComp( short nNew )
    {
        nCompress = nNew;
    }

    void SetBullet( BOOL bNew )
    {
        bBullet = bNew;
#ifdef DBG_UTIL
        bBull = TRUE;
#endif
    }

    void SetUnderFnt( SwUnderlineFont* pFntIn )
    {
        pUnderFnt = pFntIn;
#ifdef DBG_UTIL
        bSpec = TRUE;
#endif
    }

    void SetUpper( BOOL bNew )
    {
        bUpper = bNew;
#ifdef DBG_UTIL
        bUppr = TRUE;
#endif
    }

    void SetDrawSpace( BOOL bNew )
    {
        bDrawSpace = bNew;
#ifdef DBG_UTIL
        bDrawSp = TRUE;
#endif
    }

    void SetGreyWave( BOOL bNew )
    {
        bGreyWave = bNew;
#ifdef DBG_UTIL
        bGreyWv = TRUE;
#endif
    }

    void SetSpaceStop( BOOL bNew )
    {
        bSpaceStop = bNew;
    }

    void SetSnapToGrid( BOOL bNew )
    {
        bSnapToGrid = bNew;
    }

    void SetIgnoreFrmRTL( BOOL bNew )
    {
        bIgnoreFrmRTL = bNew;
    }

    void SetPosMatchesBounds( BOOL bNew )
    {
        bPosMatchesBounds = bNew;
    }


    // sets a new color at the output device if necessary
    // if a font is passed as argument, the change if made to the font
    // otherwise the font at the output device is changed
    // returns if the font has been changed
};

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
