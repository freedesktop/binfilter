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
#include <bf_svx/brkitem.hxx>

#include <horiornt.hxx>

#include <doc.hxx>
#include <docstat.hxx>
#include <docary.hxx>
#include <fmtpdsc.hxx>
#include <laycache.hxx>
#include <layhelp.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <txtfrm.hxx>
#include <ndtxt.hxx>
#include <swtable.hxx>
#include <tabfrm.hxx>
#include <rowfrm.hxx>
#include <ndindex.hxx>
#include <sectfrm.hxx>
#include <fmtcntnt.hxx>
#include <pagedesc.hxx>
#include <frmtool.hxx>
#include <dflyobj.hxx>
#include <dcontact.hxx>
#include <flyfrm.hxx>

#include <set>
namespace binfilter {

/*N*/ SV_IMPL_PTRARR( SwPageFlyCache, SwFlyCachePtr )

/*N*/ void SwLayoutCache::ClearImpl()
/*N*/ {
/*N*/     if( !IsLocked() )
/*N*/     {
/*N*/         delete pImpl;
/*N*/         pImpl = 0;
/*N*/     }
/*N*/ }

/*N*/ SwLayoutCache::~SwLayoutCache()
/*N*/ {
/*N*/     OSL_ENSURE( !nLockCount, "Deleting a locked SwLayoutCache!?" );
/*N*/     delete pImpl;
/*N*/ }

/*
 * SwActualSection,
 *  a help class to create not nested section frames
 *  for nested sections.
 */

/*N*/ SwActualSection::SwActualSection( SwActualSection *pUp,
/*N*/                                 SwSectionFrm    *pSect,
/*N*/                                 SwSectionNode   *pNd ) :
/*N*/   pUpper( pUp ),
/*N*/   pSectFrm( pSect ),
/*N*/   pSectNode( pNd )
/*N*/ {
/*N*/   if ( !pSectNode )
/*N*/   {
/*?*/       const SwNodeIndex *pIndex = pSect->GetFmt()->GetCntnt().GetCntntIdx();
/*?*/       pSectNode = pSect->GetFmt()->GetDoc()->GetNodes()[*pIndex]->
/*?*/                                                           FindSectionNode();
/*N*/   }
/*N*/ }

/*
 * SwLayHelper
 *  is the helper class, which utilizes the layout cache information
 *  to distribute the document content to the rigth pages.
 * It's used by the _InsertCnt(..)-function.
 * If there's no layout cache, the distibution to the pages is more
 * a guess, but a guess with statistical background.
 */

/*N*/ SwLayHelper::SwLayHelper( SwDoc *pD, SwFrm* &rpF, SwFrm* &rpP, SwPageFrm* &rpPg,
/*N*/             SwLayoutFrm* &rpL, SwActualSection* &rpA, BOOL &rB,
/*N*/             ULONG nNodeIndex, BOOL bCache )
/*N*/     : rpFrm( rpF ), rpPrv( rpP ), rpPage( rpPg ), rpLay( rpL ),
/*N*/       rpActualSection( rpA ), rbBreakAfter(rB), pDoc(pD), nMaxParaPerPage( 25 ),
/*N*/       nParagraphCnt( bCache ? 0 : USHRT_MAX ), bFirst( bCache )
/*N*/ {
/*N*/     pImpl = pDoc->GetLayoutCache() ? pDoc->GetLayoutCache()->LockImpl() : NULL;
/*N*/     if( pImpl )
/*N*/     {
/*?*/         nMaxParaPerPage = 1000;
/*?*/         nStartOfContent = pDoc->GetNodes().GetEndOfContent().FindStartNode()
/*?*/                           ->GetIndex();
/*?*/         nNodeIndex -= nStartOfContent;
/*?*/         nIndex = 0;
/*?*/         nFlyIdx = 0;
/*?*/         while( nIndex < pImpl->Count() && (*pImpl)[ nIndex ] < nNodeIndex )
/*?*/             ++nIndex;
/*?*/         if( nIndex >= pImpl->Count() )
/*?*/         {
/*?*/             pDoc->GetLayoutCache()->UnlockImpl();
/*?*/             pImpl = NULL;
/*?*/         }
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         nIndex = USHRT_MAX;
/*N*/         nStartOfContent = ULONG_MAX;
/*N*/     }
/*N*/ }

/*N*/ SwLayHelper::~SwLayHelper()
/*N*/ {
/*N*/     if( pImpl )
/*N*/     {
/*?*/         OSL_ENSURE( pDoc && pDoc->GetLayoutCache(), "Missing layoutcache" );
/*?*/         pDoc->GetLayoutCache()->UnlockImpl();
/*N*/     }
/*N*/ }

/*
 * SwLayHelper::CalcPageCount() does not really calculate the page count,
 * it returns the page count value from the layout cache, if available,
 * otherwise it estimates the page count.
 */

/*N*/ ULONG SwLayHelper::CalcPageCount()
/*N*/ {
/*N*/     ULONG nPgCount;
/*N*/     SwLayCacheImpl *pCache = pDoc->GetLayoutCache() ?
/*N*/                              pDoc->GetLayoutCache()->LockImpl() : NULL;
/*N*/     if( pCache )
/*N*/     {
/*?*/         nPgCount = pCache->Count() + 1;
/*?*/         pDoc->GetLayoutCache()->UnlockImpl();
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/       nPgCount = pDoc->GetDocStat().nPage;
/*N*/         if ( nPgCount <= 10 ) // no page insertion for less than 10 pages
/*N*/           nPgCount = 0;
/*N*/       ULONG nNdCount = pDoc->GetDocStat().nPara;
/*N*/       if ( nNdCount <= 1 )
/*N*/       {
/*N*/             //Estimates the number of paragraphs.
/*N*/           ULONG nTmp = pDoc->GetNodes().GetEndOfContent().GetIndex() -
/*N*/                       pDoc->GetNodes().GetEndOfExtras().GetIndex();
/*N*/             //Tables have a little overhead..
/*N*/           nTmp -= pDoc->GetTblFrmFmts()->Count() * 25;
/*N*/             //Fly frames, too ..
/*N*/           nTmp -= (pDoc->GetNodes().GetEndOfAutotext().GetIndex() -
/*N*/                      pDoc->GetNodes().GetEndOfInserts().GetIndex()) / 3 * 5;
/*N*/           if ( nTmp > 0 )
/*N*/               nNdCount = nTmp;
/*N*/       }
/*N*/         if ( nNdCount > 100 ) // no estimation below this value
/*N*/       {
/*N*/           if ( nPgCount > 0 )
/*N*/               nMaxParaPerPage = nNdCount / nPgCount;
/*N*/           else
/*N*/           {
/*N*/               nMaxParaPerPage = Max( ULONG(20),
/*N*/                                      ULONG(20 + nNdCount / 1000 * 3) );
/*N*/ #ifdef PM2
/*N*/               const ULONG nMax = 49;
/*N*/ #elif MAC
/*N*/               const ULONG nMax = 56;
/*N*/ #elif UNIX
/*N*/               const ULONG nMax = 57;
/*N*/ #else
/*N*/               const ULONG nMax = 53;
/*N*/ #endif
/*N*/               nMaxParaPerPage = Min( nMaxParaPerPage, nMax );
/*N*/               nPgCount = nNdCount / nMaxParaPerPage;
/*N*/           }
/*N*/             if ( nNdCount < 1000 )
/*N*/                 nPgCount = 0;// no progress bar for small documents
/*N*/           if ( pDoc->IsBrowseMode() )
/*?*/               nMaxParaPerPage *= 6;
/*N*/       }
/*N*/     }
/*N*/     return nPgCount;
/*N*/ }

/*
 * SwLayHelper::CheckInsertPage()
 * inserts a page and return TRUE, if
 * - the break after flag is set
 * - the actual content wants a break before
 * - the maximum count of paragraph/rows is reached
 *
 * The break after flag is set, if the actual content
 * wants a break after.
 */

/*N*/ BOOL SwLayHelper::CheckInsertPage()
/*N*/ {
/*N*/   bool bEnd = 0 == rpPage->GetNext();
/*N*/     const SwAttrSet *pAttr = rpFrm->GetAttrSet();
/*N*/   const SvxFmtBreakItem &rBrk = pAttr->GetBreak();
/*N*/   const SwFmtPageDesc &rDesc = pAttr->GetPageDesc();
/*N*/   const SwPageDesc *pDesc = rDesc.GetPageDesc();
/*N*/
/*N*/   BOOL bBrk = nParagraphCnt > nMaxParaPerPage || rbBreakAfter;
/*N*/   rbBreakAfter = rBrk.GetBreak() == SVX_BREAK_PAGE_AFTER ||
/*N*/                  rBrk.GetBreak() == SVX_BREAK_PAGE_BOTH;
/*N*/   if ( !bBrk )
/*N*/       bBrk = rBrk.GetBreak() == SVX_BREAK_PAGE_BEFORE ||
/*N*/              rBrk.GetBreak() == SVX_BREAK_PAGE_BOTH;
/*N*/
/*N*/   if ( bBrk || pDesc )
/*N*/   {
/*N*/       USHORT nPgNum = 0;
/*N*/       if ( !pDesc )
/*N*/           pDesc = rpPage->GetPageDesc()->GetFollow();
/*N*/       BOOL bOdd = !rpPage->OnRightPage();
/*N*/       BOOL bInsertEmpty = FALSE;
/*N*/       if( nPgNum && bOdd != ( ( nPgNum % 2 ) != 0 ) )
/*N*/       {
/*N*/           bOdd = !bOdd;
/*N*/           bInsertEmpty = TRUE;
/*N*/       }
/*N*/       ::binfilter::InsertNewPage( (SwPageDesc&)*pDesc, rpPage->GetUpper(),
/*N*/                        bOdd, bInsertEmpty, FALSE, rpPage->GetNext() );
/*N*/       if ( bEnd )
/*N*/       {
/*N*/           OSL_ENSURE( rpPage->GetNext(), "Keine neue Seite?" );
/*N*/           do
/*N*/           {   rpPage = (SwPageFrm*)rpPage->GetNext();
/*N*/           } while ( rpPage->GetNext() );
/*N*/       }
/*N*/       else
/*N*/       {
/*?*/           OSL_ENSURE( rpPage->GetNext(), "Keine neue Seite?" );
/*?*/           rpPage = (SwPageFrm*)rpPage->GetNext();
/*?*/           if ( rpPage->IsEmptyPage() )
/*?*/           {
/*?*/               OSL_ENSURE( rpPage->GetNext(), "Keine neue Seite?" );
/*?*/               rpPage = (SwPageFrm*)rpPage->GetNext();
/*?*/           }
/*N*/       }
/*N*/       rpLay = rpPage->FindBodyCont();
/*N*/       while( rpLay->Lower() )
/*N*/           rpLay = (SwLayoutFrm*)rpLay->Lower();
/*N*/       return TRUE;
/*N*/   }
/*N*/   return FALSE;
/*N*/ }

/*
 * SwLayHelper::CheckInsert
 *  is the entry point for the _InsertCnt-function.
 *  The document content index is checked either it is
 *  in the layout cache either it's time to insert a page
 *  cause the maximal estimation of content per page is reached.
 *  A really big table or long paragraph may contains more than
 *  one page, in this case the needed count of pages will inserted.
 */

/*N*/ BOOL SwLayHelper::CheckInsert( ULONG nNodeIndex )
/*N*/ {
/*N*/     BOOL bRet = FALSE;
/*N*/     BOOL bLongTab = FALSE;
/*N*/     ULONG nMaxRowPerPage(0);
/*N*/     nNodeIndex -= nStartOfContent;
/*N*/     USHORT nRows = 0;
/*N*/     if( rpFrm->IsTabFrm() )
/*N*/     {
/*N*/         //Inside a table counts every row as a paragraph
/*N*/         SwFrm *pLow = ((SwTabFrm*)rpFrm)->Lower();
/*N*/         nRows = 0;
/*N*/         do
/*N*/         {
/*N*/             ++nRows;
/*N*/             pLow = pLow->GetNext();
/*N*/         } while ( pLow );
/*N*/         nParagraphCnt += nRows;
/*N*/         if( !pImpl && nParagraphCnt > nMaxParaPerPage + 10 )
/*N*/         {
/*N*/             // improve heuristics:
/*N*/             // Assume that a table, which has more than three times the quantity
/*N*/             // of maximal paragraphs per page rows, consists of rows, which have
/*N*/             // the height of a normal paragraph. Thus, allow as much rows per page
/*N*/             // as much paragraphs are allowed.
/*N*/             if ( nRows > ( 3*nMaxParaPerPage ) )
/*N*/             {
/*N*/                 nMaxRowPerPage = nMaxParaPerPage;
/*N*/             }
/*N*/             else
/*N*/             {
/*N*/                   SwFrm *pTmp = ((SwTabFrm*)rpFrm)->Lower();
/*N*/                   if( pTmp->GetNext() )
/*N*/                   pTmp = pTmp->GetNext();
/*N*/                   pTmp = ((SwRowFrm*)pTmp)->Lower();
/*N*/                   USHORT nCnt = 0;
/*N*/                   do
/*N*/                   {
/*N*/                       ++nCnt;
/*N*/                       pTmp = pTmp->GetNext();
/*N*/                   } while( pTmp );
/*N*/                   nMaxRowPerPage = Max( ULONG(2), nMaxParaPerPage / nCnt );
/*N*/            }
/*N*/             bLongTab = TRUE;
/*N*/         }
/*N*/     }
/*N*/     else
/*N*/         ++nParagraphCnt;
/*N*/     if( bFirst && pImpl && nIndex < pImpl->Count() &&
/*N*/         pImpl->GetBreakIndex( nIndex ) == nNodeIndex &&
/*N*/         ( pImpl->GetBreakOfst( nIndex ) < STRING_LEN ||
/*N*/           ( ++nIndex < pImpl->Count() &&
/*N*/           pImpl->GetBreakIndex( nIndex ) == nNodeIndex ) ) )
/*?*/         bFirst = FALSE;
/*N*/
/*N*/     // always split a big tables.
/*N*/     if ( !bFirst ||
/*N*/          ( rpFrm->IsTabFrm() && bLongTab )
/*N*/        )
/*N*/     {
/*N*/         ULONG nRowCount = 0;
/*N*/         do
/*N*/         {
/*N*/             if( pImpl || bLongTab )
/*N*/             {
/*N*/                 xub_StrLen nOfst = STRING_LEN;
/*N*/                 USHORT nType = SW_LAYCACHE_IO_REC_PAGES;
/*N*/                 if( bLongTab )
/*N*/                 {
/*N*/                     rbBreakAfter = sal_True;
/*N*/                     nOfst = nRowCount + nMaxRowPerPage;
/*N*/                 }
/*N*/                 else
/*N*/                 {
/*?*/                     while( nIndex < pImpl->Count() &&
/*?*/                            pImpl->GetBreakIndex(nIndex) < nNodeIndex)
/*?*/                         ++nIndex;
/*?*/                     if( nIndex < pImpl->Count() &&
/*?*/                         pImpl->GetBreakIndex(nIndex) == nNodeIndex )
/*?*/                     {
/*?*/                         nType = pImpl->GetBreakType( nIndex );
/*?*/                         nOfst = pImpl->GetBreakOfst( nIndex++ );
/*?*/                         rbBreakAfter = sal_True;
/*?*/                     }
/*N*/                 }
/*N*/
/*N*/                 if( nOfst < STRING_LEN )
/*N*/                 {
/*N*/                     sal_Bool bSplit = sal_False;
/*N*/                     sal_Bool bRepeat = sal_False;
/*N*/                     if( !bLongTab && rpFrm->IsTxtFrm() &&
/*N*/                         SW_LAYCACHE_IO_REC_PARA == nType &&
/*N*/                         nOfst<((SwTxtFrm*)rpFrm)->GetTxtNode()->GetTxt().Len() )
/*?*/                         bSplit = sal_True;
/*N*/                     else if( rpFrm->IsTabFrm() && nRowCount < nOfst &&
/*N*/                              ( bLongTab || SW_LAYCACHE_IO_REC_TABLE == nType ) )
/*N*/                     {
/*N*/                         bRepeat = ((SwTabFrm*)rpFrm)->
/*N*/                                   GetTable()->IsHeadlineRepeat();
/*N*/                         bSplit = nOfst < nRows;
/*N*/                         bLongTab = bLongTab && bSplit;
/*N*/                     }
/*N*/                     if( bSplit )
/*N*/                     {
/*N*/                         rpFrm->InsertBehind( rpLay, rpPrv );
/*N*/                         rpFrm->Frm().Pos() = rpLay->Frm().Pos();
/*N*/                         rpFrm->Frm().Pos().Y() += 1;
/*N*/                         rpPrv = rpFrm;
/*N*/                         if( rpFrm->IsTabFrm() )
/*N*/                         {
/*N*/                             SwTabFrm* pTab = (SwTabFrm*)rpFrm;
/*N*/                             SwFrm *pRow = pTab->Lower();
/*N*/                             SwTabFrm *pFoll = new SwTabFrm( *pTab );
/*N*/
/*N*/                             SwFrm *pPrv;
/*N*/                             if( bRepeat )
/*N*/                             {
/*N*/                                 bDontCreateObjects = TRUE; //frmtool
/*N*/                                 SwRowFrm *pHeadline = new SwRowFrm(
/*N*/                                         *pTab->GetTable()->GetTabLines()[0] );
/*N*/                                 pHeadline->InsertBefore( pFoll, 0 );
/*N*/                                 bDontCreateObjects = FALSE;
/*N*/                                 pPrv = pFoll->Lower();
/*N*/                                 ++nRows;
/*N*/                             }
/*N*/                             else
/*?*/                                 pPrv = 0;
/*N*/                             while( pRow && nRowCount < nOfst )
/*N*/                             {
/*N*/                                 pRow = pRow->GetNext();
/*N*/                                 ++nRowCount;
/*N*/                             }
/*N*/                             while ( pRow )
/*N*/                             {
/*N*/                                 SwFrm* pNxt = pRow->GetNext();
/*N*/                                 pRow->Remove();
/*N*/                                 pRow->InsertBehind( pFoll, pPrv );
/*N*/                                 pPrv = pRow;
/*N*/                                 pRow = pNxt;
/*N*/                             }
/*N*/                             rpFrm = pFoll;
/*N*/                         }
/*N*/                         else
/*N*/                         {
/*?*/                             SwTxtFrm *pNew = new SwTxtFrm( ((SwTxtFrm*)rpFrm)->
/*?*/                                                            GetTxtNode() );
/*?*/                             pNew->_SetIsFollow( sal_True );
/*?*/                             pNew->ManipOfst( nOfst );
/*?*/                             pNew->SetFollow( ((SwTxtFrm*)rpFrm)->GetFollow() );
/*?*/                             ((SwTxtFrm*)rpFrm)->SetFollow( pNew );
/*?*/                             rpFrm = pNew;
/*N*/                         }
/*N*/                     }
/*N*/                 }
/*N*/             }
/*N*/
/*N*/             SwPageFrm* pLastPage = rpPage;
/*N*/             if( CheckInsertPage() )
/*N*/             {
/*N*/                 _CheckFlyCache( pLastPage );
/*N*/                 if( rpPrv && rpPrv->IsTxtFrm() && !rpPrv->GetValidSizeFlag() )
/*N*/                     rpPrv->Frm().Height( rpPrv->GetUpper()->Prt().Height() );
/*N*/
/*N*/                 bRet = TRUE;
/*N*/                 rpPrv = 0;
/*N*/                 nParagraphCnt = 0;
/*N*/
/*N*/                 if ( rpActualSection )
/*N*/                 {
/*N*/                     //Hatte der SectionFrm ueberhaupt Inhalt? Wenn
/*N*/                     //nicht kann er gleich umgehaengt werden.
/*N*/                     SwSectionFrm *pSct;
/*N*/                     BOOL bInit = FALSE;
/*N*/                     if ( !rpActualSection->GetSectionFrm()->ContainsCntnt())
/*N*/                     {
/*N*/                         pSct = rpActualSection->GetSectionFrm();
/*N*/                         pSct->Remove();
/*N*/                     }
/*N*/                     else
/*N*/                     {
/*N*/                         pSct = new SwSectionFrm(
/*N*/                             *rpActualSection->GetSectionFrm(), FALSE );
/*N*/                         rpActualSection->GetSectionFrm()->SimpleFormat();
/*N*/                         bInit = TRUE;
/*N*/                     }
/*N*/                     rpActualSection->SetSectionFrm( pSct );
/*N*/                     pSct->InsertBehind( rpLay, 0 );
/*N*/                     if( bInit )
/*N*/                         pSct->Init();
/*N*/                     pSct->Frm().Pos() = rpLay->Frm().Pos();
/*N*/                     pSct->Frm().Pos().Y() += 1; //wg. Benachrichtigungen.
/*N*/
/*N*/                     rpLay = pSct;
/*N*/                     if ( rpLay->Lower() && rpLay->Lower()->IsLayoutFrm() )
/*?*/                         rpLay = rpLay->GetNextLayoutLeaf();
/*N*/                 }
/*N*/             }
/*N*/         } while( bLongTab || ( pImpl && nIndex < pImpl->Count() &&
/*N*/                  (*pImpl)[ nIndex ] == nNodeIndex ) );
/*N*/     }
/*N*/     bFirst = FALSE;
/*N*/     return bRet;
/*N*/ }

/*N*/ struct SdrObjectCompare
/*N*/ {
/*N*/   bool operator()( const SdrObject* pF1, const SdrObject* pF2 ) const
/*N*/   {
/*N*/     return pF1->GetOrdNum() < pF2->GetOrdNum();
/*N*/   }
/*N*/ };

/*N*/ struct FlyCacheCompare
/*N*/ {
/*N*/   bool operator()( const SwFlyCache* pC1, const SwFlyCache* pC2 ) const
/*N*/   {
/*N*/     return pC1->nOrdNum < pC2->nOrdNum;
/*N*/   }
/*N*/ };

 /*
  * SwLayHelper::_CheckFlyCache(..)
  * If a new page is inserted, the last page is analysed.
  * If there are text frames with default position, the fly cache
  * is checked, if these frames are stored in the cache.
  */

/*N*/ void SwLayHelper::_CheckFlyCache( SwPageFrm* pPage )
/*N*/ {
/*N*/     if( !pImpl || !pPage )
/*N*/         return;
/*N*/     USHORT nFlyCount = pImpl->GetFlyCount();
/*N*/     // Any text frames at the page, fly cache avaiable?
/*N*/     if( pPage->GetSortedObjs() && nFlyIdx < nFlyCount )
/*N*/     {
/*N*/         SwSortDrawObjs &rObjs = *pPage->GetSortedObjs();
/*N*/         USHORT nPgNum = pPage->GetPhyPageNum();

        // NOTE: Here we do not use the absolute ordnums but
        // relative ordnums for the objects on this page.

        // skip fly frames from pages before the current page
/*N*/         SwFlyCache* pFlyC;
/*N*/         while( nFlyIdx < nFlyCount && ( pFlyC = pImpl->
/*N*/                GetFlyCache(nFlyIdx) )->nPageNum < nPgNum)
/*N*/             ++nFlyIdx;
/*N*/
/*N*/         // sort cached objects on this page by ordnum
/*N*/         std::set< const SwFlyCache*, FlyCacheCompare > aFlyCacheSet;
/*N*/         USHORT nIdx = nFlyIdx;
/*N*/
/*N*/         while( nIdx < nFlyCount && ( pFlyC = pImpl->
/*N*/                GetFlyCache( nIdx ) )->nPageNum == nPgNum )
/*N*/         {
/*N*/             aFlyCacheSet.insert( pFlyC );
/*N*/             ++nIdx;
/*N*/         }
/*N*/
/*N*/         // sort objects on this page by ordnum
/*N*/         std::set< const SdrObject*, SdrObjectCompare > aFlySet;
/*N*/         for ( USHORT i = 0; i < rObjs.Count(); ++i )
/*N*/         {
/*N*/             SdrObject* pO = rObjs[i];
/*N*/             if ( pO->IsWriterFlyFrame() )  // a text frame?
/*N*/             {
/*N*/                 SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
/*N*/                 if( pFly->GetAnchor() &&
/*N*/                     !pFly->GetAnchor()->FindFooterOrHeader() )
/*N*/                 {
/*N*/                     const SwContact *pC = (SwContact*)GetUserCall(pO);
/*N*/                     if( pC )
/*N*/                     {
/*N*/                         aFlySet.insert( pO );
/*N*/                     }
/*N*/                 }
/*N*/             }
/*N*/         }
/*N*/
/*N*/         if ( aFlyCacheSet.size() == aFlySet.size() )
/*N*/         {
/*N*/             std::set< const SwFlyCache*, FlyCacheCompare >::iterator aFlyCacheSetIt =
/*N*/                     aFlyCacheSet.begin();
/*N*/             std::set< const SdrObject*, SdrObjectCompare >::iterator aFlySetIt =
/*N*/                     aFlySet.begin();
/*N*/
/*N*/             while ( aFlyCacheSetIt != aFlyCacheSet.end() )
/*N*/             {
/*N*/                 const SwFlyCache* pFlyC1 = *aFlyCacheSetIt;
/*N*/                 SwFlyFrm* pFly = ((SwVirtFlyDrawObj*)*aFlySetIt)->GetFlyFrm();
/*N*/
/*N*/                 if ( pFly->Frm().Left() == WEIT_WECH )
/*N*/                 {
/*N*/                     // we get the stored information
/*N*/                     pFly->Frm().Pos().X() = pFlyC1->Left() +
/*N*/                                             pPage->Frm().Left();
/*N*/                     pFly->Frm().Pos().Y() = pFlyC1->Top() +
/*N*/                                             pPage->Frm().Top();
/*N*/                     if ( pImpl->IsUseFlyCache() )
/*N*/                     {
/*N*/                         pFly->Frm().Width( pFlyC1->Width() );
/*N*/                         pFly->Frm().Height( pFlyC1->Height() );
/*N*/                     }
/*N*/                 }
/*N*/
/*N*/                 ++aFlyCacheSetIt;
/*N*/                 ++aFlySetIt;
/*N*/             }
/*N*/         }
/*N*/     }
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
