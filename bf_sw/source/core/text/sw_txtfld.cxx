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


#include <fmtfld.hxx>
#include <txtfld.hxx>
#include <charfmt.hxx>

#include "viewsh.hxx"   // NewFldPortion, GetDoc()

#include <horiornt.hxx>

#include "doc.hxx"      // NewFldPortion, GetSysFldType()
#include "rootfrm.hxx"  // Info ueber virt. PageNumber
#include "pagefrm.hxx"  // NewFldPortion, GetVirtPageNum()
#include "ndtxt.hxx"    // NewNumberPortion, pHints->GetNum()
#include "viewopt.hxx"  // SwViewOptions
#include "flyfrm.hxx"   //IsInBody()
#include "viewimp.hxx"

#include "txtcfg.hxx"


#include "porftn.hxx"   // NewExtraPortion
#include "portox.hxx"   // NewExtraPortion
#include "porhyph.hxx"   // NewExtraPortion
#include "porfly.hxx"   // NewExtraPortion
#include "itrform2.hxx"   // SwTxtFormatter

#include "chpfld.hxx"
#include "dbfld.hxx"
#include "expfld.hxx"
#include "docufld.hxx"
#include "pagedesc.hxx"  // NewFldPortion, GetNum()
namespace binfilter {


/*************************************************************************
 *                      SwTxtFormatter::NewFldPortion()
 *************************************************************************/


/*N*/ sal_Bool lcl_IsInBody( SwFrm *pFrm )
/*N*/ {
/*N*/   if ( pFrm->IsInDocBody() )
/*N*/       return sal_True;
/*N*/   else
/*N*/   {
/*N*/       SwFrm *pTmp = pFrm;
/*N*/       SwFlyFrm *pFly;
/*N*/       while ( 0 != (pFly = pTmp->FindFlyFrm()) )
/*N*/           pTmp = pFly->GetAnchor();
/*N*/       return pTmp->IsInDocBody();
/*N*/   }
/*N*/ }


/*N*/ SwExpandPortion *SwTxtFormatter::NewFldPortion( SwTxtFormatInfo &rInf,
/*N*/                                               const SwTxtAttr *pHint ) const
/*N*/ {
/*N*/   SwExpandPortion *pRet = NULL;
/*N*/   SwFrm *pFrame = (SwFrm*)pFrm;
/*N*/   SwField *pFld = (SwField*)pHint->GetFld().GetFld();
/*N*/   const sal_Bool bName = rInf.GetOpt().IsFldName();
/*N*/
/*N*/   SwCharFmt* pChFmt = 0;
/*N*/   sal_Bool bNewFlyPor = sal_False,
/*N*/        bINet = sal_False;
/*N*/
/*N*/   // Sprache setzen
/*N*/   ((SwTxtFormatter*)this)->SeekAndChg( rInf );
/*N*/   pFld->SetLanguage( GetFnt()->GetLanguage() );
/*N*/
/*N*/   ViewShell *pSh = rInf.GetVsh();
/*N*/
/*N*/   switch( pFld->GetTyp()->Which() )
/*N*/   {
/*N*/       case RES_SCRIPTFLD:
/*N*/       case RES_POSTITFLD:
/*?*/           break;
/*?*/
/*?*/       case RES_COMBINED_CHARS:
/*?*/           {
/*?*/               String sStr( pFld->GetCntnt( bName ));
/*?*/               if( bName )
/*?*/                   pRet = new SwFldPortion( sStr );
/*?*/           }
/*?*/           break;
/*N*/
/*N*/       case RES_HIDDENTXTFLD:
/*N*/           pRet = new SwHiddenPortion(pFld->GetCntnt( bName ));
/*N*/           break;
/*N*/
/*N*/       case RES_CHAPTERFLD:
/*N*/           if( !bName && pSh )
/*N*/           {
/*N*/               ((SwChapterField*)pFld)->ChangeExpansion( pFrame,
/*N*/                                       &((SwTxtFld*)pHint)->GetTxtNode() );
/*N*/           }
/*N*/           pRet = new SwFldPortion( pFld->GetCntnt( bName ) );
/*N*/           break;
/*N*/
/*N*/       case RES_DOCSTATFLD:
/*N*/           if( !bName && pSh )
/*N*/               ((SwDocStatField*)pFld)->ChangeExpansion( pFrame );
/*N*/           pRet = new SwFldPortion( pFld->GetCntnt( bName ) );
/*N*/           break;
/*N*/
/*N*/       case RES_PAGENUMBERFLD:
/*N*/       {
/*N*/           if( !bName && pSh )
/*N*/           {
/*N*/               SwDoc* pDoc = pSh->GetDoc();
/*N*/               SwPageNumberFieldType *pPageNr = (SwPageNumberFieldType *)
/*N*/                               pFld->GetTyp();
/*N*/
/*N*/               MSHORT nVirtNum = pFrame->GetVirtPageNum();
/*N*/               sal_Int16 nNumFmt = -1;
/*N*/               if(SVX_NUM_PAGEDESC == pFld->GetFormat())
/*N*/                   nNumFmt = pFrame->FindPageFrm()->GetPageDesc()->GetNumType().GetNumberingType();
/*N*/
/*N*/               pPageNr->ChangeExpansion( pDoc, nVirtNum, 0,
/*N*/                                           false, nNumFmt > -1 ? &nNumFmt : 0);
/*N*/           }
/*N*/           pRet = new SwFldPortion( pFld->GetCntnt( bName ) );
/*N*/           break;
/*N*/       }
/*N*/       case RES_GETEXPFLD:
/*N*/       {
/*N*/           pRet = new SwFldPortion( pFld->GetCntnt( bName ) );
/*N*/           break;
/*N*/       }
/*N*/       case RES_DBFLD:
/*N*/       {
/*N*/           if( !bName )
/*N*/           {
/*N*/               SwDBField* pDBFld = (SwDBField*)pFld;
/*N*/               pDBFld->ChgBodyTxtFlag( ::binfilter::lcl_IsInBody( pFrame ) );
/*N*/           }
/*N*/           pRet = new SwFldPortion( pFld->GetCntnt( bName ) );
/*N*/           break;
/*N*/       }
/*?*/       case RES_REFPAGEGETFLD:
/*?*/           pRet = new SwFldPortion( pFld->GetCntnt( bName ) );
/*?*/           break;
/*N*/
/*N*/       case RES_JUMPEDITFLD:
/*N*/           if( !bName )
/*N*/               pChFmt =  ((SwJumpEditField*)pFld)->GetCharFmt();
/*N*/           bNewFlyPor = sal_True;
/*N*/           break;
/*N*/
/*N*/       default:
/*N*/       {
/*N*/           pRet = new SwFldPortion(pFld->GetCntnt( bName ) );
/*N*/       }
/*N*/   }
/*N*/
/*N*/   if( bNewFlyPor )
/*N*/   {
/*N*/       SwFont *pTmpFnt = 0;
/*N*/       if( !bName )
/*N*/       {
/*N*/           pTmpFnt = new SwFont( *pFnt );
/*N*/           if( bINet )
/*N*/           {
/*?*/               SwAttrPool* pPool = pChFmt->GetAttrSet().GetPool();
/*?*/               SfxItemSet aSet( *pPool, RES_CHRATR_BEGIN, RES_CHRATR_END );
/*?*/               SfxItemSet aTmpSet( aSet );
/*?*/               pFrm->GetTxtNode()->GetAttr(aSet,rInf.GetIdx(),rInf.GetIdx()+1);
/*?*/               aTmpSet.Set( pChFmt->GetAttrSet() );
/*?*/               aTmpSet.Differentiate( aSet );
/*?*/               if( aTmpSet.Count() )
/*?*/                     pTmpFnt->SetDiffFnt( &aTmpSet, rInf.GetDoc() );
/*N*/           }
/*N*/           else
/*N*/                 pTmpFnt->SetDiffFnt( &pChFmt->GetAttrSet(), rInf.GetDoc() );
/*N*/       }
/*N*/       pRet = new SwFldPortion( pFld->GetCntnt( bName ), pTmpFnt );
/*N*/   }
/*N*/
/*N*/   return pRet;
/*N*/ }


/*************************************************************************
 *                      SwTxtFormatter::NewExtraPortion()
 *************************************************************************/


/*N*/ SwLinePortion *SwTxtFormatter::NewExtraPortion( SwTxtFormatInfo &rInf )
/*N*/ {
/*N*/   SwTxtAttr *pHint = GetAttr( rInf.GetIdx() );
/*N*/   SwLinePortion *pRet = 0;
/*N*/   if( !pHint )
/*N*/   {
/*?*/       pRet = new SwTxtPortion;
/*?*/       pRet->SetLen( 1 );
/*?*/       rInf.SetLen( 1 );
/*?*/       return pRet;
/*N*/   }
/*N*/
/*N*/   switch( pHint->Which() )
/*N*/   {
/*N*/       case RES_TXTATR_FLYCNT :
/*N*/       {
/*N*/           pRet = NewFlyCntPortion( rInf, pHint );
/*N*/           break;
/*N*/       }
/*N*/       case RES_TXTATR_FTN :
/*N*/       {
/*N*/           pRet = NewFtnPortion( rInf, pHint );
/*N*/           break;
/*N*/       }
/*?*/       case RES_TXTATR_SOFTHYPH :
/*?*/       {
/*?*/           pRet = new SwSoftHyphPortion;
/*?*/           break;
/*?*/       }
/*?*/       case RES_TXTATR_HARDBLANK :
/*N*/       case RES_TXTATR_FIELD :
/*N*/       {
/*N*/           pRet = NewFldPortion( rInf, pHint );
/*N*/           break;
/*N*/       }
/*?*/       case RES_TXTATR_REFMARK :
/*N*/       case RES_TXTATR_TOXMARK :
/*N*/       {
/*N*/           pRet = new SwIsoToxPortion;
/*N*/           break;
/*N*/       }
/*N*/       default: ;
/*N*/   }
/*N*/   if( !pRet )
/*N*/   {
/*?*/       const XubString aNothing;
/*?*/       pRet = new SwFldPortion( aNothing );
/*?*/       rInf.SetLen( 1 );
/*N*/   }
/*N*/   return pRet;
/*N*/ }

/*************************************************************************
 *                      SwTxtFormatter::NewNumberPortion()
 *************************************************************************/


/*N*/ SwNumberPortion *SwTxtFormatter::NewNumberPortion( SwTxtFormatInfo &rInf ) const
/*N*/ {
/*N*/   if( rInf.IsNumDone() || rInf.GetTxtStart() != nStart
/*N*/               || rInf.GetTxtStart() != rInf.GetIdx() )
/*N*/       return 0;
/*N*/
/*N*/   SwNumberPortion *pRet = 0;
/*N*/   const SwTxtNode* pTxtNd = GetTxtFrm()->GetTxtNode();
/*N*/   const SwNumRule* pNumRule = pTxtNd->GetNumRule();
/*N*/   const SwNodeNum* pNum = pTxtNd->GetNum();
/*N*/
/*N*/   if( !pNumRule )     // oder sollte OutlineNum an sein?
/*N*/   {
/*N*/       pNum = pTxtNd->GetOutlineNum();
/*N*/       if( pNum )
/*N*/           pNumRule = pTxtNd->GetDoc()->GetOutlineNumRule();
/*N*/   }
/*N*/   // hat ein "gueltige" Nummer ?
/*N*/   if( pNumRule && pNum && MAXLEVEL > pNum->GetLevel() )
/*N*/   {
/*N*/       CONST SwNumFmt &rNumFmt = pNumRule->Get( pNum->GetLevel() );
/*N*/       const sal_Bool bLeft = SVX_ADJUST_LEFT == rNumFmt.GetNumAdjust();
/*N*/       const sal_Bool bCenter = SVX_ADJUST_CENTER == rNumFmt.GetNumAdjust();
/*N*/       const KSHORT nMinDist = rNumFmt.GetCharTextDistance();
/*N*/
/*N*/       if( SVX_NUM_BITMAP == rNumFmt.GetNumberingType() )
/*N*/       {
/*?*/           pRet = new SwGrfNumPortion( (SwFrm*)GetTxtFrm(),rNumFmt.GetBrush(),
/*?*/               NULL, rNumFmt.GetGraphicSize(), bLeft, bCenter, nMinDist );
/*N*/       }
/*N*/       else
/*N*/       {
/*N*/           // Der SwFont wird dynamisch angelegt und im CTOR uebergeben,
/*N*/           // weil das CharFmt nur einen SV-Font zurueckliefert.
/*N*/           // Im Dtor vom SwNumberPortion wird der SwFont deletet.
/*N*/           SwFont *pNumFnt = 0;
/*N*/           const SwAttrSet* pFmt = rNumFmt.GetCharFmt() ?
/*N*/               &rNumFmt.GetCharFmt()->GetAttrSet() : NULL;
/*N*/           if( SVX_NUM_CHAR_SPECIAL == rNumFmt.GetNumberingType() )
/*N*/           {
/*N*/               const Font *pFmtFnt = rNumFmt.GetBulletFont();
/*N*/                 pNumFnt = new SwFont( &rInf.GetCharAttr(), rInf.GetDoc() );
/*N*/               if( pFmt )
/*N*/                     pNumFnt->SetDiffFnt( pFmt, rInf.GetDoc() );
/*N*/               if ( pFmtFnt )
/*N*/               {
/*N*/                   const BYTE nAct = pNumFnt->GetActual();
/*N*/                   pNumFnt->SetFamily( pFmtFnt->GetFamily(), nAct );
/*N*/                   pNumFnt->SetName( pFmtFnt->GetName(), nAct );
/*N*/                   pNumFnt->SetStyleName( pFmtFnt->GetStyleName(), nAct );
/*N*/                   pNumFnt->SetCharSet( pFmtFnt->GetCharSet(), nAct );
/*N*/                   pNumFnt->SetPitch( pFmtFnt->GetPitch(), nAct );
/*N*/               }
/*N*/                 // we do not allow a vertical font
/*N*/ #ifdef VERTICAL_LAYOUT
/*N*/                 pNumFnt->SetVertical( pNumFnt->GetOrientation(),
/*N*/                                       pFrm->IsVertical() );
/*N*/ #else
/*N*/                 pNumFnt->SetVertical( 0 );
/*N*/ #endif
/*N*/
/*N*/               pRet = new SwBulletPortion( rNumFmt.GetBulletChar(), pNumFnt, bLeft,
/*N*/                           bCenter, nMinDist );
/*N*/           }
/*N*/           else
/*N*/           {
/*N*/               XubString aTxt( pNumRule->MakeNumString( *pNum ) );
/*N*/
/*N*/               // 7974: Nicht nur eine Optimierung...
/*N*/               // Eine Numberportion ohne Text wird die Breite von 0
/*N*/               // erhalten. Die nachfolgende Textportion wird im BreakLine
/*N*/               // in das BreakCut laufen, obwohl rInf.GetLast()->GetFlyPortion()
/*N*/               // vorliegt!
/*N*/               if( aTxt.Len() )
/*N*/               {
/*N*/                     pNumFnt = new SwFont( &rInf.GetCharAttr(), rInf.GetDoc() );
/*N*/                   if( pFmt )
/*N*/                         pNumFnt->SetDiffFnt( pFmt, rInf.GetDoc() );
/*N*/                   // Die SSize muss erhalten bleiben
/*N*/                   // pNumFnt->ChangeSize( rInf.GetFont()->GetSize() );
/*N*/
/*N*/                     // we do not allow a vertical font
/*N*/ #ifdef VERTICAL_LAYOUT
/*N*/                     pNumFnt->SetVertical( pNumFnt->GetOrientation(), pFrm->IsVertical() );
/*N*/ #else
/*N*/                     pNumFnt->SetVertical( 0 );
/*N*/ #endif
/*N*/
/*N*/                     pRet = new SwNumberPortion( aTxt, pNumFnt, bLeft, bCenter,
/*N*/                                               nMinDist );
/*N*/               }
/*N*/           }
/*N*/       }
/*N*/   }
/*N*/   return pRet;
/*N*/ }

void SwTxtFld::NotifyContentChange(SwFmtFld& rFmtFld)
{
    //if not in undo section notify the change
    if(pMyTxtNd && pMyTxtNd->GetNodes().IsDocNodes())
        pMyTxtNd->Modify(0, &rFmtFld);
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
