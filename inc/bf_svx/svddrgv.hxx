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

#ifndef _SVDDRGV_HXX
#define _SVDDRGV_HXX

#include <bf_svtools/bf_solar.h>
#include <bf_svx/svdxcgv.hxx>

namespace binfilter {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// DRAGVIEW
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class SdrDragView: public SdrExchangeView
{
    friend class                SdrPageView;

protected:
    unsigned                    bMarkedHitMovesAlways : 1; // Persistent
    unsigned                    bDragStripes : 1;    // Persistent
    unsigned                    bNoDragHdl : 1;      // Persistent - Handles waehrend des Draggens verstecken
    unsigned                    bMirrRefDragObj : 1; // Persistent - Waehrend des Draggens der Spiegelachse die gespiegelten Objekte als Xor zeigen
    unsigned                    bSolidDragging : 1;  // Dragging und Create in Echtzeit erlaubt
    unsigned                    bDragWithCopy : 1;
    unsigned                    bNoDragXorPolys : 1;

private:
    void ImpClearVars();

protected:
    virtual void WriteRecords(SvStream& rOut) const;
    virtual BOOL ReadRecord(const SdrIOHeader& rViewHead, const SdrNamedSubRecord& rSubHead, SvStream& rIn);

public:
    SdrDragView(SdrModel* pModel1, OutputDevice* pOut=NULL);
    virtual ~SdrDragView();

    virtual void ToggleShownXor(OutputDevice* pOut, const Region* pRegion) const;
    virtual BOOL IsAction() const;
    virtual void BrkAction();

    // Interaktives einfuegen eines neuen Punktes. nIdx=0 => vor dem ersten Punkt.
    BOOL IsInsObjPoint() const { return FALSE; }    // DBG_BF_ASSERT

    // Begrenzungslinien ueber's gesamte Win waehrend des Draggens
    // Persistent. Default=FALSE.
    void SetDragStripes(BOOL bOn);
    BOOL IsDragStripes() const { return bDragStripes; }

    // Beim Draggen werden i.d.R. die Konturen der markierten Objekte
    // als Xor-Polygone dargestellt. Wird dieses Flag hier gesetzt,
    // wird (z.B. bei Mehrfachselektion) nur ein Xor-Frame gezeichnet.
    // Bei objektspeziefischem Dragging (Polygonpunkte,Eckenradius,...
    // hat diese Einstellung keine Auswirkung.
    // Auch waerend des Draggens umschaltbar.
    // Default=Off
    void SetNoDragXorPolys(BOOL bOn);
    BOOL IsNoDragXorPolys() const { return bNoDragXorPolys; }

    void SetSolidDragging(BOOL bOn) { bSolidDragging = bOn; }
    BOOL IsSolidDragging() const { return bSolidDragging; }

    // Ist ein spezieller Dragmode eingeschaltet, wie Rotate, Mirror oder Crook,
    // dann leitet ein Hit auf das markierte Objekt genau dieses Dragging ein.
    // Setzt man MarkedHitMovesAlways auf TRUE, so leitet ein Hit auf das
    // markierte Objekt immer ein Moven ein, unabhaengig vom gesetzten DragMode.
    // Dieses Flag ist persistent und sollte von der App fuer den Anwender
    // konfigurierbar sein!
    void SetMarkedHitMovesAlways(BOOL bOn) { bMarkedHitMovesAlways = bOn; }
    BOOL IsMarkedHitMovesAlways() const { return bMarkedHitMovesAlways; }
};

}//end of namespace binfilter
#endif //_SVDDRGV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
