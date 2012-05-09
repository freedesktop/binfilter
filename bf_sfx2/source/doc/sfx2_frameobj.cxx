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

#include <sot/clsids.hxx>
#include <bf_so3/ipwin.hxx>


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "frameobj.hxx"

#include "frmdescr.hxx"

#include <appdata.hxx>

#include "app.hxx"

#include "objsh.hxx"

#include "bf_so3/staticbaseurl.hxx"
namespace binfilter {







/*N*/ struct SfxFrameObject_Impl
/*  [Beschreibung]

    Impl-Struktur des SfxFrameObjects.
*/
/*N*/ {
/*N*/   SfxFrameDescriptor*     pFrmDescr;
/*N*/   Timer*                  pNewObjectTimer;
/*N*/   SfxObjectShellLock      xDoc;
/*N*/
/*N*/               SfxFrameObject_Impl()
/*N*/                   : pFrmDescr( new SfxFrameDescriptor( NULL ) )
/*N*/                   , pNewObjectTimer( NULL )
/*N*/               {
/*N*/               }
/*N*/
/*N*/               ~SfxFrameObject_Impl()
/*N*/               {
/*N*/                   // Der Descriptor geh"ort dem Object, andere erhalten
/*N*/                   // davon immer nur Clones
/*N*/                   delete pFrmDescr;
/*N*/               }
/*N*/ };


//=========================================================================
//============== SfxFrameObject ===========================================
//=========================================================================
SO2_IMPL_BASIC_CLASS1_DLL(
    SfxFrameObject,
    SvFactory,
    SvInPlaceObject,
    SvGlobalName( BF_SO3_IFRAME_CLASSID )
)

//=========================================================================
SfxFrameObject::SfxFrameObject()
    : pImpl( new SfxFrameObject_Impl )
{
}

//=========================================================================
SfxFrameObject::~SfxFrameObject()
{
    delete pImpl;
}

BOOL SfxFrameObject::Load
(
    SvStorage* /*pStor*/
)
{
    DBG_BF_ASSERT(0, "STRIP");  // VIRTUAL
    return FALSE;
}

//=========================================================================

SfxFrameObjectFactoryPtr* SfxFrameObject::GetFactoryPtr()
{
    return SFX_APP()->Get_Impl()->pSfxFrameObjectFactoryPtr;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
