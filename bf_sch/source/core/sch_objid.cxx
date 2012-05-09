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

#include "objid.hxx"
#include "glob.hxx"

#include "defines.hxx"

#include <tools/debug.hxx>

namespace binfilter {

/*************************************************************************
|*
|* Objekt-Id ermitteln;
|* Liefert -1, wenn das Objekt keine Id hat
|*
\************************************************************************/

/*N*/ SchObjectId* GetObjectId(const SdrObject& rObj)
/*N*/ {
/*N*/   for (USHORT i = 0; i < rObj.GetUserDataCount(); i++)
/*N*/   {
/*N*/       SdrObjUserData *pData = rObj.GetUserData(i);
/*N*/       if (pData && pData->GetId() == SCH_OBJECTID_ID)
/*N*/           return (SchObjectId*)pData;
/*N*/   }
/*N*/
/*N*/   return NULL;
/*N*/ }


/*************************************************************************
|*
|* Objekt mit Id suchen;
|* liefert NULL, wenn kein Objekt gefunden wurde.
|*
\************************************************************************/

/*N*/ SdrObject* GetObjWithId(UINT16 nObjId, const SdrObjList& rObjList,
/*N*/                       ULONG* pIndex, SdrIterMode eMode)
/*N*/ {
/*N*/   ULONG nIndex = 0;
/*N*/
/*N*/   SdrObjListIter aIterator(rObjList, eMode);
/*N*/   while (aIterator.IsMore())
/*N*/   {
/*N*/       SdrObject* pObj = aIterator.Next();
/*N*/       SchObjectId* pObjId = GetObjectId(*pObj);
/*N*/       if (pObjId && pObjId->GetObjId() == nObjId)
/*N*/       {
/*N*/           if (pIndex)
/*?*/               *pIndex = nIndex;
/*N*/           return pObj;
/*N*/       }
/*N*/
/*N*/       nIndex++;
/*N*/   }
/*N*/
/*N*/   return NULL;
/*N*/ }

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

/*N*/ SchObjectId::SchObjectId() :
/*N*/   SdrObjUserData(SchInventor, SCH_OBJECTID_ID, 0),
/*N*/   nObjId(0)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

/*N*/ SchObjectId::SchObjectId(UINT16 nId) :
/*N*/   SdrObjUserData(SchInventor, SCH_OBJECTID_ID, 0),
/*N*/   nObjId(nId)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|* Daten aus Stream lesen
|*
\************************************************************************/

/*N*/ void SchObjectId::ReadData(SvStream& rIn)
/*N*/ {
/*N*/   SdrObjUserData::ReadData(rIn);
/*N*/
/*N*/   rIn >> nObjId;
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
