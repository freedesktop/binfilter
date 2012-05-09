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

#include <bf_svtools/itemiter.hxx>
#include <bf_svtools/whiter.hxx>


#include "shellio.hxx"
#include "wrt_fn.hxx"

#include "node.hxx"
namespace binfilter {

/*N*/ Writer& Out( const SwNodeFnTab pTab, SwNode& rNode, Writer & rWrt )
/*N*/ {
/*N*/   // es muss ein CntntNode sein !!
/*N*/   SwCntntNode * pCNd = rNode.GetCntntNode();
/*N*/   if( !pCNd )
/*?*/       return rWrt;
/*N*/
/*N*/   USHORT nId = RES_TXTNODE;
/*N*/   switch( pCNd->GetNodeType() )
/*N*/   {
/*N*/   case ND_TEXTNODE:   nId = RES_TXTNODE;  break;
/*?*/   case ND_GRFNODE:    nId = RES_GRFNODE;  break;
/*?*/   case ND_OLENODE:    nId = RES_OLENODE;  break;
/*?*/   default:
/*?*/       OSL_ENSURE( FALSE, "was fuer ein Node ist es denn nun?" );
/*N*/   }
/*N*/   FnNodeOut pOut;
/*N*/   if( 0 != ( pOut = pTab[ nId - RES_NODE_BEGIN ] ))
/*N*/       (*pOut)( rWrt, *pCNd );
/*N*/   return rWrt;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
