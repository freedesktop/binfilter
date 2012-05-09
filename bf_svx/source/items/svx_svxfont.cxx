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

// include ----------------------------------------------------------------

#include <vcl/print.hxx>
#include <unotools/charclass.hxx>
#include <unolingu.hxx>
#ifdef _MSC_VER
#pragma hdrstop
#endif

#define ITEMID_ESCAPEMENT   0

#define _SVX_SVXFONT_CXX

#include "svxfont.hxx"
namespace binfilter {

/*************************************************************************
 *                      class SvxFont
 *************************************************************************/

/*N*/ SvxFont::SvxFont()
/*N*/ {
/*N*/   nKern = nEsc = 0;
/*N*/   nPropr = 100;
/*N*/   eCaseMap = SVX_CASEMAP_NOT_MAPPED;
/*N*/   eLang = LANGUAGE_SYSTEM;
/*N*/ }

/*N*/ SvxFont::SvxFont( const Font &rFont )
/*N*/   : Font( rFont )
/*N*/ {
/*N*/   nKern = nEsc = 0;
/*N*/   nPropr = 100;
/*N*/   eCaseMap = SVX_CASEMAP_NOT_MAPPED;
/*N*/   eLang = LANGUAGE_SYSTEM;
/*N*/ }

/*************************************************************************
 *                      class SvxFont: Copy-Ctor
 *************************************************************************/

/*N*/ SvxFont::SvxFont( const SvxFont &rFont )
/*N*/   : Font( rFont )
/*N*/ {
/*N*/   nKern = rFont.GetFixKerning();
/*N*/   nEsc  = rFont.GetEscapement();
/*N*/   nPropr = rFont.GetPropr();
/*N*/   eCaseMap = rFont.GetCaseMap();
/*N*/   eLang = rFont.GetLanguage();
/*N*/ }

/*************************************************************************
 *                      SvxFont::CalcCaseMap
 *************************************************************************/

/*N*/ XubString SvxFont::CalcCaseMap( const XubString &rTxt ) const
/*N*/ {
/*N*/   if( !IsCaseMap() || !rTxt.Len() ) return rTxt;
/*N*/   XubString aTxt( rTxt );
/*N*/   // Ich muss mir noch die Sprache besorgen
/*N*/   const LanguageType eLng = LANGUAGE_DONTKNOW == eLang
/*N*/                           ? LANGUAGE_SYSTEM : eLang;
/*N*/
/*N*/   CharClass aCharClass( SvxCreateLocale( eLng ) );
/*N*/
/*N*/   switch( eCaseMap )
/*N*/   {
/*N*/       case SVX_CASEMAP_KAPITAELCHEN:
/*N*/       case SVX_CASEMAP_VERSALIEN:
/*N*/       {
/*N*/           aTxt = aCharClass.uppercase( aTxt );
/*N*/           break;
/*N*/       }
/*N*/
/*N*/       case SVX_CASEMAP_GEMEINE:
/*N*/       {
/*?*/           aTxt = aCharClass.lowercase( aTxt );
/*?*/           break;
/*?*/       }
/*?*/       case SVX_CASEMAP_TITEL:
/*?*/       {
/*?*/           // Jeder Wortbeginn wird gross geschrieben,
/*?*/           // der Rest des Wortes wird unbesehen uebernommen.
/*?*/           // Bug: wenn das Attribut mitten im Wort beginnt.
/*?*/           BOOL bBlank = TRUE;
/*?*/
/*?*/           for( UINT32 i = 0; i < aTxt.Len(); ++i )
/*?*/           {
/*?*/               if( sal_Unicode(' ') == aTxt.GetChar(i) || sal_Unicode('\t') == aTxt.GetChar(i) )
/*?*/                   bBlank = TRUE;
/*?*/               else
/*?*/               {
/*?*/                   if( bBlank )
/*?*/                   {
/*?*/                       String aTemp( aTxt.GetChar( i ) );
/*?*/                       aTemp = aCharClass.uppercase( aTemp );
/*?*/                       aTxt.Replace( i, 1, aTemp );
/*?*/                   }
/*?*/                   bBlank = FALSE;
/*?*/               }
/*?*/           }
/*?*/           break;
/*?*/       }
/*?*/       default:
/*?*/       {
/*?*/           DBG_ASSERT(!this, "SvxFont::CaseMapTxt: unknown casemap");
/*N*/           break;
/*N*/       }
/*N*/   }
/*N*/   return aTxt;
/*N*/ }

/**************************************************************************
 *                    SvxFont::SetPhysFont()
 *************************************************************************/

/*N*/ void SvxFont::SetPhysFont( OutputDevice *pOut ) const
/*N*/ {
/*N*/   const Font& rCurrentFont = pOut->GetFont();
/*N*/   if ( nPropr == 100 )
/*N*/   {
/*N*/       if ( !rCurrentFont.IsSameInstance( *this ) )
/*N*/           pOut->SetFont( *this );
/*N*/   }
/*N*/   else
/*N*/   {
/*N*/       Font aNewFont( *this );
/*N*/       Size aSize( aNewFont.GetSize() );
/*N*/       aNewFont.SetSize( Size( aSize.Width() * nPropr / 100L,
/*N*/                                   aSize.Height() * nPropr / 100L ) );
/*N*/       if ( !rCurrentFont.IsSameInstance( aNewFont ) )
/*N*/           pOut->SetFont( aNewFont );
/*N*/   }
/*N*/ }


/*************************************************************************
 *                    SvxFont::GetPhysTxtSize()
 *************************************************************************/


/*N*/ Size SvxFont::GetPhysTxtSize( const OutputDevice *pOut, const XubString &rTxt )
/*N*/ {
/*N*/   if ( !IsCaseMap() && !IsKern() )
/*N*/       return Size( pOut->GetTextWidth( rTxt ), pOut->GetTextHeight() );
/*N*/
/*?*/   Size aTxtSize;
/*?*/   aTxtSize.setHeight( pOut->GetTextHeight() );
/*?*/   if ( !IsCaseMap() )
/*?*/       aTxtSize.setWidth( pOut->GetTextWidth( rTxt ) );
/*?*/   else
/*?*/       aTxtSize.setWidth( pOut->GetTextWidth( CalcCaseMap( rTxt ) ) );
/*?*/
/*?*/   if( IsKern() && ( rTxt.Len() > 1 ) )
/*?*/       aTxtSize.Width() += ( ( rTxt.Len()-1 ) * long( nKern ) );
/*?*/
/*?*/   return aTxtSize;
/*N*/ }

/*N*/ Size SvxFont::QuickGetTextSize( const OutputDevice *pOut, const XubString &rTxt,
/*N*/                        const USHORT nIdx, const USHORT nLen, sal_Int32* pDXArray ) const
/*N*/ {
/*N*/   if ( !IsCaseMap() && !IsKern() )
/*N*/       return Size( pOut->GetTextArray( rTxt, pDXArray, nIdx, nLen ),
/*N*/                    pOut->GetTextHeight() );
/*N*/
/*N*/   Size aTxtSize;
/*N*/   aTxtSize.setHeight( pOut->GetTextHeight() );
/*N*/   if ( !IsCaseMap() )
/*N*/       aTxtSize.setWidth( pOut->GetTextArray( rTxt, pDXArray, nIdx, nLen ) );
/*N*/   else
/*?*/       aTxtSize.setWidth( pOut->GetTextArray( CalcCaseMap( rTxt ),
/*?*/                          pDXArray, nIdx, nLen ) );
/*N*/
/*N*/   if( IsKern() && ( nLen > 1 ) )
/*N*/   {
/*N*/       aTxtSize.Width() += ( ( nLen-1 ) * long( nKern ) );
/*N*/
/*N*/       if ( pDXArray )
/*N*/       {
/*N*/           for ( xub_StrLen i = 0; i < nLen; i++ )
/*N*/               pDXArray[i] += ( (i+1) * long( nKern ) );
/*N*/           // Der letzte ist um ein nKern zu gross:
/*N*/           pDXArray[nLen-1] -= nKern;
/*N*/       }
/*N*/   }
/*N*/   return aTxtSize;
/*N*/ }

/*************************************************************************
 *                    SvxFont::DrawText()
 *************************************************************************/

/*N*/ void SvxFont::QuickDrawText( OutputDevice *pOut,
/*N*/   const Point &rPos, const XubString &rTxt,
/*N*/   const xub_StrLen nIdx, const xub_StrLen nLen, const sal_Int32* pDXArray ) const
/*N*/ {
/*N*/   // Font muss ins OutputDevice selektiert sein...
/*N*/   if ( !IsCaseMap() && !IsCapital() && !IsKern() && !IsEsc() )
/*N*/   {
/*N*/       pOut->DrawTextArray( rPos, rTxt, pDXArray, nIdx, nLen );
/*N*/       return;
/*N*/   }
/*N*/
/*?*/   Point aPos( rPos );
/*?*/
/*?*/     if ( nEsc )
/*?*/     {
/*?*/         long nDiff = GetSize().Height();
/*?*/         nDiff *= nEsc;
/*?*/         nDiff /= 100;
/*?*/
/*?*/         if ( !IsVertical() )
/*?*/           aPos.Y() -= nDiff;
/*?*/         else
/*?*/             aPos.X() += nDiff;
/*?*/     }
/*?*/
/*?*/   if( !IsCapital() )
/*?*/   {
/*?*/       if ( !( IsKern() && !pDXArray ) )
/*?*/       {
/*?*/           if ( !IsCaseMap() )
/*?*/               pOut->DrawTextArray( aPos, rTxt, pDXArray, nIdx, nLen );
/*?*/           else
/*?*/               pOut->DrawTextArray( aPos, CalcCaseMap( rTxt ), pDXArray, nIdx, nLen );
/*?*/       }
/*?*/   }
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxFont& SvxFont::operator=( const Font& rFont )
/*N*/ {
/*N*/   Font::operator=( rFont );
/*N*/   return *this;
/*N*/ }

/*N*/ SvxFont& SvxFont::operator=( const SvxFont& rFont )
/*N*/ {
/*N*/   Font::operator=( rFont );
/*N*/   eLang = rFont.eLang;
/*N*/   eCaseMap = rFont.eCaseMap;
/*N*/   nEsc = rFont.nEsc;
/*N*/   nPropr = rFont.nPropr;
/*N*/   nKern = rFont.nKern;
/*N*/   return *this;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
