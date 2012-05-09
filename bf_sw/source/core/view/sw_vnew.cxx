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

#include <rtl/logfile.hxx>

#include <horiornt.hxx>

#include <doc.hxx>
#include <docsh.hxx>
#include <viewsh.hxx>
#include <rootfrm.hxx>
#include <viewimp.hxx>
#include <viewopt.hxx>
#include <txtfrm.hxx>       // Zugriff auf TxtCache
#include <notxtfrm.hxx>
#include <fntcache.hxx>
#include <docufld.hxx>
#include <ptqueue.hxx>
#include <dview.hxx>        // SdrView
#include <ndgrf.hxx>
#include <ndindex.hxx>
#include <accessibilityoptions.hxx>
namespace binfilter {

/******************************************************************************
|*
|*  ViewShell::~ViewShell()
|*
******************************************************************************/

/*N*/ ViewShell::~ViewShell()
/*N*/ {
/*N*/   {
/*N*/       SET_CURR_SHELL( this );
/*N*/       bPaintWorks = FALSE;
/*N*/
/*N*/       //Die Animierten Grafiken abschalten!
/*N*/       if( pDoc )
/*N*/       {
/*N*/           SwNodes& rNds = pDoc->GetNodes();
/*N*/           SwGrfNode *pGNd;
/*N*/
/*N*/           SwStartNode *pStNd;
/*N*/           SwNodeIndex aIdx( *rNds.GetEndOfAutotext().StartOfSectionNode(), 1 );
/*N*/           while ( 0 != (pStNd = aIdx.GetNode().GetStartNode()) )
/*N*/           {
/*N*/               aIdx++;
/*N*/               if ( 0 != ( pGNd = aIdx.GetNode().GetGrfNode() ) )
/*N*/               {
/*N*/                   if( pGNd->IsAnimated() )
/*N*/                   {
/*N*/                       SwClientIter aIter( *pGNd );
/*N*/                       for( SwFrm* pFrm = (SwFrm*)aIter.First( TYPE(SwFrm) );
/*N*/                           pFrm; pFrm = (SwFrm*)aIter.Next() )
/*N*/                       {
/*N*/                           OSL_ENSURE( pFrm->IsNoTxtFrm(), "GraphicNode with Text?" );
/*N*/                           ((SwNoTxtFrm*)pFrm)->StopAnimation( pOut );
/*N*/                       }
/*N*/                   }
/*N*/               }
/*N*/               aIdx.Assign( *pStNd->EndOfSectionNode(), +1 );
/*N*/           }
/*N*/
/*N*/           GetDoc()->StopNumRuleAnimations( pOut );
/*N*/       }
/*N*/
/*N*/       delete pImp; //Erst loeschen, damit die LayoutViews vernichtet werden.
/*N*/       pImp = 0;   // Set to zero, because ~SwFrm relies on it.
/*N*/
/*N*/       if ( pDoc )
/*N*/       {
/*N*/           if( !pDoc->RemoveLink() )
/*?*/               delete pDoc, pDoc = 0;
/*N*/       }
/*N*/
/*N*/       delete pOpt;
/*N*/
/*N*/       //Format-Cache zurueckschrauben.
/*N*/       if ( SwTxtFrm::GetTxtCache()->GetCurMax() > 250 )
/*N*/           SwTxtFrm::GetTxtCache()->DecreaseMax( 100 );
/*N*/
/*N*/       //Ggf. aus der PaintQueue entfernen lassen
/*N*/       SwPaintQueue::Remove( this );
/*N*/
/*N*/       OSL_ENSURE( !nStartAction, "EndAction() pending." );
/*N*/   }
/*N*/
/*N*/     delete mpTmpRef;
/*N*/     delete pAccOptions;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
