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

#include <bf_svtools/bf_solar.h>

#include "swerror.h"

#include <horiornt.hxx>

#include "doc.hxx"

#include <osl/diagnose.h>

#include "pam.hxx"
#include "ndtxt.hxx"

#include <bf_svx/brshitem.hxx>
#include <bf_svx/lrspitem.hxx>
#include <bf_svx/tstpitem.hxx>

#include <vcl/font.hxx>
#include <tools/tenccvt.hxx>

#include <fmtornt.hxx>
#include <charfmt.hxx>
#include <poolfmt.hxx>
#include <frmatr.hxx>

#include "sw3imp.hxx"
#include "ftninfo.hxx"
#include "pagedesc.hxx"
#include <SwStyleNameMapper.hxx>
#include <bf_tools/string.hxx>
namespace binfilter {

////////////////////////////////////////////////////////////////////////////

/*N*/ SV_IMPL_OP_PTRARR_SORT(Sw3NumRuleInfos, Sw3NumRuleInfoPtr)

/*N*/ BYTE lcl_sw3io__GetIncludeUpperLevel( BOOL bInclUpperLevel )
/*N*/ {
/*N*/   return bInclUpperLevel ? MAXLEVEL : 1;
/*N*/ }

// Numerierungs-Format einlesen
/*N*/ void Sw3IoImp::InNumFmt( SwNumFmt& rFmt )
/*N*/ {
/*N*/   // Flags:
/*N*/   // 0x10 - Bullet-Font gueltig
/*N*/   BYTE cFlags, eType, eNumAdjust;
/*N*/   sal_Char cBullet8;
/*N*/   USHORT nFmt, nStart;
/*N*/   String aFontName, aFontStyle, sPrefix, sPostfix;
/*N*/   BYTE eFamily, ePitch, eCharSet;
/*N*/   INT32 nLSpace, nFirstLineOffset;
/*N*/   BYTE nUpperLevel;
/*N*/   OpenRec( SWG_NUMFMT );
/*N*/   InString( *pStrm, sPrefix );
/*N*/   InString( *pStrm, sPostfix );
/*N*/   InString( *pStrm, aFontName );
/*N*/   InString( *pStrm, aFontStyle );
/*N*/   *pStrm >> nFmt
/*N*/          >> eType
/*N*/          >> cBullet8;
/*N*/
/*N*/   // nUpperLevel war bis zur 5.0 nur ein Flag.
/*N*/   if( IsVersion(SWG_LONGIDX) )
/*N*/   {
/*N*/       *pStrm >> nUpperLevel;
/*N*/   }
/*N*/   else
/*N*/   {
/*N*/       BYTE bInclUpperLevel;
/*N*/       *pStrm >> bInclUpperLevel;
/*N*/       nUpperLevel = lcl_sw3io__GetIncludeUpperLevel( bInclUpperLevel );
/*N*/   }
/*N*/
/*N*/   *pStrm >> nStart
/*N*/          >> eNumAdjust
/*N*/          >> nLSpace
/*N*/          >> nFirstLineOffset
/*N*/          >> eFamily
/*N*/          >> ePitch
/*N*/          >> eCharSet;
/*N*/   if(RTL_TEXTENCODING_DONTKNOW== eCharSet)
/*N*/       eCharSet = RTL_TEXTENCODING_SYMBOL;
/*N*/   else if( RTL_TEXTENCODING_SYMBOL != eCharSet )
/*N*/       eCharSet = GetSOLoadTextEncoding( rtl_TextEncoding( eCharSet ) );
/*N*/   cFlags = OpenFlagRec();
/*N*/   CloseFlagRec();
/*N*/
/*N*/   sal_Bool bBatsToSymbol = sal_False;
/*N*/   sal_Bool bMathToSymbol = sal_False;
/*N*/   if( (cFlags & 0x10) != 0 && RTL_TEXTENCODING_SYMBOL == eCharSet )
/*N*/   {
/*N*/       if( aFontName.EqualsIgnoreCaseAscii( sStarBats ) )
/*N*/           bBatsToSymbol = sal_True;
/*N*/       else if( aFontName.EqualsIgnoreCaseAscii( sStarMath ) )
/*N*/           bMathToSymbol = sal_True;
/*N*/   }
/*N*/
/*N*/   rFmt.SetNumberingType((sal_Int16)eType );
/*N*/   if( bBatsToSymbol )
/*N*/       rFmt.SetBulletChar( ConvStarBatsCharToStarSymbol( cBullet8 ) );
/*N*/   else if( bMathToSymbol )
            rFmt.SetBulletChar( ConvStarMathCharToStarSymbol( cBullet8 ) );
/*N*/   else
/*N*/       rFmt.SetBulletChar( ByteString::ConvertToUnicode( cBullet8, eCharSet ) );
/*N*/   rFmt.SetIncludeUpperLevels( nUpperLevel );
/*N*/   rFmt.SetStart( nStart );
/*N*/   rFmt.SetNumAdjust( SvxAdjust( eNumAdjust ));
/*N*/   rFmt.SetAbsLSpace( (USHORT)nLSpace );
/*N*/   rFmt.SetFirstLineOffset( (short)nFirstLineOffset );
/*N*/   rFmt.SetSuffix( sPostfix );
/*N*/   rFmt.SetPrefix( sPrefix );
/*N*/
/*N*/   // Nur im spaeten 3.1-Format steht hier die Zeichen-Vorlage,
/*N*/   // ab dem 4.0-Format steht sie dann wieder oeben.
/*N*/   if( IsVersion( SWG_NEWNUMRULE, SWG_EXPORT31 ) )
/*N*/   {
/*?*/       *pStrm >> nFmt;
/*N*/   }
/*N*/
/*N*/   // erst das Format setzen
/*N*/   if( nFmt != IDX_NO_VALUE )
/*N*/       rFmt.SetCharFmt( (SwCharFmt*) FindFmt( nFmt, SWG_CHARFMT ) );
/*N*/
/*N*/   // Relative Abstaende gab es in spaeten 3.1- und im 4.0-Format, aber
/*N*/   // nicht mehr im 5.0 Format.
/*N*/   if( IsVersion( SWG_NEWNUMRULE, SWG_EXPORT31, SWG_DESKTOP40, SWG_LONGIDX ) )
/*N*/   {
/*N*/       BYTE bRelSpace;
/*N*/       INT32 nRelLSpace;
/*N*/
/*N*/       *pStrm >> bRelSpace
/*N*/              >> nRelLSpace;
/*N*/   }
/*N*/
/*N*/   // Den Rest gibt es in spaeten 3.1-Formaten und seit dem 4.0-Format
/*N*/   if( IsVersion( SWG_NEWNUMRULE, SWG_EXPORT31, SWG_DESKTOP40 ) )
/*N*/   {
/*N*/       USHORT nTextOffset;
/*N*/       *pStrm >> nTextOffset;
/*N*/
/*N*/       rFmt.SetCharTextDistance( nTextOffset );
/*N*/
/*N*/       if( SVX_NUM_BITMAP == rFmt.GetNumberingType() )
/*N*/       {
/*?*/           BYTE cF;
/*?*/           Size aSz;
/*?*/
/*?*/           *pStrm >> aSz.Width() >> aSz.Height();
/*?*/
/*?*/           *pStrm >> cF;
/*?*/           if( cF )
/*?*/           {
/*?*/               SvxBrushItem* pBrush = 0;
/*?*/               SwFmtVertOrient* pVOrient = 0;
/*?*/               USHORT nVer;
/*?*/
/*?*/               if( cF & 1 )
/*?*/               {
/*?*/                   *pStrm >> nVer;
/*?*/                   pBrush = (SvxBrushItem*)GetDfltAttr( RES_BACKGROUND )
/*?*/                                           ->Create( *pStrm, nVer );
/*?*/               }
/*?*/
/*?*/               if( cF & 2 )
/*?*/               {
/*?*/                   *pStrm >> nVer;
/*?*/                   pVOrient = (SwFmtVertOrient*)GetDfltAttr( RES_VERT_ORIENT )
/*?*/                                           ->Create( *pStrm, nVer );
/*?*/               }
/*?*/               SvxFrameVertOrient eOrient = SVX_VERT_NONE;
/*?*/               if(pVOrient)
/*?*/                   eOrient = (SvxFrameVertOrient)pVOrient->GetVertOrient();
/*?*/               rFmt.SetGraphicBrush( pBrush, &aSz, pVOrient ? &eOrient : 0);
/*?*/
/*?*/               // Ggf. Grafik holen, damit nicht neu numeriert wird,
/*?*/               // wenn die Grafik (spaeter) geladen wird.
/*?*/               //if( pBrush && rFmt.GetCharFmt() )
/*?*/               //  rFmt.GetGraphic();
/*?*/           }
/*N*/       }
/*N*/
/*N*/   }
/*N*/
/*N*/   if( cFlags & 0x10 )
/*N*/   {
/*N*/       Font aFont;
/*N*/       aFont.SetName( aFontName );
/*N*/       aFont.SetStyleName( aFontStyle );
/*N*/       aFont.SetFamily( FontFamily( eFamily ) );
/*N*/       aFont.SetPitch( FontPitch( ePitch ) );
/*N*/       aFont.SetCharSet( eCharSet );
/*N*/       if( bMathToSymbol || bBatsToSymbol )
/*N*/       {
/*N*/           aFont = SwNumRule::GetDefBulletFont();
/*N*/       }
/*N*/       else
/*N*/       {
/*?*/           aFont.SetName( aFontName );
/*?*/           aFont.SetStyleName( aFontStyle );
/*?*/           aFont.SetFamily( FontFamily( eFamily ) );
/*?*/           aFont.SetPitch( FontPitch( ePitch ) );
/*?*/           aFont.SetCharSet( rtl_TextEncoding( eCharSet ) );
/*?*/           aFont.SetCharSet(
/*?*/               GetSOLoadTextEncoding( rtl_TextEncoding( eCharSet ) ) );
/*N*/       }
/*N*/       // muss sein...
/*N*/       aFont.SetTransparent( TRUE );
/*N*/       rFmt.SetBulletFont( &aFont );
/*N*/
/*N*/       // JP 13.10.95: kleiner BugFix fuer vordefinierte benannte NumRules
/*N*/       //              aus der Configuration: diese lasen Bullet Fonts nicht
/*N*/       //              richtig ein - Prefix/PostFixString koennen dadurch
/*N*/       //              ungueltig sein. AMA kommt damit leicht durcheinander
/*N*/       if( sPrefix.Len() > 50 )        // Max-Length vom Dialog
/*?*/           rFmt.SetPrefix( aEmptyStr );
/*N*/       if( sPostfix.Len() > 50 )       // Max-Length vom Dialog
/*?*/           rFmt.SetSuffix( aEmptyStr );
/*N*/   }
/*N*/
/*N*/   CloseRec( SWG_NUMFMT );
/*N*/ }

// Numerierungs-Regelwerk einlesen

/*N*/ SwNumRule* Sw3IoImp::InNumRule( BYTE cType )
/*N*/ {
/*N*/   BYTE eType, nFmt, cFmt[ MAXLEVEL ];
/*N*/   OpenRec( cType );
/*N*/
/*N*/   // Den Flag-Record und Pool-Ids gibt's erst seit der 5.0. Anderenfalls
/*N*/   // ist das Invalid-Flag immer zu setzen.
/*N*/   USHORT nStrIdx = IDX_NO_VALUE, nPoolId = USHRT_MAX, nPoolHelpId;
/*N*/   BYTE cFlags = 0x20, nPoolHlpFileId;
/*N*/   if( IsVersion(SWG_LONGIDX) )
/*N*/   {
/*N*/       cFlags = OpenFlagRec();
/*N*/
/*N*/       *pStrm >> nStrIdx;
/*N*/
/*N*/       if( cFlags & 0x10 )
/*N*/       {
/*N*/           *pStrm >> nPoolId
/*N*/                  >> nPoolHelpId
/*N*/                  >> nPoolHlpFileId;
/*N*/       }
/*N*/   }
/*N*/
/*N*/   *pStrm >> eType;
/*N*/
/*N*/   if( IsVersion(SWG_LONGIDX) )
/*N*/       CloseFlagRec();
/*N*/
/*N*/   *pStrm >> nFmt;
/*N*/
/*N*/   // C 8.0 bug:
/*N*/   SwNumRuleType eTemp = (SwNumRuleType) eType;
/*N*/   String aName;
/*N*/   if( (cFlags & 0x10) != 0 &&
/*N*/       nPoolId >= RES_POOLNUMRULE_BEGIN &&
/*N*/       nPoolId < RES_POOLNUMRULE_END )
/*N*/   {
/*N*/       // Name der Pool-Vorlage vom Dok holen.
/*N*/       SwStyleNameMapper::FillUIName( nPoolId, aName );
/*N*/   }
/*N*/   else if( nStrIdx != IDX_NO_VALUE )
/*N*/   {
/*N*/       aName = aStringPool.Find( nStrIdx );
/*N*/
/*N*/       // NumRule ist in jedem Fall Benutzer-Definiert.
/*N*/       nPoolId = USHRT_MAX;
/*N*/   }
/*N*/   else if( SWG_OUTLINE == cType )
/*N*/       aName.AssignAscii( SwNumRule::GetOutlineRuleName() );
/*N*/   else
/*N*/       aName = pDoc->GetUniqueNumRuleName();
/*N*/
/*N*/   SwNumRule* pRule = new SwNumRule( aName, eTemp, (cFlags & 0x10)==0 );
/*N*/
/*N*/   if( (cFlags & 0x10) != 0 )
/*N*/   {
/*N*/       pRule->SetPoolFmtId( nPoolId );
/*N*/       pRule->SetPoolHelpId( nPoolHelpId );
/*N*/       pRule->SetPoolHlpFileId( nPoolHlpFileId );
/*N*/   }
/*N*/   pRule->SetInvalidRule( (cFlags & 0x20) != 0 || !bNormal || bInsert );
/*N*/   pRule->SetContinusNum( (cFlags & 0x40) != 0 );
/*N*/ #ifdef NUM_RELSPACE
/*N*/   pRule->SetAbsSpaces( IsVersion(SWG_NUMRELSPACE) && (cFlags & 0x80) != 0 );
/*N*/ #endif
/*N*/
/*N*/   // in Zukunft koennten auch mal mehr als MAXLEVEL Formate geschrieben
/*N*/   // werden, wir kennen aber maximal MAXLEVEL Formate davon
/*N*/   BYTE nKnownFmt = nFmt > MAXLEVEL ? MAXLEVEL : nFmt;
/*N*/   BYTE nRead = 0;
/*N*/
        int i=0;
/*N*/   for( i = 0; i < nKnownFmt; i++ )
/*N*/   {
/*N*/       BYTE nFmtLvl;
/*N*/       *pStrm >> nFmtLvl;
/*N*/       nRead++;
/*N*/
/*N*/       if( nFmtLvl < MAXLEVEL  )
/*N*/       {
/*N*/           cFmt[ i ] = nFmtLvl;
/*N*/       }
/*N*/       else
/*N*/       {
/*N*/           // wenn das Format zu einer Ebene gehoert, die wir nicht kennen
/*N*/           // muessen wir dieses und alle Formate hiernach ignorieren.
/*N*/           nKnownFmt = i;
/*N*/           break;
/*N*/       }
/*N*/   }
/*N*/
/*N*/   // noch nicht gelesene Format-Nummern ueberlesen
/*N*/   for( i=nRead; i<nFmt; i++ )
/*N*/   {
/*N*/       BYTE nFmtLvl;
/*?*/       *pStrm >> nFmtLvl;
/*N*/   }
/*N*/
/*N*/   for( i = 0; Good() && i < nKnownFmt; i++ )
/*N*/   {
/*N*/       SwNumFmt aFmt;
/*N*/       InNumFmt( aFmt );
/*N*/       if( SWG_OUTLINE == cType && !IsVersion(SWG_LONGIDX) )
/*N*/       {
/*N*/           // In der 3.1/4.0 wurden diese Werte ignoriert
/*N*/           aFmt.SetAbsLSpace( 0U );
/*N*/           aFmt.SetFirstLineOffset( 0 );
/*N*/       }
/*N*/
/*N*/       pRule->Set( (USHORT) cFmt[ i ], aFmt );
/*N*/   }
/*N*/
/*N*/   // falls es nicht unterstuetzte Formate koennte man sie ueberlesen.
/*N*/   // Da danach aber nicht sinnvolles kommt, lassen wir das erstmal
/*N*/
/*N*/ #ifdef NUM_RELSPACE
/*N*/   if( SWG_OUTLINE == cType && !IsVersion(SWG_NUMRELSPACE) )
/*N*/   {
/*N*/       // In 3.1-/4.0-Doks muss die Outline-Numerierung noch an
/*N*/       // die Vorlagen angepasst werden und anschliessend noch das
/*N*/       // LRSpace-Item in der Vorlage geloescht werden.
/*N*/
/*N*/       const SwTxtFmtColls *pColls = pDoc->GetTxtFmtColls();
/*N*/       USHORT nArrLen = pColls->Count();
/*N*/       for( USHORT ii=0; ii<nArrLen; ii++ )
/*N*/       {
/*N*/           SwTxtFmtColl* pColl = (*pColls)[ii];
/*N*/           BYTE nLevel = pColl->GetOutlineLevel();
/*N*/           if( NO_NUMBERING != nLevel )
/*N*/           {
/*N*/               nLevel = GetRealLevel( nLevel );
/*N*/               const SvxLRSpaceItem& rLRSpace = pColl->GetLRSpace();
/*N*/               sal_Int32 nOldLSpace = rLRSpace.GetTxtLeft();
/*N*/               const SwNumFmt& rNumFmt = pRule->Get( nLevel );
/*N*/               if( IsVersion(SWG_NEWFIELDS) || rLRSpace.GetPropLeft() == 100U )
/*N*/               {
/*N*/                   // absoluter linker Absatz-Einzug oder 5.0-Dok (dort
/*N*/                   // wurde der relative Wert nicht beachtet)
/*N*/
/*N*/                   // In 3.1- und 4.0-Dokumenten den linken und
/*N*/                   // Ertzeilen-Einzug in die NumRule uebernehmen. In
/*N*/                   // 5.0-Dokumenten steht er dort schon.
/*N*/                   if( !IsVersion(SWG_NEWFIELDS) &&
/*N*/                       (rNumFmt.GetAbsLSpace() != rLRSpace.GetTxtLeft() ||
/*N*/                        rNumFmt.GetFirstLineOffset() !=
/*N*/                                       rLRSpace.GetTxtFirstLineOfst()) )
/*N*/                   {
/*N*/                       SwNumFmt aNumFmt( rNumFmt );
/*N*/                       aNumFmt.SetAbsLSpace( (USHORT)rLRSpace.GetTxtLeft() );
/*N*/                       aNumFmt.SetFirstLineOffset(
/*N*/                           rLRSpace.GetTxtFirstLineOfst() );
/*N*/                       pRule->Set( nLevel, aNumFmt );
/*N*/                   }
/*N*/
/*N*/                   // Den linken-Einzug in der Vorlage auf 0 setzen, damit
/*N*/                   // er nicht doppelt gezaehlt wird. Wenn das
/*N*/                   SvxLRSpaceItem aLRSpace( rLRSpace );
/*N*/                   aLRSpace.SetTxtFirstLineOfst( 0 );
/*N*/                   aLRSpace.SetTxtLeft( 0U );
/*N*/                   SwFmt *pParFmt = pColl->DerivedFrom();
/*N*/                   if( pParFmt && pParFmt->GetLRSpace() == aLRSpace )
/*N*/                       pColl->ResetAttr( RES_LR_SPACE );
/*N*/                   else if( aLRSpace != rLRSpace )
/*N*/                       pColl->SetAttr( aLRSpace );
/*N*/                   if( nOldLSpace != 0 )
/*N*/                   {
/*N*/                       const SfxPoolItem* pItem;
/*N*/                       if( SFX_ITEM_SET == pColl->GetAttrSet().GetItemState(
/*N*/                                           RES_PARATR_TABSTOP, TRUE, &pItem ))
/*N*/                       {
/*N*/                           SvxTabStopItem aTStop( *(SvxTabStopItem*)pItem );
/*N*/                           lcl_sw3io__ConvertNumTabStop( aTStop, nOldLSpace );
/*N*/                           pColl->SetAttr( aTStop );
/*N*/                       }
/*N*/                   }
/*N*/               }
/*N*/               else
/*N*/               {
/*N*/                   // relativer linker Absatz-Einzug in 3.1- oder 4.0-Dok
/*N*/
/*N*/                   // Die Vorlage nicht aendern sondern stattdessen
/*N*/                   // den linken Abstand und den Erstzeilen-Einzug
/*N*/                   // in der NumRule auf 0 setzen. Da der Erstzeilen-Einzug
/*N*/                   // der Vorlage nicht ausgewertet wird, geht er verloren.
/*?*/                   if( rNumFmt.GetAbsLSpace() != 0U ||
/*?*/                       rNumFmt.GetFirstLineOffset() != 0 )
/*?*/                   {
/*?*/                       SwNumFmt aNumFmt( rNumFmt );
/*?*/                       aNumFmt.SetAbsLSpace( 0U );
/*?*/                       aNumFmt.SetFirstLineOffset( 0 );
/*?*/                       pRule->Set( nLevel, aNumFmt );
/*?*/                   }
/*N*/               }
/*N*/           }
/*N*/       }
/*N*/   }
/*N*/ #else
/*N*/   if( SWG_OUTLINE == cType && !IsVersion(SWG_NEWFIELDS) )
/*N*/   {
/*N*/       // In 3.1-/4.0-Doks muss die Outline-Numerierung noch an
/*N*/       // die Vorlagen angepasst werden und anschliessend noch das
/*N*/       // LRSpace-Item in der Vorlage gloescht werden.
/*N*/
/*N*/       const SwTxtFmtColls *pColls = pDoc->GetTxtFmtColls();
/*N*/       USHORT nArrLen = pColls->Count();
/*N*/       for( USHORT i=0; i<nArrLen; i++ )
/*N*/       {
/*N*/           SwTxtFmtColl* pColl = (*pColls)[i];
/*N*/           BYTE nLevel = pColl->GetOutlineLevel();
/*N*/           if( NO_NUMBERING != nLevel &&
/*N*/               GetRealLevel(nLevel) < MAXLEVEL )
/*N*/           {
/*N*/               const SwNumFmt& rFmt = pRule->Get( GetRealLevel(nLevel) );
/*N*/               const SvxLRSpaceItem& rLRSpace = pColl->GetLRSpace();
/*N*/               if( rFmt.GetAbsLSpace() != rLRSpace.GetTxtLeft() ||
/*N*/                   rFmt.GetFirstLineOffset() != rLRSpace.GetTxtFirstLineOfst())
/*N*/               {
/*N*/                   SwNumFmt aFmt( rFmt );
/*N*/                   aFmt.SetAbsLSpace( rLRSpace.GetTxtLeft() );
/*N*/                   aFmt.SetFirstLineOffset( rLRSpace.GetTxtFirstLineOfst() );
/*N*/                   pRule->Set( nLevel, aFmt );
/*N*/               }
/*N*/           }
/*N*/       }
/*N*/   }
/*N*/ #endif
/*N*/
/*N*/   CloseRec( cType );
/*N*/   if( !Good() )
/*?*/       delete pRule, pRule = NULL;
/*N*/   return pRule;
/*N*/ }

/*N*/ void Sw3IoImp::InOutlineExt()
/*N*/ {
/*N*/   OpenRec( SWG_OUTLINEEXT );
/*N*/
/*N*/   BYTE nFmts;
/*N*/   OpenFlagRec();
/*N*/   *pStrm >> nFmts;
/*N*/   CloseFlagRec();
/*N*/
/*N*/   const SwNumRule *pOutline = pDoc->GetOutlineNumRule();
/*N*/   OSL_ENSURE( pOutline, "Wo ist die Outline-NumRule?" );
/*N*/
/*N*/   // Hier stehen jetzt die Original-linken-Abstaende der Outline-NumRule.
/*N*/   for( BYTE i=0; i<nFmts; i++ )
/*N*/   {
/*N*/       BYTE nLevel;
/*N*/       UINT16 nAbsLSpace;
/*N*/
/*N*/       *pStrm >> nLevel >> nAbsLSpace;
/*N*/
/*N*/       if( pOutline && nLevel < MAXLEVEL )
/*N*/       {
/*N*/           OSL_ENSURE( pOutline->GetNumFmt(nLevel), "Format nicht gesetzt?" );
/*N*/           short nFirstLineOffset = pOutline->Get(nLevel).GetFirstLineOffset();
/*N*/           pDoc->SetOutlineLSpace( nLevel, nFirstLineOffset, nAbsLSpace );
/*N*/       }
/*N*/   }
/*N*/
/*N*/   CloseRec( SWG_OUTLINEEXT );
/*N*/ }

// Numerierungs-Regelwerk einlesen

void lcl_sw3io__copyNumRule( const SwNumRule& rSrc, SwNumRule& rDst )
{
    rDst.SetRuleType( rSrc.GetRuleType() );
    rDst.SetPoolHelpId( rSrc.GetPoolHelpId() );
    rDst.SetPoolHlpFileId( rSrc.GetPoolHlpFileId() );
    rDst.SetContinusNum( rSrc.IsContinusNum() );

    OSL_ENSURE( rDst.GetPoolFmtId() == rSrc.GetPoolFmtId(),
            "NumRule-PoolIds sind unterschiedlich" );
    OSL_ENSURE( rDst.IsAutoRule() == rSrc.IsAutoRule(),
            "NumRule-Auto-Flags sind unterschiedlich" );
    OSL_ENSURE( rDst.GetName() == rSrc.GetName(),
            "NumRule-Namen sind unterschiedlich" );

    rDst.SetInvalidRule( TRUE );
}


/*N*/ void Sw3IoImp::InNumRules()
/*N*/ {
/*N*/   InHeader( TRUE );
/*N*/   if( !Good() )
/*N*/       return;
/*N*/
/*N*/   InStringPool( SWG_STRINGPOOL, aStringPool );
/*N*/
/*N*/   SwNumRule* pRule;
/*N*/   BOOL bDone = FALSE;
/*N*/   while( !bDone )
/*N*/   {
/*N*/       BYTE cType = Peek();
/*N*/       if( !Good() || pStrm->IsEof() )
/*N*/           bDone = TRUE;
/*N*/       else switch( cType )
/*N*/       {
/*N*/           case SWG_EOF:
/*N*/               bDone = TRUE;
/*N*/               break;
/*N*/
/*N*/           case SWG_NUMRULE:
/*N*/               pRule = InNumRule( cType );
/*N*/               if( pRule )
/*N*/               {
/*N*/                   BOOL bInsertRule = FALSE;
/*N*/                   if( pRule->IsAutoRule() )
/*N*/                   {
/*N*/                       // Automatische Rules werden nur beim normalen
/*N*/                       // Laden und Laden von Seitenvorlagen eingelesen,
/*N*/                       // aber nicht, wenn nur Num-Vorl. gelesen werden.
/*N*/                       if( bNormal || bPageDescs )
/*N*/                       {
/*N*/                           bInsertRule = TRUE;
/*N*/
/*N*/                           // Auto-NumRule ggf. umbenennen, wenn es sie
/*N*/                           // schon gibt und ausserdem den Namen merken,
/*N*/                           // damit unbenutzte Rules entfernt werden
/*N*/                           // koennen.
/*N*/                           String aOldName( pRule->GetName() );
/*N*/                           // Nur wenn Vorlagen geladen werden (!bNormal)
/*N*/                           // oder im Einfuege-Modus (bInsert) kann es die
/*N*/                           // NumRule schon geben.
/*N*/                           if( !bNormal || bInsert )
/*N*/                           {
/*N*/                               pRule->SetName(
/*?*/                                   pDoc->GetUniqueNumRuleName( &aOldName ) );
/*N*/                           }
/*N*/                           aNumRuleInfos.Insert(
/*N*/                               new Sw3NumRuleInfo( aOldName, pRule->GetName() ) );
/*N*/                       }
/*N*/                   }
/*N*/                   else
/*N*/                   {
/*N*/                       // Nicht-automatische Vorlagen werden nur eingefuegt,
/*N*/                       // wenn normale Dokumente oder Num-Vorl. geladen
/*N*/                       // werden, nicht aber, wenn nur Seiten-Vorlagen
/*N*/                       // geladen werden.
/*N*/                       // Beim Einfuegen oder Num-Vorlagen-Laden ohne
/*N*/                       // ueberschreiben die NumRule nur einfuegen, wenn
/*N*/                       // es sie noch nicht gibt. (bAdditive ist TRUE
/*N*/                       // wenn eingefuegt wird) und sonst nur invalidieren.
/*N*/                       if( ( bNormal || bNumRules ) )
/*N*/                       {
/*N*/                           if( bAdditive )
/*N*/                           {
/*?*/                               SwNumRule *pDocRule =
/*?*/                                   pDoc->FindNumRulePtr( pRule->GetName() );
/*?*/                               if( pDocRule )
/*?*/                                   pDocRule->SetInvalidRule( TRUE );
/*?*/                               else
/*?*/                                   bInsertRule = TRUE;
/*N*/                           }
/*N*/                           else if( bNumRules )
/*N*/                           {
/*?*/                               SwNumRule *pDocRule =
/*?*/                                   pDoc->FindNumRulePtr( pRule->GetName() );
/*?*/                               if( pDocRule )
/*?*/                               {
                                        pDoc->ChgNumRuleFmts( *pRule );
/*?*/                                   lcl_sw3io__copyNumRule( *pRule, *pDocRule );
/*?*/                               }
/*?*/                               else
/*?*/                                   bInsertRule = TRUE;
/*N*/                           }
/*N*/                           else
/*N*/                           {
/*N*/                               bInsertRule = TRUE;
/*N*/                           }
/*N*/                       }
/*N*/                   }
/*N*/
/*N*/                   if( bInsertRule )
/*N*/                   {
/*N*/                       OSL_ENSURE( !pDoc->FindNumRulePtr( pRule->GetName() ),
/*N*/                               "NumRule existiert bereits" );
/*N*/                       pDoc->MakeNumRule( pRule->GetName(), pRule );
/*N*/                   }
/*N*/                   delete pRule;
/*N*/               }
/*N*/               break;
/*N*/
/*N*/           case SWG_OUTLINE:
/*N*/               if( bNormal && !bInsert )
/*N*/               {
/*N*/                   pRule = InNumRule( cType );
/*N*/                   if( pRule )
/*N*/                   {
/*N*/                       pDoc->SetOutlineNumRule( *pRule );
/*N*/                   }
/*N*/                   delete pRule;
/*N*/               }
/*N*/               else
/*N*/               {
/*?*/                   SkipRec();
/*N*/               }
/*N*/               break;
/*N*/
/*N*/ #ifdef NUM_RELSPACE
/*N*/           case SWG_OUTLINEEXT:
/*N*/               if( bNormal && !bInsert )
/*N*/                   InOutlineExt();
/*N*/               else
/*?*/                   SkipRec();
/*N*/               break;
/*N*/ #endif
/*N*/ #ifdef TEST_HUGE_DOCS
/*N*/           case SWG_TESTHUGEDOCS:
/*N*/               InHugeRecord();
/*N*/ #endif
/*N*/
/*N*/           default:
/*?*/               SkipRec();
/*N*/               break;
/*N*/       }
/*N*/   }
/*N*/ }

// Absatz-Numerierung einlesen (seit 5.0)

/*N*/ void Sw3IoImp::InNodeNum( SwNodeNum& rNodeNum )
/*N*/ {
/*N*/   OpenRec( SWG_NODENUM );
/*N*/
/*N*/   BYTE nLevel;
/*N*/   UINT16 nSetValue;
/*N*/
/*N*/   // 0x10: Start-Flag ist gesetzt.
/*N*/   // 0x20: Start-Wert ist vorhanden.
/*N*/   BYTE cFlags = OpenFlagRec();
/*N*/
/*N*/   *pStrm  >> nLevel;
/*N*/   if( (cFlags & 0x20) != 0 )
/*N*/       *pStrm  >> nSetValue;
/*N*/
/*N*/   CloseFlagRec();
/*N*/
/*N*/   BYTE nRealLevel = 0;
/*N*/   if( nLevel != NO_NUMBERING )
/*N*/   {
/*N*/       nRealLevel = GetRealLevel(nLevel);
/*N*/       if( nRealLevel >= MAXLEVEL )
/*N*/       {
/*?*/           BYTE cTmp = MAXLEVEL-1;
/*?*/           if( nLevel & NO_NUMLEVEL )
/*?*/               cTmp |= NO_NUMLEVEL;
/*?*/           nLevel = cTmp;
/*N*/       }
/*N*/   }
/*N*/
/*N*/   rNodeNum.SetLevel( nLevel );
/*N*/   rNodeNum.SetStart( (cFlags & 0x10)!=0 );
/*N*/   if( (cFlags & 0x20) != 0 )
/*N*/       rNodeNum.SetSetValue( nSetValue );
/*N*/
/*N*/   if( nLevel != NO_NUMBERING )
/*N*/   {
/*N*/       USHORT *pLevelVal = rNodeNum.GetLevelVal();
/*N*/       UINT16 nVal;
/*N*/       for( BYTE i=0; i<=nRealLevel; i++ )
/*N*/       {
/*N*/           *pStrm >> nVal;
/*N*/           if( i < MAXLEVEL )
/*N*/               pLevelVal[i] = nVal;
/*N*/           else
/*?*/               Warning();
/*N*/       }
/*N*/   }
/*N*/
/*N*/   CloseRec( SWG_NODENUM );
/*N*/ }

// globale Fussnoten-Info einlesen

/*N*/ void Sw3IoImp::InEndNoteInfo( SwEndNoteInfo &rENInf )
/*N*/ {
/*N*/   BYTE cFlags = OpenFlagRec();
/*N*/   BYTE eType;
/*N*/   UINT16 nPageIdx, nCollIdx, nFtnOffset = 0,
/*N*/           nChrIdx = IDX_NO_VALUE, nAnchorChrIdx = IDX_NO_VALUE;
/*N*/   String sPrefix, sSuffix;
/*N*/   *pStrm >> eType >> nPageIdx >> nCollIdx >> nFtnOffset;
/*N*/   if( IsVersion(SWG_HTMLCOLLCHG) )
/*N*/       *pStrm >> nChrIdx;
/*N*/   if( IsVersion( SWG_FTNANCHORFMT ) && ( 0x10 & cFlags ))
/*N*/       *pStrm >> nAnchorChrIdx;
/*N*/   CloseFlagRec();
/*N*/
/*N*/   if( IsVersion(SWG_HTMLCOLLCHG) )
/*N*/   {
/*N*/       InString( *pStrm, sPrefix );
/*N*/       InString( *pStrm, sSuffix );
/*N*/   }
/*N*/
/*N*/   // PageDesc suchen
/*N*/   if( nPageIdx < IDX_SPEC_VALUE )
/*N*/   {
/*N*/       SwPageDesc *pPageDesc = FindPageDesc( nPageIdx );
/*N*/       if( pPageDesc )
/*N*/           rENInf.ChgPageDesc( pPageDesc );
/*N*/   }
/*N*/   if( nChrIdx != IDX_NO_VALUE )
/*N*/   {
/*N*/       SwCharFmt *pChrFmt = (SwCharFmt *)FindFmt( nChrIdx, SWG_CHARFMT );
/*N*/       if( pChrFmt )
/*N*/           rENInf.SetCharFmt( pChrFmt );
/*N*/   }
/*N*/   if( nAnchorChrIdx != IDX_NO_VALUE )
/*N*/   {
/*N*/       SwCharFmt *pChrFmt = (SwCharFmt *)FindFmt( nAnchorChrIdx, SWG_CHARFMT );
/*N*/       if( pChrFmt )
/*N*/           rENInf.SetAnchorCharFmt( pChrFmt );
/*N*/   }
/*N*/   rENInf.aFmt.SetNumberingType(eType);
/*N*/   rENInf.nFtnOffset = nFtnOffset;
/*N*/   rENInf.SetPrefix( sPrefix );
/*N*/   rENInf.SetSuffix( sSuffix );
/*N*/ }

/*N*/ void Sw3IoImp::InEndNoteInfo()
/*N*/ {
/*N*/   OpenRec( SWG_ENDNOTEINFO );
/*N*/   SwEndNoteInfo aENInf( pDoc->GetEndNoteInfo() );
/*N*/   InEndNoteInfo( aENInf );
/*N*/   pDoc->SetEndNoteInfo( aENInf );
/*N*/   CloseRec( SWG_ENDNOTEINFO );
/*N*/ }

/*N*/ void Sw3IoImp::InFtnInfo()
/*N*/ {
/*N*/   if( !IsVersion(SWG_LONGIDX) )
/*N*/   {
/*N*/       InFtnInfo40();
/*N*/       return;
/*N*/   }
/*N*/
/*N*/   OpenRec( SWG_FOOTINFO );
/*N*/   SwFtnInfo aFtn( pDoc->GetFtnInfo() );
/*N*/   InEndNoteInfo( aFtn );
/*N*/
/*N*/   OpenFlagRec();
/*N*/   BYTE ePos, eNum;
/*N*/   *pStrm >> ePos
/*N*/          >> eNum;
/*N*/   CloseFlagRec();
/*N*/
/*N*/   InString( *pStrm, aFtn.aQuoVadis );
/*N*/   InString( *pStrm, aFtn.aErgoSum );
/*N*/
/*N*/   aFtn.ePos = (SwFtnPos) ePos;
/*N*/   aFtn.eNum = (SwFtnNum) eNum;
/*N*/
/*N*/   //#91003# don't allow counting of footnotes by chapter and end-of-document position
/*N*/   if(FTNPOS_CHAPTER == aFtn.ePos && FTNNUM_CHAPTER == aFtn.eNum)
/*N*/         aFtn.eNum = FTNNUM_DOC;
/*N*/   pDoc->SetFtnInfo( aFtn );
/*N*/   CloseRec( SWG_FOOTINFO );
/*N*/ }

/*N*/ void Sw3IoImp::InFtnInfo40()
/*N*/ {
/*N*/   OpenRec( SWG_FOOTINFO );
/*N*/   SwFtnInfo aFtn( pDoc->GetFtnInfo() );
/*N*/   InString( *pStrm, aFtn.aQuoVadis );
/*N*/     InString( *pStrm, aFtn.aErgoSum );
/*N*/   OpenFlagRec();
/*N*/   BYTE ePos, eNum, eType;
/*N*/   UINT16 nPageIdx, nCollIdx, nFtnOffset = 0;
/*N*/   *pStrm >> ePos >> eNum >> eType >> nPageIdx >> nCollIdx;
/*N*/   if( nVersion>SWG_DELETEOLE )
/*N*/       *pStrm >> nFtnOffset;
/*N*/   CloseFlagRec();
/*N*/   // PageDesc suchen
/*N*/   if( nPageIdx < IDX_SPEC_VALUE )
/*N*/   {
/*N*/       SwPageDesc *pPageDesc = FindPageDesc( nPageIdx );
/*N*/       if( pPageDesc )
/*N*/           aFtn.ChgPageDesc( pPageDesc );
/*N*/   }
/*N*/   aFtn.ePos = (SwFtnPos) ePos;
/*N*/   aFtn.eNum = (SwFtnNum) eNum;
/*N*/   aFtn.aFmt.SetNumberingType(eType);
/*N*/   aFtn.nFtnOffset = nFtnOffset;
/*N*/   pDoc->SetFtnInfo( aFtn );
/*N*/   CloseRec( SWG_FOOTINFO );
/*N*/ }

// Oeffnen eines Numerierungsregel-Bereichs

/*N*/ void Sw3IoImp::OpenNumRange40( const SwNodeIndex& rPos )
/*N*/ {
/*N*/   CloseNumRange40( rPos );
/*N*/   pCurNumRange = new SwPaM( rPos );
/*N*/   pCurNumRange->SetMark();
/*N*/   pCurNumRule = InNumRule( SWG_NUMRULE );
/*N*/ }

// Schliessen eines Numerierungsregel-Bereichs
// Der Index zeigt auf den Node HINTER dem letzten zu numerierenden Node!


/*N*/ void Sw3IoImp::CloseNumRange40( const SwNodeIndex& rPos )
/*N*/ {
/*N*/   if( pCurNumRange )
/*N*/   {
/*N*/       SwNodeIndex& rNodePos = pCurNumRange->GetPoint()->nNode;
/*N*/       rNodePos = rPos.GetIndex() - 1;
/*N*/       SwCntntNode* pCntNd =
/*N*/           pDoc->GetNodes()[ rNodePos ]->GetCntntNode();
/*N*/       if( !pCntNd )
/*?*/           pCntNd = pDoc->GetNodes().GoPrevious( &rNodePos );
/*N*/       pCurNumRange->GetPoint()->nContent.Assign( pCntNd, 0 );
/*N*/       pCurNumRange->GetMark()->nContent.Assign
/*N*/           ( pCurNumRange->GetMark()->nNode.GetNode().GetCntntNode(), 0 );
/*N*/       pDoc->SetNumRule( *pCurNumRange, *pCurNumRule );
/*N*/
/*N*/       // Es wurde ein NO_NUM in der aktuellen Numerierung eingelesen.
/*N*/       // Das muss jetzt noch in ein NO_NUMLEVEL umgewandelt werden.
/*N*/       BYTE nPrevLevel = 0;
/*N*/       for( ULONG n = pCurNumRange->GetMark()->nNode.GetIndex(),
/*N*/           nEnd = rPos.GetIndex(); n < nEnd; ++n )
/*N*/       {
/*N*/           SwTxtNode* pTxtNd = pDoc->GetNodes()[n]->GetTxtNode();
/*N*/           if( pTxtNd && pTxtNd->GetNum() )
/*N*/           {
/*N*/               SwNodeNum* pNum = (SwNodeNum*)pTxtNd->GetNum();
/*N*/               if( bConvertNoNum )
/*N*/               {
/*?*/                   if( NO_NUM == pNum->GetLevel() )
/*?*/                       pNum->SetLevel( nPrevLevel | NO_NUMLEVEL );
/*?*/                   else
/*?*/                       nPrevLevel = GetRealLevel( pNum->GetLevel() );
/*N*/               }
/*N*/ #ifdef NUM_RELSPACE
/*N*/               lcl_sw3io__ConvertNumLRSpace( *pTxtNd, *pCurNumRule,
/*N*/                                             pNum->GetLevel(), TRUE );
/*N*/ #endif
/*N*/           }
/*N*/       }
/*N*/
/*N*/       delete pCurNumRange, pCurNumRange = 0;
/*N*/       delete pCurNumRule, pCurNumRule = 0;
/*N*/       bConvertNoNum = FALSE;
/*N*/   }
/*N*/ }

/*N*/ void lcl_sw3io__ConvertNumTabStop( SvxTabStopItem& rTStop, long nOffset )
/*N*/ {
/*N*/   for( USHORT n = 0; n < rTStop.Count(); ++n )
/*N*/   {
/*N*/       SvxTabStop& rTab = (SvxTabStop&)rTStop[ n ];
/*N*/       if( SVX_TAB_ADJUST_DEFAULT != rTab.GetAdjustment() &&
/*N*/           rTab.GetTabPos() )
/*N*/       {
/*?*/           rTab.GetTabPos() += nOffset;
/*N*/       }
/*N*/   }
/*N*/ }

/*N*/ void lcl_sw3io__ConvertNumTabStop( SwTxtNode& rTxtNd, long nOffset,
/*N*/                                  BOOL bDeep )
/*N*/ {
/*N*/   const SfxPoolItem* pItem;
/*N*/   if( SFX_ITEM_SET == rTxtNd.GetSwAttrSet().GetItemState( RES_PARATR_TABSTOP,
/*N*/                                                           bDeep, &pItem ) )
/*N*/   {
/*N*/       SvxTabStopItem aTStop( *(SvxTabStopItem*)pItem );
/*N*/       lcl_sw3io__ConvertNumTabStop( aTStop, nOffset );
/*N*/       rTxtNd.SwCntntNode::SetAttr( aTStop );
/*N*/   }
/*N*/ }

/*N*/ void lcl_sw3io__ConvertNumLRSpace( SwTxtNode& rTxtNd, const SwNumRule& rNumRule,
/*N*/                                  BYTE nLevel, BOOL bTabStop )
/*N*/ {
/*N*/   if( nLevel == NO_NUMBERING )
/*N*/       return;
/*N*/
/*N*/   nLevel = GetRealLevel( nLevel );
/*N*/   USHORT nNumLSpace = rNumRule.Get( nLevel ).GetAbsLSpace();
/*N*/
/*N*/   // Wenn im Absatz der Einzug eingestellt ist, dann will
/*N*/   // man den wieder Erreichen, sonst den der NumRule.
/*N*/   const SfxPoolItem *pItem;
/*N*/   const SvxLRSpaceItem *pParaLRSpace = 0;
/*N*/   const SwAttrSet *pAttrSet = rTxtNd.GetpSwAttrSet();
/*N*/   if( pAttrSet &&
/*N*/       SFX_ITEM_SET == pAttrSet->GetItemState( RES_LR_SPACE, FALSE, &pItem ) )
/*N*/       pParaLRSpace = (const SvxLRSpaceItem *)pItem;
/*N*/   USHORT nWishLSpace = pParaLRSpace ? pParaLRSpace->GetTxtLeft() : nNumLSpace;
/*N*/
/*N*/   // Dazu muss man den folgenden Wert im Absatz einstellen
/*N*/   const SvxLRSpaceItem& rCollLRSpace = rTxtNd.GetAnyFmtColl().GetLRSpace();
/*N*/   USHORT nOldLSpace = pParaLRSpace ? pParaLRSpace->GetTxtLeft()
/*N*/                                    : rCollLRSpace.GetTxtLeft();
/*N*/   USHORT nNewLSpace;
/*N*/   if( rNumRule.IsAbsSpaces() )
/*?*/       nNewLSpace = rCollLRSpace.GetTxtLeft();
/*N*/   else
/*N*/       nNewLSpace = nWishLSpace > nNumLSpace ? nWishLSpace - nNumLSpace : 0U;
/*N*/
/*N*/   // Wenn der neue Wert zufaellig der der Vorlage ist und der
/*N*/   // rechte Einzug auch mit dem der Vorlage ueberseinstimmt,
/*N*/   // dann braucht bzw. darf man das Absatz-Attribut nicht
/*N*/   // setzen, sonst muss man es setzen.
/*N*/   if( nNewLSpace == rCollLRSpace.GetTxtLeft() &&
/*N*/       (!pParaLRSpace || pParaLRSpace->GetRight() == rCollLRSpace.GetRight()) )
/*N*/   {
/*N*/       if( pParaLRSpace )
/*N*/           rTxtNd.ResetAttr( RES_LR_SPACE );
/*N*/   }
/*N*/   else
/*N*/   {
/*N*/       if( !pParaLRSpace )
/*N*/           pParaLRSpace = &rCollLRSpace;
/*N*/       SvxLRSpaceItem aLRSpace( *pParaLRSpace );
/*N*/       short nFirst = aLRSpace.GetTxtFirstLineOfst();
/*N*/       if( nFirst < 0 && (USHORT)-nFirst > nNewLSpace )
/*N*/           aLRSpace.SetTxtFirstLineOfst( -(short)nNewLSpace );
/*N*/       aLRSpace.SetTxtLeft( nNewLSpace );
/*N*/       ((SwCntntNode&)rTxtNd).SetAttr( aLRSpace );
/*N*/   }
/*N*/
/*N*/   if( bTabStop && nOldLSpace != nNewLSpace )
/*N*/       lcl_sw3io__ConvertNumTabStop( rTxtNd,
/*N*/                                     (long)nOldLSpace - (long)nNewLSpace,
/*N*/                                     TRUE );
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
