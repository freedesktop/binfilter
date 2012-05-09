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

#include <string.h>
#include <bf_tools/string.hxx>
#include "boost/static_assert.hpp"
#include <osl/diagnose.h>
#include <osl/interlck.h>
#include <rtl/alloc.h>
#include <rtl/memory.h>
#include <rtl/tencinfo.h>

#include <impstrg.hxx>

#include <tools/debug.hxx>

namespace binfilter
{

// =======================================================================

DBG_NAME( ByteString )
DBG_NAMEEX( UniString )

// -----------------------------------------------------------------------

#define STRCODE         sal_Char
#define STRCODEU        unsigned char
#define STRING          ByteString
#define STRINGDATA      ByteStringData
#define DBGCHECKSTRING  DbgCheckByteString
#define STRING_TYPE     rtl_String
#define STRING_ACQUIRE  rtl_string_acquire
#define STRING_RELEASE  rtl_string_release
#define STRING_NEW      rtl_string_new


// -----------------------------------------------------------------------

xub_StrLen ImplStringLen( const sal_Char* pStr )
{
    const sal_Char* pTempStr = pStr;
    while( *pTempStr )
        ++pTempStr;
    return (xub_StrLen)(pTempStr-pStr);
}

// -----------------------------------------------------------------------

xub_StrLen ImplStringLen( const sal_Unicode* pStr )
{
    const sal_Unicode* pTempStr = pStr;
    while( *pTempStr )
        ++pTempStr;
    return (xub_StrLen)(pTempStr-pStr);
}

// -----------------------------------------------------------------------

}

#include <tl_strimp.cxx>
#include <tl_strcvt.cxx>

namespace binfilter
{

static sal_Int32 ImplStringCompare( const STRCODE* pStr1, const STRCODE* pStr2 )
{
    sal_Int32 nRet;
    while ( ((nRet = ((sal_Int32)((STRCODEU)*pStr1))-((sal_Int32)((STRCODEU)*pStr2))) == 0) &&
            *pStr2 )
    {
        ++pStr1,
        ++pStr2;
    }

    return nRet;
}

sal_Bool STRING::Equals( const STRCODE* pCharStr ) const
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    return (ImplStringCompare( mpData->maStr, pCharStr ) == 0);
}

STRING& STRING::Insert( const STRCODE* pCharStr, xub_StrLen nIndex )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_ASSERT( pCharStr, "String::Insert() - pCharStr is NULL" );

    // Stringlaenge ermitteln
    sal_Int32 nCopyLen = ImplStringLen( pCharStr );

    // Ueberlauf abfangen
    nCopyLen = ImplGetCopyLen( mpData->mnLen, nCopyLen );

    // Ist der einzufuegende String ein Leerstring
    if ( !nCopyLen )
        return *this;

    // Index groesser als Laenge
    if ( nIndex > mpData->mnLen )
        nIndex = static_cast< xub_StrLen >(mpData->mnLen);

    // Neue Laenge ermitteln und neuen String anlegen
    STRINGDATA* pNewData = ImplAllocData( mpData->mnLen+nCopyLen );

    // String kopieren
    memcpy( pNewData->maStr, mpData->maStr, nIndex*sizeof( STRCODE ) );
    memcpy( pNewData->maStr+nIndex, pCharStr, nCopyLen*sizeof( STRCODE ) );
    memcpy( pNewData->maStr+nIndex+nCopyLen, mpData->maStr+nIndex,
            (mpData->mnLen-nIndex)*sizeof( STRCODE ) );

    // Alte Daten loeschen und Neue zuweisen
    STRING_RELEASE((STRING_TYPE *)mpData);
    mpData = pNewData;

    return *this;
}

    ByteString ByteString::CreateFromInt32(sal_Int32 n, sal_Int16 nRadix)
    {
        sal_Char aBuf[RTL_STR_MAX_VALUEOFINT32];
        BOOST_STATIC_ASSERT(RTL_STR_MAX_VALUEOFINT32 <= STRING_MAXLEN);
        return rtl::OString(
            aBuf, rtl_str_valueOfInt32( aBuf, n, nRadix ) );
    }

    sal_Unicode ConvertToUnicode( const char* pChar, sal_Size* pLen, rtl_TextEncoding eTextEncoding )
    {
        // TextEncoding Dontknow wird nicht konvertiert
        if ( eTextEncoding == RTL_TEXTENCODING_DONTKNOW )
            return 0;

        rtl_TextToUnicodeConverter	hConverter;
        sal_uInt32					nInfo;
        sal_Size					nSrcBytes;
        sal_Size					nDestChars;
        sal_Unicode 				nConvChar;
        hConverter = rtl_createTextToUnicodeConverter( eTextEncoding );
        nDestChars = rtl_convertTextToUnicode( hConverter, 0,
                                               (const sal_Char*)pChar, *pLen,
                                               &nConvChar, 1,
                                               RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_DEFAULT |
                                               RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT |
                                               RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT |
                                               RTL_TEXTTOUNICODE_FLAGS_FLUSH,
                                               &nInfo, &nSrcBytes );
        rtl_destroyTextToUnicodeConverter( hConverter );

        if ( nDestChars == 1 )
        {
            *pLen = nSrcBytes;
            return nConvChar;
        }
        else
        {
            *pLen = 0;
            return 0;
        }
    }

    sal_Size ConvertFromUnicode( sal_Unicode c, char* pBuf, sal_Size nBufLen, rtl_TextEncoding eTextEncoding )
    {
        // TextEncoding Dontknow wird nicht konvertiert
        if ( eTextEncoding == RTL_TEXTENCODING_DONTKNOW )
            return '\0';

        rtl_UnicodeToTextConverter  hConverter;
        sal_uInt32                  nInfo;
        sal_Size                    nSrcChars;
        sal_Size                    nDestBytes;
        sal_Unicode                 cUni = c;
        sal_uInt32                  nFlags = RTL_UNICODETOTEXT_FLAGS_NONSPACING_IGNORE |
                                             RTL_UNICODETOTEXT_FLAGS_CONTROL_IGNORE |
                                             RTL_UNICODETOTEXT_FLAGS_FLUSH;

        nFlags |= RTL_UNICODETOTEXT_FLAGS_UNDEFINED_DEFAULT |
                  RTL_UNICODETOTEXT_FLAGS_INVALID_DEFAULT;
        nFlags |= RTL_UNICODETOTEXT_FLAGS_UNDEFINED_REPLACE;
        if ( nBufLen > 1 )
            nFlags |= RTL_UNICODETOTEXT_FLAGS_UNDEFINED_REPLACESTR;

        hConverter = rtl_createUnicodeToTextConverter( eTextEncoding );
        nDestBytes = rtl_convertUnicodeToText( hConverter, 0,
                                               &cUni, 1,
                                               (sal_Char*)pBuf, nBufLen,
                                               nFlags,
                                               &nInfo, &nSrcChars );
        rtl_destroyUnicodeToTextConverter( hConverter );
        return nDestBytes;
    }

    sal_Unicode ByteString::ConvertToUnicode( char c, rtl_TextEncoding eTextEncoding )
    {
        sal_Size nLen = 1;
        return binfilter::ConvertToUnicode( &c, &nLen, eTextEncoding );
    }

    char ByteString::ConvertFromUnicode(sal_Unicode c, rtl_TextEncoding eTextEncoding)
    {
        sal_Size    nLen;
        char        aBuf[30];
        nLen = binfilter::ConvertFromUnicode(c, aBuf, sizeof( aBuf ), eTextEncoding);
        if ( nLen == 1 )
            return aBuf[0];
        else
            return 0;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
