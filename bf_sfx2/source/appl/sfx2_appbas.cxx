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

#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/uno/Reference.h>

#include <tools/urlobj.hxx>
#include <bf_svtools/rectitem.hxx>
#include <bf_svtools/intitem.hxx>
#include <bf_svtools/eitem.hxx>
#include <bf_svtools/stritem.hxx>
#include <bf_svtools/pathoptions.hxx>

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "appuno.hxx"
#include "objsh.hxx"
#include "app.hxx"
#include "appdata.hxx"
#include "appimp.hxx"
#include "dlgcont.hxx"

#include "scriptcont.hxx"

#define ITEMID_SEARCH SID_SEARCH_ITEM


#include <legacysmgr/legacy_binfilters_smgr.hxx>
#include "bf_so3/staticbaseurl.hxx"
#include <boost/scoped_ptr.hpp>

namespace binfilter {

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::script;

//--------------------------------------------------------------------

namespace
{
    class AppBasicManagerHolder
    {
    private:
        SfxScriptLibraryContainer* m_pBasicCont;
        SfxDialogLibraryContainer* m_pDialogCont;
        BasicManager* m_pAppBasicManager;
    public:
        AppBasicManagerHolder();
        BasicManager* getBasicManager() { return m_pAppBasicManager; };
        ~AppBasicManagerHolder();
    };

    boost::scoped_ptr<AppBasicManagerHolder> s_pAppBasicManager;

    AppBasicManagerHolder::AppBasicManagerHolder()
    {
        // Directory bestimmen
        SvtPathOptions aPathCFG;
        String aAppBasicDir( aPathCFG.GetBasicPath() );
        if ( !aAppBasicDir.Len() )
            aPathCFG.SetBasicPath( String::CreateFromAscii("$(prog)") );

        // #58293# soffice.new nur im ::com::sun::star::sdbcx::User-Dir suchen => erstes Verzeichnis
        String aAppFirstBasicDir = aAppBasicDir.GetToken(1);

        // Basic erzeugen und laden
        // MT: #47347# AppBasicDir ist jetzt ein PATH!
        INetURLObject aAppBasic( SvtPathOptions().SubstituteVariable( String::CreateFromAscii("$(progurl)") ) );
        aAppBasic.insertName( Application::GetAppName() );

        m_pAppBasicManager = new BasicManager(new StarBASIC, &aAppBasicDir);

        // Als Destination das erste Dir im Pfad:
        String aFileName( aAppBasic.getName() );
        aAppBasic = INetURLObject( aAppBasicDir.GetToken(1) );
        DBG_ASSERT( aAppBasic.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );
        aAppBasic.insertName( aFileName );
        m_pAppBasicManager->SetStorageName( aAppBasic.PathToFileName() );

        // globale Variablen
        StarBASIC *pBas = m_pAppBasicManager->GetLib(0);
        sal_Bool bBasicWasModified = pBas->IsModified();

        // Basic container
        m_pBasicCont = new SfxScriptLibraryContainer
            ( DEFINE_CONST_UNICODE( "StarBasic" ), m_pAppBasicManager );
        m_pBasicCont->acquire();    // Hold via UNO
        Reference< XLibraryContainer > xBasicCont = static_cast< XLibraryContainer* >( m_pBasicCont );
        m_pBasicCont->setBasicManager( m_pAppBasicManager );

        // Dialog container
        m_pDialogCont = new SfxDialogLibraryContainer( NULL );
        m_pDialogCont->acquire();   // Hold via UNO
        Reference< XLibraryContainer > xDialogCont = static_cast< XLibraryContainer* >( m_pDialogCont );

        LibraryContainerInfo* pInfo = new LibraryContainerInfo
            ( xBasicCont, xDialogCont, static_cast< OldBasicPassword* >( m_pBasicCont ) );
        m_pAppBasicManager->SetLibraryContainerInfo( pInfo );

        // Durch MakeVariable wird das Basic modifiziert.
        if ( !bBasicWasModified )
            pBas->SetModified( sal_False );
    }

    AppBasicManagerHolder::~AppBasicManagerHolder()
    {
        m_pDialogCont->release();
        m_pBasicCont->release();
        BasicManager::LegacyDeleteBasicManager(m_pAppBasicManager);
    }
}

//========================================================================

StarBASIC* SfxApplication::GetBasic_Impl() const
{
    if (!s_pAppBasicManager)
        return NULL;
    return s_pAppBasicManager->getBasicManager()->GetLib(0);
}

//=========================================================================

BasicManager* SfxApplication::GetBasicManager()
{
    if ( pAppData_Impl->nBasicCallLevel == 0 )
        // sicherheitshalber
        EnterBasicCall();

    if (!s_pAppBasicManager)
        s_pAppBasicManager.reset(new AppBasicManagerHolder);

    return s_pAppBasicManager->getBasicManager();
}

//--------------------------------------------------------------------

/*N*/ StarBASIC* SfxApplication::GetBasic()
/*N*/ {
/*N*/   return GetBasicManager()->GetLib(0);
/*N*/ }

//--------------------------------------------------------------------

/*N*/ void SfxApplication::EnterBasicCall()
/*N*/ {
/*N*/   if ( 1 == ++pAppData_Impl->nBasicCallLevel )
/*N*/   {
/*N*/       OSL_TRACE( "SfxShellObject: BASIC-on-demand" );
/*N*/
/*N*/       // zuerst das BASIC laden
/*N*/       GetBasic();
/*N*/   }
/*N*/ }

//--------------------------------------------------------------------

/*N*/ void SfxApplication::LeaveBasicCall()
/*N*/ {
/*N*/   --pAppData_Impl->nBasicCallLevel;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
