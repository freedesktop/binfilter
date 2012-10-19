/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <tools/errcode.hxx>
#include "sbx.hxx"
#include "sbxconv.hxx"
#include "sbxres.hxx"

namespace binfilter {

enum SbxBOOL ImpGetBool( const SbxValues* p )
{
    enum SbxBOOL nRes;
    switch( +p->eType )
    {
        case SbxNULL:
            SbxBase::SetError( SbxERR_CONVERSION );
        case SbxEMPTY:
            nRes = SbxFALSE; break;
        case SbxCHAR:
            nRes = p->nChar ? SbxTRUE : SbxFALSE; break;
        case SbxBYTE:
            nRes = p->nByte ? SbxTRUE : SbxFALSE; break;
        case SbxINTEGER:
        case SbxBOOL:
            nRes = p->nInteger ? SbxTRUE : SbxFALSE; break;
        case SbxERROR:
        case SbxUSHORT:
            nRes = p->nUShort ? SbxTRUE : SbxFALSE; break;
        case SbxLONG:
            nRes = p->nLong ? SbxTRUE : SbxFALSE; break;
        case SbxULONG:
            nRes = p->nULong ? SbxTRUE : SbxFALSE; break;
        case SbxSINGLE:
            nRes = p->nSingle ? SbxTRUE : SbxFALSE; break;
        case SbxDATE:
        case SbxDOUBLE:
            nRes = p->nDouble ? SbxTRUE : SbxFALSE; break;
        case SbxDECIMAL:
        case SbxBYREF | SbxDECIMAL:
            {
            double dVal = 0.0;
            if( p->pDecimal )
                p->pDecimal->getDouble( dVal );
            nRes = dVal ? SbxTRUE : SbxFALSE;
            }
            break;
        case SbxSALINT64:
            nRes = p->nInt64 ? SbxTRUE : SbxFALSE; break;
        case SbxSALUINT64:
            nRes = p->uInt64 ? SbxTRUE : SbxFALSE; break;
        case SbxULONG64:
            nRes = !!p->nULong64 ? SbxTRUE : SbxFALSE; break;
        case SbxLONG64:
        case SbxCURRENCY:
            nRes = !!p->nLong64 ? SbxTRUE : SbxFALSE; break;
        case SbxBYREF | SbxSTRING:
        case SbxSTRING:
        case SbxLPSTR:
            nRes = SbxFALSE;
            if( p->pString )
            {
                if( p->pString->EqualsIgnoreCaseAscii( SbxRes( STRING_TRUE ) ) )
                    nRes = SbxTRUE;
                else if( !p->pString->EqualsIgnoreCaseAscii( SbxRes( STRING_FALSE ) ) )
                {
                    // Jetzt kann es noch in eine Zahl konvertierbar sein
                    BOOL bError = TRUE;
                    double n;
                    SbxDataType t;
                    USHORT nLen = 0;
                    String s( *p->pString );
                    if( ImpScan( s, n, t, &nLen ) == SbxERR_OK )
                    {
                        if( nLen == s.Len() )
                        {
                            bError = FALSE;
                            if( n != 0.0 )
                                nRes = SbxTRUE;
                        }
                    }
                    if( bError )
                        SbxBase::SetError( SbxERR_CONVERSION );
                }
            }
            break;
        case SbxOBJECT:
        {
            SbxValue* pVal = PTR_CAST(SbxValue,p->pObj);
            if( pVal )
                nRes = pVal->GetBool() ? SbxTRUE : SbxFALSE;
            else
            {
                SbxBase::SetError( SbxERR_NO_OBJECT ); nRes = SbxFALSE;
            }
            break;
        }

        case SbxBYREF | SbxCHAR:
            nRes = *p->pChar ? SbxTRUE : SbxFALSE; break;
        case SbxBYREF | SbxBYTE:
            nRes = *p->pByte ? SbxTRUE : SbxFALSE; break;
        case SbxBYREF | SbxINTEGER:
        case SbxBYREF | SbxBOOL:
            nRes = *p->pInteger ? SbxTRUE : SbxFALSE; break;
        case SbxBYREF | SbxLONG:
            nRes = *p->pLong ? SbxTRUE : SbxFALSE; break;
        case SbxBYREF | SbxULONG:
            nRes = *p->pULong ? SbxTRUE : SbxFALSE; break;
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            nRes = *p->pUShort ? SbxTRUE : SbxFALSE; break;
        case SbxBYREF | SbxSINGLE:
            nRes = ( *p->pSingle != 0 ) ? SbxTRUE : SbxFALSE; break;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            nRes = ( *p->pDouble != 0 ) ? SbxTRUE : SbxFALSE; break;
        case SbxBYREF | SbxSALINT64:
            nRes = ( *p->pnInt64 ) ? SbxTRUE : SbxFALSE; break;
        case SbxBYREF | SbxSALUINT64:
            nRes = ( *p->puInt64 ) ? SbxTRUE : SbxFALSE; break;
        case SbxBYREF | SbxULONG64:
            nRes = !!*p->pULong64 ? SbxTRUE : SbxFALSE; break;
        case SbxBYREF | SbxLONG64:
        case SbxBYREF | SbxCURRENCY:
            nRes = !!*p->pLong64 ? SbxTRUE : SbxFALSE; break;

        default:
            SbxBase::SetError( SbxERR_CONVERSION ); nRes = SbxFALSE;
    }
    return nRes;
}

void ImpPutBool( SbxValues* p, INT16 n )
{
    if( n )
        n = SbxTRUE;
    switch( +p->eType )
    {
        case SbxCHAR:
            p->nChar = (sal_Unicode) n; break;
        case SbxUINT:
            p->nByte = (BYTE) n; break;
        case SbxINTEGER:
        case SbxBOOL:
            p->nInteger = n; break;
        case SbxLONG:
            p->nLong = n; break;
        case SbxULONG:
            p->nULong = (UINT32) n; break;
        case SbxERROR:
        case SbxUSHORT:
            p->nUShort = (UINT16) n; break;
        case SbxSINGLE:
            p->nSingle = n; break;
        case SbxDATE:
        case SbxDOUBLE:
            p->nDouble = n; break;
        case SbxSALINT64:
            p->nInt64 = n; break;
        case SbxSALUINT64:
            p->uInt64 = n; break;
        case SbxULONG64:
            p->nULong64.Set( (UINT32)n ); break;
        case SbxLONG64:
        case SbxCURRENCY:
            p->nLong64.Set( (INT32)n ); break;
        case SbxDECIMAL:
        case SbxBYREF | SbxDECIMAL:
            ImpCreateDecimal( p )->setInt( (INT16)n );
            break;

        case SbxBYREF | SbxSTRING:
        case SbxSTRING:
        case SbxLPSTR:
            if( !p->pString )
                p->pString = new XubString;
            *p->pString = SbxRes( n ? STRING_TRUE : STRING_FALSE );
            break;

        case SbxOBJECT:
        {
            SbxValue* pVal = PTR_CAST(SbxValue,p->pObj);
            if( pVal )
                pVal->PutBool( BOOL( n != 0 ) );
            else
                SbxBase::SetError( SbxERR_NO_OBJECT );
            break;
        }
        case SbxBYREF | SbxCHAR:
            *p->pChar = (sal_Unicode) n; break;
        case SbxBYREF | SbxBYTE:
            *p->pByte = (BYTE) n; break;
        case SbxBYREF | SbxINTEGER:
        case SbxBYREF | SbxBOOL:
            *p->pInteger = (INT16) n; break;
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            *p->pUShort = (UINT16) n; break;
        case SbxBYREF | SbxLONG:
            *p->pLong = n; break;
        case SbxBYREF | SbxULONG:
            *p->pULong = (UINT32) n; break;
        case SbxBYREF | SbxSINGLE:
            *p->pSingle = n; break;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            *p->pDouble = n; break;
        case SbxBYREF | SbxSALINT64:
            *p->pnInt64 = n; break;
        case SbxBYREF | SbxSALUINT64:
            *p->puInt64 = n; break;
        case SbxBYREF | SbxULONG64:
            p->pULong64->Set( (UINT32)n ); break;
        case SbxBYREF | SbxLONG64:
        case SbxBYREF | SbxCURRENCY:
            p->pLong64->Set( (INT32)n ); break;

        default:
            SbxBase::SetError( SbxERR_CONVERSION );
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
