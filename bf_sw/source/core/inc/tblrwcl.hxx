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
#ifndef _TBLRWCL_HXX
#define _TBLRWCL_HXX

#include <bf_svtools/bf_solar.h>

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_USHORTS
#include <bf_svtools/svstdarr.hxx>
#endif

#include <swtypes.hxx>
#include <tblsel.hxx>
#include <swtable.hxx>
namespace binfilter {

class SfxPoolItem;
class SvxBorderLine;
class SwDoc;
class SwTableNode;
class _FndLine;
class _FndBox;
class SwTableLine;
class SwTableBox;
class SwTableBoxFmt;
class SwTableLineFmt;
class SwHistory;
class SwCntntNode;

class SwShareBoxFmts;
class SwFmtFrmSize;

// Funktions Deklarationen:
BOOL lcl_CopyRow( const _FndLine*& rpFndLine, void* pPara );
BOOL lcl_CopyCol( const _FndBox*& rpFndBox, void* pPara );

BOOL lcl_MergeGCBox( const SwTableBox*& rpBox, void* pPara );
BOOL lcl_MergeGCLine( const SwTableLine*& rpLine, void* pPara );

BOOL lcl_Merge_MoveBox( const _FndBox*& rpFndBox, void* pPara );
BOOL lcl_Merge_MoveLine( const _FndLine*& rpFndLine, void* pPara );

BOOL lcl_CopyBoxToDoc( const _FndBox*& rpFndBox, void* pPara );
BOOL lcl_CopyLineToDoc( const _FndLine*& rpFndLn, void* pPara );

BOOL lcl_BoxSetHeadCondColl( const SwTableBox*& rpBox, void* pPara );
BOOL lcl_LineSetHeadCondColl( const SwTableLine*& rpLine, void* pPara );


void _InsTblBox( SwDoc* pDoc, SwTableNode* pTblNd,
                SwTableLine* pLine, SwTableBoxFmt* pBoxFrmFmt,
                SwTableBox* pBox, USHORT nInsPos, USHORT nCnt = 1 );

// Klasse fuers SplitTable
// sammelt von einer Line die Boxen aller obersten oder untersten Lines
// in einem Array. Zusaetzlich werden die Positionen vermerkt.
// ( die Implementierung steht im ndtbl.cxx)

 class SwCollectTblLineBoxes
 {
    SvUShorts aPosArr;
    SwSelBoxes_SAR aBoxes;
    SwHistory* pHst;
    USHORT nMode, nWidth;
    BOOL bGetFromTop : 1;
    BOOL bGetValues : 1;

 public:
    SwCollectTblLineBoxes( BOOL bTop, USHORT nMd = 0, SwHistory* pHist=0 )
        : aPosArr( 16, 16 )
        , aBoxes( 16, 16 )
        , pHst( pHist )
        , nMode( nMd )
        , nWidth( 0 )
        , bGetFromTop( bTop )
        , bGetValues( TRUE )
    {}

    void AddBox( const SwTableBox& rBox );
    const SwTableBox* GetBoxOfPos( const SwTableBox& rBox );

    USHORT Count() const                { return aBoxes.Count(); }
    const SwTableBox& GetBox( USHORT nPos, USHORT* pWidth = 0 ) const
        {
            // hier wird die EndPos der Spalte benoetigt!
            if( pWidth )
                *pWidth = nPos+1 == aPosArr.Count() ? nWidth
                                                    : aPosArr[ nPos+1 ];
            return *aBoxes[ nPos ];
        }

    BOOL IsGetFromTop() const           { return bGetFromTop; }
    BOOL IsGetValues() const            { return bGetValues; }

    USHORT GetMode() const              { return nMode; }
    void SetValues( BOOL bFlag )        { bGetValues = FALSE; nWidth = 0;
                                          bGetFromTop = bFlag; }
    bool Resize( USHORT nOffset, USHORT nWidth );
 };

BOOL lcl_Box_CollectBox( const SwTableBox*& rpBox, void* pPara );
BOOL lcl_Line_CollectBox( const SwTableLine*& rpLine, void* pPara );

BOOL lcl_BoxSetSplitBoxFmts( const SwTableBox*& rpBox, void* pPara );

struct _SwGCLineBorder
{
    const SwTableLines* pLines;
    SwShareBoxFmts* pShareFmts;
    USHORT nLinePos;

    _SwGCLineBorder( const SwTable& rTable )
        : pLines( &rTable.GetTabLines() )
        , pShareFmts(0)
        , nLinePos( 0 )
        {}

    _SwGCLineBorder( const SwTableBox& rBox )
        : pLines( &rBox.GetTabLines() )
        , pShareFmts(0)
        , nLinePos( 0 )
        {}
    BOOL IsLastLine() const { return nLinePos + 1 >= pLines->Count(); }
};

class _SwGCBorder_BoxBrd
{
    const SvxBorderLine* pBrdLn;
    BOOL bAnyBorderFnd;
public:
    _SwGCBorder_BoxBrd() : pBrdLn( 0 ), bAnyBorderFnd( FALSE ) {}

    void SetBorder( const SvxBorderLine& rBorderLine )
        { pBrdLn = &rBorderLine; bAnyBorderFnd = FALSE; }

    // checke, ob die linke Border dieselbe wie die gesetzte ist
    // returnt FALSE falls gar keine Border gesetzt ist
    BOOL CheckLeftBorderOfFormat( const SwFrmFmt& rFmt );

    BOOL IsAnyBorderFound() const { return bAnyBorderFnd; }
};

BOOL lcl_GC_Line_Border( const SwTableLine*& , void* pPara );
BOOL lcl_GC_Box_Border( const SwTableBox*& , void* pPara );

BOOL lcl_GCBorder_ChkBoxBrd_L( const SwTableLine*& , void* pPara );
BOOL lcl_GCBorder_ChkBoxBrd_B( const SwTableBox*& , void* pPara );

BOOL lcl_GCBorder_GetLastBox_L( const SwTableLine*& , void* pPara );
BOOL lcl_GCBorder_GetLastBox_B( const SwTableBox*& , void* pPara );


class SwShareBoxFmt
{
    const SwFrmFmt* pOldFmt;
    SvPtrarr aNewFmts;

public:
    SwShareBoxFmt( const SwFrmFmt& rFmt )
        : pOldFmt( &rFmt ), aNewFmts( 1, 4 )
    {}

    const SwFrmFmt& GetOldFormat() const { return *pOldFmt; }

    SwFrmFmt* GetFormat( const SfxPoolItem& rItem ) const;
    void AddFormat( const SwFrmFmt& rFmt );
    bool RemoveFormat( const SwFrmFmt& rFmt );
};


SV_DECL_PTRARR_DEL( _SwShareBoxFmts, SwShareBoxFmt*, 8, 8 )

class SwShareBoxFmts
{
    _SwShareBoxFmts aShareArr;
    BOOL Seek_Entry( const SwFrmFmt& rFmt, USHORT* pPos ) const;

    void ChangeFrmFmt( SwTableBox* pBox, SwTableLine* pLn, SwFrmFmt& rFmt );
public:
    SwShareBoxFmts() {}
    ~SwShareBoxFmts();

    SwFrmFmt* GetFormat( const SwFrmFmt& rFmt, const SfxPoolItem& ) const;
    void AddFormat( const SwFrmFmt& rOld, const SwFrmFmt& rNew );
    void SetAttr( SwTableBox& rBox, const SfxPoolItem& rItem );
    void RemoveFormat( const SwFrmFmt& rFmt );
};



} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
