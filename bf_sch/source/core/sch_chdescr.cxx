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
#include <bf_svtools/eitem.hxx>

#include "chdescr.hxx"

#include "float.h"
#include "schgroup.hxx"
#include "chtscene.hxx"
namespace binfilter {

/*N*/ ChartDataDescription::ChartDataDescription(long nCols, long nRows, SdrObjList *pList,
/*N*/                                          ChartModel* pModel, BOOL bEnable) :
/*N*/       mnRows(nRows),
/*N*/       mnCols(nCols),
/*N*/       mpModel(pModel),
/*N*/       mpDescrLists(NULL),
/*N*/       mpList(pList),
/*N*/       mpDescrArray(NULL),
/*N*/       mbEnable(bEnable),
/*N*/       mpDescrGroups(NULL)
/*N*/ {
/*N*/ }

/*N*/ void ChartDataDescription::Build( BOOL bRowDescr )
/*N*/ {
/*N*/   Dirty2D( bRowDescr );
/*N*/   if(mpList && mpDescrGroups)
/*N*/   {
/*?*/       for( long nRow=0; nRow<mnRows; nRow++ )
/*?*/       {
/*?*/           if(mpDescrGroups[nRow])
/*?*/               mpList->NbcInsertObject( mpDescrGroups[nRow] );
/*?*/       }
/*?*/   }
/*N*/ }

/*N*/ void ChartDataDescription::Build3D( ChartScene *pScene, Matrix4D* pMatrix )
/*N*/ {
/*N*/   if(pScene && mpDescrGroups)
/*N*/   {
/*?*/       for( long nRow=0; nRow<mnRows; nRow++ )
/*?*/       {
/*?*/           for( long nCol=0; nCol<mnCols; nCol++ )
/*?*/           {
/*?*/               long nIndex = nCol + nRow * mnCols;
/*?*/               if( mpDescrArray[nIndex].fValue != DBL_MIN )
/*?*/               {
/*?*/                   mpDescrArray[nIndex].pLabelObj->SetMarkProtect(TRUE);
/*?*/                   E3dLabelObj* pLabel=new E3dLabelObj(mpDescrArray[nIndex].aTextPos3D,
/*?*/                                                       mpDescrArray[nIndex].pLabelObj);
/*?*/
/*?*/                   CHART_TRACE3( "Descr::Build3D TextPos = (%ld, %ld, %ld)",
/*?*/                                 mpDescrArray[ nIndex ].aTextPos3D.X(),
/*?*/                                 mpDescrArray[ nIndex ].aTextPos3D.Y(),
/*?*/                                 mpDescrArray[ nIndex ].aTextPos3D.Z() );
/*?*/
/*?*/                   pLabel->SetMarkProtect(TRUE);
/*?*/                   pScene->Insert3DObj(pLabel);
/*?*/                   if(pMatrix)
/*?*/                       pLabel->NbcSetTransform(*pMatrix);
/*?*/               }
/*?*/           }
/*?*/       }
/*?*/   }
/*N*/ }

/*N*/ ChartDataDescription::~ChartDataDescription()
/*N*/ {
/*N*/   if(mpDescrLists)
/*?*/       delete[] mpDescrLists;
/*N*/   if(mpDescrGroups)
/*?*/       delete[] mpDescrGroups;
/*N*/   if(mpDescrArray)
/*?*/       delete[] mpDescrArray;
/*N*/ }


/*N*/ void ChartDataDescription::Dirty2D( BOOL /*bRowDescr*/ )
/*N*/ {
/*N*/
/*N*/   if(mpDescrLists)
/*N*/   {
/*?*/       for( long nRow = 0; nRow < mnRows; nRow ++ )
/*?*/       {
/*?*/           if( mpDescrLists[nRow] )
/*?*/           {
/*?*/               for( long nCol = 0; nCol < mnCols; nCol++ )
/*?*/               {
/*?*/                       long nIndex = nCol + nRow * mnCols;
/*?*/                       if (mpDescrArray[nIndex].fValue != DBL_MIN)
/*?*/                           if (mpDescrArray[nIndex].pLabelObj)
/*?*/                               mpDescrLists[nRow]->NbcInsertObject(mpDescrArray[nIndex].pLabelObj);
/*?*/               }
/*?*/           }
/*?*/       }
/*N*/   }
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
