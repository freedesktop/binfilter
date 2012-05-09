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
#pragma optimize("",off)
#endif


#include <bf_sfx2/docfilt.hxx>

#include <bf_sfx2/app.hxx>

#include <comphelper/classids.hxx>

#include <sot/formats.hxx>
#include "schmod.hxx"
#include "docshell.hxx"
#include "schlib.hxx"

class ValueSet;

#include "res_bmp.hrc"
namespace binfilter {

#define SCH_FORMAT_STR_50_FORMAT String( RTL_CONSTASCII_USTRINGPARAM( "StarChart 5.0" ))

/*?*/ TYPEINIT1( SchModuleDummy, SfxModule );

extern "C" { static void SAL_CALL thisModule() {} }

/*?*/ SFX_IMPL_OBJECTFACTORY_LIB( SchChartDocShell, SFXOBJECTSHELL_STD_NORMAL, schart,
                            SvGlobalName(BF_SO3_SCH_CLASSID), Sch, String( RTL_CONSTASCII_USTRINGPARAM( DLL_NAME )) )
// from macro: void SchChartDocShell::InitFactory()
/*?*/ {
/*?*/   SfxObjectFactory& rFactory = (SfxObjectFactory&)Factory();
/*?*/   rFactory.SetDocumentServiceName(String::CreateFromAscii("com.sun.star.chart.ChartDocument"));
/*?*/ }

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

/*?*/ SchDLL::SchDLL()
/*?*/ {
/*?*/ }

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

/*?*/ SchDLL::~SchDLL()
/*?*/ {
/*?*/ }

/*************************************************************************
|*
|*
|*
\************************************************************************/


/*?*/ void SchDLL::LibInit()
/*?*/ {
/*?*/   // Das Anlegen des ResMgr ist nur fuer die obige Filter-Registierung
/*?*/   // notwendig. Sobald dort kein SdResId mehr notwendig ist, kann das
/*?*/   // Anlegen entfallen
/*?*/   SchChartDocShell::RegisterFactory(SDT_SCH_DOCFACTPRIO);
/*?*/
/*?*/   SCH_MOD() = (SchModule *) new SchModuleDummy(NULL, TRUE, &SchChartDocShell::Factory() );
/*?*/ }

/*************************************************************************
|*
|*
|*
\************************************************************************/


/*?*/ void SchDLL::LibExit()
/*?*/ {
/*?*/   FreeLibSch();
/*?*/
/*?*/   // destroy the dummy-module with Object-Factory-Pointer
/*?*/   DELETEZ( SCH_MOD() );
/*?*/ }

/*************************************************************************
|*
|*
|*
\************************************************************************/


/*N*/ void SchDLL::Update( SvInPlaceObjectRef aIPObj, SchMemChart* pData, OutputDevice* pOut )
/*N*/ {
/*N*/   void (__LOADONCALLAPI*fp)(SvInPlaceObjectRef, SchMemChart*, OutputDevice*);
/*N*/
/*N*/   fp = (void (__LOADONCALLAPI*)(SvInPlaceObjectRef, SchMemChart*, OutputDevice*))GetFuncSch("SchUpdate");
/*N*/
/*N*/   if (fp)
/*N*/   {
/*N*/   fp(aIPObj, pData, pOut);
/*N*/   }
/*N*/ }

/*************************************************************************
|*
|*
|*
\************************************************************************/


/*N*/ SchMemChart* SchDLL::GetChartData (SvInPlaceObjectRef aIPObj)
/*N*/ {
/*N*/   SchMemChart* (__LOADONCALLAPI*fp)(SvInPlaceObjectRef);
/*N*/
/*N*/   fp = (SchMemChart* (__LOADONCALLAPI*)(SvInPlaceObjectRef aIPObj))GetFuncSch("SchGetChartData");
/*N*/
/*N*/   return fp
/*N*/              ? fp (aIPObj)
/*N*/              : 0;
/*N*/ }

/*************************************************************************
|*
|*
|*
\************************************************************************/

/*N*/ SchMemChart* SchDLL::NewMemChart (short nCols, short nRows)
/*N*/ {
/*N*/   SchMemChart* (__LOADONCALLAPI*fp)(short, short);
/*N*/
/*N*/   fp = (SchMemChart* (__LOADONCALLAPI*)(short, short))GetFuncSch("SchNewMemChartXY");
/*N*/
/*N*/   if (fp)
/*N*/   {
/*N*/       return fp(nCols, nRows);
/*N*/   }
/*N*/   else return 0;
/*N*/ }

/*************************************************************************
|*
|* Lade Module von aussen
|*
\************************************************************************/

/*?*/ SfxModule *SchModuleDummy::Load()
/*?*/ {
/*?*/   return (LoadLibSch() ? SCH_MOD() : NULL);
/*?*/ }

/*************************************************************************
|*
|* Bestimme die GUID abh. von der Version
|*
\************************************************************************/

/*?*/ const SvGlobalName SchModuleDummy::GetID (USHORT nFileFormat)
/*?*/ {
/*?*/   SvGlobalName aName;
/*?*/
/*?*/   switch (nFileFormat)
/*?*/   {
/*?*/       case SOFFICE_FILEFORMAT_60:
/*?*/       {
/*?*/           aName = SvGlobalName(BF_SO3_SCH_CLASSID_60);
/*?*/       }
/*?*/       break;
/*?*/
/*?*/       case SOFFICE_FILEFORMAT_50:
/*?*/       {
/*?*/           aName = SvGlobalName(BF_SO3_SCH_CLASSID_50);
/*?*/       }
/*?*/       break;
/*?*/
/*?*/       case SOFFICE_FILEFORMAT_40:
/*?*/       {
/*?*/           aName = SvGlobalName(BF_SO3_SCH_CLASSID_40);
/*?*/       }
/*?*/       break;
/*?*/
/*?*/       case SOFFICE_FILEFORMAT_31:
/*?*/       {
/*?*/           aName = SvGlobalName(BF_SO3_SCH_CLASSID_30);
/*?*/       }
/*?*/       break;
/*?*/
/*?*/       default:
/*?*/       {
/*?*/           OSL_FAIL( "Unknown file format" );
/*?*/       }
/*?*/       break;
/*?*/   }
/*?*/
/*?*/   return aName;
/*?*/ }

/*************************************************************************
|*
|* Bestimme die Version aus der GUID
|*
\************************************************************************/

/*?*/ USHORT SchModuleDummy::HasID (const SvGlobalName &rName)
/*?*/ {
/*?*/   USHORT nRet = 0;
/*?*/
/*?*/   if (GetID(SOFFICE_FILEFORMAT_31) == rName)
/*?*/   {
/*?*/       nRet = SOFFICE_FILEFORMAT_31;
/*?*/   }
/*?*/   else if (GetID(SOFFICE_FILEFORMAT_40) == rName)
/*?*/   {
/*?*/       nRet = SOFFICE_FILEFORMAT_40;
/*?*/   }
/*?*/   else if (GetID(SOFFICE_FILEFORMAT_50) == rName)
/*?*/   {
/*?*/       nRet = SOFFICE_FILEFORMAT_50;
/*?*/   }
/*?*/   else if (GetID(SOFFICE_FILEFORMAT_60) == rName)
/*?*/   {
/*?*/       nRet = SOFFICE_FILEFORMAT_60;
/*?*/   }
/*?*/
/*?*/   return(nRet);
/*?*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
