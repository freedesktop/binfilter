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

#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <bf_sfx2/interno.hxx>
#include <comphelper/classids.hxx>
#include "svdoole2.hxx"
#include <bf_svtools/solar.hrc>
#include "svdio.hxx"
#include "svdstr.hrc"   // Objektname
#include "svdetc.hxx"
#include "svdview.hxx"
#include "unomlstr.hxx"
#include <bf_so3/client.hxx>

namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;

#ifndef SO2_DECL_SVINPLACEOBJECT_DEFINED
#define SO2_DECL_SVINPLACEOBJECT_DEFINED
SO2_DECL_REF(SvInPlaceObject)
#endif

class SdrOle2ObjImpl
{
public:
    GDIMetaFile*    pMetaFile;          // Metafile fuer GetMtf kopieren und merken
    BfGraphicObject*    pGraphicObject;
    String          aPersistName;       // name of object in persist

    // #107645#
    // New local var to avoid repeated loading if load of OLE2 fails
    sal_Bool        mbLoadingOLEObjectFailed;

    bool    mbConnected;

};

// Predicate determining whether the given OLE is an internal math
// object
static bool ImplIsMathObj( const SvInPlaceObjectRef& rObjRef )
{
    if( !rObjRef.Is() )
        return false;

    SvGlobalName aClassName( rObjRef->GetClassName() );

    if( aClassName == SvGlobalName(BF_SO3_SM_CLASSID_30) ||
        aClassName == SvGlobalName(BF_SO3_SM_CLASSID_40) ||
        aClassName == SvGlobalName(BF_SO3_SM_CLASSID_50) ||
        aClassName == SvGlobalName(BF_SO3_SM_CLASSID_60) ||
        aClassName == SvGlobalName(BF_SO3_SM_CLASSID)         )
    {
        return true;
    }
    else
    {
        return false;
    }
}

TYPEINIT1(SdrOle2Obj,SdrRectObj);

SdrOle2Obj::SdrOle2Obj(bool bFrame_)
{
    bInDestruction = FALSE;
    Init();

    ppObjRef=new SvInPlaceObjectRef;
    bFrame=bFrame_;
}

SdrOle2Obj::SdrOle2Obj(const SvInPlaceObjectRef& /*rNewObjRef*/, const XubString& /*rNewObjName*/, const Rectangle& rNewRect, bool /*bFrame_*/) :
    SdrRectObj(rNewRect)
{
}

void SdrOle2Obj::Init()
{
    mpImpl = new SdrOle2ObjImpl;
    pModifyListener = NULL;
    pGraphic=NULL;
    mpImpl->pMetaFile=NULL;
    mpImpl->pGraphicObject=NULL;

    // #107645#
    // init to start situation, loading did not fail
    mpImpl->mbLoadingOLEObjectFailed = sal_False;

    mpImpl->mbConnected = false;
}

SdrOle2Obj::~SdrOle2Obj()
{
    bInDestruction = TRUE;
    // Aus Cache entfernen
    GetSdrGlobalData().GetOLEObjCache().RemoveObj(this);

    SvInPlaceObjectRef aRef( *ppObjRef );

    Disconnect();

    if(pModel && aRef.Is())
    {
        SvPersist* pPers=pModel->GetPersist();
        if(pPers!=NULL)
        {
            pPers->Remove(aRef);
        }
    }
    aRef.Clear();

    delete ppObjRef;

    if(pGraphic!=NULL)
        delete pGraphic;

    if(mpImpl->pMetaFile!=NULL)
        delete mpImpl->pMetaFile;

    if(mpImpl->pGraphicObject!=NULL)
        delete mpImpl->pGraphicObject;

    if(pModifyListener)
    {
        pModifyListener->invalidate();
        pModifyListener->release();
        pModifyListener = NULL;
    }
    delete mpImpl;
}

void SdrOle2Obj::SetGraphic(const Graphic* pGrf)
{
    if ( pGraphic )
    {
        delete pGraphic;
        pGraphic = NULL;
        delete mpImpl->pGraphicObject;
        mpImpl->pGraphicObject = NULL;
    }

    if (pGrf!=NULL)
    {
        pGraphic = new Graphic(*pGrf);
        mpImpl->pGraphicObject = new BfGraphicObject( *pGraphic );
    }

    if ( ppObjRef->Is() && pGrf )
        SendRepaintBroadcast();

    SetChanged();
}

bool SdrOle2Obj::IsEmpty() const
{
    return !ppObjRef->Is();
}

void SdrOle2Obj::Connect()
{
    if(pModel && mpImpl->aPersistName.Len())
    {
        SvPersist* pPers=pModel->GetPersist();
        if (pPers!=NULL)
        {
            SvInfoObjectRef xIObj;
            SvInfoObject* pInfo = pPers->Find(mpImpl->aPersistName);

            if (!pInfo)
            {
                if ( !ppObjRef->Is() )
                    GetObjRef();    // try to load inplace object

                xIObj = pInfo = new SvEmbeddedInfoObject(*ppObjRef,mpImpl->aPersistName);
            }

            if ( !pPers->HasObject(mpImpl->aPersistName) )
            {
                pPers->Move(pInfo, mpImpl->aPersistName);
            }
            else
            {
                pInfo->SetDeleted(FALSE);
            }
        }

        // In Cache einfuegen
        GetSdrGlobalData().GetOLEObjCache().InsertObj(this);
    }

    if ( ppObjRef->Is() && (*ppObjRef)->GetMiscStatus() & SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE )
    {
        if (pModel && pModel->GetRefDevice() &&
            pModel->GetRefDevice()->GetOutDevType() == OUTDEV_PRINTER)
        {
            // Kein RefDevice oder RefDevice kein Printer
            BOOL bModified = (*ppObjRef)->IsModified();
            Printer* pPrinter = (Printer*) pModel->GetRefDevice();
            (*ppObjRef)->OnDocumentPrinterChanged( pPrinter );
            (*ppObjRef)->SetModified( bModified );
        }
    }

    if( !IsEmpty() )
    {
        // register modify listener
        if( pModifyListener == NULL )
        {
            ((SdrOle2Obj*)this)->pModifyListener = new SvxUnoShapeModifyListener( (SdrOle2Obj*)this );
            pModifyListener->acquire();
        }

        uno::Reference< util::XModifyBroadcaster > xBC( getXModel(), uno::UNO_QUERY );
        if( xBC.is() && pModifyListener )
        {
            uno::Reference< util::XModifyListener > xListener( pModifyListener );
            xBC->addModifyListener( xListener );
        }

        mpImpl->mbConnected = true;
    }
}

void SdrOle2Obj::Disconnect()
{
    if( !mpImpl->mbConnected )
        return;

    if( !IsEmpty() && mpImpl->aPersistName.Len() )
    {
        uno::Reference< util::XModifyBroadcaster > xBC( getXModel(), uno::UNO_QUERY );
        if( xBC.is() && pModifyListener )
        {
            uno::Reference< util::XModifyListener > xListener( pModifyListener );
            xBC->removeModifyListener( xListener );
        }
    }

    if (pModel && mpImpl->aPersistName.Len() )
    {
        if( pModel->IsInDestruction() )
        {
            *ppObjRef = NULL;
        }
        else
        {
            SvPersist* pPers = pModel->GetPersist();

            if (pPers)
            {
                SvInfoObject* pInfo = pPers->Find(mpImpl->aPersistName);

                if (pInfo)
                {
                    pInfo->SetDeleted(TRUE);
                    pInfo->SetObj(0);
                }
            }

            if ( ppObjRef->Is() )
                (*ppObjRef)->DoClose();
        }

        // Aus Cache entfernen
        GetSdrGlobalData().GetOLEObjCache().RemoveObj(this);

        if ( ppObjRef->Is() )
            ppObjRef->Clear();
    }

    mpImpl->mbConnected = false;
}

void SdrOle2Obj::SetModel(SdrModel* pNewModel)
{
    bool bChg = pNewModel!=pModel;

    if( bChg )
        Disconnect(); // mit dem alten Namen

    SdrRectObj::SetModel( pNewModel );

    if( bChg )
        Connect();
}

void SdrOle2Obj::SetPage(SdrPage* pNewPage)
{
    bool bRemove=pNewPage==NULL && pPage!=NULL;
    bool bInsert=pNewPage!=NULL && pPage==NULL;

    if (bRemove) Disconnect();

    SdrRectObj::SetPage(pNewPage);

    if (bInsert) Connect();
}

void SdrOle2Obj::SetObjRef(const SvInPlaceObjectRef& rNewObjRef)
{
    if( rNewObjRef == *ppObjRef )
        return;

    Disconnect();

    *ppObjRef=rNewObjRef;

    SvInPlaceObjectRef& rIPRef = *ppObjRef;

    if (rIPRef.Is() &&
        (rIPRef->GetMiscStatus() & SVOBJ_MISCSTATUS_NOTRESIZEABLE) )
    {
        SetResizeProtect(TRUE);
    }

    // #108759# For math objects, set closed state to transparent
    if( ImplIsMathObj( *ppObjRef ) )
        SetClosedObj( false );

    Connect();
    SetChanged();
    SendRepaintBroadcast();
}

void SdrOle2Obj::SetClosedObj( bool bIsClosed )
{
    // #108759# Allow changes to the closed state of OLE objects
    bClosedObj = bIsClosed;
  }

XubString SdrOle2Obj::GetName() const
{
    return aName;
}

void SdrOle2Obj::SetPersistName( const String& rPersistName )
{
    mpImpl->aPersistName = rPersistName;
    SetChanged();
}

String SdrOle2Obj::GetPersistName() const
{
    return mpImpl->aPersistName;
}

UINT16 SdrOle2Obj::GetObjIdentifier() const
{
    return bFrame ? UINT16(OBJ_FRAME) : UINT16(OBJ_OLE2);
}

SdrObject* SdrOle2Obj::CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const
{
    return ImpCheckHit(rPnt,nTol,pVisiLayer,TRUE,TRUE);
}

void SdrOle2Obj::ImpSetVisAreaSize()
{
    GetObjRef();    // try to load inplace object
    SvInPlaceObjectRef& rIPRef=*ppObjRef;

    if (rIPRef.Is())
    {
        if (rIPRef->GetMiscStatus() & SVOBJ_MISCSTATUS_SERVERRESIZE)
        {
            // Server resized selbst (StarChart)
            // Neue VisArea setzen
            Rectangle aVisArea = OutputDevice::LogicToLogic( aRect,
                                    pModel->GetScaleUnit(), rIPRef->GetMapUnit() );
            rIPRef->SetVisArea(aVisArea);

            // Wurde die VisArea akzeptiert?
            Rectangle aAcceptedVisArea(rIPRef->GetVisArea());

            if (aVisArea.GetSize() != aAcceptedVisArea.GetSize())
            {
                // VisArea wurde nicht akzeptiert -> korrigieren
                aRect.SetSize(OutputDevice::LogicToLogic( aAcceptedVisArea.GetSize(),
                        rIPRef->GetMapUnit(), pModel->GetScaleUnit()));
            }
        }
        else
        {
            ///**********************************************************************
            //* Nun wird nicht mehr die VisArea gesetzt, sondern es erfolgt eine
            //* Skalierung
            //**********************************************************************/
            SvEmbeddedClient* pClient = (*ppObjRef)->GetClient();

            if (pClient)
            {
                SvClientData* pData = pClient->GetClientData();
                Size aObjAreaSize = rIPRef->GetVisArea().GetSize();
                aObjAreaSize = OutputDevice::LogicToLogic( aObjAreaSize,
                                                            rIPRef->GetMapUnit(),
                                                            pModel->GetScaleUnit() );

                Size aSize = aRect.GetSize();
                Fraction aScaleWidth (aSize.Width(),  aObjAreaSize.Width() );
                Fraction aScaleHeight(aSize.Height(), aObjAreaSize.Height() );
                // Nun auf 10 Binaerstellen kuerzen
                Kuerzen(aScaleHeight, 10);
                Kuerzen(aScaleWidth,  10);

                pData->SetSizeScale(aScaleWidth, aScaleHeight);

                Rectangle aScaleRect(aRect.TopLeft(), aObjAreaSize);
                pData->SetObjArea(aScaleRect);
            }
        }
    }
}

void SdrOle2Obj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    SdrRectObj::NbcResize(rRef,xFact,yFact);
    if (aGeo.nShearWink!=0 || aGeo.nDrehWink!=0) { // kleine Korrekturen
        if (aGeo.nDrehWink>=9000 && aGeo.nDrehWink<27000) {
            aRect.Move(aRect.Left()-aRect.Right(),aRect.Top()-aRect.Bottom());
        }
        aGeo.nDrehWink=0;
        aGeo.nShearWink=0;
        aGeo.nSin=0.0;
        aGeo.nCos=1.0;
        aGeo.nTan=0.0;
        SetRectsDirty();
    }
    if( (NULL == pModel) || !pModel->isLocked() )
    ImpSetVisAreaSize();
}

void SdrOle2Obj::NbcSetSnapRect(const Rectangle& rRect)
{
    SdrRectObj::NbcSetSnapRect(rRect);
    if( (NULL == pModel) || !pModel->isLocked() )
    ImpSetVisAreaSize();
}

void SdrOle2Obj::NbcSetLogicRect(const Rectangle& rRect)
{
    SdrRectObj::NbcSetLogicRect(rRect);
    if( (NULL == pModel) || !pModel->isLocked() )
    ImpSetVisAreaSize();
}

void SdrOle2Obj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
{
    rIn.SetError( 0 );

    if (rIn.GetError()!=0) return;
    SdrRectObj::ReadData(rHead,rIn);
    SdrDownCompat aCompat(rIn,STREAM_READ); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
#ifdef DBG_UTIL
    aCompat.SetID("SdrOle2Obj");
#endif

    // UNICODE: rIn >> mpImpl->aPersistName;
    mpImpl->aPersistName = rIn.ReadUniOrByteString(rIn.GetStreamCharSet());

    // UNICODE: rIn >> aProgName;
    aProgName = rIn.ReadUniOrByteString(rIn.GetStreamCharSet());

    BOOL bObjRefValid;
    rIn>>bObjRefValid;

    BOOL bHasGraphic;
    rIn>>bHasGraphic;
    if (bHasGraphic)
    {
        if(pGraphic==NULL)
            pGraphic=new Graphic;

    // #i27418# Remember stream position to decide if something was read
    const sal_Size nFilePosition(rIn.Tell());

        if(rHead.GetVersion()>=11)
        { // ab V11 eingepackt
            SdrDownCompat aGrafCompat(rIn,STREAM_READ);
#ifdef DBG_UTIL
            aGrafCompat.SetID("SdrOle2Obj(Graphic)");
#endif
            rIn>>*pGraphic;
        }
        else
            rIn>>*pGraphic;

    // #i27418# If 4 bytes were read but an error is set, it was tried to read an empty
    // bitmap into the Graphic. This a follow-up error from previously wrong saves.
    // Reset error code here, it's not really an error but leads to a crash in SO7
    if(0L != rIn.GetError() && (nFilePosition + 4L) == rIn.Tell())
    {
        rIn.ResetError();
    }

        if( mpImpl->pGraphicObject )
            delete mpImpl->pGraphicObject;

        mpImpl->pGraphicObject = new BfGraphicObject( *pGraphic );
    }
}

void SdrOle2Obj::NbcMove(const Size& rSize)
{
    SdrRectObj::NbcMove(rSize);
    if( (NULL == pModel) || !pModel->isLocked() )
    ImpSetVisAreaSize();
}

BOOL SdrOle2Obj::Unload()
{
    BOOL bUnloaded = FALSE;

    if( ppObjRef && ppObjRef->Is() )
    {
        //Nicht notwendig im Doc DTor (MM)
        ULONG nRefCount = (*ppObjRef)->GetRefCount();
        // prevent Unload if there are external references
        if( nRefCount > 2 )
            return FALSE;
        DBG_ASSERT( nRefCount == 2, "Wrong RefCount for unload" );
    }
    else
        bUnloaded = TRUE;

    if (pModel && ppObjRef && ppObjRef->Is() &&
        SVOBJ_MISCSTATUS_ALWAYSACTIVATE != (*ppObjRef)->GetMiscStatus() &&
        1 < (*ppObjRef)->GetRefCount()                                  &&
        !(*ppObjRef)->IsModified()                                      &&
        !(*ppObjRef)->GetProtocol().IsInPlaceActive() )
    {
        SvPersist* pPersist = pModel->GetPersist();

        if (pPersist)
        {
            SvPersist* pO = *ppObjRef;
            ppObjRef->Clear();
            if (pPersist->Unload(pO))
                bUnloaded = TRUE;
            else
                *ppObjRef = pO;
        }
    }

    return bUnloaded;
}

const SvInPlaceObjectRef& SdrOle2Obj::GetObjRef() const
{
    if ( !ppObjRef->Is() && pModel && pModel->GetPersist() && !pModel->GetPersist()->IsHandsOff() )
    {
        // #107645#
        // Only try loading if it did not wrent wrong up to now
        if(!mpImpl->mbLoadingOLEObjectFailed)
        {
            // Objekt laden
            (*ppObjRef) = &( pModel->GetPersist()->GetObject( mpImpl->aPersistName ) );

            // #107645#
            // If loading of OLE object failed, remember that to not invoke a endless
            // loop trying to load it again and again.
            if(!ppObjRef->Is())
            {
                mpImpl->mbLoadingOLEObjectFailed = sal_True;
            }

            // #108759# For math objects, set closed state to transparent
            if( ImplIsMathObj( *ppObjRef ) )
                const_cast<SdrOle2Obj*>(this)->SetClosedObj( false );
        }

        if ( ppObjRef->Is() )
        {
            if( !IsEmptyPresObj() )
            {
                // #75637# remember modified status of model
                BOOL bWasChanged(pModel ? pModel->IsChanged() : FALSE);

                // perhaps preview not valid anymore
                // #75637# This line changes the modified state of the model
                ( (SdrOle2Obj*) this )->SetGraphic( NULL );

                // #75637# if status was not set before, force it back
                // to not set, so that SetGraphic(0L) above does not
                // set the modified state of the model.
                if(!bWasChanged && pModel && pModel->IsChanged())
                    pModel->SetChanged(FALSE);
            }

            if ( (*ppObjRef)->GetMiscStatus() & SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE )
            {
                if (pModel && pModel->GetRefDevice() &&
                    pModel->GetRefDevice()->GetOutDevType() == OUTDEV_PRINTER)
                {
                    if(!bInDestruction)
                    {
                        // prevent SetModified (don't want no update here)
                        sal_Bool bWasEnabled = (*ppObjRef)->IsEnableSetModified();
                        if ( bWasEnabled )
                            (*ppObjRef)->EnableSetModified( sal_False );

                        // Kein RefDevice oder RefDevice kein Printer
                        Printer* pPrinter = (Printer*) pModel->GetRefDevice();
                        (*ppObjRef)->OnDocumentPrinterChanged( pPrinter );

                        // reset state
                        (*ppObjRef)->EnableSetModified( bWasEnabled );
                    }
                }
            }
            // register modify listener on initial load
            if( pModifyListener == NULL )
            {
                ((SdrOle2Obj*)this)->pModifyListener = new SvxUnoShapeModifyListener( (SdrOle2Obj*)this );
                pModifyListener->acquire();

                uno::Reference< util::XModifyBroadcaster > xBC( getXModel(), uno::UNO_QUERY );
                if( xBC.is() && pModifyListener )
                {
                    uno::Reference< util::XModifyListener > xListener( pModifyListener );
                    xBC->addModifyListener( xListener );
                }
            }
        }
    }

    if (ppObjRef->Is())
    {
        // In Cache einfuegen
        GetSdrGlobalData().GetOLEObjCache().InsertObj((SdrOle2Obj*) this);
    }

    return *ppObjRef;
}

uno::Reference< frame::XModel > SdrOle2Obj::getXModel() const
{
    uno::Reference< frame::XModel > xModel;

    if( pModel )
    {
        SvInPlaceObjectRef xSvIPO( GetObjRef() );
        SfxInPlaceObjectRef xIPO( xSvIPO );
        if( xIPO.Is() )
        {
            SfxObjectShell* pShell = xIPO->GetObjectShell();

            if( pShell )
                xModel = pShell->GetModel();
        }
    }

    return xModel;
}

/// #110015# sets the visible area at the SvInPlaceObject and SvEmbeddedInfoObject
void SdrOle2Obj::SetVisibleArea( const Rectangle& rVisArea )
{
    const SvInPlaceObjectRef& xInplace = GetObjRef();
    if( xInplace.Is() )
    {
        xInplace->SetVisArea( rVisArea );

        if( pModel && (!pModel->GetPersist()->IsEnableSetModified()) )
            xInplace->SetModified(FALSE);
    }

    if(pModel && mpImpl->aPersistName.Len() )
    {
        SvPersist* pPers = pModel->GetPersist();

        if (pPers)
        {
            SvInfoObject* pInfo = pPers->Find(mpImpl->aPersistName);
            SvEmbeddedInfoObject * pEmbed = PTR_CAST(SvEmbeddedInfoObject, pInfo );

            if( pEmbed )
                pEmbed->SetInfoVisArea( rVisArea );
        }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
