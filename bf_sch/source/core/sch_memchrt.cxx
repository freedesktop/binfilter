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

#ifndef _ZFORLIST_HXX //autogen
#ifndef _ZFORLIST_DECLARE_TABLE
#define _ZFORLIST_DECLARE_TABLE
#endif
#include <bf_svtools/zforlist.hxx>
#endif

#include <tools/tenccvt.hxx>

#include <float.h>
#include <math.h>
#include "schiocmp.hxx"
#include "memchrt.hxx"

#include "schresid.hxx"
#include "glob.hrc"

#include <functional>
#include <algorithm>

namespace binfilter {

// ========================================
// Helper objects
// ========================================

/** unary function that escapes backslashes and single quotes in a sal_Unicode
    array (which you can get from an OUString with getStr()) and puts the result
    into the OUStringBuffer given in the CTOR
 */
/*N*/  class lcl_Escape : public ::std::unary_function< sal_Unicode, void >
/*N*/  {
/*N*/  public:
/*N*/      lcl_Escape( ::rtl::OUStringBuffer & aResultBuffer ) : m_aResultBuffer( aResultBuffer ) {}
/*N*/      void operator() ( sal_Unicode aChar )
/*N*/      {
/*N*/          static const sal_Unicode m_aQuote( '\'' );
/*N*/          static const sal_Unicode m_aBackslash( '\\' );
/*N*/
/*N*/          if( aChar == m_aQuote ||
/*N*/              aChar == m_aBackslash )
/*N*/              m_aResultBuffer.append( m_aBackslash );
/*N*/          m_aResultBuffer.append( aChar );
/*N*/      }
/*N*/
/*N*/  private:
/*N*/      ::rtl::OUStringBuffer & m_aResultBuffer;
/*N*/  };

/** unary function that removes backslash escapes in a sal_Unicode array (which
    you can get from an OUString with getStr()) and puts the result into the
    OUStringBuffer given in the CTOR
 */
/*N*/  class lcl_UnEscape : public ::std::unary_function< sal_Unicode, void >
/*N*/  {
/*N*/  public:
/*N*/      lcl_UnEscape( ::rtl::OUStringBuffer & aResultBuffer ) : m_aResultBuffer( aResultBuffer ) {}
/*N*/      void operator() ( sal_Unicode aChar )
/*N*/      {
/*N*/          static const sal_Unicode m_aBackslash( '\\' );
/*N*/
/*N*/          if( aChar != m_aBackslash )
/*N*/              m_aResultBuffer.append( aChar );
/*N*/      }
/*N*/
/*N*/  private:
/*N*/      ::rtl::OUStringBuffer & m_aResultBuffer;
/*N*/  };



#define MIN(a,b) ( ((a)<(b))? (a) : (b) )

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

/*N*/ SchMemChart::SchMemChart(ChartDataId nMyID) :
/*N*/   nLastSelInfoReturn(0),
/*N*/   nRefCount (0),
/*N*/   mpColNameBuffer(NULL),
/*N*/   mpRowNameBuffer(NULL),
/*N*/   nTranslated(TRANS_NONE),
/*N*/   nRowCnt (0),
/*N*/   nColCnt (0),
/*N*/   eDataType(NUMBERFORMAT_NUMBER),
/*N*/   pData (0),
/*N*/   pColText (0),
/*N*/   pRowText (0),
/*N*/   myID (nMyID),
/*N*/   mpNumFormatter(NULL),
/*N*/   pRowNumFmtId(NULL),
/*N*/   pColNumFmtId(NULL),
/*N*/   pRowTable(NULL),
/*N*/   pColTable(NULL),
/*N*/   bReadOnly(FALSE)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

/*N*/ SchMemChart::SchMemChart(short nCols, short nRows) :
/*N*/   nLastSelInfoReturn(0),
/*N*/   nRefCount (0),
/*N*/   mpColNameBuffer(NULL),
/*N*/   mpRowNameBuffer(NULL),
/*N*/   nTranslated(TRANS_NONE),
/*N*/   eDataType(NUMBERFORMAT_NUMBER),
/*N*/   pData (0),
/*N*/   pColText (0),
/*N*/   pRowText (0),
/*N*/   myID (CHDATAID_MEMCHART_PLUS),
/*N*/   mpNumFormatter(NULL),
/*N*/   pRowNumFmtId(NULL),
/*N*/   pColNumFmtId(NULL),
/*N*/   pRowTable(NULL),
/*N*/   pColTable(NULL),
/*N*/   bReadOnly(FALSE)
/*N*/ {
/*N*/   nRowCnt = nRows;
/*N*/   nColCnt = nCols;
        pData = ArrayHelper< double >::create_short_size( nColCnt, nRowCnt );

        pRowNumFmtId = ArrayHelper< sal_Int32 >::create_short_size( nRowCnt );
        pColNumFmtId = ArrayHelper< sal_Int32 >::create_short_size( nColCnt );
/*N*/   InitNumFmt();

        pRowTable = ArrayHelper< sal_Int32 >::create_short_size( nRowCnt );
        pColTable = ArrayHelper< sal_Int32 >::create_short_size( nColCnt );
/*N*/   ResetTranslation(pRowTable,nRowCnt);
/*N*/   ResetTranslation(pColTable,nColCnt);
/*N*/
/*N*/   if (pData)
/*N*/   {
/*N*/       double *pFill = pData;
/*N*/
/*N*/       for (short i = 0; i < nColCnt; i++)
/*N*/           for (short j = 0; j < nRowCnt; j++)
/*N*/               *(pFill ++) = 0.0;
/*N*/   }
/*N*/
        pColText = ArrayHelper< String >::create_short_size( nColCnt );
        pRowText = ArrayHelper< String >::create_short_size( nRowCnt );

        if ( !pData || !pRowNumFmtId || !pColNumFmtId || !pRowTable || !pColTable || !pColText || !pRowText )
        {
            delete[] pData;
            pData = 0;
            delete[] pRowNumFmtId;
            pRowNumFmtId = 0;
            delete[] pColNumFmtId;
            pColNumFmtId = 0;
            delete[] pRowTable;
            pRowTable = 0;
            delete[] pColTable;
            pColTable = 0;
            delete[] pColText;
            pColText = 0;
            delete[] pRowText;
            pRowText = 0;

            nRowCnt = 0;
            nColCnt = 0;
        }
/*N*/ }
/*************************************************************************
|*
|* Kopiere alles ausser den numerischen Daten (d.h. Texte!)
|*
\************************************************************************/
/*N*/ void SchMemChart::SetNonNumericData(const SchMemChart &rMemChart)
/*N*/ {
/*N*/   aMainTitle  = rMemChart.aMainTitle;
/*N*/   aSubTitle   = rMemChart.aSubTitle;
/*N*/   aXAxisTitle = rMemChart.aXAxisTitle;
/*N*/   aYAxisTitle = rMemChart.aYAxisTitle;
/*N*/   aZAxisTitle = rMemChart.aZAxisTitle;
/*N*/   eDataType   = rMemChart.eDataType;
/*N*/   aSomeData1  = ((SchMemChart&) rMemChart).SomeData1 ();
/*N*/   aSomeData2  = ((SchMemChart&) rMemChart).SomeData2 ();
/*N*/   aSomeData3  = ((SchMemChart&) rMemChart).SomeData3 ();
/*N*/   aSomeData4  = ((SchMemChart&) rMemChart).SomeData4 ();
/*N*/
/*N*/   long nCols=MIN(nColCnt,rMemChart.nColCnt);
/*N*/   long nRows=MIN(nRowCnt,rMemChart.nRowCnt);
/*N*/
/*N*/   short i;
        if ( pColText )
        {
            for ( i = 0; i < nCols; i++ )
            {
                pColText[i] = rMemChart.pColText[i];
            }
        }
        if ( pRowText )
        {
            for ( i = 0; i < nRows; i++ )
            {
                pRowText[i] = rMemChart.pRowText[i];
            }
        }
/*N*/
/*N*/     // copy chart range
/*N*/     SetChartRange( rMemChart.GetChartRange());
/*N*/ }
/*************************************************************************
|*
|* Kopier-Konstruktor
|*
\************************************************************************/

/*N*/ SchMemChart::SchMemChart(const SchMemChart& rMemChart) :
/*N*/   nRefCount (0),
/*N*/   mpColNameBuffer(NULL),
/*N*/   mpRowNameBuffer(NULL),
/*N*/   nTranslated(TRANS_NONE),
/*N*/   myID (CHDATAID_MEMCHART_PLUS),
/*N*/   mpNumFormatter(NULL),
/*N*/   pRowNumFmtId(NULL),
/*N*/   pColNumFmtId(NULL),
/*N*/   pRowTable(NULL),
/*N*/   pColTable(NULL)
/*N*/ {
/*N*/   nColCnt     = rMemChart.nColCnt;
/*N*/   nRowCnt     = rMemChart.nRowCnt;
/*N*/   aMainTitle  = rMemChart.aMainTitle;
/*N*/   aSubTitle   = rMemChart.aSubTitle;
/*N*/   aXAxisTitle = rMemChart.aXAxisTitle;
/*N*/   aYAxisTitle = rMemChart.aYAxisTitle;
/*N*/   aZAxisTitle = rMemChart.aZAxisTitle;
/*N*/   eDataType   = rMemChart.eDataType;
/*N*/   aSomeData1  = ((SchMemChart&) rMemChart).SomeData1 ();
/*N*/   aSomeData2  = ((SchMemChart&) rMemChart).SomeData2 ();
/*N*/   aSomeData3  = ((SchMemChart&) rMemChart).SomeData3 ();
/*N*/   aSomeData4  = ((SchMemChart&) rMemChart).SomeData4 ();
        pData = ArrayHelper< double >::create_short_size( nColCnt, nRowCnt );

        pRowNumFmtId = ArrayHelper< sal_Int32 >::create_short_size( nRowCnt );
        pColNumFmtId = ArrayHelper< sal_Int32 >::create_short_size( nColCnt );
        pRowTable = ArrayHelper< sal_Int32 >::create_short_size( nRowCnt );
        pColTable = ArrayHelper< sal_Int32 >::create_short_size( nColCnt );
/*N*/
/*N*/     aAppLink = rMemChart.aAppLink;
/*N*/   nLastSelInfoReturn = rMemChart.nLastSelInfoReturn;
/*N*/
/*N*/   nTranslated = rMemChart.nTranslated;
        if ( pColTable && pColNumFmtId )
        {
            for (long i = 0; i < nColCnt; i++)
            {
                pColTable[i]	= rMemChart.pColTable[i];
                pColNumFmtId[i]	= rMemChart.pColNumFmtId[i];
            }
        }
        if ( pRowTable && pRowNumFmtId )
        {
            for (long i = 0; i < nRowCnt; i++)
            {
                pRowTable[i]	= rMemChart.pRowTable[i];
                pRowNumFmtId[i]	= rMemChart.pRowNumFmtId[i];
            }
        }
/*N*/   mpNumFormatter=rMemChart.mpNumFormatter;
/*N*/
/*N*/   if (pData)
/*N*/   {
/*N*/       double *pDest   = pData;
/*N*/       double *pSource = rMemChart.pData;
/*N*/
/*N*/       for (short i = 0; i < nColCnt; i++)
/*N*/           for (short j = 0; j < nRowCnt; j++)
/*N*/               *(pDest ++) = *(pSource ++);
/*N*/   }
/*N*/
        pColText = ArrayHelper< String >::create_short_size( nColCnt );
        if ( pColText )
        {
            for (long i = 0; i < nColCnt; i++)
            {
                pColText[i] = rMemChart.pColText[i];
            }
        }

        pRowText = ArrayHelper< String >::create_short_size( nRowCnt );
        if ( pRowText )
        {
            for (long i = 0; i < nRowCnt; i++)
            {
                pRowText[i] = rMemChart.pRowText[i];
            }
        }
/*N*/
/*N*/   bReadOnly = rMemChart.bReadOnly;            // bm #69410#
/*N*/
/*N*/   // copy address members
/*N*/   maCategoriesRangeAddress = rMemChart.maCategoriesRangeAddress;
/*N*/   maSeriesAddresses = rMemChart.maSeriesAddresses;
/*N*/
/*N*/     // copy chart range
/*N*/     SetChartRange( rMemChart.GetChartRange());

        if ( !pData || !pRowNumFmtId || !pColNumFmtId || !pRowTable || !pColTable || !pColText || !pRowText )
        {
            delete[] pData;
            pData = 0;
            delete[] pRowNumFmtId;
            pRowNumFmtId = 0;
            delete[] pColNumFmtId;
            pColNumFmtId = 0;
            delete[] pRowTable;
            pRowTable = 0;
            delete[] pColTable;
            pColTable = 0;
            delete[] pColText;
            pColText = 0;
            delete[] pRowText;
            pRowText = 0;

            nRowCnt = 0;
            nColCnt = 0;
        }
/*N*/ }

//�berpr�ft, ob die Umordnung/Translation OK ist, Fehlerfall, wenn :
// a) Spaltenumordnung aber Reihen vertauscht (FALSE,TRANS_ERROR)
// b) Reihenumordnung aber Spalten vertauscht (FALSE,TRANS_ERROR)
// c) keine Umordnung, Reihen oder Spalten vertauscht (FALSE,TRANS_ERROR)
// d) Umordnungsflag gesetzt (auf TRANS_ROW,TRANS_COL oder TRANS_ERROR) aber
//      keine Umordnung (mehr) vorhanden (FALSE,TRANS_NONE)
// sonst wird TRUE zur�ckgegeben

#if OSL_DEBUG_LEVEL > 0
/*N*/ BOOL SchMemChart::VerifyTranslation()
/*N*/ {
/*N*/   //Fehler ?
/*N*/ 	if(nTranslated!=TRANS_COL && pColTable)
/*N*/   {
/*N*/       for(long nCol=0;nCol<nColCnt;nCol++)
/*N*/       {
/*N*/           if(pColTable[nCol]!=nCol)
/*N*/           {
/*?*/               //nTranslated=TRANS_ERROR;
/*?*/               if(nTranslated==TRANS_NONE)
/*?*/               {
/*?*/                   OSL_FAIL("SchMemChart::Correcting Translationmode");
/*?*/                   nTranslated=TRANS_COL;
/*?*/               }
/*?*/               else
/*?*/               {
/*?*/                   OSL_FAIL("fatal error in SchMemChart-translation");
/*?*/                   return FALSE;
/*?*/               }
/*N*/           }
/*N*/       }
/*N*/   }
/*N*/ 	if(nTranslated!=TRANS_ROW && pRowTable)
/*N*/   {
/*N*/       for(long nRow=0;nRow<nRowCnt;nRow++)
/*N*/       {
/*N*/           if(pRowTable[nRow]!=nRow)
/*N*/           {
/*?*/               //nTranslated=TRANS_ERROR;
/*?*/               if(nTranslated==TRANS_NONE)
/*?*/               {
/*?*/                   OSL_FAIL("SchMemChart::Correcting Translationmode");
/*?*/                   nTranslated=TRANS_ROW;
/*?*/               }
/*?*/               else
/*?*/               {
/*?*/                   OSL_FAIL("fatal error in SchMemChart-translation");
/*?*/                   return FALSE;
/*?*/               }
/*N*/           }
/*N*/       }
/*N*/   }
/*N*/
/*N*/   // Sortierung besteht noch ?
/*N*/ 	if(nTranslated==TRANS_ROW && pRowTable)
/*N*/   {
/*?*/       for(long nRow=0;nRow<nRowCnt;nRow++)
/*?*/       {
/*?*/           if(pRowTable[nRow]!=nRow)
/*?*/               return TRUE;
/*?*/       }
/*N*/   }
/*N*/ 	if(nTranslated==TRANS_COL && pColTable)
/*N*/   {
/*?*/       for(long nCol=0;nCol<nColCnt;nCol++)
/*?*/       {
/*?*/           if(pColTable[nCol]!=nCol)
/*?*/               return TRUE;
/*?*/       }
/*N*/   }
/*N*/
/*N*/   if(nTranslated!=TRANS_NONE)
/*N*/   {
/*?*/       nTranslated=TRANS_NONE;
/*?*/       return FALSE;
/*N*/   }
/*N*/   return TRUE;
/*N*/
/*N*/ }
#endif

/*N*/ double SchMemChart::GetTransData(long nCol,long nRow)
/*N*/ {
/*N*/   DBG_ASSERT(VerifyTranslation(), "Translation table corrupted in MemChart");
        if ( !pColTable || !pRowTable || !IsValidColumn( nCol ) || !IsValidRow( nRow ) )
        {
            return DBL_MIN;
        }
/*N*/   return GetData( (short)pColTable[nCol], (short)pRowTable[nRow]);
/*N*/ }
/*N*/ double SchMemChart::GetTransDataInPercent(long nCol, long nRow, BOOL bRowData) const
/*N*/ {
/*N*/   DBG_ASSERT(((SchMemChart*)this)->VerifyTranslation(), "Translation table corrupted in MemChart");
        if ( !pColTable || !pRowTable || !IsValidColumn( nCol ) || !IsValidRow( nRow ) )
        {
            return DBL_MIN;
        }
/*N*/   return GetDataInPercent( (short)pColTable[nCol], (short)pRowTable[nRow], bRowData);
/*N*/ }
/*N*/ const String& SchMemChart::GetTransColText(long nCol) const
/*N*/ {
/*N*/   DBG_ASSERT(((SchMemChart*)this)->VerifyTranslation(), "Translation table corrupted in MemChart");
        if ( !pColTable || !IsValidColumn( nCol ) )
        {
            return String::EmptyString();
        }
/*N*/   return GetColText( (short)pColTable[nCol] );
/*N*/ }
/*N*/ const String& SchMemChart::GetTransRowText(long nRow) const
/*N*/ {
/*N*/   DBG_ASSERT(((SchMemChart*)this)->VerifyTranslation(), "Translation table corrupted in MemChart");
        if ( !pRowTable || !IsValidRow( nRow ) )
        {
            return String::EmptyString();
        }
/*N*/   return GetRowText( (short)pRowTable[nRow] );
/*N*/ }
/*N*/ long SchMemChart::GetTransNumFormatIdRow(const long nRow) const
/*N*/ {
/*N*/   DBG_ASSERT(((SchMemChart*)this)->VerifyTranslation(), "Translation table corrupted in MemChart");
        if ( pRowNumFmtId && IsValidRow( nRow ) )
        {
            if ( nTranslated == TRANS_ROW )
            {
                if ( pRowTable )
                {
                    long nTransRow = pRowTable[ nRow ];
                    if ( IsValidRow( nTransRow ) )
                    {
                        return pRowNumFmtId[ nTransRow ];
                    }
                }
            }
            else
            {
                return pRowNumFmtId[ nRow ];
            }
        }
        return NUMBERFORMAT_UNDEFINED;
/*N*/ }
/*N*/ long SchMemChart::GetTransNumFormatIdCol(const long nCol) const
/*N*/ {
/*N*/   DBG_ASSERT(((SchMemChart*)this)->VerifyTranslation(), "Translation table corrupted in MemChart");
        if ( pColNumFmtId && IsValidColumn( nCol ) )
        {
            if ( nTranslated == TRANS_COL )
            {
                if ( pColTable )
                {
                    long nTransCol = pColTable[ nCol ];
                    if ( IsValidColumn( nTransCol ) )
                    {
                        return pColNumFmtId[ nTransCol ];
                    }
                }
            }
            else
            {
                return pColNumFmtId[ nCol ];
            }
        }
        return NUMBERFORMAT_UNDEFINED;
/*N*/ }


/*************************************************************************
|*
|* Wie GetData, aber in Prozentwerten
|* Optimierungsvorschlag: fTotal fuer jede Zeile und Spalte buffern,
|*                        dazu muessen alle Schnittstellen bekannt sein,
|*                        die Daten am MemChart veraendern koennen.
|*
\************************************************************************/
/*N*/ double SchMemChart::GetDataInPercent(const short nCol , const short nRow, const BOOL bRowData) const
/*N*/ {
/*N*/    double fTotal=0.0,fTemp,fData;
/*N*/    short i;
/*N*/
/*N*/    fData = GetData(nCol,nRow);
/*N*/
/*N*/    if(bRowData)
/*N*/    {
/*N*/      for(i=0;i<nRowCnt;i++)
/*N*/      {
/*N*/          fTemp=GetData(nCol,i);
/*N*/
/*N*/          if(fTemp != DBL_MIN)   fTotal += fabs(fTemp);
/*N*/      }
/*N*/    }
/*N*/    else
/*N*/    {
/*N*/      for(i=0;i<nColCnt;i++)
/*N*/      {
/*N*/          fTemp=GetData(i,nRow);
/*N*/
/*N*/          if(fTemp != DBL_MIN)   fTotal += fabs(fTemp);
/*N*/      }
/*N*/
/*N*/    }
/*N*/    return fTotal ? ( (fabs(fData) / fTotal) * 100.0 ) : DBL_MIN;
/*N*/ }

/*************************************************************************
|*
|* Inserter fuer SvStream zum Speichern
|*
\************************************************************************/

/*N*/ SvStream& operator << (SvStream& rOut, const SchMemChart& rMemChart)
/*N*/ {
/*N*/   CharSet aSysCharSet = static_cast< CharSet >( ::GetSOStoreTextEncoding( osl_getThreadTextEncoding() ) );
/*N*/   rOut.SetStreamCharSet( aSysCharSet );
/*N*/
/*N*/   //Version 1: Abspeichern der pRow,pColTable (long-array)
/*N*/   SchIOCompat aIO(rOut, STREAM_WRITE, 2);
/*N*/
/*N*/   rOut << (INT16)rMemChart.nColCnt;
/*N*/   rOut << (INT16)rMemChart.nRowCnt;
/*N*/
/*N*/   double *pOut = rMemChart.pData;
/*N*/
/*N*/   short i;
        if ( pOut )
        {
            for ( i = 0; i < rMemChart.nColCnt; i++ )
            {
                for ( short j = 0; j < rMemChart.nRowCnt; j++ )
                {
                    rOut << *(pOut ++);
                }
            }
        }
/*N*/
/*N*/   rOut << (INT16)aSysCharSet;
/*N*/   rOut.WriteUniOrByteString( rMemChart.aMainTitle, rOut.GetStreamCharSet() );
/*N*/   rOut.WriteUniOrByteString( rMemChart.aSubTitle, rOut.GetStreamCharSet() );
/*N*/   rOut.WriteUniOrByteString( rMemChart.aXAxisTitle, rOut.GetStreamCharSet() );
/*N*/   rOut.WriteUniOrByteString( rMemChart.aYAxisTitle, rOut.GetStreamCharSet() );
/*N*/   rOut.WriteUniOrByteString( rMemChart.aZAxisTitle, rOut.GetStreamCharSet() );
/*N*/
        if ( rMemChart.pColText )
        {
            for ( i = 0; i < rMemChart.nColCnt; i++ )
            {
                rOut.WriteUniOrByteString( rMemChart.pColText[ i ], rOut.GetStreamCharSet() );
            }
        }

        if ( rMemChart.pRowText )
        {
            for ( i = 0; i < rMemChart.nRowCnt; i++ )
            {
                rOut.WriteUniOrByteString( rMemChart.pRowText[ i ], rOut.GetStreamCharSet() );
            }
        }
/*N*/
/*N*/   rOut << (INT16)rMemChart.eDataType;
/*N*/
/*N*/   //IOVersion = 1
/*N*/   long nIndex;
        if ( rMemChart.pColTable )
        {
            for ( nIndex = 0; nIndex < rMemChart.nColCnt; nIndex++ )
            {
                rOut << rMemChart.pColTable[nIndex];
            }
        }

        if ( rMemChart.pRowTable )
        {
            for ( nIndex = 0; nIndex < rMemChart.nRowCnt; nIndex++ )
            {
                rOut << rMemChart.pRowTable[nIndex];
            }
        }
/*N*/
/*N*/   //IOVersion = 2
/*N*/   rOut << rMemChart.nTranslated;
/*N*/
/*N*/
/*N*/   return rOut;
/*N*/ }

/*************************************************************************
|*
|* Extractor fuer SvStream zum Laden
|*
\************************************************************************/

/*N*/ SvStream& operator >> (SvStream& rIn, SchMemChart& rMemChart)
/*N*/ {
/*N*/   INT16 nInt16;
/*N*/
/*N*/   SchIOCompat aIO(rIn, STREAM_READ);
/*N*/
        short nCols=0;
        short nRows=0;
/*N*/ 	rIn >> nInt16; nCols = static_cast<short>(nInt16);
/*N*/ 	rIn >> nInt16; nRows = static_cast<short>(nInt16);
/*N*/
        double* pTmpData = ArrayHelper< double >::create_short_size( nCols, nRows );
        if ( !pTmpData )
        {
            rIn.SetError( ERRCODE_IO_GENERAL );
            return rIn;
        }

        rMemChart.nColCnt = nCols;
        rMemChart.nRowCnt = nRows;

        delete[] rMemChart.pData;
        rMemChart.pData = pTmpData;

        double *pIn = rMemChart.pData;
        if ( pIn )
        {
            for (short i = 0; i < rMemChart.nColCnt; i++)
            {
                for ( short j = 0; j < rMemChart.nRowCnt; j++ )
                {
                    rIn >> *(pIn ++);
                }
            }
        }
/*N*/
/*N*/   INT16 nCharSet;
/*N*/   rIn >> nCharSet;
/*N*/
/*N*/   rtl_TextEncoding aCharSet = ::GetSOLoadTextEncoding( static_cast< rtl_TextEncoding >( nCharSet ) );
/*N*/     rIn.SetStreamCharSet( aCharSet );
/*N*/
/*N*/   rMemChart.aMainTitle = rIn.ReadUniOrByteString( rIn.GetStreamCharSet() );
/*N*/   rMemChart.aSubTitle = rIn.ReadUniOrByteString( rIn.GetStreamCharSet() );
/*N*/   rMemChart.aXAxisTitle = rIn.ReadUniOrByteString( rIn.GetStreamCharSet() );
/*N*/   rMemChart.aYAxisTitle = rIn.ReadUniOrByteString( rIn.GetStreamCharSet() );
/*N*/   rMemChart.aZAxisTitle = rIn.ReadUniOrByteString( rIn.GetStreamCharSet() );
/*N*/
        delete[] rMemChart.pColText;
        rMemChart.pColText = ArrayHelper< String >::create_short_size( rMemChart.nColCnt );
        if ( !rMemChart.pColText )
        {
            rIn.SetError( ERRCODE_IO_GENERAL );
            return rIn;
        }
        for (short i = 0; i < rMemChart.nColCnt; i++ )
        {
            rMemChart.pColText[ i ] = rIn.ReadUniOrByteString( rIn.GetStreamCharSet() );
        }

        delete[] rMemChart.pRowText;
        rMemChart.pRowText = ArrayHelper< String >::create_short_size( rMemChart.nRowCnt );
        if ( !rMemChart.pRowText )
        {
            rIn.SetError( ERRCODE_IO_GENERAL );
            return rIn;
        }
        for (short i = 0; i < rMemChart.nRowCnt; i++ )
        {
            rMemChart.pRowText[ i ] = rIn.ReadUniOrByteString( rIn.GetStreamCharSet() );
        }
/*N*/
/*N*/   rIn >> nInt16; rMemChart.eDataType = (short)nInt16;
/*N*/
        delete[] rMemChart.pRowNumFmtId;
        rMemChart.pRowNumFmtId = ArrayHelper< sal_Int32 >::create_short_size( rMemChart.nRowCnt );
        if ( !rMemChart.pRowNumFmtId )
        {
            rIn.SetError( ERRCODE_IO_GENERAL );
            return rIn;
        }

        delete[] rMemChart.pColNumFmtId;
        rMemChart.pColNumFmtId = ArrayHelper< sal_Int32 >::create_short_size( rMemChart.nColCnt );
        if ( !rMemChart.pColNumFmtId )
        {
            rIn.SetError( ERRCODE_IO_GENERAL );
            return rIn;
        }

        delete[] rMemChart.pRowTable;
        rMemChart.pRowTable = ArrayHelper< sal_Int32 >::create_short_size( rMemChart.nRowCnt );
        if ( !rMemChart.pRowTable )
        {
            rIn.SetError( ERRCODE_IO_GENERAL );
            return rIn;
        }

        delete[] rMemChart.pColTable;
        rMemChart.pColTable = ArrayHelper< sal_Int32 >::create_short_size( rMemChart.nColCnt );
        if ( !rMemChart.pColTable )
        {
            rIn.SetError( ERRCODE_IO_GENERAL );
            return rIn;
        }
/*N*/
/*N*/   if(aIO.GetVersion()>=1)
/*N*/   {
            for (long i = 0; i < rMemChart.nColCnt; i++)
            {
                rIn >> rMemChart.pColTable[i];
            }
	         for (long i = 0; i < rMemChart.nRowCnt; i++)
            {
		        rIn >> rMemChart.pRowTable[i];
            }
/*N*/
/*N*/       if(aIO.GetVersion()>=2)
/*N*/           rIn >> rMemChart.nTranslated;
/*N*/
/*N*/   }
/*N*/   else
/*N*/   {
/*?*/       rMemChart.ResetTranslation(rMemChart.pRowTable,rMemChart.nRowCnt);
/*?*/       rMemChart.ResetTranslation(rMemChart.pColTable,rMemChart.nColCnt);
/*N*/   }
/*N*/
/*N*/   rMemChart.InitNumFmt(); //ab IOVersion 2 (ToDo:)
/*N*/
/*N*/   return rIn;
/*N*/ }


/*************************************************************************
|*
|* QuickSort ueber Zeilen
|*
\************************************************************************/


/*N*/ void SchMemChart::InitNumFmt()
/*N*/ {
/*N*/   long i;
        if ( pColNumFmtId )
        {
            for ( i = 0; i < nColCnt; i++ )
            {
                pColNumFmtId[i] = -1; //uninitialised!
            }
        }

        if ( pRowNumFmtId )
        {
            for ( i = 0; i < nRowCnt; i++ )
            {
                pRowNumFmtId[i] = -1;
            }
        }
/*N*/ }


/*************************************************************************
|*
|* QuickSort ueber Zeilen
|*
\************************************************************************/


/*N*/ String SchMemChart::GetDefaultColumnText( sal_Int32 nCol ) const
/*N*/ {
/*N*/   if( !mpColNameBuffer )
/*N*/   {
/*N*/       // initialize resource string
/*N*/       mpColNameBuffer = new String[ 2 ];
/*N*/       DBG_ASSERT( mpColNameBuffer, "couldn't create two strings!" );
/*N*/       String aResStr( SchResId( STR_COLUMN ).toString() );
/*N*/       xub_StrLen nPos = aResStr.SearchAscii( "$(N)" );
/*N*/       if( nPos != STRING_NOTFOUND )
/*N*/       {
/*N*/           mpColNameBuffer[ 0 ] = String( aResStr, 0, nPos );
/*N*/           mpColNameBuffer[ 1 ] = String( aResStr, nPos + sizeof( "$(N)" ) - 1, STRING_LEN );
/*N*/       }
/*N*/       else
/*?*/           mpColNameBuffer[ 0 ] = aResStr;
/*N*/   }
/*N*/
/*N*/   if( mpColNameBuffer )
/*N*/   {
/*N*/       String aResult( mpColNameBuffer[ 0 ] );
/*N*/       aResult.Append( String::CreateFromInt32( nCol + 1 ));
/*N*/       aResult.Append( mpColNameBuffer[ 1 ] );
/*N*/       return aResult;
/*N*/   }
/*N*/   else
/*?*/       return String();
/*N*/ }

/*N*/ String SchMemChart::GetDefaultRowText( sal_Int32 nRow ) const
/*N*/ {
/*N*/   if( !mpRowNameBuffer )
/*N*/   {
/*N*/       // initialize resource string
/*N*/       mpRowNameBuffer = new String[ 2 ];
/*N*/       DBG_ASSERT( mpRowNameBuffer, "couldn't create two strings!" );
/*N*/       String aResStr( SchResId( STR_ROW ).toString() );
/*N*/       xub_StrLen nPos = aResStr.SearchAscii( "$(N)" );
/*N*/       if( nPos != STRING_NOTFOUND )
/*N*/       {
/*N*/           mpRowNameBuffer[ 0 ] = String( aResStr, 0, nPos );
/*N*/           mpRowNameBuffer[ 1 ] = String( aResStr, nPos + sizeof( "$(N)" ) - 1, STRING_LEN );
/*N*/       }
/*N*/       else
/*?*/           mpRowNameBuffer[ 0 ] = aResStr;
/*N*/   }
/*N*/
/*N*/   if( mpRowNameBuffer )
/*N*/   {
/*N*/       String aResult( mpRowNameBuffer[ 0 ] );
/*N*/       aResult.Append( String::CreateFromInt32( nRow + 1 ));
/*N*/       aResult.Append( mpRowNameBuffer[ 1 ] );
/*N*/       return aResult;
/*N*/   }
/*N*/   else
/*?*/       return String();
/*N*/ }

using namespace ::com::sun::star;

// ========================================

/*N*/ void SchMemChart::getXMLStringForCellAddress( const SchCellAddress& rCell, rtl::OUStringBuffer * output )
/*N*/ {
/*N*/     OSL_ASSERT(output != 0);
/*N*/     ::std::vector< SchSingleCell >::const_iterator aIter;
/*N*/     const ::std::vector< SchSingleCell >::const_iterator aEndIter = rCell.maCells.end();
/*N*/
/*N*/     for( aIter = rCell.maCells.begin(); aIter != aEndIter; ++aIter )
/*N*/     {
/*N*/         sal_Int32 nCol = aIter->mnColumn;
/*N*/         output->append( (sal_Unicode)'.' );
/*N*/         if( ! aIter->mbRelativeColumn )
/*N*/             output->append( (sal_Unicode)'$' );
/*N*/
/*N*/         // get A, B, C, ..., AA, AB, ... representation of column number
/*N*/         if( nCol < 26 )
/*N*/             output->append( (sal_Unicode)('A' + nCol) );
/*N*/         else if( nCol < 702 )
/*N*/         {
/*?*/             output->append( (sal_Unicode)('A' + nCol / 26 - 1 ));
/*?*/             output->append( (sal_Unicode)('A' + nCol % 26) );
/*N*/         }
/*N*/         else    // works for nCol <= 18,278
/*N*/         {
/*?*/             output->append( (sal_Unicode)('A' + nCol / 702 - 1 ));
/*?*/             output->append( (sal_Unicode)('A' + (nCol % 702) / 26 ));
/*?*/             output->append( (sal_Unicode)('A' + nCol % 26) );
/*N*/         }
/*N*/
/*N*/         // write row number as number
/*N*/         if( ! aIter->mbRelativeRow )
/*N*/             output->append( (sal_Unicode)'$' );
/*N*/         output->append( aIter->mnRow + (sal_Int32)1 );
/*N*/     }
/*N*/ }

/*N*/  void SchMemChart::getSingleCellAddressFromXMLString(
/*N*/      const ::rtl::OUString& rXMLString,
/*N*/      sal_Int32 nStartPos, sal_Int32 nEndPos,
/*N*/      SchSingleCell& rSingleCell )
/*N*/  {
/*N*/      // expect "\$?[a-zA-Z]+\$?[1-9][0-9]*"
/*N*/      static const sal_Unicode aDollar( '$' );
/*N*/      static const sal_Unicode aLetterA( 'A' );
/*N*/
/*N*/      ::rtl::OUString aCellStr = rXMLString.copy( nStartPos, nEndPos - nStartPos + 1 ).toAsciiUpperCase();
/*N*/      const sal_Unicode* pStrArray = aCellStr.getStr();
/*N*/      sal_Int32 nLength = aCellStr.getLength();
/*N*/      sal_Int32 i = nLength - 1, nColumn = 0;
/*N*/
/*N*/      // parse number for row
/*N*/      while( CharClass::isAsciiDigit( pStrArray[ i ] ) && i >= 0 )
/*N*/          i--;
/*N*/      rSingleCell.mnRow = (aCellStr.copy( i + 1 )).toInt32() - 1;
/*N*/      // a dollar in XML means absolute (whereas in UI it means relative)
/*N*/      if( pStrArray[ i ] == aDollar )
/*N*/      {
/*N*/          i--;
/*N*/          rSingleCell.mbRelativeRow = sal_False;
/*N*/      }
/*N*/      else
/*N*/          rSingleCell.mbRelativeRow = sal_True;
/*N*/
/*N*/      // parse rest for column
/*N*/      sal_Int32 nPower = 1;
/*N*/      while( CharClass::isAsciiAlpha( pStrArray[ i ] ))
/*N*/      {
/*N*/          nColumn += (pStrArray[ i ] - aLetterA + 1) * nPower;
/*N*/          i--;
/*N*/          nPower *= 26;
/*N*/      }
/*N*/      rSingleCell.mnColumn = nColumn - 1;
/*N*/
/*N*/      rSingleCell.mbRelativeColumn = sal_True;
/*N*/      if( i >= 0 &&
/*N*/          pStrArray[ i ] == aDollar )
/*N*/          rSingleCell.mbRelativeColumn = sal_False;
/*N*/  }

/*N*/  bool SchMemChart::getCellAddressFromXMLString(
/*N*/      const ::rtl::OUString& rXMLString,
/*N*/      sal_Int32 nStartPos, sal_Int32 nEndPos,
/*N*/      SchCellAddress& rOutCell,
/*N*/      ::rtl::OUString& rOutTableName )
/*N*/  {
/*N*/      static const sal_Unicode aDot( '.' );
/*N*/      static const sal_Unicode aQuote( '\'' );
/*N*/      static const sal_Unicode aBackslash( '\\' );
/*N*/
/*N*/      sal_Int32 nNextDelimiterPos = nStartPos;
/*N*/
/*N*/      sal_Int32 nDelimiterPos = nStartPos;
/*N*/      bool bInQuotation = false;
/*N*/      // parse table name
/*N*/      while( nDelimiterPos < nEndPos &&
/*N*/             ( bInQuotation || rXMLString[ nDelimiterPos ] != aDot ))
/*N*/      {
/*N*/          // skip escaped characters (with backslash)
/*N*/          if( rXMLString[ nDelimiterPos ] == aBackslash )
/*N*/              ++nDelimiterPos;
/*N*/          // toggle quotation mode when finding single quotes
/*N*/          else if( rXMLString[ nDelimiterPos ] == aQuote )
/*N*/              bInQuotation = ! bInQuotation;
/*N*/
/*N*/          ++nDelimiterPos;
/*N*/      }
/*N*/
/*N*/      if( nDelimiterPos == -1 ||
/*N*/          nDelimiterPos >= nEndPos )
/*N*/      {
/*N*/  #if OSL_DEBUG_LEVEL > 1
/*N*/          rtl::OUString aStr( rXMLString.copy( nStartPos, nEndPos - nStartPos + 1 ));
/*N*/          rtl::OString aBstr(rtl::OUStringToOString(aStr, RTL_TEXTENCODING_ASCII_US));
/*N*/          OSL_TRACE( "Invalid Cell Address <%s> found in XML file", aBstr.getStr());
/*N*/  #endif
/*N*/          return false;
/*N*/      }
/*N*/      if( nDelimiterPos > nStartPos )
/*N*/      {
/*N*/          // there is a table name before the address
/*N*/
/*N*/          ::rtl::OUStringBuffer aTableNameBuffer;
/*N*/          const sal_Unicode * pTableName = rXMLString.getStr();
/*N*/
/*N*/          // remove escapes from table name
/*N*/          ::std::for_each( pTableName + nStartPos,
/*N*/                           pTableName + nDelimiterPos,
/*N*/                           lcl_UnEscape( aTableNameBuffer ));
/*N*/
/*N*/          // unquote quoted table name
/*N*/          const sal_Unicode * pBuf = aTableNameBuffer.getStr();
/*N*/          if( pBuf[ 0 ] == aQuote &&
/*N*/              pBuf[ aTableNameBuffer.getLength() - 1 ] == aQuote )
/*N*/          {
/*N*/              ::rtl::OUString aName = aTableNameBuffer.makeStringAndClear();
/*N*/              rOutTableName = aName.copy( 1, aName.getLength() - 2 );
/*N*/          }
/*N*/          else
/*N*/              rOutTableName = aTableNameBuffer.makeStringAndClear();
/*N*/      }
/*N*/
/*N*/      for( sal_Int32 i = 0;
/*N*/           nNextDelimiterPos < nEndPos;
/*N*/           nDelimiterPos = nNextDelimiterPos, i++ )
/*N*/      {
/*N*/          nNextDelimiterPos = rXMLString.indexOf( aDot, nDelimiterPos + 1 );
/*N*/          if( nNextDelimiterPos == -1 ||
/*N*/              nNextDelimiterPos > nEndPos )
/*N*/              nNextDelimiterPos = nEndPos + 1;
/*N*/
/*N*/          rOutCell.maCells.resize( i + 1 );
/*N*/          getSingleCellAddressFromXMLString( rXMLString,
/*N*/                                             nDelimiterPos + 1, nNextDelimiterPos - 1,
/*N*/                                             rOutCell.maCells[ i ] );
/*N*/      }
/*N*/
/*N*/      return true;
/*N*/  }

/*N*/ bool SchMemChart::getCellRangeAddressFromXMLString(
/*N*/     const ::rtl::OUString& rXMLString,
/*N*/     sal_Int32 nStartPos, sal_Int32 nEndPos,
/*N*/     SchCellRangeAddress& rOutRange )
/*N*/ {
/*N*/      bool bResult = true;
/*N*/      static const sal_Unicode aColon( ':' );
/*N*/      static const sal_Unicode aQuote( '\'' );
/*N*/      static const sal_Unicode aBackslash( '\\' );
/*N*/
/*N*/      sal_Int32 nDelimiterPos = nStartPos;
/*N*/      bool bInQuotation = false;
/*N*/      // parse table name
/*N*/      while( nDelimiterPos < nEndPos &&
/*N*/             ( bInQuotation || rXMLString[ nDelimiterPos ] != aColon ))
/*N*/      {
/*N*/          // skip escaped characters (with backslash)
/*N*/          if( rXMLString[ nDelimiterPos ] == aBackslash )
/*N*/              ++nDelimiterPos;
/*N*/          // toggle quotation mode when finding single quotes
/*N*/          else if( rXMLString[ nDelimiterPos ] == aQuote )
/*N*/              bInQuotation = ! bInQuotation;
/*N*/
/*N*/          ++nDelimiterPos;
/*N*/      }
/*N*/
/*N*/      if( nDelimiterPos <= nStartPos ||              // includes == and 'not found' (==-1)
/*N*/          nDelimiterPos >= nEndPos )
/*N*/      {
/*N*/  #if OSL_DEBUG_LEVEL > 0
/*N*/          rtl::OUString aStr( rXMLString.copy( nStartPos, nEndPos - nStartPos + 1 ));
/*N*/          rtl::OString aBstr(rtl::OUStringToOString(aStr, RTL_TEXTENCODING_ASCII_US));
/*N*/          OSL_TRACE( "Invalid Cell Range <%s> found in XML file", aBstr.getStr());
/*N*/  #endif
/*N*/          return false;
/*N*/      }
/*N*/      bResult = getCellAddressFromXMLString( rXMLString, nStartPos, nDelimiterPos - 1,
/*N*/                                             rOutRange.maUpperLeft,
/*N*/                                             rOutRange.msTableName );
/*N*/      ::rtl::OUString sTableSecondName;
/*N*/      if( bResult )
/*N*/      {
/*N*/          bResult = getCellAddressFromXMLString( rXMLString, nDelimiterPos + 1, nEndPos,
/*N*/                                                 rOutRange.maLowerRight,
/*N*/                                                 sTableSecondName );
/*N*/      }
/*N*/      DBG_ASSERT( sTableSecondName.getLength() == 0 ||
/*N*/                  sTableSecondName.equals( rOutRange.msTableName ),
/*N*/                  "Cell Range must be inside the same sheet" );
/*N*/      return bResult;
/*N*/ }

/// interpret maChartRange and fill XML string with that
/*N*/ ::rtl::OUString SchMemChart::getXMLStringForChartRange()
/*N*/ {
/*N*/     static const sal_Unicode aSpace( ' ' );
/*N*/     static const sal_Unicode aQuote( '\'' );
/*N*/
/*N*/     ::rtl::OUStringBuffer aBuffer;
/*N*/     ::std::vector< SchCellRangeAddress >::iterator aIter;
/*N*/     const ::std::vector< SchCellRangeAddress >::iterator aEndIter = maChartRange.maRanges.end();
/*N*/
/*N*/     for( aIter = maChartRange.maRanges.begin(); aIter != aEndIter; /* increment done in body */ )
/*N*/     {
/*?*/         if( (aIter->msTableName).getLength())
/*?*/         {
/*?*/             bool bNeedsEscaping = ( aIter->msTableName.indexOf( aQuote ) > -1 );
/*?*/             bool bNeedsQuoting = bNeedsEscaping || ( aIter->msTableName.indexOf( aSpace ) > -1 );
/*?*/
/*?*/             // quote table name if it contains spaces or quotes
/*?*/             if( !bNeedsQuoting )
/*?*/                 aBuffer.append( aIter->msTableName );
/*?*/         }
/*?*/         getXMLStringForCellAddress( aIter->maUpperLeft, &aBuffer );
/*?*/
/*?*/         if( aIter->maLowerRight.maCells.size())
/*?*/         {
/*?*/             // we have a range (not a single cell)
/*?*/             aBuffer.append( sal_Unicode( ':' ));
/*?*/             getXMLStringForCellAddress( aIter->maLowerRight, &aBuffer );
/*?*/         }
/*?*/
/*?*/         ++aIter;
/*?*/         // separator for more than one range
/*?*/         if( aIter != aEndIter )
/*?*/             aBuffer.append( sal_Unicode( ' ' ));
/*N*/     }
/*N*/
/*N*/     return aBuffer.makeStringAndClear();
/*N*/ }

/// parse String and put results into maChartRange
/*N*/ void SchMemChart::getChartRangeForXMLString( const ::rtl::OUString& rXMLString )
/*N*/ {
/*N*/     static const sal_Unicode aSpace( ' ' );
/*N*/     static const sal_Unicode aQuote( '\'' );
/*N*/     static const sal_Unicode aDollar( '$' );
/*N*/     static const sal_Unicode aBackslash( '\\' );
/*N*/
/*N*/     sal_Int32 nStartPos = 0;
/*N*/     sal_Int32 nEndPos = nStartPos;
/*N*/     const sal_Int32 nLength = rXMLString.getLength();
/*N*/
/*N*/     // reset
/*N*/     maChartRange.maRanges.clear();
/*N*/
/*N*/     // iterate over different ranges
/*N*/     for( sal_Int32 i = 0;
/*N*/          nEndPos < nLength;
/*N*/          nStartPos = ++nEndPos, i++ )
/*N*/     {
/*N*/         // find start point of next range
/*N*/
/*N*/         // ignore leading '$'
/*N*/         if( rXMLString[ nEndPos ] == aDollar)
/*N*/             nEndPos++;
/*N*/
/*N*/         bool bInQuotation = false;
/*N*/         // parse range
/*N*/         while( nEndPos < nLength &&
/*N*/                ( bInQuotation || rXMLString[ nEndPos ] != aSpace ))
/*N*/         {
/*N*/             // skip escaped characters (with backslash)
/*N*/             if( rXMLString[ nEndPos ] == aBackslash )
/*N*/                 ++nEndPos;
/*N*/             // toggle quotation mode when finding single quotes
/*N*/             else if( rXMLString[ nEndPos ] == aQuote )
/*N*/                 bInQuotation = ! bInQuotation;
/*N*/
/*N*/             ++nEndPos;
/*N*/         }
/*N*/
/*N*/         maChartRange.maRanges.resize( i + 1 );
/*N*/         if( ! getCellRangeAddressFromXMLString(
/*N*/                 rXMLString,
/*N*/                 nStartPos, nEndPos - 1,
/*N*/                 maChartRange.maRanges[ i ] ))
/*N*/         {
/*N*/             // if an error occurred, bail out
/*N*/             maChartRange.maRanges.clear();
/*N*/             break;
/*N*/         }
/*N*/     }
/*N*/
/*N*/ #if OSL_DEBUG_LEVEL > 0
/*N*/     // output result
/*N*/     OSL_TRACE(
/*N*/         ::rtl::OUStringToOString(
/*N*/             ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Ranges retrieved from XML-String: \"" )) +
/*N*/             rXMLString +
/*N*/             ::rtl::OUString( sal_Unicode( '\"' )),
/*N*/             RTL_TEXTENCODING_ASCII_US ).getStr() );
/*N*/     OSL_TRACE( "Size: %d", maChartRange.maRanges.size() );
/*N*/     for( ::std::vector< SchCellRangeAddress >::const_iterator aIter = maChartRange.maRanges.begin();
/*N*/          aIter != maChartRange.maRanges.end();
/*N*/          ++aIter )
/*N*/     {
/*N*/         OSL_TRACE( " Cell Address found:" );
/*N*/         OSL_TRACE( "  Upper-Left: " );
/*N*/         for( ::std::vector< SchSingleCell >::const_iterator aIter2 = (*aIter).maUpperLeft.maCells.begin();
/*N*/              aIter2 != (*aIter).maUpperLeft.maCells.end();
/*N*/              ++aIter2 )
/*N*/         {
/*N*/             OSL_TRACE( "  Column: %d, Row: %d,  Rel-Col: %s, Rel-Row: %s",
/*N*/                        (*aIter2).mnColumn, (*aIter2).mnRow,
/*N*/                        (*aIter2).mbRelativeColumn ? "true" : "false",
/*N*/                        (*aIter2).mbRelativeRow ? "true" : "false" );
/*N*/         }
/*N*/         OSL_TRACE( "  Lower-Right: " );
/*N*/         for( ::std::vector< SchSingleCell >::const_iterator aIter3 = (*aIter).maLowerRight.maCells.begin();
/*N*/              aIter3 != (*aIter).maLowerRight.maCells.end();
/*N*/              ++aIter3 )
/*N*/         {
/*N*/             OSL_TRACE( "  Column: %d, Row: %d,  Rel-Col: %s, Rel-Row: %s",
/*N*/                        (*aIter3).mnColumn, (*aIter3).mnRow,
/*N*/                        (*aIter3).mbRelativeColumn ? "true" : "false",
/*N*/                        (*aIter3).mbRelativeRow ? "true" : "false" );
/*N*/         }
/*N*/         OSL_TRACE(
/*N*/             ::rtl::OUStringToOString(
/*N*/                 ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "  Table-Name: \"" )) +
/*N*/                 (*aIter).msTableName +
/*N*/                 ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "\", Number: " )) +
/*N*/                 ::rtl::OUString::valueOf( (*aIter).mnTableNumber ),
/*N*/                 RTL_TEXTENCODING_ASCII_US ).getStr() );
/*N*/     }
/*N*/ #endif
/*N*/ }

/** this is needed for export of charts embedded in calc
    for saving an XML document in old binary format
*/
/*N*/ ::rtl::OUString SchMemChart::createTableNumberList()
/*N*/ {
/*N*/     ::rtl::OUStringBuffer aBuffer;
/*N*/     ::std::vector< SchCellRangeAddress >::iterator aIter;
/*N*/     const ::std::vector< SchCellRangeAddress >::iterator aEndIter = maChartRange.maRanges.end();
/*N*/     sal_Bool bStarted = sal_False;
/*N*/
/*N*/     for( aIter = maChartRange.maRanges.begin(); aIter != aEndIter; ++aIter )
/*N*/     {
/*N*/         if( aIter->mnTableNumber != -1 )
/*N*/         {
/*N*/             if( bStarted )
/*?*/                 aBuffer.append( (sal_Unicode)' ' );
/*N*/             else
/*N*/                 bStarted = sal_True;
/*N*/
/*N*/             aBuffer.append( aIter->mnTableNumber );
/*N*/         }
/*N*/     }
/*N*/
/*N*/     return aBuffer.makeStringAndClear();
/*N*/ }

// prerequisite: maChartRange must be set and have the correct dimension
// for all table numbers to fit in
/*N*/ void SchMemChart::parseTableNumberList( const ::rtl::OUString& aList )
/*N*/ {
/*N*/     static const sal_Unicode aSpace( ' ' );
/*N*/     sal_Int32 nChartRangeSize = maChartRange.maRanges.size();
/*N*/     sal_Int32 nStartPos = 0, nEndPos;
/*N*/     sal_Int32 nLength = aList.getLength();
/*N*/     sal_Int32 nRangeNumber = 0;
/*N*/
/*N*/     while( nStartPos < nLength )
/*N*/     {
/*N*/         nEndPos = aList.indexOf( aSpace, nStartPos );
/*N*/         if( nEndPos == -1 )
/*N*/             nEndPos = nLength;
/*N*/
/*N*/         if( nStartPos != nEndPos ) // there were more than one space
/*N*/         {
/*N*/             if( nRangeNumber < nChartRangeSize )
/*N*/             {
/*N*/                 maChartRange.maRanges[ nRangeNumber++ ].mnTableNumber =
/*N*/                     aList.copy( nStartPos, (nEndPos - nStartPos)).toInt32();
/*N*/             }
/*N*/             else
/*N*/             {
/*N*/                 OSL_FAIL( "SchMemChart::parseTableNumberList: Too many table numbers for chart range" );
/*N*/             }
/*N*/         }
/*N*/
/*N*/         nStartPos = nEndPos + 1;
/*N*/     }
/*N*/ }


// methods to modify SchChartRange
// -------------------------------

/*N*/ static sal_Int32 lcl_GetWriterBoxNum( String& rStr, BOOL bInFirst )
/*N*/ {
/*N*/   sal_Int32 nRet = 0;
/*N*/   xub_StrLen nPos = 0;
/*N*/   if( bInFirst )
/*N*/   {
/*N*/       // the first box starts with a letter
/*N*/       sal_Unicode cChar;
/*N*/       BOOL bFirst = TRUE;
/*N*/       while( 0 != ( cChar = rStr.GetChar( nPos )) &&
/*N*/              ( (cChar >= 'A' && cChar <= 'Z') ||
/*N*/                (cChar >= 'a' && cChar <= 'z') ) )
/*N*/       {
/*N*/           if( (cChar -= 'A') >= 26 )
/*N*/               cChar -= 'a' - '[';
/*N*/           if( bFirst )
/*N*/               bFirst = FALSE;
/*N*/           else
/*N*/               ++nRet;
/*N*/           nRet = nRet * 52 + cChar;
/*N*/           ++nPos;
/*N*/       }
/*N*/       rStr.Erase( 0, nPos );      // remove the read characters
/*N*/   }
/*N*/   else if( STRING_NOTFOUND == ( nPos = rStr.Search( ':' ) ))
/*N*/   {
/*N*/       nRet = rStr.ToInt32();
/*N*/       rStr.Erase();
/*N*/   }
/*N*/   else
/*N*/   {
/*N*/       nRet = rStr.Copy( 0, nPos ).ToInt32();
/*N*/       rStr.Erase( 0, nPos+1 );
/*N*/   }
/*N*/   return nRet;
/*N*/ }

/*N*/ static void lcl_GetWriterTblBox( const String& rStr,
/*N*/                               SchCellAddress& rToFill )
/*N*/ {
/*N*/   BOOL bFirst = TRUE;
/*N*/   String sNm( rStr );
/*N*/   while( sNm.Len() )
/*N*/   {
/*N*/       SchSingleCell aCell;
/*N*/       aCell.mnColumn = ::binfilter::lcl_GetWriterBoxNum( sNm, bFirst );
/*N*/       bFirst = FALSE;
/*N*/       aCell.mnRow = ::binfilter::lcl_GetWriterBoxNum( sNm, bFirst );
/*N*/       rToFill.maCells.push_back( aCell );
/*N*/   }
/*N*/ }

/*N*/ String lcl_GetWriterBoxName( const SchCellAddress& rCell )
/*N*/ {
/*N*/   String sNm;
/*N*/
/*N*/     ::std::vector< SchSingleCell >::const_iterator aIter = rCell.maCells.begin();
/*N*/     const ::std::vector< SchSingleCell >::const_iterator aEnd = rCell.maCells.end();
/*N*/   BOOL bFirst = TRUE;
/*N*/   for( ; aIter != aEnd; ++aIter )
/*N*/   {
/*N*/       String sTmp( String::CreateFromInt32( aIter->mnRow ));
/*N*/       if( sNm.Len() )
/*N*/           sNm.Insert( '.', 0 ).Insert( sTmp, 0 );
/*N*/       else
/*N*/           sNm = sTmp;
/*N*/
/*N*/       if( bFirst )
/*N*/       {
/*N*/           const sal_Int32 coDiff = 52;    // 'A'-'Z' 'a' - 'z'
/*N*/           register sal_Int32 nCalc, nCol = aIter->mnColumn;
/*N*/
/*N*/           do {
/*N*/               nCalc = nCol % coDiff;
/*N*/               if( nCalc >= 26 )
/*N*/                   sNm.Insert( sal_Unicode('a' - 26 + nCalc ), 0 );
/*N*/               else
/*N*/                   sNm.Insert( sal_Unicode('A' + nCalc ), 0 );
/*N*/
/*N*/               if( !(nCol -= nCalc) )
/*N*/                   break;
/*N*/               nCol /= coDiff;
/*N*/               --nCol;
/*N*/           } while( 1 );
/*N*/           bFirst = FALSE;
/*N*/       }
/*N*/       else
/*N*/           sNm.Insert( '.', 0 ).Insert(
/*N*/                       String::CreateFromInt32( aIter->mnColumn ), 0 );
/*N*/   }
/*N*/   return sNm;
/*N*/ }


/// convert SomeData string(s) to SchChartRange and vice versa for Writer
/*N*/ bool SchMemChart::ConvertChartRangeForWriter( BOOL bOldToNew )
/*N*/ {
/*N*/     if( bOldToNew )     // convert SomeData1 to SchChartRange
/*N*/     {
/*N*/       SchChartRange aRange;
/*N*/       if( 2 < aSomeData1.Len() )
/*N*/       {
/*N*/           // spitze Klammern am Anfang & Ende enfernen
/*N*/           String sBox( aSomeData1 );
/*N*/           if( '<' == sBox.GetChar( 0  ) ) sBox.Erase( 0, 1 );
/*N*/           if( '>' == sBox.GetChar( sBox.Len()-1  ) ) sBox.Erase( sBox.Len()-1 );
/*N*/
/*N*/           xub_StrLen nTrenner = sBox.Search( ':' );
                if( STRING_NOTFOUND == nTrenner )
                    return false;

/*N*/           SchCellRangeAddress aCRA;
/*N*/           ::binfilter::lcl_GetWriterTblBox( sBox.Copy( 0, nTrenner ), aCRA.maUpperLeft );
/*N*/           ::binfilter::lcl_GetWriterTblBox( sBox.Copy( nTrenner+1 ), aCRA.maLowerRight );
/*N*/           aRange.maRanges.push_back( aCRA );
/*N*/       }
/*N*/       if( aSomeData2.Len() )
/*N*/       {
/*N*/           aRange.mbFirstRowContainsLabels = '1' == aSomeData2.GetChar(0);
/*N*/           aRange.mbFirstColumnContainsLabels = '1' == aSomeData2.GetChar(1);
/*N*/       }
/*N*/       SetChartRange( aRange );
/*N*/     }
/*N*/     else                // convert SchChartRange to SomeData1
/*N*/     {
/*N*/       String sData1, sData2;
/*N*/       const SchChartRange& rRg = GetChartRange();
/*N*/       if( rRg.maRanges.size() )
/*N*/       {
/*N*/           ::std::vector< SchCellRangeAddress >::const_iterator
/*N*/                                               aIter = rRg.maRanges.begin();
/*N*/           sData1.Assign( '<' )
/*N*/                 .Append( ::binfilter::lcl_GetWriterBoxName( aIter->maUpperLeft ))
/*N*/                 .Append( ':' )
/*N*/                 .Append( ::binfilter::lcl_GetWriterBoxName( aIter->maLowerRight ))
/*N*/                 .Append( '>' );
/*N*/
/*N*/           sData2.Assign( rRg.mbFirstRowContainsLabels ? '1' : '0' )
/*N*/                 .Append( rRg.mbFirstColumnContainsLabels ? '1' : '0' );
/*N*/       }
/*N*/       aSomeData1 = sData1;
/*N*/       aSomeData2 = sData2;
/*N*/     }
          return true;
/*N*/ }

/// convert SomeData string(s) to SchChartRange and vice versa for Calc
/*N*/  bool SchMemChart::ConvertChartRangeForCalc( BOOL bOldToNew )
/*N*/  {
/*N*/      if( bOldToNew )
/*N*/      {   // convert SomeData1/2/3 to SchChartRange
               if( ! SomeData1().Len() || ! SomeData2().Len() || ! SomeData3().Len())
                   return false;
/*N*/ //         DBG_ASSERT( SomeData1().Len() && SomeData2().Len() && SomeData3().Len(),
/*N*/ //            "ConvertChartRangeForCalc: can't convert old to new" );
/*N*/          SchChartRange aChartRange;
/*N*/       const sal_Unicode cTok = ';';
/*N*/       xub_StrLen nToken;
/*N*/          String aPos = SomeData1();
/*N*/       if ( (nToken = aPos.GetTokenCount( cTok )) >= 5)
/*N*/       {
/*N*/              aChartRange.mbKeepCopyOfData = sal_False;
/*N*/              String aOpt = SomeData2();
/*N*/           xub_StrLen nOptToken = aOpt.GetTokenCount( cTok );
/*N*/              BOOL bNewChart = (nOptToken >= 4);      // as of 341/342
/*N*/              DBG_ASSERT( SomeData3().Len(), "ConvertChartRangeForCalc: no sheet names" );
/*N*/              String aSheetNames = SomeData3();       // as of 638m
/*N*/           USHORT nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
/*N*/           xub_StrLen nInd = 0;
/*N*/              xub_StrLen nSheetInd = 0;
/*N*/           for ( xub_StrLen j=0; j < nToken; j+=5 )
/*N*/           {
/*N*/               xub_StrLen nInd2 = nInd;
                    // #i73906#, #144135# if table number is -1 avoid conversion to 65535
                    {
                        sal_Int32 nTableNum = aPos.GetToken( 0, cTok, nInd ).ToInt32();
                        nTab1 = (nTableNum<0 ? 0: static_cast< USHORT >( nTableNum ));
                    }
/*N*/                  // To make old versions (<341/342) skip it, the token separator
/*N*/                  // is a ','
/*N*/               if ( bNewChart )
                    {
                        // #i73906#, #144135# if table number is -1 avoid conversion to 65535
                        sal_Int32 nTableNum = aPos.GetToken( 1, ',', nInd2 ).ToInt32();
                        nTab2 = (nTableNum<0 ? 0: static_cast< USHORT >( nTableNum ));
                    }
/*N*/               else
/*N*/                   nTab2 = nTab1;
/*N*/               nCol1 = (USHORT) aPos.GetToken( 0, cTok, nInd ).ToInt32();
/*N*/               nRow1 = (USHORT) aPos.GetToken( 0, cTok, nInd ).ToInt32();
/*N*/               nCol2 = (USHORT) aPos.GetToken( 0, cTok, nInd ).ToInt32();
/*N*/               nRow2 = (USHORT) aPos.GetToken( 0, cTok, nInd ).ToInt32();
/*N*/                  for ( USHORT nTab = nTab1; nTab <= nTab2; ++nTab )
/*N*/                  {
/*N*/                      SchCellRangeAddress aCellRangeAddress;
/*N*/                      SchSingleCell aCell;
/*N*/                      aCell.mnColumn = nCol1;
/*N*/                      aCell.mnRow = nRow1;
/*N*/                      aCellRangeAddress.maUpperLeft.maCells.push_back( aCell );
/*N*/                      aCell.mnColumn = nCol2;
/*N*/                      aCell.mnRow = nRow2;
/*N*/                      aCellRangeAddress.maLowerRight.maCells.push_back( aCell );
/*N*/                      aCellRangeAddress.mnTableNumber = nTab;
/*N*/                      String aName( aSheetNames.GetToken( 0, cTok, nSheetInd ) );
/*N*/                      aCellRangeAddress.msTableName = aName;
/*N*/                      aChartRange.maRanges.push_back( aCellRangeAddress );
/*N*/                  }
/*N*/           }
/*N*/
/*N*/              if ( aOpt.Len() >= 2 )
/*N*/           {
/*N*/                  aChartRange.mbFirstRowContainsLabels    = ( aOpt.GetChar(0) != '0' );
/*N*/                  aChartRange.mbFirstColumnContainsLabels = ( aOpt.GetChar(1) != '0' );
/*N*/           }
/*N*/           else
/*N*/              {
/*N*/                  aChartRange.mbFirstColumnContainsLabels = sal_False;
/*N*/                  aChartRange.mbFirstRowContainsLabels = sal_False;
/*N*/              }
/*N*/       }
/*N*/       else
/*N*/       {
/*N*/              aChartRange.mbFirstColumnContainsLabels = sal_False;
/*N*/              aChartRange.mbFirstRowContainsLabels = sal_False;
/*N*/              aChartRange.mbKeepCopyOfData = sal_True;
/*N*/       }
/*N*/          SetChartRange( aChartRange );
/*N*/      }
/*N*/      else
/*N*/      {   // convert SchChartRange to SomeData1/2/3
/*N*/          const sal_Unicode cTok = ';';
/*N*/          String aRef, aSheetNames;
/*N*/          const SchChartRange& rChartRange = GetChartRange();
/*N*/          ::std::vector< SchCellRangeAddress >::const_iterator iRange =
/*N*/              rChartRange.maRanges.begin();
/*N*/          DBG_ASSERT( iRange != rChartRange.maRanges.end(),
/*N*/              "ConvertChartRangeForCalc: no SchCellRangeAddress vector" );
/*N*/          for ( ; iRange != rChartRange.maRanges.end(); ++iRange )
/*N*/          {
/*N*/              const SchSingleCell& rAddr1 = iRange->maUpperLeft.maCells[0];
/*N*/              const SchSingleCell& rAddr2 = iRange->maLowerRight.maCells[0];
/*N*/              sal_Int32 nTab = iRange->mnTableNumber;
                    // #i73906#, #144135# do not export -1 as table number
                   if( nTab < 0 )
                       nTab = 0;
/*N*/              if ( aRef.Len() )
/*N*/                  aRef += cTok;
/*N*/              aRef += String::CreateFromInt32( nTab );
/*N*/              // here ',' as TokenSep so old versions (<341/342) will ignore it
/*N*/              aRef += ',';  aRef += String::CreateFromInt32( nTab );
/*N*/              aRef += cTok; aRef += String::CreateFromInt32( rAddr1.mnColumn );
/*N*/              aRef += cTok; aRef += String::CreateFromInt32( rAddr1.mnRow );
/*N*/              aRef += cTok; aRef += String::CreateFromInt32( rAddr2.mnColumn );
/*N*/              aRef += cTok; aRef += String::CreateFromInt32( rAddr2.mnRow );
/*N*/              if ( aSheetNames.Len() )
/*N*/                  aSheetNames += cTok;
/*N*/              aSheetNames += String( iRange->msTableName );
/*N*/          }
/*N*/
/*N*/          String aFlags = rChartRange.mbFirstRowContainsLabels ? '1' : '0';
/*N*/          aFlags += rChartRange.mbFirstColumnContainsLabels ? '1' : '0';
/*N*/
/*N*/          SomeData1() = aRef;
/*N*/          SomeData2() = aFlags;
/*N*/          SomeData3() = aSheetNames;
/*N*/      }
           return true;
/*N*/  }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
