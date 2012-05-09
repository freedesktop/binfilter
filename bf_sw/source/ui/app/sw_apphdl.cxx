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



#define _SVSTDARR_STRINGSDTOR

#include <bf_sfx2/event.hxx>
#include <bf_svtools/colorcfg.hxx>
#include <bf_svtools/eitem.hxx>
#include <bf_svtools/whiter.hxx>
#include <bf_svtools/isethint.hxx>
#include <bf_svtools/ctloptions.hxx>


#include <horiornt.hxx>

#include <viewsh.hxx>
#include <cmdid.h>          // Funktion-Ids
#include <swmodule.hxx>
#include <wdocsh.hxx>
#include <doc.hxx>
#include <globals.h>        // globale Konstanten z.B.
#include <app.hrc>
#include <usrpref.hxx>
#include <prtopt.hxx>
#include <modcfg.hxx>
#include <fontcfg.hxx>
#include <barcfg.hxx>
#include <dbconfig.hxx>

#include <bf_sfx2/app.hxx>
#include <bf_svx/svxids.hrc>

// #107253#

using namespace ::com::sun::star;

#include <cfgid.h>

#include <shells.hrc>
#include "bf_so3/staticbaseurl.hxx"

namespace binfilter {

/*M*/ void SwModule::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
/*M*/ {
/*M*/   if( rHint.ISA( SfxEventHint ) )
/*M*/   {
/*M*/   }
/*M*/   else if(rHint.ISA(SfxItemSetHint))
/*M*/   {
/*M*/       if( SFX_ITEM_SET == ((SfxItemSetHint&)rHint).GetItemSet().
/*M*/                   GetItemState( SID_ATTR_ADDRESS, sal_False ))
/*M*/           bAuthorInitialised = FALSE;
/*M*/   }
/*M*/   else if(rHint.ISA(SfxSimpleHint))
/*M*/   {
/*M*/         ULONG nHintId = ((SfxSimpleHint&)rHint).GetId();
/*M*/         if(SFX_HINT_DEINITIALIZING == nHintId)
/*M*/         {
/*M*/             DELETEZ(pWebUsrPref);
/*M*/             DELETEZ(pUsrPref)   ;
/*M*/             DELETEZ(pModuleConfig);
/*M*/             DELETEZ(pPrtOpt)      ;
/*M*/             DELETEZ(pWebPrtOpt)   ;
/*M*/             DELETEZ(pStdFontConfig)     ;
/*M*/             DELETEZ(pAuthorNames)       ;
/*M*/             DELETEZ(pDBConfig);
/*M*/             EndListening(*pColorConfig);
/*M*/             DELETEZ(pColorConfig);
/*N*/             EndListening(*pCTLOptions);
/*N*/             DELETEZ(pCTLOptions);
/*M*/         }
/*M*/   }
/*M*/ }

/*N*/ SwDBConfig*   SwModule::GetDBConfig()
/*N*/ {
/*N*/   if(!pDBConfig)
/*N*/       pDBConfig = new SwDBConfig;
/*N*/   return pDBConfig;
/*N*/ }

/*N*/ ColorConfig& SwModule::GetColorConfig()
/*N*/ {
/*N*/     if(!pColorConfig)
/*N*/   {
/*N*/         pColorConfig = new ColorConfig;
/*N*/       SwViewOption::ApplyColorConfigValues(*pColorConfig);
/*N*/         StartListening(*pColorConfig);
/*N*/     }
/*N*/     return *pColorConfig;
/*N*/ }

SvtCTLOptions& SwModule::GetCTLOptions()
{
    if(!pCTLOptions)
    {
        pCTLOptions = new SvtCTLOptions;
        StartListening(*pCTLOptions);
    }
    return *pCTLOptions;
}

/*N*/ const SwMasterUsrPref *SwModule::GetUsrPref(sal_Bool bWeb) const
/*N*/ {
/*N*/   SwModule* pNonConstModule = (SwModule*)this;
/*N*/   if(bWeb && !pWebUsrPref)
/*N*/   {
/*N*/       // im Load der SwMasterUsrPref wird der SpellChecker gebraucht, dort darf
/*N*/       // er aber nicht angelegt werden #58256#
/*N*/       pNonConstModule->pWebUsrPref = new SwMasterUsrPref(TRUE);
/*N*/   }
/*N*/   else if(!bWeb && !pUsrPref)
/*N*/   {
/*N*/       pNonConstModule->pUsrPref = new SwMasterUsrPref(FALSE);
/*N*/   }
/*N*/   return  bWeb ? pWebUsrPref : pUsrPref;
/*N*/ }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
