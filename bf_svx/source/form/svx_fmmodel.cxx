/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#ifndef SVX_LIGHT
#include <bf_sfx2/objsh.hxx>
#else
class SfxObjectShell;
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "fmmodel.hxx"

#include "svdio.hxx"

#include "fmpage.hxx"

#include "svdobj.hxx"

namespace binfilter {

/*N*/ TYPEINIT1(FmFormModel, SdrModel);

struct FmFormModelImplData
{
    XubString               sNextPageId;
    sal_Bool                bOpenInDesignIsDefaulted;
    sal_Bool                bMovingPage;

    FmFormModelImplData()
        :bOpenInDesignIsDefaulted( sal_True )
        ,bMovingPage( sal_False )
    {
    }
};

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/
/*N*/ FmFormModel::FmFormModel(const XubString& rPath, SfxItemPool* pPool, SvPersist* pPers)
/*N*/           :SdrModel(rPath, pPool, pPers)
/*N*/           ,m_pImpl(NULL)
/*N*/           ,pObjShell(0)
/*N*/           ,bStreamingOldVersion(sal_False)
/*N*/           ,m_bOpenInDesignMode(sal_False)
/*N*/           ,m_bAutoControlFocus(sal_False)
/*N*/ {
/*N*/ #ifndef SVX_LIGHT
/*N*/   m_pImpl = new FmFormModelImplData;
/*N*/   m_pImpl->sNextPageId = '0';
/*N*/ #endif
/*N*/ }

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/
/*N*/ FmFormModel::FmFormModel(const XubString& rPath, SfxItemPool* pPool, SvPersist* pPers,
/*N*/                        bool bUseExtColorTable)
/*N*/           :SdrModel(rPath, pPool, pPers, bUseExtColorTable, LOADREFCOUNTS)
/*N*/           ,pObjShell(0)
/*N*/           ,bStreamingOldVersion(sal_False)
/*N*/           ,m_bOpenInDesignMode(sal_False)
/*N*/           ,m_bAutoControlFocus(sal_False)
/*N*/ {
/*N*/ #ifndef SVX_LIGHT
/*N*/   m_pImpl = new FmFormModelImplData;
/*N*/   m_pImpl->sNextPageId = '0';
/*N*/ #endif
/*N*/ }

/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/
/*N*/ FmFormModel::~FmFormModel()
/*N*/ {
/*N*/       SetObjectShell(NULL);
/*N*/   delete m_pImpl;
/*N*/ }

/*************************************************************************
|*
|* ReadData
|*
\************************************************************************/
/*N*/ void FmFormModel::ReadData(const SdrIOHeader& rHead, SvStream& rIn)
/*N*/ {
/*N*/   if( rIn.GetVersion() < SOFFICE_FILEFORMAT_50 )
/*N*/       ((FmFormModel*)this)->bStreamingOldVersion = sal_True;
/*N*/
/*N*/   SdrModel::ReadData( rHead, rIn );
/*N*/
/*N*/   //////////////////////////////////////////////////////////////////////
/*N*/   // Lesen der Option OpenInDesignMode
/*N*/   if (!bStreamingOldVersion)
/*N*/   {
/*N*/       SdrDownCompat aCompat(rIn,STREAM_READ);
/*N*/       sal_uInt8 nTemp = 0;
/*N*/       rIn >> nTemp;
/*N*/
/*N*/       implSetOpenInDesignMode( nTemp ? sal_True : sal_False, sal_True );
/*N*/
/*N*/       if (aCompat.GetBytesLeft())
/*N*/       {   // it is a version which already wrote the AutoControlFocus flag
/*N*/           rIn >> nTemp;
/*N*/           m_bAutoControlFocus = nTemp ? sal_True : sal_False;
/*N*/       }
/*N*/   }
/*N*/
/*N*/   ((FmFormModel*)this)->bStreamingOldVersion = sal_False;
/*N*/ }


/*************************************************************************
|*
|* InsertPage
|*
\************************************************************************/
void FmFormModel::InsertPage(SdrPage* pPage, sal_uInt16 nPos)
{
    SetObjectShell(pObjShell);
    SdrModel::InsertPage( pPage, nPos );
}

/*************************************************************************
|*
|* MovePage
|*
\************************************************************************/
void FmFormModel::MovePage( USHORT, USHORT )
{
    DBG_BF_ASSERT(0, "STRIP");  // VIRTUAL
}

/*************************************************************************
|*
|* RemovePage
|*
\************************************************************************/
/*N*/ SdrPage* FmFormModel::RemovePage(sal_uInt16 nPgNum)
/*N*/ {
/*N*/   FmFormPage* pPage = (FmFormPage*)SdrModel::RemovePage(nPgNum);
/*N*/
/*N*/
/*N*/   return pPage;
/*N*/ }

/*************************************************************************
|*
|* InsertMasterPage
|*
\************************************************************************/
/*N*/ void FmFormModel::InsertMasterPage(SdrPage* pPage, sal_uInt16 nPos)
/*N*/ {
/*N*/       SetObjectShell(pObjShell);
/*N*/
/*N*/   SdrModel::InsertMasterPage(pPage, nPos);
/*N*/
/*N*/ }

/*************************************************************************
|*
|* RemoveMasterPage
|*
\************************************************************************/
/*N*/ SdrPage* FmFormModel::RemoveMasterPage(sal_uInt16 nPgNum)
/*N*/ {
/*N*/   FmFormPage* pPage = (FmFormPage*)SdrModel::RemoveMasterPage(nPgNum);
/*N*/
/*N*/   return pPage;
/*N*/ }

//------------------------------------------------------------------------
/*N*/ SdrLayerID FmFormModel::GetControlExportLayerId( const SdrObject& rObj ) const
/*N*/ {
/*N*/   return rObj.GetLayer();
/*N*/ }

//------------------------------------------------------------------------
/*N*/ void FmFormModel::implSetOpenInDesignMode( sal_Bool _bOpenDesignMode, sal_Bool _bForce )
/*N*/ {
/*N*/   if( ( _bOpenDesignMode != m_bOpenInDesignMode ) || _bForce )
/*N*/   {
/*N*/       m_bOpenInDesignMode = _bOpenDesignMode;
/*N*/
/*N*/       if ( pObjShell )
/*N*/           pObjShell->SetModified( sal_True );
/*N*/   }
/*N*/   // no matter if we really did it or not - from now on, it does not count as defaulted anymore
/*N*/   m_pImpl->bOpenInDesignIsDefaulted = sal_False;
/*N*/ }

//------------------------------------------------------------------------
/*N*/ void FmFormModel::SetOpenInDesignMode( sal_Bool bOpenDesignMode )
/*N*/ {
/*N*/ #ifndef SVX_LIGHT
/*N*/   implSetOpenInDesignMode( bOpenDesignMode, sal_False );
/*N*/ #endif
/*N*/ }

#ifndef SVX_LIGHT
//------------------------------------------------------------------------
/*N*/ sal_Bool FmFormModel::OpenInDesignModeIsDefaulted( )
/*N*/ {
/*N*/   return m_pImpl->bOpenInDesignIsDefaulted;
/*N*/ }
#endif

//------------------------------------------------------------------------
/*N*/ void FmFormModel::SetAutoControlFocus( sal_Bool _bAutoControlFocus )
/*N*/ {
/*N*/ #ifndef SVX_LIGHT
/*N*/   if( _bAutoControlFocus != m_bAutoControlFocus )
/*N*/   {
/*?*/       m_bAutoControlFocus = _bAutoControlFocus;
/*?*/       pObjShell->SetModified( sal_True );
/*N*/   }
/*N*/ #endif
/*N*/ }

//------------------------------------------------------------------------
/*N*/ void FmFormModel::SetObjectShell( SfxObjectShell* pShell )
/*N*/ {
/*N*/   if (pShell == pObjShell)
/*N*/       return;
/*N*/
/*N*/
/*N*/   pObjShell = pShell;
/*N*/ }

//------------------------------------------------------------------------
/*N*/ XubString FmFormModel::GetUniquePageId()
/*N*/ {
/*N*/   XubString sReturn = m_pImpl->sNextPageId;
/*N*/
/*N*/   sal_Unicode aNextChar = m_pImpl->sNextPageId.GetChar(m_pImpl->sNextPageId.Len() - 1);
/*N*/   sal_Bool bNeedNewChar = sal_False;
/*N*/   switch (aNextChar)
/*N*/   {
/*N*/       case '9' : aNextChar = 'A'; break;
/*N*/       case 'Z' : aNextChar = 'a'; break;
/*N*/       case 'z' : aNextChar = '0'; bNeedNewChar = sal_True; break;
/*N*/       default: ++aNextChar; break;
/*N*/   }
/*N*/   m_pImpl->sNextPageId.SetChar(m_pImpl->sNextPageId.Len() - 1, aNextChar);
/*N*/   if (bNeedNewChar)
/*N*/       m_pImpl->sNextPageId += '0';
/*N*/
/*N*/   return sReturn;
/*N*/ }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
