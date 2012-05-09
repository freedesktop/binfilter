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

#include <stdio.h>

#include <bf_so3/ipenv.hxx>
#include <bf_so3/iface.hxx>

#include <tools/debug.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/svapp.hxx>
#include <vcl/accel.hxx>
#include <bf_so3/so2dll.hxx>
#include <bf_so3/ipobj.hxx>
#include "bf_so3/ipclient.hxx"
#include "bf_so3/ipwin.hxx"

class INetURLObject;

namespace binfilter {

//=========================================================================
//==========SvContainerEnvironment=========================================
//=========================================================================

SO2_IMPL_INTERFACE1(SvAppFrame,SvObject)

::IUnknown * SvAppFrame::GetMemberInterface( const SvGlobalName & )
{
    return NULL;
}

//=========================================================================
SvAppFrame::~SvAppFrame()
/*  [Beschreibung]

    Destruktor der Klasse SvAppFrame.

    [Querverweise]

    <SvInPlaceClient>
*/
{
}

//=========================================================================
//=========================================================================
//=========================================================================
SO2_IMPL_INTERFACE1(SvDocFrame,SvObject)

::IUnknown * SvDocFrame::GetMemberInterface( const SvGlobalName & )
{
    return NULL;
}

//=========================================================================
SvDocFrame::~SvDocFrame()
/*  [Beschreibung]

    Destruktor der Klasse SvDocFrame.

    [Querverweise]

    <SvInPlaceClient>
*/
{
}

//=========================================================================
//==========SvContainerEnvironment=========================================
//=========================================================================
static void InsertInContList( SvContainerEnvironment * p )
{
    SoDll * pSoApp = SOAPP;
    if( !pSoApp->pContEnvList )
        pSoApp->pContEnvList = new SvContainerEnvironmentList();
    pSoApp->pContEnvList->push_back( p );
}

#define INIT_CTOR                   \
      pOleInfo      ( NULL )        \
    , hOleMenuDesc  ( 0 )           \
    , nCount1       ( 0 )           \
    , nCount2       ( 0 )           \
    , nCount3       ( 0 )           \
    , nMenuUseCount ( 0 )           \
    , pAccel        ( NULL )        \
    , aClipAreaPixel( 0, 0, 0x7FFF, 0x7FFF )    \
    , bDfltUIAction ( TRUE )        \
    , bDeleteTopWin ( FALSE )       \
    , bDeleteDocWin ( FALSE )       \
    , bDeleteEditWin ( FALSE )

TYPEINIT1(SvContainerEnvironment,SvClientData);

//=========================================================================
SvContainerEnvironment::SvContainerEnvironment
(
    SvInPlaceClient * pCl,
    WorkWindow * pTopWinP,
    WorkWindow * pDocWinP,
    Window * pEditWin_
)
    : SvClientData( pCl, pEditWin_ )
    , pIPEnv( NULL )
    , pObj( pCl )
    , pParent( NULL )
    , pTopWin( pTopWinP )
    , pDocWin( pDocWinP )
    , INIT_CTOR
{
    InsertInContList( this );
}

//=========================================================================
SvContainerEnvironment::~SvContainerEnvironment()
/*  [Beschreibung]

    Im Destruktor werden alle selbsterzeugten Fenster zerst"ort.

    [Querverweise]
*/
{
    DBG_ASSERT( !pIPEnv, "IPEnv exist" );

    if( bDeleteEditWin )
    {
        Window * pWin = SvClientData::GetEditWin();
        SetEditWin( NULL );
        delete pWin;
    }
    if( bDeleteDocWin )
        delete pDocWin;
    if( bDeleteTopWin )
        delete pTopWin;

    SoDll * pSoApp = SOAPP;
    for( SvContainerEnvironmentList::iterator it = pSoApp->pContEnvList->begin();
         it < pSoApp->pContEnvList->end();
         ++it
       )
    {
        if ( *it == this )
        {
            pSoApp->pContEnvList->erase( it );
            break;
        }
    }

    delete pAccel;

    DBG_ASSERT( !xAppFrame.Is() || 1 == xAppFrame->GetRefCount(),
                "can't destroy xAppFrame" );
    DBG_ASSERT( !xDocFrame.Is() || 1 == xDocFrame->GetRefCount(),
                "can't destroy xDocFrame" );
}

/************************************************************************
|*    SvContainerEnvironment::IsStub()
|*
|*    Beschreibung:
*************************************************************************/
BOOL SvContainerEnvironment::IsStub() const
{
    if( pParent )
        return pParent->IsStub();
    return pObj != NULL && !pObj->Owner();
}

/************************************************************************
|*    SvContainerEnvironment::ShowUIByChildDeactivate()
|*
|*    Beschreibung:
*************************************************************************/
void SvContainerEnvironment::ShowUIByChildDeactivate()
{
    if( pObj && pObj->GetProtocol().IsInPlaceActive() )
    {   // gibt es einen IP-Client und ist dieser IP-Active
        // dann UI aktivieren
        pObj->GetIPObj()->DoUIActivate( TRUE );
    }
    else if( pParent )
        pParent->ShowUIByChildDeactivate();
}

#ifdef Brauch_ich_nicht_mehr
//=========================================================================
void SvContainerEnvironment::DeleteWindows_Impl()
/*  [Beschreibung]

    Die Fenster pEditWin, pDocWin und pTopWin werden in dieser
    Reihenfolge gel"oscht.
*/
{
    if( bDeleteEditWin )
    {
        // erst austragen, dann l"oschen
        Window * pWin = SvClientData::GetEditWin();
        SetEditWin( NULL );
        delete pWin;
    }
    if( bDeleteDocWin )
        delete pDocWin;
    if( bDeleteTopWin )
        delete pTopWin;
    bDeleteEditWin = bDeleteDocWin = bDeleteTopWin = FALSE;
}
#endif

//=========================================================================
WorkWindow * SvContainerEnvironment::GetTopWin() const
{
    return pTopWin;
}

//=========================================================================
WorkWindow * SvContainerEnvironment::GetDocWin() const
{
    return pDocWin;
}

//=========================================================================
Window * SvContainerEnvironment::GetEditWin() const
{
    return SvClientData::GetEditWin();
}

/************************************************************************
|*    SvContainerEnvironment::QueryMenu()
|*
|*    Beschreibung
*************************************************************************/
MenuBar * SvContainerEnvironment::QueryMenu
(
    USHORT * ,
    USHORT * ,
    USHORT *
)
{
    return 0;
}

/************************************************************************
|*    SvContainerEnvironment::SetInPlaceMenu()
|*
|*    Beschreibung
*************************************************************************/
void SvContainerEnvironment::SetInPlaceMenu( MenuBar *, BOOL )
{
}

/************************************************************************
|*    SvContainerEnvironment::MenuReleased()
|*
|*    Beschreibung
*************************************************************************/
void SvContainerEnvironment::MenuReleased()
{
}

/************************************************************************
|*    SvContainerEnvironment::UIToolsShown()
|*
|*    Beschreibung
*************************************************************************/
void SvContainerEnvironment::UIToolsShown( BOOL /* bShow */ )
{
}

/*************************************************************************
|*    SvContainerEnvironment::GetAccel()
|*
|*    Beschreibung
*************************************************************************/
Accelerator * SvContainerEnvironment::GetAccel()
{
    DBG_ASSERT( !pObj || pObj->Owner(),
                 "cannot convert HACCEL to Accelerator (not implemented)" );
    if( !pAccel && pParent )
        return pParent->GetAccel();
    return pAccel;
}

/*************************************************************************
|*    SvContainerEnvironment::DispatchAccel()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvContainerEnvironment::DispatchAccel( const KeyCode & rCode )
{
    if( pParent )
        return pParent->DispatchAccel( rCode );

    return FALSE;
}

/*************************************************************************
|*    SvContainerEnvironment::OutDevScaleChanged()
|*
|*    Beschreibung
*************************************************************************/
void SvContainerEnvironment::OutDevScaleChanged()
{
    if( pIPEnv )
        pIPEnv->DoRectsChanged();
}

/*************************************************************************
|*    SvClientData::SetSizeScale()
|*
|*    Beschreibung
*************************************************************************/
void SvContainerEnvironment::SetSizeScale( const Fraction & rScaleWidth,
                                const Fraction & rScaleHeight )
{
    if( GetScaleWidth() != rScaleWidth || GetScaleHeight() != rScaleHeight )
    {
        SvClientData::SetSizeScale( rScaleWidth, rScaleHeight );
        OutDevScaleChanged();
    }
}

/*************************************************************************
|*    SvInPLaceClient::SetObjArea()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvContainerEnvironment::SetObjArea( const Rectangle & rArea )
{
    if( SvClientData::SetObjArea( rArea ) )
    {
        if( pIPEnv )
            pIPEnv->DoRectsChanged();
        return TRUE;
    }
    return FALSE;
}

/*************************************************************************
|*    SvContainerEnvironment::GetObjArea()
|*
|*    Beschreibung
*************************************************************************/
Rectangle SvContainerEnvironment::GetObjArea() const
{
    return SvClientData::GetObjArea();
}

/*************************************************************************
|*    SvContainerEnvironment::PixelObjVisAreaToLogic()
|*
|*    Beschreibung: Der Return ist in logischen Koordinaten des Objektes
*************************************************************************/
Rectangle SvContainerEnvironment::PixelObjVisAreaToLogic(
                                    const Rectangle & rObjRect ) const
{
    DBG_ASSERT( pIPEnv, "no InPlaceEnvironment" );

    SvInPlaceObject * pIPObj = pIPEnv->GetIPObj();

    Window * pWin;
    if( !pObj || pObj->Owner() )
        pWin = GetEditWin();
    else
        pWin = pIPEnv->GetEditWin();

    MapMode aClientMap( pWin->GetMapMode().GetMapUnit() );
    MapMode aObjMap( pIPObj->GetMapUnit() );

    // VisArea entsprechend der Groessenaenderung anpassen
    Rectangle aRect = pIPObj->GetVisArea();

    // Wenn die linke obere Ecke verschoben wird, auch die linke
    // obere Ecke der VisArea verschieben. Dies aber nur, wenn gleichzeitig
    // die Groesse geaendert wird.
    Rectangle aOldArea = GetObjAreaPixel();
    long nWidth = aOldArea.GetWidth();
    if( nWidth && nWidth != rObjRect.GetWidth() )
    {
        // Differenz berechnen
        long n = rObjRect.Left() - aOldArea.Left();
        // Breite in Pixel des Orginal
        // 3 Satz, Linke Seite entsprechend Vergroessern
        aRect.Left() += n * aRect.GetWidth() / nWidth;
    }
    long nHeight = aOldArea.GetHeight();
    if( nHeight && nHeight != rObjRect.GetHeight() )
    {
        long n = rObjRect.Top() - aOldArea.Top();
        aRect.Top() += n * aRect.GetHeight() / nHeight;
    }

    Size aSize;
    // Scale und OutDev richtig
    aSize = pWin->PixelToLogic( rObjRect.GetSize() );
    // Skalierung bei der Visarea nicht beruecksichtigen
    aSize = pWin->LogicToLogic( aSize, &aClientMap, &aObjMap );
    aSize.Width()  = Fraction( aSize.Width() )  / GetScaleWidth();
    aSize.Height() = Fraction( aSize.Height() ) / GetScaleHeight();

    aRect.SetSize( aSize );
    return aRect;
}

/*************************************************************************
|*    SvContainerEnvironment::RequestObjAreaPixel()
|*
|*    Beschreibung
*************************************************************************/
void SvContainerEnvironment::RequestObjAreaPixel( const Rectangle & rObjRect )
{
    // IP-Win wurde veraendert, Client anpassen
    // Position im Client setzen
    if( !pIPEnv )
    {
        SetObjAreaPixel( rObjRect );
        return;
    }

    Rectangle aOldAreaPixel = GetObjAreaPixel();
    if( rObjRect == aOldAreaPixel )
        return;

    pIPEnv->LockRectsChanged();

    Rectangle aOldArea = GetObjArea();
    Rectangle aArea = PixelObjAreaToLogic( rObjRect );
    // Dies muss vor dem setzen der neuen ObjArea erfolgen, da die
    // alte ObjArea benoetigt wird
    // Ist in Objekt Koordinaten
    Rectangle aObjVisArea = PixelObjVisAreaToLogic( rObjRect );
    SvInPlaceObjectRef xIPObj = pIPEnv->GetIPObj();
    Rectangle aOldObjVisArea = xIPObj->GetVisArea();
    if( rObjRect.GetSize() == aOldAreaPixel.GetSize() )
    {
        // Die Grosse aendert sich nur durch Pixel-Rundung,
        // deshalb die alte logische Groesse nehmen
        aObjVisArea.SetSize( aOldObjVisArea.GetSize() );
        aArea.SetSize( aOldArea.GetSize() );
    }

    if( rObjRect.TopLeft() == aOldAreaPixel.TopLeft() )
    {
        // Die Position aendert sich nur durch Pixel-Rundung,
        // deshalb die alte logische Position nehmen
        aObjVisArea.SetPos( aOldObjVisArea.TopLeft() );
        aArea.SetPos( aOldArea.TopLeft() );
    }


    // Dadurch wird eine zu fruehe Ausgabe unterbunden
    BOOL bInval = IsInvalidate();
    if( xIPObj->GetProtocol().IsInPlaceActive() )
        SetInvalidate( FALSE );

    SetObjArea( aArea );
    SetInvalidate( bInval );

    xIPObj->SetVisArea( aObjVisArea );

    // innerhalb der Verhandlung nicht neu updaten
    pIPEnv->UnlockRectsChanged();
    pIPEnv->DoRectsChanged();
}

/*************************************************************************
|*    SvContainerEnvironment::Scroll()
|*
|*    Beschreibung
*************************************************************************/
void SvContainerEnvironment::Scroll( const Size & rSize )
{
    if( !rSize.Width() && !rSize.Height() )
        // doch nicht gescrollt
        return;

    if( !pObj || pObj->Owner() )
    {
        // Da in Scroll der Origin versetzt wird,
        // aendern sich die Pixel Koordinaten
        pIPEnv->DoRectsChanged();
    }
}

/************************************************************************
|*    SvContainerEnvironment::RequestTopToolSpacePixel()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvContainerEnvironment::RequestTopToolSpacePixel( const SvBorder & rInner )
{
    if( pParent )
        return pParent->RequestTopToolSpacePixel( rInner );

    BOOL bRet = FALSE;
    if( !pObj || pObj->Owner() )
    {
        bRet = TRUE;
    }
    return bRet;
}

/************************************************************************
|*    SvContainerEnvironment::RequestDocToolSpacePixel()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvContainerEnvironment::RequestDocToolSpacePixel( const SvBorder & rInner )
{
    if( pParent )
        return pParent->RequestDocToolSpacePixel( rInner );

    BOOL bRet = FALSE;
    if( !pObj || pObj->Owner() )
    {
        bRet = TRUE;
    }
    return bRet;
}

/************************************************************************
|*    SvContainerEnvironment::SetTopToolSpacePixel()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvContainerEnvironment::SetTopToolSpacePixel( const SvBorder & rInner )
{
    if( pParent )
        return pParent->SetTopToolSpacePixel( rInner );

    if( !pObj || pObj->Owner() )
    {
        if( RequestTopToolSpacePixel( rInner ) )
        {
            // Sind Top- und EditWin unterschiedlich, dann muss das EditWin
            // neu angeordnet werden. Dies muss von dem abgeleiteten
            // Environment geleistet werden.
            // Sind sie gleich, kann die Anordnung automatisch geschehen
            return GetEditWin() == GetTopWin();
        }
        return FALSE;
    }
    return FALSE;
}

/************************************************************************
|*    SvContainerEnvironment::SetDocToolSpacePixel()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvContainerEnvironment::SetDocToolSpacePixel( const SvBorder & rInner )
{
    if( pParent )
        return pParent->SetDocToolSpacePixel( rInner );

    if( !pObj || pObj->Owner() )
    {
        if( RequestDocToolSpacePixel( rInner ) )
        {
            // Sind Doc- und EditWin unterschiedlich, dann muss das EditWin
            // neu angeordnet werden. Dies muss von dem abgeleiteten
            // Environment geleistet werden.
            // Sind sie gleich, kann die Anordnung automatisch geschehen
            return GetEditWin() == GetDocWin();
        }
    }
    return FALSE;
}

/************************************************************************
|*    SvContainerEnvironment::GetTopOuterRectPixel()
|*
|*    Beschreibung
*************************************************************************/
Rectangle SvContainerEnvironment::GetTopOuterRectPixel() const
{
    if( pParent )
        return pParent->GetTopOuterRectPixel();

    Rectangle aOuter;
    if( !pObj || pObj->Owner() )
    {
        aOuter = Rectangle( Point(), pTopWin->GetOutputSizePixel() );
        aOuter -= aTopBorder;
    }

    return aOuter;
}

/************************************************************************
|*    SvContainerEnvironment::GetDocOuterRectPixel()
|*
|*    Beschreibung
*************************************************************************/
Rectangle SvContainerEnvironment::GetDocOuterRectPixel() const
{
    if( pParent )
        return pParent->GetDocOuterRectPixel();

    Rectangle aOuter;
    if( !pObj || pObj->Owner() )
    {
        aOuter = Rectangle( Point(), pDocWin->GetOutputSizePixel() );
        aOuter -= aDocBorder;
    }

    return aOuter;
}


/************************************************************************
|*    SvContainerEnvironment::SetStatusText()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvContainerEnvironment::SetStatusText( const String & rText )
{
    if( pParent )
        return pParent->SetStatusText( rText );

    return FALSE;
}

//=========================================================================
void SvContainerEnvironment::ShowDocument
(
    const INetURLObject &,      /* Die URL, die angezeigt werden soll. */
    const XubString &           /* Spezifiert den Frame, in dem das Dokument
                                   angezeigt werden soll.
                                   "_self", show in current frame.
                                   "_parent", show in parent frame.
                                   "_top", show in top-most frame.
                                   "_blank", show in new unnamed top-level
                                   window.
                                   name, show in a new top-level named name.
                                 */
)
/*  [Beschreibung]

    Show a new document in a target window or frame. This may be ignored
    by  the applet context.

    [Querverweise]

    <SjAppletContext::ShowDocument>
*/
{
}

/************** SvInPlaceEnvironment ***********************************************
**************************************************************************/
SvInPlaceEnvironment::SvInPlaceEnvironment( SvContainerEnvironment * pCl,
                                SvInPlaceObject * pObjP )
    : nChangeRectsLockCount( 0 )
    , bShowUITools( FALSE )
    , bTopWinResize( FALSE )
    , bDocWinResize( FALSE )
    , bDeleteEditWin( FALSE )
    , pUIMenu( NULL )
    , pClientMenu( NULL )
    , pClipWin( NULL )
    , pBorderWin( NULL )
    , pEditWin( NULL )
    , pContEnv( pCl )
    , pObj( pObjP )
{
    pContEnv->SetIPEnv( this );
}

/*************************************************************************/
SvInPlaceEnvironment::~SvInPlaceEnvironment()
{
    DBG_ASSERT( !bShowUITools, "UI-Tools werden noch angezeigt" );
    DoShowUITools( FALSE );
    if( bDeleteEditWin )
        delete pEditWin;
    pContEnv->ResetIPEnv();
}

//=========================================================================
Window * SvInPlaceEnvironment::GetEditWin()
{
    return pEditWin;
}

//=========================================================================
MenuBar * SvInPlaceEnvironment::QueryMenu
(
    USHORT *,
    USHORT *,
    USHORT *
)
{
    return NULL;
}

/*************************************************************************/
void SvInPlaceEnvironment::DoShowIPObj( BOOL bShow )
{
    if( bShow )
        DoRectsChanged();
    ShowIPObj( bShow );
}

/*************************************************************************/
void SvInPlaceEnvironment::ShowIPObj( BOOL bShow )
{
    if( pClipWin )
    {
        if( bShow )
            GetClipWin()->Show();
        else
            GetClipWin()->Hide();
    }
}

/*************************************************************************/
void SvInPlaceEnvironment::DoShowUITools( BOOL bShow )
{

    //DBG_ASSERT( !bShow || !bShowUITools, "bShowUITools && bShow" );
    if( bShow == bShowUITools )
        // bei soviel Activate/Deactivate ist es besser dies zu gestatten
        return;

    if( bShow && !GetContainerEnv()->IsStub() )
    {
        SoDll * pSoApp = SOAPP;
        // SvInPlaceEnvironment * pUIEnv = pSoApp->pUIShowIPEnv;
        // unbedingt zuerst setzen
        pSoApp->pUIShowIPEnv = this;

        // kein Child darf UI-Aktiv sein
        SvContainerEnvironment * pEnv = GetContainerEnv();

        // kein Parent darf UI-Aktiv sein
        SvContainerEnvironment * pPar = pEnv->GetParent();
        while( pPar && pPar->GetIPClient() )
        {
            pPar->GetIPClient()->GetProtocol().Reset2InPlaceActive();
            pPar = pPar->GetParent();
        }
    }
    else if( !bShow && !GetContainerEnv()->IsStub() )
    {
        SoDll * pSoApp = SOAPP;
        if( pSoApp->pUIShowIPEnv == this )
            pSoApp->pUIShowIPEnv = NULL;
    }

    if( bShow )
        // bei Show == TRUE zuerst den Conatiner benachrichtigen
        GetContainerEnv()->UIToolsShown( bShow );
    // Flag muss gesetzt werden, son kommt Resize nicht durch
    bShowUITools = bShow;

    if( GetIPObj()->Owner() && pUIMenu )
        GetContainerEnv()->SetInPlaceMenu( pUIMenu, bShow );
    if( bShow )
    {
        if( GetIPObj()->Owner() )
        {
            if( !bTopWinResize )
                // Anordnen der Tools anstossen
                DoTopWinResize();
            if( !bDocWinResize )
                // Anordnen der Tools anstossen
                DoDocWinResize();
        }
    }
    else
    {
        // Anordnen der Tools anstossen
        bTopWinResize = FALSE;
        bDocWinResize = FALSE;
    }

    ShowUITools( bShow );
    if( !bShow )
        // bei Show == FALSE zuletzt den Conatiner benachrichtigen
        GetContainerEnv()->UIToolsShown( bShow );
}

/*************************************************************************/
void SvInPlaceEnvironment::ShowUITools( BOOL )
{
}

//=========================================================================
void SvInPlaceEnvironment::DoTopWinResize()
/*  [Beschreibung]

    Es wird nur die Methode <SvInPlaceEnvironment::TopWinResize>
    gerufen, wenn die UI-Tools angezeigt werden. Ausserdem wird
    ein Status gespeichert, der Anzeigt, ob vor dem
    <SvInPlaceEnvironment::ShowUITools> ein TopWinResize gerufen werden
    muss.
*/
{
    if( bShowUITools )
    {
        bTopWinResize = TRUE;
        TopWinResize();
    }
    else
        bTopWinResize = FALSE;
}

//=========================================================================
void SvInPlaceEnvironment::TopWinResize()
/*  [Beschreibung]

    Die Gr"osse des TopWin hat sich ge"andert. Die Methode wird vor
    ShowUITools( TRUE ) mindestens einmal gerufen.
    Die Defaultaktion ist:
        GetContainerEnv()->SetTopToolSpacePixel( SvBorder() );

    [!Owner]

    Die Methode wird an den Server weitergeleitet. Dies geschieht nur
    wenn das pActiveObj gesetzt ist.

    [Querverweise]

    <IOleInPlaceActiveObject::ResizeBorder>
*/
{
    bTopWinResize = TRUE;

    if( pObj->Owner() )
        GetContainerEnv()->SetTopToolSpacePixel( SvBorder() );
}

//=========================================================================
void SvInPlaceEnvironment::DoDocWinResize()
{
/*  [Beschreibung]

    Es wird nur die Methode <SvInPlaceEnvironment::DocWinResize>
    gerufen, wenn die UI-Tools angezeigt werden. Ausserdem wird
    ein Status gespeichert, der Anzeigt, ob vor dem
    <SvInPlaceEnvironment::ShowUITools> ein DocWinResize gerufen werden
    muss.
*/

    if( bShowUITools )
    {
        bDocWinResize = TRUE;
        DocWinResize();
    }
    else
        bDocWinResize = FALSE;
}

//=========================================================================
void SvInPlaceEnvironment::DocWinResize()
/*  [Beschreibung]

    Die Gr"osse des DocWin hat sich ge"andert. Die Methode wird vor
    ShowUITools( TRUE ) mindestens einmal gerufen.

    [!Owner]
    Die Methode wird an den Server weitergeleitet. Dies geschieht nur
    wenn das pActiveObj gesetzt ist.

    [Querverweise]

    <IOleInPlaceActiveObject::ResizeBorder>
*/
{
    bDocWinResize = TRUE;
}

/*************************************************************************/
void SvInPlaceEnvironment::RectsChangedPixel( const Rectangle & rObjRect,
                                        const Rectangle & rClip )
{
    if( pObj->Owner() )
    {
        if( pClipWin )
            pClipWin->SetRectsPixel( rObjRect, rClip );
    }
}

/**************************************************************************/
void SvInPlaceEnvironment::MakeWindows()
{
    // Fenster fuers Clipping erzeugen
    pClipWin = new SvInPlaceClipWindow( pContEnv->GetEditWin() );
    // Fenster fuer den Border Erzeugen
    pBorderWin = new SvInPlaceWindow( pClipWin, this );
    pBorderWin->Show();

    // Fenstergroesse mit Beachtung der Border setzen
    Rectangle aRect( pContEnv->GetObjAreaPixel() );
    pBorderWin->SetInnerPosSizePixel( aRect.TopLeft(), aRect.GetSize() );

    // Fenster zuordnen (fuers Resize)
    pClipWin->SetResizeWin( pBorderWin );
}

/**************************************************************************/
void SvInPlaceEnvironment::DeleteWindows()
{
    // Fenster zuordnen (fuers Resize)
    pClipWin->Hide();
    pClipWin->SetResizeWin( NULL );
    delete pBorderWin;
    pBorderWin = NULL;
    delete pClipWin;
    pClipWin = NULL;
}

/************************************************************************
|*    SvInPlaceEnvironment::QueryRectsChanged()
|*
|*    Beschreibung
*************************************************************************/
void SvInPlaceEnvironment::QueryObjAreaPixel( Rectangle & ) const
{
}

/************************************************************************
|*    SvInPlaceEnvironment::LockRectsChanged();
|*    SvInPlaceEnvironment::UnlockRectsChanged();
|*
|*    Beschreibung
*************************************************************************/
void SvInPlaceEnvironment::LockRectsChanged()
{
    nChangeRectsLockCount++;
}

void SvInPlaceEnvironment::UnlockRectsChanged()
{
    nChangeRectsLockCount--;
}

/************************************************************************
|*  SvInPlaceEnvironment::DoRectsChanged()
|*
|*  Beschreibung:   Die Methode darf den Handler nur rufen, wenn die
|*                  Bereiche Werte groesser als Null haben. Ausserdem muss
|*                  sichergestellt sein, das der Handler fuer dieselben
|*                  Werte nicht zweimal gerufen wird.
*************************************************************************/
void SvInPlaceEnvironment::DoRectsChanged( BOOL bIfEqual )
{
    if( nChangeRectsLockCount ) return;

    Rectangle aClipAreaPixel = pContEnv->GetClipAreaPixel();
    if( aClipAreaPixel.GetWidth() > 0 && aClipAreaPixel.GetHeight() > 0 )
    {
        Rectangle aObjRect = pContEnv->GetObjAreaPixel();

        if( aObjRect.GetWidth() > 0 && aObjRect.GetHeight() > 0 )
            if( bIfEqual ||
              (aOldObjAreaPixel != aObjRect
               || aClipAreaPixel != aOldClipAreaPixel) )
            {
                aOldObjAreaPixel = aObjRect;
                aOldClipAreaPixel = aClipAreaPixel;

                RectsChangedPixel( aObjRect, aClipAreaPixel );
            }
    }
}

/*************************************************************************
|*    SvInPlaceEnvironment::DispatchAccel()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvInPlaceEnvironment::DispatchAccel( const KeyCode & )
{
    return FALSE;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
