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

#ifdef _MSC_VER
#pragma optimize("",off)
#endif
#ifdef _MSC_VER
#pragma optimize("q",off) // p-code off
#endif

#include <bf_sfx2/objsh.hxx>
#include <bf_svx/svditer.hxx>
#include <bf_svx/svdoole2.hxx>
#include <bf_svx/svdpage.hxx>
#include <bf_sch/schdll.hxx>
#include <bf_sch/memchrt.hxx>
#include <bf_sch/schdll0.hxx>

#include "document.hxx"
#include "drwlayer.hxx"
#include "chartarr.hxx"
#include "chartlis.hxx"
#include "refupdat.hxx"
namespace binfilter {

#ifndef SO2_DECL_SVINPLACEOBJECT_DEFINED
#define SO2_DECL_SVINPLACEOBJECT_DEFINED
SO2_DECL_REF(SvInPlaceObject)
#endif

// Charts aus altem Dokument updaten
void ScDocument::UpdateAllCharts(BOOL bDoUpdate)
{
    if (!pDrawLayer)
        return;

    USHORT nDataCount = pChartCollection->GetCount();
    if ( !nDataCount )
        return ;        // nothing to do

    USHORT nPos;

    for (USHORT nTab=0; nTab<=MAXTAB; nTab++)
    {
        if (pTab[nTab])
        {
            SdrPage* pPage = pDrawLayer->GetPage(nTab);
            DBG_ASSERT(pPage,"Page ?");

            ScRange aRange;
            SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
            SdrObject* pObject = aIter.Next();
            while (pObject)
            {
                if ( pObject->GetObjIdentifier() == OBJ_OLE2 )
                {
                    SvInPlaceObjectRef aIPObj = ((SdrOle2Obj*)pObject)->GetObjRef();
                    if (aIPObj.Is())
                    {
                        SvInfoObject* pInfoObj = pShell->Find( aIPObj );
                        String aIPName;

                        if ( pInfoObj )
                            aIPName = pInfoObj->GetObjName();

                        for (nPos=0; nPos<nDataCount; nPos++)
                        {
                            ScChartArray* pChartObj = (*pChartCollection)[nPos];
                            if (pChartObj->GetName() == aIPName)
                            {
                                if (bDoUpdate)
                                {
                                    SchMemChart* pMemChart = pChartObj->CreateMemChart();
                                    SchDLL::Update( aIPObj, pMemChart );
                                    delete pMemChart;
                                }
                                else        // nur Position uebernehmen
                                {
                                    SchMemChart* pChartData = SchDLL::GetChartData(aIPObj);
                                    if (pChartData)
                                    {
                                        pChartObj->SetExtraStrings(*pChartData);
                                    }
                                }
                                ScChartListener* pCL = new ScChartListener(
                                    aIPName, this, pChartObj->GetRangeList() );
                                pChartListenerCollection->Insert( pCL );
                                pCL->StartListeningTo();
                            }
                        }
                    }
                }
                pObject = aIter.Next();
            }
        }
    }

    pChartCollection->FreeAll();
}

void ScDocument::UpdateChartArea( const String& rChartName,
            const ScRangeListRef& rNewList, BOOL bColHeaders, BOOL bRowHeaders,
            BOOL bAdd, Window* pWindow )
{
    if (!pDrawLayer)
        return;

    for (USHORT nTab=0; nTab<=MAXTAB && pTab[nTab]; nTab++)
    {
        SdrPage* pPage = pDrawLayer->GetPage(nTab);
        DBG_ASSERT(pPage,"Page ?");

        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            if ( pObject->GetObjIdentifier() == OBJ_OLE2 &&
                    ((SdrOle2Obj*)pObject)->GetPersistName() == rChartName )
            {
                SvInPlaceObjectRef aIPObj = ((SdrOle2Obj*)pObject)->GetObjRef();
                if (aIPObj.Is())
                {
                    const SchMemChart* pChartData = SchDLL::GetChartData(aIPObj);
                    if ( pChartData )
                    {
                        ScChartArray aArray( this, *pChartData );
                        if ( bAdd )
                        {
                            // bei bAdd werden Header-Angaben ignoriert
                            aArray.AddToRangeList( rNewList );
                        }
                        else
                        {
                            aArray.SetRangeList( rNewList );
                            aArray.SetHeaders( bColHeaders, bRowHeaders );
                        }

                        SchMemChart* pMemChart = aArray.CreateMemChart();
                        ScChartArray::CopySettings( *pMemChart, *pChartData );

                        SchDLL::Update( aIPObj, pMemChart, pWindow );
                        delete pMemChart;

                        // Dies veranlaesst Chart zum sofortigen Update
                        aIPObj->SendViewChanged();
                        pObject->SendRepaintBroadcast();

                        return;            // nicht weitersuchen
                    }
                }
            }
            pObject = aIter.Next();
        }
    }
}

void ScDocument::UpdateChart( const String& rChartName, Window* pWindow )
{
    if (!pDrawLayer || bInDtorClear)
        return;

    for (USHORT nTab=0; nTab<=MAXTAB && pTab[nTab]; nTab++)
    {
        SdrPage* pPage = pDrawLayer->GetPage(nTab);
        DBG_ASSERT(pPage,"Page ?");

        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            if ( pObject->GetObjIdentifier() == OBJ_OLE2 &&
                    ((SdrOle2Obj*)pObject)->GetPersistName() == rChartName )
            {
                SvInPlaceObjectRef aIPObj = ((SdrOle2Obj*)pObject)->GetObjRef();
                if (aIPObj.Is())
                {
                    const SchMemChart* pChartData = SchDLL::GetChartData(aIPObj);
                    if ( pChartData )
                    {
                        ScChartArray aArray( this, *pChartData );

                        SchMemChart* pMemChart = aArray.CreateMemChart();
                        ScChartArray::CopySettings( *pMemChart, *pChartData );

                        BOOL bEnabled = ( ((pShell && pShell->IsReadOnly()) ||
                                            IsImportingXML()) &&
                                            aIPObj->IsEnableSetModified() );
                        if (bEnabled)
                            aIPObj->EnableSetModified(FALSE);

                        SchDLL::Update( aIPObj, pMemChart, pWindow );
                        delete pMemChart;

                        // Dies veranlaesst Chart zum sofortigen Update
                        aIPObj->SendViewChanged();
                        pObject->SendRepaintBroadcast();

                        if (bEnabled)
                            aIPObj->EnableSetModified(TRUE);

                        return;            // nicht weitersuchen
                    }
                }
            }
            pObject = aIter.Next();
        }
    }
}

void ScDocument::UpdateChartRef( UpdateRefMode eUpdateRefMode,
                                    USHORT nCol1, USHORT nRow1, USHORT nTab1,
                                    USHORT nCol2, USHORT nRow2, USHORT nTab2,
                                    short nDx, short nDy, short nDz )
{
    if (!pDrawLayer)
        return;

    USHORT nChartCount = pChartListenerCollection->GetCount();
    for ( USHORT nIndex = 0; nIndex < nChartCount; nIndex++ )
    {
        ScChartListener* pChartListener =
            (ScChartListener*) (pChartListenerCollection->At(nIndex));
        ScRangeListRef aRLR( pChartListener->GetRangeList() );
        ScRangeListRef aNewRLR( new ScRangeList );
        BOOL bDataChanged = FALSE;
        for ( ScRangePtr pR = aRLR->First(); pR; pR = aRLR->Next() )
        {
            USHORT theCol1 = pR->aStart.Col();
            USHORT theRow1 = pR->aStart.Row();
            USHORT theTab1 = pR->aStart.Tab();
            USHORT theCol2 = pR->aEnd.Col();
            USHORT theRow2 = pR->aEnd.Row();
            USHORT theTab2 = pR->aEnd.Tab();
            ScRefUpdateRes eRes = ScRefUpdate::Update(
                this, eUpdateRefMode,
                nCol1,nRow1,nTab1, nCol2,nRow2,nTab2,
                nDx,nDy,nDz,
                theCol1,theRow1,theTab1,
                theCol2,theRow2,theTab2 );
            if ( eRes != UR_NOTHING )
            {
                aNewRLR->Append( ScRange(
                    theCol1, theRow1, theTab1,
                    theCol2, theRow2, theTab2 ));
                if ( eUpdateRefMode == URM_INSDEL
                    && !bDataChanged
                    && (eRes == UR_INVALID ||
                        ((pR->aEnd.Col() - pR->aStart.Col()
                        != theCol2 - theCol1)
                    || (pR->aEnd.Row() - pR->aStart.Row()
                        != theRow2 - theRow1)
                    || (pR->aEnd.Tab() - pR->aStart.Tab()
                        != theTab2 - theTab1))) )
                {
                    bDataChanged = TRUE;
                }
            }
            else
                aNewRLR->Append( *pR );
        }
    }
}

BOOL ScDocument::HasData( USHORT nCol, USHORT nRow, USHORT nTab )
{
    if (pTab[nTab])
        return pTab[nTab]->HasData( nCol, nRow );
    else
        return FALSE;
}

BOOL lcl_StringInCollection( const StrCollection* pColl, const String& rStr )
{
    if ( !pColl )
        return FALSE;

    StrData aData( rStr );
    USHORT nDummy;
    return pColl->Search( &aData, nDummy );
}

void ScDocument::UpdateChartListenerCollection()
{
    bChartListenerCollectionNeedsUpdate = FALSE;
    if (!pDrawLayer)
        return;
    else
    {
        ScRange aRange;
        // Range fuer Suche unwichtig
        ScChartListener aCLSearcher( EMPTY_STRING, this, aRange );
        for (USHORT nTab=0; nTab<=MAXTAB; nTab++)
        {
            if (pTab[nTab])
            {
                SdrPage* pPage = pDrawLayer->GetPage(nTab);
                DBG_ASSERT(pPage,"Page ?");

                SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
                SdrObject* pObject = aIter.Next();
                while (pObject)
                {
                    if ( pObject->GetObjIdentifier() == OBJ_OLE2 )
                    {
                        String aObjName = ((SdrOle2Obj*)pObject)->GetPersistName();
                        aCLSearcher.SetString( aObjName );
                        USHORT nIndex;
                        if ( pChartListenerCollection->Search( &aCLSearcher, nIndex ) )
                        {
                            ((ScChartListener*) (pChartListenerCollection->
                                At( nIndex )))->SetUsed( TRUE );
                        }
                        else if ( lcl_StringInCollection( pOtherObjects, aObjName ) )
                        {
                            // non-chart OLE object -> don't touch
                        }
                        else
                        {
                            //    SchDLL::GetChartData always loads the chart dll,
                            //    so SchModuleDummy::HasID must be tested before
                            BOOL bIsChart = FALSE;
                            USHORT nId;

                            //  Ask the SvPersist for the InfoObject to find out
                            //  whether it is a Chart. The old way with GetObjRef
                            //  loads the object which takes too much unnecessary
                            //  time
                            SvInfoObject* pInfoObj = pShell->Find(aObjName);
                            DBG_ASSERT(pInfoObj, "Why isn't here a SvInfoObject?");
                            if ( pInfoObj &&
                                ((nId = SchModuleDummy::HasID(pInfoObj->GetClassName()) ) != 0) )
                            {
                                SvInPlaceObjectRef aIPObj = ((SdrOle2Obj*)pObject)->GetObjRef();
                                DBG_ASSERT(aIPObj.Is(), "no SvInPlaceObject given");
                                if (aIPObj.Is())
                                {
                                    BOOL bSO6 = (nId >= SOFFICE_FILEFORMAT_60);
                                    SchMemChart* pChartData = SchDLL::GetChartData(aIPObj);
                                    if ( pChartData &&
                                        ((!bSO6 && pChartData->SomeData1().Len()) ||
                                        (bSO6 && pChartData->GetChartRange().maRanges.size())) )
                                    {
                                        if ( PastingDrawFromOtherDoc() )
                                        {
                                            pChartData->SomeData1().Erase();
                                            pChartData->SomeData2().Erase();
                                            pChartData->SomeData3().Erase();
                                            pChartData->SomeData4().Erase();
                                            SchChartRange aChartRange;
                                            pChartData->SetChartRange( aChartRange );
                                            pChartData->SetReadOnly( FALSE );
                                            SchDLL::Update( aIPObj, pChartData );
                                        }
                                        else
                                        {
                                            bIsChart = TRUE;

                                            ScChartArray aArray( this, *pChartData );
                                            ScChartListener* pCL = new ScChartListener(
                                                aObjName,
                                                this, aArray.GetRangeList() );
                                            pChartListenerCollection->Insert( pCL );
                                            pCL->StartListeningTo();
                                            pCL->SetUsed( TRUE );

                                            //  Set ReadOnly flag at MemChart, so Chart knows
                                            //  about the external data in a freshly loaded document.
                                            //  #73642# only if the chart really has external data
                                            if ( aArray.IsValid() )
                                            {
                                                pChartData->SetReadOnly( TRUE );
                                                if ( !bSO6 )
                                                {
                                                    String aOldData3 = pChartData->SomeData3();
                                                    aArray.SetExtraStrings( *pChartData );
                                                }
                                            }
                                            BOOL bEnabled = aIPObj->IsEnableSetModified();
                                            if (bEnabled)
                                                aIPObj->EnableSetModified(FALSE);
                                            pChartData->SetNumberFormatter( GetFormatTable() );
                                            SchDLL::Update( aIPObj, pChartData );
                                            //! pChartData got deleted, don't use it anymore
                                            if (bEnabled)
                                                aIPObj->EnableSetModified(TRUE);
                                        }
                                    }
                                }
                            }
                            if (!bIsChart)
                            {
                                //    put into list of other ole objects, so the object doesn't have to
                                //    be swapped in the next time UpdateChartListenerCollection is called
                                //!    remove names when objects are no longer there?
                                //    (object names aren't used again before reloading the document)
                                if (!pOtherObjects)
                                    pOtherObjects = new StrCollection;
                                pOtherObjects->Insert( new StrData( aObjName ) );
                            }
                        }
                    }
                    pObject = aIter.Next();
                }
            }
        }
        // alle nicht auf SetUsed gesetzten loeschen
        pChartListenerCollection->FreeUnused();
    }
}

void ScDocument::AddOLEObjectToCollection(const String& rName)
{
    if (!pOtherObjects)
        pOtherObjects = new StrCollection;
    pOtherObjects->Insert( new StrData( rName ) );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
