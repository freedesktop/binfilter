/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sw_initui.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 07:03:34 $
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

// auto strip #ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
// auto strip #include <unotools/localedatawrapper.hxx>
// auto strip #endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif
// auto strip #ifndef _INITUI_HXX
// auto strip #include <initui.hxx>
// auto strip #endif
#ifndef _EDTWIN_HXX
#include <edtwin.hxx>
#endif
#ifndef _SHELLRES_HXX
#include <shellres.hxx>
#endif
// auto strip #ifndef _FLDBAS_HXX
// auto strip #include <fldbas.hxx>
// auto strip #endif
#ifndef _GLOSDOC_HXX
#include <glosdoc.hxx>
#endif
#ifndef _GLOSLST_HXX
#include <gloslst.hxx>
#endif

#ifndef _UTLUI_HRC
#include <utlui.hrc>
#endif
#ifndef _INITUI_HRC
#include <initui.hrc>
#endif
#ifndef _COMCORE_HRC
#include <comcore.hrc>
#endif
#ifndef _AUTHFLD_HXX
#include <authfld.hxx>
#endif
#ifndef _DBMGR_HXX
#include <dbmgr.hxx>
#endif
#ifndef _SWTYPES_HXX
#include "swtypes.hxx"
#endif
namespace binfilter {

#define C2S(cChar) UniString::CreateFromAscii(cChar)
/*--------------------------------------------------------------------
    Beschreibung:   globale Pointer
 --------------------------------------------------------------------*/

SwGlossaries*       pGlossaries = 0;

// Liefert alle benoetigten Pfade. Wird durch UI initialisiert
SwGlossaryList*     pGlossaryList = 0;

String* pOldGrfCat = 0;
String* pOldTabCat = 0;
String* pOldFrmCat = 0;
String* pOldDrwCat = 0;
String* pCurrGlosGroup = 0;

SvStringsDtor* pDBNameList = 0;

SvStringsDtor*  pAuthFieldNameList = 0;
SvStringsDtor*  pAuthFieldTypeList = 0;

/*--------------------------------------------------------------------
    Beschreibung:   UI beenden
 --------------------------------------------------------------------*/

/*N*/ void _FinitUI()
/*N*/ {
/*N*/   SwNewDBMgr::RemoveDbtoolsClient();
/*N*/   delete ViewShell::GetShellRes();
/*N*/   ViewShell::SetShellRes( 0 );
/*N*/
/*N*/   SwEditWin::_FinitStaticData();
/*N*/
/*N*/   DELETEZ(pGlossaries);
/*N*/
/*N*/   delete SwFieldType::pFldNames;
/*N*/
/*N*/   delete pOldGrfCat;
/*N*/   delete pOldTabCat;
/*N*/   delete pOldFrmCat;
/*N*/   delete pOldDrwCat;
/*N*/   delete pCurrGlosGroup;
/*N*/   delete pDBNameList;
/*N*/   delete pGlossaryList;
/*N*/   delete pAuthFieldNameList;
/*N*/   delete pAuthFieldTypeList;
/*N*/
/*N*/
/*N*/ }
/*--------------------------------------------------------------------
    Beschreibung:   Initialisierung
 --------------------------------------------------------------------*/


/*N*/ void _InitUI()
/*N*/ {
/*N*/   // ShellResource gibt der CORE die Moeglichkeit mit Resourcen zu arbeiten
/*N*/   ViewShell::SetShellRes( new ShellResource );
/*N*/   pDBNameList = new SvStringsDtor( 5, 5 );
/*N*/   SwEditWin::_InitStaticData();
/*N*/ }


/*N*/ ShellResource::ShellResource()
/*N*/   : Resource( SW_RES(RID_SW_SHELLRES) ),
/*N*/   aPostItPage( SW_RES( STR_POSTIT_PAGE ) ),
/*N*/   aPostItAuthor( SW_RES( STR_POSTIT_AUTHOR ) ),
/*N*/   aPostItLine( SW_RES( STR_POSTIT_LINE ) ),
/*N*/   aCalc_Syntax( SW_RES( STR_CALC_SYNTAX ) ),
/*N*/   aCalc_ZeroDiv( SW_RES( STR_CALC_ZERODIV ) ),
/*N*/   aCalc_Brack( SW_RES( STR_CALC_BRACK ) ),
/*N*/   aCalc_Pow( SW_RES( STR_CALC_POW ) ),
/*N*/   aCalc_VarNFnd( SW_RES( STR_CALC_VARNFND ) ),
/*N*/   aCalc_Overflow( SW_RES( STR_CALC_OVERFLOW ) ),
/*N*/   aCalc_WrongTime( SW_RES( STR_CALC_WRONGTIME ) ),
/*N*/   aCalc_Default( SW_RES( STR_CALC_DEFAULT ) ),
/*N*/   aCalc_Error( SW_RES( STR_CALC_ERROR ) ),
/*N*/   aGetRefFld_Up( SW_RES( STR_GETREFFLD_UP ) ),
/*N*/   aGetRefFld_Down( SW_RES( STR_GETREFFLD_DOWN ) ),
/*N*/   aStrAllPageHeadFoot( SW_RES( STR_ALLPAGE_HEADFOOT ) ),
/*N*/   aStrNone( SW_RES( STR_TEMPLATE_NONE )),
/*N*/   aFixedStr( SW_RES( STR_FIELD_FIXED )),
/*N*/   aTOXIndexName(          SW_RES(STR_TOI)),
/*N*/   aTOXUserName(           SW_RES(STR_TOU)),
/*N*/   aTOXContentName(        SW_RES(STR_TOC)),
/*N*/   aTOXIllustrationsName(  SW_RES(STR_TOX_ILL)),
/*N*/   aTOXObjectsName(        SW_RES(STR_TOX_OBJ)),
/*N*/   aTOXTablesName(         SW_RES(STR_TOX_TBL)),
/*N*/   aTOXAuthoritiesName(    SW_RES(STR_TOX_AUTH)),
/*N*/   sPageDescFirstName(     SW_RES(STR_PAGEDESC_FIRSTNAME)),
/*N*/   sPageDescFollowName(    SW_RES(STR_PAGEDESC_FOLLOWNAME)),
/*N*/   sPageDescName(          SW_RES(STR_PAGEDESC_NAME)),
/*N*/   pAutoFmtNameLst( 0 )
/*N*/ {
/*N*/   const USHORT nCount = FLD_DOCINFO_END - FLD_DOCINFO_BEGIN;
/*N*/
/*N*/   for(USHORT i = 0; i < nCount; ++i)
/*N*/   {
/*N*/       String* pNew = new SW_RESSTR(FLD_DOCINFO_BEGIN + i);
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

//STRIP001 String ShellResource::GetPageDescName( USHORT nNo, BOOL bIsFirst, BOOL bFollow )
//STRIP001 {
//STRIP001  String sRet( bIsFirst ? sPageDescFirstName
//STRIP001                        : bFollow ? sPageDescFollowName
//STRIP001                                  : sPageDescName );
//STRIP001  sRet.SearchAndReplaceAscii( "$(ARG1)", String::CreateFromInt32( nNo ));
//STRIP001  return sRet;
//STRIP001 }


/*N*/ SwGlossaries* GetGlossaries()
/*N*/ {
/*N*/   if (!pGlossaries)
/*N*/       pGlossaries = new SwGlossaries;
/*N*/   return (pGlossaries);
/*N*/ }

/*N*/ BOOL HasGlossaryList()
/*N*/ {
/*N*/   return pGlossaryList != 0;
/*N*/ }

/*N*/ SwGlossaryList* GetGlossaryList()
/*N*/ {
/*N*/   if(!pGlossaryList)
/*N*/       pGlossaryList = new SwGlossaryList();
/*N*/
/*N*/   return pGlossaryList;
/*N*/ }

//STRIP001 struct ImpAutoFmtNameListLoader : public Resource
//STRIP001 {
//STRIP001  ImpAutoFmtNameListLoader( SvStringsDtor& rLst );
//STRIP001 };

//STRIP001 void ShellResource::_GetAutoFmtNameLst() const
//STRIP001 {
//STRIP001  SvStringsDtor** ppLst = (SvStringsDtor**)&pAutoFmtNameLst;
//STRIP001  *ppLst = new SvStringsDtor( STR_AUTOFMTREDL_END );
//STRIP001  ImpAutoFmtNameListLoader aTmp( **ppLst );
//STRIP001 }

//STRIP001 ImpAutoFmtNameListLoader::ImpAutoFmtNameListLoader( SvStringsDtor& rLst )
//STRIP001  : Resource( ResId(RID_SHELLRES_AUTOFMTSTRS, pSwResMgr) )
//STRIP001 {
//STRIP001  for( USHORT n = 0; n < STR_AUTOFMTREDL_END; ++n )
//STRIP001  {
//STRIP001      String* p = new String( ResId( n + 1, pSwResMgr) );
//STRIP001      if(STR_AUTOFMTREDL_TYPO == n)
//STRIP001      {
//STRIP001          LocaleDataWrapper& rLclD = GetAppLocaleData();
//STRIP001 #ifdef WNT
//STRIP001          //fuer Windows Sonderbehandlung, da MS hier ein paar Zeichen im Dialogfont vergessen hat
//STRIP001          p->SearchAndReplace(C2S("%1"), C2S(",,"));
//STRIP001          p->SearchAndReplace(C2S("%2"), C2S("''"));
//STRIP001 #else
//STRIP001          //unter richtigen Betriebssystemen funktioniert es auch so
//STRIP001          p->SearchAndReplace(C2S("%1"), rLclD.getDoubleQuotationMarkStart());
//STRIP001          p->SearchAndReplace(C2S("%2"), rLclD.getDoubleQuotationMarkEnd());
//STRIP001 #endif
//STRIP001      }
//STRIP001      rLst.Insert( p, n );
//STRIP001  }
//STRIP001  FreeResource();
//STRIP001 }
/* -----------------16.09.99 12:28-------------------

 --------------------------------------------------*/
//STRIP001 const String&    SwAuthorityFieldType::GetAuthFieldName(ToxAuthorityField eType)
//STRIP001 {
//STRIP001  if(!pAuthFieldNameList)
//STRIP001  {
//STRIP001      pAuthFieldNameList = new SvStringsDtor(AUTH_FIELD_END, 1);
//STRIP001      for(USHORT i = 0; i < AUTH_FIELD_END; i++)
//STRIP001      {
//STRIP001          String*  pTmp = new String(SW_RES(STR_AUTH_FIELD_START + i));
//STRIP001          pAuthFieldNameList->Insert(pTmp, pAuthFieldNameList->Count());
//STRIP001      }
//STRIP001  }
//STRIP001  return *pAuthFieldNameList->GetObject(eType);
//STRIP001 }
/* -----------------16.09.99 12:29-------------------

 --------------------------------------------------*/
 const String&  SwAuthorityFieldType::GetAuthTypeName(ToxAuthorityType eType)
 {
    if(!pAuthFieldTypeList)
    {
        pAuthFieldTypeList = new SvStringsDtor(AUTH_TYPE_END, 1);
        for(USHORT i = 0; i < AUTH_TYPE_END; i++)
            pAuthFieldTypeList->Insert(
                new String(SW_RES(STR_AUTH_TYPE_START + i)),
                                    pAuthFieldTypeList->Count());
    }
    return *pAuthFieldTypeList->GetObject(eType);
 }



}
