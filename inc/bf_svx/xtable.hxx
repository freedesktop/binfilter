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
#ifndef _XTABLE_HXX
#define _XTABLE_HXX

#include <bf_svtools/bf_solar.h>

// include ---------------------------------------------------------------

#include <bf_svx/xpoly.hxx>
#include <bf_svx/xdash.hxx>
#include <bf_svx/xhatch.hxx>
#include <bf_svx/xgrad.hxx>
#include <bf_svx/xbitmap.hxx>
#include <bf_svx/xflasit.hxx>
#include <bf_svx/xlnasit.hxx>

#include <tools/color.hxx>

#include <bf_tools/string.hxx>

class Color;
class Bitmap;
class VirtualDevice;
namespace binfilter {

class XOutdevItemPool;
class XOutputDevice;

// Breite und Hoehe der LB-Bitmaps
#define BITMAP_WIDTH  32
#define BITMAP_HEIGHT 12

// Standard-Vergleichsstring
extern sal_Unicode pszStandard[]; // "standard"

// Funktion zum Konvertieren in echte RGB-Farben, da mit
// enum COL_NAME nicht verglichen werden kann.
Color RGB_Color( ColorData nColorName );

// ---------------------
// class XPropertyEntry
// ---------------------

class XPropertyEntry
{
protected:
    String  aName;

            XPropertyEntry(const String& rName) : aName(rName) {}
            XPropertyEntry(const XPropertyEntry& rOther): aName(rOther.aName) {}
public:

    virtual        ~XPropertyEntry() {}
    void            SetName(const String& rName)    { aName = rName; }
    String&         GetName()                       { return aName; }
};

// ------------------
// class XColorEntry
// ------------------

class XColorEntry : public XPropertyEntry
{
    Color   aColor;

public:
            XColorEntry(const Color& rColor, const String& rName) :
                XPropertyEntry(rName), aColor(rColor) {}
            XColorEntry(const XColorEntry& rOther) :
                XPropertyEntry(rOther), aColor(rOther.aColor) {}

    void    SetColor(const Color& rColor)   { aColor = rColor; }
    Color&  GetColor()                      { return aColor; }
};

// --------------------
// class XLineEndEntry
// --------------------

class XLineEndEntry : public XPropertyEntry
{
    XPolygon    aXPolygon;

public:
                XLineEndEntry(const XPolygon& rXPolygon, const String& rName) :
                    XPropertyEntry(rName), aXPolygon(rXPolygon) {}
                XLineEndEntry(const XLineEndEntry& rOther) :
                    XPropertyEntry(rOther), aXPolygon(rOther.aXPolygon) {}

    void        SetLineEnd(const XPolygon& rXPolygon) { aXPolygon = rXPolygon; }
    XPolygon&   GetLineEnd()                          { return aXPolygon; }
};

// ------------------
// class XDashEntry
// ------------------

class XDashEntry : public XPropertyEntry
{
    XDash   aDash;

public:
            XDashEntry(const XDash& rDash, const String& rName) :
                XPropertyEntry(rName), aDash(rDash) {}
            XDashEntry(const XDashEntry& rOther) :
                XPropertyEntry(rOther), aDash(rOther.aDash) {}

    void    SetDash(const XDash& rDash)    { aDash = rDash; }
    XDash&  GetDash()                      { return aDash; }
};

// ------------------
// class XHatchEntry
// ------------------

class XHatchEntry : public XPropertyEntry
{
    XHatch  aHatch;

public:
            XHatchEntry(const XHatch& rHatch, const String& rName) :
                XPropertyEntry(rName), aHatch(rHatch) {}
            XHatchEntry(const XHatchEntry& rOther) :
                XPropertyEntry(rOther), aHatch(rOther.aHatch) {}

    void    SetHatch(const XHatch& rHatch)  { aHatch = rHatch; }
    XHatch& GetHatch()                      { return aHatch; }
};

// ---------------------
// class XGradientEntry
// ---------------------

class XGradientEntry : public XPropertyEntry
{
    XGradient  aGradient;

public:
                XGradientEntry(const XGradient& rGradient, const String& rName):
                    XPropertyEntry(rName), aGradient(rGradient) {}
                XGradientEntry(const XGradientEntry& rOther) :
                    XPropertyEntry(rOther), aGradient(rOther.aGradient) {}

    void        SetGradient(const XGradient& rGrad) { aGradient = rGrad; }
    XGradient&  GetGradient()                       { return aGradient; }
};

// ---------------------
// class XBitmapEntry
// ---------------------

class XBitmapEntry : public XPropertyEntry
{
    XOBitmap aXOBitmap;

public:
            XBitmapEntry( const XOBitmap& rXOBitmap, const String& rName ):
                XPropertyEntry( rName ), aXOBitmap( rXOBitmap ) {}
            XBitmapEntry( const XBitmapEntry& rOther ) :
                XPropertyEntry( rOther ), aXOBitmap( rOther.aXOBitmap ) {}

    void     SetXBitmap(const XOBitmap& rXOBitmap) { aXOBitmap = rXOBitmap; }
    XOBitmap& GetXBitmap()                    { return aXOBitmap; }
};

// --------------------
// class XPropertyList
// --------------------

class XPropertyList
{
protected:
    String              aName; // nicht persistent !
    String              aPath;
    XOutdevItemPool*    pXPool;

    List                aList;
    List*               pBmpList;

    BOOL                bListDirty;
    BOOL                bBitmapsDirty;
    BOOL                bOwnPool;

                        XPropertyList(  const String& rPath,
                                        XOutdevItemPool* pXPool = NULL,
                                        USHORT nInitSize = 16,
                                        USHORT nReSize = 16 );
public:
    virtual             ~XPropertyList();

    long                Count() const;

    void                Insert( XPropertyEntry* pEntry, long nIndex = LIST_APPEND );
    XPropertyEntry*     Replace( XPropertyEntry* pEntry, long nIndex );
    XPropertyEntry*     Remove( long nIndex, USHORT nDummy );
    XPropertyEntry*     Get( long nIndex, USHORT nDummy ) const;

    long                Get(const String& rName);

    const String&       GetName() const { return aName; }
    void                SetName( const String& rString );
    const String&       GetPath() const { return aPath; }
    void                SetPath( const String& rString ) { aPath = rString; }
    BOOL                IsDirty() const { return bListDirty && bBitmapsDirty; }
    void                SetDirty( BOOL bDirty = TRUE )
                            { bListDirty = bDirty; bBitmapsDirty = bDirty; }

    virtual BOOL        Load() = 0;
    virtual BOOL        Save() = 0;
    virtual BOOL        Create() = 0;
    virtual BOOL        CreateBitmapsForUI() = 0;
    virtual Bitmap*     CreateBitmapForUI( long nIndex, BOOL bDelete = TRUE ) = 0;
};

// ------------------
// class XColorTable
// ------------------

class XColorTable : public XPropertyList
{
protected:
    SvStream&       ImpRead( SvStream& rIn );

    XubString&      ConvertName( XubString& rStrName );

public:
                    XColorTable( const String& rPath,
                                 XOutdevItemPool* pXPool = NULL,
                                 USHORT nInitSize = 16,
                                 USHORT nReSize = 16 );
    virtual         ~XColorTable();

    XColorEntry*    Replace(long nIndex, XColorEntry* pEntry );
    XColorEntry*    Get(long nIndex) const;

    virtual BOOL    Load();
    virtual BOOL    Save() {return false;}
    virtual BOOL    Create();
    virtual BOOL    CreateBitmapsForUI();
    virtual Bitmap* CreateBitmapForUI( long nIndex, BOOL bDelete = TRUE );
};

// -------------------
// class XLineEndList
// -------------------

class XLineEndList : public XPropertyList
{
protected:
    VirtualDevice*      pVD;
    XOutputDevice*      pXOut;
    XFillAttrSetItem*   pXFSet;
    XLineAttrSetItem*   pXLSet;

    SvStream&       ImpRead( SvStream& rIn );

    XubString&      ConvertName( XubString& rStrName );

public:
                    XLineEndList( const String& rPath,
                                    XOutdevItemPool* pXPool = NULL,
                                    USHORT nInitSize = 16,
                                    USHORT nReSize = 16 );
    virtual         ~XLineEndList();

    XLineEndEntry*  Get(long nIndex) const;

    virtual BOOL    Load();
    virtual BOOL    Save() {return false;}
    virtual BOOL    Create();
    virtual BOOL    CreateBitmapsForUI();
    virtual Bitmap* CreateBitmapForUI( long nIndex, BOOL bDelete = TRUE );
};

// -------------------
// class XDashList
// -------------------

class XDashList : public XPropertyList
{
protected:
    VirtualDevice*      pVD;
    XOutputDevice*      pXOut;
    XFillAttrSetItem*   pXFSet;
    XLineAttrSetItem*   pXLSet;

    SvStream&       ImpRead( SvStream& rIn );

    XubString&      ConvertName( XubString& rStrName );

public:
                    XDashList( const String& rPath,
                                XOutdevItemPool* pXPool = NULL,
                                USHORT nInitSize = 16,
                                USHORT nReSize = 16 );
    virtual         ~XDashList();

    XDashEntry* Get(long nIndex) const;

    virtual BOOL    Load();
    virtual BOOL    Save() {return false;}
    virtual BOOL    Create();
    virtual BOOL    CreateBitmapsForUI();
    virtual Bitmap* CreateBitmapForUI( long nIndex, BOOL bDelete = TRUE );
};

// -------------------
// class XHatchList
// -------------------

class XHatchList : public XPropertyList
{
protected:
    VirtualDevice*      pVD;
    XOutputDevice*      pXOut;
    XFillAttrSetItem*   pXFSet;

    SvStream&       ImpRead( SvStream& rIn );

    XubString&      ConvertName( XubString& rStrName );

public:
                    XHatchList( const String& rPath,
                                XOutdevItemPool* pXPool = NULL,
                                USHORT nInitSize = 16,
                                USHORT nReSize = 16 );
                    ~XHatchList();

    XHatchEntry*    Get(long nIndex) const;

    virtual BOOL    Load();
    virtual BOOL    Save() {return false;}
    virtual BOOL    Create();
    virtual BOOL    CreateBitmapsForUI();
    virtual Bitmap* CreateBitmapForUI( long nIndex, BOOL bDelete = TRUE );
};

// -------------------
// class XGradientList
// -------------------

class XGradientList : public XPropertyList
{
protected:
    VirtualDevice*      pVD;
    XOutputDevice*      pXOut;
    XFillAttrSetItem*   pXFSet;

    SvStream&       ImpRead( SvStream& rIn );

    XubString&      ConvertName( XubString& rStrName );

public:
                    XGradientList( const String& rPath,
                                    XOutdevItemPool* pXPool = NULL,
                                    USHORT nInitSize = 16,
                                    USHORT nReSize = 16 );
    virtual         ~XGradientList();

    XGradientEntry* Get(long nIndex) const;

    virtual BOOL    Load();
    virtual BOOL    Save() {return false;}
    virtual BOOL    Create();
    virtual BOOL    CreateBitmapsForUI();
    virtual Bitmap* CreateBitmapForUI( long nIndex, BOOL bDelete = TRUE );
};

// -------------------
// class XBitmapList
// -------------------

class XBitmapList : public XPropertyList
{
protected:
    SvStream&       ImpRead( SvStream& rIn );

    XubString&      ConvertName( XubString& rStrName );

public:
                    XBitmapList( const String& rPath,
                                    XOutdevItemPool* pXPool = NULL,
                                    USHORT nInitSize = 16,
                                    USHORT nReSize = 16 );
    virtual         ~XBitmapList();

    XBitmapEntry*   Get(long nIndex) const;

    virtual BOOL    Load();
    virtual BOOL    Save() {return false;}
    virtual BOOL    Create();
    virtual BOOL    CreateBitmapsForUI();
    virtual Bitmap* CreateBitmapForUI( long nIndex, BOOL bDelete = TRUE );
};

}//end of namespace binfilter
#endif // _XTABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
