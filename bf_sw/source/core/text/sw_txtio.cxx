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

#ifndef DBG_UTIL
#error Wer fummelt denn an den makefiles?
#endif

#define CONSTCHAR( name, string ) static const sal_Char name[] = string

#include <horiornt.hxx>

#include "flyfrms.hxx"
#include "inftxt.hxx"
#include "porfly.hxx"
#include "porftn.hxx"
#include "porhyph.hxx"
#include "porref.hxx"
#include "porrst.hxx"
#include "portab.hxx"
#include "portox.hxx"
#include "pordrop.hxx"
#include "pormulti.hxx"

// So kann man die Layoutstruktur ausgeben lassen
// #define AMA_LAYOUT
#ifdef AMA_LAYOUT
#include <stdio.h>
#include <stdlib.h>         // getenv()
namespace binfilter {


/*N*/ void lcl_OutFollow( XubString &rTmp, const SwFrm* pFrm )
/*N*/ {
/*N*/   if( pFrm->IsFlowFrm() )
/*N*/   {
/*N*/       const SwFlowFrm *pFlow = SwFlowFrm::CastFlowFrm( pFrm );
/*N*/       if( pFlow->IsFollow() || pFlow->GetFollow() )
/*N*/       {
/*N*/           rTmp += "(";
/*N*/           if( pFlow->IsFollow() )
/*N*/               rTmp += ".";
/*N*/           if( pFlow->GetFollow() )
/*N*/           {
/*N*/               MSHORT nFrmId = pFlow->GetFollow()->GetFrm()->GetFrmId();
/*N*/               rTmp += nFrmId;
/*N*/           }
/*N*/           rTmp += ")";
/*N*/       }
/*N*/   }
/*N*/ }

/*N*/ void lcl_OutFrame( SvFileStream& rStr, const SwFrm* pFrm, ByteString& rSp, sal_Bool bNxt )
/*N*/ {
/*N*/   if( !pFrm )
/*N*/       return;
/*N*/   KSHORT nSpc = 0;
/*N*/   MSHORT nFrmId = pFrm->GetFrmId();
/*N*/   ByteString aTmp;
/*N*/   if( pFrm->IsLayoutFrm() )
/*N*/   {
/*N*/       if( pFrm->IsRootFrm() )
/*N*/           aTmp = "R";
/*N*/       else if( pFrm->IsPageFrm() )
/*N*/           aTmp = "P";
/*N*/       else if( pFrm->IsBodyFrm() )
/*N*/           aTmp = "B";
/*N*/       else if( pFrm->IsColumnFrm() )
/*N*/           aTmp = "C";
/*N*/       else if( pFrm->IsTabFrm() )
/*N*/           aTmp = "Tb";
/*N*/       else if( pFrm->IsRowFrm() )
/*N*/           aTmp = "Rw";
/*N*/       else if( pFrm->IsCellFrm() )
/*N*/           aTmp = "Ce";
/*N*/       else if( pFrm->IsSctFrm() )
/*N*/           aTmp = "S";
/*N*/       else if( pFrm->IsFlyFrm() )
/*N*/       {
/*N*/           aTmp = "F";
/*N*/           const SwFlyFrm *pFly = (SwFlyFrm*)pFrm;
/*N*/           if( pFly->IsFlyInCntFrm() )
/*N*/               aTmp += "in";
/*N*/           else if( pFly->IsFlyAtCntFrm() )
/*N*/           {
/*N*/               aTmp += "a";
/*N*/               if( pFly->IsAutoPos() )
/*N*/                   aTmp += "u";
/*N*/               else
/*N*/                   aTmp += "t";
/*N*/           }
/*N*/           else
/*N*/               aTmp += "l";
/*N*/       }
/*N*/       else if( pFrm->IsHeaderFrm() )
/*N*/           aTmp = "H";
/*N*/       else if( pFrm->IsFooterFrm() )
/*N*/           aTmp = "Fz";
/*N*/       else if( pFrm->IsFtnContFrm() )
/*N*/           aTmp = "Fc";
/*N*/       else if( pFrm->IsFtnFrm() )
/*N*/           aTmp = "Fn";
/*N*/       else
/*N*/           aTmp = "?L?";
/*N*/       aTmp += nFrmId;
/*N*/       lcl_OutFollow( aTmp, pFrm );
/*N*/       aTmp += " ";
/*N*/       rStr << aTmp;
/*N*/       nSpc = aTmp.Len();
/*N*/       rSp.Expand( nSpc + rSp.Len() );
/*N*/       lcl_OutFrame( rStr, ((SwLayoutFrm*)pFrm)->Lower(), rSp, sal_True );
/*N*/   }
/*N*/   else
/*N*/   {
/*N*/       if( pFrm->IsTxtFrm() )
/*N*/           aTmp = "T";
/*N*/       else if( pFrm->IsNoTxtFrm() )
/*N*/           aTmp = "N";
/*N*/       else
/*N*/           aTmp = "?C?";
/*N*/       aTmp += nFrmId;
/*N*/       lcl_OutFollow( aTmp, pFrm );
/*N*/       aTmp += " ";
/*N*/       rStr << aTmp;
/*N*/       nSpc = aTmp.Len();
/*N*/       rSp.Expand( nSpc + rSp.Len() );
/*N*/   }
/*N*/   if( pFrm->IsPageFrm() )
/*N*/   {
/*N*/       const SwPageFrm* pPg = (SwPageFrm*)pFrm;
/*N*/       const SwSortDrawObjs *pSorted = pPg->GetSortedObjs();
/*N*/       const MSHORT nCnt = pSorted ? pSorted->Count() : 0;
/*N*/       if( nCnt )
/*N*/       {
/*N*/           for( MSHORT i=0; i < nCnt; ++i )
/*N*/           {
/*N*/               SdrObject *pObj = (*pSorted)[ i ];
/*N*/               if( pObj->IsWriterFlyFrame() )
/*N*/               {
/*N*/                   SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
/*N*/                   lcl_OutFrame( rStr, pFly, rSp, sal_False );
/*N*/               }
/*N*/               else
/*N*/               {
/*N*/                   aTmp = pObj->IsUnoObj() ? "UNO" : "Drw";
/*N*/                   rStr << aTmp;
/*N*/               }
/*N*/               if( i < nCnt - 1 )
/*N*/                   rStr << endl << rSp;
/*N*/           }
/*N*/       }
/*N*/   }
/*N*/   else if( pFrm->GetDrawObjs() )
/*N*/   {
/*N*/       MSHORT nCnt = pFrm->GetDrawObjs()->Count();
/*N*/       if( nCnt )
/*N*/       {
/*N*/           for( MSHORT i=0; i < nCnt; ++i )
/*N*/           {
/*N*/               SdrObject *pObj = (*pFrm->GetDrawObjs())[ i ];
/*N*/               if( pObj->IsWriterFlyFrame() )
/*N*/               {
/*N*/                   SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
/*N*/                   lcl_OutFrame( rStr, pFly, rSp, sal_False );
/*N*/               }
/*N*/               else
/*N*/               {
/*N*/                   aTmp = pObj->IsUnoObj() ? "UNO" : "Drw";
/*N*/                   rStr << aTmp;
/*N*/               }
/*N*/               if( i < nCnt - 1 )
/*N*/                   rStr << endl << rSp;
/*N*/           }
/*N*/       }
/*N*/   }
/*N*/   if( nSpc )
/*N*/       rSp.Erase( rSp.Len() - nSpc );
/*N*/   if( bNxt && pFrm->GetNext() )
/*N*/   {
/*N*/       do
/*N*/       {
/*N*/           pFrm = pFrm->GetNext();
/*N*/           rStr << endl << rSp;
/*N*/           lcl_OutFrame( rStr, pFrm, rSp, sal_False );
/*N*/       } while ( pFrm->GetNext() );
/*N*/   }
/*N*/ }

/*N*/ #ifdef USED
/*N*/       IsFtnContFrm()
/*N*/       IsFtnFrm()
/*N*/ #endif

/*N*/ void LayOutPut( const SwFrm* pFrm )
/*N*/ {
/*N*/ #ifndef MAC
/*N*/   static char *pOutName = 0;
/*N*/   const sal_Bool bFirstOpen = pOutName ? sal_False : sal_True;
/*N*/   if( bFirstOpen )
/*N*/   {
/*N*/       char *pPath = getenv( "TEMP" );
/*N*/       char *pName = "layout.txt";
/*N*/       if( !pPath )
/*N*/           pOutName = pName;
/*N*/       else
/*N*/       {
/*N*/           const int nLen = strlen(pPath);
/*N*/           // fuer dieses new wird es kein delete geben.
/*N*/           pOutName = new char[nLen + strlen(pName) + 3];
/*N*/           if(nLen && (pPath[nLen-1] == '\\') || (pPath[nLen-1] == '/'))
/*N*/               snprintf( pOutName, sizeof(pOutName), "%s%s", pPath, pName );
/*N*/           else
/*N*/               snprintf( pOutName, sizeof(pOutName), "%s/%s", pPath, pName );
/*N*/       }
/*N*/   }
/*N*/   SvFileStream aStream( pOutName, (bFirstOpen
/*N*/                                       ? STREAM_WRITE | STREAM_TRUNC
/*N*/                                       : STREAM_WRITE ));
/*N*/
/*N*/   if( !aStream.GetError() )
/*N*/   {
/*N*/       if ( bFirstOpen )
/*N*/           aStream << "Layout-Struktur";
/*N*/       else
/*N*/           aStream.Seek( STREAM_SEEK_TO_END );
/*N*/       aStream << endl;
/*N*/       aStream << "---------------------------------------------" << endl;
/*N*/       XubString aSpace;
/*N*/       lcl_OutFrame( aStream, pFrm, aSpace, sal_False );
/*N*/   }
/*N*/ #endif
/*N*/ }
} //namespace binfilter
/*N*/ #endif
namespace binfilter {
/*N*/ SvStream &operator<<( SvStream &rOs, const SwpHints & )
/*N*/ {
/*N*/   rOs << " {HINTS:";
/*N*/   rOs << '}';
/*N*/   return rOs;
/*N*/ }
} //namespace binfilter

/*N*/ #if OSL_DEBUG_LEVEL < 2
namespace binfilter {
/*N*/ static void Error()
/*N*/ {
/*N*/   // wegen PM und BCC
/*N*/   sal_Bool bFalse = sal_False;
/*N*/   OSL_ENSURE( bFalse, "txtio: No debug version" );
/*N*/ }

#define IMPL_OUTOP(class) \
        SvStream &class::operator<<( SvStream &rOs ) const /*$ostream*/\
        { \
            Error(); \
            return rOs; \
        }

/*N*/ IMPL_OUTOP( SwTxtPortion )
/*N*/ IMPL_OUTOP( SwLinePortion )
/*N*/ IMPL_OUTOP( SwBreakPortion )
/*N*/ IMPL_OUTOP( SwGluePortion )
/*N*/ IMPL_OUTOP( SwFldPortion )
/*N*/ IMPL_OUTOP( SwHiddenPortion )
/*N*/ IMPL_OUTOP( SwHyphPortion )
/*N*/ IMPL_OUTOP( SwFixPortion )
/*N*/ IMPL_OUTOP( SwFlyPortion )
/*N*/ IMPL_OUTOP( SwFlyCntPortion )
/*N*/ IMPL_OUTOP( SwMarginPortion )
/*N*/ IMPL_OUTOP( SwNumberPortion )
/*N*/ IMPL_OUTOP( SwBulletPortion )
/*N*/ IMPL_OUTOP( SwGrfNumPortion )
/*N*/ IMPL_OUTOP( SwLineLayout )
/*N*/ IMPL_OUTOP( SwParaPortion )
/*N*/ IMPL_OUTOP( SwFtnPortion )
/*N*/ IMPL_OUTOP( SwFtnNumPortion )
/*N*/ IMPL_OUTOP( SwHyphStrPortion )
/*N*/ IMPL_OUTOP( SwExpandPortion )
/*N*/ IMPL_OUTOP( SwBlankPortion )
/*N*/ IMPL_OUTOP( SwToxPortion )
/*N*/ IMPL_OUTOP( SwRefPortion )
/*N*/ IMPL_OUTOP( SwIsoToxPortion )
/*N*/ IMPL_OUTOP( SwSoftHyphPortion )
/*N*/ IMPL_OUTOP( SwTabPortion )
/*N*/ IMPL_OUTOP( SwTabLeftPortion )
/*N*/ IMPL_OUTOP( SwTabRightPortion )
/*N*/ IMPL_OUTOP( SwTabCenterPortion )
/*N*/ IMPL_OUTOP( SwTabDecimalPortion )
/*N*/ IMPL_OUTOP( SwQuoVadisPortion )
/*N*/ IMPL_OUTOP( SwErgoSumPortion )
/*N*/ IMPL_OUTOP( SwHolePortion )
/*N*/ IMPL_OUTOP( SwDropPortion )
/*N*/ IMPL_OUTOP( SwKernPortion )
/*N*/ IMPL_OUTOP( SwArrowPortion )
/*N*/ IMPL_OUTOP( SwMultiPortion )
} //namespace binfilter
#else
# include <limits.h>
# include <stdlib.h>
# include "swtypes.hxx"      // ZTCCONST
# include "swfont.hxx"     // SwDropPortion
namespace binfilter {
/*N*/ CONSTCHAR( pClose, "} " );

/*N*/ SvStream &SwLinePortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/   rOs << " {";
/*N*/   rOs <<  "L:" << nLineLength;
/*N*/   rOs << " H:" << Height();
/*N*/   rOs << " W:" << PrtWidth();
/*N*/   rOs << " A:" << nAscent;
/*N*/   rOs << pClose;
/*N*/   return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwTxtPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/   CONSTCHAR( pTxt, " {TXT:" );
/*N*/   rOs << pTxt;
/*N*/   SwLinePortion::operator<<( rOs );
/*N*/   rOs << pClose;
/*N*/   return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwBreakPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/   CONSTCHAR( pTxt, " {BREAK:" );
/*N*/   rOs << pTxt;
/*N*/   SwLinePortion::operator<<( rOs );
/*N*/   rOs << pClose;
/*N*/   return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwKernPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/   CONSTCHAR( pTxt, " {KERN:" );
/*N*/   rOs << pTxt;
/*N*/   SwLinePortion::operator<<( rOs );
/*N*/   rOs << pClose;
/*N*/   return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwArrowPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/   CONSTCHAR( pTxt, " {ARROW:" );
/*N*/   rOs << pTxt;
/*N*/   SwLinePortion::operator<<( rOs );
/*N*/   rOs << pClose;
/*N*/   return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwMultiPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/   CONSTCHAR( pTxt, " {MULTI:" );
/*N*/   rOs << pTxt;
/*N*/   SwLinePortion::operator<<( rOs );
/*N*/   rOs << pClose;
/*N*/   return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwLineLayout::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/   CONSTCHAR( pTxt, " {LINE:" );
/*N*/   rOs << pTxt;
/*N*/   SwLinePortion::operator<<( rOs );
/*N*/   SwLinePortion *pPos = GetPortion();
/*N*/   while( pPos )
/*N*/   {
///*N*/         DBG_LOOP;
/*N*/       rOs << "\t";
/*N*/       pPos->operator<<( rOs );
/*N*/       pPos = pPos->GetPortion();
/*N*/   }
/*N*/   rOs << pClose;
/*N*/   return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwGluePortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/   CONSTCHAR( pTxt, " {GLUE:" );
/*N*/   rOs << pTxt;
/*N*/   SwLinePortion::operator<<( rOs );
/*N*/   rOs << " F:" << GetFixWidth();
/*N*/   rOs << " G:" << GetPrtGlue();
/*N*/   rOs << pClose;
/*N*/   return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwFixPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/   CONSTCHAR( pTxt, " {FIX:" );
/*N*/   rOs << pTxt;
/*N*/   SwGluePortion::operator<<( rOs );
/*N*/   rOs << " Fix:" << nFix;
/*N*/   rOs << pClose;
/*N*/   return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwFlyPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/   CONSTCHAR( pTxt, " {FLY:" );
/*N*/   rOs << pTxt;
/*N*/   SwFixPortion::operator<<( rOs );
/*N*/   rOs << pClose;
/*N*/   return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwMarginPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/   CONSTCHAR( pTxt, " {MAR:" );
/*N*/   rOs << pTxt;
/*N*/   SwGluePortion::operator<<( rOs );
/*N*/   rOs << pClose;
/*N*/   return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwFlyCntPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/   {
/*N*/       CONSTCHAR( pTxt, " {FLYCNT:" );
/*N*/       rOs << pTxt;
/*N*/   }
/*N*/   SwLinePortion::operator<<( rOs );
/*N*/   if( bDraw )
/*N*/   {
/*N*/       CONSTCHAR( pTxt, " {DRAWINCNT" );
/*N*/       rOs << pTxt;
/*N*/       rOs << pClose;
/*N*/   }
/*N*/   else
/*N*/   {
/*N*/       CONSTCHAR( pTxt, " {FRM:" );
/*N*/       rOs << pTxt;
/*N*/       rOs << " {FRM:" << GetFlyFrm()->Frm() << pClose;
/*N*/       rOs << " {PRT:" << GetFlyFrm()->Prt() << pClose;
/*N*/       rOs << pClose;
/*N*/   }
/*N*/   rOs << pClose;
/*N*/   return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwExpandPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/   CONSTCHAR( pTxt, " {EXP:" );
/*N*/   rOs << pTxt;
/*N*/   SwLinePortion::operator<<( rOs );
/*N*/   rOs << pClose;
/*N*/   return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwFtnPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/   CONSTCHAR( pTxt, " {FTN:" );
/*N*/   rOs << pTxt;
/*N*/   SwExpandPortion::operator<<( rOs );
/*N*/   rOs << pClose;
/*N*/   return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwFtnNumPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/   CONSTCHAR( pTxt, " {FTNNUM:" );
/*N*/   rOs << pTxt;
/*N*/   SwNumberPortion::operator<<( rOs );
/*N*/   rOs << pClose;
/*N*/   return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwNumberPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/   CONSTCHAR( pTxt, " {NUMBER:" );
/*N*/   rOs << pTxt;
/*N*/   SwExpandPortion::operator<<( rOs );
/*N*/   rOs << " Exp:\"" << '\"';
/*N*/   rOs << pClose;
/*N*/   return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwBulletPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/   CONSTCHAR( pTxt, " {BULLET:" );
/*N*/   rOs << pTxt;
/*N*/   SwNumberPortion::operator<<( rOs );
/*N*/   rOs << pClose;
/*N*/   return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwGrfNumPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/   CONSTCHAR( pTxt, " {GRFNUM:" );
/*N*/   rOs << pTxt;
/*N*/   SwGrfNumPortion::operator<<( rOs );
/*N*/   rOs << pClose;
/*N*/   return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwHiddenPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/   CONSTCHAR( pTxt, " {Hidden:" );
/*N*/   rOs << pTxt;
/*N*/   SwFldPortion::operator<<( rOs );
/*N*/   rOs << pClose;
/*N*/   return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwToxPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/   CONSTCHAR( pTxt, " {TOX:" );
/*N*/   rOs << pTxt;
/*N*/   SwTxtPortion::operator<<( rOs );
/*N*/   rOs << pClose;
/*N*/   return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwRefPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/   CONSTCHAR( pTxt, " {Ref:" );
/*N*/   rOs << pTxt;
/*N*/   SwTxtPortion::operator<<( rOs );
/*N*/   rOs << pClose;
/*N*/   return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwIsoToxPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/   CONSTCHAR( pTxt, " {ISOTOX:" );
/*N*/   rOs << pTxt;
/*N*/   SwToxPortion::operator<<( rOs );
/*N*/   rOs << pClose;
/*N*/   return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwHyphPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/   CONSTCHAR( pTxt, " {HYPH:" );
/*N*/   rOs << pTxt;
/*N*/   SwExpandPortion::operator<<( rOs );
/*N*/   rOs << pClose;
/*N*/   return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwHyphStrPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/   CONSTCHAR( pTxt, " {HYPHSTR:" );
/*N*/   rOs << pTxt;
/*N*/   SwExpandPortion::operator<<( rOs );
/*N*/   rOs << pClose;
/*N*/   return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwSoftHyphPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/   CONSTCHAR( pTxt, " {SOFTHYPH:" );
/*N*/   rOs << pTxt;
/*N*/   SwHyphPortion::operator<<( rOs );
/*N*/   rOs << (IsExpand() ? " on" : " off");
/*N*/   rOs << pClose;
/*N*/   return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwBlankPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/   CONSTCHAR( pTxt, " {BLANK:" );
/*N*/   rOs << pTxt;
/*N*/   SwExpandPortion::operator<<( rOs );
/*N*/   rOs << pClose;
/*N*/   return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwFldPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/   CONSTCHAR( pTxt, " {FLD:" );
/*N*/   rOs << pTxt;
/*N*/   SwLinePortion::operator<<( rOs );
/*N*/   if( IsFollow() )
/*N*/       rOs << " F!";
/*N*/   rOs << pClose;
/*N*/   return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwTabPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/   CONSTCHAR( pTxt, " {TAB" );
/*N*/   rOs << pTxt;
/*N*/   SwFixPortion::operator<<( rOs );
/*N*/   rOs << " T:" << nTabPos;
/*N*/   if( IsFilled() )
/*N*/       rOs << " \"" << cFill << '\"';
/*N*/   rOs << pClose;
/*N*/   return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwTabLeftPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/   CONSTCHAR( pTxt, " {TABLEFT" );
/*N*/   rOs << pTxt;
/*N*/   SwTabPortion::operator<<( rOs );
/*N*/   rOs << pClose;
/*N*/   return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwTabRightPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/   CONSTCHAR( pTxt, " {TABRIGHT" );
/*N*/   rOs << pTxt;
/*N*/   SwTabPortion::operator<<( rOs );
/*N*/   rOs << pClose;
/*N*/   return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwTabCenterPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/   CONSTCHAR( pTxt, " {TABCENTER" );
/*N*/   rOs << pTxt;
/*N*/   SwTabPortion::operator<<( rOs );
/*N*/   rOs << pClose;
/*N*/   return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwTabDecimalPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/   CONSTCHAR( pTxt, " {TABDECIMAL" );
/*N*/   rOs << pTxt;
/*N*/   SwTabPortion::operator<<( rOs );
/*N*/   rOs << pClose;
/*N*/   return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwParaPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/   CONSTCHAR( pTxt, " {PAR" );
/*N*/   rOs << pTxt;
/*N*/   SwLineLayout::operator<<( rOs );
/*N*/   rOs << pClose;
/*N*/   return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwHolePortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/   CONSTCHAR( pTxt, " {HOLE" );
/*N*/   rOs << pTxt;
/*N*/   SwLinePortion::operator<<( rOs );
/*N*/   rOs << pClose;
/*N*/   return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwQuoVadisPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/   CONSTCHAR( pTxt, " {QUOVADIS" );
/*N*/   rOs << pTxt;
/*N*/   SwFldPortion::operator<<( rOs );
/*N*/   rOs << pClose;
/*N*/   return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwErgoSumPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/   CONSTCHAR( pTxt, " {ERGOSUM" );
/*N*/   rOs << pTxt;
/*N*/   SwFldPortion::operator<<( rOs );
/*N*/   rOs << pClose;
/*N*/   return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &operator<<( SvStream &rOs, const SwTxtSizeInfo &rInf ) //$ ostream
/*N*/ {
/*N*/   CONSTCHAR( pTxt, " {SIZEINFO:" );
/*N*/   rOs << pTxt;
/*N*/   rOs << ' ' << (rInf.OnWin() ? "WIN:" : "PRT:" );
/*N*/   rOs << " Idx:" << rInf.GetIdx();
/*N*/   rOs << " Len:" << rInf.GetLen();
/*N*/   rOs << pClose;
/*N*/   return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwDropPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/   CONSTCHAR( pTxt, " {DROP:" );
/*N*/   rOs << pTxt;
/*N*/   SwTxtPortion::operator<<( rOs );
/*N*/     if( pPart && nDropHeight )
/*N*/   {
/*N*/       rOs << " H:" << nDropHeight;
/*N*/       rOs << " L:" << nLines;
/*N*/         rOs <<" Fnt:" << pPart->GetFont().GetHeight();
/*N*/         if( nX || nY )
/*N*/             rOs << " [" << nX << '/' << nY << ']';
/*N*/   }
/*N*/   rOs << pClose;
/*N*/   return rOs;
/*N*/ }
} //namespace binfilter
/*N*/ #endif /* OSL_DEBUG_LEVEL */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
