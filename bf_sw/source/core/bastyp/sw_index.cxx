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

#include <stdlib.h>             // fuer qsort

#include <bf_svtools/bf_solar.h>

#include <osl/diagnose.h>
#include "index.hxx"

#include <bf_tools/string.hxx>

namespace binfilter {

#ifdef DBG_UTIL
int SwIndex::nSerial = 0;
#endif


/*N*/ TYPEINIT0(SwIndexReg);    // rtti


#ifdef CHK

#define IDX_CHK_ARRAY       pArray->ChhkArr();
#define ARR_CHK_ARRAY       ChhkArr();



#else                                   // CHK

#define IDX_CHK_ARRAY
#define ARR_CHK_ARRAY

#endif                                  // CHK



/*N*/ SwIndex::SwIndex( SwIndexReg* pArr, xub_StrLen nIdx )
/*N*/   : nIndex( nIdx ), pArray( pArr ), pNext( 0 ), pPrev( 0 )
/*N*/ {
/*N*/   if( !pArray )
/*N*/   {
/*N*/       pArray = SwIndexReg::pEmptyIndexArray;
/*N*/       nIndex = 0;     // steht immer auf 0 !!!
/*N*/   }
/*N*/
/*N*/   if( !pArray->pFirst )         // 1. Index ??
/*N*/       pArray->pFirst = pArray->pLast = this;
/*N*/   else if( pArray->pMiddle )
/*N*/   {
/*N*/       if( pArray->pMiddle->nIndex <= nIdx )
/*N*/       {
/*N*/           if( nIdx > ((pArray->pLast->nIndex - pArray->pMiddle->nIndex) / 2) )
/*N*/               ChgValue( *pArray->pLast, nIdx );
/*N*/           else
/*N*/               ChgValue( *pArray->pMiddle, nIdx );
/*N*/       }
/*N*/       else if( nIdx > ((pArray->pMiddle->nIndex - pArray->pFirst->nIndex) / 2) )
/*N*/           ChgValue( *pArray->pMiddle, nIdx );
/*N*/       else
/*N*/           ChgValue( *pArray->pFirst, nIdx );
/*N*/   }
/*N*/   else if( nIdx > ((pArray->pLast->nIndex - pArray->pFirst->nIndex) / 2) )
/*N*/       ChgValue( *pArray->pLast, nIdx );
/*N*/   else
/*N*/       ChgValue( *pArray->pFirst, nIdx );
/*N*/
/*N*/ #ifdef DBG_UTIL
/*N*/   MySerial = ++nSerial;       // nur in der nicht PRODUCT-Version
/*N*/ #endif
/*N*/ IDX_CHK_ARRAY
/*N*/ }




/*N*/ SwIndex::SwIndex( const SwIndex& rIdx )
/*N*/   : nIndex( rIdx.nIndex ), pArray( rIdx.pArray ), pNext( 0 ), pPrev( 0 )
/*N*/ {
/*N*/   ChgValue( rIdx, rIdx.nIndex );
/*N*/ #ifdef DBG_UTIL
/*N*/   MySerial = ++nSerial;       // nur in der nicht PRODUCT-Version
/*N*/ #endif
/*N*/ IDX_CHK_ARRAY
/*N*/ }


/*N*/ SwIndex& SwIndex::ChgValue( const SwIndex& rIdx, xub_StrLen nNewValue )
/*N*/ {
/*N*/   register SwIndex* pFnd = (SwIndex*)&rIdx;
/*N*/   if( rIdx.nIndex > nNewValue )               // nach vorne versuchen
/*N*/   {
/*N*/       register SwIndex* pPrv;
/*N*/       while( 0 != ( pPrv = pFnd->pPrev ) && pPrv->nIndex > nNewValue )
/*N*/           pFnd = pPrv;
/*N*/
/*N*/       if( pFnd != this )
/*N*/       {
/*N*/           // an alter Position ausketten
/*N*/           // erstmal an alter Position ausketten
/*N*/           if( pPrev )
/*N*/               pPrev->pNext = pNext;
/*N*/           else if( pArray->pFirst == this )
/*?*/               pArray->pFirst = pNext;
/*N*/
/*N*/           if( pNext )
/*N*/               pNext->pPrev = pPrev;
/*N*/           else if( pArray->pLast == this )
/*N*/               pArray->pLast = pPrev;
/*N*/
/*N*/           pNext = pFnd;
/*N*/           pPrev = pFnd->pPrev;
/*N*/           if( pPrev )
/*N*/               pPrev->pNext = this;
/*N*/           else
/*N*/               pArray->pFirst = this;
/*N*/           pFnd->pPrev = this;
/*N*/       }
/*N*/   }
/*N*/   else if( rIdx.nIndex < nNewValue )
/*N*/   {
/*N*/       register SwIndex* pNxt;
/*N*/       while( 0 != ( pNxt = pFnd->pNext ) && pNxt->nIndex < nNewValue )
/*N*/           pFnd = pNxt;
/*N*/
/*N*/       if( pFnd != this )
/*N*/       {
/*N*/           // erstmal an alter Position ausketten
/*N*/           if( pPrev )
/*N*/               pPrev->pNext = pNext;
/*N*/           else if( pArray->pFirst == this )
/*N*/               pArray->pFirst = pNext;
/*N*/
/*N*/           if( pNext )
/*N*/               pNext->pPrev = pPrev;
/*N*/           else if( pArray->pLast == this )
/*?*/               pArray->pLast = pPrev;
/*N*/
/*N*/           pPrev = pFnd;
/*N*/           pNext = pFnd->pNext;
/*N*/           if( pNext )
/*N*/               pNext->pPrev = this;
/*N*/           else
/*N*/               pArray->pLast = this;
/*N*/           pFnd->pNext = this;
/*N*/       }
/*N*/   }
/*N*/   else if( pFnd != this )
/*N*/   {
/*N*/       // erstmal an alter Position ausketten
/*N*/       if( pPrev )
/*N*/           pPrev->pNext = pNext;
/*N*/       else if( pArray->pFirst == this )
/*N*/           pArray->pFirst = pNext;
/*N*/
/*N*/       if( pNext )
/*N*/           pNext->pPrev = pPrev;
/*N*/       else if( pArray->pLast == this )
/*N*/           pArray->pLast = pPrev;
/*N*/
/*N*/       pPrev = (SwIndex*)&rIdx;
/*N*/       pNext = rIdx.pNext;
/*N*/       pPrev->pNext = this;
/*N*/
/*N*/       if( !pNext )            // im IndexArray als letzes
/*N*/           pArray->pLast = this;
/*N*/       else
/*N*/           pNext->pPrev = this;
/*N*/   }
/*N*/   pArray = rIdx.pArray;
/*N*/
/*N*/   if( pArray->pFirst == pNext )
/*?*/       pArray->pFirst = this;
/*N*/   if( pArray->pLast == pPrev )
/*?*/       pArray->pLast = this;
/*N*/
/*N*/   nIndex = nNewValue;
/*N*/
/*N*/ IDX_CHK_ARRAY
/*N*/
/*N*/   return *this; }


/*N*/ void SwIndex::Remove()
/*N*/ {
/*N*/   if( !pPrev )
/*N*/       pArray->pFirst = pNext;
/*N*/   else
/*N*/       pPrev->pNext = pNext;
/*N*/
/*N*/   if( !pNext )
/*N*/       pArray->pLast = pPrev;
/*N*/   else
/*N*/       pNext->pPrev = pPrev;
/*N*/
/*N*/   if( this == pArray->pMiddle )   pArray->pMiddle = pPrev;
/*N*/ IDX_CHK_ARRAY
/*N*/ }

/*************************************************************************
|*
|*    SwIndex & SwIndex::operator=( const SwIndex & aSwIndex )
|*
*************************************************************************/


/*N*/ SwIndex& SwIndex::operator=( const SwIndex& rIdx )
/*N*/ {
/*N*/   int bEqual;
/*N*/   if( rIdx.pArray != pArray )         // im alten abmelden !!
/*N*/   {
/*N*/       Remove();
/*N*/       pArray = rIdx.pArray;
/*N*/       pNext = pPrev = 0;
/*N*/       bEqual = FALSE;
/*N*/   }
/*N*/   else
/*N*/       bEqual = rIdx.nIndex == nIndex;
/*N*/
/*N*/   if( !bEqual )
/*N*/       ChgValue( rIdx, rIdx.nIndex );
/*N*/   return *this;
/*N*/ }

/*************************************************************************
|*
|*    SwIndex &SwIndex::Assign
|*
*************************************************************************/


/*N*/ SwIndex& SwIndex::Assign( SwIndexReg* pArr, xub_StrLen nIdx )
/*N*/ {
/*N*/   if( !pArr )
/*N*/   {
/*N*/       pArr = SwIndexReg::pEmptyIndexArray;
/*N*/       nIdx = 0;       // steht immer auf 0 !!!
/*N*/   }
/*N*/
/*N*/   if( pArr != pArray )            // im alten abmelden !!
/*N*/   {
/*N*/       Remove();
/*N*/       pArray = pArr;
/*N*/       pNext = pPrev = 0;
/*N*/       if( !pArr->pFirst )         // 1. Index ??
/*N*/       {
/*N*/           pArr->pFirst = pArr->pLast = this;
/*N*/           nIndex = nIdx;
/*N*/       }
/*N*/       else if( pArray->pMiddle )
/*N*/       {
/*?*/           if( pArray->pMiddle->nIndex <= nIdx )
/*?*/           {
/*?*/               if( nIdx > ((pArr->pLast->nIndex - pArr->pMiddle->nIndex) / 2) )
/*?*/                   ChgValue( *pArr->pLast, nIdx );
/*?*/               else
/*?*/                   ChgValue( *pArr->pMiddle, nIdx );
/*?*/           }
/*?*/           else if( nIdx > ((pArr->pMiddle->nIndex - pArr->pFirst->nIndex) / 2) )
/*?*/               ChgValue( *pArr->pMiddle, nIdx );
/*?*/           else
/*?*/               ChgValue( *pArr->pFirst, nIdx );
/*N*/       }
/*N*/       else if( nIdx > ((pArr->pLast->nIndex - pArr->pFirst->nIndex) / 2) )
/*N*/           ChgValue( *pArr->pLast, nIdx );
/*N*/       else
/*N*/           ChgValue( *pArr->pFirst, nIdx );
/*N*/   }
/*N*/   else if( nIndex != nIdx )
/*N*/       ChgValue( *this, nIdx );
/*N*/ IDX_CHK_ARRAY
/*N*/   return *this;
/*N*/ }


/*N*/ SwIndexReg::SwIndexReg()
/*N*/   : pFirst( 0 ), pLast( 0 ), pMiddle( 0 )
/*N*/ {
/*N*/ }

/*N*/ SwIndexReg::~SwIndexReg()
/*N*/ {
#ifdef DBG_UTIL
/*N*/   OSL_ENSURE( !pFirst || !pLast, "Es sind noch Indizies angemeldet" );
#endif
/*N*/ }

/*N*/ void SwIndexReg::Update( const SwIndex& rIdx, xub_StrLen nDiff, BOOL bNeg )
/*N*/ {
/*N*/   register SwIndex* pStt = (SwIndex*)&rIdx;
/*N*/   register xub_StrLen nNewVal = rIdx.nIndex;
/*N*/   if( bNeg )
/*N*/   {
/*N*/       register xub_StrLen nLast = rIdx.GetIndex() + nDiff;
/*N*/       while( pStt && pStt->nIndex == nNewVal )
/*N*/       {
/*N*/           pStt->nIndex = nNewVal;
/*N*/           pStt = pStt->pPrev;
/*N*/       }
/*N*/       pStt = rIdx.pNext;
/*N*/       while( pStt && pStt->nIndex >= nNewVal &&
/*N*/               pStt->nIndex <= nLast )
/*N*/       {
/*N*/           pStt->nIndex = nNewVal;
/*N*/           pStt = pStt->pNext;
/*N*/       }
/*N*/       while( pStt )
/*N*/       {
/*?*/           pStt->nIndex -= nDiff;
/*?*/           pStt = pStt->pNext;
/*N*/       }
/*N*/   }
/*N*/   else
/*N*/   {
/*N*/       while( pStt && pStt->nIndex == nNewVal )
/*N*/       {
/*N*/           pStt->nIndex += nDiff;
/*N*/           pStt = pStt->pPrev;
/*N*/       }
/*N*/       pStt = rIdx.pNext;
/*N*/       while( pStt )
/*N*/       {
/*N*/           pStt->nIndex += nDiff;
/*N*/           pStt = pStt->pNext;
/*N*/       }
/*N*/   }
/*N*/ ARR_CHK_ARRAY
/*N*/ }





/*N*/ void SwIndexReg::MoveTo( SwIndexReg& rArr )
/*N*/ {
/*N*/   if( this != &rArr && pFirst )
/*N*/   {
/*N*/       SwIndex* pIdx = (SwIndex*)pFirst, *pNext;
/*N*/       while( pIdx )
/*N*/       {
/*N*/           pNext = pIdx->pNext;
/*N*/           pIdx->Assign( &rArr, pIdx->GetIndex() );
/*N*/           pIdx = pNext;
/*N*/       }
/*N*/       pFirst = 0, pLast = 0, pMiddle = 0;
/*N*/   }
/*N*/ }




}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
