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

#include "svdstr.hrc"
#include "svdio.hxx"
#include "globl3d.hxx"
#include "label3d.hxx"

namespace binfilter {

/*N*/ TYPEINIT1(E3dLabelObj, E3dPointObj);

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

/*N*/ E3dLabelObj::~E3dLabelObj()
/*N*/ {
/*N*/   // zugehoeriges 2D-SdrObject loeschen
/*N*/   delete p2DLabelObj;
/*N*/ }

/*************************************************************************
|*
|* Identifier zurueckgeben
|*
\************************************************************************/

/*N*/ UINT16 E3dLabelObj::GetObjIdentifier() const
/*N*/ {
/*N*/   return E3D_LABELOBJ_ID;
/*N*/ }

/*************************************************************************
|*
|* SdrPage auch an p2DLabelObj setzen
|*
\************************************************************************/

/*N*/ void E3dLabelObj::SetPage(SdrPage* pNewPage)
/*N*/ {
/*N*/   E3dPointObj::SetPage(pNewPage);
/*N*/   if ( p2DLabelObj )
/*N*/       p2DLabelObj->SetPage(pNewPage);
/*N*/ }

/*************************************************************************
|*
|* SdrModel auch an p2DLabelObj setzen
|*
\************************************************************************/

/*N*/ void E3dLabelObj::SetModel(SdrModel* pNewModel)
/*N*/ {
/*N*/   E3dPointObj::SetModel(pNewModel);
/*N*/   if ( p2DLabelObj )
/*N*/       p2DLabelObj->SetModel(pNewModel);
/*N*/ }

/*************************************************************************
|*
|* Aus Stream laden
|*
\************************************************************************/

/*N*/ void E3dLabelObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
/*N*/ {
/*N*/   if (ImpCheckSubRecords (rHead, rIn))
/*N*/   {
/*N*/       E3dPointObj::ReadData(rHead, rIn);
/*N*/
/*N*/       SdrDownCompat aCompat(rIn, STREAM_READ);
/*N*/ #ifdef DBG_UTIL
/*N*/       aCompat.SetID("E3dLabelObj");
/*N*/ #endif
/*N*/
/*N*/   // loeschen, falls schon vorhanden
/*N*/       delete p2DLabelObj;
/*N*/       p2DLabelObj = NULL;
/*N*/
/*N*/       SdrObjIOHeaderLookAhead aHead(rIn, STREAM_READ);
/*N*/
/*N*/       p2DLabelObj = SdrObjFactory::MakeNewObject(aHead.nInventor,
/*N*/                                                  aHead.nIdentifier, GetPage());
/*N*/       if ( p2DLabelObj != NULL )
/*N*/       {
/*N*/           rIn >> *p2DLabelObj;
/*N*/           p2DLabelObj->SetModel(GetModel());
/*N*/       }
/*N*/       else
/*N*/       {
/*N*/           aHead.SkipRecord();
/*N*/       }
/*N*/   }
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
