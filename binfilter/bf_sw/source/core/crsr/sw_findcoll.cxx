/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sw_findcoll.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2006-10-27 22:18:41 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


#pragma hdrstop

#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif

#ifndef _SWCRSR_HXX
#include <swcrsr.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

namespace binfilter {


//------------------ Methoden der CrsrShell ---------------------------

// Parameter fuer das Suchen vom FormatCollections






// Suchen nach Format-Collections


/*M*/ ULONG SwCursor::Find( const SwTxtFmtColl& rFmtColl,
/*M*/                     SwDocPositions nStart, SwDocPositions nEnde, BOOL& bCancel,
/*M*/                   FindRanges eFndRngs, const SwTxtFmtColl* pReplFmtColl )
/*M*/ {DBG_BF_ASSERT(0, "STRIP");return 0;//STRIP001
/*M*/   // OLE-Benachrichtigung abschalten !!
/*M*/ }



}
