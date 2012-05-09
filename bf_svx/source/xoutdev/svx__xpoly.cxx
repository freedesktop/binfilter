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

#include <string.h>
#include <tools/stream.hxx>

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "xoutx.hxx"
#include "xpolyimp.hxx"
#include <osl/endian.h>
namespace binfilter {

#define GLOBALOVERFLOW

/*N*/ DBG_NAME(XPolygon)
/*N*/ DBG_NAME(XPolyPolygon)

/*************************************************************************
|*
|*    ImpXPolygon::ImpXPolygon()
|*
*************************************************************************/

/*N*/ ImpXPolygon::ImpXPolygon( USHORT nInitSize, USHORT nInResize )
/*N*/ {
/*N*/   pPointAry               = NULL;
/*N*/   pFlagAry                = NULL;
/*N*/   bDeleteOldPoints        = FALSE;
/*N*/   nSize                   = 0;
/*N*/   nResize                 = nInResize;
/*N*/   nPoints                 = 0;
/*N*/   nRefCount               = 1;
/*N*/
/*N*/   Resize( nInitSize );
/*N*/ }

/*************************************************************************
|*
|*    ImpXPolygon::ImpXPolygon()
|*
*************************************************************************/

/*N*/ ImpXPolygon::ImpXPolygon( const ImpXPolygon& rImpXPoly )
/*N*/ {
/*N*/   ( (ImpXPolygon&) rImpXPoly ).CheckPointDelete();
/*N*/
/*N*/   pPointAry               = NULL;
/*N*/   pFlagAry                = NULL;
/*N*/   bDeleteOldPoints        = FALSE;
/*N*/   nSize                   = 0;
/*N*/   nResize                 = rImpXPoly.nResize;
/*N*/   nPoints                 = 0;
/*N*/   nRefCount               = 1;
/*N*/
/*N*/   Resize( rImpXPoly.nSize );
/*N*/
/*N*/   // Kopieren
/*N*/   nPoints = rImpXPoly.nPoints;
/*N*/   memcpy( pPointAry, rImpXPoly.pPointAry, nSize*sizeof( Point ) );
/*N*/   memcpy( pFlagAry, rImpXPoly.pFlagAry, nSize );
/*N*/ }

/*************************************************************************
|*
|*    ImpXPolygon::~ImpXPolygon()
|*
*************************************************************************/

/*N*/ ImpXPolygon::~ImpXPolygon()
/*N*/ {
/*N*/   delete[] (char*) pPointAry;
/*N*/   delete[] pFlagAry;
/*N*/   if ( bDeleteOldPoints )
/*?*/       delete[] (char*) pOldPointAry;
/*N*/ }

/*************************************************************************
|*
|*    ImpXPolygon::operator==()
|*
*************************************************************************/


/*N*/ bool ImpXPolygon::operator==(const ImpXPolygon& rImpXPoly) const
/*N*/ {
/*N*/   return nPoints==rImpXPoly.nPoints &&
/*N*/          (nPoints==0 ||
/*N*/           (memcmp(pPointAry,rImpXPoly.pPointAry,nPoints*sizeof(Point))==0 &&
/*N*/            memcmp(pFlagAry,rImpXPoly.pFlagAry,nPoints)==0));
/*N*/ }

/*************************************************************************
|*
|*    ImpXPolygon::Resize()
|*
|*    !!! Polygongroesse aendern - wenn bDeletePoints FALSE, dann den
|*    Point-Array nicht loeschen, sondern in pOldPointAry sichern und
|*    das Flag bDeleteOldPoints setzen. Beim naechsten Zugriff wird
|*    das Array dann geloescht.
|*    Damit wird verhindert, dass bei XPoly[n] = XPoly[0] durch ein
|*    Resize der fuer den rechten Ausdruck verwendete Point-Array
|*    vorzeitig geloescht wird.
|*
*************************************************************************/

/*N*/ void ImpXPolygon::Resize( USHORT nNewSize, BOOL bDeletePoints )
/*N*/ {
/*N*/   if( nNewSize == nSize )
/*N*/       return;
/*N*/
/*N*/   BYTE*   pOldFlagAry  = pFlagAry;
/*N*/   USHORT  nOldSize     = nSize;
/*N*/
/*N*/   CheckPointDelete();
/*N*/   pOldPointAry = pPointAry;
/*N*/
/*N*/   // Neue Groesse auf vielfaches von nResize runden, sofern Objekt
/*N*/   // nicht neu angelegt wurde (nSize != 0)
/*N*/   if ( nSize != 0 && nNewSize > nSize )
/*N*/   {
/*N*/       DBG_ASSERT(nResize, "Resize-Versuch trotz nResize = 0 !");
/*N*/       nNewSize = nSize + ((nNewSize-nSize-1) / nResize + 1) * nResize;
/*N*/   }
/*N*/   // Punkt Array erzeugen
/*N*/   nSize     = nNewSize;
/*N*/   pPointAry = (Point*)new char[ nSize*sizeof( Point ) ];
/*N*/   memset( pPointAry, 0, nSize*sizeof( Point ) );
/*N*/
/*N*/   // Flag Array erzeugen
/*N*/   pFlagAry = new BYTE[ nSize ];
/*N*/   memset( pFlagAry, 0, nSize );
/*N*/
/*N*/   // Eventuell umkopieren
/*N*/   if( nOldSize )
/*N*/   {
/*N*/       if( nOldSize < nSize )
/*N*/       {
/*N*/           memcpy( pPointAry, pOldPointAry, nOldSize*sizeof( Point ) );
/*N*/           memcpy( pFlagAry,  pOldFlagAry, nOldSize );
/*N*/       }
/*N*/       else
/*N*/       {
/*N*/           memcpy( pPointAry, pOldPointAry, nSize*sizeof( Point ) );
/*N*/           memcpy( pFlagAry, pOldFlagAry, nSize );
/*N*/
/*N*/           // Anzahl der gueltigen Punkte anpassen
/*N*/           if( nPoints > nSize )
/*N*/               nPoints = nSize;
/*N*/       }
/*N*/       if ( bDeletePoints )    delete[] (char*) pOldPointAry;
/*N*/       else                    bDeleteOldPoints = TRUE;
/*N*/       delete[] pOldFlagAry;
/*N*/   }
/*N*/ }


/*************************************************************************
|*
|*    ImpXPolygon::InsertSpace()
|*
*************************************************************************/

/*N*/ void ImpXPolygon::InsertSpace( USHORT nPos, USHORT nCount )
/*N*/ {
/*N*/   CheckPointDelete();
/*N*/
/*N*/   if ( nPos > nPoints )
/*N*/       nPos = nPoints;
/*N*/
/*N*/   // Wenn Polygon zu klein dann groesser machen
/*N*/   if( (nPoints + nCount) > nSize )
/*N*/       Resize( nPoints + nCount );
/*N*/
/*N*/   // Wenn nicht hinter dem letzten Punkt eingefuegt wurde,
/*N*/   // den Rest nach hinten schieben
/*N*/   if( nPos < nPoints )
/*N*/   {
/*N*/       USHORT nMove = nPoints - nPos;
/*N*/       memmove( &pPointAry[nPos+nCount], &pPointAry[nPos],
/*N*/                nMove * sizeof(Point) );
/*N*/       memmove( &pFlagAry[nPos+nCount], &pFlagAry[nPos], nMove );
/*N*/   }
/*N*/   memset( &pPointAry[nPos], 0, nCount * sizeof( Point ) );
/*N*/   memset( &pFlagAry [nPos], 0, nCount );
/*N*/
/*N*/   nPoints += nCount;
/*N*/ }


/*************************************************************************
|*
|*    ImpXPolygon::Remove()
|*
*************************************************************************/

/*N*/ void ImpXPolygon::Remove( USHORT nPos, USHORT nCount )
/*N*/ {
/*N*/   CheckPointDelete();
/*N*/
/*N*/   if( (nPos + nCount) <= nPoints )
/*N*/   {
/*N*/       USHORT nMove = nPoints - nPos - nCount;
/*N*/
/*N*/       if( nMove )
/*N*/       {
/*?*/           memmove( &pPointAry[nPos], &pPointAry[nPos+nCount],
/*?*/                    nMove * sizeof(Point) );
/*?*/           memmove( &pFlagAry[nPos], &pFlagAry[nPos+nCount], nMove );
/*N*/       }
/*N*/       memset( &pPointAry[nPoints - nCount], 0, nCount * sizeof( Point ) );
/*N*/       memset( &pFlagAry [nPoints - nCount], 0, nCount );
/*N*/       nPoints -= nCount;
/*N*/   }
/*N*/ }


/*************************************************************************
|*
|*    XPolygon::XPolygon()
|*
*************************************************************************/

/*N*/ XPolygon::XPolygon( USHORT nSize, USHORT nResize )
/*N*/ {
/*N*/   DBG_CTOR(XPolygon,NULL);
/*N*/   pImpXPolygon = new ImpXPolygon( nSize, nResize );
/*N*/ }

/*************************************************************************
|*
|*    XPolygon::XPolygon()
|*
*************************************************************************/

/*N*/ XPolygon::XPolygon( const XPolygon& rXPoly )
/*N*/ {
/*N*/   DBG_CTOR(XPolygon,NULL);
/*N*/   pImpXPolygon = rXPoly.pImpXPolygon;
/*N*/   pImpXPolygon->nRefCount++;
/*N*/ }

/*************************************************************************
|*
|*    XPolygon::XPolygon()
|*
|*    XPolygon aus einem Standardpolygon erstellen
|*
*************************************************************************/

/*N*/ XPolygon::XPolygon( const Polygon& rPoly )
/*N*/ {
/*N*/   DBG_CTOR(XPolygon,NULL);
/*N*/
/*N*/   USHORT nSize = rPoly.GetSize();
/*N*/   pImpXPolygon = new ImpXPolygon( nSize );
/*N*/   pImpXPolygon->nPoints = nSize;
/*N*/
/*N*/   for( USHORT i = 0; i < nSize;  i++ )
/*N*/   {
/*N*/       pImpXPolygon->pPointAry[i] = rPoly[i];
/*N*/       pImpXPolygon->pFlagAry[i] = (BYTE) rPoly.GetFlags( i );
/*N*/   }
/*N*/ }

/*************************************************************************
|*
|*    XPolygon::XPolygon()
|*
|*    Rechteck (auch mit abgerundeten Ecken) als Bezierpolygon erzeugen
|*
*************************************************************************/

/*N*/ XPolygon::XPolygon(const Rectangle& rRect, long nRx, long nRy)
/*N*/ {
/*N*/   DBG_CTOR(XPolygon,NULL);
/*N*/   pImpXPolygon = new ImpXPolygon(17);
/*N*/   long nWh = (rRect.GetWidth()  - 1) / 2;
/*N*/   long nHh = (rRect.GetHeight() - 1) / 2;
/*N*/
/*N*/   if ( nRx > nWh )    nRx = nWh;
/*N*/   if ( nRy > nHh )    nRy = nHh;
/*N*/
/*N*/   // Rx negativ, damit Umlauf im Uhrzeigersinn erfolgt
/*N*/   nRx = -nRx;
/*N*/
/*N*/   // Faktor fuer Kontrollpunkte der Bezierkurven: 8/3 * (sin(45g) - 0.5)
/*N*/   long    nXHdl = (long)(0.552284749 * nRx);
/*N*/   long    nYHdl = (long)(0.552284749 * nRy);
/*N*/   USHORT  nPos = 0;
/*N*/
/*N*/   if ( nRx && nRy )
/*N*/   {
/*N*/       Point aCenter;
/*N*/
/*N*/       for (USHORT nQuad = 0; nQuad < 4; nQuad++)
/*N*/       {
/*N*/           switch ( nQuad )
/*N*/           {
/*N*/               case 0: aCenter = rRect.TopLeft();
/*N*/                       aCenter.X() -= nRx;
/*N*/                       aCenter.Y() += nRy;
/*N*/                       break;
/*N*/               case 1: aCenter = rRect.TopRight();
/*N*/                       aCenter.X() += nRx;
/*N*/                       aCenter.Y() += nRy;
/*N*/                       break;
/*N*/               case 2: aCenter = rRect.BottomRight();
/*N*/                       aCenter.X() += nRx;
/*N*/                       aCenter.Y() -= nRy;
/*N*/                       break;
/*N*/               case 3: aCenter = rRect.BottomLeft();
/*N*/                       aCenter.X() -= nRx;
/*N*/                       aCenter.Y() -= nRy;
/*N*/                       break;
/*N*/           }
/*N*/           GenBezArc(aCenter, nRx, nRy, nXHdl, nYHdl, 0, 900, nQuad, nPos);
/*N*/           pImpXPolygon->pFlagAry[nPos  ] = (BYTE) XPOLY_SMOOTH;
/*N*/           pImpXPolygon->pFlagAry[nPos+3] = (BYTE) XPOLY_SMOOTH;
/*N*/           nPos += 4;
/*N*/       }
/*N*/   }
/*N*/   else
/*N*/   {
/*N*/       pImpXPolygon->pPointAry[nPos++] = rRect.TopLeft();
/*N*/       pImpXPolygon->pPointAry[nPos++] = rRect.TopRight();
/*N*/       pImpXPolygon->pPointAry[nPos++] = rRect.BottomRight();
/*N*/       pImpXPolygon->pPointAry[nPos++] = rRect.BottomLeft();
/*N*/   }
/*N*/   pImpXPolygon->pPointAry[nPos] = pImpXPolygon->pPointAry[0];
/*N*/   pImpXPolygon->nPoints = nPos + 1;
/*N*/ }

/*************************************************************************
|*
|*    XPolygon::XPolygon()
|*
|*    Ellipsen(bogen) als Bezierpolygon erzeugen
|*
*************************************************************************/

/*N*/ XPolygon::XPolygon(const Point& rCenter, long nRx, long nRy,
/*N*/                  USHORT nStartAngle, USHORT nEndAngle, BOOL bClose)
/*N*/ {
/*N*/   DBG_CTOR(XPolygon,NULL);
/*N*/   pImpXPolygon = new ImpXPolygon(17);
/*N*/
/*N*/   nStartAngle %= 3600;
/*N*/   if ( nEndAngle > 3600 ) nEndAngle %= 3600;
/*N*/   BOOL bFull = (nStartAngle == 0 && nEndAngle == 3600);
/*N*/
/*N*/   // Faktor fuer Kontrollpunkte der Bezierkurven: 8/3 * (sin(45g) - 0.5)
/*N*/   long    nXHdl = (long)(0.552284749 * nRx);
/*N*/   long    nYHdl = (long)(0.552284749 * nRy);
/*N*/   USHORT  nPos = 0;
/*N*/   BOOL    bLoopEnd = FALSE;
/*N*/
/*N*/   do
/*N*/   {
/*N*/       USHORT nA1, nA2;
/*N*/       USHORT nQuad = nStartAngle / 900;
/*N*/       if ( nQuad == 4 ) nQuad = 0;
/*N*/       bLoopEnd = CheckAngles(nStartAngle, nEndAngle, nA1, nA2);
/*N*/       GenBezArc(rCenter, nRx, nRy, nXHdl, nYHdl, nA1, nA2, nQuad, nPos);
/*N*/       nPos += 3;
/*N*/       if ( !bLoopEnd )
/*N*/           pImpXPolygon->pFlagAry[nPos] = (BYTE) XPOLY_SMOOTH;
/*N*/
/*N*/   } while ( !bLoopEnd );
/*N*/
/*N*/   // Wenn kein Vollkreis, dann ggf. Enden mit Mittelpunkt verbinden
/*N*/   if ( !bFull && bClose )
/*N*/       pImpXPolygon->pPointAry[++nPos] = rCenter;
/*N*/
/*N*/   if ( bFull )
/*N*/   {
/*N*/       pImpXPolygon->pFlagAry[0   ] = (BYTE) XPOLY_SMOOTH;
/*N*/       pImpXPolygon->pFlagAry[nPos] = (BYTE) XPOLY_SMOOTH;
/*N*/   }
/*N*/   pImpXPolygon->nPoints = nPos + 1;
/*N*/ }

/*************************************************************************
|*
|*    XPolygon::~XPolygon()
|*
*************************************************************************/

/*N*/ XPolygon::~XPolygon()
/*N*/ {
/*N*/   DBG_DTOR(XPolygon,NULL);
/*N*/   if( pImpXPolygon->nRefCount > 1 )
/*N*/       pImpXPolygon->nRefCount--;
/*N*/   else
/*N*/       delete pImpXPolygon;
/*N*/ }

/*************************************************************************
|*
|*    XPolygon::CheckReference()
|*
|*    Referenzzaehler desImpXPoly pruefen und ggf. von diesem abkoppeln
|*
*************************************************************************/

/*N*/ void XPolygon::CheckReference()
/*N*/ {
/*N*/   if( pImpXPolygon->nRefCount > 1 )
/*N*/   {
/*N*/       pImpXPolygon->nRefCount--;
/*N*/       pImpXPolygon = new ImpXPolygon( *pImpXPolygon );
/*N*/   }
/*N*/ }

/*************************************************************************
|*
|*    XPolygon::SetSize()
|*
*************************************************************************/

/*N*/ void XPolygon::SetSize( USHORT nNewSize )
/*N*/ {
/*N*/   CheckReference();
/*N*/   pImpXPolygon->Resize( nNewSize );
/*N*/ }

/*************************************************************************
|*
|*    XPolygon::SetPointCount()
|*
*************************************************************************/

/*N*/ void XPolygon::SetPointCount( USHORT nPoints )
/*N*/ {
/*N*/   pImpXPolygon->CheckPointDelete();
/*N*/   CheckReference();
/*N*/
/*N*/   if( pImpXPolygon->nSize < nPoints )
/*?*/       pImpXPolygon->Resize( nPoints );
/*N*/
/*N*/   if ( nPoints < pImpXPolygon->nPoints )
/*N*/   {
/*N*/       USHORT nSize = pImpXPolygon->nPoints - nPoints;
/*N*/       memset( &pImpXPolygon->pPointAry[nPoints], 0, nSize * sizeof( Point ) );
/*N*/       memset( &pImpXPolygon->pFlagAry [nPoints], 0, nSize );
/*N*/   }
/*N*/   pImpXPolygon->nPoints = nPoints;
/*N*/ }

/*************************************************************************
|*
|*    XPolygon::GetPointCount()
|*
*************************************************************************/

/*N*/ USHORT XPolygon::GetPointCount() const
/*N*/ {
/*N*/   pImpXPolygon->CheckPointDelete();
/*N*/   return pImpXPolygon->nPoints;
/*N*/ }

/*************************************************************************
|*
|*    XPolygon::Insert()
|*
*************************************************************************/

/*N*/ void XPolygon::Insert( USHORT nPos, const Point& rPt, XPolyFlags eFlags )
/*N*/ {
/*N*/   CheckReference();
/*N*/   if (nPos>pImpXPolygon->nPoints) nPos=pImpXPolygon->nPoints;
/*N*/   pImpXPolygon->InsertSpace( nPos, 1 );
/*N*/   pImpXPolygon->pPointAry[nPos] = rPt;
/*N*/   pImpXPolygon->pFlagAry[nPos]  = (BYTE)eFlags;
/*N*/ }

/*************************************************************************
|*
|*    XPolygon::Remove()
|*
*************************************************************************/

/*N*/ void XPolygon::Remove( USHORT nPos, USHORT nCount )
/*N*/ {
/*N*/   CheckReference();
/*N*/   pImpXPolygon->Remove( nPos, nCount );
/*N*/ }

/*************************************************************************
|*
|*    XPolygon::Move()
|*
*************************************************************************/

/*N*/ void XPolygon::Move( long nHorzMove, long nVertMove )
/*N*/ {
/*N*/   if ( !nHorzMove && !nVertMove )
/*N*/       return;
/*N*/
/*N*/   CheckReference();
/*N*/
/*N*/   // Punkte verschieben
/*N*/   USHORT nCount = pImpXPolygon->nPoints;
/*N*/   for ( USHORT i = 0; i < nCount; i++ )
/*N*/   {
/*N*/       Point* pPt = &(pImpXPolygon->pPointAry[i]);
/*N*/       pPt->X() += nHorzMove;
/*N*/       pPt->Y() += nVertMove;
/*N*/   }
/*N*/ }

/*************************************************************************
|*
|*    XPolygon::GetBoundRect()
|*
*************************************************************************/

/*N*/ Rectangle XPolygon::GetBoundRect(OutputDevice *pOut) const
/*N*/ {
/*N*/   pImpXPolygon->CheckPointDelete();
/*N*/
/*N*/   Rectangle aRect(XOutCalcXPolyExtent(*this, pOut));

        return aRect;
/*N*/ }

/*************************************************************************
|*
|*    XPolygon::operator[]()
|*
*************************************************************************/

/*N*/ const Point& XPolygon::operator[]( USHORT nPos ) const
/*N*/ {
/*N*/   DBG_ASSERT(nPos < pImpXPolygon->nPoints, "Ungueltiger Index bei const-Arrayzugriff auf XPolygon");
/*N*/
/*N*/   pImpXPolygon->CheckPointDelete();
/*N*/   return pImpXPolygon->pPointAry[nPos];
/*N*/ }

/*************************************************************************
|*
|*    XPolygon::operator[]()
|*
*************************************************************************/

/*N*/ Point& XPolygon::operator[]( USHORT nPos )
/*N*/ {
/*N*/   pImpXPolygon->CheckPointDelete();
/*N*/   CheckReference();
/*N*/
/*N*/   if( nPos >= pImpXPolygon->nSize )
/*N*/   {
/*N*/       DBG_ASSERT(pImpXPolygon->nResize, "Ungueltiger Index bei Arrayzugriff auf XPolygon");
/*N*/       pImpXPolygon->Resize(nPos + 1, FALSE);
/*N*/   }
/*N*/   if( nPos >= pImpXPolygon->nPoints )
/*N*/       pImpXPolygon->nPoints = nPos + 1;
/*N*/
/*N*/   return pImpXPolygon->pPointAry[nPos];
/*N*/ }

/*************************************************************************
|*
|*    XPolygon::operator=()
|*
|*    Beschreibung      Zuweisungsoperator
|*
*************************************************************************/

/*N*/ XPolygon& XPolygon::operator=( const XPolygon& rXPoly )
/*N*/ {
/*N*/   pImpXPolygon->CheckPointDelete();
/*N*/
/*N*/   rXPoly.pImpXPolygon->nRefCount++;
/*N*/
/*N*/   if( pImpXPolygon->nRefCount > 1 )
/*N*/       pImpXPolygon->nRefCount--;
/*N*/   else
/*N*/       delete pImpXPolygon;
/*N*/
/*N*/   pImpXPolygon = rXPoly.pImpXPolygon;
/*N*/   return *this;
/*N*/ }

/*************************************************************************
|*
|*    XPolygon::operator==()
|*
|*    Beschreibung      Gleichheitsoperator
|*
*************************************************************************/

/*N*/ BOOL XPolygon::operator==( const XPolygon& rXPoly ) const
/*N*/ {
/*N*/   pImpXPolygon->CheckPointDelete();
/*N*/   if (rXPoly.pImpXPolygon==pImpXPolygon) return TRUE;
/*N*/   return *rXPoly.pImpXPolygon == *pImpXPolygon;
/*N*/ }

/*************************************************************************
|*
|*    XPolygon::operator!=()
|*
|*    Beschreibung      Ungleichheitsoperator
|*
*************************************************************************/

/*N*/ BOOL XPolygon::operator!=( const XPolygon& rXPoly ) const
/*N*/ {
/*N*/   pImpXPolygon->CheckPointDelete();
/*N*/   if (rXPoly.pImpXPolygon==pImpXPolygon) return FALSE;
/*N*/   return *rXPoly.pImpXPolygon != *pImpXPolygon;
/*N*/ }

/*************************************************************************
|*
|*    XPolygon::GetFlags()
|*
|*    Flags fuer den Punkt an der Position nPos zurueckgeben
|*
*************************************************************************/

/*N*/ XPolyFlags XPolygon::GetFlags( USHORT nPos ) const
/*N*/ {
/*N*/   pImpXPolygon->CheckPointDelete();
/*N*/   return (XPolyFlags) pImpXPolygon->pFlagAry[nPos];
/*N*/ }

/*************************************************************************
|*
|*    XPolygon::SetFlags()
|*
|*    Flags fuer den Punkt an der Position nPos setzen
|*
*************************************************************************/

/*N*/ void XPolygon::SetFlags( USHORT nPos, XPolyFlags eFlags )
/*N*/ {
/*N*/   pImpXPolygon->CheckPointDelete();
/*N*/   CheckReference();
/*N*/   pImpXPolygon->pFlagAry[nPos] = (BYTE) eFlags;
/*N*/ }

/*************************************************************************
|*
|*    XPolygon::IsControl()
|*
|*    Kurzform zur Abfrage des CONTROL-Flags
|*
*************************************************************************/

/*N*/ BOOL XPolygon::IsControl(USHORT nPos) const
/*N*/ {
/*N*/   return ( (XPolyFlags) pImpXPolygon->pFlagAry[nPos] == XPOLY_CONTROL );
/*N*/ }

/*************************************************************************
|*
|*    XPolygon::SubdivideBezier()
|*
|*    Bezierkurve unterteilen
|*
*************************************************************************/

/*N*/ void XPolygon::SubdivideBezier(USHORT nPos, BOOL bCalcFirst, double fT)
/*N*/ {
/*N*/   Point*  pPoints = pImpXPolygon->pPointAry;
/*N*/   double  fT2 = fT * fT;
/*N*/   double  fT3 = fT * fT2;
/*N*/   double  fU = 1.0 - fT;
/*N*/   double  fU2 = fU * fU;
/*N*/   double  fU3 = fU * fU2;
/*N*/   USHORT  nIdx = nPos;
/*N*/   short   nPosInc, nIdxInc;
/*N*/
/*N*/   if ( bCalcFirst )
/*N*/   {
/*N*/       nPos += 3;
/*N*/       nPosInc = -1;
/*N*/       nIdxInc = 0;
/*N*/   }
/*N*/   else
/*N*/   {
/*N*/       nPosInc = 1;
/*N*/       nIdxInc = 1;
/*N*/   }
/*N*/   pPoints[nPos].X() = (long) (fU3 *       pPoints[nIdx  ].X() +
/*N*/                               fT  * fU2 * pPoints[nIdx+1].X() * 3 +
/*N*/                               fT2 * fU  * pPoints[nIdx+2].X() * 3 +
/*N*/                               fT3 *       pPoints[nIdx+3].X());
/*N*/   pPoints[nPos].Y() = (long) (fU3 *       pPoints[nIdx  ].Y() +
/*N*/                               fT  * fU2 * pPoints[nIdx+1].Y() * 3 +
/*N*/                               fT2 * fU  * pPoints[nIdx+2].Y() * 3 +
/*N*/                               fT3 *       pPoints[nIdx+3].Y());
/*N*/   nPos += nPosInc;
/*N*/   nIdx += nIdxInc;
/*N*/   pPoints[nPos].X() = (long) (fU2 *       pPoints[nIdx  ].X() +
/*N*/                               fT  * fU *  pPoints[nIdx+1].X() * 2 +
/*N*/                               fT2 *       pPoints[nIdx+2].X());
/*N*/   pPoints[nPos].Y() = (long) (fU2 *       pPoints[nIdx  ].Y() +
/*N*/                               fT  * fU *  pPoints[nIdx+1].Y() * 2 +
/*N*/                               fT2 *       pPoints[nIdx+2].Y());
/*N*/   nPos += nPosInc;
/*N*/   nIdx += nIdxInc;
/*N*/   pPoints[nPos].X() = (long) (fU * pPoints[nIdx  ].X() +
/*N*/                               fT * pPoints[nIdx+1].X());
/*N*/   pPoints[nPos].Y() = (long) (fU * pPoints[nIdx  ].Y() +
/*N*/                               fT * pPoints[nIdx+1].Y());
/*N*/ }

/************************************************************************/

/*N*/ void XPolygon::GenBezArc(const Point& rCenter, long nRx, long nRy,
/*N*/                        long nXHdl, long nYHdl, USHORT nStart, USHORT nEnd,
/*N*/                        USHORT nQuad, USHORT nFirst)
/*N*/ {
/*N*/   Point* pPoints = pImpXPolygon->pPointAry;
/*N*/   pPoints[nFirst  ] = rCenter;
/*N*/   pPoints[nFirst+3] = rCenter;
/*N*/
/*N*/   if ( nQuad == 1 || nQuad == 2 )
/*N*/   {
/*N*/       nRx   = -nRx; nXHdl = -nXHdl;
/*N*/   }
/*N*/   if ( nQuad == 0 || nQuad == 1 )
/*N*/   {
/*N*/       nRy   = -nRy; nYHdl = -nYHdl;
/*N*/   }
/*N*/
/*N*/   if ( nQuad == 0 || nQuad == 2 )
/*N*/   {
/*N*/       pPoints[nFirst].X() += nRx; pPoints[nFirst+3].Y() += nRy;
/*N*/   }
/*N*/   else
/*N*/   {
/*N*/       pPoints[nFirst].Y() += nRy; pPoints[nFirst+3].X() += nRx;
/*N*/   }
/*N*/   pPoints[nFirst+1] = pPoints[nFirst];
/*N*/   pPoints[nFirst+2] = pPoints[nFirst+3];
/*N*/
/*N*/   if ( nQuad == 0 || nQuad == 2 )
/*N*/   {
/*N*/       pPoints[nFirst+1].Y() += nYHdl; pPoints[nFirst+2].X() += nXHdl;
/*N*/   }
/*N*/   else
/*N*/   {
/*N*/       pPoints[nFirst+1].X() += nXHdl; pPoints[nFirst+2].Y() += nYHdl;
/*N*/   }
/*N*/   if ( nStart > 0 )
/*N*/       SubdivideBezier(nFirst, FALSE, (double)nStart / 900);
/*N*/   if ( nEnd < 900 )
/*N*/       SubdivideBezier(nFirst, TRUE, (double)(nEnd-nStart) / (900-nStart));
/*N*/   SetFlags(nFirst+1, XPOLY_CONTROL);
/*N*/   SetFlags(nFirst+2, XPOLY_CONTROL);
/*N*/ }

/************************************************************************/

/*N*/ BOOL XPolygon::CheckAngles(USHORT& nStart, USHORT nEnd, USHORT& nA1, USHORT& nA2)
/*N*/ {
/*N*/   if ( nStart == 3600 ) nStart = 0;
/*N*/   if ( nEnd == 0 ) nEnd = 3600;
/*N*/   USHORT nStPrev = nStart;
/*N*/   USHORT nMax = (nStart / 900 + 1) * 900;
/*N*/   USHORT nMin = nMax - 900;
/*N*/
/*N*/   if ( nEnd >= nMax || nEnd <= nStart )   nA2 = 900;
/*N*/   else                                    nA2 = nEnd - nMin;
/*N*/   nA1 = nStart - nMin;
/*N*/   nStart = nMax;
/*N*/
/*N*/   // TRUE zurueck, falls letztes Segment berechnet wurde
/*N*/   return (nStPrev < nEnd && nStart >= nEnd);
/*N*/ }

/*************************************************************************
|*
|*    XPolygon::Translate()
|*
|*    Polygon auf den uebergebenen Punkt verschieben
|*
*************************************************************************/

/*N*/ void XPolygon::Translate(const Point& rTrans)
/*N*/ {
/*N*/   pImpXPolygon->CheckPointDelete();
/*N*/   CheckReference();
/*N*/
/*N*/   USHORT nPntCnt = pImpXPolygon->nPoints;
/*N*/
/*N*/   for (USHORT i = 0; i < nPntCnt; i++)
/*N*/       pImpXPolygon->pPointAry[i] += rTrans;
/*N*/ }

/*************************************************************************
|*
|*    XPolygon::Rotate()
|*
|*    Alle Punkte um den Punkt rCenter drehen, Sinus und Cosinus
|*    muessen uebergeben werden
|*
*************************************************************************/

/*N*/ void XPolygon::Rotate(const Point& rCenter, double fSin, double fCos)
/*N*/ {
/*N*/   pImpXPolygon->CheckPointDelete();
/*N*/   CheckReference();
/*N*/
/*N*/   long nX;
/*N*/   long nY;
/*N*/   long nNewX;
/*N*/   long nNewY;
/*N*/   long nCenterX = rCenter.X();
/*N*/   long nCenterY = rCenter.Y();
/*N*/
/*N*/   USHORT nPntCnt = pImpXPolygon->nPoints;
/*N*/
/*N*/   for (USHORT i = 0; i < nPntCnt; i++)
/*N*/   {
/*N*/       Point *pPt = &(pImpXPolygon->pPointAry[i]);
/*N*/       nX = pPt->X()-nCenterX;
/*N*/       nY = pPt->Y()-nCenterY;
/*N*/       nNewX =  (long)floor(fCos * nX + fSin * nY + 0.5);
/*N*/       nNewY = -(long)floor(fSin * nX - fCos * nY + 0.5);
/*N*/       pPt->X() = nNewX + nCenterX;
/*N*/       pPt->Y() = nNewY + nCenterY;
/*N*/   }
/*N*/ }

/*************************************************************************
|*
|*    XPolygon::Rotate()
|*
|*    Alle Punkte um den Punkt rCenter mit dem Winkel nAngle drehen
|*    Winkel in 10tel Grad, Wertebereich 0 - 3600
|*
*************************************************************************/

/*N*/ void XPolygon::Rotate(const Point& rCenter, USHORT nAngle)
/*N*/ {
/*N*/   nAngle %= 3600;
/*N*/
/*N*/   if ( nAngle != 0 )
/*N*/   {
/*N*/       double fAngle = F_PI * nAngle / 1800;
/*N*/       double fSin = sin(fAngle);
/*N*/       double fCos = cos(fAngle);
/*N*/       Rotate(rCenter, fSin, fCos);
/*N*/   }
/*N*/ }

/*************************************************************************
|*
|*    XPolygon::Scale()
|*
|*    XPolygon in X- und/oder Y-Richtung skalieren
|*
*************************************************************************/

/*N*/ void XPolygon::Scale(double fSx, double fSy)
/*N*/ {
/*N*/   pImpXPolygon->CheckPointDelete();
/*N*/   CheckReference();
/*N*/
/*N*/   USHORT nPntCnt = pImpXPolygon->nPoints;
/*N*/
/*N*/   for (USHORT i = 0; i < nPntCnt; i++)
/*N*/   {
/*N*/       Point& rPnt = pImpXPolygon->pPointAry[i];
/*N*/       rPnt.X() = (long)(fSx * rPnt.X());
/*N*/       rPnt.Y() = (long)(fSy * rPnt.Y());
/*N*/   }
/*N*/ }

/*************************************************************************
|*
|*    XPolygon::SlantX()
|*
|*    XPolygon in X-Richtung um einen beliebigen Winkel kippen,
|*    bezogen auf eine Referenz-Y-Koordinate
|*
*************************************************************************/

/*N*/ void XPolygon::SlantX(long nYRef, double fSin, double fCos)
/*N*/ {
/*N*/   pImpXPolygon->CheckPointDelete();
/*N*/   CheckReference();
/*N*/
/*N*/   USHORT nPntCnt = pImpXPolygon->nPoints;
/*N*/
/*N*/   for (USHORT i = 0; i < nPntCnt; i++)
/*N*/   {
/*N*/       Point& rPnt = pImpXPolygon->pPointAry[i];
/*N*/       long nDy = rPnt.Y() - nYRef;
/*N*/       rPnt.X() += (long)(fSin * nDy);
/*N*/       rPnt.Y() = nYRef + (long)(fCos * nDy);
/*N*/   }
/*N*/ }

/*************************************************************************
|*
|*    XPolygon::operator>>()
|*
|*    Beschreibung      Stream-Leseoperator
|*
*************************************************************************/

/*N*/ SvStream& operator>>( SvStream& rIStream, XPolygon& rXPoly )
/*N*/ {
/*N*/   DBG_CHKOBJ( &rXPoly, XPolygon, NULL );
/*N*/
/*N*/   USHORT          i;
/*N*/   USHORT          nReadPoints; // Anzahl der Punkte im Stream
/*N*/   USHORT          nMerkPoints; // Anzahl der Punkte die ich speichern kann
/*N*/
/*N*/   rXPoly.pImpXPolygon->CheckPointDelete();
/*N*/
/*N*/   // Anzahl der Punkte einlesen und Array erzeugen
/*N*/   rIStream >> nReadPoints;
/*N*/   nMerkPoints=nReadPoints;
/*N*/   if (nMerkPoints>XPOLY_MAXPOINTS) {
/*?*/       nMerkPoints=XPOLY_MAXPOINTS;
/*N*/       // hier koennte man ein Flag am Stream setzen um zu vermerken
/*N*/       // dass beim Lesen Informationsverlusst aufgetreten ist !!!!!
/*N*/   }
/*N*/   rXPoly.pImpXPolygon->nPoints = nMerkPoints;
/*N*/
/*N*/   if ( rXPoly.pImpXPolygon->nRefCount != 1 )
/*N*/   {
/*?*/       if ( rXPoly.pImpXPolygon->nRefCount )
/*?*/           rXPoly.pImpXPolygon->nRefCount--;
/*?*/       rXPoly.pImpXPolygon = new ImpXPolygon( nMerkPoints );
/*N*/   }
/*N*/   else
/*N*/       rXPoly.pImpXPolygon->Resize( nMerkPoints );
/*N*/
/*N*/   {
        // Feststellen, ob ueber die Operatoren gelesen werden muss
#if (__SIZEOFLONG) != 4
        if ( 1 )
#else
#ifdef OSL_BIGENDIAN
/*N*/       if ( rIStream.GetNumberFormatInt() != NUMBERFORMAT_INT_BIGENDIAN )
#else
/*N*/       if ( rIStream.GetNumberFormatInt() != NUMBERFORMAT_INT_LITTLEENDIAN )
#endif
#endif
/*N*/       {
/*?*/           for( i = 0; i < nReadPoints; i++ ) {
/*?*/               long x,y;
/*?*/               rIStream >> x >> y;
/*?*/               if (i<nMerkPoints) { // restliche Punkte ueberspringen
/*?*/                   rXPoly.pImpXPolygon->pPointAry[i].X()=x;
/*?*/                   rXPoly.pImpXPolygon->pPointAry[i].Y()=y;
/*?*/               }
/*?*/           }
/*N*/       } else {
/*N*/           rIStream.Read( rXPoly.pImpXPolygon->pPointAry, nMerkPoints*sizeof(Point) );
/*N*/           if (nReadPoints>nMerkPoints) { // restliche Punkte ueberspringen
/*?*/               rIStream.SeekRel( ULONG(nReadPoints-nMerkPoints)*sizeof(Point) );
/*N*/           }
/*N*/       }
/*N*/   }
/*N*/   rIStream.Read( rXPoly.pImpXPolygon->pFlagAry, nMerkPoints );
/*N*/   if (nReadPoints>nMerkPoints) { // Flags der restlichen Punkte ueberspringen
/*?*/       rIStream.SeekRel( ULONG(nReadPoints-nMerkPoints) );
/*N*/       // Poly muesste hier noch etwas korregiert werden (Bezier-Kontrollpunkte am Ende..., geschlossen?)
/*N*/   }
/*N*/   while (rXPoly.GetPointCount()>0 && rXPoly.GetFlags(USHORT(rXPoly.GetPointCount()-1))==XPOLY_CONTROL) {
        // Kontrollpunkte am Ende entfernen (kann auftreten bei truncate wg. 64k-Grenze!)
/*?*/       rXPoly.Remove(USHORT(rXPoly.GetPointCount()-1),1);
/*N*/   }
/*N*/
/*N*/   return rIStream;
/*N*/ }

/*************************************************************************
|*
|*    XPolygon::operator<<()
|*
|*    Beschreibung      Stream-Schreiboperator
|*
*************************************************************************/

/*N*/ SvStream& operator<<( SvStream& rOStream, const XPolygon& rXPoly )
/*N*/ {
/*N*/   DBG_CHKOBJ( &rXPoly, XPolygon, NULL );
/*N*/
/*N*/   USHORT          i;
/*N*/   USHORT          nPoints = rXPoly.GetPointCount();
/*N*/
/*N*/   rXPoly.pImpXPolygon->CheckPointDelete();
/*N*/
/*N*/   // Anzahl der Punkte rausschreiben
/*N*/   rOStream << nPoints;
/*N*/
/*N*/   {
        // Feststellen, ob ueber die Operatoren geschrieben werden muss
#if (__SIZEOFLONG) != 4
/*N*/       if ( 1 )
#else
#ifdef OSL_BIGENDIAN
/*N*/       if ( rOStream.GetNumberFormatInt() != NUMBERFORMAT_INT_BIGENDIAN )
#else
/*N*/       if ( rOStream.GetNumberFormatInt() != NUMBERFORMAT_INT_LITTLEENDIAN )
#endif
#endif
/*N*/       {
/*?*/           for( i = 0; i < nPoints; i++ )
/*?*/               rOStream << rXPoly.pImpXPolygon->pPointAry[i].X()
/*?*/                        << rXPoly.pImpXPolygon->pPointAry[i].Y();
/*N*/       }
/*N*/       else if ( nPoints )
/*N*/           rOStream.Write( rXPoly.pImpXPolygon->pPointAry, nPoints*sizeof(Point) );
/*N*/   }
/*N*/
/*N*/   if ( nPoints )
/*N*/       rOStream.Write( rXPoly.pImpXPolygon->pFlagAry, nPoints );
/*N*/
/*N*/   return rOStream;
/*N*/ }


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+--------------- XPolyPolygon -----------------------------------------+
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*************************************************************************
|*
|*    ImpXPolyPolygon::ImpXPolyPolygon()
|*
|*    Beschreibung      Erzeugt das XPolygon-Array
|*
*************************************************************************/

ImpXPolyPolygon::ImpXPolyPolygon( const ImpXPolyPolygon& rImpXPolyPoly )
    : aXPolyList( rImpXPolyPoly.aXPolyList )
{
    nRefCount = 1;
    for ( size_t i = 0, n = aXPolyList.size(); i < n; ++i )
        aXPolyList[ i ] = new XPolygon( *aXPolyList[ i ] );
}


/*************************************************************************
|*
|*    ImpXPolyPolygon::~ImpXPolyPolygon()
|*
|*    Beschreibung      Loescht das Polygon-Array
|*
*************************************************************************/

ImpXPolyPolygon::~ImpXPolyPolygon()
{
    for (size_t i = 0, n = aXPolyList.size(); i < n; ++i )
        delete aXPolyList[ i ];
    aXPolyList.clear();
}

/*************************************************************************
|*
|*    XPolyPolygon::XPolyPolygon()
|*
*************************************************************************/

XPolyPolygon::XPolyPolygon( USHORT /* nInitSize */, USHORT /* nResize */ )
{
    DBG_CTOR(XPolyPolygon,NULL);
    pImpXPolyPolygon = new ImpXPolyPolygon();
}


/*************************************************************************
|*
|*    XPolyPolygon::XPolyPolygon()
|*
*************************************************************************/

/*N*/ XPolyPolygon::XPolyPolygon( const XPolygon& rXPoly )
/*N*/ {
/*N*/   DBG_CTOR(XPolyPolygon,NULL);
/*N*/   pImpXPolyPolygon = new ImpXPolyPolygon;
/*N*/   pImpXPolyPolygon->aXPolyList.push_back( new XPolygon( rXPoly ) );
/*N*/ }

/*************************************************************************
|*
|*    XPolyPolygon::XPolyPolygon()
|*
*************************************************************************/

/*N*/ XPolyPolygon::XPolyPolygon( const XPolyPolygon& rXPolyPoly )
/*N*/ {
/*N*/   DBG_CTOR(XPolyPolygon,NULL);
/*N*/   pImpXPolyPolygon = rXPolyPoly.pImpXPolyPolygon;
/*N*/   pImpXPolyPolygon->nRefCount++;
/*N*/ }

/*************************************************************************
|*
|*    XPolyPolygon::XPolyPolygon()
|*
|*    XPolyPolygon aus einen Standard-PolyPolygon erzeugen
|*
*************************************************************************/

/*N*/ XPolyPolygon::XPolyPolygon( const PolyPolygon& rPolyPoly )
/*N*/ {
/*N*/   DBG_CTOR(XPolyPolygon,NULL);
/*N*/   pImpXPolyPolygon = new ImpXPolyPolygon;
/*N*/
/*N*/   for ( size_t i = 0; i < rPolyPoly.Count(); i++ )
/*N*/       pImpXPolyPolygon->aXPolyList.push_back( new XPolygon( rPolyPoly[ i ] ) );
/*N*/ }

/*************************************************************************
|*
|*    XPolyPolygon::~XPolyPolygon()
|*
*************************************************************************/

/*N*/ XPolyPolygon::~XPolyPolygon()
/*N*/ {
/*N*/   DBG_DTOR(XPolyPolygon,NULL);
/*N*/   if( pImpXPolyPolygon->nRefCount > 1 )
/*N*/       pImpXPolyPolygon->nRefCount--;
/*N*/   else
/*N*/       delete pImpXPolyPolygon;
/*N*/ }

/*************************************************************************
|*
|*    XPolygon::CheckReference()
|*
|*    Referenzzaehler desImpXPolyPoly pruefen und ggf. von diesem abkoppeln
|*
*************************************************************************/

/*N*/ void XPolyPolygon::CheckReference()
/*N*/ {
/*N*/   if( pImpXPolyPolygon->nRefCount > 1 )
/*N*/   {
/*N*/       pImpXPolyPolygon->nRefCount--;
/*N*/       pImpXPolyPolygon = new ImpXPolyPolygon( *pImpXPolyPolygon );
/*N*/   }
/*N*/ }

/*************************************************************************
|*
|*    XPolyPolygon::Insert()
|*
*************************************************************************/

void XPolyPolygon::Insert( const XPolygon& rXPoly, USHORT nPos )
{
    CheckReference();
    XPolygon* pXPoly = new XPolygon( rXPoly );
    if ( nPos < pImpXPolyPolygon->aXPolyList.size() )
    {
        XPolygonList::iterator it = pImpXPolyPolygon->aXPolyList.begin();
        ::std::advance( it, nPos );
        pImpXPolyPolygon->aXPolyList.insert( it, pXPoly );
    }
    else
        pImpXPolyPolygon->aXPolyList.push_back( pXPoly );
}

/*************************************************************************
|*
|*    XPolyPolygon::GetObject()
|*
*************************************************************************/

/*N*/ const XPolygon& XPolyPolygon::GetObject( USHORT nPos ) const
/*N*/ {
/*N*/   return *( pImpXPolyPolygon->aXPolyList[ nPos ] );
/*N*/ }


/*************************************************************************
|*
|*    XPolyPolygon::Clear()
|*
*************************************************************************/

/*N*/ void XPolyPolygon::Clear()
/*N*/ {
/*N*/   if ( pImpXPolyPolygon->nRefCount > 1 )
/*N*/   {
/*N*/       pImpXPolyPolygon->nRefCount--;
/*N*/       pImpXPolyPolygon = new ImpXPolyPolygon();
/*N*/   }
/*N*/   else
/*N*/   {
            for ( size_t i = 0, n = pImpXPolyPolygon->aXPolyList.size(); i < n; ++i )
                delete pImpXPolyPolygon->aXPolyList[ i ];
            pImpXPolyPolygon->aXPolyList.clear();
/*N*/   }
/*N*/ }


/*************************************************************************
|*
|*    XPolyPolygon::Count()
|*
*************************************************************************/

/*N*/ USHORT XPolyPolygon::Count() const
/*N*/ {
/*N*/   return (USHORT)(pImpXPolyPolygon->aXPolyList.size());
/*N*/ }


/*************************************************************************
|*
|*    XPolyPolygon::Move()
|*
*************************************************************************/

void XPolyPolygon::Move( long nHorzMove, long nVertMove )
{
    // Diese Abfrage sollte man fuer die DrawEngine durchfuehren
    if ( !nHorzMove && !nVertMove )
        return;

    // Referenzcounter beruecksichtigen
    CheckReference();

    // Punkte verschieben
    for ( size_t i = 0, n = pImpXPolyPolygon->aXPolyList.size(); i < n; ++i )
    {
        XPolygon* pXPoly = pImpXPolyPolygon->aXPolyList[ i ];
        pXPoly->Move( nHorzMove, nVertMove );
    }
}

/*************************************************************************
|*
|*    XPolyPolygon::GetBoundRect()
|*
*************************************************************************/

/*N*/ Rectangle XPolyPolygon::GetBoundRect(OutputDevice* pOut) const
/*N*/ {
/*N*/   size_t nXPoly = pImpXPolyPolygon->aXPolyList.size();
/*N*/   Rectangle aRect;
/*N*/
/*N*/   for ( size_t n = 0; n < nXPoly; n++ )
/*N*/   {
/*N*/       const XPolygon* pXPoly = pImpXPolyPolygon->aXPolyList[ n ];
/*N*/       aRect.Union( pXPoly->GetBoundRect(pOut) );
/*N*/   }
/*N*/
/*N*/   return aRect;
/*N*/ }


/*************************************************************************
|*
|*    XPolyPolygon::operator[]()
|*
*************************************************************************/

/*N*/ XPolygon& XPolyPolygon::operator[]( USHORT nPos )
/*N*/ {
/*N*/   CheckReference();
/*N*/   return *( pImpXPolyPolygon->aXPolyList[ nPos ] );
/*N*/ }

/*************************************************************************
|*
|*    XPolyPolygon::operator=()
|*
*************************************************************************/

/*N*/ XPolyPolygon& XPolyPolygon::operator=( const XPolyPolygon& rXPolyPoly )
/*N*/ {
/*N*/   rXPolyPoly.pImpXPolyPolygon->nRefCount++;
/*N*/
/*N*/   if( pImpXPolyPolygon->nRefCount > 1 )
/*N*/       pImpXPolyPolygon->nRefCount--;
/*N*/   else
/*N*/       delete pImpXPolyPolygon;
/*N*/
/*N*/   pImpXPolyPolygon = rXPolyPoly.pImpXPolyPolygon;
/*N*/   return *this;
/*N*/ }

/*************************************************************************
|*
|*    XPolyPolygon::Translate()
|*
|*    Alle Polygone auf den uebergebenen Punkt verschieben
|*
*************************************************************************/

/*N*/ void XPolyPolygon::Translate(const Point& rTrans)
/*N*/ {
/*N*/   CheckReference();
/*N*/
/*N*/   for (USHORT i = 0; i < Count(); i++)
/*N*/       pImpXPolyPolygon->aXPolyList[ i ]->Translate(rTrans);
/*N*/ }

/*************************************************************************
|*
|*    XPolyPolygon::Rotate()
|*
|*    Alle Polygone um den Punkt rCenter drehen, Sinus und Cosinus
|*    muessen uebergeben werden
|*
*************************************************************************/

/*N*/ void XPolyPolygon::Rotate(const Point& rCenter, double fSin, double fCos)
/*N*/ {
/*N*/   CheckReference();
/*N*/
/*N*/   for (USHORT i = 0; i < Count(); i++)
/*N*/       pImpXPolyPolygon->aXPolyList[ i ]->Rotate(rCenter, fSin, fCos);
/*N*/ }

/*************************************************************************
|*
|* Bestimme den linken, unteren Punkt des Polygons und richte das
|* Polygon so aus, dass dieser Punkt auf dem Index 0 liegt
|*
\************************************************************************/


/*************************************************************************
|*
|*    XPolyPolygon::Scale()
|*
|*    Alle Polygone in X- und/oder Y-Richtung skalieren
|*
*************************************************************************/

/*N*/ void XPolyPolygon::Scale(double fSx, double fSy)
/*N*/ {
/*N*/   CheckReference();
/*N*/
/*N*/   for (USHORT i = 0; i < Count(); i++)
/*N*/       pImpXPolyPolygon->aXPolyList[ i ]->Scale(fSx, fSy);
/*N*/ }

/*************************************************************************
|*
|*    XPolyPolygon::SlantX()
|*
|*    Alle Polygone in X-Richtung um einen beliebigen Winkel kippen,
|*    bezogen auf eine Referenz-Y-Koordinate
|*
*************************************************************************/

/*N*/ void XPolyPolygon::SlantX(long nYRef, double fSin, double fCos)
/*N*/ {
/*N*/   CheckReference();
/*N*/
/*N*/   for (USHORT i = 0; i < Count(); i++)
/*N*/       pImpXPolyPolygon->aXPolyList[ i ]->SlantX(nYRef, fSin, fCos);
/*N*/ }

/*************************************************************************
|*
|*    XPolyPolygon::operator>>()
|*
|*    Beschreibung      Stream-Leseoperator
|*
*************************************************************************/

/*N*/ SvStream& operator>>( SvStream& rIStream, XPolyPolygon& rXPolyPoly )
/*N*/ {
/*N*/   DBG_CHKOBJ( &rXPolyPoly, XPolyPolygon, NULL );
/*N*/
/*N*/   XPolygon* pXPoly;
/*N*/
/*N*/   // Anzahl der Polygone einlesen
/*N*/   USHORT nXPolyCount;
/*N*/   rIStream >> nXPolyCount;
/*N*/
/*N*/   bool bTruncated=FALSE;
/*N*/   ULONG nAllPointCount=0; // Gesamtanzahl der Punkte mitzaehlen
/*N*/
/*N*/   if ( rXPolyPoly.pImpXPolyPolygon->nRefCount > 1 ) {
/*?*/       rXPolyPoly.pImpXPolyPolygon->nRefCount--;
/*N*/   } else {
/*N*/       delete rXPolyPoly.pImpXPolyPolygon;
/*N*/   }
/*N*/   rXPolyPoly.pImpXPolyPolygon = new ImpXPolyPolygon();
/*N*/
/*N*/   while (nXPolyCount>0) {
/*N*/       pXPoly = new XPolygon;
/*N*/       rIStream >> *pXPoly;
/*N*/       nAllPointCount+=pXPoly->GetPointCount();
/*N*/       if (!bTruncated) {
/*N*/           if (nAllPointCount>XPOLY_MAXPOINTS) {
/*?*/               USHORT nDel=(USHORT)(nAllPointCount-XPOLY_MAXPOINTS);
/*?*/               USHORT nPos=pXPoly->GetPointCount()-nDel;
/*?*/               pXPoly->Remove(nPos,nDel);
/*?*/               bTruncated=TRUE; // Alle nachfolgenden Polygone werden ignoriert
/*N*/           }
/*N*/           rXPolyPoly.pImpXPolyPolygon->aXPolyList.push_back( pXPoly );
/*N*/       } else {
/*?*/           delete pXPoly;
/*N*/       }
/*N*/       nXPolyCount--;
/*N*/   }

/*N*/   return rIStream;
/*N*/ }

/*************************************************************************
|*
|*    XPolyPolygon::operator<<()
|*
|*    Beschreibung      Stream-Schreiboperator
|*
*************************************************************************/

SvStream& operator<<( SvStream& rOStream, const XPolyPolygon& rXPolyPoly )
{
    DBG_CHKOBJ( &rXPolyPoly, XPolyPolygon, NULL );

    // Anzahl der Polygone rausschreiben
    rOStream << rXPolyPoly.Count();

    // Die einzelnen Polygone ausgeben
    for ( size_t i = 0, n = rXPolyPoly.Count(); i < n; ++i )
    {
        XPolygon* pXPoly = rXPolyPoly.pImpXPolyPolygon->aXPolyList[ i ];
        rOStream << *pXPoly;
    }
    return rOStream;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
