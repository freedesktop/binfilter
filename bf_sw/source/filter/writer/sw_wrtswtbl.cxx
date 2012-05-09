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

#include <hintids.hxx>


#include <wrtswtbl.hxx>
#include <osl/diagnose.h>
#include <swtable.hxx>
#include <frmfmt.hxx>
#include <fmtfsize.hxx>

#include <osl/diagnose.h>

namespace binfilter {

SwWriteTableCol::SwWriteTableCol( USHORT nPosition )
    : nPos(nPosition)
    , nWidthOpt( 0 )
    , bRelWidthOpt( FALSE )
    , bOutWidth( TRUE )
    , bLeftBorder(TRUE)
    , bRightBorder(TRUE)
{
}

long SwWriteTable::GetBoxWidth( const SwTableBox *pBox )
{
    const SwFrmFmt *pFmt = pBox->GetFrmFmt();
    const SwFmtFrmSize& aFrmSize=
         (const SwFmtFrmSize&)pFmt->GetAttr( RES_FRM_SIZE );
    return aFrmSize.GetSize().Width();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
