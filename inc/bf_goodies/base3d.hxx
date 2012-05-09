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

#ifndef _B3D_BASE3D_HXX
#define _B3D_BASE3D_HXX

#include <bf_svtools/bf_solar.h>


#include "hmatrix.hxx"

#ifndef _INC_FLOAT
#include <float.h>
#endif

#include <vcl/timer.hxx>
#include <osl/mutex.hxx>

namespace binfilter {

enum Base3DObjectMode
{

    // die folgenden Primitive muessen von jedem abgeleiteten Renderer
    // ausgefuehrt werden koennen

    Base3DPoints = 0,
    Base3DLines,
    Base3DLineLoop,
    Base3DLineStrip,
    Base3DTriangles,
    Base3DTriangleStrip,
    Base3DTriangleFan,
    Base3DQuads,
    Base3DQuadStrip,
    Base3DPolygon,

    // ab hier beginnen hoehere Primitive, die mit speziellen Methoden
    // innerhalb von Base3D auf die obenstehenden Primitive
    // abgebildet werden. Diese Methoden tun dies alles im object
    // coordinate system.

    // PolyPolygon ohne Selbstueberschneidungen und ohne
    // Ueberschneidungen der evtl. vorhandenen Teilpolygone
    Base3DComplexPolygon,

    // Beliebiges PolyPolygon mit Selbstueberschneidungen und
    // Ueberschneidungen der weiteren Teilpolygone
    Base3DComplexPolygonCut
};

/*************************************************************************
|*
|* Moegliche Darstellungsarten fuer Primitive
|*
\************************************************************************/

enum Base3DRenderMode
{
    Base3DRenderNone = 0,
    Base3DRenderPoint,
    Base3DRenderLine,
    Base3DRenderFill
};

/*************************************************************************
|*
|* Moegliche CullingModes fuer Polygone
|*
\************************************************************************/

enum Base3DCullMode
{
    Base3DCullNone = 0,
    Base3DCullFront,
    Base3DCullBack
};

/*************************************************************************
|*
|* Moegliche ShadeModels fuer Polygone
|*
\************************************************************************/

enum Base3DShadeModel
{
    Base3DSmooth = 0,
    Base3DFlat,
    Base3DPhong
};

/*************************************************************************
|*
|* Unterstuetzung PolygonOffset
|*
\************************************************************************/

enum Base3DPolygonOffset
{
    Base3DPolygonOffsetFill = 1,
    Base3DPolygonOffsetLine,
    Base3DPolygonOffsetPoint
};

/*************************************************************************
|*
|* Genauigkeit der Berechnungen; im Zweifelsfall wird dieser
|* Wert benutzt, um mit 0.0 zu vergleichen oder Abstaende von
|* Randvereichen von Wertebereichen zu testen
|*
\************************************************************************/

#define SMALL_DVALUE                    (0.0000001)

}//end of namespace binfilter

#endif          // _B3D_BASE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
