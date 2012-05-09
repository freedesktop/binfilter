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

#include <viewsh.hxx>
#include <edtwin.hxx>
#include <shellres.hxx>

#include <utlui.hrc>
#include <initui.hrc>
#include <comcore.hrc>
#include <authfld.hxx>
#include <dbmgr.hxx>
#include "swtypes.hxx"
namespace binfilter {

#define C2S(cChar) UniString::CreateFromAscii(cChar)
/*--------------------------------------------------------------------
    Beschreibung:   globale Pointer
 --------------------------------------------------------------------*/

String* pOldGrfCat = 0;
String* pOldTabCat = 0;
String* pOldFrmCat = 0;
String* pOldDrwCat = 0;

SvStringsDtor* pDBNameList = 0;

SvStringsDtor*  pAuthFieldNameList = 0;
SvStringsDtor*  pAuthFieldTypeList = 0;

/*--------------------------------------------------------------------
    Beschreibung:   UI beenden
 --------------------------------------------------------------------*/

/*N*/ ShellResource::ShellResource()
/*N*/   : Resource( SW_RES(RID_SW_SHELLRES) ),
/*N*/   aPostItAuthor( SW_RESSTR( STR_POSTIT_AUTHOR ) ),
/*N*/   aPostItPage( SW_RESSTR( STR_POSTIT_PAGE ) ),
/*N*/   aPostItLine( SW_RESSTR( STR_POSTIT_LINE ) ),
/*N*/   aCalc_Syntax( SW_RESSTR( STR_CALC_SYNTAX ) ),
/*N*/   aCalc_ZeroDiv( SW_RESSTR( STR_CALC_ZERODIV ) ),
/*N*/   aCalc_Brack( SW_RESSTR( STR_CALC_BRACK ) ),
/*N*/   aCalc_Pow( SW_RESSTR( STR_CALC_POW ) ),
/*N*/   aCalc_VarNFnd( SW_RESSTR( STR_CALC_VARNFND ) ),
/*N*/   aCalc_Overflow( SW_RESSTR( STR_CALC_OVERFLOW ) ),
/*N*/   aCalc_WrongTime( SW_RESSTR( STR_CALC_WRONGTIME ) ),
/*N*/   aCalc_Default( SW_RESSTR( STR_CALC_DEFAULT ) ),
/*N*/   aCalc_Error( SW_RESSTR( STR_CALC_ERROR ) ),
/*N*/   aGetRefFld_Up( SW_RESSTR( STR_GETREFFLD_UP ) ),
/*N*/   aGetRefFld_Down( SW_RESSTR( STR_GETREFFLD_DOWN ) ),
/*N*/   aStrAllPageHeadFoot( SW_RESSTR( STR_ALLPAGE_HEADFOOT ) ),
/*N*/   aStrNone( SW_RESSTR( STR_TEMPLATE_NONE )),
/*N*/   aFixedStr( SW_RESSTR( STR_FIELD_FIXED )),
/*N*/   aTOXIndexName(          SW_RESSTR(STR_TOI)),
/*N*/   aTOXUserName(           SW_RESSTR(STR_TOU)),
/*N*/   aTOXContentName(        SW_RESSTR(STR_TOC)),
/*N*/   aTOXIllustrationsName(  SW_RESSTR(STR_TOX_ILL)),
/*N*/   aTOXObjectsName(        SW_RESSTR(STR_TOX_OBJ)),
/*N*/   aTOXTablesName(         SW_RESSTR(STR_TOX_TBL)),
/*N*/   aTOXAuthoritiesName(    SW_RESSTR(STR_TOX_AUTH)),
/*N*/   pAutoFmtNameLst( 0 ),
/*N*/   sPageDescFirstName(     SW_RESSTR(STR_PAGEDESC_FIRSTNAME)),
/*N*/   sPageDescFollowName(    SW_RESSTR(STR_PAGEDESC_FOLLOWNAME)),
/*N*/   sPageDescName(          SW_RESSTR(STR_PAGEDESC_NAME))
/*N*/ {
/*N*/   const USHORT nCount = FLD_DOCINFO_END - FLD_DOCINFO_BEGIN;
/*N*/
/*N*/   for(USHORT i = 0; i < nCount; ++i)
/*N*/   {
/*N*/       String* pNew = new String(SW_RESSTR(FLD_DOCINFO_BEGIN + i));
/*N*/       aDocInfoLst.Insert(pNew, aDocInfoLst.Count());
/*N*/   }
/*N*/
/*N*/   FreeResource();
/*N*/ }

/*N*/ ShellResource::~ShellResource()
/*N*/ {
/*N*/   if( pAutoFmtNameLst )
/*?*/       delete pAutoFmtNameLst, pAutoFmtNameLst = 0;
/*N*/ }

 const String&  SwAuthorityFieldType::GetAuthTypeName(ToxAuthorityType eType)
 {
    if(!pAuthFieldTypeList)
    {
        pAuthFieldTypeList = new SvStringsDtor(AUTH_TYPE_END, 1);
        for(USHORT i = 0; i < AUTH_TYPE_END; i++)
            pAuthFieldTypeList->Insert(
                new String(SW_RESSTR(STR_AUTH_TYPE_START + i)),
                                    pAuthFieldTypeList->Count());
    }
    return *pAuthFieldTypeList->GetObject(eType);
 }



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
