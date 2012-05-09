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

#include <tools/debug.hxx>
#include <bf_tools/string.hxx>
#include "svdio.hxx"
#include "svdobj.hxx"
#include <osl/endian.h>
namespace binfilter {

////////////////////////////////////////////////////////////////////////////////////////////////////


/*N*/ SdrIOHeader::SdrIOHeader(SvStream& rNewStream, UINT16 nNewMode, const char cID[2],
/*N*/   BOOL bAutoOpen)
/*N*/ : rStream(rNewStream)
/*N*/ {
/*N*/   nFilePos = 0;
/*N*/   nMode = nNewMode;
/*N*/   bOpen = FALSE;
/*N*/   bClosed = FALSE;
/*N*/   bLookAhead = FALSE;
/*N*/   SetID(cID);
/*N*/   nVersion = nAktSdrFileVersion;
/*N*/   nBlkSize = 0;
/*N*/   SetMagic();
/*N*/
/*N*/   if(bAutoOpen)
/*N*/   {
/*N*/       OpenRecord();
/*N*/   }
/*N*/ }

/*N*/ SdrIOHeader::~SdrIOHeader()
/*N*/ {
/*N*/   if(bOpen && !bClosed && !bLookAhead)
/*N*/       CloseRecord();
/*N*/ }

/*N*/ void SdrIOHeader::OpenRecord()
/*N*/ {
/*N*/   if(rStream.GetError())
/*?*/       return;
/*N*/
/*N*/   DBG_ASSERT(!bClosed,"SdrIOHeader::OpenRecord(): Record ist bereits geschlossen.");
/*N*/   DBG_ASSERT(!bOpen,"SdrIOHeader::OpenRecord(): Record ist bereits geoeffnet.");
/*N*/
/*N*/   // Fileposition des Records merken
/*N*/   nFilePos = rStream.Tell();
/*N*/
/*N*/   if(nMode==STREAM_READ)
/*N*/   {
/*N*/       // RecordHeader lesen
/*N*/       Read();
/*N*/   }
/*N*/   else if(nMode == STREAM_WRITE)
/*N*/   {
/*N*/       // Platz fuer RecordHeader schaffen
/*N*/       Write();
/*N*/   }
/*N*/   else
/*N*/       OSL_FAIL("SdrIOHeader::OpenRecord(): Falscher StreamMode angegeben.");
/*N*/
/*N*/   bOpen = TRUE;
/*N*/
/*N*/   // Endemarke wird sofort geschlossen.
/*N*/   if(IsEnde() && !bLookAhead)
/*N*/       CloseRecord();
/*N*/
/*N*/   if(bLookAhead)
/*N*/       rStream.Seek(nFilePos);
/*N*/ }

/*N*/ void SdrIOHeader::CloseRecord()
/*N*/ {
/*N*/   if(rStream.GetError())
/*N*/       return;
/*N*/
/*N*/   if(bLookAhead)
/*N*/   {
/*?*/       rStream.Seek(nFilePos);
/*?*/       OSL_FAIL("SdrIOHeader::CloseRecord(): CloseRecord im Modus LookAhead.");
/*?*/       return;
/*N*/   }
/*N*/
/*N*/   DBG_ASSERT(!bClosed,"SdrIOHeader::CloseRecord(): Record ist bereits geschlossen.");
/*N*/   DBG_ASSERT(bOpen || bClosed,"SdrIOHeader::CloseRecord(): Record ist noch nicht geoeffnet.");
/*N*/
/*N*/   UINT32 nAktPos(rStream.Tell());
/*N*/
/*N*/   if(nMode == STREAM_READ)
/*N*/   {
/*N*/       UINT32 nReadAnz(nAktPos - nFilePos);
/*N*/
/*N*/       if(nReadAnz != nBlkSize)
/*N*/       {
/*N*/           // FilePos korregieren
/*N*/           rStream.Seek(nFilePos + nBlkSize);
/*N*/       }
/*N*/   }
/*N*/   else if(nMode == STREAM_WRITE)
/*N*/   {
/*N*/       // Groesse dieses Records (inkl. der Header)
/*N*/       nBlkSize = nAktPos - nFilePos;
/*N*/       // an den Anfang des Records
/*N*/       rStream.Seek(nFilePos);
/*N*/       // Header rausschreiben.
/*N*/       Write();
/*N*/       // und die FilePos restaurieren.
/*N*/       rStream.Seek(nAktPos);
/*N*/   }
/*N*/   else
/*N*/       OSL_FAIL("SdrIOHeader::CloseRecord(): Falscher StreamMode angegeben.");
/*N*/
/*N*/   bOpen = FALSE;
/*N*/   bClosed = TRUE;
/*N*/ }

/*N*/ void SdrIOHeader::Write()
/*N*/ {
/*N*/ #ifdef OSL_BIGENDIAN
/*N*/   nVersion=SWAPSHORT(nVersion);
/*N*/   nBlkSize=SWAPLONG (nBlkSize);
/*N*/ #endif
/*N*/
/*N*/   rStream.Write(cMagic, 4);
/*N*/
/*N*/   if(!IsEnde())
/*N*/   {
/*N*/       rStream.Write((char*)&nVersion, 2);
/*N*/       rStream.Write((char*)&nBlkSize, 4);
/*N*/   }
/*N*/
/*N*/ #ifdef OSL_BIGENDIAN
/*N*/   nVersion=SWAPSHORT(nVersion);
/*N*/   nBlkSize=SWAPLONG (nBlkSize);
/*N*/ #endif
/*N*/ }


/*N*/ void SdrIOHeader::Read()
/*N*/ {
/*N*/   rStream.Read(cMagic, 4);
/*N*/
/*N*/   if(!IsEnde())
/*N*/   {
/*N*/       rStream.Read((char*)&nVersion, 2);
/*N*/       rStream.Read((char*)&nBlkSize, 4);
/*N*/
/*N*/ #ifdef OSL_BIGENDIAN
/*N*/       nVersion=SWAPSHORT(nVersion);
/*N*/       nBlkSize=SWAPLONG (nBlkSize);
/*N*/ #endif
/*N*/   }
/*N*/   else
/*N*/   {
/*N*/       nBlkSize = 4;
/*N*/   }
/*N*/ }

/*N*/ UINT32 SdrIOHeader::GetBytesLeft() const
/*N*/ {
/*N*/   if(nMode == STREAM_READ)
/*N*/   {
/*N*/       UINT32 nAktPos(rStream.Tell());
/*N*/       UINT32 nReadAnz(nAktPos - nFilePos);
/*N*/
/*N*/       if(nReadAnz <= nBlkSize)
/*N*/           return nBlkSize - nReadAnz;
/*N*/       else
/*N*/           // Fehler, zuviel gelesen!
/*N*/           return 0;
/*N*/   }
/*N*/   else
/*?*/       return 0;
/*N*/ }

/*N*/ SdrIOHeaderLookAhead::SdrIOHeaderLookAhead(SvStream& rNewStream, BOOL bAutoOpen)
/*N*/ : SdrIOHeader(rNewStream, STREAM_READ, SdrIOEndeID, FALSE)
/*N*/ {
/*N*/   bLookAhead = TRUE;
/*N*/
/*N*/   if(bAutoOpen)
/*N*/   {
/*N*/       OpenRecord();
/*N*/   }
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ SdrObjIOHeader::SdrObjIOHeader(SvStream& rNewStream, UINT16 nNewMode,
/*N*/   const SdrObject* pNewObj, BOOL bAutoOpen)
/*N*/ : SdrIOHeader(rNewStream, nNewMode, SdrIODObjID, FALSE),
/*N*/   pObj(pNewObj)
/*N*/ {
/*N*/   if(pNewObj)
/*N*/   {
/*N*/       nInventor = pNewObj->GetObjInventor();
/*N*/       nIdentifier = pNewObj->GetObjIdentifier();
/*N*/   }
/*N*/   else
/*N*/   {
/*N*/       nInventor = 0;
/*N*/       nIdentifier = 0;
/*N*/   }
/*N*/
/*N*/   if(bAutoOpen)
/*N*/   {
/*N*/       OpenRecord();
/*N*/   }
/*N*/ }

/*N*/ SdrObjIOHeader::~SdrObjIOHeader()
/*N*/ {
/*N*/   if(bOpen && !bClosed && !bLookAhead)
/*N*/       CloseRecord();
/*N*/ }

/*N*/ void SdrObjIOHeader::Write()
/*N*/ {
/*N*/   SdrIOHeader::Write();
/*N*/
/*N*/   if(!IsEnde())
/*N*/   {
/*N*/ #ifdef OSL_BIGENDIAN
/*N*/       nInventor = SWAPLONG(nInventor);
/*N*/       nIdentifier = SWAPSHORT(nIdentifier);
/*N*/ #endif
/*N*/       rStream.Write((char*)&nInventor, 4);
/*N*/       rStream.Write((char*)&nIdentifier, 2);
/*N*/ #ifdef OSL_BIGENDIAN
/*N*/       nInventor = SWAPLONG(nInventor);
/*N*/       nIdentifier = SWAPSHORT(nIdentifier);
/*N*/ #endif
/*N*/   }
/*N*/ }

/*N*/ void SdrObjIOHeader::Read()
/*N*/ {
/*N*/   SdrIOHeader::Read();
/*N*/
/*N*/   if(!IsEnde())
/*N*/   {
/*N*/       rStream.Read((char*)&nInventor, 4);
/*N*/       rStream.Read((char*)&nIdentifier, 2);
/*N*/ #ifdef OSL_BIGENDIAN
/*N*/       nInventor = SWAPLONG(nInventor);
/*N*/       nIdentifier = SWAPSHORT(nIdentifier);
/*N*/ #endif
/*N*/   }
/*N*/ }

/*N*/ SdrObjIOHeaderLookAhead::SdrObjIOHeaderLookAhead(SvStream& rNewStream,
/*N*/   BOOL bAutoOpen)
/*N*/ : SdrObjIOHeader(rNewStream, STREAM_READ, NULL, FALSE)
/*N*/ {
/*N*/   bLookAhead = TRUE;
/*N*/
/*N*/   if(bAutoOpen)
/*N*/   {
/*N*/       OpenRecord();
/*N*/   }
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ SdrDownCompat::SdrDownCompat(SvStream& rNewStream, UINT16 nNewMode, BOOL bAutoOpen)
/*N*/ : rStream(rNewStream),
/*N*/   nSubRecSiz(0),
/*N*/   nSubRecPos(0),
/*N*/   nMode(nNewMode),
/*N*/   bOpen(FALSE),
/*N*/   bClosed(FALSE),
/*N*/   pRecId(NULL)
/*N*/ {
/*N*/   if(bAutoOpen)
/*N*/       OpenSubRecord();
/*N*/ }

/*N*/ SdrDownCompat::~SdrDownCompat()
/*N*/ {
/*N*/   if(!bClosed)
/*N*/       CloseSubRecord();
/*N*/ }

/*N*/ void SdrDownCompat::Read()
/*N*/ {
/*N*/   // Subrecordgroesse auslesen
/*N*/   rStream >> nSubRecSiz;
/*N*/ }

/*N*/ void SdrDownCompat::Write()
/*N*/ {
/*N*/   // Platz fuer Groessenangabe schaffen
/*N*/   rStream << nSubRecSiz;
/*N*/ }

/*N*/ void SdrDownCompat::OpenSubRecord()
/*N*/ {
/*N*/   if(rStream.GetError())
/*N*/       return;
/*N*/
/*N*/   DBG_ASSERT(!bClosed,"SdrDownCompat::OpenSubRecord(): Record ist bereits geschlossen.");
/*N*/   DBG_ASSERT(!bOpen,"SdrDownCompat::OpenSubRecord(): Record ist bereits geoeffnet.");
/*N*/
/*N*/   // Fileposition des Records merken
/*N*/   nSubRecPos = rStream.Tell();
/*N*/
/*N*/   if(nMode == STREAM_READ)
/*N*/   {
/*N*/       // Subrecordgroesse auslesen
/*N*/       Read();
/*N*/   }
/*N*/   else if(nMode == STREAM_WRITE)
/*N*/   {
/*N*/       // Platz fuer Groessenangabe schaffen
/*N*/       Write();
/*N*/   }
/*N*/   else
/*N*/       OSL_FAIL("SdrDownCompat::OpenSubRecord(): Falscher StreamMode angegeben.");
/*N*/
/*N*/   bOpen = TRUE;
/*N*/ }

/*N*/ void SdrDownCompat::CloseSubRecord()
/*N*/ {
/*N*/   if(rStream.GetError())
/*?*/       return;
/*N*/
/*N*/   DBG_ASSERT(!bClosed,"SdrDownCompat::CloseSubRecord(): Record ist bereits geschlossen.");
/*N*/   DBG_ASSERT(bOpen || bClosed,"SdrDownCompat::CloseSubRecord(): Record ist noch nicht geoeffnet.");
/*N*/
/*N*/   UINT32 nAktPos(rStream.Tell());
/*N*/
/*N*/   if(nMode == STREAM_READ)
/*N*/   {
/*N*/       UINT32 nReadAnz(nAktPos - nSubRecPos);
/*N*/
/*N*/       if(nReadAnz != nSubRecSiz)
/*N*/       {
/*N*/           // den Rest ueberspringen
/*N*/           rStream.Seek(nSubRecPos + nSubRecSiz);
/*N*/       }
/*N*/   }
/*N*/   else if(nMode == STREAM_WRITE)
/*N*/   {
/*N*/       // Groesse dieses SubRecords (inkl. der Groessenangabe selbst)
/*N*/       nSubRecSiz = nAktPos - nSubRecPos;
/*N*/
/*N*/       // an den Anfang des Records
/*N*/       rStream.Seek(nSubRecPos);
/*N*/       // rausschreiben.
/*N*/       Write();
/*N*/       // und die FilePos restaurieren.
/*N*/       rStream.Seek(nAktPos);
/*N*/   }
/*N*/   else
/*N*/       OSL_FAIL("SdrDownCompat::CloseSubRecord(): Falscher StreamMode angegeben.");
/*N*/
/*N*/   bOpen = FALSE;
/*N*/   bClosed = TRUE;
/*N*/ }

/*N*/ UINT32 SdrDownCompat::GetBytesLeft() const
/*N*/ {
/*N*/   if(nMode == STREAM_READ)
/*N*/   {
/*N*/       UINT32 nAktPos(rStream.Tell());
/*N*/       UINT32 nReadAnz(nAktPos - nSubRecPos);
/*N*/
/*N*/       if(nReadAnz <= nSubRecSiz)
/*N*/           return nSubRecSiz - nReadAnz;
/*N*/       else
/*N*/       // Fehler, zuviel gelesen!
/*N*/           return 0;
/*N*/   }
/*N*/   else
/*N*/       return 0;
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ SdrNamedSubRecord::SdrNamedSubRecord(SvStream& rNewStream, UINT16 nNewMode,
/*N*/   UINT32 nNewInventor, UINT16 nNewIdentifier,
/*N*/   BOOL bAutoOpen)
/*N*/ : SdrDownCompat(rNewStream, nNewMode, FALSE),
/*N*/   nInventor(nNewInventor),
/*N*/   nIdentifier(nNewIdentifier)
/*N*/ {
/*N*/   DBG_ASSERT(nNewMode==STREAM_READ || (nNewInventor!=0xFFFF && nNewInventor!=0xFFFF),
/*N*/       "SdrNamedSubRecord: bei Write muss Inventor und Identifier angegeben werden");
/*N*/
/*N*/   if(bAutoOpen)
/*N*/       OpenSubRecord();
/*N*/ }

/*N*/ void SdrNamedSubRecord::Read()
/*N*/ {
/*N*/   SdrDownCompat::Read();
/*N*/
/*N*/   rStream.Read((char*)&nInventor, 4);
/*N*/   rStream.Read((char*)&nIdentifier, 2);
/*N*/
/*N*/ #ifdef OSL_BIGENDIAN
/*N*/   nIdentifier = SWAPSHORT(nIdentifier);
/*N*/   nInventor = SWAPLONG(nInventor);
/*N*/ #endif
/*N*/ }

/*N*/ void SdrNamedSubRecord::Write()
/*N*/ {
/*N*/   SdrDownCompat::Write();
/*N*/
/*N*/ #ifdef OSL_BIGENDIAN
/*N*/   nIdentifier = SWAPSHORT(nIdentifier);
/*N*/   nInventor = SWAPLONG(nInventor);
/*N*/ #endif
/*N*/
/*N*/   rStream.Write((char*)&nInventor, 4);
/*N*/   rStream.Write((char*)&nIdentifier, 2);
/*N*/
/*N*/ #ifdef OSL_BIGENDIAN
/*N*/   nIdentifier = SWAPSHORT(nIdentifier);
/*N*/   nInventor = SWAPLONG(nInventor);
/*N*/ #endif
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
