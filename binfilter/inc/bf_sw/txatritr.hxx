/*************************************************************************
 *
 *  $RCSfile: txatritr.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hjs $ $Date: 2003-10-01 12:23:53 $
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
#ifndef _TXATRITR_HXX
#define _TXATRITR_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _SVARRAY_HXX
#include <svtools/svarray.hxx>
#endif
#include <tools/debug.hxx>
class String;
class SwTxtNode;
class SwTxtAttr;
class SfxPoolItem;


class SwScriptIterator
{
//STRIP001  const String& rText;
//STRIP001  xub_StrLen nChgPos;
//STRIP001  sal_uInt16 nCurScript;
//STRIP001     sal_Bool bForward;

public:
    SwScriptIterator( const String& rStr, xub_StrLen nStart = 0,
                      sal_Bool bFrwrd = sal_True ){DBG_ASSERT(0, "STRIP");};

//STRIP001  sal_Bool Next();

//STRIP001  sal_uInt16 GetCurrScript() const        { return nCurScript; }
//STRIP001  xub_StrLen GetScriptChgPos() const      { return nChgPos; }
//STRIP001  const String& GetText() const           { return rText; }
};


class SwTxtAttrIterator
{
//STRIP001  SwScriptIterator aSIter;
//STRIP001  SvPtrarr aStack;
//STRIP001  const SwTxtNode& rTxtNd;
    const SfxPoolItem *pParaItem, *pCurItem;
//STRIP001  xub_StrLen nChgPos;
//STRIP001  sal_uInt16 nAttrPos, nWhichId;
//STRIP001
//STRIP001  void AddToStack( const SwTxtAttr& rAttr );
//STRIP001  void SearchNextChg();

public:
SwTxtAttrIterator( const SwTxtNode& rTxtNd, USHORT nWhichId,//STRIP001  SwTxtAttrIterator( const SwTxtNode& rTxtNd, USHORT nWhichId,
xub_StrLen nStart = 0 ){DBG_ASSERT(0, "STRIP");} ;//STRIP001                        xub_StrLen nStart = 0 );
//STRIP001
//STRIP001  sal_Bool Next();
//STRIP001
    const SfxPoolItem& GetAttr() const  { return *pCurItem; }
//STRIP001  xub_StrLen GetChgPos() const        { return nChgPos; }
};


#ifdef ITEMID_LANGUAGE

class SwLanguageIterator : public SwTxtAttrIterator
{
public:
    SwLanguageIterator( const SwTxtNode& rTxtNd, xub_StrLen nStart = 0 )
        : SwTxtAttrIterator( rTxtNd, RES_CHRATR_LANGUAGE, nStart )
    {}

    sal_uInt16 GetLanguage() const
        { return ((SvxLanguageItem&)GetAttr()).GetValue(); }
};

#endif


#endif
