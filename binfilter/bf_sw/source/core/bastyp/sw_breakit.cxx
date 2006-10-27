/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sw_breakit.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2006-10-27 22:15:21 $
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

#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif
#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif
#ifndef _SVX_LINGU_HXX
#include <bf_svx/unolingu.hxx>
#endif
#ifndef _SVX_SCRIPTTYPEITEM_HXX
#include <bf_svx/scripttypeitem.hxx>
#endif

#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif
#ifndef _LEGACYBINFILTERMGR_HXX
#include <legacysmgr/legacy_binfilters_smgr.hxx>    //STRIP002
#endif

#ifndef _SWTYPES_HXX
#include "swtypes.hxx"
#endif

namespace binfilter {
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;


/*N*/ SwBreakIt::SwBreakIt()
/*N*/   : pLocale( NULL ), pForbidden( NULL )
/*N*/ {
/*N*/   _GetLocale( (LanguageType)GetAppLanguage() );
/*N*/   Reference< XMultiServiceFactory > xMSF = ::legacy_binfilters::getLegacyProcessServiceFactory();
/*N*/   Reference < XInterface > xI = xMSF->createInstance(
/*N*/       ::rtl::OUString::createFromAscii( "com.sun.star.i18n.BreakIterator" ) );
/*N*/   if ( xI.is() )
/*N*/   {
/*N*/       Any x = xI->queryInterface( ::getCppuType((const Reference< XBreakIterator >*)0) );
/*N*/       x >>= xBreak;
/*N*/   }
/*N*/ }

/*N*/ void SwBreakIt::_GetLocale( const LanguageType aLang )
/*N*/ {
/*N*/   aLast = aLang;
/*N*/   delete pLocale;
/*N*/   pLocale = new Locale( SvxCreateLocale( aLast ) );
/*N*/ }

/*N*/ void SwBreakIt::_GetForbidden( const LanguageType aLang )
/*N*/ {
/*N*/   aForbiddenLang = aLang;
/*N*/   Reference< XMultiServiceFactory > xMSF = ::legacy_binfilters::getLegacyProcessServiceFactory();
/*N*/   LocaleDataWrapper aWrap( xMSF, GetLocale( aLang ) );
/*N*/   delete pForbidden;
/*N*/   pForbidden = new ForbiddenCharacters( aWrap.getForbiddenCharacters() );
/*N*/ }

/*N*/ USHORT SwBreakIt::GetRealScriptOfText( const String& rTxt,
/*N*/                                       xub_StrLen nPos ) const
/*N*/ {
/*N*/   USHORT nScript = ScriptType::WEAK;
/*N*/   if( xBreak.is() && rTxt.Len() )
/*N*/   {
/*N*/       if( nPos && nPos == rTxt.Len() )
/*N*/           --nPos;
/*N*/       nScript = xBreak->getScriptType( rTxt, nPos );
/*N*/       sal_Int32 nChgPos;
/*N*/       if( ScriptType::WEAK == nScript && nPos &&
/*N*/           0 < (nChgPos = xBreak->beginOfScript( rTxt, nPos, nScript )) )
/*N*/           nScript = xBreak->getScriptType( rTxt, nChgPos-1 );
/*N*/
/*N*/       if( ScriptType::WEAK == nScript && rTxt.Len() >
/*N*/           ( nChgPos = xBreak->endOfScript( rTxt, nPos, nScript ) ) &&
/*N*/           0 <= nChgPos )
/*N*/           nScript = xBreak->getScriptType( rTxt, nChgPos );
/*N*/   }
/*N*/   if( ScriptType::WEAK == nScript )
/*N*/     nScript = GetI18NScriptTypeOfLanguage( (USHORT)GetAppLanguage() );
/*N*/   return nScript;
/*N*/ }


}
