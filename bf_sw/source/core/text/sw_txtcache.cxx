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

#include "txtcache.hxx"
#include "txtfrm.hxx"
#include "porlay.hxx"
namespace binfilter {

/*************************************************************************
|*
|*  SwTxtLine::SwTxtLine(), ~SwTxtLine()
|*
|*************************************************************************/

/*N*/ SwTxtLine::SwTxtLine( SwTxtFrm *pFrm, SwParaPortion *pNew ) :
/*N*/   SwCacheObj( (void*)pFrm ),
/*N*/   pLine( pNew )
/*N*/ {
/*N*/ }
/*N*/
/*N*/ SwTxtLine::~SwTxtLine()
/*N*/ {
/*N*/   delete pLine;
/*N*/ }

/*************************************************************************
|*
|*  SwTxtLineAccess::NewObj()
|*
|*************************************************************************/

/*N*/ SwCacheObj *SwTxtLineAccess::NewObj()
/*N*/ {
/*N*/   return new SwTxtLine( (SwTxtFrm*)pOwner );
/*N*/ }

/*************************************************************************
|*
|*  SwTxtLineAccess::GetPara()
|*
|*************************************************************************/

/*N*/ SwParaPortion *SwTxtLineAccess::GetPara()
/*N*/ {
/*N*/   SwTxtLine *pRet;
/*N*/   if ( pObj )
/*N*/       pRet = (SwTxtLine*)pObj;
/*N*/   else
/*N*/   {
/*N*/       pRet = (SwTxtLine*)Get();
/*N*/       ((SwTxtFrm*)pOwner)->SetCacheIdx( pRet->GetCachePos() );
/*N*/   }
/*N*/   if ( !pRet->GetPara() )
/*N*/       pRet->SetPara( new SwParaPortion );
/*N*/   return pRet->GetPara();
/*N*/ }


/*************************************************************************
|*
|*  SwTxtLineAccess::SwTxtLineAccess()
|*
|*************************************************************************/

/*N*/ SwTxtLineAccess::SwTxtLineAccess( const SwTxtFrm *pOwner1 ) :
/*N*/   SwCacheAccess( *SwTxtFrm::GetTxtCache(), pOwner1, pOwner1->GetCacheIdx() )
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*  SwTxtLineAccess::IsAvailable
|*
|*************************************************************************/

/*N*/ sal_Bool SwTxtLineAccess::IsAvailable() const
/*N*/ {
/*N*/   if ( pObj )
/*N*/       return ((SwTxtLine*)pObj)->GetPara() != 0;
/*N*/   return sal_False;
/*N*/ }

/*************************************************************************
|*
|*  SwTxtFrm::HasPara()
|*
|*************************************************************************/

/*N*/ sal_Bool SwTxtFrm::_HasPara() const
/*N*/ {
/*N*/   SwTxtLine *pTxtLine = (SwTxtLine*)SwTxtFrm::GetTxtCache()->
/*N*/                                           Get( this, GetCacheIdx(), sal_False );
/*N*/   if ( pTxtLine )
/*N*/   {
/*N*/       if ( pTxtLine->GetPara() )
/*N*/           return sal_True;
/*N*/   }
/*N*/   else
/*N*/       ((SwTxtFrm*)this)->nCacheIdx = MSHRT_MAX;
/*N*/
/*N*/   return sal_False;
/*N*/ }

/*************************************************************************
|*
|*  SwTxtFrm::GetPara()
|*
|*************************************************************************/

/*N*/ SwParaPortion *SwTxtFrm::GetPara()
/*N*/ {
/*N*/   if ( GetCacheIdx() != MSHRT_MAX )
/*N*/   {   SwTxtLine *pLine = (SwTxtLine*)SwTxtFrm::GetTxtCache()->
/*N*/                                       Get( this, GetCacheIdx(), sal_False );
/*N*/       if ( pLine )
/*N*/           return pLine->GetPara();
/*N*/       else
/*N*/           nCacheIdx = MSHRT_MAX;
/*N*/   }
/*N*/   return 0;
/*N*/ }


/*************************************************************************
|*
|*  SwTxtFrm::ClearPara()
|*
|*************************************************************************/

/*N*/ void SwTxtFrm::ClearPara()
/*N*/ {
/*N*/   OSL_ENSURE( !IsLocked(), "+SwTxtFrm::ClearPara: this is locked." );
/*N*/   if ( !IsLocked() && GetCacheIdx() != MSHRT_MAX )
/*N*/   {
/*N*/       SwTxtLine *pTxtLine = (SwTxtLine*)SwTxtFrm::GetTxtCache()->
/*N*/                                       Get( this, GetCacheIdx(), sal_False );
/*N*/       if ( pTxtLine )
/*N*/       {
/*N*/           delete pTxtLine->GetPara();
/*N*/           pTxtLine->SetPara( 0 );
/*N*/       }
/*N*/       else
/*N*/           nCacheIdx = MSHRT_MAX;
/*N*/   }
/*N*/ }

/*************************************************************************
|*
|*  SwTxtFrm::SetPara()
|*
|*************************************************************************/

/*N*/ void SwTxtFrm::SetPara( SwParaPortion *pNew, sal_Bool bDelete )
/*N*/ {
/*N*/   if ( GetCacheIdx() != MSHRT_MAX )
/*N*/   {
/*N*/       //Nur die Information Auswechseln, das CacheObj bleibt stehen.
/*N*/       SwTxtLine *pTxtLine = (SwTxtLine*)SwTxtFrm::GetTxtCache()->
/*N*/                                       Get( this, GetCacheIdx(), sal_False );
/*N*/       if ( pTxtLine )
/*N*/       {
/*N*/           if( bDelete )
/*N*/               delete pTxtLine->GetPara();
/*N*/           pTxtLine->SetPara( pNew );
/*N*/       }
/*N*/       else
/*N*/       {
/*?*/           OSL_ENSURE( !pNew, "+SetPara: Losing SwParaPortion" );
/*?*/           nCacheIdx = MSHRT_MAX;
/*N*/       }
/*N*/   }
/*N*/   else if ( pNew )
/*N*/   {   //Einen neuen einfuegen.
/*N*/       SwTxtLine *pTxtLine = new SwTxtLine( this, pNew );
/*N*/       if ( SwTxtFrm::GetTxtCache()->Insert( pTxtLine ) )
/*N*/           nCacheIdx = pTxtLine->GetCachePos();
/*N*/       else
/*N*/       {
/*?*/           OSL_FAIL( "+SetPara: InsertCache failed." );
/*N*/       }
/*N*/   }
/*N*/ }
/*N*/
/*N*/
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
