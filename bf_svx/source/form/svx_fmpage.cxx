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

#include "fmpage.hxx"

#include "fmmodel.hxx"

#ifndef SVX_LIGHT
#include "fmresids.hrc"
#endif

#ifndef SVX_LIGHT
#include "fmpgeimp.hxx"
#endif

#include "svdio.hxx"

#ifndef SVX_LIGHT
#include "fmprop.hrc"
using namespace ::binfilter::svxform;
#endif

namespace binfilter {

/*N*/ TYPEINIT1(FmFormPage, SdrPage);

//------------------------------------------------------------------
/*N*/ FmFormPage::FmFormPage(FmFormModel& rModel, StarBASIC* _pBasic, bool bMasterPage)
/*N*/          :SdrPage(rModel, bMasterPage)
/*N*/ #ifndef SVX_LIGHT
/*N*/          ,pImpl(new FmFormPageImpl(this))
/*N*/ #else
/*N*/          ,pImpl(NULL)
/*N*/ #endif
/*N*/          ,pBasic(_pBasic)
/*N*/ {
/*N*/ }

/*N*/ FmFormPage::~FmFormPage()
/*N*/ {
/*N*/ #ifndef SVX_LIGHT
/*N*/   delete pImpl;
/*N*/ #endif
/*N*/ }

/*N*/ void FmFormPage::ReadData(const SdrIOHeader& rHead, SvStream& rIn)
/*N*/ {
/*N*/   {
/*N*/       {
/*N*/           SdrDownCompat aVCCompat1( rIn, STREAM_READ );
/*N*/       }
/*N*/       SdrPage::ReadData( rHead, rIn );
/*N*/       {
/*N*/           SdrDownCompat aVCCompat2( rIn, STREAM_READ );
/*N*/           aPageName = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rIn,
/*N*/               osl_getThreadTextEncoding());
/*N*/       }
/*N*/   }
/*N*/
/*N*/   // dont use the flag in that way: if (rIn.GetVersion() >= SOFFICE_FILEFORMAT_40)
/*N*/   if (rIn.GetVersion() >= 3830 && rHead.GetVersion() >=14)
/*N*/   {
/*N*/       SdrDownCompat aCompat(rIn, STREAM_READ);    // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ #ifndef SVX_LIGHT
/*N*/       DBG_ASSERT( aCompat.GetBytesLeft(), "FmFormPage::ReadData: invalid file format!" );
/*N*/       if ( aCompat.GetBytesLeft() )
/*N*/           pImpl->ReadData(rHead, rIn);
/*N*/       // some old (corrupted) versions between 511 and 554 wrote an empty block here - and some of these documents
/*N*/       // are still out there
/*N*/       // So we allow for such an empty block ...
/*N*/ #endif
/*N*/   }
/*N*/ }

//------------------------------------------------------------------
/*N*/ void FmFormPage::SetModel(SdrModel* pNewModel)
/*N*/ {
/*N*/   SdrPage::SetModel( pNewModel );
/*N*/ }

//------------------------------------------------------------------
/*N*/ SdrPage* FmFormPage::Clone() const
/*N*/ {
/*N*/   return new FmFormPage(*this);
/*N*/   // hier fehlt noch ein kopieren der Objekte
/*N*/ }

//------------------------------------------------------------------
/*N*/ void FmFormPage::InsertObject(SdrObject* pObj, ULONG nPos,
/*N*/                             const SdrInsertReason* pReason)
/*N*/ {
/*N*/   SdrPage::InsertObject( pObj, nPos, pReason );
/*N*/ }

//------------------------------------------------------------------
/*N*/ const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > & FmFormPage::GetForms() const
/*N*/ {
/*N*/ #ifndef SVX_LIGHT
/*N*/   return pImpl->getForms();
/*N*/ #else
/*N*/   static ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >  aRef;
/*N*/   return aRef;
/*N*/ #endif
/*N*/ }

//------------------------------------------------------------------
/*N*/ SdrObject* FmFormPage::RemoveObject(ULONG nObjNum)
/*N*/ {
/*N*/   SdrObject* pObj = SdrPage::RemoveObject(nObjNum);
/*N*/   return pObj;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
