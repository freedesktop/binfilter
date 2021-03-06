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



#include "usrpref.hxx"

#include <horiornt.hxx>

#include "crstate.hxx"
#include <bf_offmgr/app.hxx>

#include <unotools/localedatawrapper.hxx>
namespace binfilter {

using namespace utl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

using ::rtl::OUString;

/*N*/ #define C2U(cChar) OUString::createFromAscii(cChar)
/*N*/ #define C2S(cChar) String::CreateFromAscii(cChar)


/*N*/ SwMasterUsrPref::SwMasterUsrPref(BOOL bWeb) :
/*N*/   aContentConfig(bWeb, *this),
/*N*/   aLayoutConfig(bWeb, *this),
/*N*/   aGridConfig(bWeb, *this),
/*N*/   aCursorConfig(*this),
/*N*/   pWebColorConfig(bWeb ? new SwWebColorConfig(*this) : 0),
/*N*/   nFldUpdateFlags(0),
/*N*/     nLinkUpdateMode(0),
/*N*/     bIsHScrollMetricSet(sal_False),
/*N*/     bIsVScrollMetricSet(sal_False),
/*N*/     nDefTab( MM50 * 4 )
/*N*/ {
/*N*/     MeasurementSystem eSystem = GetAppLocaleData().getMeasurementSystemEnum();
/*N*/     eUserMetric = MEASURE_METRIC == eSystem ? FUNIT_CM : FUNIT_INCH;
/*N*/     eHScrollMetric = eUserMetric;
/*N*/     eVScrollMetric = eUserMetric;
/*N*/
/*N*/     aContentConfig.Load();
/*N*/   aLayoutConfig.Load();
/*N*/   aGridConfig.Load();
/*N*/   aCursorConfig.Load();
/*N*/   if(pWebColorConfig)
/*N*/       pWebColorConfig->Load();
/*N*/ }


/*N*/ SwMasterUsrPref::~SwMasterUsrPref()
/*N*/ {
/*N*/   delete pWebColorConfig;
/*N*/ }


    void SwContentViewConfig::Commit() {}
    void SwContentViewConfig::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& ) {}

/*N*/ Sequence<OUString> SwContentViewConfig::GetPropertyNames()
/*N*/ {
/*N*/   static const char* aPropNames[] =
/*N*/   {
/*N*/       "Display/GraphicObject",                    //  0
/*N*/       "Display/Table",                            //  1
/*N*/       "Display/DrawingControl",                   //  2
/*N*/       "Display/FieldCode",                        //  3
/*N*/       "Display/Note",                             //  4
/*N*/         "Display/PreventTips",                      //  5
/*N*/         "NonprintingCharacter/MetaCharacters",     //   6
/*N*/         "NonprintingCharacter/ParagraphEnd",        //  7
/*N*/         "NonprintingCharacter/OptionalHyphen",      //  8
/*N*/         "NonprintingCharacter/Space",               //  9
/*N*/         "NonprintingCharacter/Break",               // 10
/*N*/         "NonprintingCharacter/ProtectedSpace",      // 11
/*N*/             "NonprintingCharacter/Tab",             // 12 //not in Writer/Web
/*N*/             "NonprintingCharacter/HiddenText",      // 13
/*N*/             "NonprintingCharacter/HiddenParagraph", // 14
/*N*/             "Update/Link",                          // 15
/*N*/             "Update/Field",                         // 16
/*N*/             "Update/Chart"                          // 17
/*N*/
/*N*/
/*N*/   };
/*N*/     const int nCount = bWeb ? 12 : 18;
/*N*/   Sequence<OUString> aNames(nCount);
/*N*/   OUString* pNames = aNames.getArray();
/*N*/   for(int i = 0; i < nCount; i++)
/*N*/   {
/*N*/       pNames[i] = OUString::createFromAscii(aPropNames[i]);
/*N*/   }
/*N*/   return aNames;
/*N*/ }


/*N*/ SwContentViewConfig::SwContentViewConfig(BOOL bIsWeb, SwMasterUsrPref& rPar) :
/*N*/     ConfigItem(bIsWeb ? C2U("Office.WriterWeb/Content") :  C2U("Office.Writer/Content"),
/*N*/         CONFIG_MODE_DELAYED_UPDATE|CONFIG_MODE_RELEASE_TREE),
/*N*/   rParent(rPar),
/*N*/   bWeb(bIsWeb)
/*N*/ {
/*N*/ }


/*N*/ SwContentViewConfig::~SwContentViewConfig()
/*N*/ {
/*N*/ }




/*N*/ void SwContentViewConfig::Load()
/*N*/ {
/*N*/   Sequence<OUString> aNames = GetPropertyNames();
/*N*/   Sequence<Any> aValues = GetProperties(aNames);
/*N*/   const Any* pValues = aValues.getConstArray();
/*N*/   DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
/*N*/   if(aValues.getLength() == aNames.getLength())
/*N*/   {
/*N*/       for(int nProp = 0; nProp < aNames.getLength(); nProp++)
/*N*/       {
/*N*/           if(pValues[nProp].hasValue())
/*N*/           {
/*N*/                 sal_Bool bSet = nProp != 15 ? *(sal_Bool*)pValues[nProp].getValue() : sal_False;
/*N*/               switch(nProp)
/*N*/               {
/*N*/                   case  0: rParent.SetGraphic(bSet);  break;// "Display/GraphicObject",
/*N*/                   case  1: rParent.SetTable(bSet);    break;// "Display/Table",
/*N*/                   case  2: rParent.SetDraw(bSet);     break;// "Display/DrawingControl",
/*N*/                   case  3: rParent.SetFldName(bSet);  break;// "Display/FieldCode",
/*N*/                   case  4: rParent.SetPostIts(bSet);  break;// "Display/Note",
/*N*/                     case  5: rParent.SetPreventTips(bSet);  break;// "Display/PreventTips",
/*N*/                     case  6: rParent.SetViewMetaChars(bSet); break; //"NonprintingCharacter/MetaCharacters"
/*N*/                     case  7: rParent.SetParagraph(bSet); break;// "NonprintingCharacter/ParagraphEnd",
/*N*/                     case  8: rParent.SetSoftHyph(bSet); break;// "NonprintingCharacter/OptionalHyphen",
/*N*/                     case  9: rParent.SetBlank(bSet);    break;// "NonprintingCharacter/Space",
/*N*/                     case 10: rParent.SetLineBreak(bSet);break;// "NonprintingCharacter/Break",
/*N*/                     case 11: rParent.SetHardBlank(bSet); break;// "NonprintingCharacter/ProtectedSpace",
/*N*/                     case 12: rParent.SetTab(bSet);      break;// "NonprintingCharacter/Tab",
/*N*/                     case 13: rParent.SetHidden(bSet);   break;// "NonprintingCharacter/HiddenText",
/*N*/                     case 14: rParent.SetShowHiddenPara(bSet); break;// "NonprintingCharacter/HiddenParagraph",
/*N*/                     case 15:
/*N*/                     {
                              sal_Int32 nSet(0); pValues[nProp] >>= nSet;
/*N*/                         rParent.SetUpdateLinkMode(nSet, TRUE);
/*N*/                   }
/*N*/                   break;// "Update/Link",
/*N*/                     case 16: rParent.SetUpdateFields(bSet, TRUE); break;// "Update/Field",
/*N*/                     case 17: rParent.SetUpdateCharts(bSet, TRUE); break;// "Update/Chart"
/*N*/               }
/*N*/           }
/*N*/       }
/*N*/   }
/*N*/ }


/*N*/ Sequence<OUString> SwLayoutViewConfig::GetPropertyNames()
/*N*/ {
/*N*/   static const char* aPropNames[] =
/*N*/   {
/*N*/         "Line/Guide",                       // 0
/*N*/         "Line/SimpleControlPoint",          // 1
/*N*/         "Line/LargeControlPoint",           // 2
/*N*/         "Window/HorizontalScroll",          // 3
/*N*/         "Window/VerticalScroll",            // 4
/*M*/         "Window/ShowRulers",                // 5
/*M*/         "Window/HorizontalRuler",           // 6
/*M*/         "Window/VerticalRuler",             // 7
/*M*/         "Window/HorizontalRulerUnit",       // 8
/*M*/         "Window/VerticalRulerUnit",         // 9
/*M*/         "Window/SmoothScroll",              //10
/*M*/         "Zoom/Value",                       //11
/*M*/         "Zoom/Type",                        //12
/*M*/         "Other/MeasureUnit",                //13
/*M*/         "Other/TabStop",                    //14
/*M*/         "Window/IsVerticalRulerRight"       //15
/*M*/     };
/*M*/     const int nCount = bWeb ? 14 : 16;
/*N*/     Sequence<OUString> aNames(nCount);
/*N*/   OUString* pNames = aNames.getArray();
/*N*/   for(int i = 0; i < nCount; i++)
/*N*/   {
/*N*/       pNames[i] = OUString::createFromAscii(aPropNames[i]);
/*N*/   }
/*N*/   return aNames;
/*N*/ }

    void SwLayoutViewConfig::Commit() {}
    void SwLayoutViewConfig::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& ) {}



/*N*/ SwLayoutViewConfig::SwLayoutViewConfig(BOOL bIsWeb, SwMasterUsrPref& rPar) :
/*N*/     ConfigItem(bIsWeb ? C2U("Office.WriterWeb/Layout") :  C2U("Office.Writer/Layout"),
/*N*/         CONFIG_MODE_DELAYED_UPDATE|CONFIG_MODE_RELEASE_TREE),
/*N*/   rParent(rPar),
/*N*/   bWeb(bIsWeb)
/*N*/ {
/*N*/ }


/*N*/ SwLayoutViewConfig::~SwLayoutViewConfig()
/*N*/ {
/*N*/ }




/*M*/ void SwLayoutViewConfig::Load()
/*M*/ {
/*M*/   Sequence<OUString> aNames = GetPropertyNames();
/*M*/   Sequence<Any> aValues = GetProperties(aNames);
/*M*/   const Any* pValues = aValues.getConstArray();
/*M*/   DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
/*M*/   if(aValues.getLength() == aNames.getLength())
/*M*/   {
/*M*/       for(int nProp = 0; nProp < aNames.getLength(); nProp++)
/*M*/       {
/*M*/           if(pValues[nProp].hasValue())
/*M*/           {
/*M*/                 sal_Bool bSet = nProp < 8 || nProp == 10 ? *(sal_Bool*)pValues[nProp].getValue() : sal_False;
/*M*/                 switch(nProp)
/*M*/               {
/*M*/                     case  0: rParent.SetCrossHair(bSet); break;// "Line/Guide",
/*M*/                     case  2: rParent.SetBigMarkHdl(bSet); break;// "Line/LargeControlPoint",
/*M*/                     case  3: rParent.SetViewHScrollBar(bSet); break;// "Window/HorizontalScroll",
/*M*/                     case  4: rParent.SetViewVScrollBar(bSet); break;// "Window/VerticalScroll",
/*M*/                     case  5: rParent.SetViewAnyRuler(bSet);break; // "Window/ShowRulers"
/*M*/                     case  6: rParent.SetViewHRuler(bSet); break;// "Window/HorizontalRuler",
/*M*/                     case  7: rParent.SetViewVRuler(bSet); break;// "Window/VerticalRuler",
/*M*/                     case  8:
/*M*/                     {
/*M*/                         rParent.bIsHScrollMetricSet = sal_True;
                              sal_Int32 nUnit(0); pValues[nProp] >>= nUnit;
/*M*/                         rParent.eHScrollMetric = ((FieldUnit)nUnit);  // "Window/HorizontalRulerUnit"
/*M*/                     }
/*M*/                     break;
/*M*/                     case  9:
/*M*/                     {
/*M*/                         rParent.bIsVScrollMetricSet = sal_True;
                              sal_Int32 nUnit(0); pValues[nProp] >>= nUnit;
/*M*/                         rParent.eVScrollMetric = ((FieldUnit)nUnit); // "Window/VerticalRulerUnit"
/*M*/                     }
/*M*/                     break;
/*M*/                     case 10: rParent.SetSmoothScroll(bSet); break;// "Window/SmoothScroll",
/*M*/                     case 11:
/*M*/                     {
                              sal_Int32 nVal(0); pValues[nProp] >>= nVal;
/*N*/                         rParent.SetZoom((USHORT)nVal);
/*M*/                     }
/*M*/                     break;// "Zoom/Value",
/*M*/                     case 12:
/*M*/                     {
                              sal_Int32 nVal(0); pValues[nProp] >>= nVal;
/*M*/                         rParent.SetZoomType((BYTE)nVal);
/*M*/                     }
/*M*/                     break;// "Zoom/Type",
/*M*/                     case 13:
/*M*/                   {
                              sal_Int32 nUnit(0); pValues[nProp] >>= nUnit;
/*M*/                         rParent.SetMetric((FieldUnit)nUnit, TRUE);
/*M*/                   }
/*M*/                   break;// "Other/MeasureUnit",
/*M*/                     case 14:
/*M*/                   {
                              sal_Int32 nTab(0); pValues[nProp] >>= nTab;
/*M*/                         rParent.SetDefTab(MM100_TO_TWIP(nTab), TRUE);
/*M*/                   }
/*M*/                   break;// "Other/TabStop",
/*M*/                     case 15: rParent.SetVRulerRight(bSet); break;// "Window/IsVerticalRulerRight",
/*M*/                 }
/*M*/           }
/*M*/       }
/*M*/   }
/*M*/ }



    void SwGridConfig::Commit() {}
    void SwGridConfig::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& ) {}

/*N*/ Sequence<OUString> SwGridConfig::GetPropertyNames()
/*N*/ {
/*N*/   static const char* aPropNames[] =
/*N*/   {
/*N*/       "Option/SnapToGrid",            // 0
/*N*/       "Option/VisibleGrid",           // 1
/*N*/       "Option/Synchronize",           // 2
/*N*/       "Resolution/XAxis",             // 3
/*N*/       "Resolution/YAxis",             // 4
/*N*/       "Subdivision/XAxis",            // 5
/*N*/       "Subdivision/YAxis"             // 6
/*N*/   };
/*N*/   const int nCount = 7;
/*N*/   Sequence<OUString> aNames(nCount);
/*N*/   OUString* pNames = aNames.getArray();
/*N*/   for(int i = 0; i < nCount; i++)
/*N*/   {
/*N*/       pNames[i] = OUString::createFromAscii(aPropNames[i]);
/*N*/   }
/*N*/   return aNames;
/*N*/ }


/*N*/ SwGridConfig::SwGridConfig(BOOL bIsWeb, SwMasterUsrPref& rPar) :
/*N*/     ConfigItem(bIsWeb ? C2U("Office.WriterWeb/Grid") :  C2U("Office.Writer/Grid"),
/*N*/         CONFIG_MODE_DELAYED_UPDATE|CONFIG_MODE_RELEASE_TREE),
/*N*/   rParent(rPar),
/*N*/   bWeb(bIsWeb)
/*N*/ {
/*N*/ }


/*N*/ SwGridConfig::~SwGridConfig()
/*N*/ {
/*N*/ }




/*N*/ void SwGridConfig::Load()
/*N*/ {
/*N*/   Sequence<OUString> aNames = GetPropertyNames();
/*N*/   Sequence<Any> aValues = GetProperties(aNames);
/*N*/   const Any* pValues = aValues.getConstArray();
/*N*/   DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
/*N*/   if(aValues.getLength() == aNames.getLength())
/*N*/   {
/*N*/       Size aSnap(rParent.GetSnapSize());
/*N*/       for(int nProp = 0; nProp < aNames.getLength(); nProp++)
/*N*/       {
/*N*/           if(pValues[nProp].hasValue())
/*N*/           {
/*N*/               sal_Bool bSet = nProp < 3 ? *(sal_Bool*)pValues[nProp].getValue() : sal_False;
                    sal_Int32 nSet(0);
/*N*/               if(nProp >= 3)
/*N*/                   pValues[nProp] >>= nSet;
/*N*/               switch(nProp)
/*N*/               {
/*N*/                   case  0: rParent.SetSnap(bSet); break;//        "Option/SnapToGrid",
/*N*/                   case  1: rParent.SetGridVisible(bSet); break;//"Option/VisibleGrid",
/*N*/                   case  2: rParent.SetSynchronize(bSet); break;//  "Option/Synchronize",
/*N*/                   case  3: aSnap.Width() = MM100_TO_TWIP(nSet); break;//      "Resolution/XAxis",
/*N*/                   case  4: aSnap.Height() = MM100_TO_TWIP(nSet); break;//      "Resolution/YAxis",
/*N*/                   case  5: rParent.SetDivisionX((short)nSet); break;//   "Subdivision/XAxis",
/*N*/                   case  6: rParent.SetDivisionY((short)nSet); break;//   "Subdivision/YAxis"
/*N*/               }
/*N*/           }
/*N*/       }
/*N*/       rParent.SetSnapSize(aSnap);
/*N*/   }
/*N*/ }



/*N*/ Sequence<OUString> SwCursorConfig::GetPropertyNames()
/*N*/ {
/*N*/   static const char* aPropNames[] =
/*N*/   {
/*N*/       "DirectCursor/UseDirectCursor", // 0
/*N*/       "DirectCursor/Insert",          // 1
/*N*/         "Option/ProtectedArea"          // 2
/*N*/   };
/*N*/     const int nCount = 3;
/*N*/   Sequence<OUString> aNames(nCount);
/*N*/   OUString* pNames = aNames.getArray();
/*N*/   for(int i = 0; i < nCount; i++)
/*N*/       pNames[i] = C2U(aPropNames[i]);
/*N*/   return aNames;
/*N*/ }

    void SwCursorConfig::Commit() {}
    void SwCursorConfig::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& ) {}



/*N*/ SwCursorConfig::SwCursorConfig(SwMasterUsrPref& rPar) :
/*N*/     ConfigItem(C2U("Office.Writer/Cursor"),
/*N*/         CONFIG_MODE_DELAYED_UPDATE|CONFIG_MODE_RELEASE_TREE),
/*N*/   rParent(rPar)
/*N*/ {
/*N*/ }


/*N*/ SwCursorConfig::~SwCursorConfig()
/*N*/ {
/*N*/ }




/*N*/ void SwCursorConfig::Load()
/*N*/ {
/*N*/   Sequence<OUString> aNames = GetPropertyNames();
/*N*/   Sequence<Any> aValues = GetProperties(aNames);
/*N*/   const Any* pValues = aValues.getConstArray();
/*N*/   DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
/*N*/   if(aValues.getLength() == aNames.getLength())
/*N*/   {
/*N*/
/*N*/       for(int nProp = 0; nProp < aNames.getLength(); nProp++)
/*N*/       {
/*N*/           if(pValues[nProp].hasValue())
/*N*/           {
                    sal_Bool bSet(0);
                    sal_Int32 nSet(0);
                    if(nProp != 1 )
/*N*/                   bSet = *(sal_Bool*)pValues[nProp].getValue();
/*N*/               else
/*N*/                   pValues[nProp] >>= nSet;
/*N*/               switch(nProp)
/*N*/               {
/*N*/                   case  0: rParent.SetShadowCursor(bSet);         break;//  "DirectCursor/UseDirectCursor",
/*N*/                     case  1: rParent.SetShdwCrsrFillMode((BYTE)nSet); break;//  "DirectCursor/Insert",
/*N*/                     case  2: rParent.SetCursorInProtectedArea(bSet); break;// "Option/ProtectedArea"
/*N*/               }
/*N*/           }
/*N*/       }
/*N*/
/*N*/   }
/*N*/ }


/*N*/ SwWebColorConfig::SwWebColorConfig(SwMasterUsrPref& rPar) :
/*N*/     ConfigItem(C2U("Office.WriterWeb/Background"),
/*N*/         CONFIG_MODE_DELAYED_UPDATE|CONFIG_MODE_RELEASE_TREE),
/*N*/   rParent(rPar),
/*N*/   aPropNames(1)
/*N*/ {
/*N*/   aPropNames.getArray()[0] = C2U("Color");
/*N*/ }

    void SwWebColorConfig::Commit() {}
    void SwWebColorConfig::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& ) {}



/*N*/ SwWebColorConfig::~SwWebColorConfig()
/*N*/ {
/*N*/ }




/*N*/ void SwWebColorConfig::Load()
/*N*/ {
/*N*/   Sequence<Any> aValues = GetProperties(aPropNames);
/*N*/   const Any* pValues = aValues.getConstArray();
/*N*/   DBG_ASSERT(aValues.getLength() == aPropNames.getLength(), "GetProperties failed");
/*N*/   if(aValues.getLength() == aPropNames.getLength())
/*N*/   {
/*N*/       for(int nProp = 0; nProp < aPropNames.getLength(); nProp++)
/*N*/       {
/*N*/           if(pValues[nProp].hasValue())
/*N*/           {
/*?*/               switch(nProp)
/*?*/               {
/*?*/                   case  0:
/*?*/                       sal_Int32 nSet(0);
/*?*/                       pValues[nProp] >>= nSet;
/*?*/                       rParent.SetRetoucheColor(nSet);
/*?*/                   break;// "Color",
/*?*/               }
/*N*/           }
/*N*/       }
/*N*/   }
/*N*/ }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
