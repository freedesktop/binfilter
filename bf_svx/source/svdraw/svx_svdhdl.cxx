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

#include "svdetc.hxx"
#include "svdmrkv.hxx"
#include "svdstr.hrc"

// #105678#
namespace binfilter {

SdrHdl::~SdrHdl() {}

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ class ImplHdlListData
/*N*/ {
/*N*/ public:
/*N*/   ULONG                   mnFocusIndex;
/*N*/   SdrMarkView*                pView;
/*N*/
/*N*/   ImplHdlListData(SdrMarkView* pV): mnFocusIndex(CONTAINER_ENTRY_NOTFOUND), pView(pV) {}
/*N*/ };


/*N*/ SdrHdlList::SdrHdlList(SdrMarkView* pV)
/*N*/ : pImpl(new ImplHdlListData(pV))
/*N*/ , aList(1024,32,32)
/*N*/ {
/*N*/   nHdlSize = 3;
/*N*/   bRotateShear = FALSE;
/*N*/   bMoveOutside = FALSE;
/*N*/   bDistortShear = FALSE;
/*N*/ }
/*N*/
/*N*/ SdrHdlList::~SdrHdlList()
/*N*/ {
/*N*/   Clear();
/*N*/   delete pImpl;
/*N*/ }


/*N*/ void SdrHdlList::Clear()
/*N*/ {
/*N*/   aList.Clear();
/*N*/
/*N*/   bRotateShear=FALSE;
/*N*/   bDistortShear=FALSE;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
