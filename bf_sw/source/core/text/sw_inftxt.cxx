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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <com/sun/star/uno/Sequence.h>

#include <bf_svtools/linguprops.hxx>

#include <hintids.hxx>

#include <bf_svtools/ctloptions.hxx>
#include <bf_sfx2/printer.hxx>
#include <bf_svx/hyznitem.hxx>
#include <bf_svx/hngpnctitem.hxx>
#include <bf_svx/scriptspaceitem.hxx>
#include <bf_svx/pgrditem.hxx>
#include <breakit.hxx>
#include <bf_svx/forbiddenruleitem.hxx>
#include <swmodule.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <tools/shl.hxx>
#include <viewsh.hxx>   // ViewShell

#include <horiornt.hxx>

#include <doc.hxx>      // SwDoc
#include <paratr.hxx>   // SwFmtDrop
#include <inftxt.hxx>   // SwTxtInfo
#include <noteurl.hxx>  // SwNoteURL
#include <porftn.hxx>   // SwFtnPortion
#include <itratr.hxx>
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::linguistic2;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

#define C2U(cChar) ::rtl::OUString::createFromAscii(cChar)
#define CHAR_UNDERSCORE ((sal_Unicode)0x005F)
#define CHAR_LEFT_ARROW ((sal_Unicode)0x25C0)
#define CHAR_RIGHT_ARROW ((sal_Unicode)0x25B6)
#define CHAR_TAB ((sal_Unicode)0x2192)
#define CHAR_TAB_RTL ((sal_Unicode)0x2190)
#define CHAR_LINEBREAK ((sal_Unicode)0x21B5)
#define CHAR_LINEBREAK_RTL ((sal_Unicode)0x21B3)

#ifdef BIDI
#define DRAW_SPECIAL_OPTIONS_CENTER 1
#define DRAW_SPECIAL_OPTIONS_ROTATE 2
#endif

/*************************************************************************
 *                      SwLineInfo::SwLineInfo()
 *************************************************************************/

/*N*/ void SwLineInfo::CtorInit( const SwAttrSet& rAttrSet )
/*N*/ {
/*N*/   pRuler = &rAttrSet.GetTabStops();
/*N*/   pSpace = &rAttrSet.GetLineSpacing();
/*N*/     nVertAlign = rAttrSet.GetParaVertAlign().GetValue();
/*N*/     nDefTabStop = MSHRT_MAX;
/*N*/ }

/*************************************************************************
 *                      SwTxtInfo::CtorInit()
 *************************************************************************/

/*N*/ void SwTxtInfo::CtorInit( SwTxtFrm *pFrm )
/*N*/ {
/*N*/   pPara = pFrm->GetPara();
/*N*/   nTxtStart = pFrm->GetOfst();
/*N*/   if( !pPara )
/*N*/   {
/*?*/       OSL_ENSURE( pPara, "+SwTxtInfo::CTOR: missing paragraph information" );
/*?*/       pFrm->Format();
/*?*/       pPara = pFrm->GetPara();
/*N*/   }
/*N*/ }

/*N*/ SwTxtInfo::SwTxtInfo( const SwTxtInfo &rInf )
/*N*/   : pPara( ((SwTxtInfo&)rInf).GetParaPortion() ),
/*N*/     nTxtStart( rInf.GetTxtStart() )
/*N*/ { }


#ifdef DBG_UTIL
/*************************************************************************
 *                      ChkOutDev()
 *************************************************************************/

/*N*/ void ChkOutDev( const SwTxtSizeInfo &rInf )
/*N*/ {
/*N*/     if ( !rInf.GetVsh() )
/*N*/         return;
/*N*/
/*N*/     const OutputDevice *pOut = rInf.GetOut();
/*N*/     const OutputDevice *pWin = rInf.GetVsh()->GetWin();
/*N*/     const OutputDevice *pRef = rInf.GetRefDev();
/*N*/     OSL_ENSURE( pWin && pOut && pRef, "ChkOutDev: invalid output devices" );
/*N*/ }
/*N*/ #endif    // PRODUCT


/*N*/ inline xub_StrLen GetMinLen( const SwTxtSizeInfo &rInf )
/*N*/ {
/*N*/   const xub_StrLen nInfLen = rInf.GetIdx() + rInf.GetLen();
/*N*/   return Min( rInf.GetTxt().Len(), nInfLen );
/*N*/ }


/*N*/ SwTxtSizeInfo::SwTxtSizeInfo( const SwTxtSizeInfo &rNew )
/*N*/   : SwTxtInfo( rNew ),
/*N*/       pKanaComp(((SwTxtSizeInfo&)rNew).GetpKanaComp()),
/*N*/       pVsh(((SwTxtSizeInfo&)rNew).GetVsh()),
/*N*/     pOut(((SwTxtSizeInfo&)rNew).GetOut()),
/*N*/       pRef(((SwTxtSizeInfo&)rNew).GetRefDev()),
/*N*/     pFnt(((SwTxtSizeInfo&)rNew).GetFont()),
/*N*/       pUnderFnt(((SwTxtSizeInfo&)rNew).GetUnderFnt()),
/*N*/     pFrm(rNew.pFrm),
/*N*/     pOpt(&rNew.GetOpt()),
/*N*/     pTxt(&rNew.GetTxt()),
/*N*/     nIdx(rNew.GetIdx()),
/*N*/     nLen(rNew.GetLen()),
/*N*/       nKanaIdx( rNew.GetKanaIdx() ),
/*N*/     bOnWin( rNew.OnWin() ),
/*N*/     bNotEOL( rNew.NotEOL() ),
/*N*/     bURLNotify( rNew.URLNotify() ),
/*N*/     bStopUnderFlow( rNew.StopUnderFlow() ),
/*N*/       bFtnInside( rNew.IsFtnInside() ),
/*N*/     bMulti( rNew.IsMulti() ),
/*N*/     bFirstMulti( rNew.IsFirstMulti() ),
/*N*/     bRuby( rNew.IsRuby() ),
/*N*/     bHanging( rNew.IsHanging() ),
/*N*/     bScriptSpace( rNew.HasScriptSpace() ),
/*N*/     bForbiddenChars( rNew.HasForbiddenChars() ),
/*N*/       bSnapToGrid( rNew.SnapToGrid() ),
/*N*/     nDirection( rNew.GetDirection() )
/*N*/ {
/*N*/ #ifdef DBG_UTIL
/*N*/   ChkOutDev( *this );
/*N*/ #endif
/*N*/ }

/*N*/ void SwTxtSizeInfo::CtorInit( SwTxtFrm *pFrame, SwFont *pNewFnt,
/*N*/                  const xub_StrLen nNewIdx, const xub_StrLen nNewLen )
/*N*/ {
/*N*/     pKanaComp = NULL;
/*N*/     nKanaIdx = 0;
/*N*/   pFrm = pFrame;
/*N*/   SwTxtInfo::CtorInit( pFrm );
/*N*/   const SwTxtNode *pNd = pFrm->GetTxtNode();
/*N*/   pVsh = NULL;
/*N*/
/*N*/   //Zugriff ueber StarONE, es muss keine Shell existieren oder aktiv sein.
/*N*/   if ( pNd->GetDoc()->IsBrowseMode() ) //?!?!?!?
/*N*/       //in Ermangelung eines Besseren kann hier ja wohl nur noch das
/*N*/       //AppWin genommen werden?
/*N*/       pOut = GetpApp()->GetDefaultDevice();
/*N*/   else
/*N*/       pOut = pNd->GetDoc()->GetPrt(); //Muss es geben (oder sal_True uebergeben?)

/*N*/   pRef = pOut;
/*N*/
/*N*/ #ifdef DBG_UTIL
/*N*/     ChkOutDev( *this );
/*N*/ #endif
/*N*/
/*N*/     // Set default layout mode ( LTR or RTL ).
/*N*/     if ( pFrm->IsRightToLeft() )
/*N*/     {
/*N*/         pOut->SetLayoutMode( TEXT_LAYOUT_BIDI_STRONG | TEXT_LAYOUT_BIDI_RTL );
/*N*/         pRef->SetLayoutMode( TEXT_LAYOUT_BIDI_STRONG | TEXT_LAYOUT_BIDI_RTL );
/*N*/         nDirection = DIR_RIGHT2LEFT;
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         pOut->SetLayoutMode( TEXT_LAYOUT_BIDI_STRONG );
/*N*/         pRef->SetLayoutMode( TEXT_LAYOUT_BIDI_STRONG );
/*N*/         nDirection = DIR_LEFT2RIGHT;
/*N*/     }
/*N*/
/*N*/     LanguageType eLang;
/*N*/     const SvtCTLOptions& rCTLOptions = SW_MOD()->GetCTLOptions();
/*N*/     if ( SvtCTLOptions::NUMERALS_HINDI == rCTLOptions.GetCTLTextNumerals() )
/*N*/         eLang = LANGUAGE_ARABIC_SAUDI_ARABIA;
/*N*/     else if ( SvtCTLOptions::NUMERALS_ARABIC == rCTLOptions.GetCTLTextNumerals() )
/*N*/         eLang = LANGUAGE_ENGLISH;
/*N*/     else
/*N*/         eLang = (LanguageType)::binfilter::GetAppLanguage();
/*N*/
/*N*/     pOut->SetDigitLanguage( eLang );
/*N*/     pRef->SetDigitLanguage( eLang );
/*N*/
/*N*/     //
/*N*/     // The Options
/*N*/     //
/*N*/     pOpt = SW_MOD()->GetViewOption(pNd->GetDoc()->IsHTMLMode()); //Options vom Module wg. StarONE
/*N*/
/*N*/     // bURLNotify wird gesetzt, wenn MakeGraphic dies vorbereitet
/*N*/     // TODO: Aufdr�seln
/*N*/     bURLNotify = pNoteURL && !bOnWin;
/*N*/
/*N*/     SetSnapToGrid( pNd->GetSwAttrSet().GetParaGrid().GetValue() &&
/*N*/                    pFrm->IsInDocBody() );
/*N*/
/*N*/     pFnt = pNewFnt;
/*N*/     pUnderFnt = 0;
/*N*/     pTxt = &pNd->GetTxt();
/*N*/
/*N*/   nIdx = nNewIdx;
/*N*/   nLen = nNewLen;
/*N*/   bNotEOL = sal_False;
/*N*/     bStopUnderFlow = bFtnInside = sal_False;
/*N*/   bMulti = bFirstMulti = bRuby = bHanging = bScriptSpace =
/*N*/       bForbiddenChars = sal_False;
/*N*/ #ifndef BIDI
/*N*/     nDirection = DIR_LEFT2RIGHT;
/*N*/ #endif
/*N*/
/*N*/   SetLen( GetMinLen( *this ) );
/*N*/ }

/*N*/ SwTxtSizeInfo::SwTxtSizeInfo( const SwTxtSizeInfo &rNew, const XubString &rTxt,
/*N*/                             const xub_StrLen nInIdx, const xub_StrLen nInLen )
/*N*/   : SwTxtInfo( rNew ),
/*N*/       pKanaComp(((SwTxtSizeInfo&)rNew).GetpKanaComp()),
/*N*/     pVsh(((SwTxtSizeInfo&)rNew).GetVsh()),
/*N*/     pOut(((SwTxtSizeInfo&)rNew).GetOut()),
/*N*/       pRef(((SwTxtSizeInfo&)rNew).GetRefDev()),
/*N*/     pFnt(((SwTxtSizeInfo&)rNew).GetFont()),
/*N*/       pUnderFnt(((SwTxtSizeInfo&)rNew).GetUnderFnt()),
/*N*/     pFrm( rNew.pFrm ),
/*N*/     pOpt(&rNew.GetOpt()),
/*N*/     pTxt(&rTxt),
/*N*/     nIdx(nInIdx),
/*N*/     nLen(nInLen),
/*N*/       nKanaIdx( rNew.GetKanaIdx() ),
/*N*/     bOnWin( rNew.OnWin() ),
/*N*/     bNotEOL( rNew.NotEOL() ),
/*N*/     bURLNotify( rNew.URLNotify() ),
/*N*/     bStopUnderFlow( rNew.StopUnderFlow() ),
/*N*/       bFtnInside( rNew.IsFtnInside() ),
/*N*/     bMulti( rNew.IsMulti() ),
/*N*/     bFirstMulti( rNew.IsFirstMulti() ),
/*N*/     bRuby( rNew.IsRuby() ),
/*N*/     bHanging( rNew.IsHanging() ),
/*N*/     bScriptSpace( rNew.HasScriptSpace() ),
/*N*/     bForbiddenChars( rNew.HasForbiddenChars() ),
/*N*/       bSnapToGrid( rNew.SnapToGrid() ),
/*N*/     nDirection( rNew.GetDirection() )
/*N*/ {
/*N*/ #ifdef DBG_UTIL
/*N*/   ChkOutDev( *this );
/*N*/ #endif
/*N*/   SetLen( GetMinLen( *this ) );
/*N*/ }

/*************************************************************************
 *                      SwTxtSizeInfo::SelectFont()
 *************************************************************************/

/*N*/ void SwTxtSizeInfo::SelectFont()
/*N*/ {
/*N*/   GetFont()->Invalidate();
/*N*/   GetFont()->ChgPhysFnt( pVsh, GetOut() );
/*N*/ }

/*************************************************************************
 *                      SwTxtSizeInfo::GetTxtSize()
 *************************************************************************/

/*N*/ SwPosSize SwTxtSizeInfo::GetTxtSize( OutputDevice* pOutDev,
/*N*/                                      const SwScriptInfo* pSI,
/*N*/                                      const XubString& rTxt,
/*N*/                                    const xub_StrLen nIdx2,
/*N*/                                      const xub_StrLen nLen2,
/*N*/                                      const USHORT nComp ) const
/*N*/ {
/*N*/     SwDrawTextInfo aDrawInf( pVsh, *pOutDev, pSI, rTxt, nIdx2, nLen2 );
/*N*/     aDrawInf.SetFrm( pFrm );
/*N*/     aDrawInf.SetFont( pFnt );
/*N*/     aDrawInf.SetSnapToGrid( SnapToGrid() );
/*N*/     aDrawInf.SetKanaComp( nComp );
/*N*/     SwPosSize aSize = pFnt->_GetTxtSize( aDrawInf );
/*N*/     return aSize;
/*N*/ }

/*************************************************************************
 *                      SwTxtSizeInfo::GetTxtSize()
 *************************************************************************/

/*N*/ SwPosSize SwTxtSizeInfo::GetTxtSize() const
/*N*/ {
/*N*/     const SwScriptInfo& rSI =
/*N*/                      ( (SwParaPortion*)GetParaPortion() )->GetScriptInfo();
/*N*/
/*N*/     // in some cases, compression is not allowed or surpressed for
/*N*/     // performance reasons
/*N*/     USHORT nComp =( SW_CJK == GetFont()->GetActual() &&
/*N*/                     rSI.CountCompChg() &&
/*N*/                     ! IsMulti() ) ?
/*N*/                     GetKanaComp() :
/*N*/                                 0 ;
/*N*/
/*N*/     SwDrawTextInfo aDrawInf( pVsh, *pOut, &rSI, *pTxt, nIdx, nLen );
/*N*/     aDrawInf.SetFrm( pFrm );
/*N*/     aDrawInf.SetFont( pFnt );
/*N*/     aDrawInf.SetSnapToGrid( SnapToGrid() );
/*N*/     aDrawInf.SetKanaComp( nComp );
/*N*/     return pFnt->_GetTxtSize( aDrawInf );
/*N*/ }

/*************************************************************************
 *                      SwTxtSizeInfo::GetTxtSize()
 *************************************************************************/

/*N*/ void SwTxtSizeInfo::GetTxtSize( const SwScriptInfo* pSI, const xub_StrLen nIdx3,
/*N*/                                 const xub_StrLen nLen3, const USHORT nComp,
/*N*/                                 USHORT& nMinSize, USHORT& nMaxSizeDiff ) const
/*N*/ {
/*N*/     SwDrawTextInfo aDrawInf( pVsh, *pOut, pSI, *pTxt, nIdx3, nLen3 );
/*N*/     aDrawInf.SetFrm( pFrm );
/*N*/     aDrawInf.SetFont( pFnt );
/*N*/     aDrawInf.SetSnapToGrid( SnapToGrid() );
/*N*/     aDrawInf.SetKanaComp( nComp );
/*N*/     SwPosSize aSize = pFnt->_GetTxtSize( aDrawInf );
/*N*/     nMaxSizeDiff = (USHORT)aDrawInf.GetKanaDiff();
/*N*/     nMinSize = aSize.Width();
/*N*/ }

/*************************************************************************
 *                      SwTxtSizeInfo::GetTxtBreak()
 *************************************************************************/

/*N*/ xub_StrLen SwTxtSizeInfo::GetTxtBreak( const long nLineWidth,
/*N*/                                        const xub_StrLen nMaxLen,
/*N*/                                        const USHORT nComp ) const
/*N*/ {
/*N*/     const SwScriptInfo& rScriptInfo =
/*N*/                      ( (SwParaPortion*)GetParaPortion() )->GetScriptInfo();
/*N*/
/*N*/     OSL_ENSURE( pRef == pOut, "GetTxtBreak is supposed to use the RefDev" );
/*N*/     SwDrawTextInfo aDrawInf( pVsh, *pOut, &rScriptInfo,
/*N*/                              *pTxt, GetIdx(), nMaxLen );
/*N*/     aDrawInf.SetFrm( pFrm );
/*N*/     aDrawInf.SetFont( pFnt );
/*N*/     aDrawInf.SetSnapToGrid( SnapToGrid() );
/*N*/     aDrawInf.SetKanaComp( nComp );
/*N*/     aDrawInf.SetHyphPos( 0 );
/*N*/
/*N*/     return pFnt->GetTxtBreak( aDrawInf, nLineWidth );
/*N*/ }

/*************************************************************************
 *                      SwTxtSizeInfo::GetTxtBreak()
 *************************************************************************/

/*N*/ xub_StrLen SwTxtSizeInfo::GetTxtBreak( const long nLineWidth,
/*N*/                                        const xub_StrLen nMaxLen,
/*N*/                                        const USHORT nComp,
/*N*/                                        xub_StrLen& rExtraCharPos ) const
/*N*/ {
/*N*/     const SwScriptInfo& rScriptInfo =
/*N*/                      ( (SwParaPortion*)GetParaPortion() )->GetScriptInfo();
/*N*/
/*N*/     OSL_ENSURE( pRef == pOut, "GetTxtBreak is supposed to use the RefDev" );
/*N*/     SwDrawTextInfo aDrawInf( pVsh, *pOut, &rScriptInfo,
/*N*/                              *pTxt, GetIdx(), nMaxLen );
/*N*/     aDrawInf.SetFrm( pFrm );
/*N*/     aDrawInf.SetFont( pFnt );
/*N*/     aDrawInf.SetSnapToGrid( SnapToGrid() );
/*N*/     aDrawInf.SetKanaComp( nComp );
/*N*/     aDrawInf.SetHyphPos( &rExtraCharPos );
/*N*/
/*N*/     return pFnt->GetTxtBreak( aDrawInf, nLineWidth );
/*N*/ }

/*************************************************************************
 *                     SwTxtPaintInfo::CtorInit()
 *************************************************************************/

/*N*/ void SwTxtPaintInfo::CtorInit( SwTxtFrm *pFrame, const SwRect &rPaint )
/*N*/ {
/*N*/   SwTxtSizeInfo::CtorInit( pFrame );
/*N*/   aTxtFly.CtorInit( pFrame ),
/*N*/   aPaintRect = rPaint;
/*N*/   nSpaceIdx = 0;
/*N*/   pSpaceAdd = NULL;
/*N*/   pWrongList = NULL;
/*N*/ #ifndef DBG_UTIL
/*N*/   pBrushItem = 0;
/*N*/ #else
/*N*/   pBrushItem = ((SvxBrushItem*)-1);
/*N*/ #endif
/*N*/ }


/*N*/ SwTxtPaintInfo::SwTxtPaintInfo( const SwTxtPaintInfo &rInf )
/*N*/   : SwTxtSizeInfo( rInf )
/*N*/   , pWrongList( rInf.GetpWrongList() )
/*N*/   , pSpaceAdd( rInf.GetpSpaceAdd() )
/*N*/   , pBrushItem( rInf.GetBrushItem() )
/*N*/   , aTxtFly( *rInf.GetTxtFly() )
/*N*/   , aPos( rInf.GetPos() )
/*N*/   , aPaintRect( rInf.GetPaintRect() )
/*N*/   , nSpaceIdx( rInf.GetSpaceIdx() )
/*N*/ { }

extern Color aGlobalRetoucheColor;

/*************************************************************************
 *                  lcl_InitHyphValues()
 *************************************************************************/

/*N*/ static void lcl_InitHyphValues( PropertyValues &rVals,
/*N*/           INT16 nMinLeading, INT16 nMinTrailing )
/*N*/ {
/*N*/   INT32 nLen = rVals.getLength();
/*N*/
/*N*/   if (0 == nLen)  // yet to be initialized?
/*N*/   {
/*N*/       rVals.realloc( 2 );
/*N*/       PropertyValue *pVal = rVals.getArray();
/*N*/
/*N*/       pVal[0].Name    = C2U( UPN_HYPH_MIN_LEADING );
/*N*/       pVal[0].Handle  = UPH_HYPH_MIN_LEADING;
/*N*/       pVal[0].Value   <<= nMinLeading;
/*N*/
/*N*/       pVal[1].Name    = C2U( UPN_HYPH_MIN_TRAILING );
/*N*/       pVal[1].Handle  = UPH_HYPH_MIN_TRAILING;
/*N*/       pVal[1].Value   <<= nMinTrailing;
/*N*/   }
/*N*/   else if (2 == nLen) // already initialized once?
/*N*/   {
/*?*/       PropertyValue *pVal = rVals.getArray();
/*?*/       pVal[0].Value <<= nMinLeading;
/*?*/       pVal[1].Value <<= nMinTrailing;
/*?*/   }
/*?*/   else
/*?*/       OSL_FAIL( "unxpected size of sequence" );
/*N*/ }

/*************************************************************************
 *                  SwTxtFormatInfo::GetHyphValues()
 *************************************************************************/
/*N*/
/*N*/ const PropertyValues & SwTxtFormatInfo::GetHyphValues() const
/*N*/ {
/*N*/   DBG_ASSERT( 2 == aHyphVals.getLength(),
/*N*/           "hyphenation values not yet initialized" );
/*N*/   return aHyphVals;
/*N*/ }

/*************************************************************************
 *                  SwTxtFormatInfo::InitHyph()
 *************************************************************************/

/*N*/ sal_Bool SwTxtFormatInfo::InitHyph( const sal_Bool bAutoHyph1 )
/*N*/ {
/*N*/   const SwAttrSet& rAttrSet = GetTxtFrm()->GetTxtNode()->GetSwAttrSet();
/*N*/   SetHanging( rAttrSet.GetHangingPunctuation().GetValue() );
/*N*/   SetScriptSpace( rAttrSet.GetScriptSpace().GetValue() );
/*N*/   SetForbiddenChars( rAttrSet.GetForbiddenRule().GetValue() );
/*N*/   const SvxHyphenZoneItem &rAttr = rAttrSet.GetHyphenZone();
/*N*/   MaxHyph() = rAttr.GetMaxHyphens();
/*N*/   sal_Bool bAuto = bAutoHyph1 || rAttr.IsHyphen();
/*N*/   if( bAuto || bInterHyph )
/*N*/   {
/*N*/       nHyphStart = nHyphWrdStart = STRING_LEN;
/*N*/       nHyphWrdLen = 0;
/*N*/
/*N*/       INT16 nMinLeading1  = Max(rAttr.GetMinLead(), sal_uInt8(2));
/*N*/       INT16 nMinTrailing1 = rAttr.GetMinTrail();
/*N*/       lcl_InitHyphValues( aHyphVals, nMinLeading1, nMinTrailing1);
/*N*/   }
/*N*/   return bAuto;
/*N*/ }

/*************************************************************************
 *                  SwTxtFormatInfo::CtorInit()
 *************************************************************************/

/*N*/ void SwTxtFormatInfo::CtorInit( SwTxtFrm *pNewFrm, const sal_Bool bNewInterHyph,
/*N*/                               const sal_Bool bNewQuick, const sal_Bool bTst )
/*N*/ {
/*N*/   SwTxtPaintInfo::CtorInit( pNewFrm, SwRect() );
/*N*/
/*N*/   bQuick = bNewQuick;
/*N*/   bInterHyph = bNewInterHyph;
/*N*/
/*N*/   //! needs to be done in this order
/*N*/   nMinLeading     = 2;
/*N*/   nMinTrailing    = 2;
/*N*/   nMinWordLength  = 0;
/*N*/   bAutoHyph = InitHyph();
/*N*/
/*N*/   bIgnoreFly = sal_False;
/*N*/     bFakeLineStart = sal_False;
/*N*/   bShift = sal_False;
/*N*/   bDropInit = sal_False;
/*N*/   bTestFormat = bTst;
/*N*/   nLeft = 0;
/*N*/   nRight = 0;
/*N*/   nFirst = 0;
/*N*/   nRealWidth = 0;
/*N*/   nForcedLeftMargin = 0;
/*N*/   pRest = 0;
/*N*/   nLineHeight = 0;
/*N*/     nLineNettoHeight = 0;
/*N*/   SetLineStart(0);
/*N*/   Init();
/*N*/ }

/*************************************************************************
 *                  SwTxtFormatInfo::IsHyphenate()
 *************************************************************************/
// Trennen oder nicht trennen, das ist hier die Frage:
// - in keinem Fall trennen, wenn der Hyphenator ERROR zurueckliefert,
//   oder wenn als Sprache NOLANGUAGE eingestellt ist.
// - ansonsten immer trennen, wenn interaktive Trennung vorliegt
// - wenn keine interakt. Trennung, dann nur trennen, wenn im ParaFmt
//   automatische Trennung eingestellt ist.

/*N*/ sal_Bool SwTxtFormatInfo::IsHyphenate() const
/*N*/ {
/*N*/   if( !bInterHyph && !bAutoHyph )
/*N*/       return sal_False;
/*N*/
/*N*/   LanguageType eTmp = GetFont()->GetLanguage();
/*N*/   if( LANGUAGE_DONTKNOW == eTmp || LANGUAGE_NONE == eTmp )
/*N*/       return sal_False;
/*N*/
/*N*/   uno::Reference< XHyphenator > xHyph = ::binfilter::GetHyphenator();
/*N*/
/*N*/   if( !xHyph.is() || !xHyph->hasLocale( pBreakIt->GetLocale(eTmp) ) )
/*N*/       return sal_False;
/*N*/   return sal_True;
/*N*/ }

/*************************************************************************
 *                  SwTxtFormatInfo::GetDropFmt()
 *************************************************************************/

// Dropcaps vom SwTxtFormatter::CTOR gerufen.
/*N*/ const SwFmtDrop *SwTxtFormatInfo::GetDropFmt() const
/*N*/ {
/*N*/   const SwFmtDrop *pDrop = &GetTxtFrm()->GetTxtNode()->GetSwAttrSet().GetDrop();
/*N*/   if( 1 >= pDrop->GetLines() ||
/*N*/       ( !pDrop->GetChars() && !pDrop->GetWholeWord() ) )
/*N*/       pDrop = 0;
/*N*/   return pDrop;
/*N*/ }

/*************************************************************************
 *                      SwTxtFormatInfo::Init()
 *************************************************************************/

/*N*/ void SwTxtFormatInfo::Init()
/*N*/ {
/*N*/   // Nicht initialisieren: pRest, nLeft, nRight, nFirst, nRealWidth
/*N*/   X(0);
/*N*/   bArrowDone = bFull = bFtnDone = bErgoDone = bNumDone = bNoEndHyph =
/*N*/       bNoMidHyph = bStop = bNewLine = bUnderFlow = sal_False;
/*N*/
/*N*/     // generally we do not allow number portions in follows, except...
/*N*/     if ( GetTxtFrm()->IsFollow() )
/*N*/     {
/*N*/         const SwTxtFrm* pMaster = GetTxtFrm()->FindMaster();
/*N*/         const SwLinePortion* pPara1 = pMaster->GetPara();
/*N*/
/*N*/         // there is a master for this follow and the master does not have
/*N*/         // any contents (especially it does not have a number portion)
/*N*/         bNumDone = ! pPara1 ||
/*N*/                    ! ((SwParaPortion*)pPara1)->GetFirstPortion()->IsFlyPortion();
/*N*/     }
/*N*/
/*N*/   pRoot = 0;
/*N*/   pLast = 0;
/*N*/   pFly = 0;
/*N*/   pLastFld = 0;
/*N*/   pLastTab = 0;
/*N*/   pUnderFlow = 0;
/*N*/   cTabDecimal = 0;
/*N*/   nWidth = nRealWidth;
/*N*/   nForcedLeftMargin = 0;
/*N*/   nSoftHyphPos = 0;
/*N*/     nUnderScorePos = STRING_LEN;
/*N*/   cHookChar = 0;
/*N*/   SetIdx(0);
/*N*/   SetLen( GetTxt().Len() );
/*N*/   SetPaintOfst(0);
/*N*/ }

/*--------------------------------------------------
 * There are a few differences between a copy constructor
 * and the following constructor for multi-line formatting.
 * The root is the first line inside the multi-portion,
 * the line start is the actual position in the text,
 * the line width is the rest width from the surrounding line
 * and the bMulti and bFirstMulti-flag has to be set correctly.
 * --------------------------------------------------*/


/*************************************************************************
 *                 SwTxtFormatInfo::_CheckFtnPortion()
 *************************************************************************/

/*N*/ sal_Bool SwTxtFormatInfo::_CheckFtnPortion( SwLineLayout* pCurr )
/*N*/ {
/*N*/   KSHORT nHeight = pCurr->GetRealHeight();
/*N*/   SwLinePortion *pPor = pCurr->GetPortion();
/*N*/   sal_Bool bRet = sal_False;
/*N*/   while( pPor )
/*N*/   {
/*N*/       if( pPor->IsFtnPortion() && nHeight > ((SwFtnPortion*)pPor)->Orig() )
/*N*/       {
/*?*/           bRet = sal_True;
/*?*/           SetLineHeight( nHeight );
/*?*/             SetLineNettoHeight( pCurr->Height() );
/*?*/           break;
/*N*/       }
/*N*/       pPor = pPor->GetPortion();
/*N*/   }
/*N*/   return bRet;
/*N*/ }




/*************************************************************************
 *                 SwTxtFormatInfo::ScanPortionEnd()
 *************************************************************************/
/*N*/ xub_StrLen SwTxtFormatInfo::ScanPortionEnd( const xub_StrLen nStart,
/*N*/                                             const xub_StrLen nEnd )
/*N*/ {
/*N*/   cHookChar = 0;
/*N*/   const sal_Unicode cTabDec = GetLastTab() ? (sal_Unicode)GetTabDecimal() : 0;
/*N*/   xub_StrLen i = nStart;
/*N*/
/*N*/   for( ; i < nEnd; ++i )
/*N*/   {
/*N*/       const sal_Unicode cPos = GetChar( i );
/*N*/       switch( cPos )
/*N*/       {
/*N*/       case CH_TXTATR_BREAKWORD:
/*N*/       case CH_TXTATR_INWORD:
/*N*/           if( !HasHint( i ))
/*N*/               break;
/*N*/           // no break;
/*N*/
/*N*/       case CHAR_SOFTHYPHEN:
/*N*/       case CHAR_HARDHYPHEN:
/*N*/       case CHAR_HARDBLANK:
/*N*/       case CH_TAB:
/*N*/       case CH_BREAK:
/*N*/           cHookChar = cPos;
/*N*/           return i;
/*N*/
/*N*/         case CHAR_UNDERSCORE:
/*N*/             if ( STRING_LEN == nUnderScorePos )
/*N*/                 nUnderScorePos = i;
/*N*/             break;
/*N*/
/*N*/         default:
/*N*/           if( cTabDec == cPos )
/*N*/           {
/*N*/               OSL_ENSURE( cPos, "Unexspected end of string" );
/*N*/               if( cPos ) // robust
/*N*/               {
/*N*/                   cHookChar = cPos;
/*N*/                   return i;
/*N*/               }
/*N*/           }
/*N*/       }
/*N*/   }
/*N*/   return i;
/*N*/ }

/*N*/ BOOL SwTxtFormatInfo::LastKernPortion()
/*N*/ {
/*N*/   if( GetLast() )
/*N*/   {
/*N*/       if( GetLast()->IsKernPortion() )
/*N*/           return TRUE;
/*N*/       if( GetLast()->Width() || ( GetLast()->GetLen() &&
/*N*/           !GetLast()->IsHolePortion() ) )
/*N*/           return FALSE;
/*N*/   }
/*N*/   SwLinePortion* pPor = GetRoot();
/*N*/   SwLinePortion *pKern = NULL;
/*N*/   while( pPor )
/*N*/   {
/*N*/       if( pPor->IsKernPortion() )
/*N*/           pKern = pPor;
/*N*/       else if( pPor->Width() || ( pPor->GetLen() && !pPor->IsHolePortion() ) )
/*N*/           pKern = NULL;
/*N*/       pPor = pPor->GetPortion();
/*N*/   }
/*N*/   if( pKern )
/*N*/   {
/*?*/       SetLast( pKern );
/*N*/       return TRUE;
/*N*/   }
/*N*/   return FALSE;
/*N*/ }

/*************************************************************************
 *                      class SwTxtSlot
 *************************************************************************/

/*N*/ SwTxtSlot::SwTxtSlot( const SwTxtSizeInfo *pNew, const SwLinePortion *pPor )
/*N*/ {
/*N*/   bOn = pPor->GetExpTxt( *pNew, aTxt );
/*N*/
/*N*/   // Der Text wird ausgetauscht...
/*N*/   if( bOn )
/*N*/   {
/*N*/       pInf = (SwTxtSizeInfo*)pNew;
/*N*/       nIdx = pInf->GetIdx();
/*N*/       nLen = pInf->GetLen();
/*N*/       pInf->SetLen( pPor->GetLen() );
/*N*/       pOldTxt = &(pInf->GetTxt());
/*N*/       pInf->SetTxt( aTxt );
/*N*/       pInf->SetIdx( 0 );
/*N*/   }
/*N*/ }

/*************************************************************************
 *                       SwTxtSlot::~SwTxtSlot()
 *************************************************************************/

/*N*/ SwTxtSlot::~SwTxtSlot()
/*N*/ {
/*N*/   if( bOn )
/*N*/   {
/*N*/       pInf->SetTxt( *pOldTxt );
/*N*/       pInf->SetIdx( nIdx );
/*N*/       pInf->SetLen( nLen );
/*N*/   }
/*N*/ }

/*************************************************************************
 *                      class SwTxtSlotLen
 *************************************************************************/

/*N*/ SwTxtSlotLen::SwTxtSlotLen( const SwTxtSizeInfo *pNew, const SwLinePortion *pPor,
/*N*/   const sal_Char *pCh )
/*N*/ {
/*N*/   if( pCh )
/*N*/   {
/*?*/       aTxt = XubString( pCh, RTL_TEXTENCODING_MS_1252 );
/*?*/       bOn = sal_True;
/*N*/   }
/*N*/   else
/*N*/       bOn = pPor->GetExpTxt( *pNew, aTxt );
/*N*/
/*N*/   // Der Text wird ausgetauscht...
/*N*/   if( bOn )
/*N*/   {
/*N*/       pInf = (SwTxtSizeInfo*)pNew;
/*N*/       nIdx = pInf->GetIdx();
/*N*/       nLen = pInf->GetLen();
/*N*/       pOldTxt = &(pInf->GetTxt());
/*N*/       pInf->SetTxt( aTxt );
/*N*/       pInf->SetIdx( 0 );
/*N*/       pInf->SetLen( pInf->GetTxt().Len() );
/*N*/   }
/*N*/ }

/*************************************************************************
 *                       SwTxtSlotLen::~SwTxtSlotLen()
 *************************************************************************/

/*N*/ SwTxtSlotLen::~SwTxtSlotLen()
/*N*/ {
/*N*/   if( bOn )
/*N*/   {
/*N*/       pInf->SetTxt( *pOldTxt );
/*N*/       pInf->SetIdx( nIdx );
/*N*/       pInf->SetLen( nLen );
/*N*/   }
/*N*/ }

/*************************************************************************
 *                     SwFontSave::SwFontSave()
 *************************************************************************/

/*N*/ SwFontSave::SwFontSave( const SwTxtSizeInfo &rInf, SwFont *pNew,
/*N*/       SwAttrIter* pItr )
/*N*/       : pFnt( pNew ? ((SwTxtSizeInfo&)rInf).GetFont() : 0 )
/*N*/ {
/*N*/   if( pFnt )
/*N*/   {
/*N*/       pInf = &((SwTxtSizeInfo&)rInf);
/*N*/         // In these cases we temporarily switch to the new font:
/*N*/         // 1. the fonts have a different magic number
/*N*/         // 2. they have different script types
/*N*/         // 3. their background colors differ (this is not covered by 1.)
/*N*/       if( pFnt->DifferentMagic( pNew, pFnt->GetActual() ) ||
/*N*/             pNew->GetActual() != pFnt->GetActual() ||
/*N*/             ( ! pNew->GetBackColor() && pFnt->GetBackColor() ) ||
/*N*/             ( pNew->GetBackColor() && ! pFnt->GetBackColor() ) ||
/*N*/             ( pNew->GetBackColor() && pFnt->GetBackColor() &&
/*N*/               ( *pNew->GetBackColor() != *pFnt->GetBackColor() ) ) )
/*N*/       {
/*N*/           pNew->SetTransparent( sal_True );
/*N*/           pNew->SetAlign( ALIGN_BASELINE );
/*N*/           pInf->SetFont( pNew );
/*N*/       }
/*N*/       else
/*N*/           pFnt = 0;
/*N*/       pNew->Invalidate();
/*N*/       pNew->ChgPhysFnt( pInf->GetVsh(), pInf->GetOut() );
/*N*/       if( pItr && pItr->GetFnt() == pFnt )
/*N*/       {
/*?*/           pIter = pItr;
/*?*/           pIter->SetFnt( pNew );
/*N*/       }
/*N*/       else
/*N*/           pIter = NULL;
/*N*/   }
/*N*/ }

/*************************************************************************
 *                     SwFontSave::~SwFontSave()
 *************************************************************************/

/*N*/ SwFontSave::~SwFontSave()
/*N*/ {
/*N*/   if( pFnt )
/*N*/   {
/*N*/       // SwFont zurueckstellen
/*N*/       pFnt->Invalidate();
/*N*/       pInf->SetFont( pFnt );
/*N*/       if( pIter )
/*N*/       {
/*?*/           pIter->SetFnt( pFnt );
/*?*/           pIter->nPos = STRING_LEN;
/*N*/       }
/*N*/   }
/*N*/ }

/*************************************************************************
 *                  SwTxtFormatInfo::ChgHyph()
 *************************************************************************/

/*N*/ sal_Bool SwTxtFormatInfo::ChgHyph( const sal_Bool bNew )
/*N*/ {
/*N*/   const sal_Bool bOld = bAutoHyph;
/*N*/   if( bAutoHyph != bNew )
/*N*/   {
/*N*/       bAutoHyph = bNew;
/*N*/       InitHyph( bNew );
/*N*/       // 5744: Sprache am Hyphenator einstellen.
/*N*/       if( pFnt )
/*N*/           pFnt->ChgPhysFnt( pVsh, pOut );
/*N*/   }
/*N*/   return bOld;
/*N*/ }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
