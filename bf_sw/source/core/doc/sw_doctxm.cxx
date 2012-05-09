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

#include <limits.h>

#include <hintids.hxx>

#define _SVSTDARR_STRINGSSORT

#include <docsh.hxx>

#include <osl/diagnose.h>

#include <fmtinfmt.hxx>

#include <horiornt.hxx>

#include <doc.hxx>
#include <ndtxt.hxx>
#include <doctxm.hxx>
#include <txmsrt.hxx>
#include <poolfmt.hxx>
#include <rootfrm.hxx>
#include <docary.hxx>
#include <txtfld.hxx>
#include <chpfld.hxx>
namespace binfilter {

const sal_Unicode cNumRepl      = '@';
const sal_Unicode cEndPageNum   = '~';
const sal_Char sPageDeli[] = ", ";

/*N*/ SV_IMPL_PTRARR(SwTOXSortTabBases, SwTOXSortTabBasePtr)

/*N*/ TYPEINIT2( SwTOXBaseSection, SwTOXBase, SwSection );  // fuers RTTI

struct LinkStruct
{
    SwFmtINetFmt    aINetFmt;
    xub_StrLen nStartTextPos, nEndTextPos;

    LinkStruct( const String& rURL, xub_StrLen nStart, xub_StrLen nEnd )
        : aINetFmt( rURL, aEmptyStr),
        nStartTextPos( nStart),
        nEndTextPos(nEnd) {}
};

typedef LinkStruct* LinkStructPtr;


/*--------------------------------------------------------------------
     Beschreibung: Traveln zwischen TOXMarks
 --------------------------------------------------------------------*/


/*N*/ const SwTOXBaseSection* SwDoc::InsertTableOf( const SwPosition& rPos,
/*N*/                                               const SwTOXBase& rTOX,
/*N*/                                               const SfxItemSet* pSet,
/*N*/                                               BOOL bExpand )
/*N*/ {
/*N*/
/*N*/   SwTOXBaseSection* pNew = new SwTOXBaseSection( rTOX );
/*N*/   String sSectNm( rTOX.GetTOXName() );
/*N*/   sSectNm = GetUniqueTOXBaseName( *rTOX.GetTOXType(), &sSectNm );
/*N*/   pNew->SetTOXName(sSectNm);
/*N*/   pNew->SwSection::SetName(sSectNm);
/*N*/   SwPaM aPam( rPos );
/*N*/   SwSection* pSect = Insert( aPam, *pNew, pSet, FALSE );
/*N*/   if( pSect )
/*N*/   {
/*N*/       SwSectionNode* pSectNd = pSect->GetFmt()->GetSectionNode();
/*N*/       SwSection* pCl = pNew;
/*N*/       pSect->GetFmt()->Add( pCl );
/*N*/       pSectNd->SetNewSection( pNew );
/*N*/
/*N*/       if( !bExpand && (1 == rTOX.GetTitle().Len() && IsInReading()) )
/*N*/       // insert title of TOX
/*N*/       {
/*?*/           // then insert the headline section
/*?*/           SwNodeIndex aIdx( *pSectNd, +1 );
/*?*/
/*?*/           SwTxtNode* pHeadNd = GetNodes().MakeTxtNode( aIdx,
/*?*/                           GetTxtCollFromPool( RES_POOLCOLL_STANDARD ) );
/*?*/
/*?*/           String sNm( pNew->GetTOXName() );
/*?*/ // ??Resource
/*?*/ sNm.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "_Head" ));
/*?*/
/*?*/           SwSection aSect( TOX_HEADER_SECTION, sNm );
/*?*/
/*?*/           SwNodeIndex aStt( *pHeadNd ); aIdx--;
/*?*/           SwSectionFmt* pSectFmt = MakeSectionFmt( 0 );
/*?*/           GetNodes().InsertSection( aStt, *pSectFmt, aSect, &aIdx,
/*?*/                                               TRUE );
/*?*/       }
/*N*/   }
/*N*/   else
/*?*/       delete pNew, pNew = 0;

/*N*/
/*N*/   return pNew;
/*N*/ }



/*N*/ const SwTOXBaseSection* SwDoc::InsertTableOf( ULONG nSttNd, ULONG nEndNd,
/*N*/                                               const SwTOXBase& rTOX,
/*N*/                                               const SfxItemSet* pSet )
/*N*/ {
/*N*/   // check for recursiv TOX
/*N*/   SwNode* pNd = GetNodes()[ nSttNd ];
/*N*/   SwSectionNode* pSectNd = pNd->FindSectionNode();
/*N*/   while( pSectNd )
/*N*/   {
/*?*/       SectionType eT = pSectNd->GetSection().GetType();
/*?*/       if( TOX_HEADER_SECTION == eT || TOX_CONTENT_SECTION == eT )
/*?*/           return 0;
/*?*/       pSectNd = pSectNd->FindStartNode()->FindSectionNode();
/*N*/   }
/*N*/
/*N*/   // create SectionNode around the Nodes
/*N*/   SwTOXBaseSection* pNew = new SwTOXBaseSection( rTOX );
/*N*/
/*N*/   String sSectNm( rTOX.GetTOXName() );
/*N*/   sSectNm = GetUniqueTOXBaseName(*rTOX.GetTOXType(), &sSectNm);
/*N*/   pNew->SetTOXName(sSectNm);
/*N*/   pNew->SwSection::SetName(sSectNm);
/*N*/
/*N*/   SwNodeIndex aStt( GetNodes(), nSttNd ), aEnd( GetNodes(), nEndNd );
/*N*/   SwSectionFmt* pFmt = MakeSectionFmt( 0 );
/*N*/   if(pSet)
/*?*/       pFmt->SetAttr(*pSet);
/*N*/
/*N*/ //    --aEnd;     // im InsertSection ist Ende inclusive
/*N*/
/*N*/   pSectNd = GetNodes().InsertSection( aStt, *pFmt, *pNew, &aEnd );
/*N*/   if( pSectNd )
/*N*/   {
/*N*/       SwSection* pCl = pNew;
/*N*/       pFmt->Add( pCl );
/*N*/       pSectNd->SetNewSection( pNew );
/*N*/   }
/*N*/   else
/*N*/   {
/*?*/       delete pNew, pNew = 0;
/*?*/       DelSectionFmt( pFmt );
/*N*/   }
/*N*/
/*N*/   return pNew;
/*N*/ }

/*--------------------------------------------------------------------
     Beschreibung: Aktuelles Verzeichnis ermitteln
 --------------------------------------------------------------------*/

/*N*/ const SwTOXBase* SwDoc::GetCurTOX( const SwPosition& rPos ) const
/*N*/ {
/*N*/   const SwNode& rNd = rPos.nNode.GetNode();
/*N*/   const SwSectionNode* pSectNd = rNd.FindSectionNode();
/*N*/   while( pSectNd )
/*N*/   {
/*N*/       SectionType eT = pSectNd->GetSection().GetType();
/*N*/       if( TOX_CONTENT_SECTION == eT )
/*N*/       {
/*N*/           OSL_ENSURE( pSectNd->GetSection().ISA( SwTOXBaseSection ),
/*N*/                   "keine TOXBaseSection!" );
/*N*/           SwTOXBaseSection& rTOXSect = (SwTOXBaseSection&)
/*N*/                                               pSectNd->GetSection();
/*N*/           return &rTOXSect;
/*N*/       }
/*?*/       pSectNd = pSectNd->FindStartNode()->FindSectionNode();
/*N*/   }
/*N*/   return 0;
/*N*/ }

/*N*/ const SwAttrSet& SwDoc::GetTOXBaseAttrSet(const SwTOXBase& rTOXBase) const
/*N*/ {
/*N*/   OSL_ENSURE( rTOXBase.ISA( SwTOXBaseSection ), "no TOXBaseSection!" );
/*N*/   const SwTOXBaseSection& rTOXSect = (const SwTOXBaseSection&)rTOXBase;
/*N*/   SwSectionFmt* pFmt = rTOXSect.GetFmt();
/*N*/   OSL_ENSURE( pFmt, "invalid TOXBaseSection!" );
/*N*/   return pFmt->GetAttrSet();
/*N*/ }


/*--------------------------------------------------------------------
     Beschreibung:  Verzeichnistypen verwalten
 --------------------------------------------------------------------*/

/*N*/ USHORT SwDoc::GetTOXTypeCount(TOXTypes eTyp) const
/*N*/ {
/*N*/   const SwTOXTypePtr * ppTTypes = pTOXTypes->GetData();
/*N*/   USHORT nCnt = 0;
/*N*/   for( USHORT n = 0; n < pTOXTypes->Count(); ++n, ++ppTTypes )
/*N*/       if( eTyp == (*ppTTypes)->GetType() )
/*N*/           ++nCnt;
/*N*/   return nCnt;
/*N*/ }

/*N*/ const SwTOXType* SwDoc::GetTOXType( TOXTypes eTyp, USHORT nId ) const
/*N*/ {
/*N*/   const SwTOXTypePtr * ppTTypes = pTOXTypes->GetData();
/*N*/   USHORT nCnt = 0;
/*N*/   for( USHORT n = 0; n < pTOXTypes->Count(); ++n, ++ppTTypes )
/*N*/       if( eTyp == (*ppTTypes)->GetType() && nCnt++ == nId )
/*N*/           return (*ppTTypes);
/*N*/   return 0;
/*N*/ }



/*N*/ const SwTOXType* SwDoc::InsertTOXType( const SwTOXType& rTyp )
/*N*/ {
/*N*/   SwTOXType * pNew = new SwTOXType( rTyp );
/*N*/   pTOXTypes->Insert( pNew, pTOXTypes->Count() );
/*N*/   return pNew;
/*N*/ }

/*N*/ String SwDoc::GetUniqueTOXBaseName( const SwTOXType& rType,
/*N*/                                   const String* pChkStr ) const
/*N*/ {
/*N*/   if(pChkStr && !pChkStr->Len())
/*N*/       pChkStr = 0;
/*N*/   String aName( rType.GetTypeName() );
/*N*/   xub_StrLen nNmLen = aName.Len();
/*N*/
/*N*/   USHORT nNum = 0, nTmp = 0, nFlagSize = ( pSectionFmtTbl->Count() / 8 ) +2;
/*N*/   BYTE* pSetFlags = new BYTE[ nFlagSize ];
/*N*/   memset( pSetFlags, 0, nFlagSize );
/*N*/
/*N*/   const SwSectionNode* pSectNd;
/*N*/   const SwSection* pSect;
        USHORT n=0;
/*N*/   for( n = 0; n < pSectionFmtTbl->Count(); ++n )
/*N*/       if( 0 != ( pSectNd = (*pSectionFmtTbl)[ n ]->GetSectionNode( FALSE ) )&&
/*N*/            TOX_CONTENT_SECTION == (pSect = &pSectNd->GetSection())->GetType())
/*N*/       {
/*N*/           const String& rNm = pSect->GetName();
/*N*/           if( rNm.Match( aName ) == nNmLen )
/*N*/           {
/*N*/               // Nummer bestimmen und das Flag setzen
/*N*/               nNum = (USHORT)rNm.Copy( nNmLen ).ToInt32();
/*N*/               if( nNum-- && nNum < pSectionFmtTbl->Count() )
/*N*/                   pSetFlags[ nNum / 8 ] |= (0x01 << ( nNum & 0x07 ));
/*N*/           }
/*N*/           if( pChkStr && pChkStr->Equals( rNm ) )
/*N*/               pChkStr = 0;
/*N*/       }
/*N*/
/*N*/   if( !pChkStr )
/*N*/   {
/*N*/       // alle Nummern entsprechend geflag, also bestimme die richtige Nummer
/*N*/       nNum = pSectionFmtTbl->Count();
/*N*/       for( n = 0; n < nFlagSize; ++n )
/*N*/           if( 0xff != ( nTmp = pSetFlags[ n ] ))
/*N*/           {
/*N*/               // also die Nummer bestimmen
/*N*/               nNum = n * 8;
/*N*/               while( nTmp & 1 )
/*N*/                   ++nNum, nTmp >>= 1;
/*N*/               break;
/*N*/           }
/*N*/   }
/*N*/   delete [] pSetFlags;
/*N*/   if( pChkStr )
/*N*/       return *pChkStr;
/*N*/   return aName += String::CreateFromInt32( ++nNum );
/*N*/ }


/*N*/ BOOL SwDoc::SetTOXBaseName(const SwTOXBase& rTOXBase, const String& rName)
/*N*/ {
/*N*/   OSL_ENSURE( rTOXBase.ISA( SwTOXBaseSection ),
/*N*/                   "keine TOXBaseSection!" );
/*N*/   SwTOXBaseSection* pTOX = (SwTOXBaseSection*)&rTOXBase;
/*N*/
/*N*/   String sTmp = GetUniqueTOXBaseName(*rTOXBase.GetTOXType(), &rName);
/*N*/   BOOL bRet = sTmp == rName;
/*N*/   if(bRet)
/*N*/   {
/*?*/       pTOX->SetTOXName(rName);
/*?*/       pTOX->SwTOXBaseSection::SetName(rName);
/*?*/       SetModified();
/*N*/   }
/*N*/   return bRet;
/*N*/ }






/*--------------------------------------------------------------------
     Beschreibung: Verzeichnis-Klasse
 --------------------------------------------------------------------*/

/*N*/ SwTOXBaseSection::SwTOXBaseSection( const SwTOXBase& rBase )
/*N*/   : SwTOXBase( rBase ), SwSection( TOX_CONTENT_SECTION, aEmptyStr )
/*N*/ {
/*N*/   SetProtect( rBase.IsProtected() );
/*N*/   SwSection::SetName( GetTOXName() );
/*N*/ }


/*N*/ SwTOXBaseSection::~SwTOXBaseSection()
/*N*/ {
/*N*/ }


/*--------------------------------------------------------------------
     Beschreibung: Seitennummer errechnen und nach dem Formatieren
                   eintragen
 --------------------------------------------------------------------*/

/*N*/ void SwTOXBaseSection::UpdatePageNum()
/*N*/ {
/*N*/   if( !aSortArr.Count() )
/*N*/       return ;

    // die aktuellen Seitennummern ins Verzeichnis eintragen
/*?*/   (SwDoc*)GetFmt()->GetDoc();
/*?*/
/*?*/   // nach dem Setzen der richtigen Seitennummer, das Mapping-Array
/*?*/   // wieder loeschen !!
/*?*/   aSortArr.DeleteAndDestroy( 0, aSortArr.Count() );
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
