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

#ifndef _IODETECT_HXX
#define _IODETECT_HXX

#include <bf_svtools/bf_solar.h>

#ifdef IS_SW_DLL
#include <shellio.hxx>
#endif
#include <osl/endian.h>
namespace binfilter {

class Reader;

struct SwIoDetect
{
// eigentlich privat, aber der Compiler kann sonst die
// Tabelle nicht initialisieren
    const sal_Char* pName;
    USHORT nLen;

    inline int IsFilter( const String& rNm )
    {
        return rNm.EqualsAscii( pName, 0, nLen );
    }

#ifdef IS_SW_DLL
    Reader* pReader;
    BOOL bDelReader;

   inline Reader* GetReader() const { return pReader; }
#endif

    const sal_Char* IsReader(const sal_Char* pHeader, ULONG nInLen) const;
};


#ifdef IS_SW_DLL
#define SwIoEntry(sNm, cCharLen,  bDel) sNm, cCharLen, 0,  bDel
#else
#define SwIoEntry(sNm, cCharLen, bDel)  sNm, cCharLen
#endif

#ifdef DEBUG_SH

#define DEB_SH_SwIoEntry(sNm, cCharLen, pWrt, bDel) , SwIoEntry(sNm, cCharLen, pWrt, bDel)

#else

#define DEB_SH_SwIoEntry(sNm, cCharLen, pWrt, bDel)

#endif

#if !( !defined(DBG_UTIL) || defined(PM2))
#define DEB_DBWRT_SwIoEntry(sNm, cCharLen, pWrt, bDel)  , SwIoEntry(sNm, cCharLen, pWrt, bDel)
#else
#define DEB_DBWRT_SwIoEntry(sNm, cCharLen, pWrt, bDel)
#endif


const USHORT MAXFILTER = 12;

#define FORAMTNAME_SW4      "StarWriter 4.0"
#define FORAMTNAME_SW3      "StarWriter 3.0"
#define FORAMTNAME_SWGLOB   "StarWriter/Global 4.0"


#define IO_DETECT_IMPL1 \
sal_Char FILTER_SWG[]   = "SWG";                             \
sal_Char FILTER_SW3[]   = "CSW3";                            \
sal_Char FILTER_SW4[]   = "CSW4";                            \
sal_Char FILTER_SW5[]   = "CSW5";                            \
sal_Char FILTER_BAS[]   = "BAS";                             \
sal_Char FILTER_RTF[]   = "RTF";                             \
sal_Char FILTER_SWGV[]  = "SWGV";                            \
sal_Char FILTER_SW3V[]  = "CSW3V";                           \
sal_Char FILTER_SW4V[]  = "CSW4V";                           \
sal_Char FILTER_SW5V[]  = "CSW5V";                           \
sal_Char FILTER_SWW4V[] = "CSW4VWEB";                        \
sal_Char FILTER_SWW5V[] = "CSW5VWEB";                        \
sal_Char sSwg1[]         = "SWG1";                           \
sal_Char sRtfWH[]       = "WH_RTF";                          \
sal_Char sCExcel[]      = "CEXCEL";                          \
sal_Char sExcel[]       = "EXCEL";                           \
sal_Char sLotusD[]      = "LOTUSD";                          \
sal_Char sLotusW[]      = "LOTUSW";                          \
sal_Char sHTML[]        = "HTML";                            \
sal_Char sWW1[]             = "WW1";                         \
sal_Char sWW5[]         = "WW6";                             \
sal_Char sWW6[]         = "CWW6";                            \
sal_Char FILTER_WW8[]   = "CWW8";                            \
sal_Char FILTER_TEXT_DLG[] = "TEXT_DLG";                     \
sal_Char FILTER_TEXT[]  = "TEXT";                            \
sal_Char sDebug[]       = "DEBUG";                           \
sal_Char sUndo[]        = "UNDO";                            \
sal_Char FILTER_XML[]   = "CXML";                            \
sal_Char FILTER_XMLV[]  = "CXMLV";                           \
sal_Char FILTER_XMLVW[]     = "CXMLVWEB";                    \
sal_Char sSwDos[]       = "SW6";                               \
                                                               \
                                                               \
SwIoDetect aReaderWriter[ MAXFILTER ] = {                      \
    {/* 0*/ SwIoEntry(FILTER_SW5,     4,            TRUE)},    \
    {/* 1*/ SwIoEntry(FILTER_SW4,     4,            FALSE)},   \
    {/* 2*/ SwIoEntry(FILTER_SW3,     4,            FALSE)},   \
    {/* 3*/ SwIoEntry(FILTER_SWG,     STRING_LEN,   TRUE)},    \
    {/* 4*/ SwIoEntry(FILTER_SWGV,    4,            FALSE)},   \
    {/* 5*/ SwIoEntry(sSwDos,         STRING_LEN,   TRUE)},    \
    {/* 6*/ SwIoEntry(FILTER_BAS,     STRING_LEN,   FALSE)},   \
    {/* 7*/ SwIoEntry(sCExcel,        5,            TRUE)},    \
    {/* 8*/ SwIoEntry(sExcel,         4,            FALSE)},   \
    {/* 9*/ SwIoEntry(sLotusD,        5,            TRUE)},    \
    {/*10*/ SwIoEntry(sSwg1,          4,            FALSE)},   \
    {/*11*/ SwIoEntry(FILTER_TEXT,    4,            TRUE)}     \
};

// Filter erkennung
struct W1_FIB
{
    SVBT16 wIdent;      // 0x0 int magic number
    SVBT16 nFib;        // 0x2 FIB version written
    SVBT16 nProduct;    // 0x4 product version written by
    SVBT16 nlocale;     // 0x6 language stamp---localized version;
    SVBT16 pnNext;      // 0x8
    SVBT16 fFlags;

    USHORT nFibGet()    { return SVBT16ToShort(nFib); }
    USHORT wIdentGet()  { return SVBT16ToShort(wIdent); }
    USHORT fFlagsGet()  { return SVBT16ToShort(fFlags); }
    BOOL fComplexGet() { return ((fFlagsGet() >> 2) & 1); }
};

#define IO_DETECT_IMPL2 \
const sal_Char* SwIoDetect::IsReader(const sal_Char* pHeader, ULONG nInLen )\
    const              \
{                                                                           \
    int bRet = FALSE;                                                       \
    if( FILTER_SWG == pName )                                          \
        bRet = 0 == strncmp( FILTER_SWG, pHeader, 3 ) &&                    \
                '1' != *(pHeader + 3);                                      \
    else if( sSwg1 == pName )                                               \
        bRet = 0 == strncmp( FILTER_SWG, pHeader, 3 ) &&                    \
                '1' == *(pHeader + 3);                                      \
    else if( sLotusD == pName )                                             \
        bRet = 0 == *pHeader++ && 0 == *pHeader++ &&                        \
                2 == *pHeader++ && 0 == *pHeader++ &&                       \
                ( 4 == *pHeader || 6 == *pHeader ) && 4 == *++pHeader;      \
    else if( sExcel == pName )                                              \
    {                                                                       \
        if( 0x09 == *pHeader++ )                                            \
        {                                                                   \
            if( 0x00 == *pHeader )                                          \
                bRet = 0x04 == *++pHeader && 0 == *++pHeader;               \
            else if( 0x02 == *pHeader || 0x04 == *pHeader )                 \
                bRet = 0x06 == *++pHeader && 0 == *++pHeader;               \
        }                                                                   \
    }                                                                       \
    else if( sWW5 == pName )                                                \
    {                                                                       \
        bRet = (( ((W1_FIB*)pHeader)->wIdentGet() == 0xA5DC                 \
                 && ((W1_FIB*)pHeader)->nFibGet() == 0x65 )                  \
                 /*&& ((W1_FIB*)pHeader)->fComplexGet() == 0*/);             \
    }                                                                       \
    else if( sWW1 == pName )                                                \
    {                                                                       \
        bRet = (( ((W1_FIB*)pHeader)->wIdentGet() == 0xA59C                 \
                 && ((W1_FIB*)pHeader)->nFibGet() == 0x21)                  \
                 && ((W1_FIB*)pHeader)->fComplexGet() == 0);          \
    }                                                                       \
    else if( sSwDos == pName )                                              \
    {                                                                       \
        sal_Char const sSw6_FormatStt[] =         ".\\\\\\ WRITER ";  \
        sal_Char const sSw6_FormatEnd[] =         " \\\\\\";          \
                                                                            \
        bRet = 0 == strncmp( sSw6_FormatStt, pHeader, 12 ) &&               \
                  0 == strncmp( sSw6_FormatEnd, pHeader + 12 + 1, 4 );          \
    }                                                                       \
    else if (FILTER_TEXT == pName) \
        bRet = SwIoSystem::IsDetectableText(pHeader, nInLen); \
    return bRet ? pName : 0;                                       \
}                                                                           \

#define IO_DETECT_IMPL3 \
const String SwIoSystem::GetSubStorageName( const SfxFilter& rFltr )                  \
{                                                                                     \
    /* bei den StorageFiltern noch den SubStorageNamen setzen */                      \
    const String& rUserData = rFltr.GetUserData();                                    \
    if( rUserData.EqualsAscii(FILTER_SW5) || rUserData.EqualsAscii(FILTER_SW5V) ||    \
        rUserData.EqualsAscii(FILTER_SWW5V) ||                                        \
        rUserData.EqualsAscii(FILTER_SW4 )|| rUserData.EqualsAscii(FILTER_SW4V) ||    \
        rUserData.EqualsAscii(FILTER_SWW4V) ||                                        \
        rUserData.EqualsAscii(FILTER_SW3 )|| rUserData.EqualsAscii(FILTER_SW3V) )     \
        return String::CreateFromAscii(                                               \
                RTL_CONSTASCII_STRINGPARAM( "StarWriterDocument" ));                  \
    if( rUserData.EqualsAscii(FILTER_XML) ||                                          \
        rUserData.EqualsAscii(FILTER_XMLV) ||                                         \
        rUserData.EqualsAscii(FILTER_XMLVW) )                                         \
        return String::CreateFromAscii(                                               \
                RTL_CONSTASCII_STRINGPARAM( "content.xml" ));                                \
    if( rUserData.EqualsAscii(sWW6) || rUserData.EqualsAscii(FILTER_WW8) )            \
        return String::CreateFromAscii(                                               \
                RTL_CONSTASCII_STRINGPARAM( "WordDocument" ));                        \
    if( rUserData.EqualsAscii(sExcel) || rUserData.EqualsAscii(sCExcel) )             \
        return String::CreateFromAscii(                                               \
                RTL_CONSTASCII_STRINGPARAM( "Book" ));                                \
    return String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "" ));                \
}                                                                                     \
const SfxFilter* SwIoSystem::GetFilterOfFormat( const String& rFmtNm,                 \
                                const SfxFactoryFilterContainer* pCnt )               \
{                                                                                     \
    const SfxFactoryFilterContainer* pFltCnt = pCnt ? pCnt :                          \
        ( IsDocShellRegistered()                                                      \
            ? SwDocShell::Factory().GetFilterContainer()                              \
            : SwWebDocShell::Factory().GetFilterContainer() );                        \
                                                                                      \
    do {                                                                              \
        if( pFltCnt )                                                                 \
        {                                                                             \
            const SfxFilter* pFilter;                                                 \
            USHORT nCount = pFltCnt->GetFilterCount();                                \
            for( USHORT i = 0; i < nCount; ++i )                                      \
                if( ( pFilter = pFltCnt->GetFilter( i ))->GetUserData() == rFmtNm )   \
                    return pFilter;                                                   \
        }                                                                             \
        if( pCnt || pFltCnt == SwWebDocShell::Factory().GetFilterContainer())         \
            break;                                                                    \
        pFltCnt = SwWebDocShell::Factory().GetFilterContainer();                      \
    } while( TRUE );                                                                  \
                                                                                      \
    return 0;                                                                         \
}                                                                                     \
                                                                                      \
bool SwIoSystem::IsValidStgFilter( SvStorage& rStg, const SfxFilter& rFilter )    \
{                                                                                     \
    ULONG nStgFmtId = rStg.GetFormat();                                               \
    /*#i8409# We cannot trust the clipboard id anymore :-(*/ \
    if( rFilter.GetUserData().EqualsAscii(FILTER_WW8) || \
        rFilter.GetUserData().EqualsAscii(sWW6) ) \
    { \
        nStgFmtId = 0; \
    } \
\
    BOOL bRet = SVSTREAM_OK == rStg.GetError() &&                                     \
                ( !nStgFmtId || rFilter.GetFormat() == nStgFmtId ) &&                 \
                ( rStg.IsContained( SwIoSystem::GetSubStorageName( rFilter )) ||        \
                  (rFilter.GetUserData().EqualsAscii(FILTER_XML) &&                     \
                   rStg.IsContained( String::CreateFromAscii("Content.xml") )) );          \
    if( bRet )                                                                        \
    {                                                                                 \
        /* Bug 53445 - es gibt Excel Docs ohne ClipBoardId! */                        \
        /* Bug 62703 - und auch WinWord Docs ohne ClipBoardId! */                     \
        if( rFilter.GetUserData().EqualsAscii(FILTER_WW8) ||                          \
            rFilter.GetUserData().EqualsAscii(sWW6) )                                  \
        {\
            bRet = !((rStg.IsContained( String::CreateFromAscii("0Table" )) || \
                    rStg.IsContained( String::CreateFromAscii("1Table" ))) ^ \
                       rFilter.GetUserData().EqualsAscii(FILTER_WW8));          \
            if (bRet && !rFilter.IsAllowedAsTemplate()) \
            { \
                SvStorageStreamRef xRef = \
                    rStg.OpenStream(String::CreateFromAscii("WordDocument"), \
                    STREAM_STD_READ | STREAM_NOCREATE ); \
                xRef->Seek(10); \
                BYTE nByte; \
                *xRef >> nByte; \
                bRet = !(nByte & 1); \
            } \
        }\
        else if( rFilter.GetUserData().EqualsAscii(FILTER_XML, 0, sizeof(FILTER_XML)-1) ) \
            bRet = !nStgFmtId || rFilter.GetFormat() == nStgFmtId;                      \
        else if( !rFilter.GetUserData().EqualsAscii(sCExcel) )                                   \
            bRet = rFilter.GetFormat() == nStgFmtId;                                        \
    }                                                                                 \
    return bRet;                                                                      \
}                                                                                     \
                                                                                      \
    /* Feststellen ob das File in dem entsprechenden Format vorliegt. */              \
    /* Z.z werden nur unsere eigene Filter unterstuetzt               */              \
bool SwIoSystem::IsFileFilter( SfxMedium& rMedium, const String& rFmtName,        \
                                    const SfxFilter** ppFilter )                      \
{                                                                                     \
    bool bRet = FALSE;                                                            \
    const SfxFilter* pFltr;                                                           \
    const SfxFactoryFilterContainer& rFltContainer = IsDocShellRegistered()           \
            ? *SwDocShell::Factory().GetFilterContainer()                             \
            : *SwWebDocShell::Factory().GetFilterContainer();                         \
    USHORT nFltCount = rFltContainer.GetFilterCount();                                \
    SvStorageRef xStg;                                                                \
    if( rMedium.IsStorage() )                                                                           \
         xStg = rMedium.GetStorage();                                                  \
    for( USHORT n = 0; n < nFltCount; ++n )                                           \
        if( ( pFltr = rFltContainer.GetFilter( n ))->GetUserData() == rFmtName )      \
        {                                                                             \
            if( 'C' == *pFltr->GetUserData().GetBuffer() )                               \
            {                                                                         \
                bRet = xStg.Is() && IsValidStgFilter( *xStg, *pFltr );                \
            }                                                                         \
            else if( !xStg.Is() )                                                              \
            {                                                                         \
                SvStream* pStrm = rMedium.GetInStream();                              \
                if( pStrm && !pStrm->GetError() )                                     \
                {                                                                     \
                    sal_Char aBuffer[ 4097 ];                                               \
                    ULONG nBytesRead = pStrm->Read( aBuffer, 4096 );                    \
                    pStrm->Seek( STREAM_SEEK_TO_BEGIN );                              \
                    if( nBytesRead<=80 )                                              \
                    {                                                                 \
                        aBuffer[nBytesRead] = '\0';                                   \
                        aBuffer[nBytesRead+1] = '\0';                                 \
                        if( (nBytesRead & 0x00000001) != 0 )                          \
                            aBuffer[nBytesRead+2] = '\0';                             \
                    }                                                                 \
                                                                                      \
                    for( USHORT i = 0; i < MAXFILTER; ++i )                           \
                        if( aReaderWriter[i].IsFilter( rFmtName ) )                   \
                        {                                                             \
                            bRet = 0 != aReaderWriter[i].IsReader( aBuffer, nBytesRead );         \
                            break;                                                    \
                        }                                                             \
                }                                                                     \
            }                                                                         \
                                                                                      \
            if( bRet && ppFilter  )                                                   \
                *ppFilter = pFltr;                                                    \
                                                                                      \
            break;                                                                    \
        }                                                                             \
                                                                                      \
    return bRet;                                                                      \
}                                                                                     \


#ifdef OSL_LITENDIAN
#   define ENDHACK bool bNativeLE = true;
#else
#   define ENDHACK bool bNativeLE = false;
#endif


#define IO_DETECT_IMPL4 \
/* die Methode stellt fest, von welchem Typ der stream (File) ist.        */ \
/* Es wird versucht, eine dem Filter entsprechende Byte-Folge zu finden.  */ \
/* Wird kein entsprechender gefunden, wird zur Zeit der ASCII-Reader      */ \
/* returnt !! Der Returnwert ist der interne Filtername!                  */ \
const SfxFilter* SwIoSystem::GetFileFilter( const String& rFileName,         \
                                             SfxMedium* pMedium )             \
{                                                                            \
    SfxFactoryFilterContainer* pFCntnr = IsDocShellRegistered()              \
            ? SwDocShell::Factory().GetFilterContainer()                     \
            : SwWebDocShell::Factory().GetFilterContainer();                 \
                                                                             \
    USHORT nFltrCount;                                                       \
    if( !pFCntnr || 0 == ( nFltrCount = pFCntnr->GetFilterCount() ) )        \
        return 0;                                                            \
                                                                             \
    const SfxFilter* pFilter;                                                \
    if( pMedium ? pMedium->IsStorage()                                       \
                : SvStorage::IsStorageFile( rFileName ) )                    \
    {                                                                        \
        /* Storage: Suchen nach einem Sub-Storage, dessen Name  */           \
        /* dem in einem Filter stehenden DLL-Namen entspricht   */           \
        SvStorageRef xStg;                                                   \
        if( pMedium )                                                        \
            xStg = pMedium->GetStorage();                                    \
        else                                                                 \
            xStg = new SvStorage( rFileName, STREAM_STD_READ );              \
                                                                             \
        if( xStg.Is() && ( xStg->GetError() == SVSTREAM_OK ) )               \
        {                                                                    \
            USHORT nCnt;                                                     \
            for( nCnt = 0; nCnt < nFltrCount; ++nCnt )                       \
                if( 'C' == *( pFilter = pFCntnr->GetFilter( nCnt ))->        \
                    GetUserData().GetBuffer() &&                             \
                    IsValidStgFilter( *xStg, *pFilter ))                     \
                    return pFilter;                                          \
                                                                             \
            if( IsDocShellRegistered() && 0 != ( pFCntnr =                   \
                SwWebDocShell::Factory().GetFilterContainer() ) &&           \
                0 != ( nFltrCount = pFCntnr->GetFilterCount() ) )            \
                for( nCnt = 0; nCnt < nFltrCount; ++nCnt )                   \
                    if( 'C' == *( pFilter = pFCntnr->GetFilter( nCnt ))->    \
                        GetUserData().GetBuffer() &&                         \
                        IsValidStgFilter( *xStg, *pFilter ))                 \
                        return pFilter;                                      \
        }                                                                    \
        return 0;                                                            \
    }                                                                        \
                                                                             \
    sal_Char aBuffer[ 4098 ];                                                \
    ULONG nBytesRead(0);                                                     \
    if( pMedium )                                                            \
    {                                                                        \
        SvStream* pIStrm = pMedium->GetInStream();                           \
        if( !pIStrm || SVSTREAM_OK != pIStrm->GetError() )                   \
            return 0;                                                        \
        ULONG nCurrPos = pIStrm->Tell();                                     \
        nBytesRead = pIStrm->Read( aBuffer, 4096 );                          \
        pIStrm->Seek( nCurrPos );                                            \
    }                                                                        \
    else                                                                     \
    {                                                                        \
        SvFileStream aStrm( rFileName, STREAM_READ );                        \
                                                                             \
        /* ohne FileName oder ohne Stream gibts nur den ANSI-Filter */       \
        if( !rFileName.Len() || SVSTREAM_OK != aStrm.GetError() )            \
            return 0;                                                        \
                                                                             \
        nBytesRead = aStrm.Read( aBuffer, 4096 );                            \
        aStrm.Close();                                                       \
    }                                                                        \
    DBG_ASSERT( nBytesRead<=4096, "zu viele Bytes gelesen?" );               \
    if( nBytesRead <= 4096 )                                                 \
    {                                                                        \
        aBuffer[nBytesRead] = '\0';                                          \
        aBuffer[nBytesRead+1] = '\0';                                        \
        if( (nBytesRead & 0x00000001) != 0 )                                 \
            aBuffer[nBytesRead+2] = '\0';                                    \
    }                                                                       \
                                                                             \
    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/ \
    /* suche nach dem bestimmten Filter, falls kein entsprechender        */ \
    /* gefunden wird, so wird der ASCII-Filter returnt.                   */ \
    /* Gibt es Filter ohne einen Identifizierungs-String, so werden diese */ \
    /* nie erkannt und es wird auch der ASCII-Filter returnt.             */ \
    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/ \
    {                                                                        \
        const SfxFilter* pLclFilter=NULL;                                    \
        const sal_Char* pNm;                                                 \
        for( USHORT n = 0; n < MAXFILTER; ++n )                              \
            if( 0 != ( pNm = aReaderWriter[n].IsReader(aBuffer, nBytesRead)) &&        \
                0 != ( pLclFilter =  SwIoSystem::GetFilterOfFormat(          \
                                String::CreateFromAscii(pNm), pFCntnr )))    \
                return pLclFilter;                                           \
    }                                                                        \
                                                                             \
    /* Ok, bis jetzt kein Filter gefunden, also befrage mal die */           \
    /* "WORD 4 WORD" Filter                                     */           \
    if( rFileName.Len() )                                                    \
    {                                                                        \
        if( pMedium )                                                        \
            pMedium->CloseInStream();                                        \
    }                                                                        \
    return SwIoSystem::GetTextFilter( aBuffer, nBytesRead);                  \
}                                                                            \
\
bool SwIoSystem::IsDetectableText(const sal_Char* pBuf, ULONG &rLen, \
    CharSet *pCharSet, bool *pSwap, LineEnd *pLineEnd) \
{ \
    bool bSwap = false; \
    CharSet eCharSet = RTL_TEXTENCODING_DONTKNOW; \
    bool bLE = true; \
    ULONG nHead=0; \
    /*See if its a known unicode type*/\
    if (rLen >= 2)\
    {\
        if (rLen > 2 && BYTE(pBuf[0]) == 0xEF && BYTE(pBuf[1]) == 0xBB && \
            BYTE(pBuf[2]) == 0xBF)\
        {\
            eCharSet = RTL_TEXTENCODING_UTF8;\
            nHead = 3;\
        }\
        else if (BYTE(pBuf[0]) == 0xFE && BYTE(pBuf[1]) == 0xFF)\
        {\
            eCharSet = RTL_TEXTENCODING_UCS2;\
            bLE = false;\
            nHead = 2;\
        }\
        else if (BYTE(pBuf[1]) == 0xFE && BYTE(pBuf[0]) == 0xFF)\
        {\
            eCharSet = RTL_TEXTENCODING_UCS2;\
            nHead = 2;\
        }\
        pBuf+=nHead;\
        rLen-=nHead;\
    }\
\
    bool bCR = false, bLF = false, \
        bIsBareUnicode = false;\
\
    if (eCharSet != RTL_TEXTENCODING_DONTKNOW)\
    {\
        String sWork;\
        sal_Unicode *pNewBuf = sWork.AllocBuffer(rLen);\
        sal_Size nNewLen;\
        if (eCharSet != RTL_TEXTENCODING_UCS2)\
        {\
            nNewLen = rLen;\
            rtl_TextToUnicodeConverter hConverter = \
                rtl_createTextToUnicodeConverter(eCharSet);\
            rtl_TextToUnicodeContext hContext = \
                rtl_createTextToUnicodeContext(hConverter);\
\
            sal_Size nCntBytes;\
            sal_uInt32 nInfo;\
            nNewLen = rtl_convertTextToUnicode( hConverter, hContext, pBuf, \
                rLen, pNewBuf, nNewLen, \
                (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_DEFAULT | \
                  RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT | \
                  RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT), &nInfo, &nCntBytes);\
\
            rtl_destroyTextToUnicodeContext(hConverter, hContext);\
            rtl_destroyTextToUnicodeConverter(hConverter);\
        }\
        else\
        {\
            nNewLen = rLen/2;\
            memcpy(pNewBuf, pBuf, rLen);\
            ENDHACK\
            if (bLE != bNativeLE)\
            {\
                bSwap = true;\
                sal_Char* pF = (sal_Char*)pNewBuf;\
                sal_Char* pN = pF+1;\
                for(xub_StrLen n = 0; n < nNewLen; ++n, pF+=2, pN+=2)\
                {\
                    sal_Char c = *pF;\
                    *pF = *pN;\
                    *pN = c;\
                }\
            }\
        }\
\
        sWork.ReleaseBufferAccess(nNewLen);\
        pNewBuf = sWork.GetBufferAccess();\
     \
        for (ULONG nCnt = 0; nCnt < nNewLen; ++nCnt, ++pNewBuf)\
        {\
            switch (*pNewBuf) \
            {\
                case 0xA:   \
                    bLF = true; \
                    break;\
                case 0xD:\
                    bCR = true; \
                    break;\
                default:\
                    break;\
            }\
        }\
    }\
    else\
    {\
        for( ULONG nCnt = 0; nCnt < rLen; ++nCnt, ++pBuf )\
        {\
            switch (*pBuf) \
            {\
                case 0x0:\
                    if( nCnt + 1 < rLen && !*(pBuf+1) )\
                        return 0;\
                    bIsBareUnicode = true; \
                    break;\
                case 0xA:\
                    bLF = true; \
                    break;\
                case 0xD:\
                    bCR = true; \
                    break;\
                case 0xC:\
                case 0x1A:\
                case 0x9:\
                    break;\
                default:\
                    break;\
            }\
        }\
    }\
\
    LineEnd eSysLE = GetSystemLineEnd();\
    LineEnd eLineEnd;\
    if (!bCR && !bLF)\
        eLineEnd = eSysLE;\
    else \
        eLineEnd = bCR ? ( bLF ? LINEEND_CRLF : LINEEND_CR ) : LINEEND_LF;   \
\
    if (pCharSet)\
        *pCharSet = eCharSet;\
    if (pSwap)\
        *pSwap = bSwap;\
    if (pLineEnd)\
        *pLineEnd = eLineEnd;\
        \
    return (!bIsBareUnicode && eSysLE == eLineEnd);\
}\
\
\
const SfxFilter* SwIoSystem::GetTextFilter( const sal_Char* pBuf, ULONG nInLen)\
{                                                                            \
    bool bAuto = IsDetectableText(pBuf, nInLen); \
    const sal_Char* pNm = bAuto ? FILTER_TEXT : FILTER_TEXT_DLG; \
    return SwIoSystem::GetFilterOfFormat( String::CreateFromAscii(pNm), 0 ); \
} \

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
