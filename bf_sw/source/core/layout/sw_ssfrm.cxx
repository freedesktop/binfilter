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

#include <cntfrm.hxx>

#include <horiornt.hxx>

#include <doc.hxx>

#include <dcontact.hxx>
#include <dflyobj.hxx>
#include <flyfrm.hxx>
#include <txtfrm.hxx>       // ClearPara()
#include <ftnidx.hxx>
#include <txtftn.hxx>
#include <ndtxt.hxx>
#include <ndindex.hxx>

#include <frmtool.hxx>
#include <pagedesc.hxx>
#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#include <hints.hxx>        //fuer SwFmtChg
#include <bf_svx/boxitem.hxx>
#include <bf_svx/shaditem.hxx>
#include <viewsh.hxx>
namespace binfilter {

    // No inline cause we need the function pointers
/*N*/ long SwFrm::GetTopMargin() const
/*N*/     { return Prt().Top(); }
/*N*/ long SwFrm::GetBottomMargin() const
/*N*/     { return Frm().Height() -Prt().Height() -Prt().Top(); }
/*N*/ long SwFrm::GetLeftMargin() const
/*N*/     { return Prt().Left(); }
/*N*/ long SwFrm::GetRightMargin() const
/*N*/     { return Frm().Width() - Prt().Width() - Prt().Left(); }
/*N*/ long SwFrm::GetPrtLeft() const
/*N*/     { return Frm().Left() + Prt().Left(); }
/*N*/ long SwFrm::GetPrtBottom() const
/*N*/     { return Frm().Top() + Prt().Height() + Prt().Top(); }
/*N*/ long SwFrm::GetPrtRight() const
/*N*/     { return Frm().Left() + Prt().Width() + Prt().Left(); }
/*N*/ long SwFrm::GetPrtTop() const
/*N*/     { return Frm().Top() + Prt().Top(); }

/*N*/ BOOL SwFrm::SetMinLeft( long nDeadline )
/*N*/ {
/*N*/     SwTwips nDiff = nDeadline - Frm().Left();
/*N*/     if( nDiff > 0 )
/*N*/     {
/*?*/         Frm().Left( nDeadline );
/*?*/         Prt().Width( Prt().Width() - nDiff );
/*?*/         return TRUE;
/*N*/     }
/*N*/     return FALSE;
/*N*/ }

/*N*/ BOOL SwFrm::SetMaxBottom( long nDeadline )
/*N*/ {
/*N*/     SwTwips nDiff = Frm().Top() + Frm().Height() - nDeadline;
/*N*/     if( nDiff > 0 )
/*N*/     {
/*?*/         Frm().Height( Frm().Height() - nDiff );
/*?*/         Prt().Height( Prt().Height() - nDiff );
/*?*/         return TRUE;
/*N*/     }
/*N*/     return FALSE;
/*N*/ }

/*N*/ BOOL SwFrm::SetMinTop( long nDeadline )
/*N*/ {
/*N*/     SwTwips nDiff = nDeadline - Frm().Top();
/*N*/     if( nDiff > 0 )
/*N*/     {
/*?*/         Frm().Top( nDeadline );
/*?*/         Prt().Height( Prt().Height() - nDiff );
/*?*/         return TRUE;
/*N*/     }
/*N*/     return FALSE;
/*N*/ }

/*N*/ BOOL SwFrm::SetMaxRight( long nDeadline )
/*N*/ {
/*N*/     SwTwips nDiff = Frm().Left() + Frm().Width() - nDeadline;
/*N*/     if( nDiff > 0 )
/*N*/     {
/*?*/         Frm().Width( Frm().Width() - nDiff );
/*?*/         Prt().Width( Prt().Width() - nDiff );
/*?*/         return TRUE;
/*N*/     }
/*N*/     return FALSE;
/*N*/ }

/*N*/ void SwFrm::MakeBelowPos( const SwFrm* pUp, const SwFrm* pPrv, BOOL bNotify )
/*N*/ {
/*N*/     if( pPrv )
/*N*/     {
/*N*/         aFrm.Pos( pPrv->Frm().Pos() );
/*N*/         aFrm.Pos().Y() += pPrv->Frm().Height();
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         aFrm.Pos( pUp->Frm().Pos() );
/*N*/         aFrm.Pos() += pUp->Prt().Pos();
/*N*/     }
/*N*/     if( bNotify )
/*N*/         aFrm.Pos().Y() += 1;
/*N*/ }

/*N*/ void SwFrm::MakeUpperPos( const SwFrm* pUp, const SwFrm* pPrv, BOOL bNotify )
/*N*/ {
/*N*/     if( pPrv )
/*N*/     {
/*N*/         aFrm.Pos( pPrv->Frm().Pos() );
/*N*/         aFrm.Pos().Y() -= Frm().Height();
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         aFrm.Pos( pUp->Frm().Pos() );
/*N*/         aFrm.Pos() += pUp->Prt().Pos();
/*N*/         aFrm.Pos().Y() += pUp->Prt().Height() - aFrm.Height();
/*N*/     }
/*N*/     if( bNotify )
/*N*/         aFrm.Pos().Y() -= 1;
/*N*/ }

/*N*/ void SwFrm::MakeLeftPos( const SwFrm* pUp, const SwFrm* pPrv, BOOL bNotify )
/*N*/ {
/*N*/     if( pPrv )
/*N*/     {
/*N*/         aFrm.Pos( pPrv->Frm().Pos() );
/*N*/         aFrm.Pos().X() -= Frm().Width();
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         aFrm.Pos( pUp->Frm().Pos() );
/*N*/         aFrm.Pos() += pUp->Prt().Pos();
/*N*/         aFrm.Pos().X() += pUp->Prt().Width() - aFrm.Width();
/*N*/     }
/*N*/     if( bNotify )
/*N*/         aFrm.Pos().X() -= 1;
/*N*/ }

/*N*/ void SwFrm::MakeRightPos( const SwFrm* pUp, const SwFrm* pPrv, BOOL bNotify )
/*N*/ {
/*N*/     if( pPrv )
/*N*/     {
/*N*/         aFrm.Pos( pPrv->Frm().Pos() );
/*N*/         aFrm.Pos().X() += pPrv->Frm().Width();
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         aFrm.Pos( pUp->Frm().Pos() );
/*N*/         aFrm.Pos() += pUp->Prt().Pos();
/*N*/     }
/*N*/     if( bNotify )
/*N*/         aFrm.Pos().X() += 1;
/*N*/ }

/*N*/ void SwFrm::SetTopBottomMargins( long nTop, long nBot )
/*N*/ {
/*N*/     Prt().Top( nTop );
/*N*/     Prt().Height( Frm().Height() - nTop - nBot );
/*N*/ }

/*N*/ void SwFrm::SetBottomTopMargins( long nBot, long nTop )
/*N*/ {
/*N*/     Prt().Top( nTop );
/*N*/     Prt().Height( Frm().Height() - nTop - nBot );
/*N*/ }

/*N*/ void SwFrm::SetLeftRightMargins( long nLeft, long nRight)
/*N*/ {
/*N*/     Prt().Left( nLeft );
/*N*/     Prt().Width( Frm().Width() - nLeft - nRight );
/*N*/ }

/*N*/ void SwFrm::SetRightLeftMargins( long nRight, long nLeft)
/*N*/ {
/*N*/     Prt().Left( nLeft );
/*N*/     Prt().Width( Frm().Width() - nLeft - nRight );
/*N*/ }

/*
 * SwFrm::CheckDirChange(..)
 * checks the layout direction and
 * invalidates the lower frames rekursivly, if necessary.
 */

/*N*/ void SwFrm::CheckDirChange()
/*N*/ {
/*N*/     SetInvalidVert( TRUE );
/*N*/     SetInvalidR2L( TRUE );
/*N*/ }

/*
 * SwFrm::GetAnchorPos(..)
 * returns the position for anchors based on frame direction
 */

/*N*/ Point SwFrm::GetFrmAnchorPos( sal_Bool bIgnoreFlysAnchoredAtThisFrame ) const
/*N*/ {
/*N*/     Point aAnchor = Frm().Pos();
/*N*/     if ( IsVertical() || IsRightToLeft() )
/*N*/         aAnchor.X() += Frm().Width();
/*N*/
/*N*/     if ( IsTxtFrm() )
/*N*/     {
/*N*/         SwTwips nBaseOfstForFly =
/*N*/             ((SwTxtFrm*)this)->GetBaseOfstForFly( bIgnoreFlysAnchoredAtThisFrame );
/*N*/         if ( IsVertical() )
/*N*/             aAnchor.Y() += nBaseOfstForFly;
/*N*/         else
/*N*/             aAnchor.X() += nBaseOfstForFly;
/*N*/     }
/*N*/
/*N*/     return aAnchor;
/*N*/ }


/*************************************************************************
|*
|*  SwFrm::~SwFrm()
|*
|*************************************************************************/


/*N*/ SwFrm::~SwFrm()
/*N*/ {
/*N*/   if( pDrawObjs )
/*N*/   {
/*N*/       for ( USHORT i = pDrawObjs->Count(); i; )
/*N*/       {
/*N*/           SdrObject *pObj = (*pDrawObjs)[--i];
/*N*/           if ( pObj->IsWriterFlyFrame() )
/*N*/               delete ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
/*N*/             else
/*N*/             // consider 'virtual' drawing objects
/*N*/             {
/*N*/                 if ( pObj->GetUserCall() )
/*N*/                 {
/*N*/                     ((SwDrawContact*)pObj->GetUserCall())->DisconnectObjFromLayout( pObj );
/*N*/                 }
/*N*/             }
/*N*/       }
/*N*/       if ( pDrawObjs )
/*N*/           delete pDrawObjs;
/*N*/   }
/*N*/
/*N*/ #ifdef DBG_UTIL
/*N*/   // for detection of access to deleted frames
/*N*/   pDrawObjs = (SwDrawObjs*)0x33333333;
/*N*/ #endif
/*N*/ }

/*************************************************************************
|*
|*    SwLayoutFrm::SetFrmFmt()
|*
|*************************************************************************/


/*N*/ void SwLayoutFrm::SetFrmFmt( SwFrmFmt *pNew )
/*N*/ {
/*N*/   if ( pNew != GetFmt() )
/*N*/   {
/*N*/       SwFmtChg aOldFmt( GetFmt() );
/*N*/       pNew->Add( this );
/*N*/       SwFmtChg aNewFmt( pNew );
/*N*/       Modify( &aOldFmt, &aNewFmt );
/*N*/   }
/*N*/ }

/*************************************************************************
|*
|*    SwCntntFrm::SwCntntFrm(), ~SwCntntFrm()
|*
|*************************************************************************/


/*N*/ SwCntntFrm::SwCntntFrm( SwCntntNode * const pCntnt ) :
/*N*/   SwFrm( pCntnt ),
/*N*/   SwFlowFrm( (SwFrm&)*this )
/*N*/ {
/*N*/ }



/*N*/ SwCntntFrm::~SwCntntFrm()
/*N*/ {
/*N*/   SwCntntNode* pCNd;
/*N*/   if( 0 != ( pCNd = PTR_CAST( SwCntntNode, pRegisteredIn )) &&
/*N*/       !pCNd->GetDoc()->IsInDtor() )
/*N*/   {
/*N*/         if( IsTxtFrm() && ((SwTxtFrm*)this)->HasFtn() )
/*N*/         {
/*?*/             SwTxtNode *pTxtNd = ((SwTxtFrm*)this)->GetTxtNode();
/*?*/             const SwFtnIdxs &rFtnIdxs = pCNd->GetDoc()->GetFtnIdxs();
/*?*/             USHORT nPos;
/*?*/             pCNd->GetIndex();
/*?*/             rFtnIdxs.SeekEntry( *pTxtNd, &nPos );
/*?*/             if( nPos < rFtnIdxs.Count() )
/*?*/             {
/*?*/                 while( nPos && pTxtNd == &(rFtnIdxs[ nPos ]->GetTxtNode()) )
/*?*/                     --nPos;
/*?*/                 if( nPos || pTxtNd != &(rFtnIdxs[ nPos ]->GetTxtNode()) )
/*?*/                     ++nPos;
/*?*/             }
/*N*/         }
/*N*/   }
/*N*/   if( IsTxtFrm() && ((SwTxtFrm*)this)->HasBlinkPor() )
/*?*/       ((SwTxtFrm*)this)->ClearPara();
/*N*/ }

/*************************************************************************
|*
|*  SwLayoutFrm::~SwLayoutFrm
|*
|*************************************************************************/


/*N*/ SwLayoutFrm::~SwLayoutFrm()
/*N*/ {
/*N*/   SwFrm *pFrm = pLower;
/*N*/
/*N*/   if( GetFmt() && !GetFmt()->GetDoc()->IsInDtor() )
/*N*/   {
/*N*/         while ( pFrm )
/*N*/       {
/*N*/           //Erst die Objs des Frm vernichten, denn diese koennen sich sonst nach
/*N*/           //dem Remove nicht mehr bei der Seite abmelden.
/*N*/           //Falls sich einer nicht abmeldet wollen wir nicht gleich
/*N*/           //endlos schleifen.
/*N*/
/*N*/             USHORT nCnt;
/*N*/           while ( pFrm->GetDrawObjs() && pFrm->GetDrawObjs()->Count() )
/*N*/           {
/*N*/               nCnt = pFrm->GetDrawObjs()->Count();
/*N*/               SdrObject *pObj = (*pFrm->GetDrawObjs())[0];
/*N*/               if ( pObj->IsWriterFlyFrame() )
/*N*/                   delete ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
/*N*/               else if ( pObj->GetUserCall() )
/*N*/                 {
/*N*/                     // adjustments for drawing objects
/*N*/                     // in header/footer.
/*N*/                     ((SwDrawContact*)pObj->GetUserCall())->DisconnectObjFromLayout( pObj );
/*N*/                 }
/*N*/
/*N*/               if ( pFrm->GetDrawObjs() &&
/*N*/                    nCnt == pFrm->GetDrawObjs()->Count() )
/*N*/                 {
/*N*/                   pFrm->GetDrawObjs()->Remove( 0 );
/*N*/                 }
/*N*/           }
/*N*/           pFrm->Remove();
/*N*/           delete pFrm;
/*N*/           pFrm = pLower;
/*N*/       }
/*N*/       //Fly's vernichten. Der letzte loescht gleich das Array.
/*N*/       USHORT nCnt;
/*N*/       while ( GetDrawObjs() && GetDrawObjs()->Count() )
/*N*/       {
/*N*/           nCnt = GetDrawObjs()->Count();
/*N*/           SdrObject *pObj = (*GetDrawObjs())[0];
/*N*/           if ( pObj->IsWriterFlyFrame() )
/*N*/               delete ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
/*N*/           else if ( pObj->GetUserCall() )
/*N*/             {
/*N*/                 // adjustments for drawing objects
/*N*/                 // in header/footer.
/*N*/                 ((SwDrawContact*)pObj->GetUserCall())->DisconnectObjFromLayout( pObj );
/*N*/             }
/*N*/
/*N*/           if ( GetDrawObjs() && nCnt == GetDrawObjs()->Count() )
/*N*/               GetDrawObjs()->Remove( 0 );
/*N*/       }
/*N*/   }
/*N*/   else
/*N*/   {
/*N*/       while( pFrm )
/*N*/       {
/*N*/           SwFrm *pNxt = pFrm->GetNext();
/*N*/           delete pFrm;
/*N*/           pFrm = pNxt;
/*N*/       }
/*N*/   }
/*N*/ }

/*************************************************************************
|*
|*  SwFrm::UnionFrm()
|*
|*  The unionframe is the framearea (Frm()) of a frame expanded by the
|*  printarea, if there's a negative margin at the left or right side.
|*
|*************************************************************************/

/*N*/ const SwRect SwFrm::UnionFrm( BOOL bBorder ) const
/*N*/ {
/*N*/     BOOL bVert = IsVertical();
/*N*/     SwRectFn fnRect = bVert ? fnRectVert : fnRectHori;
/*N*/     long nLeft = (Frm().*fnRect->fnGetLeft)();
/*N*/     long nWidth = (Frm().*fnRect->fnGetWidth)();
/*N*/     long nPrtLeft = (Prt().*fnRect->fnGetLeft)();
/*N*/     long nPrtWidth = (Prt().*fnRect->fnGetWidth)();
/*N*/     if( nPrtLeft + nPrtWidth > nWidth )
/*?*/         nWidth = nPrtLeft + nPrtWidth;
/*N*/     if( nPrtLeft < 0 )
/*N*/     {
/*N*/         nLeft += nPrtLeft;
/*N*/         nWidth -= nPrtLeft;
/*N*/     }
/*N*/     SwTwips nRight = nLeft + nWidth;
/*N*/     long nAdd = 0;
/*N*/   if( bBorder )
/*N*/   {
/*N*/       SwBorderAttrAccess aAccess( SwFrm::GetCache(), this );
/*N*/       const SwBorderAttrs &rAttrs = *aAccess.Get();
/*N*/       const SvxBoxItem &rBox = rAttrs.GetBox();
/*N*/       if ( rBox.GetLeft() )
/*N*/             nLeft -= rBox.CalcLineSpace( BOX_LINE_LEFT );
/*N*/       else if ( rAttrs.IsBorderDist() )
/*?*/             nLeft -= rBox.GetDistance( BOX_LINE_LEFT ) + 1;
/*N*/       if ( rBox.GetRight() )
/*N*/             nAdd += rBox.CalcLineSpace( BOX_LINE_RIGHT );
/*N*/       else if ( rAttrs.IsBorderDist() )
/*?*/             nAdd += rBox.GetDistance( BOX_LINE_RIGHT ) + 1;
/*N*/       if( rAttrs.GetShadow().GetLocation() != SVX_SHADOW_NONE )
/*N*/       {
/*N*/           const SvxShadowItem &rShadow = rAttrs.GetShadow();
/*N*/             nLeft -= rShadow.CalcShadowSpace( SHADOW_LEFT );
/*N*/             nAdd += rShadow.CalcShadowSpace( SHADOW_RIGHT );
/*N*/       }
/*N*/   }
/*N*/     if( IsTxtFrm() && ((SwTxtFrm*)this)->HasPara() )
/*N*/     {
/*N*/         long nTmp = ((SwTxtFrm*)this)->HangingMargin();
/*N*/         if( nTmp > nAdd )
/*?*/             nAdd = nTmp;
/*N*/     }
/*N*/     nWidth = nRight + nAdd - nLeft;
/*N*/   SwRect aRet( Frm() );
/*N*/     (aRet.*fnRect->fnSetPosX)( nLeft );
/*N*/     (aRet.*fnRect->fnSetWidth)( nWidth );
/*N*/   return aRet;
/*N*/ }




}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
