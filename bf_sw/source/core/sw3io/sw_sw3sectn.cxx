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


#include <bf_svtools/urihelper.hxx>
#ifndef _SVSTDARR_USHORTS_DECL
#define _SVSTDARR_USHORTS
#endif
#include <bf_so3/linkmgr.hxx>

#include <fmtanchr.hxx>

#include <horiornt.hxx>

#include <doc.hxx>

#include <osl/diagnose.h>

#include <pam.hxx>
#include <sw3imp.hxx>
#include <sw3marks.hxx>
#include <section.hxx>
#include <ndtxt.hxx>
#include <ndnotxt.hxx>
#include <swerror.h>
#include "bf_so3/staticbaseurl.hxx"
namespace binfilter {

//////////////////////////////////////////////////////////////////////////////

// Wird ein Text in einen Absatz eingefuegt, wird der erste Absatz an den
// ersten Teil angefuegt, ohne die Formate zu uebernehmen. Alle anderen
// Absaetze uebernehmen die Formate, was auch den Rest des alten Absatzes
// betrifft.
// Die Position rPos wird weitergefuehrt. Der uebergebene Offset ist
// ein Offset in einen TextNode, der an InTxtNode() uebergeben wird.
// Falls bNode1 FALSE ist, wird ein evtl. vorhandener leerer Node nicht
// gefuellt. Dadurch laesst sich diese Methode fuer mehrere Contents
// verwenden.

/*N*/ void Sw3IoImp::InContents( SwNodeIndex& rPos, xub_StrLen nOffset, BOOL bNode1,
/*N*/                          BYTE nInsFirstPara, BOOL bDontMove )
/*N*/ {
/*N*/   OpenRec( SWG_CONTENTS );
/*N*/
/*N*/   // Der aktuelle NumRange eines 3.1/40-Files muss gerettet werden, falls
/*N*/   // eine Section innerhalb einer Section gelesen wird (z.B. Flys)
/*N*/   SwNumRule* pOld40Rule = NULL;
/*N*/   SwPaM* pOld40Range =NULL;
/*N*/   BOOL bOldConvertNoNum = 0;
/*N*/   if( !IsVersion(SWG_LONGIDX) )
/*N*/   {
/*N*/       pOld40Rule = pCurNumRule;
/*N*/       pOld40Range = pCurNumRange;
/*N*/       bOldConvertNoNum = bConvertNoNum;
/*N*/       pCurNumRange = NULL;
/*N*/       pCurNumRule = NULL;
/*N*/       bConvertNoNum = FALSE;
/*N*/   }
/*N*/
/*N*/   // fuer die Sonderbehandlung des ersten/letzten Node beim "Datei/Einfuegen"
/*N*/   BYTE nSaveInsFirstPara = nInsFirstPara;
/*N*/
/*N*/   // Bei einem echten Insert muss der aktuelle Node gesplittet werden,
/*N*/   // Falls mehr als 1 Node eingelesen wird.
/*N*/   BOOL bSplit = FALSE;
/*N*/   SwTxtNode* pLast = NULL;
/*N*/   const SwStartNode *pSttNd, *pSectSttNd = 0;
/*N*/   SwNode* pStart = pDoc->GetNodes()[ rPos ];
/*N*/   if( pStart->GetStartNode() && !bDontMove )
/*N*/   {
/*N*/       pSttNd = (const SwStartNode *)pStart;
/*N*/       pSectSttNd = pSttNd;
/*N*/
/*N*/       // Index zeigt auf StartNode. In Contents-Bereich reingehen
/*N*/       // und den ersten TextNode nehmen
/*N*/       rPos++;
/*N*/       pStart = pDoc->GetNodes()[ rPos ];
/*N*/   }
/*N*/   else
/*N*/       pSttNd = pStart->FindStartNode();
/*N*/
/*N*/   SwTxtNode* pNd = bNode1 ? pStart->GetTxtNode() : NULL;
/*N*/   SwTxtNode* pNd1 = pNd;
/*N*/   OSL_ENSURE( !nInsFirstPara || pNd, "Einfuegen in Nicht-Content-Node?" );
/*N*/   SwPosition *pEndPos = 0;
/*N*/   if( nInsFirstPara && pNd )
/*N*/   {
/*?*/       SwIndex aTmp( pNd, nOffset );
/*?*/       pEndPos = new SwPosition( rPos, aTmp );
/*N*/   }
/*N*/
/*N*/   UINT32 nNodes;
/*N*/   if( IsVersion(SWG_LAYFRAMES) )
/*N*/       OpenFlagRec();
/*N*/   if( IsVersion(SWG_LONGIDX) )
/*N*/   {
/*N*/       *pStrm >> nNodes;
/*N*/   }
/*N*/   else
/*N*/   {
/*N*/       UINT16 nNodes16, nSectIdDummy;
/*N*/       if( IsVersion(SWG_LAYFRAMES) )
/*N*/           *pStrm >> nSectIdDummy;
/*N*/       *pStrm >> nNodes16;
/*N*/       nNodes = nNodes16;
/*N*/   }
/*N*/   if( IsVersion(SWG_LAYFRAMES) )
/*N*/       CloseFlagRec();
/*N*/
/*N*/   ULONG i = 0;
/*N*/   while( BytesLeft() )
/*N*/   {
/*N*/       SetPercentBar( pStrm->Tell() );
/*N*/       BYTE cType = Peek();
/*N*/       i++;
/*N*/       switch( cType )
/*N*/       {
/*N*/           case SWG_TEXTNODE:
/*N*/               // Der Node muss gesplittet werden,
/*N*/               // wenn mehrere Nodes inserted werden
/*N*/               if( !bSplit && bInsert && nNodes > 1 )
/*N*/               {
/*?*/                   if( pNd )
/*?*/                   {
/*?*/                       // natuerlich nur TextNodes splitten
/*?*/                       SwPosition aSplitPos( rPos, SwIndex( pNd, nOffset ) );
/*?*/                       pDoc->SplitNode( aSplitPos );
/*?*/                       pLast = pNd;
/*?*/                       rPos--;
/*?*/                       pNd = pDoc->GetNodes()[ rPos ]->GetTxtNode();
/*?*/                       bSplit = TRUE;
/*?*/                   }
/*N*/               }
/*N*/               else if( i == nNodes && pLast )
/*N*/               {
/*N*/                   // der letzte Node: nimm evtl. den gesplitteten
/*?*/                   pNd = pLast;
/*?*/                   if( nSaveInsFirstPara && pNd && pNd->GetTxt().Len() )
/*?*/                       nInsFirstPara = 2;
/*N*/               }
/*N*/               InTxtNode( pNd, rPos, nOffset, nInsFirstPara );
/*N*/               pNd = pNd1 = NULL; nOffset = 0;
/*N*/               nInsFirstPara = 0;
/*N*/               break;
/*N*/           case SWG_GRFNODE:
/*N*/               InGrfNode( rPos );
/*N*/               nInsFirstPara = 0;
/*N*/               break;
/*N*/           case SWG_OLENODE:
/*N*/               InOLENode( rPos );
/*N*/               nInsFirstPara = 0;
/*N*/               break;
/*N*/           case SWG_TABLE:
/*N*/               // JP 20.05.94: Dok. einfuegen: wird als 1.Node eine
/*N*/               //              Tabelle eingefuegt, dann immer splitten !!
/*N*/               nInsFirstPara = 0;
/*N*/               if( 1 == i && bInsert && pNd )
/*N*/               {
/*?*/                   SwPosition aSplitPos( rPos, SwIndex( pNd, nOffset ) );
/*?*/                   pDoc->SplitNode( aSplitPos );
/*?*/                   nOffset = 0;    //  mit dem Offset hat sichs erledigt!
/*N*/               }
/*N*/               InTable( rPos );
/*N*/               break;
/*N*/           case SWG_SECTION:
/*N*/               // JP 20.05.94: Dok. einfuegen: wird als 1.Node eine
/*N*/               //              Section eingefuegt, dann immer splitten !!
/*N*/               nInsFirstPara = 0;
/*N*/               if( 1 == i && bInsert && pNd )
/*N*/               {
/*?*/                   SwPosition aSplitPos( rPos, SwIndex( pNd, nOffset ) );
/*?*/                   pDoc->SplitNode( aSplitPos );
/*?*/                   nOffset = 0;    //  mit dem Offset hat sichs erledigt!
/*N*/               }
/*N*/               InSection( rPos );
/*N*/               break;
/*N*/           case SWG_REPTEXTNODE:
/*N*/               // keine Spezialbehandlung fuer 1. und letzten Node neoetig,
/*N*/               // weil der erste und letzte Knoten eines gespeicherten
/*N*/               // Doks nie ein RepTextNode ist.
/*N*/               // MIB 21.7.97: Irrtum: Beim Einfuegen von Tabellen wird
/*N*/               // auch immer gesplittet.
/*N*/               {
/*N*/                   BOOL bJoin = pLast && i==nNodes;
/*N*/                   InRepTxtNode( rPos );
/*N*/                   if( bJoin )
/*N*/                   {
/*?*/                       rPos--;
/*?*/                       SwTxtNode* pTxtNode = rPos.GetNode().GetTxtNode();
/*?*/                       if( pTxtNode && pTxtNode->CanJoinNext() )
/*?*/                           pTxtNode->JoinNext();
/*?*/                       rPos++;
/*N*/                   }
/*N*/               }
/*N*/               break;
/*N*/           case SWG_SDRFMT:
/*N*/               // Keine Draw-Formate in Kopf oder Fusszeilen einfuegen oder
/*N*/               // wenn kein Drawing-Layer da ist!
/*?*/               if( (nGblFlags & SW3F_NODRAWING) || bInsIntoHdrFtr )
/*?*/               {
/*?*/                   i--; // War gar kein Node
/*?*/                   SkipRec();
/*?*/                   break;
/*?*/               } // sonst weiter:
/*?*/           case SWG_FLYFMT:
/*?*/               {
/*?*/                   // Was auch immer jetzt passiert, einen Node lesen wir
/*?*/                   // nicht ein.
/*?*/                   i--;
/*?*/
/*?*/                   if( SwFlyStartNode != pSttNd->GetStartNodeType() )
/*?*/                   {
/*?*/                       OSL_ENSURE( !this,
/*?*/                               "Verankerung an Frames ist nur fuer Fly-Frames implementiert" );
/*?*/                       break;
/*?*/                   }
/*?*/
/*?*/                   // Rahmengebundener FlyFrame
/*?*/                   USHORT eSave_StartNodeType = eStartNodeType;
/*?*/                   eStartNodeType = SwFlyStartNode;
/*?*/                   SwFrmFmt* pFmt = (SwFrmFmt*) InFormat( cType, NULL );
/*?*/                   eStartNodeType = eSave_StartNodeType;
/*?*/
/*?*/                   if( !pFmt )
/*?*/                       break;
/*?*/
/*?*/                   // Anker darin versenken
/*?*/                   SwFmtAnchor aAnchor( pFmt->GetAnchor() );
/*?*/                   SwPosition aPos( *pSttNd );
/*?*/                   aAnchor.SetAnchor( &aPos );
/*?*/                   pFmt->SetAttr( aAnchor );
/*?*/
/*?*/                   // Layout-Frames im Insert Mode fuer absatzgebundene
/*?*/                   // Flys erzeugen
/*?*/                   if( bInsert && !nRes )
/*?*/                       pFmt->MakeFrms();
/*?*/               }
/*?*/               break;
/*?*/
/*?*/           case SWG_NODEREDLINE:
/*?*/               {
/*N*/ /*?*/                     i--;    //SW50.SDW
/*N*/ /*?*/                     if( nSaveInsFirstPara )
/*N*/ /*?*/                     {
/*N*/ /*?*/                         // Hier kann es nur einen End-Index geben. Der
/*N*/ /*?*/                         // entspricht dann der Einfuege-Position.
/*N*/ /*?*/                         INT32 nOffs = pEndPos->nContent.GetIndex();
/*N*/ /*?*/                         InNodeRedline( pEndPos->nNode, nOffs, nSaveInsFirstPara );
/*N*/ /*?*/                     }
/*N*/ /*?*/                     else
/*N*/ /*?*/                     {
/*N*/ /*?*/                         SwNodeIndex aIdx( *pSttNd );
/*N*/ /*?*/                         INT32 nDummy = 0;
/*N*/ /*?*/                         InNodeRedline( aIdx, nDummy );
/*N*/ /*?*/                     }
/*?*/               }
/*?*/               break;
/*?*/
/*?*/           default:
/*?*/               // MIB 15.4.97: Wenn wir diesen Record ueberspringen, dann
/*?*/               // fuegen wir auch keinen Node ein. Dann sollten wir
/*?*/               // vielleicht auch den Node-Zaehler wir eins runterzaehlen,
/*?*/               // weil sonst doch das ein oder andere schiefgehen kann.
/*?*/               i--;
/*?*/               SkipRec();
/*?*/               break;
/*N*/       }
/*N*/   }
/*N*/   CloseRec( SWG_CONTENTS );
/*N*/   if( !IsVersion(SWG_LONGIDX) )
/*N*/   {
/*N*/       CloseNumRange40( rPos );
/*N*/       pCurNumRange = pOld40Range;
/*N*/       pCurNumRule = pOld40Rule;
/*N*/       bConvertNoNum = bOldConvertNoNum;
/*N*/   }
/*N*/
/*N*/   // MIB 8.9.97: Wenn man eine Section fuellt, sollte man eigentlich nie
/*N*/   // auf einem End-Node stehen koennen, der nicht zur eigenen Section
/*N*/   // gehoert. Fuer den Fall, dass man keine Section laed und trotzdem
/*N*/   // auf einem End-Node steht, lassen wir den Code erstmal drinne, aber
/*N*/   // es ist doc recht fraglich, wozu er da ist.
/*N*/   SwEndNode *pEndNd = pDoc->GetNodes()[ rPos ]->GetEndNode();
/*N*/   OSL_ENSURE( !pEndNd || !pSectSttNd || pEndNd->FindStartNode()==pSectSttNd,
/*N*/           "PaM steht auf EndNode, der nicht zur Section gehoert." );
/*N*/   if( pEndNd && !pSectSttNd &&
/*N*/       pEndNd != &pDoc->GetNodes().GetEndOfContent())
/*N*/   {
/*N*/       rPos++;
/*N*/   }
/*N*/
/*N*/   // Eine Tabelle und eine Section koennen u.U. alleine
/*N*/   // in einer Textsection existieren, d.h. es muss ggf.
/*N*/   // der ueberfluessige Node entfernt werden!
/*N*/   if( pNd1 && !nRes )
/*N*/   {
/*?*/       SwNodeIndex aPos( *pNd1 );
/*?*/       // MIB 8.9.97: Mit dieser Abrage wird sichergestellt, dass beim
/*?*/       // Einfuegen nie der Absatz geloscht wird, in den eingefuegt wird.
/*?*/       if( !pCurPaM ||
/*?*/           pCurPaM->GetPoint()->nNode != aPos )
/*?*/       {
/*?*/           pDoc->GetNodes().Delete( aPos );
/*?*/       }
/*N*/   }
/*N*/
/*N*/   // Die aktuelle Position ggf. hinter die Section stellen,
/*N*/   // die gerade eingelesen wurde.
/*N*/   if( pSectSttNd )
/*N*/       rPos = pSectSttNd->EndOfSectionIndex() + 1;
/*N*/
/*N*/   delete pEndPos;
/*N*/ }

// Einfuegen einer nicht vorhandenen Contents-Section. Die Section
// wird am Textende eingefuegt und der numerische Index des StartNodes
// zurueckgeliefert.

/*N*/ SwStartNode& Sw3IoImp::InContents()
/*N*/ {
/*N*/   // Anlegen einer Section mit einem TextNode
/*N*/ // OPT: Section leer anlegen
/*N*/   SwNodeIndex aStart( pDoc->GetNodes().GetEndOfAutotext() );
/*N*/   SwStartNode* pSttNd = pDoc->GetNodes().MakeEmptySection(  aStart,
/*N*/                                   (SwStartNodeType)eStartNodeType );
/*N*/   aStart = *pSttNd->EndOfSectionNode();
/*N*/
/*N*/   InContents( aStart, 0, FALSE );
/*N*/ // /OPT: Section leer anlegen
/*N*/
/*N*/   return *pSttNd;
/*N*/ }

// Einlesen einer "echten" Section innerhalb eines Content-Bereichs
// Auch hier wird der uebergebene Index weitergefuehrt.

/*N*/ void Sw3IoImp::InSection( SwNodeIndex& rPos )
/*N*/ {
/*N*/   OpenRec( SWG_SECTION );
/*N*/   String aName, aCond;
/*N*/   InString( *pStrm, aName );
/*N*/   InString( *pStrm, aCond );
/*N*/   // 0x10 - hidden
/*N*/   // 0x20 - protected
/*N*/   // 0x40 - !conditional hidden (gedreht, fuers Einlesen von alten Docs)
/*N*/   BYTE cFlags = OpenFlagRec();
/*N*/   UINT16 nType;
/*N*/   *pStrm >> nType;
/*N*/   CloseFlagRec();
/*N*/
/*N*/   // beim Insert vom Doc einen eindeutigen Namen erzeugen
/*N*/   if( bInsert )
/*N*/   {
/*?*/       aName = pDoc->GetUniqueSectionName( &aName );
/*?*/       if( !pSectionDepths )
/*?*/           pSectionDepths = new SvUShorts;
/*?*/       if( 0 == pSectionDepths->Count() )
/*?*/       {
/*?*/           pSectionDepths->Insert( 1U, 0U );
/*?*/       }
/*?*/       else
/*?*/       {
/*?*/           (*pSectionDepths)[pSectionDepths->Count()-1]++;
/*?*/       }
/*N*/   }
/*N*/
/*N*/
/*N*/   SwSection aSect( (SectionType) nType, aName );
/*N*/   SwSectionFmt* pFmt = (SwSectionFmt*) InFormat( SWG_SECTFMT, NULL );
/*N*/   ULONG n = rPos.GetIndex();
/*N*/
/*N*/   SwNodeIndex aTmpIdx( pDoc->GetNodes().GetEndOfContent() );
/*N*/   SwSectionNode* pSectNd =
/*N*/       pDoc->GetNodes().InsertSection( rPos, *pFmt, aSect, &aTmpIdx, FALSE );
/*N*/
/*N*/   SwSection& rNdSection = pSectNd->GetSection();
/*N*/
/*N*/   rPos = n;
/*N*/   InContents( rPos );
/*N*/
/*N*/   // Link-Filenamen einlesen
/*N*/   if( nVersion >= SWG_FLYWRAPCHGD )
/*N*/   {
/*N*/       String aLinkFileName;
/*N*/
/*N*/       rtl::OString s8 = read_lenPrefixed_uInt8s_ToOString<sal_uInt16>(*pStrm);
/*N*/       aLinkFileName = ConvertStringNoDelim( s8, '\xff', ::binfilter::cTokenSeperator,
/*N*/                                             eSrcSet );
/*N*/
/*N*/       if( aLinkFileName.Len() && FILE_LINK_SECTION == nType )
/*N*/       {
/*N*/             xub_StrLen nTokenPos = aLinkFileName.Search( ::binfilter::cTokenSeperator );
/*N*/           if( STRING_NOTFOUND != nTokenPos && nTokenPos )
/*N*/           {
/*N*/               String sURL( aLinkFileName.Copy( 0, nTokenPos ) );
/*N*/               aLinkFileName.Erase( 0, nTokenPos );
/*N*/               aLinkFileName.Insert( ::binfilter::StaticBaseUrl::SmartRelToAbs( sURL ), 0 );
/*N*/           }
/*N*/       }
/*N*/       rNdSection.SetLinkFileName( aLinkFileName );
/*N*/   }
/*N*/
/*N*/   rNdSection.SetCondition( aCond );
/*N*/   if( cFlags & 0x10 )
/*?*/       rNdSection.SetHidden();
/*N*/   if( cFlags & 0x20 )
/*N*/       rNdSection.SetProtect();
/*N*/   if( cFlags & 0x40 )
/*?*/       rNdSection.SetCondHidden( FALSE );
/*N*/   if( cFlags & 0x80 )
/*?*/       rNdSection.SetConnectFlag( FALSE );
/*N*/
/*N*/   // ggf. Link neu verbinden aber nicht updaten
/*N*/   if( pSectNd->GetSection().IsLinkType() )
/*N*/       pSectNd->GetSection().CreateLink( CREATE_CONNECT );
/*N*/
/*N*/   // create frames
/*N*/   if( bInsert )
/*N*/   {
/*?*/       (*pSectionDepths)[pSectionDepths->Count()-1]--;
/*?*/       if( 0 == (*pSectionDepths)[pSectionDepths->Count()-1] )
/*?*/       {
/*?*/           SwSectionFmt *pSFmt = rNdSection.GetFmt();
/*?*/           pSFmt->DelFrms();
/*?*/           pSFmt->MakeFrms();
/*?*/       }
/*N*/   }
/*N*/
/*N*/   CloseRec( SWG_SECTION );
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
