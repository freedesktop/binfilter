/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <fontcfg.hxx>
#include <vcl/outdev.hxx>
#include <bf_svtools/lingucfg.hxx>



// #107253#
#include <swlinguconfig.hxx>
namespace binfilter {

using namespace utl;
using namespace ::com::sun::star::uno;

using ::rtl::OUString;

/*N*/ #define C2S(cChar) String::CreateFromAscii(cChar)
/*N*/ #define C2U(cChar) OUString::createFromAscii(cChar)


/*M*/ inline LanguageType lcl_LanguageOfType(sal_Int16 nType, sal_Int16 eWestern, sal_Int16 eCJK, sal_Int16 eCTL)
/*M*/ {
/*M*/     return LanguageType(
/*M*/                 nType < FONT_STANDARD_CJK ? eWestern :
/*N*/                     nType >= FONT_STANDARD_CTL ? eCTL : eCJK);
/*M*/ }


/*N*/ Sequence<OUString> SwStdFontConfig::GetPropertyNames()
/*N*/ {
/*N*/   static const char* aPropNames[] =
/*N*/   {
/*N*/         "DefaultFont/Standard",    // 0
/*N*/         "DefaultFont/Heading",     // 1
/*N*/         "DefaultFont/List",        // 2
/*N*/         "DefaultFont/Caption",     // 3
/*N*/         "DefaultFont/Index",       // 4
/*N*/         "DefaultFontCJK/Standard", // 5
/*N*/         "DefaultFontCJK/Heading",  // 6
/*N*/         "DefaultFontCJK/List",     // 7
/*N*/         "DefaultFontCJK/Caption",  // 8
/*N*/         "DefaultFontCJK/Index",    // 9
/*N*/         "DefaultFontCTL/Standard", // 10
/*N*/         "DefaultFontCTL/Heading",  // 11
/*N*/         "DefaultFontCTL/List",     // 12
/*N*/         "DefaultFontCTL/Caption",  // 13
/*N*/         "DefaultFontCTL/Index",    // 14
/*N*/     };
/*N*/     const int nCount = 15;
/*N*/   Sequence<OUString> aNames(nCount);
/*N*/   OUString* pNames = aNames.getArray();
/*N*/   for(int i = 0; i < nCount; i++)
/*N*/   {
/*N*/       pNames[i] = OUString::createFromAscii(aPropNames[i]);
/*N*/   }
/*N*/   return aNames;
/*N*/ }



/*N*/ SwStdFontConfig::SwStdFontConfig() :
/*N*/     ::utl::ConfigItem(C2U("Office.Writer"))
/*N*/ {
/*N*/     SvtLinguOptions aLinguOpt;
/*N*/
/*N*/   // #107253# Replaced SvtLinguConfig with SwLinguConfig wrapper with UsageCount
/*N*/     SwLinguConfig().GetOptions( aLinguOpt );
/*N*/
/*N*/   sal_Int16   eWestern = aLinguOpt.nDefaultLanguage,
/*N*/                 eCJK = aLinguOpt.nDefaultLanguage_CJK,
/*N*/                 eCTL = aLinguOpt.nDefaultLanguage_CTL;
/*N*/     for(sal_Int16 i = 0; i < DEF_FONT_COUNT; i++)
/*N*/         sDefaultFonts[i] = GetDefaultFor(i,
/*N*/             lcl_LanguageOfType(i, eWestern, eCJK, eCTL));
/*N*/
/*N*/     Sequence<OUString> aNames = GetPropertyNames();
/*N*/   Sequence<Any> aValues = GetProperties(aNames);
/*N*/   const Any* pValues = aValues.getConstArray();
/*N*/   DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
/*N*/   if(aValues.getLength() == aNames.getLength())
/*N*/   {
/*N*/       for(int nProp = 0; nProp < aNames.getLength(); nProp++)
/*N*/       {
/*N*/           if(pValues[nProp].hasValue())
/*N*/           {
/*N*/               OUString sVal;
/*N*/               pValues[nProp] >>= sVal;
/*N*/                 sDefaultFonts[nProp] = sVal;
/*N*/             }
/*N*/       }
/*N*/   }
/*N*/ }




/*N*/ SwStdFontConfig::~SwStdFontConfig()
/*N*/ {}


/*M*/ BOOL SwStdFontConfig::IsFontDefault(USHORT nFontType) const
/*M*/ {
/*M*/   BOOL bSame = TRUE;
/*M*/     SvtLinguOptions aLinguOpt;
/*N*/
/*N*/   // #107253# Replaced SvtLinguConfig with SwLinguConfig wrapper with UsageCount
/*N*/     SwLinguConfig().GetOptions( aLinguOpt );
/*N*/
/*M*/     sal_Int16   eWestern = aLinguOpt.nDefaultLanguage,
/*M*/                 eCJK = aLinguOpt.nDefaultLanguage_CJK,
/*M*/                 eCTL = aLinguOpt.nDefaultLanguage_CTL;
/*M*/     String sDefFont(GetDefaultFor(FONT_STANDARD, eWestern));
/*M*/     String sDefFontCJK(GetDefaultFor(FONT_STANDARD_CJK, eCJK));
/*M*/     String sDefFontCTL(GetDefaultFor(FONT_STANDARD_CTL, eCTL));
/*M*/     LanguageType eLang = lcl_LanguageOfType(nFontType, eWestern, eCJK, eCTL);
/*M*/     switch( nFontType )
/*M*/   {
/*M*/       case FONT_STANDARD:
/*M*/             bSame = sDefaultFonts[nFontType] == sDefFont;
/*M*/       break;
/*M*/         case FONT_STANDARD_CJK:
/*M*/             bSame = sDefaultFonts[nFontType] == sDefFontCJK;
/*M*/       break;
/*M*/         case FONT_STANDARD_CTL:
/*M*/             bSame = sDefaultFonts[nFontType] == sDefFontCTL;
/*M*/       break;
/*M*/         case FONT_OUTLINE :
/*M*/         case FONT_OUTLINE_CJK :
/*M*/         case FONT_OUTLINE_CTL :
/*M*/             bSame = sDefaultFonts[nFontType] ==
/*M*/                 GetDefaultFor(nFontType, eLang);
/*M*/         break;
/*M*/         case FONT_LIST    :
/*M*/         case FONT_CAPTION :
/*M*/         case FONT_INDEX   :
/*M*/             bSame = sDefaultFonts[nFontType] == sDefFont &&
/*M*/                     sDefaultFonts[FONT_STANDARD] == sDefFont;
/*M*/       break;
/*M*/         case FONT_LIST_CJK    :
/*M*/         case FONT_CAPTION_CJK :
/*M*/         case FONT_INDEX_CJK   :
/*M*/       {
/*M*/             BOOL b1 = sDefaultFonts[FONT_STANDARD_CJK] == sDefFontCJK;
/*M*/             bSame = b1 && sDefaultFonts[nFontType] == sDefFontCJK;
/*M*/       }
/*M*/       break;
/*M*/         case FONT_LIST_CTL    :
/*M*/         case FONT_CAPTION_CTL :
/*M*/         case FONT_INDEX_CTL   :
/*M*/       {
/*M*/             BOOL b1 = sDefaultFonts[FONT_STANDARD_CJK] == sDefFontCTL;
/*M*/             bSame = b1 && sDefaultFonts[nFontType] == sDefFontCTL;
/*M*/       }
/*M*/       break;
/*M*/     }
/*M*/   return bSame;
/*M*/ }

/* --------------------------------------------------
 * Standards auslesen
 * --------------------------------------------------*/
/*N*/ String  SwStdFontConfig::GetDefaultFor(USHORT nFontType, LanguageType eLang)
/*N*/ {
/*N*/     String sRet;
/*N*/     USHORT nFontId;
/*N*/     switch( nFontType )
/*N*/   {
/*N*/         case FONT_OUTLINE :
/*N*/             nFontId = DEFAULTFONT_LATIN_HEADING;
/*N*/         break;
/*N*/         case FONT_OUTLINE_CJK :
/*N*/             nFontId = DEFAULTFONT_CJK_HEADING;
/*N*/         break;
/*N*/         case FONT_OUTLINE_CTL :
/*N*/             nFontId = DEFAULTFONT_CTL_HEADING;
/*N*/         break;
/*N*/         case FONT_STANDARD_CJK:
/*N*/         case FONT_LIST_CJK    :
/*N*/         case FONT_CAPTION_CJK :
/*N*/         case FONT_INDEX_CJK   :
/*N*/             nFontId = DEFAULTFONT_CJK_TEXT;
/*N*/         break;
/*N*/         case FONT_STANDARD_CTL:
/*N*/         case FONT_LIST_CTL    :
/*N*/         case FONT_CAPTION_CTL :
/*N*/         case FONT_INDEX_CTL   :
/*N*/             nFontId = DEFAULTFONT_CTL_TEXT;
/*N*/         break;
/*N*/ //        case FONT_STANDARD:
/*N*/ //        case FONT_LIST    :
/*N*/ //        case FONT_CAPTION :
/*N*/ //        case FONT_INDEX   :
/*N*/         default:
/*N*/             nFontId = DEFAULTFONT_LATIN_TEXT;
/*N*/     }
/*M*/     Font aFont = OutputDevice::GetDefaultFont(nFontId, eLang, DEFAULTFONT_FLAGS_ONLYONE);
/*M*/     return  aFont.GetName();
/*N*/ }

    void SwStdFontConfig::Commit() {}
    void SwStdFontConfig::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& ) {}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
