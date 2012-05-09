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

#include <stdlib.h>
#include <stdio.h>


#include <paratr.hxx>

#include <horiornt.hxx>

#include <doc.hxx>
#include <fmturl.hxx>
#include <fmtlsplt.hxx>
#include <fmteiro.hxx>
#include <hfspacingitem.hxx>
#include <fmthbsh.hxx>
#include <fmtsrnd.hxx>
#include <fmtornt.hxx>
#include <fmtfsize.hxx>
#include <fmtfordr.hxx>
#include <fmtclds.hxx>
#include <poolfmt.hxx>
#include <fmtline.hxx>
#include <sw3imp.hxx>
#include <charfmt.hxx>
#include <grfatr.hxx>
#include <cellatr.hxx>
#include <flyfrm.hxx>
#include <fmtftntx.hxx>
#include <fmtclbl.hxx>
#include <SwStyleNameMapper.hxx>
#include <bf_tools/string.hxx>
namespace binfilter {

////////////////////////////// Frame-Attribute ////////////////////////////


/*N*/ USHORT SwFmtSurround::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/   OSL_ENSURE( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/           "SwFmtSurround: Gibt es ein neues Fileformat?" );
/*N*/   return SOFFICE_FILEFORMAT_31==nFFVer ? 0 :
/*N*/          (SOFFICE_FILEFORMAT_40==nFFVer ? 4 : 5);
/*N*/ }


/*N*/ SfxPoolItem* SwFmtSurround::Create( SvStream& rStrm, USHORT nVrs ) const
/*N*/ {
/*N*/   BYTE nType, bGold = 0, bAnch=0, bCont=0, bOutside1 = 0;
/*N*/   rStrm >> nType;
/*N*/   if ( nVrs < 5 )
/*N*/       rStrm >> bGold;
/*N*/   if ( nVrs > 1 )
/*N*/       rStrm >> bAnch;
/*N*/   if ( nVrs > 2 )
/*N*/       rStrm >> bCont;
/*N*/   if ( nVrs > 3 )
/*N*/       rStrm >> bOutside1;
/*N*/
/*N*/   SwFmtSurround *pRet = new SwFmtSurround( (SwSurround) nType );
/*N*/   if( bGold && SURROUND_NONE != (SwSurround)nType &&
/*N*/                SURROUND_THROUGHT != (SwSurround)nType )
/*N*/       pRet->SetSurround( SURROUND_IDEAL );
/*N*/   pRet->SetAnchorOnly( BOOL(bAnch) );
/*N*/   pRet->SetContour( BOOL(bCont) );
/*N*/   pRet->SetOutside( BOOL(bOutside1) );
/*N*/   return pRet;
/*N*/ }

/*N*/ USHORT SwFmtVertOrient::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/   OSL_ENSURE( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/           "SwFmtHoriOrient: Gibt es ein neues Fileformat?" );
/*N*/   return ( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/            SOFFICE_FILEFORMAT_40==nFFVer ) ? 0 : IVER_VERTORIENT_REL;
/*N*/ }

/*N*/ SfxPoolItem* SwFmtVertOrient::Create( SvStream& rStrm, USHORT nIVer ) const
/*N*/ {
/*N*/   sal_Int32 nPos(0);
/*N*/   BYTE nOrient(0), nRelation(0);
/*N*/   rStrm >> nPos >> nOrient >> nRelation;
/*N*/
/*N*/   // fix #48690#: In 4.0-Doks wurde bei VERT_NONE die Relation nicht
/*N*/   // beachtet, aber wie FRAME behandelt. Das Attribut enthielt aber PRTAREA
/*N*/   if( VERT_NONE == (SwVertOrient)nOrient && nIVer < IVER_VERTORIENT_REL )
/*N*/       nRelation = FRAME;
/*N*/
/*N*/   return new SwFmtVertOrient( (SwTwips) nPos, (SwVertOrient) nOrient,
/*N*/                               (SwRelationOrient) nRelation );
/*N*/ }

/*N*/ USHORT SwFmtHoriOrient::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/   OSL_ENSURE( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/           "SwFmtHoriOrient: Gibt es ein neues Fileformat?" );
/*N*/   return ( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/            SOFFICE_FILEFORMAT_40==nFFVer ) ? 0 : IVER_HORIORIENT_REL;
/*N*/ }

/*N*/ SfxPoolItem* SwFmtHoriOrient::Create( SvStream& rStrm, USHORT nIVer ) const
/*N*/ {
/*N*/   sal_Int32 nPos(0);
/*N*/   BYTE nOrient(0), nRelation(0), bToggle(0);
/*N*/   rStrm >> nPos >> nOrient >> nRelation;
/*N*/
/*N*/   if( nIVer >= IVER_HORIORIENT_TOGGLE )
/*N*/       rStrm >> bToggle;
/*N*/
/*N*/   // fix #48690#: In 4.0-Doks wurde bei HORI_NONE die Relation nicht
/*N*/   // beachtet, aber wie FRAME behandelt. Das Attribut enthielt aber PRTAREA
/*N*/   if( HORI_NONE == (SwHoriOrient)nOrient && nIVer < IVER_HORIORIENT_REL )
/*N*/       nRelation = FRAME;
/*N*/
/*N*/   return new SwFmtHoriOrient
/*N*/       ( (SwTwips) nPos, (SwHoriOrient) nOrient, (SwRelationOrient) nRelation,
/*N*/         BOOL( bToggle ) );
/*N*/ }

/*N*/ USHORT SwFmtFrmSize::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/   OSL_ENSURE( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/           "SwFmtFrmSize: Gibt es ein neues Fileformat?" );
/*N*/   return SOFFICE_FILEFORMAT_31==nFFVer ? 0 : 2;
/*N*/ }


/*N*/ SfxPoolItem* SwFmtFrmSize::Create( SvStream& rStrm, USHORT nVersion ) const
/*N*/ {
/*N*/   BYTE nSizeType, nWidthPercent1 = 0, nHeightPercent1 = 0;
/*N*/   INT32 nWidth, nHeight;
/*N*/   rStrm >> nSizeType >> nWidth >> nHeight;
/*N*/
/*N*/   if ( nVersion > 1 )
/*N*/       rStrm >> nWidthPercent1 >> nHeightPercent1;
/*N*/
/*N*/   SwFmtFrmSize *pRet = new SwFmtFrmSize( (SwFrmSize) nSizeType, nWidth, nHeight );
/*N*/   pRet->SetWidthPercent ( nWidthPercent1 );
/*N*/   pRet->SetHeightPercent( nHeightPercent1 );
/*N*/   return pRet;
/*N*/ }

/*N*/ SfxPoolItem* SwFmtFillOrder::Create( SvStream& rStrm, USHORT ) const
/*N*/ {
/*N*/   BYTE nFillOrder;
/*N*/   rStrm >> nFillOrder;
/*N*/   return new SwFmtFillOrder( (SwFillOrder) nFillOrder );
/*N*/ }
/*N*/
/*N*/

/*N*/ SfxPoolItem* SwFmtCol::Create( SvStream& rStrm, USHORT ) const
/*N*/ {
/*N*/   // Die Longs muessen runtergerechnet werden
/*N*/   BYTE   nLineAdj, bOrtho1, nLineHeight1, nPenStyle;
/*N*/   INT16  nGutterWidth, nPenWidth;
/*N*/   UINT16 nWishWidth, nPenRed, nPenGreen, nPenBlue;
/*N*/   rStrm >> nLineAdj
/*N*/         >> bOrtho1
/*N*/         >> nLineHeight1
/*N*/         >> nGutterWidth
/*N*/         >> nWishWidth
/*N*/         >> nPenStyle
/*N*/         >> nPenWidth
/*N*/         >> nPenRed
/*N*/         >> nPenGreen
/*N*/         >> nPenBlue;
/*N*/   SwFmtCol* p = new SwFmtCol;
/*N*/   Color aPenColor( nPenRed >> 8, nPenGreen >> 8, nPenBlue >> 8 );
/*N*/ //    Pen aPen( aPenColor, nPenWidth, (PenStyle) nPenStyle );
/*N*/   INT16 nCol;
/*N*/   rStrm >> nCol;
/*N*/   OSL_ENSURE( nWishWidth, "Damaged Doc: No WishWidth" );
/*N*/   if( !nWishWidth )
/*N*/   {
/*?*/       nWishWidth = USHRT_MAX;
/*?*/       if( nCol )
/*?*/           p->Init( nCol, nGutterWidth, nWishWidth );
/*N*/   }
/*N*/   else
/*N*/   {
/*N*/       for( short i = 0; i < nCol; i++ )
/*N*/       {
/*N*/           UINT16 nWidth1, nLeft, nUpper, nRight, nLower;
/*N*/           rStrm >> nWidth1 >> nLeft >> nUpper >> nRight >> nLower;
/*N*/           SwColumn* pCol = new SwColumn;
/*N*/           pCol->SetWishWidth( nWidth1 );
/*N*/           pCol->SetLeft( nLeft );
/*N*/           pCol->SetUpper( nUpper );
/*N*/           pCol->SetRight( nRight );
/*N*/           pCol->SetLower( nLower );
/*N*/           p->GetColumns().Insert( pCol, i );
/*N*/       }
/*N*/   }
/*N*/   p->SetLineWidth( nPenWidth );
/*N*/   p->SetLineColor( aPenColor );
/*N*/   p->SetWishWidth( nWishWidth );
/*N*/   p->SetLineHeight( nLineHeight );
/*N*/   p->SetLineAdj( (SwColLineAdj) nLineAdj );
/*N*/   // temporaerer Bug Fix
/*N*/   if( nCol )
/*N*/       // Wert direkt mit dem Silberhammer einschlagen.
/*N*/       p->_SetOrtho( (BOOL) bOrtho1 );
/*N*/   return p;
/*N*/ }

/*N*/ SfxPoolItem * SwFmtURL::Create(SvStream &rStrm, USHORT nIVer) const
/*N*/ {
/*N*/   Sw3IoImp* pIo = Sw3IoImp::GetCurrentIo();
/*N*/   OSL_ENSURE( pIo, "Reader/Writer not found" );
/*N*/
/*N*/   String sURL1, sTargetFrameName1, sName1;
/*N*/   BOOL bServerMap = FALSE;
/*N*/
/*N*/   SwFmtURL *pNew = new SwFmtURL;
/*N*/
/*N*/   if( pIo )
/*N*/       pNew->SetMap( pIo->InImageMap( sURL1, sTargetFrameName1, bServerMap ) );
/*N*/   if( nIVer>=1 )
/*N*/       sName1 = rStrm.ReadUniOrByteString( rStrm.GetStreamCharSet() );
/*N*/
/*N*/   // fix #30592#: Boese Falle: Beim setzen einer URL wird eine Image-Map
/*N*/   // wieder geloescht.
/*N*/   if( sURL1.Len() )
/*N*/       pNew->SetURL( sURL1, bServerMap );
/*N*/   pNew->SetTargetFrameName( sTargetFrameName1 );
/*N*/   pNew->SetName( sName1 );
/*N*/   return pNew;
/*N*/ }

/*N*/ USHORT SwFmtURL::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/   OSL_ENSURE( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/           "SwFmtURL: Gibt es ein neues Fileformat?" );
/*N*/   return SOFFICE_FILEFORMAT_31==nFFVer ? USHRT_MAX : 1;
/*N*/ }


SfxPoolItem* SwFmtEditInReadonly::Create(SvStream &rStrm, USHORT) const
{
    BYTE n;
    rStrm >> n;
    return new SwFmtEditInReadonly( RES_EDIT_IN_READONLY, BOOL(n) );
}

/*N*/ USHORT SwFmtEditInReadonly::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/   OSL_ENSURE( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/           "SwFmtEditInReadonly: Gibt es ein neues Fileformat?" );
/*N*/   return SOFFICE_FILEFORMAT_31==nFFVer ? USHRT_MAX : 0;
/*N*/ }


SfxPoolItem* SwFmtLayoutSplit::Create(SvStream &rStrm, USHORT) const
{
    BYTE n;
    rStrm >> n;
    return new SwFmtLayoutSplit( BOOL(n) );
}

/*N*/ USHORT SwFmtLayoutSplit::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/   OSL_ENSURE( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/           "SwFmtLayoutSplit: Gibt es ein neues Fileformat?" );
/*N*/   return SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/          SOFFICE_FILEFORMAT_40==nFFVer ? USHRT_MAX : 0;
/*N*/ }

SfxPoolItem* SwFmtFtnEndAtTxtEnd::Create( SvStream &rStrm, USHORT nVer ) const
{
    SwFmtFtnEndAtTxtEnd* pNew = (SwFmtFtnEndAtTxtEnd*)Clone();

    BYTE nVal;
    rStrm >> nVal;
    pNew->SetValue( nVal );

    if( 0 < nVer )
    {
        UINT16 nOffset1, nFmtType;
        String sPostfix, sSuffix1;
        rStrm >> nOffset1
              >> nFmtType;
        sPostfix = rStrm.ReadUniOrByteString( rStrm.GetStreamCharSet() );
        sSuffix1 = rStrm.ReadUniOrByteString( rStrm.GetStreamCharSet() );

        pNew->SetNumType( (SvxExtNumType)nFmtType );
        pNew->SetOffset( nOffset1 );
        pNew->SetPrefix( sPostfix );
        pNew->SetSuffix( sSuffix1 );
    }
    return pNew;
}

/*N*/ USHORT SwFmtFtnEndAtTxtEnd::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/   OSL_ENSURE( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/           "SwFmtFtnEndAtTxtEnd: Gibt es ein neues Fileformat?" );
/*N*/   return SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/          SOFFICE_FILEFORMAT_40==nFFVer ? USHRT_MAX : 1;
/*N*/ }

SfxPoolItem* SwFmtNoBalancedColumns::Create(SvStream &rStrm, USHORT) const
{
    BYTE n;
    rStrm >> n;
    return new SwFmtNoBalancedColumns( BOOL(n) );
}

/*N*/ USHORT SwFmtNoBalancedColumns::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/   OSL_ENSURE( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/           "SwFmtNoBalancedColumns: Gibt es ein neues Fileformat?" );
/*N*/   return SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/          SOFFICE_FILEFORMAT_40==nFFVer ? USHRT_MAX : 0;
/*N*/ }


/*M*/ SfxPoolItem* SwHeaderAndFooterEatSpacingItem::Create(SvStream &rStrm, USHORT) const
/*M*/ {
/*M*/     OSL_FAIL( "SwHeaderAndFooterEatSpacingItem::Create called for old fileformat" );
/*M*/   BYTE n;
/*M*/   rStrm >> n;
/*M*/     return new SwHeaderAndFooterEatSpacingItem( RES_HEADER_FOOTER_EAT_SPACING, BOOL(n) );
/*M*/ }

/*M*/ USHORT SwHeaderAndFooterEatSpacingItem::GetVersion( USHORT nFFVer ) const
/*M*/ {
/*M*/   OSL_ENSURE( SOFFICE_FILEFORMAT_31==nFFVer ||
/*M*/           SOFFICE_FILEFORMAT_40==nFFVer ||
/*M*/           SOFFICE_FILEFORMAT_50==nFFVer,
/*M*/             "SwHeaderAndFooterEatSpacingItem: Gibt es ein neues Fileformat?" );
        (void) nFFVer; // avoid warnings
/*M*/     return USHRT_MAX;
/*M*/ }


/////////////////////////////// Grafik-Attribute /////////////////////////


/*N*/ SfxPoolItem* SwMirrorGrf::Create( SvStream& rStrm, USHORT nIVer) const
/*N*/ {
/*N*/   BYTE nState;
/*N*/   BYTE nToggle = FALSE;
/*N*/
/*N*/   rStrm >> nState;
/*N*/
/*N*/   if (nIVer > 0)
/*N*/       rStrm >> nToggle;
/*N*/
/*N*/   SwMirrorGrf* pRet = new SwMirrorGrf( nState );
/*N*/   pRet->SetGrfToggle((BOOL)nToggle);
/*N*/   return pRet;
/*N*/ }

/*N*/ USHORT SwMirrorGrf::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/   OSL_ENSURE( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/           "SwFmtSurround: Gibt es ein neues Fileformat?" );
/*N*/
/*N*/   return SOFFICE_FILEFORMAT_40 < nFFVer ? 1 : 0;
/*N*/ }


/*N*/ USHORT    SwCropGrf::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/   OSL_ENSURE( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/           "SwCropGrf: Gibt es ein neues Fileformat?" );
        (void) nFFVer; // avoid warnings
/*N*/
/*N*/   return GRFCROP_VERSION_SWDEFAULT;
/*N*/ }

/*N*/ USHORT    SwRotationGrf::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/   OSL_ENSURE( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/           "SwRotationGrf: Gibt es ein neues Fileformat?" );
/*N*/
/*N*/   return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
/*N*/ }

SfxPoolItem* SwRotationGrf::Create( SvStream & rStrm, USHORT nVer ) const
{
    SwRotationGrf* pRet = (SwRotationGrf*)SfxUInt16Item::Create( rStrm, nVer );
    Size aSz;
    rStrm >> aSz;
    pRet->SetUnrotatedSize( aSz );
    return pRet;
}

/*N*/ USHORT SwLuminanceGrf::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/   OSL_ENSURE( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/           "SwLuminanceGrf: Gibt es ein neues Fileformat?" );
/*N*/
/*N*/   return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
/*N*/ }

/*N*/ USHORT SwContrastGrf::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/   OSL_ENSURE( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/           "SwContrastGrf: Gibt es ein neues Fileformat?" );
/*N*/
/*N*/   return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
/*N*/ }
/*N*/ USHORT SwChannelGrf::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/   OSL_ENSURE( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/           "SwChannelGrf: Gibt es ein neues Fileformat?" );
/*N*/
/*N*/   return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
/*N*/ }

/*N*/ USHORT SwGammaGrf::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/   OSL_ENSURE( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/           "SwChannelGrf: Gibt es ein neues Fileformat?" );
/*N*/
/*N*/   return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
/*N*/ }

SfxPoolItem* SwGammaGrf::Create(SvStream & rStrm, USHORT ) const
{
    SwGammaGrf* pRet = (SwGammaGrf*)Clone();
    double aVal;
    rStrm >> aVal;
    pRet->SetValue( aVal );
    return pRet;
}

/*N*/ USHORT SwInvertGrf::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/   OSL_ENSURE( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/           "SwInvertGrf: Gibt es ein neues Fileformat?" );
/*N*/
/*N*/   return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
/*N*/ }

/*N*/ USHORT    SwTransparencyGrf::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/   OSL_ENSURE( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/           "SwInvertGrf: Gibt es ein neues Fileformat?" );
/*N*/
/*N*/   return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
/*N*/ }

/*N*/ USHORT    SwDrawModeGrf::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/   OSL_ENSURE( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/           "SwDrawModeGrf: Gibt es ein neues Fileformat?" );
/*N*/
/*N*/   return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
/*N*/ }



//////////////////////////////// Text-Attribute ////////////////////////////


/*N*/ SfxPoolItem* SwFmtHardBlank::Create( SvStream& rStrm, USHORT nV ) const
/*N*/ {
/*N*/   sal_Char cChr = ' ';
/*N*/   if( nV )
/*N*/       rStrm >> cChr;
/*N*/   sal_Unicode c = ByteString::ConvertToUnicode( cChr, rStrm.GetStreamCharSet() );
/*N*/   return new SwFmtHardBlank( c, FALSE );
/*N*/ }

/*N*/ USHORT SwFmtHardBlank::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/   OSL_ENSURE( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/           "SwFmtHardBlank: Gibt es ein neues Fileformat?" );
/*N*/   return SOFFICE_FILEFORMAT_31==nFFVer ? 0 : 1;
/*N*/ }


/*N*/ SfxPoolItem* SwFmtSoftHyph::Create( SvStream& /*rStrm*/, USHORT ) const
/*N*/ {
/*N*/   return new SwFmtSoftHyph;
/*N*/ }

/////////////////////////////// Absatz-Attribute ///////////////////////////


/*N*/ USHORT SwFmtDrop::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/   OSL_ENSURE( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/           "SwFmtDrop: Gibt es ein neues Fileformat?" );
/*N*/   return ( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/            SOFFICE_FILEFORMAT_40==nFFVer ) ? 0 : DROP_WHOLEWORD;
/*N*/   return 0;
/*N*/ }

/*N*/ SfxPoolItem* SwFmtDrop::Create( SvStream& rStrm, USHORT nVer ) const
/*N*/ {
/*N*/   UINT16 nLines1, nChars1, nDistance1, nX, nY, nFmt;
/*N*/   BYTE bWhole = 0;
/*N*/   rStrm >> nFmt >> nLines1 >> nChars1 >> nDistance1;
/*N*/   if( nVer >= DROP_WHOLEWORD )
/*N*/       rStrm >> bWhole;
/*N*/   else
/*N*/       rStrm >> nX >> nY;
/*N*/   SwFmtDrop* pAttr = new SwFmtDrop;
/*N*/   pAttr->GetLines() = (BYTE) nLines1;
/*N*/   pAttr->GetChars() = (BYTE) nChars1;
/*N*/   pAttr->GetDistance() = nDistance1;
/*N*/   pAttr->GetWholeWord() = (BOOL) bWhole;
/*N*/   if( nFmt != IDX_NO_VALUE )
/*N*/   {
/*N*/       Sw3IoImp* pIo = Sw3IoImp::GetCurrentIo();
/*N*/       if( pIo )
/*N*/       {
/*N*/           if( pIo->aStringPool.Count() )
/*N*/           {
/*N*/               SwCharFmt* pSet = (SwCharFmt*) pIo->FindFmt( nFmt, SWG_CHARFMT );
/*N*/               if( pSet )
/*N*/                   pAttr->SetCharFmt( pSet );
/*N*/           }
/*N*/           else
/*N*/               pAttr->nReadFmt = nFmt;
/*N*/       }
/*N*/   }
/*N*/   return pAttr;
/*N*/ }

/*N*/ SfxPoolItem* SwRegisterItem::Create( SvStream& rStrm, USHORT ) const
/*N*/ {
/*N*/   BYTE bIsRegister;
/*N*/   rStrm >> bIsRegister;
/*N*/   return new SwRegisterItem( BOOL( bIsRegister != 0 ) );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ USHORT SwRegisterItem::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/   OSL_ENSURE( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/           "SwRegisterItem: Gibt es ein neues Fileformat?" );
/*N*/
/*N*/   // im 3.1 FF nicht speichern
/*N*/   return SOFFICE_FILEFORMAT_31==nFFVer ? USHRT_MAX : 0;
/*N*/ }

/*N*/ SfxPoolItem* SwFmtLineNumber::Create( SvStream& rStrm, USHORT ) const
/*N*/ {
/*N*/   SwFmtLineNumber *pTmp = new SwFmtLineNumber;
/*N*/   sal_uInt32 nTmp; BOOL bTmp;
/*N*/   rStrm >> nTmp; pTmp->SetStartValue( nTmp );
/*N*/   rStrm >> bTmp; pTmp->SetCountLines( bTmp );
/*N*/   return pTmp;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ USHORT SwFmtLineNumber::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/   OSL_ENSURE( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/           "SwFmtLineNumber: Gibt es ein neues Fileformat?" );
/*N*/
/*N*/   // vor 5.0 nicht speichern
/*N*/   return SOFFICE_FILEFORMAT_40 <= nFFVer ? 0 : USHRT_MAX;
/*N*/ }

/*N*/ SfxPoolItem* SwNumRuleItem::Create( SvStream& rStrm,
/*N*/                                            USHORT nIVer ) const
/*N*/ {
/*N*/   UINT16 nPoolId;
/*N*/
/*N*/   String sTmp = rStrm.ReadUniOrByteString( rStrm.GetStreamCharSet() );
/*N*/   if( nIVer>0 )
/*N*/   {
/*N*/       // Pool-NumRules muessen evtl. noch umbenannt werden.
/*N*/       rStrm >> nPoolId;
/*N*/
/*N*/       Sw3IoImp* pIo = Sw3IoImp::GetCurrentIo();
/*N*/       if( pIo && nPoolId >= RES_POOLNUMRULE_BEGIN &&
/*N*/           nPoolId < RES_POOLNUMRULE_END )
/*?*/           SwStyleNameMapper::FillUIName( nPoolId, sTmp );
/*N*/   }
/*N*/
/*N*/   return new SwNumRuleItem( sTmp );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ USHORT SwNumRuleItem::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/   OSL_ENSURE( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/           "SwNumRuleItem: Gibt es ein neues Fileformat?" );
/*N*/
/*N*/   // im 3.1 und 4.0 FF nicht speichern
/*N*/   // Die FF-Version 0 enthielt keine PoolId.
/*N*/   return nFFVer <= SOFFICE_FILEFORMAT_40 ? USHRT_MAX : 1;
/*N*/ }

// -----------------------------------------------------------------------


/*N*/ SfxPoolItem* SwTblBoxNumFormat::Create( SvStream& rStrm, USHORT /*nVer*/ ) const
/*N*/ {
/*N*/   BYTE nFlag;
/*N*/   UINT32 nTmp;
/*N*/
/*N*/   rStrm >> nTmp >> nFlag;
/*N*/
/*N*/   Sw3IoImp* pIo = Sw3IoImp::GetCurrentIo();
/*N*/   SvNumberFormatter* pN;
/*N*/   if( pIo && 0 != ( pN = pIo->pDoc->GetNumberFormatter( FALSE ) ) )
/*N*/       nTmp = pN->GetMergeFmtIndex( nTmp );
/*N*/
/*N*/   SwTblBoxNumFormat* pNew = new SwTblBoxNumFormat( nTmp, 0 != nFlag );
/*N*/   return pNew;
/*N*/ }


/*N*/ USHORT SwTblBoxNumFormat::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/   OSL_ENSURE( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/           "SwTblBoxNumFormat: Gibt es ein neues Fileformat?" );
/*N*/   return SOFFICE_FILEFORMAT_31==nFFVer ? USHRT_MAX : 0;
/*N*/ }

// -----------------------------------------------------------------------


/*N*/ SfxPoolItem* SwTblBoxValue::Create( SvStream& rStrm, USHORT nIVer ) const
/*N*/ {
/*N*/   double dVal;
/*N*/   if( 0 == nIVer )
/*N*/   {
/*N*/       sal_Char* dummy;
/*N*/       rtl::OString sValue = read_lenPrefixed_uInt8s_ToOString<sal_uInt16>(rStrm);
/*N*/       dVal = strtod( sValue.getStr(), &dummy );
/*N*/   }
/*N*/   else
/*N*/       rStrm >> dVal;
/*N*/
/*N*/   return new SwTblBoxValue( dVal );
/*N*/ }

/*N*/ USHORT SwTblBoxValue::GetVersion( USHORT nFFVer ) const
/*N*/ {
/*N*/   OSL_ENSURE( SOFFICE_FILEFORMAT_31==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_40==nFFVer ||
/*N*/           SOFFICE_FILEFORMAT_50==nFFVer,
/*N*/           "SwTblBoxValue: Gibt es ein neues Fileformat?" );
/*N*/   return SOFFICE_FILEFORMAT_31==nFFVer ? USHRT_MAX :
/*N*/          (SOFFICE_FILEFORMAT_40==nFFVer ? 0 : 1 );
/*N*/ }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
