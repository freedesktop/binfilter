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

#ifndef _SVDOGRP_HXX
#define _SVDOGRP_HXX

#include <bf_svtools/bf_solar.h>
#include <bf_tools/string.hxx>

#include <tools/datetime.hxx>

#include <bf_svx/svdobj.hxx>
class SdrObjListIter;
namespace binfilter {

class SfxItemSet;

//************************************************************
//   Vorausdeklarationen
//************************************************************

class ImpSdrObjGroupLink;
class SdrObjList;

//************************************************************
//   Defines
//************************************************************

#define SDRUSERDATA_OBJGROUPLINK    0

//************************************************************
//   Hilfsklasse ImpSdrObjGroupLinkUserData
//************************************************************

class ImpSdrObjGroupLinkUserData : public SdrObjUserData
{
    friend class                SdrObjGroup;
    friend class                ImpSdrObjGroupLink;

    SdrObject*                  pObj;
    String                      aFileName;   // Name des referenzierten Dokuments
    XubString                   aObjName;    // Name des referenzierten Objekts
    DateTime                    aFileDate0;  // Unnoetiges neuladen vermeiden
    Rectangle                   aSnapRect0;  // Letztes bekanntes SnapRect des Originals
    long                        nDrehWink0;  // Letzter bekannter Drehwinkel des Originals
    long                        nShearWink0; // Letzter bekannter Shearwinkel des Originals

    ImpSdrObjGroupLink*         pLink;
    sal_uInt32                  nObjNum;     // des referenzierten Objekts
    USHORT                      nPageNum;    // zum schnelleren wiederauffinden
    bool                    bMasterPage; // Liegt im Referenzdokoment auf einer Masterpage

    bool                    bOrigPos;    // Objekt hat immer die Position des Referenzobjekts
    bool                    bOrigSize;   // Objekt hat immer die Groesse des Referenzobjekts
    bool                    bOrigRotate; // Objekt hat immer die Drehung des Referenzobjekts
    bool                    bOrigShear;  // Objekt hat immer den Shearwinkel des Referenzobjekts

public:
    TYPEINFO();

    ImpSdrObjGroupLinkUserData(SdrObject* pObj1);
    virtual ~ImpSdrObjGroupLinkUserData();

    virtual SdrObjUserData* Clone(SdrObject* pObj1) const;
    virtual void ReadData(SvStream& rIn);
    virtual void AfterRead() {} // DBG_BF_ASSERT
};

//************************************************************
//   SdrObjGroup
//************************************************************

class SdrObjGroup : public SdrObject
{
    friend class                ImpSdrObjGroupLink;
    friend class                ImpSdrObjGroupLinkUserData;

protected:
    SfxItemSet*                 mpGroupItemSet;
    SdrObjList*                 pSub;    // Subliste (Kinder)
    long                        nDrehWink;
    long                        nShearWink;

    String                      aName;

    Point                       aRefPoint; // Referenzpunkt innerhalb der Objektgruppe
    bool                        bRefPoint; // Ist ein RefPoint gesetzt?

private:
    ImpSdrObjGroupLinkUserData* GetLinkUserData() const;

public:
    TYPEINFO();
    SdrObjGroup();
    virtual ~SdrObjGroup();

    // Folgende 5 Methoden zur Verwendung von Verknuepfungen zu Gruppenobjekten
    // in fremden Dokumenten.
    // !!! Derzeit werden noch keine Ringreferenzen abgefangen !!!
    // GruppenLinks koennen durchaus selber auch gruppiert werden. Nach betreten
    // einer gelinkten Gruppe kann zwar noch selektiert, jedoch nichts mehr
    // veraendert werden (ReadOnly). Das Referenzgruppenobjekt im fremden
    // Dokument darf nicht gruppiert sein, muss sich also immer auf dem Root-
    // Level der Page befinden.
    // Transformationen auf den gesamten Gruppenlink sind zwar vorgesehen,
    // jedoch noch nicht implementiert.
    // Zur Verwendung von GruppenLinks muessen die Methoden
    //   virtual const SdrModel* SdrModel::LoadModel(const String& rFileName);
    //   virtual const SdrModel* SdrModel::DisposeLoadedModels();
    // ueberladen werden (siehe SvdModel.HXX), die ein neues Model alloziiert
    // und in den Speicher laed bzw. zuvor geladene Models verwirft.
    // Ansonsten muss die App I.d.R. nur SetGroupLink() rufen - der Rest geschieht
    // von selbst (SfxLinkManager). Die Methode LoadGroup() kann ausserdem
    // verwendet werden, um eine benannte Gruppe aus einem fremden Dokument zu
    // laden (ohne Verknuepfung).
    bool IsLinkedGroup() const { return pPlusData!=NULL && GetLinkUserData()!=NULL; }

    // pnPgNum, etc. ist zum schnelleren wiederauffinden gedacht

    virtual UINT16 GetObjIdentifier() const { return UINT16(OBJ_GRUP); } // DBG_BF_ASSERT
    using SdrObject::GetLayer;
    virtual SdrLayerID GetLayer() const;
    virtual void NbcSetLayer(SdrLayerID nLayer);
    virtual void SetObjList(SdrObjList* pNewObjList);
    virtual void SetPage(SdrPage* pNewPage);
    virtual void SetModel(SdrModel* pNewModel);
    virtual SdrObjList* GetSubList() const;

    virtual const Rectangle& GetBoundRect() const;
    virtual const Rectangle& GetSnapRect() const;
    virtual bool Paint(ExtOutputDevice& rOut, const SdrPaintInfoRec& rInfoRec) const;
    virtual void operator=(const SdrObject& rObj);



    virtual void SetName(const String& rStr);
    virtual String GetName() const;

    virtual void RecalcSnapRect() {} // DBG_BF_ASSERT
    virtual void TakeXorPoly(XPolyPolygon& rPoly, bool bDetail) const;


    virtual long GetRotateAngle() const;
    virtual long GetShearAngle(bool bVertical=FALSE) const;

    virtual void Move(const Size& rSiz);
    virtual void Resize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    virtual void SetAnchorPos(const Point& rPnt);
    virtual void SetRelativePos(const Point& rPnt);
    virtual void SetSnapRect(const Rectangle& rRect);

    virtual void NbcMove(const Size& rSiz);
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    virtual void NbcSetAnchorPos(const Point& rPnt);
    virtual void NbcSetSnapRect(const Rectangle& rRect);
    virtual void NbcSetLogicRect(const Rectangle& rRect);

    // ItemSet access
    virtual const SfxItemSet& GetItemSet() const;
    virtual SfxItemSet* CreateNewItemSet(SfxItemPool& rPool);
    virtual void ItemSetChanged(const SfxItemSet& rSet);

    // private support routines for ItemSet access. NULL pointer means clear item.
    virtual void ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem = 0);

    virtual void NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr);
    virtual void SetStyleSheet(SfxStyleSheet*, bool ) {} // DBG_BF_ASSERT
    virtual SfxStyleSheet* GetStyleSheet() const;

    virtual void ReformatText();

    virtual void RestartAnimation(SdrPageView* pPageView) const;


    virtual void ReadData(const SdrObjIOHeader& rHead, SvStream& rIn);
    virtual void AfterRead();

    // ItemPool fuer dieses Objekt wechseln
};

}//end of namespace binfilter
#endif //_SVDOGRP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
