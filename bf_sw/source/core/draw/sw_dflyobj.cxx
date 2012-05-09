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

#include "hintids.hxx"

#include <bf_svx/protitem.hxx>
#include <bf_svx/svdpage.hxx>



#include <horiornt.hxx>

#include "frmatr.hxx"
#include "dflyobj.hxx"
#include "frmfmt.hxx"
#include "flyfrms.hxx"
namespace binfilter {

/*N*/ TYPEINIT1( SwFlyDrawObj, SdrObject )
/*N*/ TYPEINIT1( SwVirtFlyDrawObj, SdrVirtObj )

/*************************************************************************
|*
|*  SwFlyDrawObj::Ctor
|*
*************************************************************************/
/*N*/ SwFlyDrawObj::SwFlyDrawObj()
/*N*/ {
/*N*/   bNotPersistent = TRUE;
/*N*/   mpLocalItemSet = NULL;
/*N*/ }

/*N*/ SwFlyDrawObj::~SwFlyDrawObj()
/*N*/ {
/*N*/   if(mpLocalItemSet)
/*?*/       delete mpLocalItemSet;
/*N*/ }



/*************************************************************************
|*
|*  SwFlyDrawObj::Paint()
|*
*************************************************************************/


/*************************************************************************
|*
|*  SwFlyDrawObj::Factory-Methoden
|*
*************************************************************************/

/*N*/ UINT32 SwFlyDrawObj::GetObjInventor() const
/*N*/ {
/*N*/   return SWGInventor;
/*N*/ }


/*N*/ UINT16 SwFlyDrawObj::GetObjIdentifier()   const
/*N*/ {
/*N*/   return SwFlyDrawObjIdentifier;
/*N*/ }



/*************************************************************************
|*
|*  SwVirtFlyDrawObj::CToren, Dtor
|*
*************************************************************************/

/*N*/ SwVirtFlyDrawObj::SwVirtFlyDrawObj(SdrObject& rNew, SwFlyFrm* pFly) :
/*N*/   SdrVirtObj( rNew ),
/*N*/   pFlyFrm( pFly )
/*N*/ {
/*N*/   bNotPersistent = bNeedColorRestore = bWriterFlyFrame = TRUE;
/*N*/   const SvxProtectItem &rP = pFlyFrm->GetFmt()->GetProtect();
/*N*/   bMovProt = rP.IsPosProtected();
/*N*/   bSizProt = rP.IsSizeProtected();
/*N*/ }


/*N*/ SwVirtFlyDrawObj::~SwVirtFlyDrawObj()
/*N*/ {
/*N*/   if ( GetPage() )    //Der SdrPage die Verantwortung entziehen.
/*N*/       GetPage()->RemoveObject( GetOrdNum() );
/*N*/ }

/*************************************************************************
|*
|*  SwVirtFlyDrawObj::GetFmt()
|*
*************************************************************************/



/*N*/ SwFrmFmt *SwVirtFlyDrawObj::GetFmt()
/*N*/ {
/*N*/   return GetFlyFrm()->GetFmt();
/*N*/ }

/*************************************************************************
|*
|*  SwVirtFlyDrawObj::Paint()
|*
*************************************************************************/


/*************************************************************************
|*
|*  SwVirtFlyDrawObj::CheckHit()
|*  Beschreibung        Das Teil ist genau dann getroffen wenn
|*                      1. der Point im Rand des Frm liegt.
|*                      2. der Point im heissen Bereich liegt.
|*                      3. der Point in der Flaeche liegt und es sich um
|*                         einen Rahmen mit NoTxtFrm handelt und dieser
|*                         keine URL traegt.
|*                      3a nicht aber wenn ueber dem Fly noch ein Fly liegt,
|*                         und der Point in dessen Flaeche nicht steht.
|*                      4. der Point in der Flaeche liegt und der Rahmen
|*                         selektiert ist.
|*
*************************************************************************/


/*************************************************************************
|*
|*  SwVirtFlyDrawObj::TakeObjInfo()
|*
*************************************************************************/



/*************************************************************************
|*
|*  SwVirtFlyDrawObj::Groessenermittlung
|*
*************************************************************************/

/*N*/ void SwVirtFlyDrawObj::SetRect() const
/*N*/ {
/*N*/   if ( GetFlyFrm()->Frm().HasArea() )
/*N*/       ((SwVirtFlyDrawObj*)this)->aOutRect = GetFlyFrm()->Frm().SVRect();
/*N*/   else
/*?*/       ((SwVirtFlyDrawObj*)this)->aOutRect = Rectangle();
/*N*/ }


/*N*/ const Rectangle& SwVirtFlyDrawObj::GetBoundRect() const
/*N*/ {
/*N*/   SetRect();
/*N*/   return aOutRect;
/*N*/ }






/*N*/ const Rectangle& SwVirtFlyDrawObj::GetSnapRect()  const
/*N*/ {
/*N*/   SetRect();
/*N*/   return aOutRect;
/*N*/ }













/*************************************************************************
|*
|*  SwVirtFlyDrawObj::Move() und Resize()
|*
*************************************************************************/















}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
