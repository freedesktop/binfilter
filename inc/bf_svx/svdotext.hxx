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

#ifndef _SVDOTEXT_HXX
#define _SVDOTEXT_HXX

#include <bf_svtools/bf_solar.h>

#include <vcl/field.hxx>

#include <bf_svx/svdoattr.hxx>

#include <bf_svx/svdtrans.hxx> // GeoStat

#include <tools/datetime.hxx>

#include <bf_svx/xtxasit.hxx>

#include <bf_svx/xtextit0.hxx>

#include <vector>
namespace binfilter {

//************************************************************
//   Vorausdeklarationen
//************************************************************

class OutlinerParaObject;
class SdrOutliner;
class SdrTextObj;
class SvxFieldItem;
class ImpSdrObjTextLink;

//************************************************************
//   Defines
//************************************************************

#define SDRUSERDATA_OBJTEXTLINK         (1)

//************************************************************
//   Hilfsklasse SdrTextObjGeoData
//************************************************************

class SdrTextObjGeoData : public SdrObjGeoData
{
public:
    Rectangle                   aRect;
    GeoStat                     aGeo;
};

//************************************************************
//   Hilfsklasse ImpSdrObjTextLinkUserData
//************************************************************

class ImpSdrObjTextLinkUserData : public SdrObjUserData
{
    friend class                SdrTextObj;
    friend class                ImpSdrObjTextLink;

    SdrTextObj*                 pObj;
    String                      aFileName;   // Name des referenzierten Dokuments
    String                      aFilterName; // ggf. ein Filter
    DateTime                    aFileDate0;  // Unnoetiges neuladen vermeiden
    ImpSdrObjTextLink*          pLink;
    rtl_TextEncoding            eCharSet;

public:
    TYPEINFO();
    ImpSdrObjTextLinkUserData(SdrTextObj* pObj1);
    virtual ~ImpSdrObjTextLinkUserData();

    virtual SdrObjUserData* Clone(SdrObject* pObj1) const;
    virtual void ReadData(SvStream& rIn);
    virtual void AfterRead();
};

//************************************************************
//   SdrTextObj
//************************************************************

class SdrTextObj : public SdrAttrObj
{
    friend class                ImpTextPortionHandler;
    friend class                ImpSdrObjTextLink;
    friend class                ImpSdrObjTextLinkUserData;
    friend class                SdrPowerPointImport; // fuer PowerPointImport
    friend class                SdrExchangeView; // fuer ImpGetDrawOutliner
    friend class                SdrView;         // fuer ImpGetDrawOutliner
    friend class                SdrObjEditView;  // fuer TextEdit
    friend class                SdrMeasureObj;   // fuer ImpGetDrawOutliner
    friend class                SvxMSDffManager; // fuer ImpGetDrawOutliner

protected:
    // Das aRect ist gleichzeig auch das Rect vom RectObj und CircObj.
    // Bei bTextFrame=TRUE wird der Text in dieses Rect hineinformatiert.
    // Bei bTextFrame=FALSE wird der Text am Mittelpunkt des Rect zentriert.
    Rectangle                   aRect;

    // Der GeoStat enthaelt den Drehwinkel und einen Shearwinkel
    GeoStat                     aGeo;

    // Im pOutlinerParaObject steckt der Text drin
    OutlinerParaObject*         pOutlinerParaObject;

    // Hier merke ich mir die Ausmasse des Textes (n.i.)
    Size                        aTextSize;

    // Bei Fontwork muss soviel auf's BoundRect draufgerechnet werden
    // damit es ausreichend gross ist.
    Rectangle*                  pFormTextBoundRect;

    // Moegliche Werte fuer eTextKind sind:
    //     OBJ_TEXT         normaler Textrahmen
    //     OBJ_TEXTEXT      Textfortsetzungsrahmen
    //     OBJ_TITLETEXT    TitleText fuer StarDraw-Praesentation
    //     OBJ_OUTLINETEXT  OutlineText fuer StarDraw-Praesentation
    // eTextKind hat nur Bedeutung, wenn bTextFrame=TRUE, da es sich sonst
    // um ein beschriftetes Grafikobjekt handelt.
    SdrObjKind                  eTextKind;

protected:
    // Fuer beschriftete Zeichenobjekte ist bTextFrame=FALSE. Der Textblock
    // wird dann hoizontal und vertikal an aRect zentriert. Bei bTextFrame=
    // TRUE wird der Text in aRect hineinformatiert. Der eigentliche Textrahmen
    // ist durch ein SdrRectObj mit bTextFrame=TRUE realisiert.
    BOOL                        bTextFrame : 1;
    BOOL                        bPortionInfoChecked : 1; // Fuer Optimierung von Textobjekten
    BOOL                        bNoShear : 1;            // Obj darf nicht gesheart werden   (->Graf+Ole+TextFrame)
    BOOL                        bNoMirror : 1;           // Obj darf nicht gespiegelt werden (->Ole,TextFrame)
    BOOL                        bTextSizeDirty : 1;

private:
    void ImpCheckMasterCachable();
    // #101029#: Extracted from ImpGetDrawOutliner()
    void ImpInitDrawOutliner( SdrOutliner& rOutl ) const;
    // #101029#: Extracted from Paint()
    SdrOutliner& ImpGetDrawOutliner() const;
    void ImpLinkAnmeldung();
    void ImpLinkAbmeldung();
    ImpSdrObjTextLinkUserData* GetLinkUserData() const;

protected:
    bool ImpCanConvTextToCurve() const { return pOutlinerParaObject!=NULL && pModel!=NULL && !IsOutlText() && !IsFontwork(); }
    void ImpConvertSetAttrAndLayer(SdrObject* pObj, bool bNoSetAttr=FALSE) const;
    SdrObject* ImpConvertMakeObj(const XPolyPolygon& rXPP, bool bClosed, bool bBezier, bool bNoSetAttr=FALSE) const;
    SdrObject* ImpConvertAddText(SdrObject* pObj, bool bBezier) const;
    void ImpSetTextStyleSheetListeners();
    void ImpAddTextToBoundRect();
    void ImpJustifyRect(Rectangle& rRect) const;
    void ImpCheckShear();
    void SetTextSizeDirty() { bTextSizeDirty=TRUE; }

    // rAnchorRect ist InOut-Parameter!

    virtual void SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType);
    virtual void SaveGeoData(SdrObjGeoData& rGeo) const;
    virtual void RestGeoData(const SdrObjGeoData& rGeo);
    bool NbcSetEckenradius(long nRad);
    bool NbcSetAutoGrowHeight(bool bAuto);
    bool NbcSetMinTextFrameHeight(long nHgt);
    bool NbcSetMinTextFrameWidth(long nWdt);

    // Konstruktoren fuer beschriftete Zeichenobjekte
    SdrTextObj();
    SdrTextObj(const Rectangle& rNewRect);

    // Konstruktoren fuer Textrahmen
    SdrTextObj(SdrObjKind eNewTextKind);
    SdrTextObj(SdrObjKind eNewTextKind, const Rectangle& rNewRect);

    virtual ~SdrTextObj();

public:
    TYPEINFO();

    // via eCharSet kann der CharSet der vorliegenden Datei uebergeben werden.
    // Bei RTL_TEXTENCODING_DONTKNOW wird der CharSet der aktuellen Plattform verwendet.
    // Derzeit unterstuetzt wird ASCII und RTF wobei ich die Unterscheidung
    // selbst treffe. rFilterName ist noch ohne Bedeutung und muss leer gelassen
    // werden.
    // I.d.R. muss die App nur SetTextLink() rufen - der Rest geschieht von
    // selbst (SfxLinkManager). Die Methode LoadText() kann ausserdem verwendet
    // werden, um eine Datei in ein Textobjekt zu laden (ohne Verknuepfung).
    // TextLinks koennen nicht editiert werden (allenfalls spaeter mal ReadOnly).
    // Eine Attributierung kann nur am Textrahmen vollzogen werden.
    bool IsLinkedText() const { return pPlusData!=NULL && GetLinkUserData()!=NULL; }

    bool AdjustTextFrameWidthAndHeight(Rectangle& rR, bool bHgt=TRUE, bool bWdt=TRUE) const;
    bool NbcAdjustTextFrameWidthAndHeight(bool bHgt=TRUE, bool bWdt=TRUE);
    bool AdjustTextFrameWidthAndHeight(bool bHgt=TRUE, bool bWdt=TRUE);
    bool IsTextFrame() const { return bTextFrame; }
    bool IsOutlText() const { return bTextFrame && (eTextKind==OBJ_OUTLINETEXT || eTextKind==OBJ_TITLETEXT); }
    BOOL GetTextKind() const { return eTextKind; }
    bool HasText() const { return pOutlinerParaObject != NULL; }

    // FitToSize und Fontwork wird bei GetTextSize() nicht berueksichtigt!
    virtual const Size& GetTextSize() const;

    // Gleichzeitig wird der Text in den Outliner gesetzt (ggf.
    // der des EditOutliners) und die PaperSize gesetzt.
    virtual void TakeTextRect( SdrOutliner& rOutliner, Rectangle& rTextRect,
        Rectangle* pAnchorRect ) const;
    virtual void TakeTextAnchorRect(Rectangle& rAnchorRect) const;
    inline long GetEckenradius() const;
    bool IsAutoGrowHeight() const;
    inline long GetMinTextFrameHeight() const;
    inline long GetMaxTextFrameHeight() const;
    bool IsAutoGrowWidth() const;
    inline long GetMinTextFrameWidth() const;
    inline long GetMaxTextFrameWidth() const;
    SdrFitToSizeType GetFitToSize() const;

    // Feststellen, ob TextFontwork
    inline bool IsFontwork() const;

    // Soll die Fontwork-Kontour versteckt werden?
    inline bool IsHideContour() const;

    // Textfluss innerhalb Kontur
    inline bool IsContourTextFrame() const;

    // Horizontale Textausrichtung
    SdrTextHorzAdjust GetTextHorizontalAdjust() const;

    // Vertikale Textausrichtung
    SdrTextVertAdjust GetTextVerticalAdjust() const;

    // Textrahmenabstaende
    inline long GetTextLeftDistance() const;
    inline long GetTextRightDistance() const;
    inline long GetTextUpperDistance() const;
    inline long GetTextLowerDistance() const;
    inline SdrTextAniKind GetTextAniKind() const;
    inline SdrTextAniDirection GetTextAniDirection() const;

    virtual void SetPage(SdrPage* pNewPage);
    virtual void SetModel(SdrModel* pNewModel);

    // Wird zur Bestimmung des Textankerbereichs benoetigt
    virtual void TakeUnrotatedSnapRect(Rectangle& rRect) const;
    virtual SdrObject* CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const;
    virtual void operator=(const SdrObject& rObj);
    using SdrObject::TakeContour;
    virtual void TakeContour(XPolyPolygon& rPoly) const;
    virtual void RecalcSnapRect();
    virtual void NbcSetSnapRect(const Rectangle& rRect);
    virtual void NbcSetLogicRect(const Rectangle& rRect);
    virtual const Rectangle& GetLogicRect() const;
    virtual long GetRotateAngle() const;
    virtual long GetShearAngle(bool bVertical=FALSE) const;




    virtual void NbcMove(const Size& rSiz);
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    virtual void NbcRotate(const Point& rRef, long nWink, double sn, double cs);
    virtual void NbcShear(const Point& rRef, double tn, bool bVShear);

    virtual bool HasTextEdit() const;
    virtual bool BegTextEdit(SdrOutliner& rOutl);
    virtual void EndTextEdit(SdrOutliner& rOutl);
    virtual SdrObject* CheckTextEditHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const;

    virtual void NbcSetOutlinerParaObject(OutlinerParaObject* pTextObject);
    virtual OutlinerParaObject* GetOutlinerParaObject() const;

    virtual void NbcReformatText();
    virtual void ReformatText();
    virtual void RestartAnimation(SdrPageView*) const {} // DBG_BF_ASSERT

    virtual bool CalcFieldValue(const SvxFieldItem& rField, USHORT nPara, USHORT nPos,
        bool bEdit, Color*& rpTxtColor, Color*& rpFldColor, String& rRet) const;


    virtual void NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr);

    // private support routines for ItemSet access. NULL pointer means clear item.
    virtual void ItemSetChanged(const SfxItemSet& rSet);
    virtual void ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem = 0);


    virtual void ReadData(const SdrObjIOHeader& rHead, SvStream& rIn);

    void ForceOutlinerParaObject();
    BOOL IsVerticalWriting() const;
    void SetVerticalWriting( BOOL bVertical );

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // transformation interface for StarOfficeAPI. This implements support for
    // homogen 3x3 matrices containing the transformation of the SdrObject. At the
    // moment it contains a shearX, rotation and translation, but for setting all linear
    // transforms like Scale, ShearX, ShearY, Rotate and Translate are supported.
    //
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // gets base transformation and rectangle of object. If it's an SdrPathObj it fills the PolyPolygon
    // with the base geometry and returns TRUE. Otherwise it returns FALSE.
    virtual BOOL TRGetBaseGeometry(Matrix3D& rMat, XPolyPolygon& rPolyPolygon) const;
    // sets the base geometry of the object using infos contained in the homogen 3x3 matrix.
    // If it's an SdrPathObj it will use the provided geometry information. The Polygon has
    // to use (0,0) as upper left and will be scaled to the given size in the matrix.
    virtual void TRSetBaseGeometry(const Matrix3D& rMat, const XPolyPolygon& rPolyPolygon);

};

//************************************************************
//   Inlines
//************************************************************

inline long SdrTextObj::GetEckenradius() const
{
    return ((SdrEckenradiusItem&)(GetItemSet().Get(SDRATTR_ECKENRADIUS))).GetValue();
}

inline long SdrTextObj::GetMinTextFrameHeight() const
{
    return ((SdrTextMinFrameHeightItem&)(GetItemSet().Get(SDRATTR_TEXT_MINFRAMEHEIGHT))).GetValue();
}

inline long SdrTextObj::GetMaxTextFrameHeight() const
{
    return ((SdrTextMaxFrameHeightItem&)(GetItemSet().Get(SDRATTR_TEXT_MAXFRAMEHEIGHT))).GetValue();
}

inline long SdrTextObj::GetMinTextFrameWidth() const
{
    return ((SdrTextMinFrameWidthItem&)(GetItemSet().Get(SDRATTR_TEXT_MINFRAMEWIDTH))).GetValue();
}

inline long SdrTextObj::GetMaxTextFrameWidth() const
{
    return ((SdrTextMaxFrameWidthItem&)(GetItemSet().Get(SDRATTR_TEXT_MAXFRAMEWIDTH))).GetValue();
}

inline bool SdrTextObj::IsFontwork() const
{
    return (bTextFrame) ? FALSE // Default ist FALSE
        : ((XFormTextStyleItem&)(GetItemSet().Get(XATTR_FORMTXTSTYLE))).GetValue()!=XFT_NONE;
}

inline bool SdrTextObj::IsHideContour() const
{
    return (bTextFrame) ? FALSE // Default ist: Nein, kein HideContour; HideContour nicht bei TextFrames
        : ((XFormTextHideFormItem&)(GetItemSet().Get(XATTR_FORMTXTHIDEFORM))).GetValue();
}

inline bool SdrTextObj::IsContourTextFrame() const
{
    return (bTextFrame) ? FALSE // ContourFrame nicht bei normalen TextFrames
        : ((SdrTextContourFrameItem&)(GetItemSet().Get(SDRATTR_TEXT_CONTOURFRAME))).GetValue();
}

inline long SdrTextObj::GetTextLeftDistance() const
{
    return ((SdrTextLeftDistItem&)(GetItemSet().Get(SDRATTR_TEXT_LEFTDIST))).GetValue();
}

inline long SdrTextObj::GetTextRightDistance() const
{
    return ((SdrTextRightDistItem&)(GetItemSet().Get(SDRATTR_TEXT_RIGHTDIST))).GetValue();
}

inline long SdrTextObj::GetTextUpperDistance() const
{
    return ((SdrTextUpperDistItem&)(GetItemSet().Get(SDRATTR_TEXT_UPPERDIST))).GetValue();
}

inline long SdrTextObj::GetTextLowerDistance() const
{
    return ((SdrTextLowerDistItem&)(GetItemSet().Get(SDRATTR_TEXT_LOWERDIST))).GetValue();
}

inline SdrTextAniKind SdrTextObj::GetTextAniKind() const
{
    return ((SdrTextAniKindItem&)(GetItemSet().Get(SDRATTR_TEXT_ANIKIND))).GetValue();
}

inline SdrTextAniDirection SdrTextObj::GetTextAniDirection() const
{
    return ((SdrTextAniDirectionItem&)(GetItemSet().Get(SDRATTR_TEXT_ANIDIRECTION))).GetValue();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}//end of namespace binfilter
#endif //_SVDOTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
