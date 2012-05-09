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

#ifndef _PARALIST_HXX
#define _PARALIST_HXX

#include <bf_svtools/bf_solar.h>
#include <tools/link.hxx>
#include <vector>
#include <limits.h>

namespace binfilter {
class Paragraph;

class ParagraphList
{
private:
    Link            aVisibleStateChangedHdl;
    std::vector< Paragraph* > aList;

public:
    void            Clear( BOOL bDestroyParagraphs );

    ULONG           GetParagraphCount() const           { return aList.size(); }
    Paragraph*      GetParagraph( ULONG nPos ) const    { return aList[ nPos ]; }

    ULONG           GetAbsPos( Paragraph* pParent ) const {
                        for( size_t i = 0, n = aList.size(); i < n; ++i ) {
                            if ( pParent == aList[ i ] ) {
                                return i;
                            }
                        }
                        return ULONG_MAX;
                    }

    void            Insert( Paragraph* pPara, ULONG nAbsPos = ULONG_MAX ) {
                        if ( nAbsPos < aList.size() ) {
                            std::vector< Paragraph* >::iterator it = aList.begin();
                            std::advance( it, nAbsPos );
                            aList.insert( it, pPara );
                        } else {
                            aList.push_back( pPara );
                        }
                    }

    void            Remove( ULONG nPara ) {
                        if ( nPara < aList.size() ) {
                            std::vector< Paragraph* >::iterator it = aList.begin();
                            std::advance( it, nPara );
                            aList.erase( it );
                        }
                    }


/*NBFF*/    Paragraph*      GetParent( Paragraph* pParagraph, USHORT& rRelPos ) const;


    void            SetVisibleStateChangedHdl( const Link& rLink ) { aVisibleStateChangedHdl = rLink; }
    Link            GetVisibleStateChangedHdl() const { return aVisibleStateChangedHdl; }
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
