/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmtfordr.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2006-10-28 04:43:14 $
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
#ifndef _FMTFORDR_HXX
#define _FMTFORDR_HXX

#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _FORMAT_HXX //autogen
#include <format.hxx>
#endif
namespace binfilter {

//Die FillOrder ---------------------------------

enum SwFillOrder
{
    SW_FILL_ORDER_BEGIN,
    ATT_TOP_DOWN = SW_FILL_ORDER_BEGIN,
    ATT_BOTTOM_UP,
    ATT_LEFT_TO_RIGHT,
    ATT_RIGHT_TO_LEFT,
    SW_FILL_ORDER_END
};

class SwFmtFillOrder: public SfxEnumItem
{
public:
    SwFmtFillOrder( SwFillOrder = ATT_TOP_DOWN );
    inline SwFmtFillOrder &operator=( const SwFmtFillOrder &rCpy );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, USHORT nVer) const;
    virtual SvStream&       Store(SvStream &, USHORT nIVer) const;
    virtual USHORT          GetValueCount() const{DBG_BF_ASSERT(0, "STRIP"); return 0;} //STRIP001 virtual USHORT           GetValueCount() const;

    const SwFillOrder GetFillOrder() const { return SwFillOrder(GetValue()); }
    void  SetFillOrder( const SwFillOrder eNew ) { SetValue( USHORT(eNew) ); }
};

inline SwFmtFillOrder &SwFmtFillOrder::operator=( const SwFmtFillOrder &rCpy )
{
    SetValue( rCpy.GetValue() );
    return *this;
}

#if !(defined(MACOSX) && ( __GNUC__ < 3 ))
// GrP moved to gcc_outl.cxx; revisit with gcc3
inline const SwFmtFillOrder &SwAttrSet::GetFillOrder(BOOL bInP) const
    { return (const SwFmtFillOrder&)Get( RES_FILL_ORDER,bInP); }

inline const SwFmtFillOrder &SwFmt::GetFillOrder(BOOL bInP) const
    { return aSet.GetFillOrder(bInP); }
#endif

} //namespace binfilter
#endif

