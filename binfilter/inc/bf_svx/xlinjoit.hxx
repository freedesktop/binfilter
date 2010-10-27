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

#ifndef _SVX_XLINJOIT_HXX
#define _SVX_XLINJOIT_HXX

#include <bf_svtools/eitem.hxx>
#include <bf_svx/xenum.hxx>
namespace binfilter {

//---------------------
// class LineStyleItem
//---------------------

class XLineJointItem : public SfxEnumItem
{
public:
                            TYPEINFO();
                            XLineJointItem( XLineJoint eLineJoint = XLINEJOINT_ROUND );
                            XLineJointItem( SvStream& rIn );

    virtual USHORT           GetVersion( USHORT nFileFormatVersion ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxPoolItem*    Create( SvStream& rIn, USHORT nVer ) const;

    virtual bool            QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual bool            PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    virtual USHORT          GetValueCount() const;
    XLineJoint              GetValue() const { return (XLineJoint) SfxEnumItem::GetValue(); }
};

}//end of namespace binfilter
#endif // _SVX_XLINJOIT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
