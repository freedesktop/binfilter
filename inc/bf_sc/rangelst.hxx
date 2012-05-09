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

#ifndef SC_RANGELST_HXX
#define SC_RANGELST_HXX

#include <bf_svtools/bf_solar.h>

#include "global.hxx"
#include <vector>

namespace binfilter {

class ScDocument;

typedef ScRange* ScRangePtr;
typedef ::std::vector< ScRangePtr > ScRangeListBase;

class ScRangeList : public SvRefBase
{
private:
    ScRangeListBase maList;
    size_t current;

public:
                    ScRangeList() {}
                    ScRangeList( const ScRangeList& rList );
    virtual         ~ScRangeList();
    ScRangeList&    operator=(const ScRangeList& rList);
    void            RemoveAll();
    USHORT          Parse( const String&, ScDocument* = NULL,
                            USHORT nMask = SCA_VALID );
    void            Format( String&, USHORT nFlags = 0, ScDocument* = NULL ) const;
    void            Join( const ScRange&, BOOL bIsInList = FALSE );
    BOOL            UpdateReference( UpdateRefMode, ScDocument*,
                                    const ScRange& rWhere,
                                    short nDx, short nDy, short nDz );
    BOOL            Load( SvStream&, USHORT nVer );
    BOOL Store( SvStream& ) const {return FALSE;}

    size_t          Count() const { return maList.size(); }
    size_t          GetCurPos() const { return current; }

    size_t          GetPos( ScRange* item )
                    {
                        for ( size_t i = 0, n = maList.size(); i < n; ++i )
                            if ( maList[ i ] == item ) return i;
                        return size_t(-1);
                    }

    void            Seek( size_t i ) { current = i; }

    ScRange*        First()
                    {
                        current = 0;
                        return maList.empty() ? NULL : maList[ current ];
                    }

    ScRange*        Next()
                    {
                        if ( current+1 < maList.size() )
                        {
                            ++current;
                            return maList[ current ];
                        }
                        return NULL;
                    }

    ScRange*        GetObject( size_t i )
                    {
                        if ( i >= maList.size() ) return NULL;
                        return maList[ i ];
                    }

    ScRange*        GetObject( size_t i ) const
                    {
                        if ( i >= maList.size() ) return NULL;
                        return maList[ i ];
                    }

    ScRange*        Remove( size_t i )
                    {
                        if ( i < maList.size() )
                        {
                            ScRangeListBase::iterator it = maList.begin();
                            ::std::advance( it, i );
                            maList.erase( it );
                            current = i;
                        }
                        return NULL;
                    }

    void            Insert( ScRange* item, size_t i )
                    {
                        if ( i < maList.size() )
                        {
                            ScRangeListBase::iterator it = maList.begin();
                            ::std::advance( it, i );
                            maList.insert( it, item );
                            current = i;
                        }
                        else
                        {
                            current = maList.size();
                            maList.push_back( item );
                        }
                    }

    void            Append( const ScRange& rRange )
                    {
                        ScRangePtr pR = new ScRange( rRange );
                        maList.push_back( pR );
                    }

    bool            operator ==( const ScRangeList& rList ) const
                    {
                        if ( maList.size() != rList.Count() ) return false;
                        for ( size_t i = 0, n = maList.size(); i < n; ++i )
                            if ( maList[ i ] != rList.GetObject( i ) ) return false;
                        return true;
                    }

    bool            operator !=( const ScRangeList& rList ) const
                    {
                        return !( *this == rList );
                    }

};
SV_DECL_IMPL_REF( ScRangeList );


// RangePairList: erster Range (aRange[0]) eigentlicher Range, zweiter
// Range (aRange[1]) Daten zu diesem Range, z.B. Rows eines ColName
typedef ::std::vector< ScRangePair* > ScRangePairListBase;
class ScRangePairList : public SvRefBase
{
private:
    ScRangePairListBase maPairList;
    size_t              current;

public:
    virtual         ~ScRangePairList();
    ScRangePairList*    Clone() const;
    void            Append( const ScRangePair& rRangePair )
                    {
                        ScRangePair* pR = new ScRangePair( rRangePair );
                        maPairList.push_back( pR );
                    }
    size_t          Count() const { return maPairList.size(); }
    void            Join( const ScRangePair&, BOOL bIsInList = FALSE );
    BOOL            UpdateReference( UpdateRefMode, ScDocument*,
                                    const ScRange& rWhere,
                                    short nDx, short nDy, short nDz );
    ScRangePair*    Find( const ScRange& ) const;
    BOOL            Load( SvStream&, USHORT nVer );
    BOOL Store( SvStream& ) const {return FALSE;}

    size_t          GetPos( ScRangePair* item )
                    {
                        for ( size_t i = 0, n = maPairList.size(); i < n; ++i )
                            if ( maPairList[ i ] == item ) return i;
                        return size_t(-1);
                    }

    ScRangePair*    Remove( size_t i )
                    {
                        if ( i < maPairList.size() )
                        {
                            ScRangePairListBase::iterator it = maPairList.begin();
                            ::std::advance( it, i );
                            maPairList.erase( it );
                            current = i;
                        }
                        return NULL;
                    }

    ScRangePair*    Remove( ScRangePair* item )
                    {
                        size_t i = GetPos( item );
                        return Remove( i );
                    }

    ScRangePair*    GetObject( size_t i )
                    {
                        if ( i >= maPairList.size() ) return NULL;
                        return maPairList[ i ];
                    }

    ScRangePair*    GetObject( size_t i ) const
                    {
                        if ( i >= maPairList.size() ) return NULL;
                        return maPairList[ i ];
                    }

    void            Seek( size_t i ) { current = i; }

    ScRangePair*    First()
                    {
                        current = 0;
                        return maPairList.empty() ? NULL : maPairList[ current ];
                    }

    ScRangePair*    Next()
                    {
                        if ( current+1 < maPairList.size() )
                        {
                            ++current;
                            return maPairList[ current ];
                        }
                        return NULL;
                    }

};
SV_DECL_IMPL_REF( ScRangePairList );


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
