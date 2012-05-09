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

#define _ZFORLIST_DECLARE_TABLE
#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER

#include <osl/diagnose.h>
#include <bf_svx/boxitem.hxx>

#include <fmtfsize.hxx>

#include <horiornt.hxx>

#include <doc.hxx>
#include <frmtool.hxx>
#include <swtable.hxx>
#include <ndtxt.hxx>
#include <rowfrm.hxx>
#include <swtblfmt.hxx>
#include <tblrwcl.hxx>
namespace binfilter {

#define COLFUZZY 20
#define ROWFUZZY 10

typedef SwTableLine* SwTableLinePtr;

/*N*/ SV_IMPL_PTRARR( _SwShareBoxFmts, SwShareBoxFmt* )

// fuers setzen der Frame-Formate an den Boxen reicht es, das aktuelle
// im Array zu suchen. Ist es vorhanden, so gebe das neue zurueck




#if defined(DBG_UTIL)

void _CheckBoxWidth( const SwTableLine& rLine, SwTwips nSize )
{
    const SwTableBoxes& rBoxes = rLine.GetTabBoxes();

    SwTwips nAktSize = 0;
    // checke doch mal ob die Tabellen korrekte Breiten haben
    for( USHORT n = 0; n < rBoxes.Count(); ++n  )
    {
        const SwTableBox* pBox = rBoxes[ n ];
        SwTwips nBoxW = pBox->GetFrmFmt()->GetFrmSize().GetWidth();
        nAktSize += nBoxW;

        for( USHORT i = 0; i < pBox->GetTabLines().Count(); ++i )
            _CheckBoxWidth( *pBox->GetTabLines()[ i ], nBoxW );
    }

    if( Abs( nAktSize - nSize ) > ( COLFUZZY * rBoxes.Count() ) )
    {
        OSL_FAIL( "Boxen der Line zu klein/gross" );
    }
}

 #define CHECKBOXWIDTH                                          \
    {                                                           \
        SwTwips nSize = GetFrmFmt()->GetFrmSize().GetWidth();   \
        for( USHORT n = 0; n < aLines.Count(); ++n  )           \
            _CheckBoxWidth( *aLines[ n ], nSize );            \
    }

 #define CHECKTABLELAYOUT                                            \
     {                                                               \
         for ( int i = 0; i < GetTabLines().Count(); ++i )    \
         {                                                           \
             SwFrmFmt* pFmt = GetTabLines()[i]->GetFrmFmt();  \
             SwClientIter aIter( *pFmt );                            \
             SwClient* pLast = aIter.GoStart();                      \
             if( pLast )                                             \
             {                                                       \
                 do                                                  \
                 {                                                   \
                     SwFrm *pFrm = PTR_CAST( SwFrm, pLast );         \
                     if ( pFrm &&                                    \
                          ((SwRowFrm*)pFrm)->GetTabLine() == GetTabLines()[i] ) \
                     {                                               \
                         OSL_ENSURE( pFrm->GetUpper()->IsTabFrm(),       \
                                 "Table layout does not match table structure" );       \
                     }                                               \
                 } while ( 0 != ( pLast = aIter++ ) );               \
             }                                                       \
         }                                                           \
     }

#else

#define CHECKBOXWIDTH
#define CHECKTABLELAYOUT

#endif

//-----------------------------------------------------------



void lcl_LastBoxSetWidth( SwTableBoxes &rBoxes, const long nOffset,
                            BOOL bFirst, SwShareBoxFmts& rShareFmts );

void lcl_LastBoxSetWidthLine( SwTableLines &rLines, const long nOffset,
                                BOOL bFirst, SwShareBoxFmts& rShareFmts )
{
    for ( USHORT i = 0; i < rLines.Count(); ++i )
     lcl_LastBoxSetWidth( rLines[i]->GetTabBoxes(), nOffset, bFirst,
                                rShareFmts );
}

void lcl_LastBoxSetWidth( SwTableBoxes &rBoxes, const long nOffset,
                            BOOL bFirst, SwShareBoxFmts& rShareFmts )
{
    SwTableBox& rBox = *rBoxes[ bFirst ? 0 : rBoxes.Count() - 1 ];
    if( !rBox.GetSttNd() )
     lcl_LastBoxSetWidthLine( rBox.GetTabLines(), nOffset,
                                    bFirst, rShareFmts );

    //Die Box anpassen
    SwFrmFmt *pBoxFmt = rBox.GetFrmFmt();
    SwFmtFrmSize aNew( pBoxFmt->GetFrmSize() );
    aNew.SetWidth( aNew.GetWidth() + nOffset );
    SwFrmFmt *pFmt = rShareFmts.GetFormat( *pBoxFmt, aNew );
    if( pFmt )
        rBox.ChgFrmFmt( (SwTableBoxFmt*)pFmt );
    else
    {
        pFmt = rBox.ClaimFrmFmt();

        pFmt->LockModify();
        pFmt->SetAttr( aNew );
        pFmt->UnlockModify();

        rShareFmts.AddFormat( *pBoxFmt, *pFmt );
    }
}

// ---------------------------------------------------------------

/*
    ----------------------- >> MERGE << ------------------------
     Algorythmus:
        ist in der _FndBox nur eine Line angegeben, nehme die Line
        und teste die Anzahl der Boxen
        - ist mehr als 1 Box angegeben, so wird auf Boxenebene zusammen-
            gefasst, d.H. die neue Box wird so Breit wie die alten.
            - Alle Lines die ueber/unter dem Bereich liegen werden in die
            Box als Line + Box mit Lines eingefuegt
            - Alle Lines die vor/hinter dem Bereich liegen werden in
            die Boxen Left/Right eingetragen

    ----------------------- >> MERGE << ------------------------
*/

void lcl_CpyLines( USHORT nStt, USHORT nEnd,
                                SwTableLines& rLines,
                                SwTableBox* pInsBox,
                                USHORT nPos = USHRT_MAX )
{
    for( USHORT n = nStt; n < nEnd; ++n )
        rLines[n]->SetUpper( pInsBox );
    if( USHRT_MAX == nPos )
        nPos = pInsBox->GetTabLines().Count();
    pInsBox->GetTabLines().Insert( &rLines, nPos, nStt, nEnd );
    rLines.Remove( nStt, nEnd - nStt );
}

 void lcl_CpyBoxes( USHORT nStt, USHORT nEnd,
                                SwTableBoxes& rBoxes,
                                SwTableLine* pInsLine,
                                USHORT nPos = USHRT_MAX )
 {
    for( USHORT n = nStt; n < nEnd; ++n )
        rBoxes[n]->SetUpper( pInsLine );
    if( USHRT_MAX == nPos )
        nPos = pInsLine->GetTabBoxes().Count();
    pInsLine->GetTabBoxes().Insert( &rBoxes, nPos, nStt, nEnd );
    rBoxes.Remove( nStt, nEnd - nStt );
 }

 void lcl_CalcWidth( SwTableBox* pBox )
 {
    // Annahme: jede Line in der Box ist gleich gross
    SwFrmFmt* pFmt = pBox->ClaimFrmFmt();
    OSL_ENSURE( pBox->GetTabLines().Count(), "Box hat keine Lines" );

    SwTableLine* pLine = pBox->GetTabLines()[0];
    OSL_ENSURE( pLine, "Box steht in keiner Line" );

    long nWidth = 0;
    for( USHORT n = 0; n < pLine->GetTabBoxes().Count(); ++n )
        nWidth += pLine->GetTabBoxes()[n]->GetFrmFmt()->GetFrmSize().GetWidth();

    pFmt->SetAttr( SwFmtFrmSize( ATT_VAR_SIZE, nWidth, 0 ));

    // in Boxen mit Lines darf es nur noch Size/Fillorder geben
    pFmt->ResetAttr( RES_LR_SPACE, RES_FRMATR_END - 1 );
    pFmt->ResetAttr( RES_BOXATR_BEGIN, RES_BOXATR_END - 1 );
 }



struct _InsULPara
{
    SwTableNode* pTblNd;
    SwTableLine* pInsLine;
    SwTableBox* pInsBox;
    BOOL bUL_LR : 1;        // Upper-Lower(TRUE) oder Left-Right(FALSE) ?
    BOOL bUL : 1;           // Upper-Left(TRUE) oder Lower-Right(FALSE) ?

    SwTableBox* pLeftBox;
    SwTableBox* pRightBox;
    SwTableBox* pMergeBox;

    _InsULPara( SwTableNode* pTNd, BOOL bUpperLower, BOOL bUpper,
                SwTableBox* pLeft, SwTableBox* pMerge, SwTableBox* pRight,
                SwTableLine* pLine=0, SwTableBox* pBox=0 )
        : pTblNd( pTNd )
        , pInsLine( pLine )
        , pInsBox( pBox )
        , pLeftBox( pLeft )
        , pRightBox( pRight )
        , pMergeBox( pMerge )
        {   bUL_LR = bUpperLower; bUL = bUpper; }

    void SetLeft( SwTableBox* pBox=0 )
        { bUL_LR = FALSE;   bUL = TRUE; if( pBox ) pInsBox = pBox; }
    void SetRight( SwTableBox* pBox=0 )
        { bUL_LR = FALSE;   bUL = FALSE; if( pBox ) pInsBox = pBox; }
    void SetUpper( SwTableLine* pLine=0 )
        { bUL_LR = TRUE;    bUL = TRUE;  if( pLine ) pInsLine = pLine; }
    void SetLower( SwTableLine* pLine=0 )
        { bUL_LR = TRUE;    bUL = FALSE; if( pLine ) pInsLine = pLine; }
};


BOOL lcl_Merge_MoveBox( const _FndBox*& rpFndBox, void* pPara )
{
    _InsULPara* pULPara = (_InsULPara*)pPara;
    SwTableBoxes* pBoxes;

    USHORT nStt = 0, nEnd = rpFndBox->GetLines().Count();
    USHORT nInsPos = USHRT_MAX;
    if( !pULPara->bUL_LR )  // Left/Right
    {
        USHORT nPos;
        SwTableBox* pFndBox = (SwTableBox*)rpFndBox->GetBox();
        pBoxes = &pFndBox->GetUpper()->GetTabBoxes();
        if( pULPara->bUL )  // Left ?
        {
            // gibt es noch davor Boxen, dann move sie
            if( 0 != ( nPos = pBoxes->C40_GETPOS( SwTableBox, pFndBox )) )
                lcl_CpyBoxes( 0, nPos, *pBoxes, pULPara->pInsLine );
        }
        else                // Right
            // gibt es noch dahinter Boxen, dann move sie
            if( (nPos = pBoxes->C40_GETPOS( SwTableBox, pFndBox )) +1 < pBoxes->Count() )
            {
                nInsPos = pULPara->pInsLine->GetTabBoxes().Count();
                lcl_CpyBoxes( nPos+1, pBoxes->Count(),
                                    *pBoxes, pULPara->pInsLine );
            }
    }
    // Upper/Lower und gehts noch tiefer ??
    else if( rpFndBox->GetLines().Count() )
    {
        // suche nur die Line, ab der Verschoben werden muss
        nStt = pULPara->bUL ? 0 : rpFndBox->GetLines().Count()-1;
        nEnd = nStt+1;
    }

    pBoxes = &pULPara->pInsLine->GetTabBoxes();

    // geht es noch eine weitere Stufe runter?
    if( rpFndBox->GetBox()->GetTabLines().Count() )
    {
        SwTableBox* pBox = new SwTableBox(
                (SwTableBoxFmt*)rpFndBox->GetBox()->GetFrmFmt(), 0, pULPara->pInsLine );
        _InsULPara aPara( *pULPara );
        aPara.pInsBox = pBox;
        ((_FndBox*)rpFndBox)->GetLines().ForEach( nStt, nEnd,
                                                &lcl_Merge_MoveLine, &aPara );
        if( pBox->GetTabLines().Count() )
        {
            if( USHRT_MAX == nInsPos )
                nInsPos = pBoxes->Count();
            pBoxes->C40_INSERT( SwTableBox, pBox, nInsPos );
            lcl_CalcWidth( pBox );      // bereche die Breite der Box
        }
        else
            delete pBox;
    }
    return TRUE;
}

BOOL lcl_Merge_MoveLine( const _FndLine*& rpFndLine, void* pPara )
{
    _InsULPara* pULPara = (_InsULPara*)pPara;
    SwTableLines* pLines;

    USHORT nStt = 0, nEnd = rpFndLine->GetBoxes().Count();
    USHORT nInsPos = USHRT_MAX;
    if( pULPara->bUL_LR )   // UpperLower ?
    {
        USHORT nPos;
        SwTableLine* pFndLn = (SwTableLine*)rpFndLine->GetLine();
        pLines = pFndLn->GetUpper() ?
                        &pFndLn->GetUpper()->GetTabLines() :
                        &pULPara->pTblNd->GetTable().GetTabLines();

        SwTableBox* pLBx = rpFndLine->GetBoxes()[0]->GetBox();
        SwTableBox* pRBx = rpFndLine->GetBoxes()[
                            rpFndLine->GetBoxes().Count()-1]->GetBox();
        USHORT nLeft = pFndLn->GetTabBoxes().C40_GETPOS( SwTableBox, pLBx );
        USHORT nRight = pFndLn->GetTabBoxes().C40_GETPOS( SwTableBox, pRBx );

        if( !nLeft || nRight == pFndLn->GetTabBoxes().Count() )
        {
            if( pULPara->bUL )  // Upper ?
            {
                // gibt es noch davor Zeilen, dann move sie
                if( 0 != ( nPos = pLines->C40_GETPOS( SwTableLine, pFndLn )) )
                    lcl_CpyLines( 0, nPos, *pLines, pULPara->pInsBox );
            }
            else
                // gibt es noch dahinter Zeilen, dann move sie
                if( (nPos = pLines->C40_GETPOS( SwTableLine, pFndLn )) +1 < pLines->Count() )
                {
                    nInsPos = pULPara->pInsBox->GetTabLines().Count();
                    lcl_CpyLines( nPos+1, pLines->Count(), *pLines,
                                        pULPara->pInsBox );
                }
        }
        else if( nLeft )
        {
            // es gibt links noch weitere Boxen, also setze Left-
            // und Merge-Box in eine Box und Line, fuege davor/dahinter
            // eine Line mit Box ein, in die die oberen/unteren Lines
            // eingefuegt werden
            SwTableLine* pInsLine = pULPara->pLeftBox->GetUpper();
            SwTableBox* pLMBox = new SwTableBox(
                (SwTableBoxFmt*)pULPara->pLeftBox->GetFrmFmt(), 0, pInsLine );
            SwTableLine* pLMLn = new SwTableLine(
                        (SwTableLineFmt*)pInsLine->GetFrmFmt(), 2, pLMBox );
            pLMLn->ClaimFrmFmt()->ResetAttr( RES_FRM_SIZE );

            pLMBox->GetTabLines().C40_INSERT( SwTableLine, pLMLn, 0 );

            lcl_CpyBoxes( 0, 2, pInsLine->GetTabBoxes(), pLMLn );

            pInsLine->GetTabBoxes().C40_INSERT( SwTableBox, pLMBox, 0 );

            if( pULPara->bUL )  // Upper ?
            {
                // gibt es noch davor Zeilen, dann move sie
                if( 0 != ( nPos = pLines->C40_GETPOS( SwTableLine, pFndLn )) )
                    lcl_CpyLines( 0, nPos, *pLines, pLMBox, 0 );
            }
            else
                // gibt es noch dahinter Zeilen, dann move sie
                if( (nPos = pLines->C40_GETPOS( SwTableLine, pFndLn )) +1 < pLines->Count() )
                    lcl_CpyLines( nPos+1, pLines->Count(), *pLines,
                                        pLMBox );
            lcl_CalcWidth( pLMBox );        // bereche die Breite der Box
        }
        else if( nRight+1 < pFndLn->GetTabBoxes().Count() )
        {
            // es gibt rechts noch weitere Boxen, also setze Right-
            // und Merge-Box in eine Box und Line, fuege davor/dahinter
            // eine Line mit Box ein, in die die oberen/unteren Lines
            // eingefuegt werden
            SwTableLine* pInsLine = pULPara->pRightBox->GetUpper();
            SwTableBox* pRMBox;
            if( pULPara->pLeftBox->GetUpper() == pInsLine )
            {
                pRMBox = new SwTableBox(
                    (SwTableBoxFmt*)pULPara->pRightBox->GetFrmFmt(), 0, pInsLine );
                SwTableLine* pRMLn = new SwTableLine(
                    (SwTableLineFmt*)pInsLine->GetFrmFmt(), 2, pRMBox );
                pRMLn->ClaimFrmFmt()->ResetAttr( RES_FRM_SIZE );
                pRMBox->GetTabLines().C40_INSERT( SwTableLine, pRMLn, 0 );

                lcl_CpyBoxes( 1, 3, pInsLine->GetTabBoxes(), pRMLn );

                pInsLine->GetTabBoxes().C40_INSERT( SwTableBox, pRMBox, 0 );
            }
            else
            {
                // Left und Merge wurden schon zusammengefuegt, also move
                // Right auch mit in die Line

                pInsLine = pULPara->pLeftBox->GetUpper();
                USHORT nMvPos = pULPara->pRightBox->GetUpper()->GetTabBoxes().
                                    C40_GETPOS( SwTableBox, pULPara->pRightBox );
                lcl_CpyBoxes( nMvPos, nMvPos+1,
                            pULPara->pRightBox->GetUpper()->GetTabBoxes(),
                            pInsLine );
                pRMBox = pInsLine->GetUpper();

                // sind schon Lines vorhanden, dann muessen diese in eine
                // neue Line und Box
                nMvPos = pRMBox->GetTabLines().C40_GETPOS( SwTableLine, pInsLine );
                if( pULPara->bUL ? nMvPos
                                : nMvPos+1 < pRMBox->GetTabLines().Count() )
                {
                    // alle Lines zu einer neuen Line und Box zusammenfassen
                    SwTableLine* pNewLn = new SwTableLine(
                        (SwTableLineFmt*)pInsLine->GetFrmFmt(), 1, pRMBox );
                    pNewLn->ClaimFrmFmt()->ResetAttr( RES_FRM_SIZE );
                    pRMBox->GetTabLines().C40_INSERT( SwTableLine, pNewLn,
                            pULPara->bUL ? nMvPos : nMvPos+1 );
                    pRMBox = new SwTableBox( (SwTableBoxFmt*)pRMBox->GetFrmFmt(), 0, pNewLn );
                    pNewLn->GetTabBoxes().C40_INSERT( SwTableBox, pRMBox, 0 );

                    USHORT nPos1, nPos2;
                    if( pULPara->bUL )
                        nPos1 = 0,
                        nPos2 = nMvPos;
                    else
                        nPos1 = nMvPos+2,
                        nPos2 = pNewLn->GetUpper()->GetTabLines().Count();

                    lcl_CpyLines( nPos1, nPos2,
                                pNewLn->GetUpper()->GetTabLines(), pRMBox );
                    lcl_CalcWidth( pRMBox );        // bereche die Breite der Box

                    pRMBox = new SwTableBox( (SwTableBoxFmt*)pRMBox->GetFrmFmt(), 0, pNewLn );
                    pNewLn->GetTabBoxes().C40_INSERT( SwTableBox, pRMBox,
                                    pNewLn->GetTabBoxes().Count() );
                }
            }
            if( pULPara->bUL )  // Upper ?
            {
                // gibt es noch davor Zeilen, dann move sie
                if( 0 != ( nPos = pLines->C40_GETPOS( SwTableLine, pFndLn )) )
                    lcl_CpyLines( 0, nPos, *pLines, pRMBox, 0 );
            }
            else
                // gibt es noch dahinter Zeilen, dann move sie
                if( (nPos = pLines->C40_GETPOS( SwTableLine, pFndLn )) +1 < pLines->Count() )
                    lcl_CpyLines( nPos+1, pLines->Count(), *pLines,
                                        pRMBox );
            lcl_CalcWidth( pRMBox );        // bereche die Breite der Box
        }
        else
            OSL_ENSURE( FALSE , "Was denn nun" );
    }
    // Left/Right
    else
    {
        // suche nur die Line, ab der Verschoben werden muss
        nStt = pULPara->bUL ? 0 : rpFndLine->GetBoxes().Count()-1;
        nEnd = nStt+1;
    }
    pLines = &pULPara->pInsBox->GetTabLines();

    SwTableLine* pNewLine = new SwTableLine(
        (SwTableLineFmt*)rpFndLine->GetLine()->GetFrmFmt(), 0, pULPara->pInsBox );
    _InsULPara aPara( *pULPara );       // kopieren
    aPara.pInsLine = pNewLine;
    ((_FndLine*)rpFndLine)->GetBoxes().ForEach( nStt, nEnd,
                                                &::binfilter::lcl_Merge_MoveBox, &aPara );
    if( pNewLine->GetTabBoxes().Count() )
    {
        if( USHRT_MAX == nInsPos )
            nInsPos = pLines->Count();
        pLines->C40_INSERT( SwTableLine, pNewLine, nInsPos );
    }
    else
        delete pNewLine;

    return TRUE;
}

// ---------------------------------------------------------------

// suche ab dieser Line nach der naechsten Box mit Inhalt
 SwTableBox* SwTableLine::FindNextBox( const SwTable& rTbl,
                     const SwTableBox* pSrchBox, BOOL bOvrTblLns ) const
 {
    const SwTableLine* pLine = this;            // fuer M800
    SwTableBox* pBox;
    USHORT nFndPos;
    if( GetTabBoxes().Count() && pSrchBox &&
        USHRT_MAX != ( nFndPos = GetTabBoxes().GetPos( pSrchBox )) &&
        nFndPos + 1 != GetTabBoxes().Count() )
    {
        pBox = GetTabBoxes()[ nFndPos + 1 ];
        while( pBox->GetTabLines().Count() )
            pBox = pBox->GetTabLines()[0]->GetTabBoxes()[0];
        return pBox;
    }

    if( GetUpper() )
    {
        nFndPos = GetUpper()->GetTabLines().GetPos( pLine );
        OSL_ENSURE( USHRT_MAX != nFndPos, "Line nicht in der Tabelle" );
        // gibts eine weitere Line
        if( nFndPos+1 >= GetUpper()->GetTabLines().Count() )
            return GetUpper()->GetUpper()->FindNextBox( rTbl, GetUpper(), bOvrTblLns );
        pLine = GetUpper()->GetTabLines()[nFndPos+1];
    }
    else if( bOvrTblLns )       // ueber die "GrundLines" einer Tabelle ?
    {
        // suche in der Tabelle nach der naechsten Line
        nFndPos = rTbl.GetTabLines().GetPos( pLine );
        if( nFndPos + 1 >= rTbl.GetTabLines().Count() )
            return 0;           // es gibt keine weitere Box mehr

        pLine = rTbl.GetTabLines()[ nFndPos+1 ];
    }
    else
        return 0;

    if( pLine->GetTabBoxes().Count() )
    {
        pBox = pLine->GetTabBoxes()[0];
        while( pBox->GetTabLines().Count() )
            pBox = pBox->GetTabLines()[0]->GetTabBoxes()[0];
        return pBox;
    }
    return pLine->FindNextBox( rTbl, 0, bOvrTblLns );
 }

// suche ab dieser Line nach der vorherigen Box
 SwTableBox* SwTableLine::FindPreviousBox( const SwTable& rTbl,
                         const SwTableBox* pSrchBox, BOOL bOvrTblLns ) const
 {
    const SwTableLine* pLine = this;            // fuer M800
    SwTableBox* pBox;
    USHORT nFndPos;
    if( GetTabBoxes().Count() && pSrchBox &&
        USHRT_MAX != ( nFndPos = GetTabBoxes().GetPos( pSrchBox )) &&
        nFndPos )
    {
        pBox = GetTabBoxes()[ nFndPos - 1 ];
        while( pBox->GetTabLines().Count() )
        {
            pLine = pBox->GetTabLines()[pBox->GetTabLines().Count()-1];
            pBox = pLine->GetTabBoxes()[pLine->GetTabBoxes().Count()-1];
        }
        return pBox;
    }

    if( GetUpper() )
    {
        nFndPos = GetUpper()->GetTabLines().GetPos( pLine );
        OSL_ENSURE( USHRT_MAX != nFndPos, "Line nicht in der Tabelle" );
        // gibts eine weitere Line
        if( !nFndPos )
            return GetUpper()->GetUpper()->FindPreviousBox( rTbl, GetUpper(), bOvrTblLns );
        pLine = GetUpper()->GetTabLines()[nFndPos-1];
    }
    else if( bOvrTblLns )       // ueber die "GrundLines" einer Tabelle ?
    {
        // suche in der Tabelle nach der naechsten Line
        nFndPos = rTbl.GetTabLines().GetPos( pLine );
        if( !nFndPos )
            return 0;           // es gibt keine weitere Box mehr

        pLine = rTbl.GetTabLines()[ nFndPos-1 ];
    }
    else
        return 0;

    if( pLine->GetTabBoxes().Count() )
    {
        pBox = pLine->GetTabBoxes()[pLine->GetTabBoxes().Count()-1];
        while( pBox->GetTabLines().Count() )
        {
            pLine = pBox->GetTabLines()[pBox->GetTabLines().Count()-1];
            pBox = pLine->GetTabBoxes()[pLine->GetTabBoxes().Count()-1];
        }
        return pBox;
    }
    return pLine->FindPreviousBox( rTbl, 0, bOvrTblLns );
 }

// suche ab dieser Line nach der naechsten Box mit Inhalt
BOOL lcl_BoxSetHeadCondColl( const SwTableBox*& rpBox, void* /*pPara*/ )
{
    // in der HeadLine sind die Absaetze mit BedingtenVorlage anzupassen
    const SwStartNode* pSttNd = rpBox->GetSttNd();
    if( pSttNd )
        pSttNd->CheckSectionCondColl();
    else
        ((SwTableBox*)rpBox)->GetTabLines().ForEach( &lcl_LineSetHeadCondColl, 0 );
    return TRUE;
}

BOOL lcl_LineSetHeadCondColl( const SwTableLine*& rpLine, void* /*pPara*/ )
{
    ((SwTableLine*)rpLine)->GetTabBoxes().ForEach( &lcl_BoxSetHeadCondColl, 0 );
    return TRUE;
}

 SwFrmFmt* SwShareBoxFmt::GetFormat( const SfxPoolItem& rItem ) const
 {
    const SfxPoolItem* pItem;
    USHORT nWhich = rItem.Which();
    SwFrmFmt *pRet = 0, *pTmp;
    const SfxPoolItem& rFrmSz = pOldFmt->GetAttr( RES_FRM_SIZE, FALSE );
    for( USHORT n = aNewFmts.Count(); n; )
        if( SFX_ITEM_SET == ( pTmp = (SwFrmFmt*)aNewFmts[ --n ])->
            GetItemState( nWhich, FALSE, &pItem ) && *pItem == rItem &&
            pTmp->GetAttr( RES_FRM_SIZE, FALSE ) == rFrmSz )
        {
            pRet = pTmp;
            break;
        }
    return pRet;
 }

 void SwShareBoxFmt::AddFormat( const SwFrmFmt& rNew )
 {
    void* pFmt = (void*)&rNew;
    aNewFmts.Insert( pFmt, aNewFmts.Count() );
 }

 bool SwShareBoxFmt::RemoveFormat( const SwFrmFmt& rFmt )
 {
    // returnt TRUE, wenn geloescht werden kann
    if( pOldFmt == &rFmt )
        return TRUE;

    void* p = (void*)&rFmt;
    USHORT nFnd = aNewFmts.GetPos( p );
    if( USHRT_MAX != nFnd )
        aNewFmts.Remove( nFnd );
    return 0 == aNewFmts.Count();
 }

/*N*/ SwShareBoxFmts::~SwShareBoxFmts()
/*N*/ {
/*N*/ }

 SwFrmFmt* SwShareBoxFmts::GetFormat( const SwFrmFmt& rFmt,
                                     const SfxPoolItem& rItem ) const
 {
    USHORT nPos;
    return Seek_Entry( rFmt, &nPos )
                    ? aShareArr[ nPos ]->GetFormat( rItem )
                    : 0;
 }

 void SwShareBoxFmts::AddFormat( const SwFrmFmt& rOld, const SwFrmFmt& rNew )
 {
    // wenn das Format nicht geshared ist, braucht es auch nicht in die
    // Liste aufgenommen werden. Denn es gibt keinen 2. der es sucht.
 //leider werden auch die CellFrms gefunden
 // if( !rOld.IsLastDepend() )
    {
        USHORT nPos;
        SwShareBoxFmt* pEntry;
        if( !Seek_Entry( rOld, &nPos ))
        {
            pEntry = new SwShareBoxFmt( rOld );
            aShareArr.C40_INSERT( SwShareBoxFmt, pEntry, nPos );
        }
        else
            pEntry = aShareArr[ nPos ];

        pEntry->AddFormat( rNew );
    }
 }
 void SwShareBoxFmts::ChangeFrmFmt( SwTableBox* pBox, SwTableLine* pLn,
                                    SwFrmFmt& rFmt )
 {
    SwClient aCl;
    SwFrmFmt* pOld = 0;
    if( pBox )
    {
        pOld = pBox->GetFrmFmt();
        pOld->Add( &aCl );
        pBox->ChgFrmFmt( (SwTableBoxFmt*)&rFmt );
    }
    else if( pLn )
    {
        pOld = pLn->GetFrmFmt();
        pOld->Add( &aCl );
        pLn->ChgFrmFmt( (SwTableLineFmt*)&rFmt );
    }
    if( pOld && pOld->IsLastDepend() )
    {
        RemoveFormat( *pOld );
        delete pOld;
    }
 }

 void SwShareBoxFmts::SetAttr( SwTableBox& rBox, const SfxPoolItem& rItem )
 {
    SwFrmFmt *pBoxFmt = rBox.GetFrmFmt(),
             *pRet = GetFormat( *pBoxFmt, rItem );
    if( pRet )
        ChangeFrmFmt( &rBox, 0, *pRet );
    else
    {
        pRet = rBox.ClaimFrmFmt();
        pRet->SetAttr( rItem );
        AddFormat( *pBoxFmt, *pRet );
    }
 }


 void SwShareBoxFmts::RemoveFormat( const SwFrmFmt& rFmt )
 {
    for( USHORT i = aShareArr.Count(); i; )
        if( aShareArr[ --i ]->RemoveFormat( rFmt ))
            aShareArr.DeleteAndDestroy( i );
 }

 BOOL SwShareBoxFmts::Seek_Entry( const SwFrmFmt& rFmt, USHORT* pPos ) const
 {
    ULONG nIdx = (ULONG)&rFmt;
    register USHORT nO = aShareArr.Count(), nM, nU = 0;
    if( nO > 0 )
    {
        nO--;
        while( nU <= nO )
        {
            nM = nU + ( nO - nU ) / 2;
            ULONG nFmt = (ULONG)&aShareArr[ nM ]->GetOldFormat();
            if( nFmt == nIdx )
            {
                if( pPos )
                    *pPos = nM;
                return TRUE;
            }
            else if( nFmt < nIdx )
                nU = nM + 1;
            else if( nM == 0 )
            {
                if( pPos )
                    *pPos = nU;
                return FALSE;
            }
            else
                nO = nM - 1;
        }
    }
    if( pPos )
        *pPos = nU;
    return FALSE;
 }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
