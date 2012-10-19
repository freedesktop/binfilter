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
#include <vcl/svapp.hxx>
#include "sbx.hxx"

namespace binfilter {

class SbxSimpleCharClass
{
public:
    BOOL isAlpha( sal_Unicode c ) const
    {
        BOOL bRet = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
        return bRet;
    }

    BOOL isDigit( sal_Unicode c ) const
    {
        BOOL bRet = (c >= '0' && c <= '9');
        return bRet;
    }

    BOOL isAlphaNumeric( sal_Unicode c ) const
    {
        BOOL bRet = isDigit( c ) || isAlpha( c );
        return bRet;
    }
};


static SbxVariable* Element
    ( SbxObject* pObj, SbxObject* pGbl, const sal_Unicode** ppBuf,
      SbxClassType, const SbxSimpleCharClass& rCharClass );

static const sal_Unicode* SkipWhitespace( const sal_Unicode* p )
{
    while( *p && ( *p == ' ' || *p == '\t' ) )
        p++;
    return p;
}

// Scannen eines Symbol. Das Symbol wird in rSym eingetragen, der Returnwert
// ist die neue Scanposition. Bei Fehlern ist das Symbol leer.

static const sal_Unicode* Symbol( const sal_Unicode* p, XubString& rSym, const SbxSimpleCharClass& rCharClass )
{
    USHORT nLen = 0;
    // Haben wir ein Sondersymbol?
    if( *p == '[' )
    {
        rSym = ++p;
        while( *p && *p != ']' )
            p++, nLen++;
        p++;
    }
    else
    {
        // Ein Symbol muss mit einem Buchstaben oder einem Underline beginnen
        if( !rCharClass.isAlpha( *p ) && *p != '_' )
            SbxBase::SetError( SbxERR_SYNTAX );
        else
        {
            rSym = p;
            // Dann darf es Buchstaben, Zahlen oder Underlines enthalten
            while( *p && (rCharClass.isAlphaNumeric( *p ) || *p == '_') )
                p++, nLen++;
            // BASIC-Standard-Suffixe werden ignoriert
            if( *p && (*p == '%' || *p == '&' || *p == '!' || *p == '#' || *p == '$' ) )
                p++;
        }
    }
    rSym.Erase( nLen );
    return p;
}

// Qualifizierter Name. Element.Element....

static SbxVariable* QualifiedName
    ( SbxObject* pObj, SbxObject* pGbl, const sal_Unicode** ppBuf, SbxClassType t )
{
    static SbxSimpleCharClass aCharClass;

    SbxVariableRef refVar;
    const sal_Unicode* p = SkipWhitespace( *ppBuf );
    if( aCharClass.isAlpha( *p ) || *p == '_' || *p == '[' )
    {
        // Element einlesen
        refVar = Element( pObj, pGbl, &p, t, aCharClass );
        while( refVar.Is() && (*p == '.' || *p == '!') )
        {
            // Es folgt noch ein Objektelement. Das aktuelle Element
            // muss also ein SBX-Objekt sein oder liefern!
            pObj = PTR_CAST(SbxObject,(SbxVariable*) refVar);
            if( !pObj )
                // Dann muss es ein Objekt liefern
                pObj = PTR_CAST(SbxObject,refVar->GetObject());
            refVar.Clear();
            if( !pObj )
                break;
            p++;
            // Und das naechste Element bitte
            refVar = Element( pObj, pGbl, &p, t, aCharClass );
        }
    }
    else
        SbxBase::SetError( SbxERR_SYNTAX );
    *ppBuf = p;
    if( refVar.Is() )
        refVar->AddRef();
    return refVar;
}

// Einlesen eines Operanden. Dies kann eine Zahl, ein String oder
// eine Funktion (mit optionalen Parametern) sein.

static SbxVariable* Operand
    ( SbxObject* pObj, SbxObject* pGbl, const sal_Unicode** ppBuf, BOOL bVar )
{
    static SbxSimpleCharClass aCharClass;

    SbxVariableRef refVar( new SbxVariable );
    const sal_Unicode* p = SkipWhitespace( *ppBuf );
    if( !bVar && ( aCharClass.isDigit( *p )
     || ( *p == '.' && aCharClass.isDigit( *( p+1 ) ) )
     || *p == '-'
     || *p == '&' ) )
    {
        // Eine Zahl kann direkt eingescant werden!
        USHORT nLen;
        if( !refVar->Scan( XubString( p ), &nLen ) )
            refVar.Clear();
        else
            p += nLen;
    }
    else if( !bVar && *p == '"' )
    {
        // Ein String
        XubString aString;
        p++;
        for( ;; )
        {
            // Das ist wohl ein Fehler
            if( !*p )
                return NULL;
            // Doppelte Quotes sind OK
            if( *p == '"' )
                if( *++p != '"' )
                    break;
            aString += *p++;
        }
        refVar->PutString( aString );
    }
    else
        refVar = QualifiedName( pObj, pGbl, &p, SbxCLASS_DONTCARE );
    *ppBuf = p;
    if( refVar.Is() )
        refVar->AddRef();
    return refVar;
}

// Einlesen einer einfachen Term. Die Operatoren +, -, * und /
// werden unterstuetzt.

static SbxVariable* MulDiv( SbxObject* pObj, SbxObject* pGbl, const sal_Unicode** ppBuf )
{
    const sal_Unicode* p = *ppBuf;
    SbxVariableRef refVar( Operand( pObj, pGbl, &p, FALSE ) );
    p = SkipWhitespace( p );
    while( refVar.Is() && ( *p == '*' || *p == '/' ) )
    {
        sal_Unicode cOp = *p++;
        SbxVariableRef refVar2( Operand( pObj, pGbl, &p, FALSE ) );
        if( refVar2.Is() )
        {
            // temporaere Variable!
            SbxVariable* pVar = refVar;
            pVar = new SbxVariable( *pVar );
            refVar = pVar;
            if( cOp == '*' )
                *refVar *= *refVar2;
            else
                *refVar /= *refVar2;
        }
        else
        {
            refVar.Clear();
            break;
        }
    }
    *ppBuf = p;
    if( refVar.Is() )
        refVar->AddRef();
    return refVar;
}

static SbxVariable* PlusMinus( SbxObject* pObj, SbxObject* pGbl, const sal_Unicode** ppBuf )
{
    const sal_Unicode* p = *ppBuf;
    SbxVariableRef refVar( MulDiv( pObj, pGbl, &p ) );
    p = SkipWhitespace( p );
    while( refVar.Is() && ( *p == '+' || *p == '-' ) )
    {
        sal_Unicode cOp = *p++;
        SbxVariableRef refVar2( MulDiv( pObj, pGbl, &p ) );
        if( refVar2.Is() )
        {
            // temporaere Variable!
            SbxVariable* pVar = refVar;
            pVar = new SbxVariable( *pVar );
            refVar = pVar;
            if( cOp == '+' )
                *refVar += *refVar2;
            else
                *refVar -= *refVar2;
        }
        else
        {
            refVar.Clear();
            break;
        }
    }
    *ppBuf = p;
    if( refVar.Is() )
        refVar->AddRef();
    return refVar;
}


static SbxVariable* Element
    ( SbxObject* pObj, SbxObject* pGbl, const sal_Unicode** ppBuf,
      SbxClassType t, const SbxSimpleCharClass& rCharClass )
{
    XubString aSym;
    const sal_Unicode* p = Symbol( *ppBuf, aSym, rCharClass );
    SbxVariableRef refVar;
    if( aSym.Len() )
    {
        USHORT nOld = pObj->GetFlags();
        if( pObj == pGbl )
            pObj->SetFlag( SBX_GBLSEARCH );
        refVar = pObj->Find( aSym, t );
        pObj->SetFlags( nOld );
        if( refVar.Is() )
        {
            refVar->SetParameters( NULL );
            // folgen noch Parameter?
            p = SkipWhitespace( p );
            if( *p == '(' )
            {
                p++;
                SbxArrayRef refPar = new SbxArray;
                USHORT nArg = 0;
                // Wird sind mal relaxed und akzeptieren auch
                // das Zeilen- oder Komandoende als Begrenzer
                // Parameter immer global suchen!
                while( *p && *p != ')' && *p != ']' )
                {
                    SbxVariableRef refArg = PlusMinus( pGbl, pGbl, &p );
                    if( !refArg )
                    {
                        // Fehler beim Parsing
                        refVar.Clear(); break;
                    }
                    else
                    {
                        // Man kopiere den Parameter, damit
                        // man den aktuellen Zustand hat (loest auch
                        // den Aufruf per Zugriff aus)
                        SbxVariable* pArg = refArg;
                        refPar->Put( new SbxVariable( *pArg ), ++nArg );
                    }
                    p = SkipWhitespace( p );
                    if( *p == ',' )
                        p++;
                }
                if( *p == ')' )
                    p++;
                if( refVar.Is() )
                    refVar->SetParameters( refPar );
            }
        }
        else
            SbxBase::SetError( SbxERR_NO_METHOD );
    }
    *ppBuf = p;
    if( refVar.Is() )
        refVar->AddRef();
    return refVar;
}


SbxVariable* SbxObject::FindQualified( const XubString& rName, SbxClassType t )
{
    SbxVariable* pVar = NULL;
    const sal_Unicode* p = rName.GetBuffer();
    p = SkipWhitespace( p );
    if( !*p )
        return NULL;;
    pVar = QualifiedName( this, this, &p, t );
    p = SkipWhitespace( p );
    if( *p )
        SetError( SbxERR_SYNTAX );
    return pVar;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
