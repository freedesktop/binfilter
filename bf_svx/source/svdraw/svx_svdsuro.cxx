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

#include "svdsuro.hxx"
#include "svdobj.hxx"
#include "svdpage.hxx"
#include "svdmodel.hxx"

namespace binfilter {

// fuer Lesen
/*N*/ SdrObjSurrogate::SdrObjSurrogate(const SdrModel& rMod, SvStream& rIn)
/*N*/ {
/*N*/   ImpClearVars();
/*N*/   pModel=&rMod;
/*N*/   ImpRead(rIn);
/*N*/ }

// fuer Lesen
/*N*/ SdrObjSurrogate::SdrObjSurrogate(const SdrObject& rRefObj, SvStream& rIn)
/*N*/ {
/*N*/   ImpClearVars();
/*N*/   pRefObj=&rRefObj;
/*N*/   pModel=rRefObj.GetModel();
/*N*/   ImpRead(rIn);
/*N*/ }

/*N*/ SdrObjSurrogate::~SdrObjSurrogate()
/*N*/ {
/*N*/   if (pGrpOrdNums!=NULL) delete [] pGrpOrdNums;
/*N*/ }

/*N*/ void SdrObjSurrogate::ImpClearVars()
/*N*/ {
/*N*/   pObj=NULL;
/*N*/   pRefObj=NULL;
/*N*/   pList=NULL;
/*N*/   pRootList=NULL;
/*N*/   pPage=NULL;
/*N*/   pModel=NULL;
/*N*/   eList=SDROBJLIST_UNKNOWN;
/*N*/   nOrdNum=0;
/*N*/   pGrpOrdNums=NULL;
/*N*/   nGrpLevel=0;
/*N*/   nPageNum=0;
/*N*/ }

/*N*/ void SdrObjSurrogate::ImpWriteValue(SvStream& rOut, UINT32 nVal, unsigned nByteAnz) const
/*N*/ {
/*N*/   switch (nByteAnz) {
/*N*/       case 0: rOut<<BYTE(nVal);   break;
/*N*/       case 1: rOut<<UINT16(nVal); break;
/*N*/       case 3: rOut<<nVal;         break;
/*N*/       default: OSL_FAIL("ImpWriteValue(): Nur Bytelaengen 1, 2 oder 4 sind erlaubt!");
/*N*/   } // switch
/*N*/ }

/*N*/ void SdrObjSurrogate::ImpReadValue(SvStream& rIn, UINT32& rVal, unsigned nByteAnz) const
/*N*/ {
/*N*/   switch (nByteAnz) {
/*N*/       case 0: { BYTE n;   rIn>>n; rVal=n; } break;
/*N*/       case 1: { UINT16 n; rIn>>n; rVal=n; } break;
/*N*/       case 3: {           rIn>>rVal;      } break;
/*N*/       default: OSL_FAIL("ImpReadValue(): Nur Bytelaengen 1, 2 oder 4 sind erlaubt!");
/*N*/   } // switch
/*N*/ }

/*N*/ SvStream& operator<<(SvStream& rOut, const SdrObjSurrogate& rSurro)
/*N*/ {
/*N*/   BYTE nId=BYTE(rSurro.eList);
/*N*/   if (rSurro.eList==SDROBJLIST_UNKNOWN) {
/*N*/       rOut<<nId;
/*N*/   } else {
/*N*/       bool bGrp=rSurro.nGrpLevel!=0;
/*N*/       if (bGrp) nId|=0x20; // Gruppierung
/*N*/       ULONG nMaxOrdNum=rSurro.nOrdNum;
/*N*/       unsigned i;
/*N*/       for (i=0; i<rSurro.nGrpLevel; i++) {
/*?*/           if (nMaxOrdNum<rSurro.pGrpOrdNums[i]) nMaxOrdNum=rSurro.pGrpOrdNums[i];
/*N*/       }
/*N*/       unsigned nByteAnz=0;
/*N*/       if (nMaxOrdNum>0xFF) nByteAnz++;
/*N*/       if (nMaxOrdNum>0xFFFF) nByteAnz+=2;
/*N*/       nId|=BYTE(nByteAnz<<6); // Byteanzahl an der richtigen Stelle im ID-Byte eintragen
/*N*/       rOut<<nId;
/*N*/       rSurro.ImpWriteValue(rOut,rSurro.nOrdNum,nByteAnz);
/*N*/       if (SdrIsPageKind(rSurro.eList)) { // Seitennummer schreiben
/*N*/           rOut<<rSurro.nPageNum;
/*N*/       }
/*N*/       if (bGrp) { // Gruppierung
/*?*/           rOut<<rSurro.nGrpLevel;
/*?*/           for (i=0; i<rSurro.nGrpLevel; i++) {
/*?*/               rSurro.ImpWriteValue(rOut,rSurro.pGrpOrdNums[i],nByteAnz);
/*N*/           }
/*N*/       }
/*N*/   }
/*N*/   return rOut;
/*N*/ }

/*N*/ void SdrObjSurrogate::ImpRead(SvStream& rIn)
/*N*/ {
/*N*/   BYTE nId;
/*N*/   rIn>>nId;
/*N*/   eList=SdrObjListKind(nId & 0x1F);
/*N*/   if (eList!=SDROBJLIST_UNKNOWN) {
/*N*/       bool bGrp=(nId & 0x20)!=0;      // Id-Byte
/*N*/       unsigned nByteAnz=(nId & 0xC0)>>6;  // aufschluesseln
/*N*/       ImpReadValue(rIn,nOrdNum,nByteAnz); // Ordnungsnummer des Objekts
/*N*/       if (SdrIsPageKind(eList)) { // Seitennummer schreiben
/*N*/           rIn>>nPageNum;
/*N*/       }
/*N*/       if (bGrp) { // Gruppierung
/*?*/           rIn>>nGrpLevel;
/*?*/           pGrpOrdNums=new UINT32[nGrpLevel];
/*?*/           for (unsigned i=0; i<nGrpLevel; i++) {
/*?*/               ImpReadValue(rIn,pGrpOrdNums[i],nByteAnz);
/*?*/           }
/*N*/       }
/*N*/   }
/*N*/ }

/*N*/ void SdrObjSurrogate::ImpFindObj()
/*N*/ {
/*N*/   if (eList!=SDROBJLIST_UNKNOWN) {
/*N*/       switch (eList) {
/*N*/           case SDROBJLIST_SAMEPAGE: case SDROBJLIST_SAMELIST: {
/*N*/               if (pRefObj!=NULL) {
/*N*/                   pPage=pRefObj->GetPage();
/*N*/                   if (eList==SDROBJLIST_SAMELIST) {
/*N*/                       pRootList=pRefObj->GetObjList();
/*N*/                       if (pRootList==NULL) {
/*N*/                           OSL_FAIL("SdrObjSurrogate::ImpFindObj(): eList=SDROBJLIST_SAMELIST, pRefObj hat jedoch keine ObjList");
/*N*/                           return;
/*N*/                       }
/*N*/                   } else {
/*N*/                       if (pPage!=NULL) {
/*N*/                           pRootList=pPage;
/*N*/                       } else {
/*N*/                           OSL_FAIL("SdrObjSurrogate::ImpFindObj(): eList=SDROBJLIST_SAMEPAGE, pRefObj hat jedoch keine Page");
/*N*/                           return;
/*N*/                       }
/*N*/                   }
/*N*/               } else {
/*N*/                   OSL_FAIL("SdrObjSurrogate::ImpFindObj(): Relatives Surrogat benoetigt Referenzobjekt");
/*N*/                   return;
/*N*/               }
/*N*/           } break;
/*N*/           case SDROBJLIST_DRAWPAGE: pPage=pModel->GetPage(nPageNum); pRootList=pPage; break;
/*N*/           case SDROBJLIST_MASTERPAGE: pPage=pModel->GetMasterPage(nPageNum); pRootList=pPage; break;
/*N*/           case SDROBJLIST_VIRTOBJECTS: {
/*N*/               OSL_FAIL("SdrObjSurrogate::ImpFindObj(): Persistente virtuelle Objekte in dieser Version nicht verfuegbar");
/*N*/               return;
/*N*/           } break;
/*N*/           case SDROBJLIST_SYMBOLTABLE: {
/*N*/               OSL_FAIL("SdrObjSurrogate::ImpFindObj(): Symbole in dieser Version nicht verfuegbar");
/*N*/               return;
/*N*/           } break;
/*N*/           default: {
/*N*/               OSL_FAIL("SdrObjSurrogate::ImpFindObj(): Unbekannter oder unsinniger Objektlistentyp");
/*N*/               return;
/*N*/           }
/*N*/       } // switch
/*N*/       pList=pRootList;
/*N*/       for (unsigned i=0; i<nGrpLevel; i++) {
/*?*/           SdrObject* pO=pList->GetObj(pGrpOrdNums[i]);
/*?*/           if (pO!=NULL) {
/*?*/               pList=pO->GetSubList();
/*?*/               if (pList==NULL) {
/*?*/                   OSL_FAIL("SdrObjSurrogate::ImpFindObj(): Objekt ist kein Gruppenobjekt");
/*?*/                   return;
/*?*/               }
/*?*/           } else {
/*?*/               OSL_FAIL("SdrObjSurrogate::ImpFindObj(): Gruppenobjekt nicht gefunden");
/*?*/               return;
/*?*/           }
/*N*/       }
/*N*/       pObj=pList->GetObj(nOrdNum);
/*N*/       DBG_ASSERT(pObj!=NULL,"SdrObjSurrogate::ImpFindObj(): Zielobjekt nicht gefunden");
/*N*/   }
/*N*/ }

/*N*/ SdrObject* SdrObjSurrogate::GetObject()
/*N*/ {
/*N*/   if (pObj==NULL && eList!=SDROBJLIST_UNKNOWN) ImpFindObj();
/*N*/   return pObj;
/*N*/ }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
