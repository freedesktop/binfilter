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
#ifndef _PAM_HXX
#define _PAM_HXX

#include <bf_svtools/bf_solar.h>

#include <stddef.h>         // fuer MemPool
#include <tools/gen.hxx>
#include <tools/mempool.hxx>

#include <cshtyp.hxx>       // fuer die Funktions-Definitionen
#include <ring.hxx>         // Superklasse
#include <index.hxx>        // fuer SwIndex
#include <ndindex.hxx>      // fuer SwNodeIndex
namespace com { namespace sun { namespace star { namespace util {
    struct SearchOptions;
} } } }
namespace binfilter {
class SfxPoolItem;
class SfxItemSet;


class SwFmt;

class SwDoc;
class SwNode;
class SwCntntNode;
class SwNodes;
class SwPaM;


namespace utl {
    class TextSearch;
}

struct SwPosition
{
    SwNodeIndex nNode;
    SwIndex nContent;

    SwPosition( );
    SwPosition( const SwNode& rNode );
    SwPosition( const SwNodeIndex &rNode );
    SwPosition( const SwNodeIndex &rNode, const SwIndex &rCntnt );
    SwPosition( const SwPosition & );

    SwPosition &operator=(const SwPosition &);

    bool operator < (const SwPosition &) const;
    bool operator > (const SwPosition &) const;
    bool operator <=(const SwPosition &) const;
    bool operator >=(const SwPosition &) const;
    bool operator ==(const SwPosition &) const;
    bool operator !=(const SwPosition &) const;
};


// SwPointAndMark / SwPaM
struct SwMoveFnCollection;
typedef SwMoveFnCollection* SwMoveFn;
extern SwMoveFn fnMoveForward, fnMoveBackward;

typedef bool (*SwGoInDoc)( SwPaM& rPam, SwMoveFn fnMove );
extern SwGoInDoc fnGoDoc, fnGoNode, fnGoCntnt, fnGoCntntCells;

class SwPaM : public Ring
{
    SwPosition aBound1;
    SwPosition aBound2;
    SwPosition *pPoint;
    SwPosition *pMark;


public:
    SwPaM(){};
    SwPaM( const SwPosition& rPos, SwPaM* pRing = 0 );
    SwPaM( const SwPosition& rMk, const SwPosition& rPt, SwPaM* pRing = 0 );
    SwPaM( SwPaM & );
    SwPaM( const SwNode& rMk, const SwNode& rPt,
           long nMkOffset = 0, long nPtOffset = 0, SwPaM* pRing = 0 );
    SwPaM(  const SwNode& rMk, xub_StrLen nMkCntnt,
            const SwNode& rPt, xub_StrLen nPtCntnt, SwPaM* pRing = 0 );
    SwPaM( const SwNode& rNd, xub_StrLen nCntnt = 0, SwPaM* pRing = 0 );
    SwPaM( const SwNodeIndex& rNd, xub_StrLen nCntnt = 0, SwPaM* pRing = 0 );
    virtual ~SwPaM();

    // Bewegen des Cursors
    bool Move( SwMoveFn fnMove = fnMoveForward,
                    SwGoInDoc fnGo = fnGoCntnt );

    // Suchen

    virtual void SetMark();
    void DeleteMark() { pMark = pPoint; }
#ifndef DBG_UTIL
    void Exchange()
    {
        if(pPoint != pMark)
        {
            SwPosition *pTmp = pPoint;
            pPoint = pMark;
            pMark = pTmp;
        }
    }
#else
    void Exchange();
#endif
    /*
     * Undokumented Feature: Liefert zurueck, ob das Pam ueber
     * eine Selektion verfuegt oder nicht. Definition einer
     * Selektion: Point und Mark zeigen auf unterschiedliche
     * Puffer.
     */
    bool HasMark() const { return pPoint == pMark? FALSE : TRUE; }

    const SwPosition *GetPoint() const { return pPoint; }
          SwPosition *GetPoint()       { return pPoint; }
    const SwPosition *GetMark()  const { return pMark; }
          SwPosition *GetMark()        { return pMark; }

    const SwPosition *Start() const
                    { return (*pPoint) <= (*pMark)? pPoint: pMark; }
          SwPosition *Start()
                    { return (*pPoint) <= (*pMark)? pPoint: pMark; }
    const SwPosition *End()   const
                    { return (*pPoint) > (*pMark)? pPoint: pMark; }
          SwPosition *End()
                    { return (*pPoint) > (*pMark)? pPoint: pMark; }

    SwPaM& operator=( SwPaM & );

    // erfrage vom SwPaM den aktuellen Node/ContentNode am SPoint / Mark
    SwNode* GetNode( BOOL bPoint = TRUE ) const
    {
        return &( bPoint ? pPoint->nNode : pMark->nNode ).GetNode();
    }
    SwCntntNode* GetCntntNode( BOOL bPoint = TRUE ) const
    {
        return ( bPoint ? pPoint->nNode : pMark->nNode ).GetNode().GetCntntNode();
    }

    /**
       Normalizes PaM, i.e. sort point and mark.

       @param bPointFirst TRUE: If the point is behind the mark then swap.
                          FALSE: If the mark is behind the point then swap.
    */
    SwPaM & Normalize(BOOL bPointFirst = TRUE);

    // erfrage vom SwPaM das Dokument, in dem er angemeldet ist
    SwDoc* GetDoc() const { return pPoint->nNode.GetNode().GetDoc(); }
    SwPosition& GetBound( BOOL bOne = TRUE )
    {   return bOne ? aBound1 : aBound2; }
    const SwPosition& GetBound( BOOL bOne = TRUE ) const
    {   return bOne ? aBound1 : aBound2; }

    // erfrage die Seitennummer auf der der Cursor steht
    USHORT GetPageNum( BOOL bAtPoint = TRUE, const Point* pLayPos = 0 );

    DECL_FIXEDMEMPOOL_NEWDEL(SwPaM)
};


bool CheckNodesRange( const SwNodeIndex&, const SwNodeIndex&, bool );
bool GoInCntnt( SwPaM & rPam, SwMoveFn fnMove );


} //namespace binfilter
#endif  // _PAM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
