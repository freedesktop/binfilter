/*************************************************************************
 *
 *  $RCSfile: sw_xmlitmpr.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-03 18:26:09 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// auto strip #ifndef _RTL_USTRING_HXX_
// auto strip #include <rtl/ustring.hxx>
// auto strip #endif

#ifndef _XMLITMAP_HXX
#include "xmlitmap.hxx"
#endif
namespace binfilter {

using namespace rtl;
using ::binfilter::xmloff::token::IsXMLToken;
using ::binfilter::xmloff::token::XML_TOKEN_INVALID;

// TODO: optimize this!

class SvXMLItemMapEntries_impl
{
public:
    SvXMLItemMapEntry* mpEntries;
    sal_uInt16 mnCount;
};

SvXMLItemMapEntries::SvXMLItemMapEntries( SvXMLItemMapEntry* pEntries )
{
    mpImpl = new SvXMLItemMapEntries_impl;
    mpImpl->mpEntries = pEntries;

    mpImpl->mnCount = 0;
    while( pEntries->eLocalName != XML_TOKEN_INVALID )
    {
        pEntries++;
        mpImpl->mnCount++;
    }
}

SvXMLItemMapEntries::~SvXMLItemMapEntries()
{
    delete mpImpl;
}

SvXMLItemMapEntry* SvXMLItemMapEntries::getByName(  sal_uInt16 nNameSpace,
                                                    const OUString& rString,
                                                    SvXMLItemMapEntry* pStartAt /* = NULL */  ) const
{
    SvXMLItemMapEntry* pMap =
        (pStartAt && (pStartAt->eLocalName!=XML_TOKEN_INVALID)) ?
                                           &(pStartAt[1]) : mpImpl->mpEntries;
    while( pMap && (pMap->eLocalName != XML_TOKEN_INVALID) )
    {
        if( pMap->nNameSpace == nNameSpace &&
            IsXMLToken( rString, pMap->eLocalName ) )
            break;
        pMap++;
    }

    return (pMap->eLocalName != XML_TOKEN_INVALID) ? pMap : NULL;
}

SvXMLItemMapEntry* SvXMLItemMapEntries::getByIndex( UINT16 nIndex ) const
{
    return &mpImpl->mpEntries[nIndex];
}

UINT16 SvXMLItemMapEntries::getCount() const
{
    return mpImpl->mnCount;
}

}
