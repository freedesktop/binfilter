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


#include <tools/debug.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <bf_svx/unolingu.hxx>
#include <com/sun/star/text/ChapterFormat.hpp>
#include <com/sun/star/text/ReferenceFieldPart.hpp>
#include <com/sun/star/text/BibliographyDataField.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>


#include <cmdid.h>
#include <swtypes.hxx>
#include <shellres.hxx>
#include <viewsh.hxx>

#include <horiornt.hxx>

#include <doc.hxx>
#include <docary.hxx>
#include <poolfmt.hxx>
#include <poolfmt.hrc>
#include <pagedesc.hxx>
#include <unomap.hxx>

#include <osl/diagnose.h>

#include <unoobj.hxx>
#include <doctxm.hxx>
#include <txttxmrk.hxx>
#include <unocrsr.hxx>
#include <unostyle.hxx>
#include <ndtxt.hxx>
#include <unoidx.hxx>
#include <docsh.hxx>
#include <chpfld.hxx>
#include <hints.hxx>
#include <SwStyleNameMapper.hxx>
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;

using rtl::OUString;

//-----------------------------------------------------------------------------
String lcl_AnyToString(uno::Any rVal) throw(IllegalArgumentException)
{
    OUString sRet;
    if(!(rVal >>= sRet))
        throw IllegalArgumentException();
    return sRet;
}
//-----------------------------------------------------------------------------
sal_Int16 lcl_AnyToInt16(uno::Any rVal) throw(IllegalArgumentException)
{
    sal_Int16 nRet = 0;
    if(!(rVal >>= nRet))
        throw IllegalArgumentException();
    return nRet;
}
//-----------------------------------------------------------------------------
sal_Bool lcl_AnyToBool(uno::Any rVal) throw(IllegalArgumentException)
{
    sal_Bool bRet = sal_False;
    if(!(rVal >>= bRet))
        throw IllegalArgumentException();
    return bRet;
}

SwTOXMark* lcl_GetMark(SwTOXType* pType, const SwTOXMark* pOwnMark)
{
    SwClientIter aIter(*pType);
    SwTOXMark* pMark = (SwTOXMark*)aIter.First(TYPE(SwTOXMark));
    while( pMark )
    {
        if(pMark == pOwnMark)
            return pMark;
        else
            pMark = (SwTOXMark*)aIter.Next();
    }
    return 0;
}
//-----------------------------------------------------------------------------
void lcl_ReAssignTOXType(SwDoc* pDoc, SwTOXBase& rTOXBase, const OUString& rNewName)
{
    sal_uInt16 nUserCount = pDoc->GetTOXTypeCount( TOX_USER );
    const SwTOXType* pNewType = 0;
    for(sal_uInt16 nUser = 0; nUser < nUserCount; nUser++)
    {
        const SwTOXType* pType = pDoc->GetTOXType( TOX_USER, nUser );
        if(pType->GetTypeName().Equals((String)rNewName))
        {
            pNewType = pType;
            break;
        }
    }
    if(!pNewType)
    {
        SwTOXType aNewType(TOX_USER, rNewName);
        pNewType = pDoc->InsertTOXType( aNewType );
    }
    //has to be non-const-casted
    ((SwTOXType*)pNewType)->Add(&rTOXBase);
}
//-----------------------------------------------------------------------------
static const char cUserDefined[] = "User-Defined";
static const char cUserSuffix[] = " (user)";
#define USER_LEN 12
#define USER_AND_SUFFIXLEN 19

void lcl_ConvertTOUNameToProgrammaticName(OUString& rTmp)
{
    ShellResource* pShellRes = ViewShell::GetShellRes();

    if(rTmp.equals(pShellRes->aTOXUserName))
        rTmp = OUString(C2U(cUserDefined));
    //if the version is not English but the alternative index's name is "User-Defined"
    //a " (user)" is appended
    else if(rTmp.equalsAscii(cUserDefined))
        rTmp += C2U(cUserSuffix);
}
//-----------------------------------------------------------------------------
void lcl_ConvertTOUNameToUserName(OUString& rTmp)
{
    ShellResource* pShellRes = ViewShell::GetShellRes();
    if(rTmp.equalsAscii(cUserDefined))
    {
        rTmp = pShellRes->aTOXUserName;
    }
    else if(!pShellRes->aTOXUserName.EqualsAscii(cUserDefined) &&
        USER_AND_SUFFIXLEN == rTmp.getLength())
    {
        String sToChange(rTmp);
        //make sure that in non-English versions the " (user)" suffix is removed
        if(!sToChange.SearchAscii(cUserDefined) && USER_LEN == sToChange.SearchAscii(cUserSuffix))
            rTmp = C2U(cUserDefined);
    }
}

/******************************************************************
 * SwXDocumentIndex
 ******************************************************************/

class SwDocIdxProperties_Impl
{
    SwTOXBase*      pTOXBase;
    OUString        sUserTOXTypeName;
public:
    SwTOXBase&      GetTOXBase() {return *pTOXBase;}
    SwDocIdxProperties_Impl(const SwTOXType* pType);
    ~SwDocIdxProperties_Impl(){delete pTOXBase;}

    const OUString& GetTypeName()const {return sUserTOXTypeName;}
    void            SetTypeName(const OUString& rSet) {sUserTOXTypeName = rSet;}
};

SwDocIdxProperties_Impl::SwDocIdxProperties_Impl(const SwTOXType* pType)
{
    SwForm aForm(pType->GetType());
    pTOXBase = new SwTOXBase(pType, aForm,
                                TOX_MARK, pType->GetTypeName());
    if(pType->GetType() == TOX_CONTENT || pType->GetType() == TOX_USER)
        pTOXBase->SetLevel(MAXLEVEL);
    sUserTOXTypeName = pType->GetTypeName();
}

const uno::Sequence< sal_Int8 > & SwXDocumentIndex::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::binfilter::CreateUnoTunnelId();
    return aSeq;
}

sal_Int64 SAL_CALL SwXDocumentIndex::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
            return (sal_Int64)this;
    }
    return 0;
}

OUString SwXDocumentIndex::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXDocumentIndex");
}

BOOL SwXDocumentIndex::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.BaseIndex") == rServiceName ||
                ( TOX_INDEX == eTOXType && C2U("com.sun.star.text.DocumentIndex") == rServiceName) ||
                ( TOX_CONTENT == eTOXType && C2U("com.sun.star.text.ContentIndex") == rServiceName) ||
                ( TOX_USER == eTOXType && C2U("com.sun.star.text.UserDefinedIndex") == rServiceName) ||
                ( TOX_ILLUSTRATIONS == eTOXType && C2U("com.sun.star.text.IllustrationIndex") == rServiceName) ||
                ( TOX_TABLES == eTOXType && C2U("com.sun.star.text.TableIndex") == rServiceName) ||
                ( TOX_OBJECTS == eTOXType && C2U("com.sun.star.text.ObjectIndex") == rServiceName) ||
                ( TOX_AUTHORITIES == eTOXType && C2U("com.sun.star.text.Bibliography") == rServiceName);
}

Sequence< OUString > SwXDocumentIndex::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.BaseIndex");
    switch( eTOXType )
    {
        case TOX_INDEX:         pArray[1] = C2U("com.sun.star.text.DocumentIndex");break;
        case TOX_CONTENT:       pArray[1] = C2U("com.sun.star.text.ContentIndex");break;
        case TOX_TABLES:        pArray[1] = C2U("com.sun.star.text.TableIndex");break;
        case TOX_ILLUSTRATIONS: pArray[1] = C2U("com.sun.star.text.IllustrationIndex");break;
        case TOX_OBJECTS:       pArray[1] = C2U("com.sun.star.text.ObjectIndex");break;
        case TOX_AUTHORITIES :  pArray[1] = C2U("com.sun.star.text.Bibliography");break;
        //case TOX_USER:
        default:
            pArray[1] = C2U("com.sun.star.text.UserDefinedIndex");
    }
    return aRet;
}

TYPEINIT1(SwXDocumentIndex, SwClient)
SwXDocumentIndex::SwXDocumentIndex(const SwTOXBaseSection* pB, SwDoc* pDc) :
    aLstnrCntnr( (text::XTextContent*)this),
    _pMap(0),
    m_pDoc(pDc),
    pBase(pB),
    eTOXType(TOX_USER),
    bIsDescriptor(sal_False),
    pProps(0),
    pStyleAccess(0),
    pTokenAccess(0)
{
    if(pBase && m_pDoc)
    {
        pBase->GetFmt()->Add(this);
        sal_uInt16 PropertyId;
        eTOXType = pBase->SwTOXBase::GetType();
        switch( eTOXType )
        {
            case TOX_INDEX:     PropertyId = PROPERTY_MAP_INDEX_IDX; break;
            case TOX_CONTENT:   PropertyId = PROPERTY_MAP_INDEX_CNTNT; break;
            case TOX_TABLES:        PropertyId = PROPERTY_MAP_INDEX_TABLES; break;
            case TOX_ILLUSTRATIONS: PropertyId = PROPERTY_MAP_INDEX_ILLUSTRATIONS; break;
            case TOX_OBJECTS:       PropertyId = PROPERTY_MAP_INDEX_OBJECTS; break;
            case TOX_AUTHORITIES :  PropertyId = PROPERTY_MAP_BIBLIOGRAPHY; break;
            //case TOX_USER:
            default:
                PropertyId = PROPERTY_MAP_INDEX_USER;
        }
        _pMap = aSwMapProvider.GetPropertyMap(PropertyId);
    }
}

SwXDocumentIndex::SwXDocumentIndex(TOXTypes eType, SwDoc& rDoc) :
    aLstnrCntnr( (text::XTextContent*)this),
    m_pDoc(0),
    pBase(0),
    eTOXType(eType),
    bIsDescriptor(sal_True),
    pProps(new SwDocIdxProperties_Impl(rDoc.GetTOXType(eType, 0))),
    pStyleAccess(0),
    pTokenAccess(0)
{
    sal_uInt16 PropertyId;
    switch(eType)
    {
        case TOX_INDEX:     PropertyId = PROPERTY_MAP_INDEX_IDX; break;
        case TOX_CONTENT:   PropertyId = PROPERTY_MAP_INDEX_CNTNT; break;
        case TOX_TABLES:        PropertyId = PROPERTY_MAP_INDEX_TABLES; break;
        case TOX_ILLUSTRATIONS: PropertyId = PROPERTY_MAP_INDEX_ILLUSTRATIONS; break;
        case TOX_OBJECTS:       PropertyId = PROPERTY_MAP_INDEX_OBJECTS; break;
        case TOX_AUTHORITIES :  PropertyId = PROPERTY_MAP_BIBLIOGRAPHY; break;
        //case TOX_USER:
        default:
            PropertyId = PROPERTY_MAP_INDEX_USER;
    }
    _pMap = aSwMapProvider.GetPropertyMap(PropertyId);
}


SwXDocumentIndex::~SwXDocumentIndex()
{
    delete pProps;
}

OUString SwXDocumentIndex::getServiceName(void) throw( RuntimeException )
{
    USHORT nObjectType = SW_SERVICE_TYPE_INDEX;
    switch(eTOXType)
    {
        case TOX_USER:              nObjectType = SW_SERVICE_USER_INDEX;break;
        case TOX_CONTENT:           nObjectType = SW_SERVICE_CONTENT_INDEX;break;
        case TOX_ILLUSTRATIONS:     nObjectType = SW_SERVICE_INDEX_ILLUSTRATIONS;break;
        case TOX_OBJECTS:           nObjectType = SW_SERVICE_INDEX_OBJECTS;break;
        case TOX_TABLES:            nObjectType = SW_SERVICE_INDEX_TABLES;break;
        case TOX_AUTHORITIES:       nObjectType = SW_SERVICE_INDEX_BIBLIOGRAPHY;break;
        default: break;
    }
    return SwXServiceProvider::GetProviderName(nObjectType);
}

void SwXDocumentIndex::update(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    SwTOXBase* pTOXBase = (SwTOXBaseSection*)GetFmt()->GetSection();
    if(!pTOXBase)
        throw RuntimeException();
    // Seitennummern eintragen
    ((SwTOXBaseSection*)pTOXBase)->UpdatePageNum();
}

Reference< XPropertySetInfo >  SwXDocumentIndex::getPropertySetInfo(void) throw( RuntimeException )
{
    Reference< XPropertySetInfo >  aRef = new SfxItemPropertySetInfo( _pMap );
    return aRef;
}

void SwXDocumentIndex::setPropertyValue(const OUString& rPropertyName,
                                        const uno::Any& aValue)
        throw( UnknownPropertyException, PropertyVetoException,
                 IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                                        _pMap, rPropertyName);
    if (!pMap)
        throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    if ( pMap->nFlags & PropertyAttribute::READONLY)
        throw PropertyVetoException ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

    SwTOXBase* pTOXBase(0);
    if(GetFmt())
        pTOXBase = (SwTOXBaseSection*)GetFmt()->GetSection();
    else if(bIsDescriptor)
        pTOXBase = &pProps->GetTOXBase();
    if(pTOXBase)
    {
        sal_uInt16 nCreate = pTOXBase->GetCreateType();
        sal_uInt16 nTOIOptions = 0;
        sal_uInt16 nOLEOptions = pTOXBase->GetOLEOptions();
        TOXTypes eTxBaseType = pTOXBase->GetTOXType()->GetType();
        if( eTxBaseType == TOX_INDEX )
            nTOIOptions = pTOXBase->GetOptions();
        SwForm  aForm(pTOXBase->GetTOXForm());
        sal_Bool bForm = sal_False;
        SfxItemSet* pAttrSet = 0;
        switch(pMap->nWID)
        {
            case WID_IDX_TITLE  :
            {
                OUString sNewName;
                aValue >>= sNewName;
                pTOXBase->SetTitle(sNewName);
            }
            break;
            case WID_IDX_NAME:
            {
                OUString sNewName;
                aValue >>= sNewName;
                pTOXBase->SetTOXName(sNewName);
            }
            break;
            case WID_USER_IDX_NAME:
            {
                OUString sNewName;
                aValue >>= sNewName;
                lcl_ConvertTOUNameToUserName(sNewName);
                DBG_ASSERT(TOX_USER == eTxBaseType, "tox type name can only be changed for user indexes");
                if(GetFmt())
                {
                    OUString sTmp = pTOXBase->GetTOXType()->GetTypeName();
                    if(sTmp != sNewName)
                    {
                        lcl_ReAssignTOXType(GetFmt()->GetDoc(), *pTOXBase, sNewName);
                    }
                }
                else
                    pProps->SetTypeName(sNewName);
            }
            break;
            case WID_IDX_LOCALE:
            {
                lang::Locale aLocale;
                if(aValue>>= aLocale)
                    pTOXBase->SetLanguage(SvxLocaleToLanguage(aLocale));
                else
                    throw IllegalArgumentException();
            }
            break;
            case WID_IDX_SORT_ALGORITHM:
            {
                OUString sTmp;
                if(aValue >>= sTmp)
                    pTOXBase->SetSortAlgorithm(sTmp);
                else
                    throw IllegalArgumentException();
            }
            break;
            case WID_LEVEL      :
                pTOXBase->SetLevel(lcl_AnyToInt16(aValue));
            break;
            case WID_CREATE_FROM_MARKS                 :
                nCreate = lcl_AnyToBool(aValue) ? nCreate | TOX_MARK: nCreate & ~TOX_MARK;
            break;
            case WID_CREATE_FROM_OUTLINE               :
                nCreate = lcl_AnyToBool(aValue) ? nCreate | TOX_OUTLINELEVEL: nCreate & ~TOX_OUTLINELEVEL;
            break;
//          case WID_PARAGRAPH_STYLE_NAMES             :OSL_FAIL("not implemented")
//          break;
            case WID_CREATE_FROM_CHAPTER               :
                pTOXBase->SetFromChapter(lcl_AnyToBool(aValue));
            break;
            case WID_CREATE_FROM_LABELS                :
                pTOXBase->SetFromObjectNames(! lcl_AnyToBool(aValue));
            break;
            case WID_PROTECTED                         :
            {
                sal_Bool bSet = lcl_AnyToBool(aValue);
                pTOXBase->SetProtected(bSet);
                if(GetFmt())
                    ((SwTOXBaseSection*)pTOXBase)->SetProtect(bSet);
            }
            break;
            case WID_USE_ALPHABETICAL_SEPARATORS:
                nTOIOptions = lcl_AnyToBool(aValue) ?
                    nTOIOptions | TOI_ALPHA_DELIMITTER : nTOIOptions & ~TOI_ALPHA_DELIMITTER;
            break;
            case WID_USE_KEY_AS_ENTRY                  :
                nTOIOptions = lcl_AnyToBool(aValue) ?
                    nTOIOptions | TOI_KEY_AS_ENTRY : nTOIOptions & ~TOI_KEY_AS_ENTRY;
            break;
            case WID_USE_COMBINED_ENTRIES              :
                nTOIOptions = lcl_AnyToBool(aValue) ?
                    nTOIOptions | TOI_SAME_ENTRY : nTOIOptions & ~TOI_SAME_ENTRY;
            break;
            case WID_IS_CASE_SENSITIVE                 :
                nTOIOptions = lcl_AnyToBool(aValue) ?
                    nTOIOptions | TOI_CASE_SENSITIVE : nTOIOptions & ~TOI_CASE_SENSITIVE;
            break;
            case WID_USE_P_P                           :
                nTOIOptions = lcl_AnyToBool(aValue) ?
                    nTOIOptions | TOI_FF : nTOIOptions & ~TOI_FF;
            break;
            case WID_USE_DASH                          :
                nTOIOptions = lcl_AnyToBool(aValue) ?
                    nTOIOptions | TOI_DASH : nTOIOptions & ~TOI_DASH;
            break;
            case WID_USE_UPPER_CASE                    :
                nTOIOptions = lcl_AnyToBool(aValue) ?
                    nTOIOptions | TOI_INITIAL_CAPS : nTOIOptions & ~TOI_INITIAL_CAPS;
            break;
            case WID_IS_COMMA_SEPARATED :
                bForm = sal_True;
                aForm.SetCommaSeparated(lcl_AnyToBool(aValue));
            break;
            case WID_LABEL_CATEGORY                    :
            {
                // convert file-format/API/external programmatic english name
                // to internal UI name before usage
                String aName( SwStyleNameMapper::GetSpecialExtraUIName(
                                    lcl_AnyToString(aValue) ) );
                pTOXBase->SetSequenceName( aName );
            }
            break;
            case WID_LABEL_DISPLAY_TYPE                :
            {
                sal_Int16 nVal = lcl_AnyToInt16(aValue);
                    sal_uInt16 nSet = CAPTION_COMPLETE;
                    switch (nVal)
                    {
                        case text::ReferenceFieldPart::TEXT: nSet = CAPTION_COMPLETE;
                        break;
                        case text::ReferenceFieldPart::CATEGORY_AND_NUMBER  : nSet = CAPTION_NUMBER;
                        break;
                        case text::ReferenceFieldPart::ONLY_CAPTION : nSet = CAPTION_TEXT;
                        break;
                        default:
                            throw IllegalArgumentException();
                    }
                    pTOXBase->SetCaptionDisplay((SwCaptionDisplay)nSet);
            }
            break;
            case WID_USE_LEVEL_FROM_SOURCE             :
                pTOXBase->SetLevelFromChapter(lcl_AnyToBool(aValue));
            break;
//          case WID_RECALC_TAB_STOPS                  :OSL_FAIL("not implemented")
//              lcl_AnyToBool(aValue) ?
//          break;
            break;
            case WID_MAIN_ENTRY_CHARACTER_STYLE_NAME   :
            {
                String aString;
                SwStyleNameMapper::FillUIName(lcl_AnyToString(aValue), aString, GET_POOLID_CHRFMT, sal_True);
                pTOXBase->SetMainEntryCharStyle( aString );
            }
            break;
            case WID_CREATE_FROM_TABLES                :
                nCreate = lcl_AnyToBool(aValue) ? nCreate | TOX_TABLE : nCreate & ~TOX_TABLE;
            break;
            case WID_CREATE_FROM_TEXT_FRAMES           :
                nCreate = lcl_AnyToBool(aValue) ? nCreate | TOX_FRAME : nCreate & ~TOX_FRAME;
            break;
            case WID_CREATE_FROM_GRAPHIC_OBJECTS       :
                nCreate = lcl_AnyToBool(aValue) ? nCreate | TOX_GRAPHIC : nCreate & ~TOX_GRAPHIC;
            break;
            case WID_CREATE_FROM_EMBEDDED_OBJECTS      :
                if(lcl_AnyToBool(aValue))
                    nCreate |= TOX_OLE;
                else
                    nCreate &= ~TOX_OLE;
            break;
            case WID_CREATE_FROM_STAR_MATH:
                nOLEOptions = lcl_AnyToBool(aValue) ? nOLEOptions | TOO_MATH : nOLEOptions & ~TOO_MATH;
            break;
            case WID_CREATE_FROM_STAR_CHART            :
                nOLEOptions = lcl_AnyToBool(aValue) ? nOLEOptions | TOO_CHART : nOLEOptions & ~TOO_CHART;
            break;
            case WID_CREATE_FROM_STAR_CALC             :
                nOLEOptions = lcl_AnyToBool(aValue) ? nOLEOptions | TOO_CALC : nOLEOptions & ~TOO_CALC;
            break;
            case WID_CREATE_FROM_STAR_DRAW             :
                nOLEOptions = lcl_AnyToBool(aValue) ? nOLEOptions | TOO_DRAW_IMPRESS : nOLEOptions & ~TOO_DRAW_IMPRESS;
            break;
            case WID_CREATE_FROM_OTHER_EMBEDDED_OBJECTS:
                nOLEOptions = lcl_AnyToBool(aValue) ? nOLEOptions | TOO_OTHER : nOLEOptions & ~TOO_OTHER;
            break;
            case WID_PARA_HEAD             :
            {
                String aString;
                SwStyleNameMapper::FillUIName( lcl_AnyToString(aValue), aString, GET_POOLID_TXTCOLL, sal_True);
                bForm = sal_True;
                //Header steht an Pos 0
                aForm.SetTemplate( 0, aString );
            }
            break;
            case WID_IS_RELATIVE_TABSTOPS:
                bForm = sal_True;
                aForm.SetRelTabPos(lcl_AnyToBool(aValue));
            break;
            case WID_PARA_SEP              :
            {
                String aString;
                bForm = sal_True;
                SwStyleNameMapper::FillUIName( lcl_AnyToString(aValue), aString, GET_POOLID_TXTCOLL, sal_True);
                aForm.SetTemplate( 1, aString );
            }
            break;
            case WID_CREATE_FROM_PARAGRAPH_STYLES:
                nCreate = lcl_AnyToBool(aValue) ?
                    (nCreate | TOX_TEMPLATE) : (nCreate & ~TOX_TEMPLATE);
            break;

            case WID_PARA_LEV1             :
            case WID_PARA_LEV2             :
            case WID_PARA_LEV3             :
            case WID_PARA_LEV4             :
            case WID_PARA_LEV5             :
            case WID_PARA_LEV6             :
            case WID_PARA_LEV7             :
            case WID_PARA_LEV8             :
            case WID_PARA_LEV9             :
            case WID_PARA_LEV10            :
            {
                bForm = sal_True;
                // im sdbcx::Index beginnt Lebel 1 bei Pos 2 sonst bei Pos 1
                sal_uInt16 nLPos = pTOXBase->GetType() == TOX_INDEX ? 2 : 1;
                String aString;
                SwStyleNameMapper::FillUIName( lcl_AnyToString(aValue), aString, GET_POOLID_TXTCOLL, sal_True);
                aForm.SetTemplate(nLPos + pMap->nWID - WID_PARA_LEV1, aString );
            }
            break;
            default:
                //this is for items only
                if(WID_PRIMARY_KEY > pMap->nWID)
                {
                    SfxItemPropertySet aPropSet(_pMap);
                    const SwAttrSet& rSet = m_pDoc->GetTOXBaseAttrSet(*pTOXBase);
                    pAttrSet = new SfxItemSet(rSet);
                    aPropSet.setPropertyValue(*pMap, aValue, *pAttrSet);

                    const SwSectionFmts& rSects = m_pDoc->GetSections();
                    const SwSectionFmt* pOwnFmt = GetFmt();
                    for(sal_uInt16 i = 0; i < rSects.Count(); i++)
                    {
                        const SwSectionFmt* pTmpFmt = rSects[ i ];
                        if(pTmpFmt == pOwnFmt)
                        {
                            m_pDoc->ChgSection( i, *(SwTOXBaseSection*)pTOXBase, pAttrSet );
                            break;
                        }
                    }
                }
        }
        pTOXBase->SetCreate(nCreate);
        pTOXBase->SetOLEOptions(nOLEOptions);
        if(pTOXBase->GetTOXType()->GetType() == TOX_INDEX)
            pTOXBase->SetOptions(nTOIOptions);
        if(bForm)
            pTOXBase->SetTOXForm(aForm);
        delete pAttrSet;
    }
    else
        throw RuntimeException();

}

uno::Any SwXDocumentIndex::getPropertyValue(const OUString& rPropertyName)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                                    _pMap, rPropertyName);
    if (!pMap)
        throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    SwTOXBase* pTOXBase(0);
    if(GetFmt())
        pTOXBase = (SwTOXBaseSection*)GetFmt()->GetSection();
    else if(bIsDescriptor)
        pTOXBase = &pProps->GetTOXBase();
    if(pTOXBase)
    {
        sal_uInt16 nCreate = pTOXBase->GetCreateType();
        sal_uInt16 nTOIOptions = 0;
        sal_uInt16 nOLEOptions = pTOXBase->GetOLEOptions();
        if(pTOXBase->GetTOXType()->GetType() == TOX_INDEX)
            nTOIOptions = pTOXBase->GetOptions();
        const SwForm& rForm = pTOXBase->GetTOXForm();
        sal_Bool bBOOL = sal_True;
        sal_Bool bRet = sal_False;
        switch(pMap->nWID)
        {
            case WID_IDX_CONTENT_SECTION:
            case WID_IDX_HEADER_SECTION :
                bBOOL = sal_False;
                if(WID_IDX_CONTENT_SECTION == pMap->nWID)
                {
                    Reference <XTextSection> xContentSect = SwXTextSections::GetObject( *GetFmt() );
                    aRet <<= xContentSect;
                }
                else
                {
                    SwSections aSectArr;
                    GetFmt()->GetChildSections( aSectArr, SORTSECT_NOT, FALSE);
                    for(USHORT i = 0; i < aSectArr.Count(); i++)
                    {
                        SwSection* pSect = aSectArr[i];
                        if(pSect->GetType() == TOX_HEADER_SECTION)
                        {
                            Reference <XTextSection> xHeaderSect = SwXTextSections::GetObject( *pSect->GetFmt() );
                            aRet <<= xHeaderSect;
                            break;
                        }
                    }
                }
            break;
            case WID_IDX_TITLE  :
            {
                bBOOL = sal_False;
                OUString uRet(pTOXBase->GetTitle());
                aRet <<= uRet;
                break;
            }
            case WID_IDX_NAME:
                bBOOL = sal_False;
                aRet <<= OUString(pTOXBase->GetTOXName());
            break;
            case WID_USER_IDX_NAME:
            {
                bBOOL = sal_False;
                OUString sTmp;
                if(!IsDescriptor())
                    sTmp = pTOXBase->GetTOXType()->GetTypeName();
                else
                    sTmp = pProps->GetTypeName();
                //I18N
                lcl_ConvertTOUNameToProgrammaticName(sTmp);
                aRet <<= sTmp;
            }
            break;
            case WID_IDX_LOCALE:
                bBOOL = sal_False;
                aRet <<= SvxCreateLocale(pTOXBase->GetLanguage());
            break;
            case WID_IDX_SORT_ALGORITHM:
                bBOOL = sal_False;
                aRet <<= OUString(pTOXBase->GetSortAlgorithm());
            break;
            case WID_LEVEL      :
                bBOOL = sal_False;
                aRet <<= (sal_Int16)pTOXBase->GetLevel();
            break;
            case WID_CREATE_FROM_MARKS                 :
                bRet = 0 != (nCreate & TOX_MARK);
            break;
            case WID_CREATE_FROM_OUTLINE               :
                bRet = 0 != (nCreate & TOX_OUTLINELEVEL);
            break;
            case WID_CREATE_FROM_CHAPTER               :
                bRet = pTOXBase->IsFromChapter();
            break;
            case WID_CREATE_FROM_LABELS                :
                bRet = ! pTOXBase->IsFromObjectNames();
            break;
            case WID_PROTECTED                         :
                bRet = pTOXBase->IsProtected();
            break;
            case WID_USE_ALPHABETICAL_SEPARATORS:
                bRet = 0 != (nTOIOptions & TOI_ALPHA_DELIMITTER);
            break;
            case WID_USE_KEY_AS_ENTRY                  :
                bRet = 0 != (nTOIOptions & TOI_KEY_AS_ENTRY);
            break;
            case WID_USE_COMBINED_ENTRIES              :
                bRet = 0 != (nTOIOptions & TOI_SAME_ENTRY);
            break;
            case WID_IS_CASE_SENSITIVE                 :
                bRet = 0 != (nTOIOptions & TOI_CASE_SENSITIVE);
            break;
            case WID_USE_P_P:
                bRet = 0 != (nTOIOptions & TOI_FF);
            break;
            case WID_USE_DASH                          :
                bRet = 0 != (nTOIOptions & TOI_DASH);
            break;
            case WID_USE_UPPER_CASE                    :
                bRet = 0 != (nTOIOptions & TOI_INITIAL_CAPS);
            break;
            case WID_IS_COMMA_SEPARATED :
                bRet = rForm.IsCommaSeparated();
            break;
            case WID_LABEL_CATEGORY                    :
            {
                // convert internal UI name to
                // file-format/API/external programmatic english name
                // before usage
                String aName( SwStyleNameMapper::GetSpecialExtraProgName(
                                    pTOXBase->GetSequenceName() ) );
                aRet <<= OUString( aName );
                bBOOL = sal_False;
            }
            break;
            case WID_LABEL_DISPLAY_TYPE                :
            {
                bBOOL = sal_False;
                sal_Int16 nSet = text::ReferenceFieldPart::TEXT;
                switch (pTOXBase->GetCaptionDisplay())
                {
                    case CAPTION_COMPLETE:  nSet = text::ReferenceFieldPart::TEXT;break;
                    case CAPTION_NUMBER  :  nSet = text::ReferenceFieldPart::CATEGORY_AND_NUMBER;   break;
                    case CAPTION_TEXT    :  nSet = text::ReferenceFieldPart::ONLY_CAPTION;      break;
                }
                aRet <<= nSet;
            }
            break;
            case WID_USE_LEVEL_FROM_SOURCE             :
                bRet = pTOXBase->IsLevelFromChapter();
            break;
            case WID_LEVEL_FORMAT                      :
            {
                Reference< container::XIndexReplace >  xTokenAcc =
                                    ((SwXDocumentIndex*)this)->GetTokenAccess();
                if(!xTokenAcc.is())
                    xTokenAcc = new SwXIndexTokenAccess_Impl(*
                                                (SwXDocumentIndex*)this);
                aRet.setValue(&xTokenAcc, ::getCppuType((const Reference<container::XIndexReplace>*)0));
                bBOOL = sal_False;
            }
            break;
            case WID_LEVEL_PARAGRAPH_STYLES            :
            {
                Reference< container::XIndexReplace >  xStyleAcc =
                                    ((SwXDocumentIndex*)this)->GetStyleAccess();
                if(!xStyleAcc.is())
                    xStyleAcc = new SwXIndexStyleAccess_Impl(*
                                                (SwXDocumentIndex*)this);
                aRet.setValue(&xStyleAcc, ::getCppuType((const Reference<container::XIndexReplace>*)0));
                bBOOL = sal_False;
            }
            break;
//          case WID_RECALC_TAB_STOPS                  :
//              tab stops are alway recalculated
//          break;
            //case WID_???                             :
            break;
            case WID_MAIN_ENTRY_CHARACTER_STYLE_NAME   :
            {
                bBOOL = sal_False;
                String aString;
                SwStyleNameMapper::FillProgName(
                        pTOXBase->GetMainEntryCharStyle(),
                        aString,
                        GET_POOLID_CHRFMT,
                        sal_True);
                aRet <<= OUString( aString );
            }
            break;
            case WID_CREATE_FROM_TABLES                :
                bRet = 0 != (nCreate & TOX_TABLE);
            break;
            case WID_CREATE_FROM_TEXT_FRAMES           :
                bRet = 0 != (nCreate & TOX_FRAME);
            break;
            case WID_CREATE_FROM_GRAPHIC_OBJECTS       :
                bRet = 0 != (nCreate & TOX_GRAPHIC);
            break;
            case WID_CREATE_FROM_EMBEDDED_OBJECTS      :
                bRet = 0 != (nCreate & TOX_OLE);
            break;
            case WID_CREATE_FROM_STAR_MATH:
                bRet = 0 != (nOLEOptions & TOO_MATH);
            break;
            case WID_CREATE_FROM_STAR_CHART            :
                bRet = 0 != (nOLEOptions & TOO_CHART);
            break;
            case WID_CREATE_FROM_STAR_CALC             :
                bRet = 0 != (nOLEOptions & TOO_CALC);
            break;
            case WID_CREATE_FROM_STAR_DRAW             :
                bRet = 0 != (nOLEOptions & TOO_DRAW_IMPRESS);
            break;
            case WID_CREATE_FROM_OTHER_EMBEDDED_OBJECTS:
                bRet = 0 != (nOLEOptions & TOO_OTHER);
            break;
            case WID_CREATE_FROM_PARAGRAPH_STYLES:
                bRet = 0 != (nCreate & TOX_TEMPLATE);
            break;
            case WID_PARA_HEAD             :
            {
                //Header steht an Pos 0
                String aString;
                SwStyleNameMapper::FillProgName(rForm.GetTemplate( 0 ), aString,
                        GET_POOLID_TXTCOLL, sal_True );
                aRet <<= OUString( aString );
                bBOOL = sal_False;
            }
            break;
            case WID_PARA_SEP              :
            {
                String aString;
                SwStyleNameMapper::FillProgName(
                        rForm.GetTemplate( 1 ),
                        aString,
                        GET_POOLID_TXTCOLL,
                        sal_True);
                aRet <<= OUString( aString );
                bBOOL = sal_False;
            }
            break;
            case WID_PARA_LEV1             :
            case WID_PARA_LEV2             :
            case WID_PARA_LEV3             :
            case WID_PARA_LEV4             :
            case WID_PARA_LEV5             :
            case WID_PARA_LEV6             :
            case WID_PARA_LEV7             :
            case WID_PARA_LEV8             :
            case WID_PARA_LEV9             :
            case WID_PARA_LEV10            :
            {
                // im sdbcx::Index beginnt Lebel 1 bei Pos 2 sonst bei Pos 1
                sal_uInt16 nLPos = pTOXBase->GetType() == TOX_INDEX ? 2 : 1;
                String aString;
                SwStyleNameMapper::FillProgName(
                        rForm.GetTemplate(nLPos + pMap->nWID - WID_PARA_LEV1),
                        aString,
                        GET_POOLID_TXTCOLL,
                        sal_True);
                aRet <<= OUString( aString );
                bBOOL = sal_False;
            }
            break;
            case WID_IS_RELATIVE_TABSTOPS:
                bRet = rForm.IsRelTabPos();
            break;
            case WID_INDEX_MARKS:
            {
                SwTOXMarks aMarks;
                const SwTOXType* pType = pTOXBase->GetTOXType();
                SwClientIter aIter(*(SwTOXType*)pType);
                SwTOXMark* pMark = (SwTOXMark*)aIter.First(TYPE(SwTOXMark));
                while( pMark )
                {
                    if(pMark->GetTxtTOXMark())
                        aMarks.C40_INSERT(SwTOXMark, pMark, aMarks.Count());
                    pMark = (SwTOXMark*)aIter.Next();
                }
                Sequence< Reference < XDocumentIndexMark > > aXMarks(aMarks.Count());
                Reference<XDocumentIndexMark>* pxMarks = aXMarks.getArray();
                for(USHORT i = 0; i < aMarks.Count(); i++)
                {
                    SwTOXMark* pLclMark = aMarks.GetObject(i);
                    pxMarks[i] = SwXDocumentIndexMark::GetObject((SwTOXType*)pType, pLclMark, m_pDoc);
                }
                aRet.setValue(&aXMarks, ::getCppuType((Sequence< Reference< XDocumentIndexMark > >*)0));
                bBOOL = sal_False;
            }
            break;
            default:
                //this is for items only
                bBOOL = sal_False;
                if(WID_PRIMARY_KEY > pMap->nWID)
                {
                    SfxItemPropertySet aPropSet(_pMap);
                    const SwAttrSet& rSet = m_pDoc->GetTOXBaseAttrSet(*pTOXBase);
                    aRet = aPropSet.getPropertyValue(*pMap, rSet);
                }
        }
        if(bBOOL)
            aRet.setValue(&bRet, ::getCppuBooleanType());
    }
    return aRet;
}

void SwXDocumentIndex::addPropertyChangeListener(
        const OUString& /*PropertyName*/, const Reference< XPropertyChangeListener > & /*aListener*/)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SwXDocumentIndex::removePropertyChangeListener(
        const OUString& /*PropertyName*/, const Reference< XPropertyChangeListener > & /*aListener*/)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SwXDocumentIndex::addVetoableChangeListener(
        const OUString& /*PropertyName*/, const Reference< XVetoableChangeListener > & /*aListener*/)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SwXDocumentIndex::removeVetoableChangeListener(
        const OUString& /*PropertyName*/, const Reference< XVetoableChangeListener > & /*aListener*/)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SwXDocumentIndex::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    BOOL bRemove = FALSE;
    switch( pOld ? pOld->Which() : 0 )
    {
    case RES_REMOVE_UNO_OBJECT:
    case RES_OBJECTDYING:
        bRemove = (void*)GetRegisteredIn() == ((SwPtrMsgPoolItem *)pOld)->pObject;
        break;
    case RES_FMT_CHG:
        // wurden wir an das neue umgehaengt und wird das alte geloscht?
        bRemove = ((SwFmtChg*)pNew)->pChangedFmt == GetRegisteredIn() &&
                  ((SwFmtChg*)pOld)->pChangedFmt->IsFmtInDTOR();
        break;
    }
    if( bRemove )
    {
        ((SwModify*)GetRegisteredIn())->Remove( this );
        aLstnrCntnr.Disposing();
    }
}

void SwXDocumentIndex::attachToRange(const Reference< text::XTextRange > & xTextRange)
    throw( IllegalArgumentException, RuntimeException )
{
    if(!bIsDescriptor)
        throw RuntimeException();
    Reference<XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
    SwXTextRange* pRange = 0;
    OTextCursorHelper* pCursor = 0;
    if(xRangeTunnel.is())
    {
        pRange = (SwXTextRange*)xRangeTunnel->getSomething(
                                SwXTextRange::getUnoTunnelId());
        pCursor = (OTextCursorHelper*)xRangeTunnel->getSomething(
                                OTextCursorHelper::getUnoTunnelId());
    }

    SwDoc* pDoc = pRange ? (SwDoc*)pRange->GetDoc() : pCursor ? (SwDoc*)pCursor->GetDoc() : 0;
    if(pDoc )
    {
        SwUnoInternalPaM aPam(*pDoc);
        //das muss jetzt sal_True liefern
        SwXTextRange::XTextRangeToSwPaM(aPam, xTextRange);

            const SwTOXBase* pOld = pDoc->GetCurTOX( *aPam.Start() );
            if(!pOld)
            {
                UnoActionContext aAction(pDoc);
                if(aPam.HasMark())
                    pDoc->DeleteAndJoin(aPam);

                SwTOXBase& rTOXBase = pProps->GetTOXBase();
                const SwTOXType* pTOXType = rTOXBase.GetTOXType();
                if(TOX_USER == pTOXType->GetType() && !pProps->GetTypeName().equals(pTOXType->GetTypeName()))
                {
                    lcl_ReAssignTOXType(pDoc, rTOXBase, pProps->GetTypeName());
                }
                //TODO: apply Section attributes (columns and background)
                const SwTOXBaseSection* pTOX = pDoc->InsertTableOf(
                                    *aPam.GetPoint(), rTOXBase, 0, sal_False );

                pDoc->SetTOXBaseName( *pTOX, pProps->GetTOXBase().GetTOXName() );

                // Seitennummern eintragen
                pBase = (const SwTOXBaseSection*)pTOX;
                pBase->GetFmt()->Add(this);
                ((SwTOXBaseSection*)pTOX)->UpdatePageNum();
            }
            else
                throw IllegalArgumentException();

        DELETEZ(pProps);
        m_pDoc = pDoc;
        bIsDescriptor = sal_False;
    }
}

void SwXDocumentIndex::attach(const Reference< text::XTextRange > & xTextRange)
    throw( IllegalArgumentException, RuntimeException )
{
    SolarMutexGuard aGuard;
    attachToRange( xTextRange );
}

Reference< text::XTextRange >  SwXDocumentIndex::getAnchor(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    Reference< text::XTextRange >   xRet;
    if(GetRegisteredIn())
    {
        SwSectionFmt*  pSectFmt = GetFmt();
        const SwNodeIndex* pIdx;
        if( 0 != ( pIdx = pSectFmt->GetCntnt().GetCntntIdx() ) &&
            pIdx->GetNode().GetNodes().IsDocNodes() )
        {
            SwPaM aPaM(*pIdx);
            aPaM.Move( fnMoveForward, fnGoCntnt );
            aPaM.SetMark();
            aPaM.GetPoint()->nNode = *pIdx->GetNode().EndOfSectionNode();
            aPaM.Move( fnMoveBackward, fnGoCntnt );
            xRet = SwXTextRange::CreateTextRangeFromPosition(pSectFmt->GetDoc(),
                *aPaM.GetMark(), aPaM.GetPoint());
        }
    }
    else
        throw RuntimeException();
    return xRet;
}

void lcl_RemoveChildSections(SwSectionFmt& rParentFmt)
{
    SwSections aTmpArr;
    SwDoc* pDoc = rParentFmt.GetDoc();
    sal_uInt16 nCnt = rParentFmt.GetChildSections(aTmpArr,SORTSECT_POS);
    if( nCnt )
    {
        for( sal_uInt16 n = 0; n < nCnt; ++n )
            if( aTmpArr[n]->GetFmt()->IsInNodesArr() )
            {
                SwSectionFmt* pFmt = aTmpArr[n]->GetFmt();
                lcl_RemoveChildSections(*pFmt);
                pDoc->DelSectionFmt( pFmt );
            }
    }
}
void SwXDocumentIndex::dispose(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if( !GetRegisteredIn() )
        throw RuntimeException();
}

void SwXDocumentIndex::addEventListener(const Reference< XEventListener > & aListener) throw( RuntimeException )
{
    if(!GetRegisteredIn())
        throw RuntimeException();
    aLstnrCntnr.AddListener(aListener);
}

void SwXDocumentIndex::removeEventListener(const Reference< XEventListener > & aListener) throw( RuntimeException )
{
    if(!GetRegisteredIn() || !aLstnrCntnr.RemoveListener(aListener))
        throw RuntimeException();
}

OUString SwXDocumentIndex::getName(void) throw( RuntimeException )
{
    SwSectionFmt* pSectionFmt = GetFmt();
    OUString uRet;
    if(bIsDescriptor)
    {
        uRet = OUString(pProps->GetTOXBase().GetTOXName());
    }
    else if(pSectionFmt)
    {
        uRet = OUString(pSectionFmt->GetSection()->GetName());
    }
    else
        throw RuntimeException();
    return uRet;
}

void SwXDocumentIndex::setName(const OUString& rName) throw( RuntimeException )
{
    SwSectionFmt* pSectionFmt = GetFmt();
    String sNewName(rName);
    sal_Bool bExcept = sal_False;
    if(!sNewName.Len())
        bExcept = sal_True;
    if(bIsDescriptor)
    {
        pProps->GetTOXBase().SetTOXName(sNewName);
    }
    else if(!pSectionFmt ||
        !pSectionFmt->GetDoc()->SetTOXBaseName(
                *(SwTOXBaseSection*)pSectionFmt->GetSection(), sNewName))
        bExcept = sal_True;

    if(bExcept)
        throw RuntimeException();
}

/******************************************************************
 * SwXDocumentIndexMark
 ******************************************************************/

const uno::Sequence< sal_Int8 > & SwXDocumentIndexMark::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::binfilter::CreateUnoTunnelId();
    return aSeq;
}

sal_Int64 SAL_CALL SwXDocumentIndexMark::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
            return (sal_Int64)this;
    }
    return 0;
}

TYPEINIT1(SwXDocumentIndexMark, SwClient)
const sal_Char cBaseMark[]      = "com.sun.star.text.BaseIndexMark";
const sal_Char cContentMark[]   = "com.sun.star.text.ContentIndexMark";
const sal_Char cIdxMark[]       = "com.sun.star.text.DocumentIndexMark";
const sal_Char cIdxMarkAsian[]  = "com.sun.star.text.DocumentIndexMarkAsian";
const sal_Char cUserMark[]      = "com.sun.star.text.UserIndexMark";
const sal_Char cTextContent[]   = "com.sun.star.text.TextContent";

OUString SwXDocumentIndexMark::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXDocumentIndexMark");
}

BOOL SwXDocumentIndexMark::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return !rServiceName.compareToAscii(cBaseMark)||
        !rServiceName.compareToAscii(cTextContent) ||
        (eType == TOX_USER && !rServiceName.compareToAscii(cUserMark)) ||
        (eType == TOX_CONTENT && !rServiceName.compareToAscii(cContentMark)) ||
        (eType == TOX_INDEX && !rServiceName.compareToAscii(cIdxMark)) ||
        (eType == TOX_INDEX && !rServiceName.compareToAscii(cIdxMarkAsian));
}

Sequence< OUString > SwXDocumentIndexMark::getSupportedServiceNames(void) throw( RuntimeException )
{
    INT32 nCnt = (eType == TOX_INDEX) ? 4 : 3;
    Sequence< OUString > aRet(nCnt);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U(cBaseMark);
    pArray[1] = C2U(cTextContent);
    switch(eType)
    {
        case TOX_USER:
            pArray[2] = C2U(cUserMark);
        break;
        case TOX_CONTENT:
            pArray[2] = C2U(cContentMark);
        break;
        case TOX_INDEX:
            pArray[2] = C2U(cIdxMark);
            pArray[3] = C2U(cIdxMarkAsian);
        break;
        default:
        break;
    }
    return aRet;
}

SwXDocumentIndexMark::SwXDocumentIndexMark(TOXTypes eToxType) :
    aLstnrCntnr( (text::XTextContent*)this),
    aTypeDepend(this, 0),
    m_pDoc(0),
    m_pTOXMark(0),
    bIsDescriptor(sal_True),
    bMainEntry(sal_False),
    eType(eToxType),
    nLevel(0)
{
    InitMap(eToxType);
}

SwXDocumentIndexMark::SwXDocumentIndexMark(const SwTOXType* pType,
                                    const SwTOXMark* pMark,
                                    SwDoc* pDc) :
    aLstnrCntnr( (text::XTextContent*)this),
    aTypeDepend(this, (SwTOXType*)pType),
    m_pDoc(pDc),
    m_pTOXMark(pMark),
    bIsDescriptor(sal_False),
    bMainEntry(sal_False),
    eType(pType->GetType()),
    nLevel(0)
{
    m_pDoc->GetUnoCallBack()->Add(this);
    InitMap(eType);
}

SwXDocumentIndexMark::~SwXDocumentIndexMark()
{

}

void SwXDocumentIndexMark::InitMap(TOXTypes eToxType)
{
    sal_uInt16 nMapId = PROPERTY_MAP_USER_MARK; //case TOX_USER:
    switch( eToxType )
    {
        case TOX_INDEX:
            nMapId = PROPERTY_MAP_INDEX_MARK ;
        break;
        case TOX_CONTENT:
            nMapId = PROPERTY_MAP_CNTIDX_MARK;
        break;
        default:
        break;
    }
    _pMap = aSwMapProvider.GetPropertyMap(nMapId);
}

OUString SwXDocumentIndexMark::getMarkEntry(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    SwTOXType* pType = ((SwXDocumentIndexMark*)this)->GetTOXType();
    OUString sRet;
    if(pType)
    {
        SwTOXMark* pCurMark = lcl_GetMark(pType, GetTOXMark());
        SwTOXMark aMark(*pCurMark);
        sRet = OUString(aMark.GetAlternativeText());
    }
    else if(bIsDescriptor)
         sRet = sAltText;
    else
        throw RuntimeException();
    return sRet;
}

void SwXDocumentIndexMark::setMarkEntry(const OUString& rIndexEntry) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    SwTOXType* pType = ((SwXDocumentIndexMark*)this)->GetTOXType();
    if(pType)
    {
        SwTOXMark* pCurMark = lcl_GetMark(pType, GetTOXMark());
        SwTOXMark aMark(*pCurMark);
        aMark.SetAlternativeText(rIndexEntry);
        SwTxtTOXMark* pTxtMark = pCurMark->GetTxtTOXMark();
        SwPaM aPam(pTxtMark->GetTxtNode(), *pTxtMark->GetStart());
        aPam.SetMark();
        if(pTxtMark->GetEnd())
        {
            aPam.GetPoint()->nContent = *pTxtMark->GetEnd();
        }
        else
            aPam.GetPoint()->nContent++;

        //die alte Marke loeschen
        m_pTOXMark = pCurMark = 0;

        SwTxtAttr* pTxtAttr = 0;
        sal_Bool bInsAtPos = aMark.IsAlternativeText();
        const SwPosition *pStt = aPam.Start(),
                            *pEnd = aPam.End();
        if( bInsAtPos )
        {
            SwPaM aTmp( *pStt );
            m_pDoc->Insert( aTmp, aMark, 0 );
            pTxtAttr = pStt->nNode.GetNode().GetTxtNode()->GetTxtAttr(
                        pStt->nContent.GetIndex()-1, RES_TXTATR_TOXMARK);
        }
        else if( *pEnd != *pStt )
        {
            m_pDoc->Insert( aPam, aMark, SETATTR_DONTEXPAND );
            pTxtAttr = pStt->nNode.GetNode().GetTxtNode()->GetTxtAttr(
                                pStt->nContent, RES_TXTATR_TOXMARK);
        }
        //und sonst - Marke geloescht?
        if(pTxtAttr)
            m_pTOXMark = &pTxtAttr->GetTOXMark();
    }
    else if(bIsDescriptor)
    {
        sAltText = rIndexEntry;
    }
    else
        throw RuntimeException();
}

void SwXDocumentIndexMark::attachToRange(const Reference< text::XTextRange > & xTextRange)
                throw( IllegalArgumentException, RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!bIsDescriptor)
        throw RuntimeException();

    Reference<XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
    SwXTextRange* pRange = 0;
    OTextCursorHelper* pCursor = 0;
    if(xRangeTunnel.is())
    {
        pRange = (SwXTextRange*)xRangeTunnel->getSomething(
                                SwXTextRange::getUnoTunnelId());
        pCursor = (OTextCursorHelper*)xRangeTunnel->getSomething(
                                OTextCursorHelper::getUnoTunnelId());
    }

    SwDoc* pDoc = pRange ? (SwDoc*)pRange->GetDoc() : pCursor ? (SwDoc*)pCursor->GetDoc() : 0;

    if(pDoc )
    {
        const SwTOXType* pTOXType = 0;
        switch(eType)
        {
            case TOX_INDEX:
            case TOX_CONTENT:
                pTOXType = pDoc->GetTOXType( eType, 0 );
            break;
            case TOX_USER:
            {
                if(!sUserIndexName.Len())
                    pTOXType = pDoc->GetTOXType( eType, 0 );
                else
                {
                    sal_uInt16 nCount = pDoc->GetTOXTypeCount( eType);
                    for(sal_uInt16 i = 0; i < nCount; i++)
                    {
                        const SwTOXType* pTemp = pDoc->GetTOXType( eType, i );
                        if(sUserIndexName == pTemp->GetTypeName())
                        {
                            pTOXType = pTemp;
                            break;
                        }
                    }
                    if(!pTOXType)
                    {
                        SwTOXType aUserType(TOX_USER, sUserIndexName);
                        pTOXType = pDoc->InsertTOXType(aUserType);
                    }
                }
            }
            break;
            default:
            break;
        }
        if(!pTOXType)
            throw IllegalArgumentException();
        pDoc->GetUnoCallBack()->Add(this);
        ((SwTOXType*)pTOXType)->Add(&aTypeDepend);

        SwUnoInternalPaM aPam(*pDoc);
        //das muss jetzt sal_True liefern
        SwXTextRange::XTextRangeToSwPaM(aPam, xTextRange);
        SwTOXMark aMark (pTOXType);
        if(sAltText.Len())
            aMark.SetAlternativeText(sAltText);
        switch(eType)
        {
            case TOX_INDEX:
                if(sPrimaryKey.Len())
                    aMark.SetPrimaryKey(sPrimaryKey);
                if(sSecondaryKey.Len())
                    aMark.SetSecondaryKey(sSecondaryKey);
                if(sTextReading.Len())
                    aMark.SetTextReading(sTextReading);
                if(sPrimaryKeyReading.Len())
                    aMark.SetPrimaryKeyReading(sPrimaryKeyReading);
                if(sSecondaryKeyReading.Len())
                    aMark.SetSecondaryKeyReading(sSecondaryKeyReading);
                aMark.SetMainEntry(bMainEntry);
            break;
            case TOX_USER:
            case TOX_CONTENT:
                if(USHRT_MAX != nLevel)
                    aMark.SetLevel(nLevel);
            break;
            default:
            break;
        }
        UnoActionContext aAction(pDoc);
        sal_Bool bMark = *aPam.GetPoint() != *aPam.GetMark();
        // Marks ohne Alternativtext ohne selektierten Text koennen nicht eingefuegt werden,
        // deshalb hier ein Leerzeichen - ob das die ideale Loesung ist?
        if(!bMark && !aMark.GetAlternativeText().Len())
            aMark.SetAlternativeText( String(' ') );
        pDoc->Insert(aPam, aMark, SETATTR_DONTEXPAND);
        if( bMark && *aPam.GetPoint() > *aPam.GetMark())
            aPam.Exchange();

        SwTxtAttr* pTxtAttr = 0;
        if( bMark )
            pTxtAttr = aPam.GetNode()->GetTxtNode()->GetTxtAttr(
                            aPam.GetPoint()->nContent, RES_TXTATR_TOXMARK );
        else
            pTxtAttr = aPam.GetNode()->GetTxtNode()->GetTxtAttr(
                aPam.GetPoint()->nContent.GetIndex()-1, RES_TXTATR_TOXMARK );

        if(pTxtAttr)
        {
            m_pTOXMark = &pTxtAttr->GetTOXMark();
            m_pDoc = pDoc;
            bIsDescriptor = sal_False;
        }
        else
            throw RuntimeException();
    }
}

void SwXDocumentIndexMark::attach(const Reference< text::XTextRange > & xTextRange)
                throw( IllegalArgumentException, RuntimeException )
{
    SolarMutexGuard aGuard;
    attachToRange( xTextRange );
}

Reference< text::XTextRange >  SwXDocumentIndexMark::getAnchor(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    Reference< text::XTextRange >  aRet;
    SwTOXType* pType = ((SwXDocumentIndexMark*)this)->GetTOXType();
    if(pType)
    {
        SwTOXMark* pCurMark = lcl_GetMark(pType, GetTOXMark());
        if(pCurMark && pCurMark->GetTxtTOXMark())
        {
            SwTxtTOXMark* pTxtMark = pCurMark->GetTxtTOXMark();
            SwPaM aPam(pTxtMark->GetTxtNode(), *pTxtMark->GetStart());
            aPam.SetMark();
            if(pTxtMark->GetEnd())
            {
                aPam.GetPoint()->nContent = *pTxtMark->GetEnd();
            }
            else
                aPam.GetPoint()->nContent++;
            Reference< frame::XModel >  xModel = m_pDoc->GetDocShell()->GetBaseModel();
            Reference< text::XTextDocument > xTDoc(xModel, uno::UNO_QUERY);
            aRet = new SwXTextRange(aPam, xTDoc->getText());
        }
    }
    if(!aRet.is())
        throw RuntimeException();
    return aRet;
}

void SwXDocumentIndexMark::dispose(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    SwTOXType* pType = ((SwXDocumentIndexMark*)this)->GetTOXType();
    if(!pType)
        throw RuntimeException();
}

void SwXDocumentIndexMark::addEventListener(const Reference< XEventListener > & aListener)
    throw( RuntimeException )
{
    if(!GetRegisteredIn())
        throw RuntimeException();
    aLstnrCntnr.AddListener(aListener);
}

void SwXDocumentIndexMark::removeEventListener(const Reference< XEventListener > & aListener)
    throw( RuntimeException )
{
    if(!GetRegisteredIn() || !aLstnrCntnr.RemoveListener(aListener))
        throw RuntimeException();
}

Reference< XPropertySetInfo >  SwXDocumentIndexMark::getPropertySetInfo(void)
    throw( RuntimeException )
{
    static uno::Reference< beans::XPropertySetInfo >  xInfos[3];
    int nPos = 0;
    switch(eType)
    {
        case TOX_INDEX: nPos = 0; break;
        case TOX_CONTENT: nPos = 1; break;
        case TOX_USER:  nPos = 2; break;
        default: break;
    }
    if(!xInfos[nPos].is())
    {
        uno::Reference< beans::XPropertySetInfo >  xInfo = new SfxItemPropertySetInfo(_pMap);
        // extend PropertySetInfo!
        const uno::Sequence<beans::Property> aPropSeq = xInfo->getProperties();
        xInfos[nPos] = new SfxExtItemPropertySetInfo(
            aSwMapProvider.GetPropertyMap(PROPERTY_MAP_PARAGRAPH_EXTENSIONS),
            aPropSeq );
    }
    return xInfos[nPos];
}

void SwXDocumentIndexMark::setPropertyValue(const OUString& rPropertyName,
                                            const uno::Any& aValue)
    throw( UnknownPropertyException, PropertyVetoException,
        IllegalArgumentException, WrappedTargetException, RuntimeException )
{
    SolarMutexGuard aGuard;
    SwTOXType* pType = ((SwXDocumentIndexMark*)this)->GetTOXType();
    const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                                        _pMap, rPropertyName);
    if (!pMap)
        throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    if ( pMap->nFlags & PropertyAttribute::READONLY)
        throw PropertyVetoException ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    if(pType)
    {
        SwDoc* pLocalDoc = m_pDoc;
        SwTOXMark* pCurMark = lcl_GetMark(pType, GetTOXMark());
        if(pCurMark)
        {
            SwTOXMark aMark(*pCurMark);
            switch(pMap->nWID)
            {
                case WID_ALT_TEXT:
                    aMark.SetAlternativeText(lcl_AnyToString(aValue));
                break;
                case WID_LEVEL:
                    aMark.SetLevel(Min( (sal_Int8) MAXLEVEL,
                                        (sal_Int8)lcl_AnyToInt16(aValue)));
                break;
                case WID_PRIMARY_KEY  :
                    aMark.SetPrimaryKey(lcl_AnyToString(aValue));
                break;
                case WID_SECONDARY_KEY:
                    aMark.SetSecondaryKey(lcl_AnyToString(aValue));
                break;
                case WID_MAIN_ENTRY:
                    aMark.SetMainEntry(lcl_AnyToBool(aValue));
                break;
                case WID_TEXT_READING:
                    aMark.SetTextReading(lcl_AnyToString(aValue));
                break;
                case WID_PRIMARY_KEY_READING:
                    aMark.SetPrimaryKeyReading(lcl_AnyToString(aValue));
                break;
                case WID_SECONDARY_KEY_READING:
                    aMark.SetSecondaryKeyReading(lcl_AnyToString(aValue));
                break;
            }

            SwTxtTOXMark* pTxtMark = pCurMark->GetTxtTOXMark();
            SwPaM aPam(pTxtMark->GetTxtNode(), *pTxtMark->GetStart());
            aPam.SetMark();
            if(pTxtMark->GetEnd())
            {
                aPam.GetPoint()->nContent = *pTxtMark->GetEnd();
            }
            else
                aPam.GetPoint()->nContent++;

            //die alte Marke loeschen
            m_pTOXMark = pCurMark = 0;

            sal_Bool bInsAtPos = aMark.IsAlternativeText();
            const SwPosition *pStt = aPam.Start(),
                                *pEnd = aPam.End();

            SwTxtAttr* pTxtAttr = 0;
            if( bInsAtPos )
            {
                SwPaM aTmp( *pStt );
                pLocalDoc->Insert( aTmp, aMark, 0 );
                pTxtAttr = pStt->nNode.GetNode().GetTxtNode()->GetTxtAttr(
                        pStt->nContent.GetIndex()-1, RES_TXTATR_TOXMARK );
            }
            else if( *pEnd != *pStt )
            {
                pLocalDoc->Insert( aPam, aMark, SETATTR_DONTEXPAND );
                pTxtAttr = pStt->nNode.GetNode().GetTxtNode()->GetTxtAttr(
                                pStt->nContent, RES_TXTATR_TOXMARK );
            }
            m_pDoc = pLocalDoc;
            //und sonst - Marke geloescht?

            if(pTxtAttr)
            {
                m_pTOXMark = &pTxtAttr->GetTOXMark();
                m_pDoc->GetUnoCallBack()->Add(this);
                pType->Add(&aTypeDepend);
            }
        }
    }
    else if(bIsDescriptor)
    {
        switch(pMap->nWID)
        {
            case WID_ALT_TEXT:
                sAltText = lcl_AnyToString(aValue);
            break;
            case WID_LEVEL:
            {
                const sal_Int16 nVal = lcl_AnyToInt16(aValue);
                if(nVal >= 0 && nVal < MAXLEVEL)
                    nLevel = nVal;
                else
                    throw IllegalArgumentException();
            }
            break;
            case WID_PRIMARY_KEY  :
                sPrimaryKey = lcl_AnyToString(aValue);
            break;
            case WID_SECONDARY_KEY:
                sSecondaryKey = lcl_AnyToString(aValue);
            break;
            case WID_TEXT_READING:
                sTextReading = lcl_AnyToString(aValue);
            break;
            case WID_PRIMARY_KEY_READING:
                sPrimaryKeyReading = lcl_AnyToString(aValue);
            break;
            case WID_SECONDARY_KEY_READING:
                sSecondaryKeyReading = lcl_AnyToString(aValue);
            break;
            case WID_USER_IDX_NAME :
            {
                OUString sTmp(lcl_AnyToString(aValue));
                lcl_ConvertTOUNameToUserName(sTmp);
                sUserIndexName = sTmp;
            }
            break;
            case WID_MAIN_ENTRY:
                bMainEntry = lcl_AnyToBool(aValue);
            break;
        }
    }
    else
        throw RuntimeException();
}

uno::Any SwXDocumentIndexMark::getPropertyValue(const OUString& rPropertyName)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    SwTOXType* pType = ((SwXDocumentIndexMark*)this)->GetTOXType();
    const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                                        _pMap, rPropertyName);

    if (!pMap)
        throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
     if(SwXParagraph::getDefaultTextContentValue(aRet, rPropertyName, pMap->nWID))
        return aRet;
    if(pType)
    {
        SwTOXMark* pCurMark = lcl_GetMark(pType, GetTOXMark());
        if(pCurMark)
        {
            switch(pMap->nWID)
            {
                case WID_ALT_TEXT:
                    aRet <<= OUString(pCurMark->GetAlternativeText());
                break;
                case WID_LEVEL:
                    aRet <<= (sal_Int16)pCurMark->GetLevel();
                break;
                case WID_PRIMARY_KEY  :
                    aRet <<= OUString(pCurMark->GetPrimaryKey());
                break;
                case WID_SECONDARY_KEY:
                    aRet <<= OUString(pCurMark->GetSecondaryKey());
                break;
                case WID_TEXT_READING:
                    aRet <<= OUString(pCurMark->GetTextReading());
                break;
                case WID_PRIMARY_KEY_READING:
                    aRet <<= OUString(pCurMark->GetPrimaryKeyReading());
                break;
                case WID_SECONDARY_KEY_READING:
                    aRet <<= OUString(pCurMark->GetSecondaryKeyReading());
                break;
                case WID_USER_IDX_NAME :
                {
                    OUString sTmp(pType->GetTypeName());
                    lcl_ConvertTOUNameToProgrammaticName(sTmp);
                    aRet <<= sTmp;
                }
                break;
                case WID_MAIN_ENTRY:
                {
                    sal_Bool bTemp = pCurMark->IsMainEntry();
                    aRet.setValue(&bTemp, ::getBooleanCppuType());
                }
                break;
            }
        }
    }
    else if(bIsDescriptor)
    {
        switch(pMap->nWID)
        {
            case WID_ALT_TEXT:
                aRet <<= OUString(sAltText);
            break;
            case WID_LEVEL:
                aRet <<= (sal_Int16)nLevel;
            break;
            case WID_PRIMARY_KEY  :
                aRet <<= OUString(sPrimaryKey);
            break;
            case WID_SECONDARY_KEY:
                aRet <<= OUString(sSecondaryKey);
            break;
            case WID_TEXT_READING:
                aRet <<= OUString(sTextReading);
            break;
            case WID_PRIMARY_KEY_READING:
                aRet <<= OUString(sPrimaryKeyReading);
            break;
            case WID_SECONDARY_KEY_READING:
                aRet <<= OUString(sSecondaryKeyReading);
            break;
            case WID_USER_IDX_NAME :
                aRet <<= OUString(sUserIndexName);
            break;
            case WID_MAIN_ENTRY:
            {
                aRet.setValue(&bMainEntry, ::getBooleanCppuType());
            }
            break;
        }
    }
    else
        throw RuntimeException();
    return aRet;
}

void SwXDocumentIndexMark::addPropertyChangeListener(
      const OUString& /*PropertyName*/, const Reference< XPropertyChangeListener > & /*aListener*/)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SwXDocumentIndexMark::removePropertyChangeListener(
        const OUString& /*PropertyName*/, const Reference< XPropertyChangeListener > & /*aListener*/)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SwXDocumentIndexMark::addVetoableChangeListener(
       const OUString& /*PropertyName*/, const Reference< XVetoableChangeListener > & /*aListener*/)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SwXDocumentIndexMark::removeVetoableChangeListener(
        const OUString& /*PropertyName*/, const Reference< XVetoableChangeListener > & /*aListener*/)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented");
}

SwXDocumentIndexMark*   SwXDocumentIndexMark::GetObject(SwTOXType* pType,
                                    const SwTOXMark* pMark, SwDoc* pDoc)
{
    SwClientIter aIter(*pType);
    SwXDocumentIndexMark* pxMark = (SwXDocumentIndexMark*)
                                            aIter.First(TYPE(SwXDocumentIndexMark));
    while( pxMark )
    {
        if(pxMark->GetTOXMark() == pMark)
            return pxMark;
        pxMark = (SwXDocumentIndexMark*)aIter.Next();
    }
    return new SwXDocumentIndexMark(pType, pMark, pDoc);
}

void SwXDocumentIndexMark::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    switch( pOld ? pOld->Which() : 0 )
    {
    case RES_REMOVE_UNO_OBJECT:
    case RES_OBJECTDYING:
        if( (void*)GetRegisteredIn() == ((SwPtrMsgPoolItem *)pOld)->pObject )
            Invalidate();
        break;

    case RES_FMT_CHG:
        // wurden wir an das neue umgehaengt und wird das alte geloscht?
        if( ((SwFmtChg*)pNew)->pChangedFmt == GetRegisteredIn() &&
            ((SwFmtChg*)pOld)->pChangedFmt->IsFmtInDTOR() )
            Invalidate();
        break;
    case  RES_TOXMARK_DELETED:
        if( (void*)m_pTOXMark == ((SwPtrMsgPoolItem *)pOld)->pObject )
            Invalidate();
        break;
    }
}

void SwXDocumentIndexMark::Invalidate()
{
    if(GetRegisteredIn())
    {
        ((SwModify*)GetRegisteredIn())->Remove(this);
        if(aTypeDepend.GetRegisteredIn())
            ((SwModify*)aTypeDepend.GetRegisteredIn())->Remove(&aTypeDepend);
        aLstnrCntnr.Disposing();
        m_pTOXMark = 0;
        m_pDoc = 0;
    }
}

OUString SwXDocumentIndexes::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXDocumentIndexes");
}

BOOL SwXDocumentIndexes::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.DocumentIndexes") == rServiceName;
}

Sequence< OUString > SwXDocumentIndexes::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.DocumentIndexes");
    return aRet;
}

SwXDocumentIndexes::SwXDocumentIndexes(SwDoc* pInDoc) :
    SwUnoCollection(pInDoc)
{
}

SwXDocumentIndexes::~SwXDocumentIndexes()
{
}

sal_Int32 SwXDocumentIndexes::getCount(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();

    sal_uInt32 nRet = 0;
    const SwSectionFmts& rFmts = GetDoc()->GetSections();
    for( sal_uInt16 n = 0; n < rFmts.Count(); ++n )
    {
        const SwSection* pSect = rFmts[ n ]->GetSection();
        if( TOX_CONTENT_SECTION == pSect->GetType() &&
            pSect->GetFmt()->GetSectionNode() )
            ++nRet;
    }
    return nRet;
}

uno::Any SwXDocumentIndexes::getByIndex(sal_Int32 nIndex)
    throw( IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();

    uno::Any aRet;
    sal_Int32 nIdx = 0;

    const SwSectionFmts& rFmts = GetDoc()->GetSections();
    for( sal_uInt16 n = 0; n < rFmts.Count(); ++n )
    {
        const SwSection* pSect = rFmts[ n ]->GetSection();
        if( TOX_CONTENT_SECTION == pSect->GetType() &&
            pSect->GetFmt()->GetSectionNode() &&
            nIdx++ == nIndex )
            {
               Reference< text::XDocumentIndex >  xTmp = new SwXDocumentIndex(
                                    (SwTOXBaseSection*)pSect, GetDoc() );
               aRet.setValue(&xTmp, ::getCppuType((Reference<text::XDocumentIndex>*)0));
               return aRet;
            }
    }

    throw IndexOutOfBoundsException();
    return aRet;
}


uno::Any SwXDocumentIndexes::getByName(const OUString& rName)
    throw( container::NoSuchElementException, WrappedTargetException, RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();

    uno::Any aRet;

    String sToFind(rName);
    const SwSectionFmts& rFmts = GetDoc()->GetSections();
    for( sal_uInt16 n = 0; n < rFmts.Count(); ++n )
    {
        const SwSection* pSect = rFmts[ n ]->GetSection();
        if( TOX_CONTENT_SECTION == pSect->GetType() &&
            pSect->GetFmt()->GetSectionNode() &&
                ((SwTOXBaseSection*)pSect)->GetTOXName() == sToFind)
            {
               Reference< text::XDocumentIndex >  xTmp = new SwXDocumentIndex(
                                    (SwTOXBaseSection*)pSect, GetDoc() );
               aRet.setValue(&xTmp, ::getCppuType((Reference<text::XDocumentIndex>*)0));
               return aRet;
            }
    }
    throw container::NoSuchElementException();
    return aRet;
}

uno::Sequence< OUString > SwXDocumentIndexes::getElementNames(void)
    throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();

    const SwSectionFmts& rFmts = GetDoc()->GetSections();
    sal_Int32 nCount = 0;
    sal_uInt16 n;
    for( n = 0; n < rFmts.Count(); ++n )
    {
        const SwSection* pSect = rFmts[ n ]->GetSection();
        if( TOX_CONTENT_SECTION == pSect->GetType() &&
            pSect->GetFmt()->GetSectionNode() )
            ++nCount;
    }

    uno::Sequence< OUString > aRet(nCount);
    OUString* pArray = aRet.getArray();
    sal_uInt16 nCnt;
    for( n = 0, nCnt = 0; n < rFmts.Count(); ++n )
    {
        const SwSection* pSect = rFmts[ n ]->GetSection();
        if( TOX_CONTENT_SECTION == pSect->GetType() &&
            pSect->GetFmt()->GetSectionNode())
        {
            pArray[nCnt++] = OUString(((SwTOXBaseSection*)pSect)->GetTOXName());
        }
    }
    return aRet;
}

sal_Bool SwXDocumentIndexes::hasByName(const OUString& rName)
    throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();

    String sToFind(rName);
    const SwSectionFmts& rFmts = GetDoc()->GetSections();
    for( sal_uInt16 n = 0; n < rFmts.Count(); ++n )
    {
        const SwSection* pSect = rFmts[ n ]->GetSection();
        if( TOX_CONTENT_SECTION == pSect->GetType() &&
            pSect->GetFmt()->GetSectionNode())
        {
            if(((SwTOXBaseSection*)pSect)->GetTOXName() == sToFind)
                return sal_True;
        }
    }
    return sal_False;
}

uno::Type SwXDocumentIndexes::getElementType(void) throw( RuntimeException )
{
    return ::getCppuType((Reference< text::XDocumentIndex> *)0);
}

sal_Bool SwXDocumentIndexes::hasElements(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();
    return 0 != getCount();
}

SwXDocumentIndex* SwXDocumentIndexes::GetObject(const SwTOXBaseSection* pTOX)
{
    SwSectionFmt* pFmt = pTOX->GetFmt();
    SwClientIter aIter(*pFmt);
    SwXDocumentIndex* pxIdx = (SwXDocumentIndex*)aIter.First(TYPE(SwXDocumentIndex));
    if(pxIdx)
        return pxIdx;
    else
        return new SwXDocumentIndex(pTOX, pFmt->GetDoc());
}


OUString SwXIndexStyleAccess_Impl::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXIndexStyleAccess_Impl");
}

BOOL SwXIndexStyleAccess_Impl::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.DocumentIndexParagraphStyles") == rServiceName;
}

Sequence< OUString > SwXIndexStyleAccess_Impl::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.DocumentIndexParagraphStyles");
    return aRet;
}

SwXIndexStyleAccess_Impl::SwXIndexStyleAccess_Impl(SwXDocumentIndex& rParentIdx) :
    rParent(rParentIdx),
    xParent(&rParentIdx)
{
    SolarMutexGuard aGuard;
    rParent.SetStyleAccess(this);
}

SwXIndexStyleAccess_Impl::~SwXIndexStyleAccess_Impl()
{
    SolarMutexGuard aGuard;
    rParent.SetStyleAccess(0);
}

void SwXIndexStyleAccess_Impl::replaceByIndex(sal_Int32 nIndex, const uno::Any& rElement)
    throw( IllegalArgumentException, IndexOutOfBoundsException,
          WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    const sal_Bool bDescriptor = rParent.IsDescriptor();
    SwSectionFmt* pSectFmt = rParent.GetFmt();
    if(!pSectFmt && !bDescriptor)
        throw RuntimeException();
    if(nIndex < 0 || nIndex > MAXLEVEL)
        throw IndexOutOfBoundsException();
    SwTOXBase* pTOXBase = bDescriptor ? &rParent.GetProperties_Impl()->GetTOXBase() :
            (SwTOXBaseSection*)pSectFmt->GetSection();

    uno::Sequence<OUString> aSeq;
    if(!(rElement >>= aSeq))
        throw IllegalArgumentException();

    sal_uInt16 nStyles = aSeq.getLength();
    const OUString* pStyles = aSeq.getConstArray();
    String sSetStyles;
    String aString;
    for(sal_uInt16 i = 0; i < nStyles; i++)
    {
        if(i)
            sSetStyles += TOX_STYLE_DELIMITER;
        SwStyleNameMapper::FillUIName(pStyles[i], aString, GET_POOLID_TXTCOLL, sal_True);
        sSetStyles +=  aString;
    }
    pTOXBase->SetStyleNames(sSetStyles, (sal_uInt16) nIndex);
}

sal_Int32 SwXIndexStyleAccess_Impl::getCount(void) throw( RuntimeException )
{
    return MAXLEVEL;
}

uno::Any SwXIndexStyleAccess_Impl::getByIndex(sal_Int32 nIndex)
    throw( IndexOutOfBoundsException, WrappedTargetException,
                 RuntimeException)
{
    SolarMutexGuard aGuard;
    const sal_Bool bDescriptor = rParent.IsDescriptor();
    SwSectionFmt* pSectFmt = rParent.GetFmt();
    if(!pSectFmt && !bDescriptor)
        throw RuntimeException();
    if(nIndex < 0 || nIndex > MAXLEVEL)
        throw IndexOutOfBoundsException();
    SwTOXBase* pTOXBase = bDescriptor ? &rParent.GetProperties_Impl()->GetTOXBase() :
                (SwTOXBaseSection*)pSectFmt->GetSection();

    const String& rStyles = pTOXBase->GetStyleNames((sal_uInt16) nIndex);
    sal_uInt16 nStyles = rStyles.GetTokenCount(TOX_STYLE_DELIMITER);
    uno::Sequence<OUString> aStyles(nStyles);
    OUString* pStyles = aStyles.getArray();
    String aString;
    for(sal_uInt16 i = 0; i < nStyles; i++)
    {
        SwStyleNameMapper::FillProgName(
            rStyles.GetToken(i, TOX_STYLE_DELIMITER),
            aString,
            GET_POOLID_TXTCOLL,
            sal_True);
        pStyles[i] = OUString( aString );
    }
    uno::Any aRet(&aStyles, ::getCppuType((uno::Sequence<OUString>*)0));
    return aRet;
}

uno::Type SwXIndexStyleAccess_Impl::getElementType(void)
    throw( RuntimeException )
{
    return ::getCppuType((uno::Sequence<OUString>*)0);
}

sal_Bool SwXIndexStyleAccess_Impl::hasElements(void) throw( RuntimeException )
{
    return sal_True;
}



OUString SwXIndexTokenAccess_Impl::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXIndexTokenAccess_Impl");
}

BOOL SwXIndexTokenAccess_Impl::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.DocumentIndexLevelFormat") == rServiceName;
}

Sequence< OUString > SwXIndexTokenAccess_Impl::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.DocumentIndexLevelFormat");
    return aRet;
}

SwXIndexTokenAccess_Impl::SwXIndexTokenAccess_Impl(SwXDocumentIndex& rParentIdx) :
    rParent(rParentIdx),
    xParent(&rParentIdx),
    nCount(SwForm::GetFormMaxLevel(rParent.GetTOXType()))
{
    SolarMutexGuard aGuard;
    rParent.SetTokenAccess(this);
}

SwXIndexTokenAccess_Impl::~SwXIndexTokenAccess_Impl()
{
    SolarMutexGuard aGuard;
    rParent.SetTokenAccess(0);
}

void SwXIndexTokenAccess_Impl::replaceByIndex(sal_Int32 nIndex, const uno::Any& rElement)
    throw( IllegalArgumentException, IndexOutOfBoundsException,
            WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    const sal_Bool bDescriptor = rParent.IsDescriptor();
    SwSectionFmt* pSectFmt = rParent.GetFmt();
    if(!pSectFmt && !bDescriptor)
        throw RuntimeException();

    SwTOXBase* pTOXBase = bDescriptor ? &rParent.GetProperties_Impl()->GetTOXBase() :
                            (SwTOXBaseSection*)pSectFmt->GetSection();
    if(nIndex < 0 ||
        (nIndex > pTOXBase->GetTOXForm().GetFormMax()))
            throw IndexOutOfBoundsException();

    uno::Sequence<PropertyValues> aSeq;
    if(!(rElement >>= aSeq))
        throw IllegalArgumentException();

    String sPattern;
    sal_uInt16 nTokens = aSeq.getLength();
    const PropertyValues* pTokens = aSeq.getConstArray();
    for(sal_uInt16 i = 0; i < nTokens; i++)
    {
        const PropertyValue* pProperties = pTokens[i].getConstArray();
        sal_uInt16 nProperties = pTokens[i].getLength();
        //create an invalid token
        SwFormToken aToken(TOKEN_END);
        for(sal_uInt16 j = 0; j < nProperties; j++)
        {
            if( COMPARE_EQUAL == pProperties[j].Name.compareToAscii("TokenType"))
            {
                const String sTokenType =
                        lcl_AnyToString(pProperties[j].Value);
                if(sTokenType.EqualsAscii("TokenEntryNumber"))
                    aToken.eTokenType = TOKEN_ENTRY_NO;
                else if(sTokenType.EqualsAscii("TokenEntryText" ))
                    aToken.eTokenType = TOKEN_ENTRY_TEXT;
                else if(sTokenType.EqualsAscii("TokenTabStop"   ))
                    aToken.eTokenType = TOKEN_TAB_STOP;
                else if(sTokenType.EqualsAscii("TokenText"      ))
                    aToken.eTokenType = TOKEN_TEXT;
                else if(sTokenType.EqualsAscii("TokenPageNumber"))
                    aToken.eTokenType = TOKEN_PAGE_NUMS;
                else if(sTokenType.EqualsAscii("TokenChapterInfo"      ))
                    aToken.eTokenType = TOKEN_CHAPTER_INFO;
                else if(sTokenType.EqualsAscii("TokenHyperlinkStart" ))
                    aToken.eTokenType = TOKEN_LINK_START;
                else if(sTokenType.EqualsAscii("TokenHyperlinkEnd"))
                    aToken.eTokenType = TOKEN_LINK_END;
                else if(sTokenType.EqualsAscii("TokenBibliographyDataField" ))
                    aToken.eTokenType = TOKEN_AUTHORITY;
            }
            else if( pProperties[j].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("CharacterStyleName"  )  ))
            {
                String sCharStyleName;
                SwStyleNameMapper::FillUIName(
                        lcl_AnyToString(pProperties[j].Value),
                        sCharStyleName,
                        GET_POOLID_CHRFMT,
                        sal_True);
                aToken.sCharStyleName = sCharStyleName;
                aToken.nPoolId = SwStyleNameMapper::GetPoolIdFromUIName (
                            sCharStyleName, GET_POOLID_CHRFMT );
            }
            else if( pProperties[j].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("TabStopRightAligned") ))
            {
                sal_Bool bRight = lcl_AnyToBool(pProperties[j].Value);
                aToken.eTabAlign = bRight ?
                                    SVX_TAB_ADJUST_END : SVX_TAB_ADJUST_LEFT;
            }
            else if( pProperties[j].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("TabStopPosition"  )))
            {
                sal_Int32 nPosition(0);
                if(pProperties[j].Value.getValueType() != ::getCppuType((sal_Int32*)0))
                    throw IllegalArgumentException();
                pProperties[j].Value >>= nPosition;
                nPosition = MM100_TO_TWIP(nPosition);
                if(nPosition < 0)
                    throw IllegalArgumentException();
                aToken.nTabStopPosition = nPosition;
            }
            else if( pProperties[j].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("TabStopFillCharacter" )))
            {
                const String sFillChar =
                    lcl_AnyToString(pProperties[j].Value);
                if(sFillChar.Len() > 1)
                    throw IllegalArgumentException();
                aToken.cTabFillChar = sFillChar.Len() ?
                                sFillChar.GetChar(0) : ' ';
            }
            else if( pProperties[j].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Text" )))
               {
                const String sText =
                    lcl_AnyToString(pProperties[j].Value);
                aToken.sText = sText;
            }
            else if( pProperties[j].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("ChapterFormat"    )))
            {
                sal_Int16 nFormat = lcl_AnyToInt16(pProperties[j].Value);
                switch(nFormat)
                {
                    case text::ChapterFormat::NUMBER:           nFormat = CF_NUMBER;
                    break;
                    case text::ChapterFormat::NAME:             nFormat = CF_TITLE;
                    break;
                    case text::ChapterFormat::NAME_NUMBER:      nFormat = CF_NUM_TITLE;
                    break;
                    case text::ChapterFormat::NO_PREFIX_SUFFIX:nFormat = CF_NUMBER_NOPREPST;
                    break;
                    case text::ChapterFormat::DIGIT:           nFormat = CF_NUM_NOPREPST_TITLE;
                    break;
                    default:
                        throw IllegalArgumentException();
                }
                aToken.nChapterFormat = nFormat;
            }
            else if( pProperties[j].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("BibliographyDataField")))
            {
                sal_Int16 nType(0); pProperties[j].Value >>= nType;
                if(nType < 0 || nType > BibliographyDataField::ISBN)
                {
                    IllegalArgumentException aExcept;
                    aExcept.Message = C2U("BibliographyDataField - wrong value");
                    aExcept.ArgumentPosition = j;
                    throw aExcept;
                }
                aToken.nAuthorityField = nType;
            }

        }
        //exception if wrong TokenType
        if(TOKEN_END <= aToken.eTokenType )
            throw IllegalArgumentException();
        // set TokenType from TOKEN_ENTRY_TEXT to TOKEN_ENTRY if it is
        // not a content index
        if(TOKEN_ENTRY_TEXT == aToken.eTokenType &&
                                TOX_CONTENT != pTOXBase->GetType())
            aToken.eTokenType = TOKEN_ENTRY;
        sPattern += aToken.GetString();
    }
    SwForm aForm(pTOXBase->GetTOXForm());
    aForm.SetPattern((sal_uInt16) nIndex, sPattern);
    pTOXBase->SetTOXForm(aForm);
}

sal_Int32 SwXIndexTokenAccess_Impl::getCount(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    const sal_Bool bDescriptor = rParent.IsDescriptor();
    SwSectionFmt* pSectFmt = rParent.GetFmt();
    if(!pSectFmt && !bDescriptor)
        throw RuntimeException();
    sal_Int32 nRet = bDescriptor ?
        nCount :
        ((SwTOXBaseSection*)pSectFmt->GetSection())->
                                            GetTOXForm().GetFormMax();
    return nRet;
}

uno::Any SwXIndexTokenAccess_Impl::getByIndex(sal_Int32 nIndex)
    throw( IndexOutOfBoundsException, WrappedTargetException,
         RuntimeException)
{
    SolarMutexGuard aGuard;
    const sal_Bool bDescriptor = rParent.IsDescriptor();
    SwSectionFmt* pSectFmt = rParent.GetFmt();
    if(!pSectFmt && !bDescriptor)
        throw RuntimeException();
    SwTOXBase* pTOXBase = bDescriptor ? &rParent.GetProperties_Impl()->GetTOXBase() :
            (SwTOXBaseSection*)pSectFmt->GetSection();
    if(nIndex < 0 ||
    (nIndex > pTOXBase->GetTOXForm().GetFormMax()))
        throw IndexOutOfBoundsException();

    SwFormTokenEnumerator aEnumerator(pTOXBase->GetTOXForm().
                                        GetPattern((sal_uInt16) nIndex));
    sal_uInt16 nTokenCount = 0;
    uno::Sequence< PropertyValues > aRetSeq;
    String aString;
    while(aEnumerator.HasNextToken())
    {
        nTokenCount++;
        aRetSeq.realloc(nTokenCount);
        PropertyValues* pTokenProps = aRetSeq.getArray();
        SwFormToken  aToken = aEnumerator.GetNextToken();

        Sequence< PropertyValue >& rCurTokenSeq = pTokenProps[nTokenCount-1];
        SwStyleNameMapper::FillProgName(
                        aToken.sCharStyleName,
                        aString,
                        GET_POOLID_CHRFMT,
                        sal_True );
        const OUString aProgCharStyle( aString );
        switch(aToken.eTokenType)
        {
            case TOKEN_ENTRY_NO     :
            {
                rCurTokenSeq.realloc( 2 );
                PropertyValue* pArr = rCurTokenSeq.getArray();

                pArr[0].Name = C2U("TokenType");
                pArr[0].Value <<= OUString( RTL_CONSTASCII_USTRINGPARAM( "TokenEntryNumber" ));
//              pArr[0].Value <<= C2U("TokenEntryNumber");

                pArr[1].Name = C2U("CharacterStyleName");
                pArr[1].Value <<= aProgCharStyle;
            }
            break;
            case TOKEN_ENTRY        :   // no difference between Entry and Entry Text
            case TOKEN_ENTRY_TEXT   :
            {
                rCurTokenSeq.realloc( 2 );
                PropertyValue* pArr = rCurTokenSeq.getArray();

                pArr[0].Name = C2U("TokenType");
                pArr[0].Value <<= OUString( RTL_CONSTASCII_USTRINGPARAM( "TokenEntryText" ));

                pArr[1].Name = C2U("CharacterStyleName");
                pArr[1].Value <<= aProgCharStyle;
            }
            break;
            case TOKEN_TAB_STOP     :
            {
                rCurTokenSeq.realloc(4);
                PropertyValue* pArr = rCurTokenSeq.getArray();

                pArr[0].Name = C2U("TokenType");
                pArr[0].Value <<= OUString( RTL_CONSTASCII_USTRINGPARAM( "TokenTabStop" ));


                if(SVX_TAB_ADJUST_END == aToken.eTabAlign)
                {
                    pArr[1].Name = C2U("TabStopRightAligned");
                    BOOL bTemp = sal_True;
                    pArr[1].Value.setValue(&bTemp, ::getCppuBooleanType());
                }
                else
                {
                    pArr[1].Name = C2U("TabStopPosition");
                    sal_Int32 nPos = (TWIP_TO_MM100(aToken.nTabStopPosition));
                    if(nPos < 0)
                        nPos = 0;
                    pArr[1].Value <<= (sal_Int32)nPos;
                }
                pArr[2].Name = C2U("TabStopFillCharacter");
                pArr[2].Value <<= OUString(aToken.cTabFillChar);
                pArr[3].Name = C2U("CharacterStyleName");
                pArr[3].Value <<= aProgCharStyle;
            }
            break;
            case TOKEN_TEXT         :
            {
                rCurTokenSeq.realloc( 3 );
                PropertyValue* pArr = rCurTokenSeq.getArray();

                pArr[0].Name = C2U("TokenType");
                pArr[0].Value <<= OUString( RTL_CONSTASCII_USTRINGPARAM( "TokenText" ));

                pArr[1].Name = C2U("CharacterStyleName");
                pArr[1].Value <<= aProgCharStyle;

                pArr[2].Name = C2U("Text");
                pArr[2].Value <<= OUString(aToken.sText);
            }
            break;
            case TOKEN_PAGE_NUMS    :
            {
                rCurTokenSeq.realloc( 2 );
                PropertyValue* pArr = rCurTokenSeq.getArray();

                pArr[0].Name = C2U("TokenType");
                pArr[0].Value <<= OUString( RTL_CONSTASCII_USTRINGPARAM( "TokenPageNumber" ));

                pArr[1].Name = C2U("CharacterStyleName");
                pArr[1].Value <<= aProgCharStyle;
            }
            break;
            case TOKEN_CHAPTER_INFO :
            {
                rCurTokenSeq.realloc( 3 );
                PropertyValue* pArr = rCurTokenSeq.getArray();

                pArr[0].Name = C2U("TokenType");
                pArr[0].Value <<= OUString( RTL_CONSTASCII_USTRINGPARAM( "TokenChapterInfo" ));

                pArr[1].Name = C2U("CharacterStyleName");
                pArr[1].Value <<= aProgCharStyle;

                pArr[2].Name = C2U("ChapterFormat");
                sal_Int16 nVal = text::ChapterFormat::NUMBER;
                switch(aToken.nChapterFormat)
                {
                    case CF_NUMBER:             nVal = text::ChapterFormat::NUMBER; break;
                    case CF_TITLE:              nVal = text::ChapterFormat::NAME; break;
                    case CF_NUM_TITLE:          nVal = text::ChapterFormat::NAME_NUMBER; break;
                    case CF_NUMBER_NOPREPST:    nVal = text::ChapterFormat::NO_PREFIX_SUFFIX; break;
                    case CF_NUM_NOPREPST_TITLE: nVal = text::ChapterFormat::DIGIT; break;
                }
                pArr[2].Value <<= (sal_Int16)nVal;
            }
            break;
            case TOKEN_LINK_START   :
            {
                rCurTokenSeq.realloc( 1 );
                PropertyValue* pArr = rCurTokenSeq.getArray();

                pArr[0].Name = C2U("TokenType");
                pArr[0].Value <<= OUString( RTL_CONSTASCII_USTRINGPARAM( "TokenHyperlinkStart" ));
            }
            break;
            case TOKEN_LINK_END     :
            {
                rCurTokenSeq.realloc( 1 );
                PropertyValue* pArr = rCurTokenSeq.getArray();

                pArr[0].Name = C2U("TokenType");
                pArr[0].Value <<= OUString( RTL_CONSTASCII_USTRINGPARAM( "TokenHyperlinkEnd" ));
            }
            break;
            case TOKEN_AUTHORITY :
            {
                rCurTokenSeq.realloc( 3 );
                PropertyValue* pArr = rCurTokenSeq.getArray();

                pArr[0].Name = C2U("TokenType");
                pArr[0].Value <<= OUString( RTL_CONSTASCII_USTRINGPARAM( "TokenBibliographyDataField" ));

                pArr[1].Name = C2U("CharacterStyleName");
                pArr[1].Value <<= aProgCharStyle;

                pArr[2].Name = C2U("BibliographyDataField");
                pArr[2].Value <<= sal_Int16(aToken.nAuthorityField);
            }
            break;
            default:
            break;
        }
    }

    uno::Any aRet(&aRetSeq, ::getCppuType((uno::Sequence< PropertyValues >*)0));

    return aRet;
}

uno::Type  SwXIndexTokenAccess_Impl::getElementType(void)
    throw( RuntimeException )
{
    return ::getCppuType((uno::Sequence< PropertyValues >*)0);
}

sal_Bool SwXIndexTokenAccess_Impl::hasElements(void) throw( RuntimeException )
{
    return sal_True;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
