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

#ifndef _SVDOSMEV_HXX
#define _SVDOSMEV_HXX

#include <bf_svtools/bf_solar.h>

#include <tools/bigint.hxx>

#include <vcl/field.hxx>
class Date;
class Time;
class DateTime;
namespace binfilter {

////////////////////////////////////////////////////////////////////////////////////////////////////

class SdrObject;
class SdrObjSmartData;
class ImpSdrExprTokenList;
class ImpSdrExprToken;
class SdrObjList;
class SdrSmartGeometricList;
class SdrSmartGeometric;
class SdrSmartGeometricLine;
class SdrSmartDraftLineList;
class SdrSmartDraftLine;
class SdrSmartGluePointList;
class SdrSmartGluePoint;
class SdrSmartDragPointList;
class SdrSmartDragPoint;

// D:\PU\EvalExpr.Pas

enum SdrExprErrKind {
    SDREXPRERR_NONE,            // kein Fehler
    SDREXPRERR_DIVZERO,         // Division durch 0
    SDREXPRERR_CIRCREF,         // Zirkulare Referenz
    SDREXPRERR_VALUEEXPECTED,   // Wert erwartet
    SDREXPRERR_VAREXPECTED,     // Feld oder Speichervariable erwartet
    SDREXPRERR_VALUETOOBIG,     // Zahlenwert zu gross
    SDREXPRERR_UNEXPECTEDCHAR,  // unerwartetes Zeichen
    SDREXPRERR_UNEXPECTEDEOLN,  // unerwartetes Zeilenende
    SDREXPRERR_UNKNOWNIDENT,    // Unbekannte Funktion oder Variable
    SDREXPRERR_BRKOPEXPECTED,   // ( erwartet    Die 6 Klammern
    SDREXPRERR_BRKCLEXPECTED,   // ) erwartet    muessen genau
    SDREXPRERR_SQBRKOPEXPECTED, // [ erwartet    in dieser
    SDREXPRERR_SQBRKCLEXPECTED, // ] erwartet    Reihenfolge und
    SDREXPRERR_SPBRKOPEXPECTED, // { erwartet    Zusammenhaengend
    SDREXPRERR_SPBRKCLEXPECTED, // } erwartet    stehen bleiben!
    SDREXPRERR_PARAMEXPECTED,   // ; und Parameter erwartet
    SDREXPRERR_ELSEEXPECTED,    // : erwartet
    SDREXPRERR_MEMBEREXPECTED,  // . oder Member erwartet
    SDREXPRERR_FALSEMEMBER,     // Falscher Klassen-Member
    SDREXPRERR_BASEMUSTBEINT,   // Nur ganzzahlige Basen erlaubt
    SDREXPRERR_BASEMUSTBEPOS,   // Nur positive Basen erlaubt
    SDREXPRERR_DUPLICATEBASE,   // Nur eine Basis je Wert erlaubt
    SDREXPRERR_BASECLEXPECTED,  // ] erwartet wegen Zahlenbasis
    SDREXPRERR_INVALIDFUNCARG,  // Ung�ltiges Funktionsargument
    SDREXPRERR_INVALIDDIGIT,    // Unerlaubte Ziffer
    SDREXPRERR_COMPLEXVALUE,    // x^y: negative Basis, gebrochener Exponent oder Wurzel(-val)
    SDREXPRERR_DOUBLEDECPOINT   // Zu viele Dezimalpunkte
};

enum SdrExprValueKind {
    SDREXPRVALUE_SIMPLE,     // Normale Zahl
    SDREXPRVALUE_COMPLEX,    // Komplexe Zahl
    SDREXPRVALUE_UNITS,      // eine Entfernung
    SDREXPRVALUE_ANGLE,      // ein Winkel in 1/100deg
    // ab hier nur temporaere Typen fuer Evaluate
    SDREXPRVALUE_POINT,      // ein Punkt (Units)
    SDREXPRVALUE_DATE,       // ein Datum
    SDREXPRVALUE_TIME,       // eine Uhrzeit
    SDREXPRVALUE_DATETIME    // ein Datum (Y) mit Uhrzeit (X)
};

// todo: FieldPointer fuer Zuweisungen
class SdrExprValue {
friend class ImpSdrSmartObjEvaluator;
//public:
protected:
    BigInt    nX,nY; //,nX2,nY2;
    double    fX,fY; //,fX2,fY2;
    ULONG     nMSecsUsed;
    bool  bFloat; // Switch zwischen BigInt und Float (fuer alle 4 Werte)
    bool  bUIMap;
    bool  bDirty;
    bool  bHasValue;
    bool  bLocked;
    bool  bIsField;     // RTTI-Sparversion
    bool  bIsFieldPtr;  // Ist Zeiger auf ein Feld oder 2 Felder (X/Y)
    SdrExprValueKind eKind;
    FieldUnit eUIMap;
    MapUnit   eCoreMap;
    USHORT    nUnitPower;
private:
    virtual void Undirty(const SdrObject* pObj);
    void CTor();
    bool HasUnit() const                    { return nUnitPower!=0; }
    bool IsSameUnit(const SdrExprValue& rVal) const;
public:
    SdrExprValue()                              { CTor(); }
    SdrExprValue(double fVal)                   { CTor(); SetValue(fVal);         }
    SdrExprValue(long nVal)                     { CTor(); SetValue(nVal);         }
    SdrExprValue(const BigInt& nVal)            { CTor(); SetValue(nVal);         }
    SdrExprValue(double fVal, FieldUnit eU)     { CTor(); SetUnits(fVal,eU);      }
    SdrExprValue(long nVal, FieldUnit eU)       { CTor(); SetUnits(nVal,eU);      }
    SdrExprValue(const BigInt& n, FieldUnit eU) { CTor(); SetUnits(n,eU);         }
    SdrExprValue(double fVal, MapUnit eU)       { CTor(); SetUnits(fVal,eU);      }
    SdrExprValue(long nVal, MapUnit eU)         { CTor(); SetUnits(nVal,eU);      }
    SdrExprValue(const BigInt& n, MapUnit eU)   { CTor(); SetUnits(n,eU);         }
    SdrExprValue(const Point& P, MapUnit eU)    { CTor(); SetUnits(P,eU);         }
    SdrExprValue(bool bVal)                 { CTor(); SetBool(bVal);          }
    SdrExprValue(const Time& rTime)             { CTor(); SetTime(rTime);         }
    SdrExprValue(const Date& rDate)             { CTor(); SetDate(rDate);         }
    SdrExprValue(const DateTime& rDateTime)     { CTor(); SetDateTime(rDateTime); }
    virtual ~SdrExprValue();

    const SdrExprValue& ForceUndirty(const SdrObject* pObj) const { if (bDirty) ((SdrExprValue*)this)->Undirty(pObj); return *this; }
    void             SetDirty()                             { bDirty=TRUE; }
    bool         IsDirty() const                        { return bDirty; }
    bool         IsField() const                        { return bIsField; }
    bool         IsFieldPtr() const                     { return bIsFieldPtr; }

    bool         IsLocked(const SdrObject* pObj) const  { ForceUndirty(pObj); return bLocked; }
    ULONG            GetMSecsUsed(const SdrObject* pO) const{ ForceUndirty(pO  ); return nMSecsUsed; }
    bool         HasValue(const SdrObject* pObj) const  { ForceUndirty(pObj); return bHasValue; }
    SdrExprValueKind GetExprKind(const SdrObject* pObj) const{ForceUndirty(pObj); return eKind; }
    bool         IsLong(const SdrObject* pObj) const    { ForceUndirty(pObj); return !bFloat /*&& !nX.IsLong()*/; }
    bool         IsBigInt(const SdrObject* pObj) const  { ForceUndirty(pObj); return !bFloat /*&& nX.IsLong()*/; }
    bool         IsFloat(const SdrObject* pObj) const   { ForceUndirty(pObj); return bFloat; }
    double           GetFloat(const SdrObject* pObj) const  { ForceUndirty(pObj); return fX; }
    const BigInt&    GetBigInt(const SdrObject* pObj) const { ForceUndirty(pObj); return nX; }
    long             GetLong(const SdrObject* pObj) const   { ForceUndirty(pObj); return long(nX); }
    bool         GetBool(const SdrObject* pObj) const   { ForceUndirty(pObj); return !nX.IsZero(); }
    MapUnit          GetCoreMap(const SdrObject* pObj) const{ ForceUndirty(pObj); return eCoreMap; }
    FieldUnit        GetUIMap(const SdrObject* pObj) const  { ForceUndirty(pObj); return eUIMap; }
    bool         QuickIsLocked() const                  { return bLocked; }
    ULONG            QuickGetMSecsUsed() const              { return nMSecsUsed; }
    bool         QuickHasValue() const                  { return bHasValue; }
    SdrExprValueKind QuickGetExprKind() const               { return eKind; }
    bool         QuickIsLong() const                    { return !bFloat /*&& !nX.IsLong()*/; }
    bool         QuickIsBigInt() const                  { return !bFloat /*&& nX.IsLong()*/; }
    bool         QuickIsFloat() const                   { return bFloat; }
    double           QuickGetFloat() const                  { return fX; }
    const BigInt&    QuickGetBigInt() const                 { return nX; }
    long             QuickGetLong() const                   { return long(nX); }
    bool         QuickGetBool() const                   { return !nX.IsZero(); }
    MapUnit          QuickGetCoreMap() const                { return eCoreMap; }
    FieldUnit        QuickGetUIMap() const                  { return eUIMap; }

    long             QuickGetUnits(const BigInt& rVal, MapUnit eU) const;
    long             QuickGetUnits(double nVal, MapUnit eU) const;
    double           QuickGetFloatNoUnits(const BigInt& rVal) const;
    double           QuickGetFloatNoUnits(double nVal) const;

    long             GetUnits(MapUnit eU, const SdrObject* pObj) const  { ForceUndirty(pObj); return QuickGetUnits(eU); }
    long             QuickGetUnits(MapUnit eU) const;
    long             QuickGetLongNoUnits() const { return (long)QuickGetFloatNoUnits(); }
    double           QuickGetFloatNoUnits() const;

    Time             QuickGetTime() const;
    Date             QuickGetDate() const;
    DateTime         QuickGetDateTime() const;

    bool         QuickIsDate() const       { return eKind==SDREXPRVALUE_DATE; }
    bool         QuickIsTime() const       { return eKind==SDREXPRVALUE_TIME; }
    bool         QuickIsDateTime() const   { return eKind==SDREXPRVALUE_DATETIME; }
    bool         QuickIsDateOrTime() const { return eKind==SDREXPRVALUE_DATE || eKind==SDREXPRVALUE_TIME || eKind==SDREXPRVALUE_DATETIME; }
    BigInt           QuickGetDateTimeMSecs() const;

    void SetExprKind(SdrExprValueKind eNew)  { eKind=eNew; }

    void Negate();

    void SetValue(double fVal);
    void SetValue(long nVal);
    void SetValue(const BigInt& nVal);

    void SetUnits(double fVal, FieldUnit eU);
    void SetUnits(long nVal, FieldUnit eU);
    void SetUnits(const BigInt& n, FieldUnit eU);

    void SetUnits(double fVal, MapUnit eU);
    void SetUnits(long nVal, MapUnit eU);
    void SetUnits(const BigInt& n, MapUnit eU);
    void SetUnits(const Point& P, MapUnit eU);

    void SetBool(bool bVal);
    void SetTime(const Time& rTime);
    void SetDate(const Date& rDate);
    void SetDateTime(const DateTime& rDateTime);

/*  const SdrObject*             GetObject       () const { return (const SdrObject*            )pPointer; }
    const SdrObjList*            GetObjList      () const { return (const SdrObjList*           )pPointer; }
    const SdrSmartGeometricList* GetGeometricList() const { return (const SdrSmartGeometricList*)pPointer; }
    const SdrSmartGeometric*     GetGeometricArea() const { return (const SdrSmartGeometric*    )pPointer; }
    const SdrSmartGeometricLine* GetGeometricLine() const { return (const SdrSmartGeometricLine*)pPointer; }
    const SdrSmartDraftLineList* GetDraftArea    () const { return (const SdrSmartDraftLineList*)pPointer; }
    const SdrSmartDraftLine*     GetDraftLine    () const { return (const SdrSmartDraftLine*    )pPointer; }
    const SdrSmartGluePointList* GetGlueArea     () const { return (const SdrSmartGluePointList*)pPointer; }
    const SdrSmartGluePoint*     GetGlueLine     () const { return (const SdrSmartGluePoint*    )pPointer; }
    const SdrSmartDragPointList* GetDragArea     () const { return (const SdrSmartDragPointList*)pPointer; }
    const SdrSmartDragPoint*     GetDragLine     () const { return (const SdrSmartDragPoint*    )pPointer; }
    void SetObject       (const SdrObject*             pObj      ) { pPointer=pObj;       eKind=SDREXPRVALUE_OBJECT;        }
    void SetObjList      (const SdrObjList*            pObjList  ) { pPointer=pObjList;   eKind=SDREXPRVALUE_OBJLIST;       }
    void SetGeometricList(const SdrSmartGeometricList* pGeos     ) { pPointer=pGeos;      eKind=SDREXPRVALUE_GEOMETRICS;    }
    void SetGeometricArea(const SdrSmartGeometric*     pGeo      ) { pPointer=pGeo;       eKind=SDREXPRVALUE_GEOMETRIC;     }
    void SetGeometricLine(const SdrSmartGeometricLine* pGeoLine  ) { pPointer=pGeoLine;   eKind=SDREXPRVALUE_GEOLINE;       }
    void SetDraftArea    (const SdrSmartDraftLineList* pDraftArea) { pPointer=pDraftArea; eKind=SDREXPRVALUE_DRAFTAREA;     }
    void SetDraftLine    (const SdrSmartDraftLine*     pDraftLine) { pPointer=pDraftLine; eKind=SDREXPRVALUE_DRAFTAREALINE; }
    void SetGlueArea     (const SdrSmartGluePointList* pGlueArea ) { pPointer=pGlueArea;  eKind=SDREXPRVALUE_GLUEAREA;      }
    void SetGlueLine     (const SdrSmartGluePoint*     pGlueLine ) { pPointer=pGlueLine;  eKind=SDREXPRVALUE_GLUEAREALINE;  }
    void SetDragArea     (const SdrSmartDragPointList* pDragArea ) { pPointer=pDragArea;  eKind=SDREXPRVALUE_DRAGAREA;      }
    void SetDragLine     (const SdrSmartDragPoint*     pDragLine ) { pPointer=pDragLine;  eKind=SDREXPRVALUE_DRAGAREALINE;  }*/

    void operator+=(const SdrExprValue& rVal);
    void operator-=(const SdrExprValue& rVal);
    void operator*=(const SdrExprValue& rVal);
    void operator/=(const SdrExprValue& rVal);
    void operator%=(const SdrExprValue& rVal);
    bool operator==(const SdrExprValue& rVal) const;
    bool operator!=(const SdrExprValue& rVal) const    { return !operator==(rVal); }
    bool operator==(double fVal) const;
    bool operator!=(double fVal) const                 { return !operator==(fVal); }
    bool operator==(long nVal) const;
    bool operator!=(long nVal) const                   { return !operator==(nVal); }
    bool operator==(const BigInt& nVal) const;
    bool operator!=(const BigInt& nVal) const          { return !operator==(nVal); }
    bool operator> (const SdrExprValue& rVal) const;
    bool operator>=(const SdrExprValue& rVal) const;
    bool operator< (const SdrExprValue& rVal) const    { return !operator>=(rVal); }
    bool operator<=(const SdrExprValue& rVal) const    { return !operator>(rVal); }
    bool operator> (double fVal) const;
    bool operator>=(double fVal) const;
    bool operator< (double fVal) const                 { return !operator>=(fVal); }
    bool operator<=(double fVal) const                 { return !operator>(fVal); }
    bool operator> (long nVal) const;
    bool operator>=(long nVal) const;
    bool operator< (long nVal) const                   { return !operator>=(nVal); }
    bool operator<=(long nVal) const                   { return !operator>(nVal); }
    bool operator> (const BigInt& nVal) const;
    bool operator>=(const BigInt& nVal) const;
    bool operator< (const BigInt& nVal) const          { return !operator>=(nVal); }
    bool operator<=(const BigInt& nVal) const          { return !operator>(nVal); }
};

/*class ImpSdrSmartObjEvaluator0 {
    String aStr;
    const SdrObject* pObj;
    const SdrObjSmartData* pSmart;
    SdrExprErrKind eError;
    USHORT nErrPos;
    USHORT nPos;
    char   cAktChar;
    SdrExprValue aResult;
    //bool bHasValue;
    //bool bLocked;
    bool bMetric;
    bool bMetricX;
    bool bMetricY;
    //double fValue;
    //long   nValue;
    //long   nUnitValue;
    FieldUnit eUIMap;
    MapUnit   eCoreMap;
private:
    void Error(USHORT nEPos, SdrExprErrKind eECode);
    bool CharsLeft() const { return nPos<aStr.Len(); }
    void KillSpaces();
    char NextChar();
    SdrExprValue Func();
    SdrExprValue Valu();
    SdrExprValue Wert();
    SdrExprValue Fakt();
    SdrExprValue Term();
    SdrExprValue Expr();
public:
    ImpSdrSmartObjEvaluator0(const SdrObject* pObj_);
    void SetMetric(bool bOn)  { bMetric=bOn; bMetricX=FALSE; bMetricY=FALSE; }
    void SetMetricX(bool bOn) { bMetric=bOn; bMetricX=TRUE;  bMetricY=FALSE; }
    void SetMetricY(bool bOn) { bMetric=bOn; bMetricX=FALSE; bMetricY=TRUE;  }
    bool Evaluate(const String& rStr);
//    double   GetFloatValue() const  { return aResult.fValue; }
//    long     GetIntValue() const    { return aResult.nValue; }
//    long     GetUnitValue() const   { return aResult.nUnitValue; }
    bool HasValue() const       { return aResult.HasValue(pObj); }
    bool IsLocked() const       { return aResult.IsLocked(pObj); }
    bool IsError() const        { return eError!=SDREXPRERR_NONE; }
    SdrExprErrKind GetError() const { return eError; }
    USHORT   GetErrorPos() const    { return nErrPos; }
    const SdrExprValue& GetResult() const { return aResult; }
};*/

class ImpSdrSmartObjEvaluator {
friend class ImpZwischenergebnis;
    const ImpSdrExprTokenList* pTokenList;
    const SdrObject* pObj;
    const SdrObjSmartData* pSmart;
    const ImpSdrExprToken* pAktTok;
    const SdrExprValue* pTargetField; // Zur Bestimmung der AktLine, ..., anhand pObj
    SdrExprErrKind eError;
    ULONG          nErrPos;
    ULONG          nAnz;
    ULONG          nPos;
    SdrExprValue   aResult;
    bool       bMetric;
    bool       bMetricX;
    bool       bMetricY;
    FieldUnit      eUIMap;
    MapUnit        eCoreMap;
    // Feldlokation in Geo,Draft,Vars,Glue oder Drag
    bool       bFieldLocationSearched;
    bool       bFieldLocationFound;
    USHORT         nFieldArea; // eigentlich enum ...
    ULONG          nGeoFieldAreaNum;
    ULONG          nFieldLine;
    ULONG          nFieldColumn;
    ULONG          nMSecsUsed; // Falls nur Date, dann =86400000 (ms/Tag)
    bool       bLocked;
private:
    void         SetMSecsUsed(ULONG n);
    void         SearchFieldLocation();
    void         ForceFieldLocation() const { if (!bFieldLocationSearched) ((ImpSdrSmartObjEvaluator*)this)->SearchFieldLocation(); }
    void         Error(ULONG nEPos, SdrExprErrKind eECode);
    bool     TokensLeft() const { return nPos<nAnz; }
    const ImpSdrExprToken* NextToken();
    bool     BracketOpen();
    bool     BracketClose(USHORT eOpen);
    bool     NextArg(SdrExprValue*& pArg, bool bSkip);
    void         FuncRound(bool bSkip, double nRet, SdrExprValue* pRet, SdrExprValue*& pArg);
    void         FuncRandom(bool bSkip, bool bBracket, double nRet, SdrExprValue* pRet, SdrExprValue*& pArg);
    SdrExprValue* Memb(bool bSkip);
    SdrExprValue* Func(bool bSkip);
    SdrExprValue* Wert(bool bSkip);
    SdrExprValue* Fakt(bool bSkip);
    SdrExprValue* Term(bool bSkip);
    SdrExprValue* Expr2(bool bSkip);
    SdrExprValue* CompTerm(bool bSkip);
    SdrExprValue* BoolFakt(bool bSkip);
    SdrExprValue* BoolXTerm(bool bSkip);
    SdrExprValue* BoolTerm(bool bSkip);
    SdrExprValue* Condition(bool bSkip);
    SdrExprValue* Expr(bool bSkip);
public:
    ImpSdrSmartObjEvaluator(const SdrObject* pObj_);
    void SetMetric(bool bOn)  { bMetric=bOn; bMetricX=FALSE; bMetricY=FALSE; }
    void SetMetricX(bool bOn) { bMetric=bOn; bMetricX=TRUE;  bMetricY=FALSE; }
    void SetMetricY(bool bOn) { bMetric=bOn; bMetricX=FALSE; bMetricY=TRUE;  }
    bool Evaluate(const ImpSdrExprTokenList* pTokenList_, const SdrExprValue* pTargetField_);
    bool HasValue() const       { return aResult.HasValue(pObj); }
    bool IsLocked() const       { return aResult.IsLocked(pObj); }
    bool IsError() const        { return eError!=SDREXPRERR_NONE; }
    SdrExprErrKind GetError() const { return eError; }
    static XubString GetErrorStr(SdrExprErrKind eErr);
    XubString GetErrorStr() const    { return GetErrorStr(eError); }
    ULONG    GetErrorPos() const    { return nErrPos; }
    const SdrExprValue& GetResult() const { return aResult; }
    MapUnit GetCoreMap() const      { return eCoreMap; }
    const SdrObject* GetObject() const { return pObj; }
    const SdrObjSmartData* GetSmartData() const { return pSmart; }
    ULONG GetMSecsUsed() const  { return nMSecsUsed; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}//end of namespace binfilter
#endif //_SVDOSMEV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
