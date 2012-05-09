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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <hintids.hxx>

#if !(defined _SVSTDARR_STRINGS_DECL && \
      defined _SVSTDARR_USHORTS_DECL && defined _SVSTDARR_XUB_STRLEN_DECL && \
      defined _SVSTDARR_BOOLS_DECL)
#define _SVSTDARR_STRINGS
#define _SVSTDARR_BYTESTRINGS
#define _SVSTDARR_USHORTS
#define _SVSTDARR_XUB_STRLEN
#define _SVSTDARR_BOOLS
#endif

#include <bf_svtools/imap.hxx>
#include <bf_svtools/urihelper.hxx>
#include <bf_svx/fontitem.hxx>
#include <bf_svx/cscoitem.hxx>
#include <bf_svx/lrspitem.hxx>
#include <bf_svx/tstpitem.hxx>

#include <horiornt.hxx>

#include <doc.hxx>

#include <osl/diagnose.h>

#include <pam.hxx>
#include <fmtanchr.hxx>
#include <txtftn.hxx>
#include <fmturl.hxx>
#include <fchrfmt.hxx>
#include <fmtftn.hxx>
#include <fmtflcnt.hxx>
#include <fmtfld.hxx>
#include <fmtinfmt.hxx>
#include <txtflcnt.hxx>
#include <charatr.hxx>
#include <frmfmt.hxx>
#include <charfmt.hxx>
#include <paratr.hxx>
#include <poolfmt.hxx>
#include <sw3io.hxx>
#include <sw3imp.hxx>
#include <ndtxt.hxx>
#include <ndgrf.hxx>
#include <ndole.hxx>
#include <crypter.hxx>
#include <wrong.hxx>
#include <tox.hxx>
#include <fmthbsh.hxx>

// Export
#include <fldbas.hxx>
#include <frmatr.hxx>

#include <swerror.h>
#include <SwStyleNameMapper.hxx>
#include "bf_so3/staticbaseurl.hxx"
#include <bf_tools/string.hxx>
namespace binfilter {

#define URL_DECODE  \
    , INetURLObject::WAS_ENCODED, INetURLObject::DECODE_UNAMBIGUOUS

/*N*/ SV_IMPL_PTRARR_SORT(Sw3SortFmts,SwFmtPtr)

////////////////////////////////////////////////////////////////////////////

// Vorbereitung des Exports eines Text-Nodes in das Sw31 File-Format
// (dazu muessen SwFmtInetFmt-Hints in Felder umgewandelt werden)
// Idee:
// Die Start- und End-Positionen aller Hints werden in zwei USHORT-Arrays
// gespeichert, die SfxPoolItems der Hints in einem dritten. Diese
// "Hint"-Arrays werden entsprechend aufbereitet und dann statt der
// Original-Hints ausgegeben.
//
// Wie wird aufbereitet?
// Die "Hints" werden zunaechst in die Arrays kopiert. dabei werden
// - Schachtelungen von SwFmtINetFmt-Hints aufgeloest.
// - leere SwFmtINetFmt-Hints "entfernt"
// - Hints innerhalb von SwFmtINetFmt-Hints "entfernt"
//
// Danach werden die Texte der SwFmtINetFmt-Hints extrahiert und der
// Text des Nodes sowie die Htnt-Psotionen an die Verwendung von Feldern
// angepasst.

SV_DECL_PTRARR(SfxPoolItems,SfxPoolItem * ,16,16)

class SwInsHardBlankSoftHyph
{
    SvXub_StrLens   aItemStarts;    // Start-Pos der Hints
    SfxPoolItems    aItems;         // Items der Hints
public:
    SwInsHardBlankSoftHyph() {}
    ~SwInsHardBlankSoftHyph();
    void AddItem( xub_StrLen nPos, sal_Unicode c );
};

/*N*/ sal_Char Sw3IoImp::ConvStarSymbolCharToStarBats( sal_Unicode c )
/*N*/ {
/*N*/   if( !hBatsFontConv )
/*N*/   {
/*N*/       hBatsFontConv = CreateFontToSubsFontConverter( sStarSymbol,
/*N*/                FONTTOSUBSFONT_EXPORT|FONTTOSUBSFONT_ONLYOLDSOSYMBOLFONTS );
/*N*/       OSL_ENSURE( hBatsFontConv, "Got no symbol font converter" );
/*N*/   }
/*N*/   if( hBatsFontConv )
/*N*/   {
/*N*/       c = ConvertFontToSubsFontChar( hBatsFontConv, c );
/*N*/   }
/*N*/
/*N*/   return (sal_Char)c;
/*N*/ }

/*N*/ sal_Unicode Sw3IoImp::ConvStarBatsCharToStarSymbol( sal_Char c )
/*N*/ {
/*N*/   sal_Unicode cNew = (sal_Unicode)(sal_uChar)c;
/*N*/   if( !hBatsFontConv )
/*N*/   {
/*N*/       hBatsFontConv = CreateFontToSubsFontConverter( sStarBats,
/*N*/                FONTTOSUBSFONT_IMPORT|FONTTOSUBSFONT_ONLYOLDSOSYMBOLFONTS );
/*N*/       OSL_ENSURE( hBatsFontConv, "Got no symbol font converter" );
/*N*/   }
/*N*/   if( hBatsFontConv )
/*N*/   {
/*N*/       cNew = ConvertFontToSubsFontChar( hBatsFontConv, (sal_Unicode)(sal_uChar)c + 0xf000 );
/*N*/   }
/*N*/
/*N*/   return cNew;
/*N*/ }

sal_Unicode Sw3IoImp::ConvStarMathCharToStarSymbol( sal_Char c )
{
    sal_Unicode cNew = c;
    if( !hMathFontConv )
    {
        hMathFontConv = CreateFontToSubsFontConverter( sStarMath,
                 FONTTOSUBSFONT_IMPORT|FONTTOSUBSFONT_ONLYOLDSOSYMBOLFONTS );
        OSL_ENSURE( hMathFontConv, "Got no symbol font converter" );
    }
    if( hMathFontConv )
    {
        cNew = ConvertFontToSubsFontChar( hMathFontConv, (sal_Unicode)(sal_uChar)c + 0xf000 );
    }

    return cNew;
}

/*N*/ sal_Bool lcl_sw3io_isStarSymbolFontItem( const SvxFontItem& rFontItem )
/*N*/ {
/*N*/   return ( rFontItem.GetFamilyName().EqualsAscii( "StarSymbol", 0, sizeof("StarSymbol")-1 ) ||
/*N*/            rFontItem.GetFamilyName().EqualsAscii( "OpenSymbol", 0, sizeof("OpenSymbol")-1 ) );
/*N*/ }

// Hilfsroutine fuer ConvertText: Suche nach dem naechsten Hint,
// der eine Konversion verbietet. Zur Zeit sind dies Hints, die entweder
// direkt oder indirekt auf einen Font mit CHARSET_SYMBOL hinweisen.
/*N*/ const SvxFontItem *lcl_sw3io_getNextFontHint( const SwpHints* pHints, USHORT& rHint,
/*N*/                               xub_StrLen& rStart, xub_StrLen& rEnd,
/*N*/                               sal_Bool& rIsMathOrBatsFontItem,
/*N*/                               Sw3Fmts *pConvToSymbolFmts,
/*N*/                               const SvxFontItem& rStarBatsItem,
/*N*/                               const SvxFontItem& rStarMathItem )
/*N*/ {
/*N*/   rStart = rEnd = (xub_StrLen)-1;
/*N*/   rIsMathOrBatsFontItem = sal_False;
/*N*/
/*N*/   if( !pHints )
/*N*/       return 0;
/*N*/
/*N*/   const SvxFontItem *pFontItem = 0;
/*N*/   while( rHint < pHints->Count() )
/*N*/   {
/*N*/       const SwTxtAttr* pHnt = (*pHints) [rHint++];
/*N*/       if( pHnt->Which() == RES_CHRATR_FONT )
/*N*/       {
/*N*/           rStart = *pHnt->GetStart();
/*N*/           rEnd   = *pHnt->GetEnd();
/*N*/           pFontItem = &pHnt->GetFont();
/*N*/           rIsMathOrBatsFontItem =
/*N*/               RTL_TEXTENCODING_SYMBOL == pFontItem->GetCharSet() &&
/*N*/               ( pFontItem->GetFamilyName().EqualsIgnoreCaseAscii( "StarBats", 0, sizeof("StarBats")-1 ) ||
/*N*/                 pFontItem->GetFamilyName().EqualsIgnoreCaseAscii( "StarMath", 0, sizeof("StarMath")-1 ) );
/*N*/           break;
/*N*/       }
/*N*/       // Gibt es einen CharFormat-Hint mit einem Symbol-Font?
/*N*/       else if( pHnt->Which() == RES_TXTATR_CHARFMT )
/*N*/       {
/*N*/           SwCharFmt* pFmt = pHnt->GetCharFmt().GetCharFmt();
/*N*/           if( pFmt->GetAttrSet().GetItemState( RES_CHRATR_FONT, FALSE )
/*N*/               == SFX_ITEM_SET )
/*N*/           {
/*N*/               rStart = *pHnt->GetStart();
/*N*/               rEnd   = *pHnt->GetEnd();
/*N*/               pFontItem = &pFmt->GetFont();
/*N*/               if( pConvToSymbolFmts &&
/*N*/                   lcl_sw3io_isStarSymbolFontItem( *pFontItem ) )
/*N*/               {
/*?*/                   BYTE nFlags = pConvToSymbolFmts->GetFlags( pFmt );
/*?*/                   if( (SW3IO_CONV_FROM_BATS & nFlags) != 0 )
/*?*/                       pFontItem = &rStarBatsItem;
/*?*/                   else if( (SW3IO_CONV_FROM_MATH & nFlags) != 0 )
/*?*/                       pFontItem = &rStarMathItem;
/*N*/               }
/*N*/               break;
/*N*/           }
/*N*/       }
/*N*/   }
/*N*/
/*N*/   return pFontItem;
/*N*/ }

// Text Node konvertieren
// Wird aufgerufen, wenn sich die Systeme unterscheiden. Der Text wird
// vom einen in den anderen Zeichensatz konvertiert. Nicht konvertierbare
// Zeichen werden farblich unterlegt; Hints mit CHARSET_SYMBOL-Zeichensaetzen
// werden uebersprungen

/*N*/ const SwTxtAttr* lcl_sw3io_hasTxtAttr( const SwpHints *pHints, xub_StrLen nIdx )
/*N*/ {
/*N*/   const SwTxtAttr* pRet = 0;
/*N*/   if( pHints )
/*N*/   {
/*N*/       USHORT nHints = pHints->Count();
/*N*/       for( USHORT i = 0; i < nHints; i++ )
/*N*/       {
/*N*/           const SwTxtAttr *pPos = (*pHints)[i];
/*N*/           const xub_StrLen nStart = *pPos->GetStart();
/*N*/           if( nIdx == nStart && !pPos->GetEnd() )
/*N*/           {
/*N*/               pRet = pPos;
/*N*/               break;
/*N*/           }
/*N*/           if( nStart > nIdx )
/*N*/               break;
/*N*/       }
/*N*/   }
/*N*/
/*N*/   return pRet;
/*N*/ }

/*N*/ sal_Bool Sw3IoImp::ConvertText( ByteString& rText8, String& rText,
/*N*/                           xub_StrLen nStart, xub_StrLen nEnd,
/*N*/                           xub_StrLen nOffset, const SwTxtNode& rNd,
/*N*/                           rtl_TextEncoding eEnc,
/*N*/                           const SvxFontItem& rFontItem,
/*N*/                           SwInsHardBlankSoftHyph* pHBSH, BOOL bTo8 )
/*N*/ {
/*N*/   rtl::OStringBuffer aText8(rText8);
/*N*/   sal_Bool bRet = sal_False;
/*N*/   const SwpHints *pHints = rNd.GetpSwpHints();
/*N*/   if( bTo8 )
/*N*/   {
/*N*/       sal_Bool bToBats = lcl_sw3io_isStarSymbolFontItem( rFontItem );
/*N*/       bRet = bToBats;
/*N*/       if( bToBats || RTL_TEXTENCODING_SYMBOL == rFontItem.GetCharSet() )
/*N*/       {
/*N*/           for( xub_StrLen nPos = nStart; nPos < nEnd; nPos++ )
/*N*/           {
/*N*/               sal_Unicode c = rText.GetChar( nPos );
/*N*/               switch ( c )
/*N*/               {
/*N*/               case CHAR_HARDBLANK:
/*N*/               case CHAR_HARDHYPHEN:
/*?*/               case CHAR_SOFTHYPHEN:
/*?*/                   if( pHBSH )
/*?*/                   {
/*?*/                       pHBSH->AddItem( nPos, c );
/*?*/                       c = '\xff';
/*?*/                   }
/*?*/                   break;
/*N*/
/*N*/               case CH_TXTATR_BREAKWORD:
/*N*/               case CH_TXTATR_INWORD:
/*N*/                   if( lcl_sw3io_hasTxtAttr( pHints, nPos+nOffset ) )
/*N*/                       c = '\xff';
/*N*/                   break;
/*N*/               }
/*N*/               if( bToBats )
/*N*/                   aText8.append(ConvStarSymbolCharToStarBats(c));
/*N*/               else
/*N*/                   aText8.append(static_cast<sal_Char>(c));
/*N*/           }
/*N*/       }
/*N*/       else
/*N*/       {
/*N*/           xub_StrLen nCopy = nStart;
/*N*/           for( xub_StrLen nPos = nStart; nPos < nEnd; nPos++ )
/*N*/           {
/*N*/               sal_Unicode c = rText.GetChar( nPos );
/*N*/               BOOL bToFF = FALSE;
/*N*/               switch ( c )
/*N*/               {
/*N*/               case CHAR_HARDBLANK:
/*N*/               case CHAR_HARDHYPHEN:
/*N*/               case CHAR_SOFTHYPHEN:
/*N*/                   if( pHBSH )
/*N*/                   {
/*N*/                       pHBSH->AddItem( nPos, c );
/*N*/                       bToFF = TRUE;
/*N*/                   }
/*N*/                   break;
/*N*/
/*N*/               case CH_TXTATR_BREAKWORD:
/*N*/               case CH_TXTATR_INWORD:
/*N*/                   bToFF = 0 != lcl_sw3io_hasTxtAttr( pHints, nPos+nOffset );
/*N*/                   break;
/*N*/               }
/*N*/               if( bToFF )
/*N*/               {
/*N*/                   if( nCopy < nPos )
/*N*/                       aText8.append(rtl::OUStringToOString(
                                rText.Copy(nCopy,nPos-nCopy), eEnc));
/*N*/                   aText8.append('\xff');
/*N*/                   nCopy = nPos + 1;
/*N*/               }
/*N*/           }
/*N*/           if( nCopy < nEnd )
/*N*/               aText8.append(rtl::OUStringToOString(
                        rText.Copy(nCopy,nEnd-nCopy), eEnc));
/*N*/       }
/*N*/   }
/*N*/   else
/*N*/   {
/*N*/       const SwTxtAttr* pTAttr;
/*N*/       if( RTL_TEXTENCODING_SYMBOL == rFontItem.GetCharSet() )
/*N*/       {
/*N*/           sal_Bool bBatsToSymbol =
/*N*/               rFontItem.GetFamilyName().EqualsIgnoreCaseAscii( sStarBats );
/*N*/           sal_Bool bMathToSymbol =
/*N*/               rFontItem.GetFamilyName().EqualsIgnoreCaseAscii( sStarMath );
/*N*/           bRet = bBatsToSymbol || bMathToSymbol;
/*N*/           for( xub_StrLen nPos = nStart; nPos < nEnd; nPos++ )
/*N*/           {
/*N*/               sal_Char c = aText8[nPos];
/*N*/               if( '\xff' == c && 0 != (pTAttr =
/*N*/                       lcl_sw3io_hasTxtAttr( pHints, nPos+nOffset )) )
/*N*/                   rText += GetCharOfTxtAttr( *pTAttr );
/*N*/               else if( bBatsToSymbol )
/*N*/                   rText += ConvStarBatsCharToStarSymbol( c );
/*N*/               else if( bMathToSymbol )
                        rText += ConvStarMathCharToStarSymbol( c );
/*N*/               else
/*N*/                   rText += ByteString::ConvertToUnicode( c,
/*N*/                                       RTL_TEXTENCODING_SYMBOL );
/*N*/           }
/*N*/       }
/*N*/       else
/*N*/       {
/*N*/           xub_StrLen nCopy = nStart;
/*N*/           for( xub_StrLen nPos = nStart; nPos < nEnd; nPos++ )
/*N*/           {
/*N*/               sal_Char c = aText8[nPos];
/*N*/               sal_Unicode cNew(0);
/*N*/               if( '\xff' == c )
/*N*/               {
/*N*/                   if( 0 != ( pTAttr = lcl_sw3io_hasTxtAttr(
/*N*/                                               pHints, nPos+nOffset )) ||
/*N*/                       CHAR_HARDBLANK ==
/*N*/                           ( cNew = rNd.GetTxt().GetChar( nPos+nOffset )) ||
/*N*/                       CHAR_HARDHYPHEN == cNew || CHAR_SOFTHYPHEN == cNew )
/*N*/                   {
/*N*/                       if( nCopy < nPos )
/*N*/                           rText += String(aText8.getStr()+nCopy,nPos-nCopy, eEnc);
/*N*/                       if( pTAttr )
/*N*/                           rText += GetCharOfTxtAttr( *pTAttr );
/*N*/                       else
/*N*/                           rText += cNew;
/*N*/                       nCopy = nPos + 1;
/*N*/                   }
/*N*/               }
/*N*/               else if( CHAR_SOFTHYPHEN ==rNd.GetTxt().GetChar( nPos+nOffset ) )
/*N*/               {
/*N*/                   // The original charcter has been converted into a soft
/*N*/                   // hyphen, but there was no text attribute at this position.
/*N*/                   // We then have to replace the soft hyphen with a hard one.
/*N*/                   // The check is based on the source char set and not on
/*N*/                   // the actual one. The assumption is here that there is
/*N*/                   // no difference in the position of the soft hyphen.
/*N*/                   // However, to not accidentially do a wrong conversion
/*N*/                   // we check this again. The only mistake we might make
/*N*/                   // if the assumption is wrong is to not convert a soft
/*N*/                   // hyphen.
/*N*/                   if( eEnc == eSrcSet ||
/*N*/                       CHAR_SOFTHYPHEN == ByteString::ConvertToUnicode( c, eEnc ) )
/*N*/                   {
/*N*/                       if( nCopy < nPos )
/*N*/                           rText += String(aText8.getStr()+nCopy,nPos-nCopy, eEnc);
/*N*/                       rText += '-';
/*N*/                       nCopy = nPos + 1;
/*N*/                   }
/*N*/               }
/*N*/           }
/*N*/           if( nCopy < nEnd )
/*N*/               rText += String(aText8.getStr()+nCopy, nEnd-nCopy, eEnc);
/*N*/       }
/*N*/   }
/*N*/   rText8 = aText8.makeStringAndClear();
/*N*/   return bRet;
/*N*/ }

typedef const SvxFontItem *SvxFontItemPtr;
SV_DECL_PTRARR( SvxFontItems, SvxFontItemPtr, 5, 5 )

typedef SwTxtAttr *SwTxtAttrPtr;
SV_DECL_PTRARR( SwTxtAttrs, SwTxtAttrPtr, 5, 5 )

/*N*/ void Sw3IoImp::ConvertText( ByteString& rText8, String& rText,
/*N*/                           xub_StrLen nOffset, SwTxtNode& rNd,
/*N*/                           rtl_TextEncoding eEnc, const SvxFontItem& rFontItem,
/*N*/                           SwInsHardBlankSoftHyph* pHBSH, BOOL bTo8 )
/*N*/
/*N*/ {
/*N*/   SvxFontItems aFontItemStack;
/*N*/   SvXub_StrLens aEndPosStack;
/*N*/   SwTxtAttrs aDeleteFontTxtAttrs;
/*N*/   SvXub_StrLens aInsertSymbolFontStartPoss;
/*N*/   SvXub_StrLens aInsertSymbolFontEndPoss;
/*N*/
/*N*/   SvxFontItem aStarBatsItem( FAMILY_DONTKNOW, sStarBats, aEmptyStr,
/*N*/                              PITCH_DONTKNOW, RTL_TEXTENCODING_SYMBOL );
/*N*/   SvxFontItem aStarMathItem( FAMILY_DONTKNOW, sStarMath, aEmptyStr,
/*N*/                              PITCH_DONTKNOW, RTL_TEXTENCODING_SYMBOL );
/*N*/
/*N*/   SwpHints *pHints = rNd.GetpSwpHints();
/*N*/   // find next
/*N*/   xub_StrLen nFntStart = (xub_StrLen)-1, nFntEnd = 0;
/*N*/   USHORT nHint = 0;
/*N*/   const SvxFontItem *pFontItem = &rFontItem;
/*N*/   sal_Bool bIsBatsOrMathFontItem;
/*N*/   const SvxFontItem *pNewFontItem =
/*N*/           lcl_sw3io_getNextFontHint( pHints, nHint, nFntStart,
/*N*/                                      nFntEnd, bIsBatsOrMathFontItem,
/*N*/                                      pConvToSymbolFmts, aStarBatsItem,
/*N*/                                      aStarMathItem );
/*N*/   if( !bTo8 && pNewFontItem && bIsBatsOrMathFontItem )
/*N*/       aDeleteFontTxtAttrs.Insert( pHints->GetHt( nHint-1), aDeleteFontTxtAttrs.Count() );
/*N*/   xub_StrLen nLen = nOffset + (bTo8 ? rText.Len() : rText8.Len() );
/*N*/   xub_StrLen nCopy = nOffset;
        xub_StrLen nPos=0;
/*N*/   for( nPos = 0; nPos < nLen; nPos++ )
/*N*/   {
/*N*/       if( aEndPosStack.Count() &&
/*N*/           nPos == aEndPosStack[aEndPosStack.Count()-1] )
/*N*/       {
/*N*/           if( nPos > nCopy )
/*N*/           {
/*N*/               sal_Bool bSymConv = ConvertText( rText8, rText, nCopy, nPos,
/*N*/                                                nOffset, rNd, eEnc,
/*N*/                                                *pFontItem, pHBSH, bTo8 );
/*N*/               if( bSymConv && !bTo8 )
/*N*/               {
/*N*/                   aInsertSymbolFontStartPoss.Insert( nCopy,
/*N*/                           aInsertSymbolFontStartPoss.Count() );
/*N*/                   aInsertSymbolFontEndPoss.Insert( nPos,
/*N*/                           aInsertSymbolFontEndPoss.Count() );
/*N*/               }
/*N*/               nCopy = nPos;
/*N*/           }
/*N*/           pFontItem = aFontItemStack[ aFontItemStack.Count()-1 ];
/*N*/           aFontItemStack.Remove( aFontItemStack.Count()-1 );
/*N*/           aEndPosStack.Remove( aEndPosStack.Count()-1 );
/*N*/       }
/*N*/       while( (xub_StrLen)-1 != nFntStart && nPos == nFntStart )
/*N*/       {
/*N*/           if( nPos > nCopy )
/*N*/           {
/*N*/               sal_Bool bSymConv = ConvertText( rText8, rText, nCopy, nPos,
/*N*/                                                nOffset, rNd,eEnc, *pFontItem,
/*N*/                                                pHBSH, bTo8 );
/*N*/               if( bSymConv && !bTo8 )
/*N*/               {
/*?*/                   aInsertSymbolFontStartPoss.Insert( nCopy,
/*?*/                           aInsertSymbolFontStartPoss.Count() );
/*?*/                   aInsertSymbolFontEndPoss.Insert( nPos,
/*?*/                           aInsertSymbolFontEndPoss.Count() );
/*N*/               }
/*N*/               nCopy = nPos;
/*N*/           }
/*N*/           aEndPosStack.Insert( nFntEnd, aEndPosStack.Count() );
/*N*/           aFontItemStack.Insert( pFontItem, aFontItemStack.Count() );
/*N*/           pFontItem = pNewFontItem;
/*N*/           pNewFontItem = lcl_sw3io_getNextFontHint( pHints, nHint, nFntStart,
/*N*/                                    nFntEnd, bIsBatsOrMathFontItem,
/*N*/                                    pConvToSymbolFmts, aStarBatsItem,
/*N*/                                    aStarMathItem);
/*N*/           if( !bTo8 && pNewFontItem && bIsBatsOrMathFontItem )
/*N*/               aDeleteFontTxtAttrs.Insert( pHints->GetHt( nHint-1 ), aDeleteFontTxtAttrs.Count() );
/*N*/       }
/*N*/   }
/*N*/   if( nLen > nCopy )
/*N*/   {
/*N*/       sal_Bool bSymConv = ConvertText( rText8, rText, nCopy, nLen, nOffset,
/*N*/                              rNd, eEnc, *pFontItem, pHBSH, bTo8 );
/*N*/       if( bSymConv && !bTo8 )
/*N*/       {
/*N*/           aInsertSymbolFontStartPoss.Insert( nCopy,
/*N*/                   aInsertSymbolFontStartPoss.Count() );
/*N*/           aInsertSymbolFontEndPoss.Insert( nPos,
/*N*/                   aInsertSymbolFontEndPoss.Count() );
/*N*/       }
/*N*/   }
/*N*/   while( aDeleteFontTxtAttrs.Count() )
/*N*/   {
/*N*/       SwTxtAttr *pAttr = aDeleteFontTxtAttrs[0];
/*N*/       aDeleteFontTxtAttrs.Remove( 0 );
/*N*/       rNd.Delete( pAttr );
/*N*/   }
/*N*/   if( aInsertSymbolFontStartPoss.Count() )
/*N*/   {
/*N*/       const Font& rSymbolFont = SwNumRule::GetDefBulletFont();
/*N*/       SvxFontItem aFontItem( rSymbolFont.GetFamily(), rSymbolFont.GetName(),
/*N*/                              rSymbolFont.GetStyleName(),
/*N*/                              rSymbolFont.GetPitch(),
/*N*/                              rSymbolFont.GetCharSet() );
/*N*/       for( USHORT i=0; i < aInsertSymbolFontStartPoss.Count(); i++ )
/*N*/       {
/*N*/           rNd.Insert( aFontItem, aInsertSymbolFontStartPoss[i],
/*N*/                       aInsertSymbolFontEndPoss[i] );
/*N*/       }
/*N*/   }
/*N*/ }

/*N*/ void Sw3IoImp::ConvertText( SwTxtNode& rNd, const ByteString& rText8,
/*N*/                           xub_StrLen nOffset,
/*N*/                           SvUShorts *pEncs, SvXub_StrLens *pPoss )
/*N*/ {
/*N*/   if( !rText8.Len() )
/*N*/       return;
/*N*/
/*N*/   const SvxFontItem& rFont = rNd.GetSwAttrSet().GetFont();
/*N*/   BOOL bNdSym = rFont.GetCharSet() == RTL_TEXTENCODING_SYMBOL;
/*N*/
/*N*/   String& rNdText = (String &)rNd.GetTxt();
/*N*/       String aText;
/*N*/       ByteString aText8( rText8 );
/*N*/       SvxFontItem aFontItem1 ( rFont );
/*N*/       if( pConvToSymbolFmts &&
/*N*/           lcl_sw3io_isStarSymbolFontItem( aFontItem1 ) )
/*N*/       {
/*?*/           BYTE nFlags = pConvToSymbolFmts->GetFlags( rNd.GetFmtColl() );
/*?*/           if( (SW3IO_CONV_FROM_BATS & nFlags) != 0 )
/*?*/           {
/*?*/               aFontItem1.GetFamilyName() = sStarBats;
/*?*/               aFontItem1.GetCharSet() = RTL_TEXTENCODING_SYMBOL;
/*?*/           }
/*?*/           else if( (SW3IO_CONV_FROM_MATH & nFlags) != 0 )
/*?*/           {
/*?*/               aFontItem1.GetFamilyName() = sStarMath;
/*?*/               aFontItem1.GetCharSet() = RTL_TEXTENCODING_SYMBOL;
/*?*/           }
/*N*/       }
/*N*/       ConvertText( aText8, aText, nOffset, rNd,
/*N*/                              eSrcSet, aFontItem1, 0, FALSE );
/*N*/       rNdText.Replace( nOffset, aText.Len(), aText );
/*N*/       if( bNdSym &&
/*N*/           SFX_ITEM_SET == rNd.GetSwAttrSet().GetItemState( RES_CHRATR_FONT,
/*N*/                                                            sal_False ) &&
/*N*/           ( rFont.GetFamilyName().EqualsIgnoreCaseAscii( sStarBats ) ||
/*N*/             rFont.GetFamilyName().EqualsIgnoreCaseAscii( sStarMath ) ) )
/*N*/       {
/*?*/           const Font& rSymbolFont = SwNumRule::GetDefBulletFont();
/*?*/           SvxFontItem aFontItem2( rSymbolFont.GetFamily(),
/*?*/                                  rSymbolFont.GetName(),
/*?*/                                  rSymbolFont.GetStyleName(),
/*?*/                                  rSymbolFont.GetPitch(),
/*?*/                                  rSymbolFont.GetCharSet() );
/*?*/           ((SwCntntNode&)rNd).SetAttr( aFontItem2 );
/*N*/       }
/*N*/
/*N*/   if( pEncs )
/*N*/   {
/*?*/       for( USHORT i=0; i < pEncs->Count(); i++ )
/*?*/       {
/*?*/           xub_StrLen nStart = (*pPoss)[2*i];
/*?*/           String aTmp( rText8.Copy( nStart - nOffset,
/*?*/                                     (*pPoss)[2*i+1] - nStart ),
/*?*/                        (rtl_TextEncoding)(*pEncs)[i] );
/*?*/           rNdText.Replace( nStart, aTmp.Len(), aTmp );
/*?*/       }
/*N*/   }
/*N*/ }

// Text Node einlesen
// Falls kein Node angegeben ist, wird ein neuer Node an der angegebenen
// Position erzeugt.
// nInsFirstPara - beim Document einfuegen, muss der erste Absatz
//                  sonderbehandelt werden (Absatz Attribute!)

/*N*/ void Sw3IoImp::InTxtNode( SwTxtNode* pNd, SwNodeIndex& rPos, xub_StrLen nOffset,
/*N*/                         BYTE nInsFirstPara )
/*N*/ {
/*N*/   SwTxtNode *pOldNd = pNd && pNd->GetDepends() ? pNd : 0;
/*N*/
/*N*/   SvStringsDtor *pINetFldTexts = 0;   // Texte aus Internet-Feldern
/*N*/   SvXub_StrLens *pINetFldPoss  = 0;       // Positionen der Internet-Felder
/*N*/
/*N*/   SvXub_StrLens *pErasePoss  = 0;         // Positionen der Draw-Formate
/*N*/
/*N*/   SvUShorts     *pCharSetColorEncs = 0;
/*N*/   SvXub_StrLens *pCharSetColorPoss  = 0;  //
/*N*/
/*N*/   OpenRec( SWG_TEXTNODE );
/*N*/   BYTE   cNumLevel = NO_NUMBERING;
/*N*/   USHORT nColl = IDX_DFLT_VALUE, nCondColl = IDX_DFLT_VALUE;
/*N*/   SwWrongList *pWrong = 0;
/*N*/   // 0x0L: Laenge der Daten
/*N*/   // 0x10: Numerierung folgt
/*N*/   // 0x20: Wrong-Liste ist nicht dirty
/*N*/   BYTE cFlags = OpenFlagRec();
/*N*/   *pStrm >> nColl;
/*N*/   if( !IsVersion(SWG_LONGIDX) && (cFlags & 0x10) )
/*N*/   {
/*N*/       *pStrm >> cNumLevel;
/*N*/       // Im SW31-format wurde fuer nicht numerierte
/*N*/       // Absaetzte noch ein NO_NUMLEVEL mit rausgeschrieben.
/*N*/       if( NO_NUM == cNumLevel  &&
/*N*/           IsVersion( SWG_NONUMLEVEL, SWG_DESKTOP40 ) &&
/*N*/           pStrm->Tell() != nFlagRecEnd )      // wenn noch Daten da sind
/*?*/           *pStrm >> cNumLevel;                // NO_NUM -> NO_NUMLEVEL
/*N*/
/*N*/       // Wenn ein NO_NUM gelesen wurde muss es noch in ein NO_NUMLEVEL
/*N*/       // umgewandelt werden.
/*N*/       bConvertNoNum |= (NO_NUM == cNumLevel);
/*N*/   }
/*N*/
/*N*/   if( IsVersion( SWG_CONDCOLLS, SWG_EXPORT31, SWG_DESKTOP40 ) )
/*N*/   {
/*N*/       // bedingte Vorlagen gibt es nicht im 31-Export-Format
/*N*/       *pStrm >> nCondColl;
/*N*/       if( IDX_DFLT_VALUE != nCondColl )
/*N*/       {
/*N*/           // es ist eine gesetzt, dieses ist die bedingte Vorlage
/*N*/           USHORT nTmp = nCondColl;
/*N*/           nCondColl = nColl;
/*N*/           nColl = nTmp;
/*N*/       }
/*N*/   }
/*N*/
/*N*/   CloseFlagRec();
/*N*/   SwTxtFmtColl* pColl = FindTxtColl( nColl );
/*N*/
/*N*/   // JP 07.08.00: set never the default text format collection on a node
/*N*/   if( pColl == pDoc->GetDfltTxtFmtColl() )
/*?*/       pColl = pDoc->GetTxtCollFromPool( RES_POOLCOLL_STANDARD );
/*N*/
/*N*/   // Der Text des Nodes darf nicht einfach so konvertiert werden!
/*N*/   rtl::OString aText8 = read_lenPrefixed_uInt8s_ToOString<sal_uInt16>(*pStrm);
/*N*/   if( pCrypter )
        {
            rtl::OStringBuffer aBuffer(aText8);
            pCrypter->Decrypt( aBuffer );
            aText8 = aBuffer.makeStringAndClear();
        }
/*N*/   String aText( aText8, eSrcSet );
/*N*/   if( !pNd )
/*N*/   {
/*N*/       pNd = pDoc->GetNodes().MakeTxtNode( rPos, pColl );
/*N*/       rPos--;
/*N*/       (String&) pNd->GetTxt() = aText;
/*N*/   }
/*N*/   else
/*N*/   {
/*N*/       if( !nInsFirstPara )
/*N*/           pNd->ChgFmtColl( pColl );
/*N*/       SwIndex aOff( pNd, nOffset );
/*N*/       pNd->Insert( aText, aOff );
/*N*/   }
/*N*/
/*N*/   // Der Offset kann wegen Einf. von nicht sichtbaren Redlines auch
/*N*/   // negativ werden. Das darf aber auch ausser weiteren Redlines nichts
/*N*/   // mehr kommen.
/*N*/   INT32 nOffsetL = nOffset;
/*N*/
/*N*/   BOOL bConverted = FALSE;
/*N*/
/*N*/   while( BytesLeft() )
/*N*/   {
/*N*/       BYTE cType = Peek();
/*N*/       switch( cType )
/*N*/       {
/*N*/           case SWG_ATTRSET:
/*N*/               if( nInsFirstPara )
/*N*/               {
/*?*/                   SwAttrSet aTmpSet( pDoc->GetAttrPool(),
/*?*/                           RES_CHRATR_BEGIN, RES_CHRATR_END - 1 );
/*?*/                   InAttrSet( aTmpSet );
/*?*/                   if( aTmpSet.Count() )
/*?*/                   {
/*?*/                       OSL_ENSURE( nOffsetL>=0,
/*?*/                               "Offset darf hier nicht negativ sein" );
/*?*/                       if( 2 == nInsFirstPara )
/*?*/                           pNd->SetAttr( aTmpSet, 0, aText.Len() );
/*?*/                       else
/*?*/                           pNd->SetAttr( aTmpSet, (xub_StrLen)nOffsetL,
/*?*/                                         pNd->GetTxt().Len() );
/*?*/
/*?*/                       if( pNd->GetpSwAttrSet() )
/*?*/                           pNd->GetpSwAttrSet()->SetModifyAtAttr( pNd );
/*N*/                   }
/*N*/               }
/*N*/               else if( pOldNd )
/*N*/               {
/*?*/                   SwAttrSet aTmpSet( pDoc->GetAttrPool(), aTxtNodeSetRange );
/*?*/                   InAttrSet( aTmpSet );
/*?*/                   if( aTmpSet.Count() )
/*?*/                   {
/*?*/                       pNd->SwCntntNode::SetAttr( aTmpSet );
/*?*/
/*?*/                       if( pNd->GetpSwAttrSet() )
/*?*/                           pNd->GetpSwAttrSet()->SetModifyAtAttr( pNd );
/*?*/                   }
/*N*/               }
/*N*/               else
/*N*/               {
/*N*/                   if( !pNd->GetpSwAttrSet() )
/*N*/                       ((SwCntntNode*) pNd)->NewAttrSet( pDoc->GetAttrPool() );
/*N*/                   InAttrSet( *pNd->GetpSwAttrSet() );
/*N*/                   pNd->GetpSwAttrSet()->SetModifyAtAttr( pNd );
/*N*/               }
/*N*/               break;
/*N*/           case SWG_SDRFMT:
/*N*/               // Keine Draw-Formate in Kopf oder Fusszeilen einfuegen oder
/*N*/               // wenn kein Drawing-Layer da ist!
/*N*/               if( (nGblFlags & SW3F_NODRAWING) || bInsIntoHdrFtr )
/*N*/               {
/*?*/                   SkipRec(); break;
/*N*/               } // sonst weiter:
/*N*/           case SWG_FLYFMT:
/*N*/           {
/*N*/               // Absatzgebundener oder Rahmengebundener FlyFrame
/*N*/               USHORT eSave_StartNodeType = eStartNodeType;
/*N*/               eStartNodeType = SwFlyStartNode;
/*N*/               SwFrmFmt* pFmt = (SwFrmFmt*) InFormat( cType, NULL );
/*N*/               eStartNodeType = eSave_StartNodeType;
/*N*/
/*N*/               if( !pFmt )
/*N*/                   break;
/*N*/
/*N*/               // Anker darin versenken
/*N*/               SwFmtAnchor aAnchor( pFmt->GetAnchor() );
/*N*/               if( FLY_AT_CNTNT==aAnchor.GetAnchorId() ||
/*N*/                   FLY_IN_CNTNT==aAnchor.GetAnchorId() )
/*N*/               {
/*N*/                   // Absatzgebunende Rahmen: Die Abfrage auf FLY_IN_CNTNT
/*N*/                   // ist drinne, weil der SW31-Export sowas dummerweise
/*N*/                   // mal exportiert hat...
/*N*/                   aAnchor.SetType( FLY_AT_CNTNT );
/*N*/                   SwPosition aPos( rPos );
/*N*/                   aAnchor.SetAnchor( &aPos );
/*N*/               }
/*N*/               else
/*N*/               {
/*N*/                   // Dies sollte bisher nur ein rahmengebundener Rahmen
/*N*/                   // sein, koennte aber auch mal was anderes werden. Es
/*N*/                   // bleibt dann auf jeden Fall auch di Cntnt-Position
/*N*/                   // erhalten.
/*?*/                   SwPosition aPos( rPos, SwIndex(pNd,aAnchor.GetPageNum()) );
/*?*/                   aAnchor.SetAnchor( &aPos );
/*N*/               }
/*N*/               aAnchor.SetPageNum( 0 );
/*N*/               pFmt->SetAttr( aAnchor );
/*N*/               // Layout-Frames im Insert Mode fuer absatzgebundene
/*N*/               // Flys erzeugen
/*N*/               if( bInsert && !nRes ) pFmt->MakeFrms();
/*N*/               break;
/*N*/           }
/*N*/           case SWG_ATTRIBUTE:
/*N*/               OSL_ENSURE( nOffsetL>=0, "Offset darf hier nicht negativ sein" );
/*N*/               InTxtAttr( *pNd, aText8, (xub_StrLen)nOffsetL, &pINetFldTexts,
/*N*/                          &pINetFldPoss, &pErasePoss,
/*N*/                          &pCharSetColorEncs, &pCharSetColorPoss );
/*N*/               break;
/*N*/           case SWG_NUMRULE:
/*N*/               // NumRules gibt es an dieser Stelle nur im 3.1 und 4.0
/*N*/               // Fileformat. Seit dem 5.0-Filformat werden sie in einem
/*N*/               // eigenen Stream bzw. am Dok-Anfang gespeichert.
/*N*/               OpenNumRange40( rPos );
/*N*/               break;
/*N*/           case SWG_NODENUM:
/*N*/               // Den NodeNum-Record gibt es seit der 5.0
/*N*/               {
/*N*/                   SwNodeNum aNodeNum;
/*N*/                   InNodeNum( aNodeNum );
/*N*/                   pNd->UpdateNum( aNodeNum );
/*N*/               }
/*N*/               break;
/*N*/           case SWG_MARK:
/*N*/               OSL_ENSURE( nOffsetL>=0, "Offset darf hier nicht negativ sein" );
/*N*/               InNodeMark( rPos, (xub_StrLen)nOffsetL );
/*N*/               break;
/*N*/
/*N*/           case SWG_NODEREDLINE:
/*N*/               // nOffsetL ist Referenz-Parameter.
/*N*/               // nOffsetL kann jetzt negativ werden!
/*N*/               // The text has to be converted before any redlines are
/*N*/               // inserted. Otherwise, the content positions will not match
/*N*/               // the indices within the 8-Bit-Text.
/*N*/ /*?*/                 if( !bConverted )   // SW50.SDW
/*N*/ /*?*/                 {
/*N*/ /*?*/                     ConvertText( *pNd, aText8, (xub_StrLen)nOffsetL,
/*N*/ /*?*/                                  pCharSetColorEncs, pCharSetColorPoss );
/*N*/ /*?*/                     bConverted = TRUE;
/*N*/ /*?*/                 }
/*N*/ /*?*/                 InNodeRedline( rPos, nOffsetL );
/*?*/               break;
/*N*/
/*N*/           case SWG_WRONGLIST:
/*N*/           {
/*N*/               if( IsVersion( SWG_DESKTOP40 ) )
/*N*/               {
/*N*/                   OpenRec( SWG_WRONGLIST );
/*N*/                   pWrong = new SwWrongList;
/*N*/                   UINT16 nBeginInv, nEndInv, nCount;
/*N*/                   OpenFlagRec();
/*N*/                   *pStrm >> nBeginInv >> nEndInv;
/*N*/                   CloseFlagRec();
/*N*/                   pWrong->SetInvalid( (xub_StrLen)nBeginInv,
/*N*/                                       (xub_StrLen)nEndInv );
/*N*/                   *pStrm >> nCount;
/*N*/                   for( USHORT i=0; Good() && i<nCount; i++ )
/*N*/                   {
/*N*/                       UINT32 nWrong;
/*N*/                       *pStrm >> nWrong;
/*N*/                       xub_StrLen nPos = (xub_StrLen)nWrong;
/*N*/                       xub_StrLen nLen = (xub_StrLen)(0xFFFF & (nWrong >> 16));
/*N*/                       pWrong->Insert( nPos, nLen, pWrong->Count() );
/*N*/                   }
/*N*/                   if( bSpellAllAgain )
/*N*/                       pWrong->SetInvalid( 0, STRING_MAXLEN );
/*N*/                   if( bSpellWrongAgain )
/*N*/                       pWrong->InvalidateWrong();
/*N*/                   CloseRec( SWG_WRONGLIST );
/*N*/               }
/*N*/               else
/*N*/                   SkipRec();
/*N*/           }
/*N*/               break;
/*N*/           default:
/*?*/               SkipRec();
/*N*/       }
/*N*/   }
/*N*/   CloseRec( SWG_TEXTNODE );
/*N*/
/*N*/   // Eventuell den Text konvertieren
/*N*/   if( !bConverted )
/*N*/       ConvertText( *pNd, aText8, (xub_StrLen)nOffsetL,
/*N*/                    pCharSetColorEncs, pCharSetColorPoss );
/*N*/
/*N*/   // Numerierung uebernehmen
/*N*/   if( !IsVersion(SWG_LONGIDX) )
/*N*/   {
/*N*/       if( cNumLevel != NO_NUMBERING )
/*N*/       {
/*N*/           // MAXLEVEL war im 3.1/4.0-SW 5 und kann sich nichr mehr aendern,
/*N*/           // deshalb baruchen wir es nicht zu beachten.
/*N*/           pNd->UpdateNum( SwNodeNum( cNumLevel ) );
/*N*/       }
/*N*/       else
/*N*/           CloseNumRange40( rPos );
/*N*/   }
/*N*/
/*N*/   const SwNodeNum *pNdNum = pNd->GetNum();
/*N*/   const SwAttrSet *pAttrSet = pNd->GetpSwAttrSet();
/*N*/   if( pNdNum && IsVersion(SWG_LONGIDX) )
/*N*/   {
/*N*/       if( pAttrSet )
/*N*/       {
/*N*/           // Wenn der Absatz numeriert ist, muss die zugehoerige Numerierung
/*N*/           // noch als benutzt markiert werden bzw. eine automatische
/*N*/           // Numerierung beim Einfuegen umbenannt werden. Da automatische
/*N*/           // Numerierungen nicht in Vorlagen vorkommen koennen, gehen
/*N*/           // wir hier ueber das Attribut im Node-AttrSet und damit direkt
/*N*/           // an unser Namens-Array.
/*N*/           const SfxPoolItem *pItem;
/*N*/           if( SFX_ITEM_SET == pAttrSet->GetItemState( RES_PARATR_NUMRULE,
/*N*/                                                       FALSE, &pItem )  )
/*N*/           {
/*N*/               const String& rName = ((const SwNumRuleItem*)pItem)->GetValue();
/*N*/               if( rName.Len() )
/*N*/               {
/*N*/                   Sw3NumRuleInfo aTmp( rName );
/*N*/                   USHORT nPos;
/*N*/                   if( aNumRuleInfos.Seek_Entry( &aTmp, &nPos ) )
/*N*/                   {
/*N*/                       Sw3NumRuleInfo *pInfo = aNumRuleInfos[nPos];
/*N*/                       if( !bNormal || bInsert )
/*N*/                       {
/*N*/                           // Beim Einfuegen oder Laden von Seitenvorlagen
/*N*/                           // muss das Item evtl. noch an den geaenderten Namen
/*N*/                           // der Seiten-Vorlage angepasst werden.
/*?*/                           pInfo->SetUsed();
/*?*/                           if( rName != pInfo->GetNewName() )
/*?*/                           {
/*?*/                               ((SwCntntNode *)pNd)
/*?*/                                   ->SetAttr( SwNumRuleItem(pInfo->GetNewName()) );
/*?*/                           }
/*N*/                       }
/*N*/                       else
/*N*/                       {
/*N*/                           // Die entsprechende NumRule wird benutzt und braucht
/*N*/                           // nicht mehr geloescht zu werden. Also raus aus
/*N*/                           // dem Array damit.
/*N*/                           aNumRuleInfos.Remove( nPos, 1 );
/*N*/                           delete pInfo;
/*N*/                       }
/*N*/                   }
/*N*/               }
/*N*/               else
/*N*/               {
/*?*/                   SwNodeNum aNodeNum( NO_NUMBERING );
/*?*/                   pNd->UpdateNum( aNodeNum );
/*?*/                   pNdNum = 0;
/*N*/               }
/*N*/           }
/*N*/       }
/*N*/
/*N*/       if( bPageDescs && !bNumRules )
/*N*/       {
/*N*/           // Wenn Seiten-Vorlagen aber keine Numerierungs-Vorlagen
/*N*/           // geladen werden, dann muessen wir sicherstellen, dass
/*N*/           // die Numerierungs-Vorlage auch existiert.
/*?*/           const SfxPoolItem* pItem =
/*?*/               pNd->GetNoCondAttr( RES_PARATR_NUMRULE, TRUE );
/*?*/           if( pItem && ((SwNumRuleItem*)pItem)->GetValue().Len() &&
/*?*/               !pDoc->FindNumRulePtr( ((SwNumRuleItem*)pItem)->GetValue() ) )
/*?*/           {
/*?*/               const String& rName = ((SwNumRuleItem*)pItem)->GetValue();
/*?*/               USHORT nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( rName, GET_POOLID_NUMRULE );
/*?*/               if( nPoolId != USHRT_MAX )
/*?*/                   pDoc->GetNumRuleFromPool( nPoolId );
/*?*/               else
/*?*/                   pDoc->MakeNumRule( rName );
/*?*/           }
/*N*/       }
/*N*/   }
/*N*/
/*N*/ #ifdef NUM_RELSPACE
/*N*/   // Wenn der Absatz ein LRSpace-Item enthaelt und in der Kapitel-Numerierung
/*N*/   // ist muss das LRSpace-Item noch angepasst werden. Relative Werte
/*N*/   // koennen dabei nicht vorkommen.
/*N*/   const SwNumRule *pOutline = pDoc->GetOutlineNumRule();
/*N*/   const SfxPoolItem *pItem;
/*N*/   if( pAttrSet && (!pNdNum || NO_NUMBERING != pNdNum->GetLevel()) &&
/*N*/       NO_NUMBERING != pColl->GetOutlineLevel() &&
/*N*/       pOutline && nVersion != SWG_NUMRELSPACE )
/*N*/   {
/*N*/       const SwNumFmt& rNumFmt = pOutline->Get(
/*N*/               GetRealLevel(((const SwTxtFmtColl*)pColl)->GetOutlineLevel()) );
/*N*/       USHORT nNumLSpace = rNumFmt.GetAbsLSpace();
/*N*/
/*N*/       if( SFX_ITEM_SET == pAttrSet->GetItemState( RES_LR_SPACE, FALSE,
/*N*/                                                   &pItem ) )
/*N*/       {
/*N*/           const SvxLRSpaceItem *pParaLRSpace =
/*N*/               (const SvxLRSpaceItem *)pItem;
/*N*/
/*N*/           USHORT nWishLSpace = (USHORT)pParaLRSpace->GetTxtLeft();
/*N*/           USHORT nNewLSpace =
/*N*/                       nWishLSpace > nNumLSpace ? nWishLSpace-nNumLSpace : 0U;
/*N*/
/*N*/           const SvxLRSpaceItem& rCollLRSpace = pColl->GetLRSpace();
/*N*/           if( nNewLSpace == rCollLRSpace.GetTxtLeft() &&
/*N*/               pParaLRSpace->GetRight() == rCollLRSpace.GetRight() &&
/*N*/               pParaLRSpace->GetTxtFirstLineOfst() ==
/*N*/                                           rCollLRSpace.GetTxtFirstLineOfst() )
/*N*/           {
/*N*/               pNd->ResetAttr( RES_LR_SPACE );
/*N*/           }
/*N*/           else if( nNewLSpace != pParaLRSpace->GetTxtLeft() )
/*N*/           {
/*?*/               SvxLRSpaceItem aLRSpace( *pParaLRSpace );
/*?*/               short nFirst = aLRSpace.GetTxtFirstLineOfst();
/*?*/               if( nFirst < 0 && (USHORT)-nFirst > nNewLSpace )
/*?*/                   aLRSpace.SetTxtFirstLineOfst( -(short)nNewLSpace );
/*?*/               aLRSpace.SetTxtLeft( nNewLSpace );
/*?*/               ((SwCntntNode *)pNd)->SetAttr( aLRSpace );
/*N*/           }
/*N*/
/*N*/           if( !IsVersion(SWG_NUMRELSPACE) && nWishLSpace != nNewLSpace )
/*N*/               lcl_sw3io__ConvertNumTabStop( *pNd, (long)nWishLSpace -
/*N*/                                                   (long)nNewLSpace, FALSE );
/*N*/       }
/*N*/       else if( nNumLSpace > 0 && !IsVersion(SWG_NUMRELSPACE) )
/*N*/       {
/*N*/           lcl_sw3io__ConvertNumTabStop( *pNd, nNumLSpace, FALSE   );
/*N*/       }
/*N*/   }
/*N*/ #endif
/*N*/
/*N*/
/*N*/   if( pINetFldTexts )
/*N*/   {
/*N*/       OSL_ENSURE( pINetFldPoss, "INet-Feld-Texte ohne Positionen???" );
/*N*/
/*N*/       // Es mussen noch Texte von Internet-Feldern eingefuegt werden
/*N*/
/*N*/       INT32 nOffset2 = 0; // Verschiebung durch die Felder selbst
/*N*/
/*N*/       for( USHORT i=0; i<pINetFldTexts->Count(); i++ )
/*N*/       {
/*N*/           const String &rStr = *(*pINetFldTexts)[i];
/*N*/
/*N*/           // den Text hinter dem 0xff vom Feld einfuegen
/*N*/           xub_StrLen nPos = xub_StrLen( nOffset2 + nOffset +
/*N*/                                         (*pINetFldPoss)[i] + 1 );
/*N*/           SwIndex aOff( pNd, nPos );
/*N*/
/*N*/           if( rStr.Len() )
/*N*/           {
/*N*/               pNd->Insert( rStr, aOff );
/*N*/           }
/*N*/
/*N*/           // und das 0xff loeschen
/*N*/           aOff.Assign( pNd, nPos-1 );
/*N*/           pNd->Erase( aOff, 1 );
/*N*/
/*N*/           // und den Offset korrigieren
/*N*/           nOffset2 += rStr.Len();
/*N*/           nOffset2--;
/*N*/       }
/*N*/
/*N*/       // die Wrong-Liste ist jetzt ungueltig
/*N*/       delete pWrong;
/*N*/       pWrong = 0;
/*N*/       cFlags &= 0xdf;
/*N*/
/*N*/       // und die Array loeschen
/*N*/       delete pINetFldTexts;
/*N*/       delete pINetFldPoss;
/*N*/   }
/*N*/
/*N*/   if( pErasePoss )
/*N*/   {
/*N*/       // Es mussen noch 0xff-Zeichen aus dem Node geloescht werden
/*?*/       USHORT i = pErasePoss->Count();
/*?*/       while( i )
/*?*/       {
/*?*/           xub_StrLen nPos = (*pErasePoss)[--i];
/*?*/
/*?*/           OSL_ENSURE( CH_TXTATR_BREAKWORD == pNd->GetTxt().GetChar( nPos ) ||
/*?*/                   CH_TXTATR_INWORD == pNd->GetTxt().GetChar( nPos ),
/*?*/                   "Es sollten nur 0xff geloescht werden" );
/*?*/
/*?*/           SwIndex aOff( pNd, nPos );
/*?*/           pNd->Erase( aOff, 1 );
/*?*/       }
/*?*/
/*?*/
/*?*/       // die Wrong-Liste ist jetzt ungueltig
/*?*/       delete pWrong;
/*?*/       pWrong = 0;
/*?*/       cFlags &= 0xdf;
/*?*/
/*?*/       delete pErasePoss;
/*N*/   }
/*N*/
/*N*/   // Wrong-Liste uebernehmen
/*N*/   // ACHTUNG: dirty-bit wird invers gespeichert weil in alten Doks 0 steht
/*N*/   BOOL bWrongDirty = ( ( cFlags & 0x20 ) == 0 ) ||
/*N*/                       nVersion < SWG_DESKTOP40 ||
/*N*/                       bSpellAllAgain || bSpellWrongAgain;
/*N*/   pNd->SetWrongDirty( bWrongDirty );
/*N*/   pNd->SetWrong( pWrong );
/*N*/
/*N*/   // Condition-Collections setzen:
/*N*/   if( IDX_DFLT_VALUE != nCondColl )
/*N*/   {
/*?*/       if( bInsert )
/*?*/       {
/*?*/           // dann muss die richtige Collection neu bestimmt werden!
/*?*/           pNd->ChkCondColl();
/*?*/       }
/*?*/       else
/*?*/       {
/*?*/           SwTxtFmtColl* pCColl = FindTxtColl( nCondColl );
/*?*/           pNd->SetCondFmtColl( pCColl );
/*?*/       }
/*N*/   }
/*N*/
/*N*/ #ifdef NUM_RELSPACE
/*N*/   if( pNdNum && NO_NUMBERING != pNdNum->GetLevel() &&
/*N*/       IsVersion(SWG_LONGIDX) )
/*N*/   {
/*N*/       // In Dokumenten, in denen der Einzug einer Numerierung noch
/*N*/       // absolut war, muss der Absatz-Einzug noch angepasst werden.
/*N*/       // Weil man dazu die bedingte Vorlage braucht, darf das erst
/*N*/       // hier geschehen.
/*N*/       const SwNumRule *pNumRule = pNd->GetNumRule();
/*N*/       if( pNumRule )
/*N*/           lcl_sw3io__ConvertNumLRSpace( *pNd, *pNumRule,
/*N*/                                         pNdNum->GetLevel(),
/*N*/                                         !IsVersion(SWG_NUMRELSPACE) );
/*N*/   }
/*N*/ #endif
/*N*/
/*N*/   rPos++;
/*N*/ }

/*N*/ SwInsHardBlankSoftHyph::~SwInsHardBlankSoftHyph()
/*N*/ {
/*N*/   for( USHORT n = 0, nCnt = aItems.Count(); n < nCnt; ++n )
/*N*/       delete aItems[ n ];
/*N*/ }
/*N*/ void SwInsHardBlankSoftHyph::AddItem( xub_StrLen nPos, sal_Unicode c )
/*N*/ {
/*N*/   SfxPoolItem* pItem = 0;
/*N*/   switch ( c )
/*N*/   {
/*N*/   case CHAR_HARDBLANK:    pItem = new SwFmtHardBlank( ' ', FALSE ); break;
/*?*/   case CHAR_HARDHYPHEN:   pItem = new SwFmtHardBlank( '-', FALSE ); break;
/*N*/   case CHAR_SOFTHYPHEN:   pItem = new SwFmtSoftHyph; break;
/*N*/   }
/*N*/   if( pItem )
/*N*/   {
/*N*/       USHORT nInsPos = aItemStarts.Count();
/*N*/       aItemStarts.Insert( nPos, nInsPos );
/*N*/       aItems.C40_INSERT( SfxPoolItem, pItem, nInsPos );
/*N*/   }
/*N*/ }

// nOffset ist ungleich Null, wenn innerhalb eines Nodes eingefuegt werden
// soll. Dann ist nOffset die Start-Position des Textes.

/*N*/ void Sw3IoImp::InTxtAttr( SwTxtNode& rNd, const ByteString& rText8,
/*N*/                         xub_StrLen nOffset,
/*N*/                         SvStringsDtor **pINetFldTexts,
/*N*/                         SvXub_StrLens **pINetFldPoss,
/*N*/                         SvXub_StrLens **pErasePoss,
/*N*/                         SvUShorts     **pCharSetColorEncs,
/*N*/                         SvXub_StrLens **pCharSetColorPoss )
/*N*/ {
/*N*/   // Dieser Record kann auch leer sein
/*N*/   // (bei teilweisem Speichern eines Nodes, z.B.)
/*N*/   xub_StrLen nLen = rNd.GetTxt().Len();
/*N*/   if( nLen ) nLen --;
/*N*/   xub_StrLen nStart, nEnd;
/*N*/   SfxPoolItem* pItem = InAttr( nStart, nEnd, &rNd );
/*N*/   if( !pItem )
/*N*/   {
/*N*/       if( bDrawFmtSkipped )
/*N*/       {
/*N*/           OSL_ENSURE( bInsIntoHdrFtr,
/*?*/                   "Draw-Formate durften nur in Kopf-/Fusszeilen geloecht werden" );
/*?*/           OSL_ENSURE( CH_TXTATR_BREAKWORD == rNd.GetTxt().GetChar(nStart) ||
/*?*/                   CH_TXTATR_INWORD == rNd.GetTxt().GetChar(nStart),
/*?*/                   "Wo ist das 0xff des Draw-Formats?" );
/*?*/
/*?*/           if( !(*pErasePoss) )
/*?*/               *pErasePoss = new SvXub_StrLens;
/*?*/           (*pErasePoss)->Insert( nStart, (*pErasePoss)->Count() );
/*?*/
/*?*/           bDrawFmtSkipped = FALSE;
/*?*/           return;
/*N*/       }
/*N*/
/*N*/       if( !pFmtINetFmt )
/*N*/           return;
/*N*/
/*N*/       // Es wurde ein INetFlield gelesen und in einen INet-Attribut
/*N*/       // umgewandelt
/*N*/
/*N*/       // Text und Position merken
/*N*/       if( !(*pINetFldTexts) )
/*N*/           *pINetFldTexts = new SvStringsDtor;
/*N*/       (*pINetFldTexts)->Insert( new String( aINetFldText ),
/*N*/                                       (*pINetFldTexts)->Count() );
/*N*/       if( !(*pINetFldPoss) )
/*N*/           *pINetFldPoss = new SvXub_StrLens;
/*N*/       (*pINetFldPoss)->Insert( nStart, (*pINetFldPoss)->Count() );
/*N*/
/*N*/       if( aINetFldText.Len() )
/*N*/       {
/*N*/           // ggf. das Attribut ueber dem =xff aufspannen
/*N*/           // das Item wird ann unten geloescht
/*N*/           pItem = pFmtINetFmt;
/*N*/           nEnd++;
/*N*/           aINetFldText.Erase();
/*N*/       }
/*N*/       else
/*N*/       {
/*N*/           // leere Felder nicht einfuegen
/*N*/           delete pFmtINetFmt;
/*N*/       }
/*N*/
/*N*/       pFmtINetFmt = 0;
/*N*/
/*N*/       if( !pItem )
/*N*/           return;
/*N*/   }
/*N*/
/*N*/   if( nEnd < nStart ) nEnd = nLen;
/*N*/   nStart += nOffset;
/*N*/   nEnd   += nOffset;
/*N*/   USHORT nWhich = pItem->Which();
/*N*/   if( nWhich == RES_TXTATR_FTN )
/*N*/   {
/*N*/       //JP 02.12.96:
/*N*/       // Das Fussnoten-Attribut liest seine Section "auf der Wiese" ein
/*N*/       // und erzeugt auch ihr TextAttribut (weil an dem noch weitere
/*N*/       // Informationen gesetzt werden muessen - Referenznummer)
/*N*/       SwTxtFtn& rFtn = *((SwFmtFtn*)pItem)->GetTxtFtn();
/*N*/       *rFtn.GetStart() = nStart;
/*N*/       rNd.Insert( &rFtn, SETATTR_NOTXTATRCHR );
/*N*/       return ;
/*N*/   }
/*N*/   else if( RES_CHRATR_CHARSETCOLOR == nWhich )
/*N*/   {
/*?*/       if( !(*pCharSetColorEncs) )
/*?*/           *pCharSetColorEncs = new SvUShorts;
/*?*/       (*pCharSetColorEncs)->Insert(
/*?*/           ((const SvxCharSetColorItem *)pItem)->GetCharSet(),
/*?*/           (*pCharSetColorEncs)->Count() );
/*?*/
/*?*/       if( !(*pCharSetColorPoss) )
/*?*/           *pCharSetColorPoss = new SvXub_StrLens;
/*?*/       (*pCharSetColorPoss)->Insert( nStart, (*pCharSetColorPoss)->Count() );
/*?*/       (*pCharSetColorPoss)->Insert( nEnd, (*pCharSetColorPoss)->Count() );
/*N*/   }
/*N*/   else
/*N*/   {
/*N*/       // Bug 31560: mehrere TOX-Marks ohne Ende an der gleichen Position!
/*N*/       if( nStart == nEnd && (( RES_TXTATR_TOXMARK == nWhich &&
/*N*/             ((SwTOXMark*)pItem)->IsAlternativeText() ) ||
/*N*/           ( RES_TXTATR_NOEND_BEGIN <= nWhich && nWhich < RES_TXTATR_NOEND_END )))
/*N*/       {
/*N*/           // teste doch mal ob das Zeichen am der Position steht und on
/*N*/           // an der Position nicht schon ein Attribut ohne Ende gesetzt ist!
/*N*/           if( '\xff' != rText8.GetChar(nStart-nOffset) )
/*N*/           {
/*N*/               nWhich = 0;
/*N*/               OSL_ENSURE( !this, "TextAttribut ohne Ende ohne 0xFF" );
/*N*/           }
/*N*/           else
/*N*/           {
/*N*/               sal_Unicode cReplace = 0;
/*N*/               switch( nWhich )
/*N*/               {
/*N*/               case RES_TXTATR_TOXMARK:
/*N*/                   {
/*N*/                       // pruefe mal auf doppelte:
/*N*/                       SwTxtAttr* pAttr = rNd.GetTxtAttr( nStart, nWhich );
/*N*/                       if( pAttr )
/*N*/                       {
/*N*/                           nWhich = 0;
/*N*/                           OSL_ENSURE( !this, "TOXMark ohne Ende doppelt" );
/*N*/                       }
/*N*/                   }
/*N*/                   break;
/*N*/               case RES_TXTATR_SOFTHYPH:
/*N*/                   // set the unicode character into the node text
/*N*/                   cReplace = CHAR_SOFTHYPHEN;
/*N*/                   break;
/*N*/
/*N*/               case RES_TXTATR_HARDBLANK:
/*N*/                   cReplace = ((SwFmtHardBlank*)pItem)->GetChar();
/*N*/                   if( ' ' == cReplace )
/*N*/                       cReplace = CHAR_HARDBLANK;
/*N*/                   else if( '-' == cReplace )
/*N*/                       cReplace = CHAR_HARDHYPHEN;
/*N*/                   else
/*N*/                       cReplace = 0;
/*N*/                   break;
/*N*/               }
/*N*/               if( cReplace )
/*N*/               {
/*N*/                   rNd.Replace( SwIndex( &rNd, nStart ), cReplace );
/*N*/                   nWhich = 0;
/*N*/               }
/*N*/           }
/*N*/       }
/*N*/
/*N*/       if( nWhich )
/*N*/       {
/*N*/           SwTxtAttr* pAttr = rNd.Insert( *pItem, nStart, nEnd, SETATTR_NOTXTATRCHR );
/*N*/           // Sonderbehandlung fuer einige Text-Attribute:
/*N*/           if( pAttr && RES_TXTATR_FLYCNT == nWhich )
/*N*/               // ein zeichengebundener FlyFrm muss noch verankert werden
/*N*/               ((SwTxtFlyCnt*) pAttr)->SetAnchor( &rNd );
/*N*/       }
/*N*/   }
/*N*/   delete pItem;
/*N*/ }

/*N*/ void Sw3IoImp::InGrfNode( SwNodeIndex& rPos )
/*N*/ {
/*N*/   Graphic aGrf;
/*N*/   Graphic* pGrf = &aGrf;
/*N*/   String aGrfName, aFltName, aStrmName, aURL, aTarget, aAltText;
/*N*/   ImageMap *pImgMap = 0;
/*N*/   PolyPolygon *pContour = 0;
/*N*/   OpenRec( SWG_GRFNODE );
/*N*/   BYTE cFlags = OpenFlagRec();
/*N*/   CloseFlagRec();
/*N*/   BOOL bLink = BOOL( ( cFlags & 0x10 ) == 0 );
/*N*/   BOOL bEmptyGrf = BOOL( cFlags & 0x20 );
/*N*/   BOOL bIsServerMap = BOOL( (cFlags & 0x40) != 0 );
/*N*/
/*N*/   InString( *pStrm, aGrfName );
/*N*/   InString( *pStrm, aFltName );
/*N*/
/*N*/   if( IsVersion( SWG_DESKTOP40 ) )
/*N*/       InString( *pStrm, aAltText );
/*N*/
/*N*/   aStrmName = aGrfName;
/*N*/   SwAttrSet aSet( pDoc->GetAttrPool(), aNoTxtNodeSetRange );
/*N*/   while( BytesLeft() )
/*N*/   {
/*N*/       BYTE cType = Peek();
/*N*/       switch( cType )
/*N*/       {
/*N*/           case SWG_ATTRSET:
/*N*/               InAttrSet( aSet );
/*N*/               break;
/*N*/           case SWG_IMAGEMAP:
/*N*/               {
/*N*/                   BOOL bDummy; // IsURL-Map-Flag wird im Node selbst gesp.
/*N*/                   pImgMap = InImageMap( aURL, aTarget, bDummy );
/*N*/               }
/*N*/               break;
/*N*/
/*N*/           case SWG_CONTOUR:
                    pContour = InContour();
/*N*/               break;
/*N*/
/*N*/           default:
/*?*/               SkipRec();
/*N*/       }
/*N*/   }
/*N*/   CloseRec( SWG_GRFNODE );
/*N*/   if( CheckPersist() )
/*N*/   {
/*N*/       // Muss die Grafik noch geladen werden?
/*N*/       if( !bEmptyGrf )
/*N*/       {
/*N*/           if( bLink )
/*N*/           {
/*N*/               pGrf = 0;
/*N*/               if( aGrfName.Len() )
/*N*/           aGrfName = ::binfilter::StaticBaseUrl::RelToAbs( aGrfName );
/*N*/           }
/*N*/           else
/*N*/           {
/*N*/               SvStorageRef pPicStg = pRoot->OpenStorage( N_PICTURES,
/*N*/                           STREAM_READ | STREAM_SHARE_DENYWRITE, 0 );
/*N*/               SvStorageStreamRef pPicStrm;
/*N*/               if( pPicStg.Is() )
/*N*/                   pPicStrm = pPicStg->OpenStream
/*N*/                       ( aGrfName, STREAM_READ | STREAM_SHARE_DENYWRITE );
/*N*/               aGrfName.Erase();   // Ist ja gar kein Grafikname!
/*N*/               OSL_ENSURE( pPicStrm.Is() && pPicStrm->GetError() == SVSTREAM_OK, "Grafik nicht gefunden" );
/*N*/               if( pPicStrm.Is() && pPicStrm->GetError() == SVSTREAM_OK )
/*N*/               {
/*N*/                   // Wenn kein DocFileName gesetzt ist, wird eine TmpFile
/*N*/                   // erzeugt, was wir im Moment nicht wollen!
/*N*/                   pPicStrm->SetVersion( pRoot->GetVersion() );
/*N*/                   String aDummy( String::CreateFromAscii("file:///Dummy") );
/*N*/                   aGrf.SetDocFileName( aDummy, 0L );
/*N*/                   BOOL bSwapOut = BOOL( !bInsert );
/*N*/                   // Beim SW3.1-Import verliern wir unserden Dok-Storage,
/*N*/                   // also darf die Grafik nicht rausgeswappt werden.
/*N*/                   // MIB 3.9.98: Ausserdem muessen Grafiken immer beim
/*N*/                   // LoadStyles reingeswappt werden, weil dann der Storage
/*N*/                   // aus dem geladen wird nicht unbedingt der Doc-Storage
/*N*/                   // sein muss. Das ist zum Beispiel beim Aktualisieren von
/*N*/                   // Vorlagen aus einer Dokument-Vorlage der Fall (#55896#)
/*N*/                   // Aufgrund eines Hackks im Organizer stimmt dort der
/*N*/                   // Dok-Storage uebrigens ...
/*N*/                   if( bBlock || bOrganizer || nVersion < SWG_MAJORVERSION )
/*N*/                       bSwapOut = FALSE;
/*N*/                   aGrf.ReadEmbedded( *pPicStrm, bSwapOut );
/*N*/                   aDummy.Erase();
/*N*/                   aGrf.SetDocFileName( aDummy, 0L );
/*N*/                   if( pPicStrm->GetError() != SVSTREAM_OK )
/*N*/                       Error( ERR_SWG_READ_ERROR );
/*N*/               }
/*N*/               else
/*N*/                   Warning( WARN_SWG_POOR_LOAD );
/*N*/           }
/*N*/       }
/*N*/
/*N*/       if( !nRes )
/*N*/       {
/*N*/           if( !IsVersion( SWG_URLANDMAP, SWG_EXPORT31, SWG_DESKTOP40 ) )
/*N*/           {
/*N*/               // bei importierten Dateien muss ggf noch die URL in das
/*N*/               // Format gestopft werden
/*N*/               SwFmtURL aFmtURL;
/*N*/               aFmtURL.SetURL( aURL, bIsServerMap );
/*N*/               aFmtURL.SetTargetFrameName( aTarget );
/*N*/               if( pImgMap )
/*?*/                   aFmtURL.SetMap( pImgMap );
/*N*/               aSet.Put( aFmtURL );
/*N*/           }
/*N*/           SwGrfNode* pNd = pDoc->GetNodes().MakeGrfNode( rPos,
/*N*/                                     aGrfName, aFltName, pGrf,
/*N*/                                     (SwGrfFmtColl*) pDoc->GetDfltGrfFmtColl(),
/*N*/                                     &aSet, bLink );
/*N*/           if( !bLink && !bBlock && !bInsert && !bOrganizer )
/*N*/               pNd->SetStreamName( aStrmName );
/*N*/           pNd->SetAlternateText( aAltText );
/*N*/           pNd->SetContour( pContour );
/*N*/       }
/*N*/   }
/*N*/   delete pImgMap;
/*N*/   delete pContour;
/*N*/ }

// Einlesen eines OLE-Nodes

/*N*/ void Sw3IoImp::InOLENode( SwNodeIndex& rPos )
/*N*/ {
/*N*/   SwOLENode* pOLENd = 0;
/*N*/   SwNoTxtNode* pNoTxtNd = 0;
/*N*/   String aObjName, aAltText;
/*N*/
/*N*/   OpenRec( SWG_OLENODE );
/*N*/   InString( *pStrm, aObjName );
/*N*/
/*N*/   if( IsVersion( SWG_DESKTOP40 ) )
/*N*/       InString( *pStrm, aAltText );
/*N*/
/*N*/   // change the StarImageObj to a graphic
/*N*/   SvPersistRef xSrcDoc( new SvPersist() );
/*N*/   SvInfoObjectRef xObjInfo;
/*N*/   if( xSrcDoc->DoOwnerLoad( pRoot ) && xSrcDoc->GetObjectList() )
/*N*/   {
/*N*/       // Suche die richtige Info
/*N*/       xObjInfo = xSrcDoc->Find( aObjName );
/*N*/       OSL_ENSURE( xObjInfo.Is(), "Keine Objektinfo zum Einfuegen gefunden" );
/*N*/   }
/*N*/
/*N*/   if( xObjInfo.Is() )
/*N*/   {
/*N*/       SvStorageRef xSimStg( pRoot->OpenStorage(
/*N*/                                           xObjInfo->GetStorageName() ) );
/*N*/       String aStmName;
/*N*/       if( xSimStg.Is() && (
/*N*/           xSimStg->IsStream( aStmName = String(
/*N*/                   RTL_CONSTASCII_USTRINGPARAM( "StarImageDocument" )) ) ||
/*N*/           xSimStg->IsStream( aStmName = String(
/*N*/                   RTL_CONSTASCII_USTRINGPARAM( "StarImageDocument 4.0" ))) ))
/*N*/       {
/*N*/           SvStorageStreamRef xSimStm( xSimStg->OpenStream( aStmName ) );
/*N*/
/*N*/           if( xSimStm.Is() && !xSimStm->GetError() )
/*N*/           {
/*N*/               Graphic aGraphic;
/*N*/               xSimStm->SetBufferSize( 32768 );
/*N*/               xSimStm->SetCryptMaskKey( xSimStg->GetKey() );
/*N*/               *xSimStm >> aGraphic;
/*N*/               xSimStm->SetBufferSize( 0 );
/*N*/
/*N*/               pNoTxtNd = pDoc->GetNodes().MakeGrfNode( rPos, aEmptyStr,
/*N*/                       aEmptyStr, &aGraphic,
/*N*/                       (SwGrfFmtColl*) pDoc->GetDfltGrfFmtColl() );
/*N*/           }
/*N*/       }
/*N*/   }
/*N*/
/*N*/   if( !pNoTxtNd && CheckPersist() )
/*N*/   {
/*N*/       // Im Insert Mode muss das OLE-Objekt in den Ziel-Storage kopiert werden
/*N*/       if( bInsert && xObjInfo.Is() )
/*N*/       {
/*?*/           SvPersistRef rpDstDoc( pDoc->GetPersist() );
/*?*/
/*?*/           String aStgName( xObjInfo->GetStorageName() );
/*?*/           SvStorageRef rpDst( pDoc->GetPersist()->GetStorage() );
/*?*/           // Sind Objektname und Storagename eindeutig?
/*?*/           if( rpDstDoc->GetObjectList() )
/*?*/               for( ULONG i = 0; i < rpDstDoc->GetObjectList()->Count(); i++ )
/*?*/               {
/*?*/                   SvInfoObject* pTst = rpDstDoc->GetObjectList()->GetObject(i);
/*?*/                   // TODO: unicode: is this correct?
/*?*/                   if( aObjName.EqualsIgnoreCaseAscii( pTst->GetObjName() ))
/*?*/                       aObjName = Sw3Io::UniqueName( rpDst, "Obj" );
/*?*/                   // TODO: unicode: is this correct?
/*?*/                   if( aStgName.EqualsIgnoreCaseAscii( pTst->GetStorageName() ) )
/*?*/                       aStgName = Sw3Io::UniqueName( rpDst, "Obj" );
/*?*/               }
/*?*/           if( !rpDstDoc->Copy( aObjName, aStgName, &xObjInfo, xSrcDoc ) )
/*?*/           {
/*?*/               Error( ERR_SWG_READ_ERROR );
/*?*/               return;
/*?*/           }
/*N*/       }
/*N*/       pNoTxtNd = pOLENd = pDoc->GetNodes().MakeOLENode( rPos, aObjName,
/*N*/                       (SwGrfFmtColl*) pDoc->GetDfltGrfFmtColl() );
/*N*/   }
/*N*/
/*N*/   if( pNoTxtNd )
/*N*/   {
/*N*/       pNoTxtNd->SetAlternateText( aAltText );
/*N*/
/*N*/       while( BytesLeft() )
/*N*/       {
/*N*/           BYTE cType = Peek();
/*N*/           switch( cType )
/*N*/           {
/*?*/           case SWG_ATTRSET:
/*?*/               if( !pNoTxtNd->GetpSwAttrSet() )
/*?*/                   ((SwCntntNode*) pNoTxtNd)->NewAttrSet( pDoc->GetAttrPool() );
/*?*/               InAttrSet( *pNoTxtNd->GetpSwAttrSet() );
/*?*/               pNoTxtNd->GetpSwAttrSet()->SetModifyAtAttr( pNoTxtNd );
/*?*/               break;
/*N*/
/*N*/           case SW_OLE_CHARTNAME:
/*N*/               if( pOLENd )
/*N*/               {
/*N*/                   String sStr;
/*N*/                   OpenRec( SW_OLE_CHARTNAME );
/*N*/                   InString( *pStrm, sStr );
/*N*/                   CloseRec( SW_OLE_CHARTNAME );
/*N*/                   pOLENd->SetChartTblName( sStr );
/*N*/               }
/*N*/               else
/*?*/                   SkipRec();
/*N*/               break;
/*N*/
/*?*/           case SWG_IMAGEMAP:
/*?*/               {
/*?*/                   String aURL, aTarget;
/*?*/                   BOOL bIsServerMap = FALSE;
/*?*/                   ImageMap *pImgMap = InImageMap( aURL, aTarget, bIsServerMap);
/*?*/                   SwFmtURL aFmtURL;
/*?*/                   aFmtURL.SetURL( aURL, bIsServerMap );
/*?*/                   aFmtURL.SetTargetFrameName( aTarget );
/*?*/                   if ( pImgMap )
/*?*/                   {
/*?*/                       aFmtURL.SetMap( pImgMap );
/*?*/                       delete pImgMap;
/*?*/                   }
/*?*/                   pNoTxtNd->SetAttr( aFmtURL );
/*?*/               }
/*?*/               break;
/*?*/
/*?*/           case SWG_CONTOUR:
/*?*/               if( pOLENd )
/*?*/               {
                        PolyPolygon *pContour = InContour();
 /*?*/                  pOLENd->SetContour( pContour );
 /*?*/                  delete pContour;
/*?*/               }
/*?*/               else
/*?*/                   SkipRec();
/*?*/               break;
/*N*/
/*N*/           default:
/*N*/               SkipRec();
/*N*/           }
/*N*/       }
/*N*/       // falls ein 3.0-Dokument gelesen wird: Node merken
/*N*/       if( pOLENd )
/*N*/       {
/*N*/           if( bNormal && !bInsert && !bBlock && nVersion<=SWG_SHORTFIELDS )
/*N*/           {
/*?*/               if( !p30OLENodes )
/*?*/                   p30OLENodes = new SwOLENodes;
/*?*/
/*?*/               p30OLENodes->Insert( pOLENd, p30OLENodes->Count() );
/*N*/           }
/*N*/
/*N*/           if( bInsert )
/*?*/               pOLENd->SetOLESizeInvalid( TRUE );  //wg. Druckerwechsel
/*N*/       }
/*N*/   }
/*N*/   if( !xObjInfo.Is() )
/*?*/       Warning( WARN_SWG_POOR_LOAD );
/*N*/
/*N*/   CloseRec( SWG_OLENODE );
/*N*/ }

// Einlesen eines Text-Wiederholungs-Nodes

/*N*/ void Sw3IoImp::InRepTxtNode( SwNodeIndex& rPos )
/*N*/ {
/*N*/   UINT32 nRepetitions;
/*N*/
/*N*/   OpenRec( SWG_REPTEXTNODE );
/*N*/   *pStrm >> nRepetitions;
/*N*/
/*N*/   rPos--;
/*N*/   SwTxtNode *pNode = pDoc->GetNodes()[rPos]->GetTxtNode();
/*N*/   rPos++;
/*N*/
/*N*/   for( ULONG i=0; i<nRepetitions; i++ )
/*N*/       pNode->MakeCopy( pDoc, rPos );
/*N*/
/*N*/   CloseRec( SWG_REPTEXTNODE );
/*N*/ }
/*N*/

// Der Image-Map-Record war frueher ein SWG_GRAPHIC_EXT-Record.
// Deshalb enthaelt er immer der URL fuer eine Server-seitige
// Image-Map und kein ismap-Flag! Aus dem gleichen Grund wird die
// URL fuer eine Client-seitige Image-Map ueber ein Falg gesteuert.
// damit alte Writer-Version keine Warnung ausgeben, wenn der String
// leer ist.

/*N*/ ImageMap *Sw3IoImp::InImageMap( String& rURL, String& rTarget, BOOL& rIsMap )
/*N*/ {
/*N*/   OpenRec( SWG_IMAGEMAP );
/*N*/   BYTE cFlags = OpenFlagRec();
/*N*/   CloseFlagRec();
/*N*/
/*N*/   rIsMap = BOOL( (cFlags & 0x10) != 0 );
/*N*/
/*N*/   InString( *pStrm, rURL );
/*N*/   if( rURL.Len() )
/*N*/       rURL = ::binfilter::StaticBaseUrl::SmartRelToAbs( rURL );
/*N*/
/*N*/   // bis hier hatten wir frueher einen SWG_GRAPHIC_EXT-Record!
/*N*/   if( IsVersion( SWG_TARGETFRAME, SWG_EXPORT31, SWG_DESKTOP40 ) )
/*N*/   {
/*N*/       String sDummy;
/*N*/       InString( *pStrm, rTarget );
/*N*/       InString( *pStrm, sDummy );
/*N*/   }
/*N*/
/*N*/   ImageMap *pIMap = 0;
/*N*/   if( cFlags & 0x20 )
/*N*/   {
/*?*/       pIMap = new ImageMap;
/*?*/       pIMap->Read(
                *pStrm,
                ::binfilter::StaticBaseUrl::GetBaseURL(INetURLObject::NO_DECODE));
/*N*/   }
/*N*/
/*N*/   CloseRec( SWG_IMAGEMAP );
/*N*/
/*N*/   return pIMap;   // muss ggf. vom Aufrufer geloescht werden!
/*N*/ }

 PolyPolygon *Sw3IoImp::InContour()
 {
    PolyPolygon *pContour = 0;

    OpenRec( SWG_CONTOUR );
    BYTE cFlags = OpenFlagRec();
    CloseFlagRec();

    if( (cFlags & 0x10) != 0 )
    {
        pContour = new PolyPolygon;
        *pStrm >> *pContour;
    }

    CloseRec( SWG_CONTOUR );

    return pContour;
 }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
