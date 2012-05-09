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

#ifndef _B3D_B3DTEX_HXX
#define _B3D_B3DTEX_HXX

#include <bf_svtools/bf_solar.h>


#include <tools/color.hxx>
#include <vcl/salbtype.hxx>

namespace binfilter {

/*************************************************************************
|*
|* define for lifetime of a texture in texture cache.
|* Parameter of Time(...) call, so hrs, min, sec, 100thsec.
|* Timer for cache uses ten secs delays
|*
\************************************************************************/

#define B3D_TEXTURE_LIFETIME                0, 1, 0

/*************************************************************************
|*
|* Long-Zeiger fuer OpenGL Texturdatenuebergabe
|*
\************************************************************************/

#ifdef WIN
typedef UINT8 huge* GL_UINT8;
#else
typedef UINT8*      GL_UINT8;
#endif

/*************************************************************************
|*
|* Art der Pixeldaten der Textur
|*
\************************************************************************/

enum Base3DTextureKind
{
    Base3DTextureLuminance = 1,
    Base3DTextureIntensity,
    Base3DTextureColor
};

/*************************************************************************
|*
|* Modus der Textur
|*
\************************************************************************/

enum Base3DTextureMode
{
    Base3DTextureReplace = 1,
    Base3DTextureModulate,
    Base3DTextureBlend
};

/*************************************************************************
|*
|* Filtermodus der Textur
|*
\************************************************************************/

enum Base3DTextureFilter
{
    Base3DTextureNearest = 1,
    Base3DTextureLinear
};

/*************************************************************************
|*
|* Wrapping-Modus
|*
\************************************************************************/

enum Base3DTextureWrap
{
    Base3DTextureClamp = 1,
    Base3DTextureRepeat,
    Base3DTextureSingle
};

/*************************************************************************
|*
|* Defines fuer Maskenbildung um Entscheidung innerhalb von ModifyColor
|* zu beschleunigen
|*
\************************************************************************/

#define B3D_TXT_KIND_LUM        0x00
#define B3D_TXT_KIND_INT        0x01
#define B3D_TXT_KIND_COL        0x02

#define B3D_TXT_MODE_REP        0x04
#define B3D_TXT_MODE_MOD        0x08
#define B3D_TXT_MODE_BND        0x0C

#define B3D_TXT_FLTR_NEA        0x10

/*************************************************************************
|*
|* Klassen fuer TexturAttribute beim Anfordern von Texturen
|*
\************************************************************************/

#define TEXTURE_ATTRIBUTE_TYPE_COLOR            0x0000
#define TEXTURE_ATTRIBUTE_TYPE_BITMAP           0x0001
#define TEXTURE_ATTRIBUTE_TYPE_GRADIENT         0x0002
#define TEXTURE_ATTRIBUTE_TYPE_HATCH            0x0003

}//end of namespace binfilter

#endif          // _B3D_B3DTEX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
