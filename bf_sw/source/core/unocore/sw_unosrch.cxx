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
#include <horiornt.hxx>

#include <doc.hxx>
#include <hints.hxx>
#include <unosrch.hxx>
#include <unomap.hxx>

#include <osl/diagnose.h>

#include <unoobj.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/util/SearchOptions.hpp>
#include <com/sun/star/util/SearchFlags.hpp>
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::beans;

using rtl::OUString;
/******************************************************************************
 *
 ******************************************************************************/


class SwSearchProperties_Impl
{
    PropertyValue**             pValueArr; //
    sal_uInt16                      nArrLen;
public:
    SwSearchProperties_Impl();
    ~SwSearchProperties_Impl();

    void    SetProperties(const Sequence< PropertyValue >& aSearchAttribs)
        throw( UnknownPropertyException, lang::IllegalArgumentException, RuntimeException );
    const Sequence< PropertyValue > GetProperties() const;

    void    FillItemSet(SfxItemSet& rSet, sal_Bool bIsValueSearch) const;
    sal_Bool    HasAttributes() const;
};

SwSearchProperties_Impl::SwSearchProperties_Impl() :
    nArrLen(0)
{
    const SfxItemPropertyMap* pMap = aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_CURSOR);
    while(pMap->nWID)
    {
        if(pMap->nWID < RES_FRMATR_END)
            nArrLen++;
        pMap++;
    }
    pValueArr = new PropertyValue*[nArrLen];
    *pValueArr = new PropertyValue[nArrLen];
    for(sal_uInt16 i = 0; i < nArrLen; i++)
        pValueArr[i] = 0;
}

SwSearchProperties_Impl::~SwSearchProperties_Impl()
{
    for(sal_uInt16 i = 0; i < nArrLen; i++)
        delete pValueArr[i];
    delete[] pValueArr;
}

void    SwSearchProperties_Impl::SetProperties(const Sequence< PropertyValue >& aSearchAttribs)
                throw( UnknownPropertyException, lang::IllegalArgumentException, RuntimeException )
{
    const SfxItemPropertyMap* pMap = aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_CURSOR);
    long nLen = aSearchAttribs.getLength();
    const PropertyValue* pProps = aSearchAttribs.getConstArray();
    //delete all existing values
    long i=0;
    for( i = 0; i < nArrLen; i++)
    {
        delete pValueArr[i];
        pValueArr[i] = 0;
    }

    for(i = 0; i < nLen; i++)
    {
        String sPropertyName(pProps[i].Name);
        sal_uInt16 nIndex = 0;
        const SfxItemPropertyMap* pTempMap = pMap;
        while(pTempMap->nWID && !sPropertyName.EqualsAscii(pTempMap->pName))
        {
            pTempMap++;
            nIndex++;
        }
        if(!pTempMap->nWID)
            throw UnknownPropertyException();
        pValueArr[nIndex] = new PropertyValue(pProps[i]);
    }
}

const Sequence< PropertyValue > SwSearchProperties_Impl::GetProperties() const
{
    sal_uInt16 nPropCount = 0;
    sal_uInt16 i=0;
    for(i = 0; i < nArrLen; i++)
        if(pValueArr[i])
            nPropCount++;

    Sequence< PropertyValue > aRet(nPropCount);
    PropertyValue* pProps = aRet.getArray();
    nPropCount = 0;
    for(i = 0; i < nArrLen; i++)
    {
        if(pValueArr[i])
        {
            pProps[nPropCount] = *(pValueArr[i]);
        }
        nPropCount++;
    }
    return aRet;
}

void SwSearchProperties_Impl::FillItemSet(SfxItemSet& rSet, sal_Bool bIsValueSearch) const
{
    const SfxItemPropertyMap* pMap = aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_CURSOR);
    //

    SfxPoolItem* pBoxItem = 0,
    *pBreakItem = 0,
    *pAutoKernItem  = 0,
    *pWLineItem   = 0,
    *pTabItem  = 0,
    *pSplitItem  = 0,
    *pRegItem  = 0,
    *pLineSpaceItem  = 0,
    *pLineNumItem  = 0,
    *pKeepItem  = 0,
    *pLRItem  = 0,
    *pULItem  = 0,
    *pBackItem  = 0,
    *pAdjItem  = 0,
    *pDescItem  = 0,
    *pInetItem  = 0,
    *pDropItem  = 0,
    *pWeightItem  = 0,
    *pULineItem  = 0,
    *pCharFmtItem  = 0,
    *pShadItem  = 0,
    *pPostItem  = 0,
    *pNHyphItem  = 0,
    *pLangItem  = 0,
    *pKernItem  = 0,
    *pFontSizeItem  = 0,
    *pFontItem  = 0,
    *pBlinkItem  = 0,
    *pEscItem  = 0,
    *pCrossedOutItem  = 0,
    *pContourItem  = 0,
    *pCharColorItem  = 0,
    *pCasemapItem  = 0,
    *pBrushItem  = 0;

    for(sal_uInt16 i = 0; i < nArrLen; i++)
    {
        if(pValueArr[i])
        {
            const SfxItemPropertyMap*   pTempMap =  pMap + i;
            SfxPoolItem* pTempItem = 0;
            switch(pTempMap->nWID)
            {
                case  RES_BOX:
                    if(!pBoxItem)
                        pBoxItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pBoxItem;
                break;
                case  RES_BREAK:
                    if(!pBreakItem)
                        pBreakItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pBreakItem;
                break;
                case  RES_CHRATR_AUTOKERN:
                    if(!pAutoKernItem)
                        pAutoKernItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pAutoKernItem;
                    break;
                case  RES_CHRATR_BACKGROUND:
                    if(!pBrushItem)
                        pBrushItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pBrushItem;
                break;
                case  RES_CHRATR_CASEMAP:
                    if(!pCasemapItem)
                        pCasemapItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pCasemapItem;
                break;
                case  RES_CHRATR_COLOR:
                    if(!pCharColorItem)
                        pCharColorItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pCharColorItem;
                break;
                case  RES_CHRATR_CONTOUR:
                    if(!pContourItem)
                        pContourItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pContourItem;
                break;
                case  RES_CHRATR_CROSSEDOUT:
                    if(!pCrossedOutItem)
                        pCrossedOutItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pCrossedOutItem;
                break;
                case  RES_CHRATR_ESCAPEMENT:
                    if(!pEscItem)
                        pEscItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pEscItem;
                break;
                case  RES_CHRATR_BLINK:
                    if(!pBlinkItem)
                        pBlinkItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pBlinkItem;
                break;
                case  RES_CHRATR_FONT:
                    if(!pFontItem)
                        pFontItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pFontItem;
                break;
                case  RES_CHRATR_FONTSIZE:
                    if(!pFontSizeItem)
                        pFontSizeItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pFontSizeItem;
                break;
                case  RES_CHRATR_KERNING:
                    if(!pKernItem)
                        pKernItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pKernItem;
                break;
                case  RES_CHRATR_LANGUAGE:
                    if(!pLangItem)
                        pLangItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pLangItem;
                break;
                case  RES_CHRATR_NOHYPHEN:
                    if(!pNHyphItem)
                        pNHyphItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pNHyphItem;
                break;
                case  RES_CHRATR_POSTURE:
                    if(!pPostItem)
                        pPostItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pPostItem;
                break;
                case  RES_CHRATR_SHADOWED:
                    if(!pShadItem)
                        pShadItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pShadItem;
                break;
                case  RES_TXTATR_CHARFMT:
                    if(!pCharFmtItem)
                        pCharFmtItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pCharFmtItem;
                break;
                case  RES_CHRATR_UNDERLINE:
                    if(!pULineItem)
                        pULineItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pULineItem;
                break;
                case  RES_CHRATR_WEIGHT:
                    if(!pWeightItem)
                        pWeightItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pWeightItem;
                break;
                case  RES_PARATR_DROP:
                    if(!pDropItem)
                        pDropItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pDropItem;
                break;
                case  RES_TXTATR_INETFMT:
                    if(!pInetItem)
                        pInetItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pInetItem;
                break;
                case  RES_PAGEDESC:
                    if(!pDescItem)
                        pDescItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pDescItem;
                break;
                case  RES_PARATR_ADJUST:
                    if(!pAdjItem)
                        pAdjItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pAdjItem;
                break;
                case  RES_BACKGROUND:
                    if(!pBackItem)
                        pBackItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pBackItem;
                break;
                case  RES_UL_SPACE:
                    if(!pULItem)
                        pULItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pULItem;
                break;
                case  RES_LR_SPACE:
                    if(!pLRItem)
                        pLRItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pLRItem;
                break;
                case  RES_KEEP:
                    if(!pKeepItem)
                        pKeepItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pKeepItem;
                break;
                case  RES_LINENUMBER:
                    if(!pLineNumItem)
                        pLineNumItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pLineNumItem;
                break;
                case  RES_PARATR_LINESPACING:
                    if(!pLineSpaceItem)
                        pLineSpaceItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pLineSpaceItem;
                break;
                case  RES_PARATR_REGISTER:
                    if(!pRegItem)
                        pRegItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pRegItem;
                break;
                case  RES_PARATR_SPLIT:
                    if(!pSplitItem)
                        pSplitItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pSplitItem;
                break;
                case  RES_PARATR_TABSTOP:
                    if(!pTabItem)
                        pTabItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pTabItem;
                break;
                case  RES_CHRATR_WORDLINEMODE:
                    if(!pWLineItem)
                        pWLineItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pWLineItem;
                break;
            }
            if(pTempItem)
            {
                if(bIsValueSearch)
                {
                    pTempItem->PutValue(pValueArr[i]->Value, pTempMap->nMemberId);
                    rSet.Put(*pTempItem);
                }
                else
                    rSet.InvalidateItem( pTempItem->Which() );
            }
        }
    }
    delete pBoxItem;
    delete pBreakItem ;
    delete pAutoKernItem ;
    delete pWLineItem;
    delete pTabItem;
    delete pSplitItem;
    delete pRegItem;
    delete pLineSpaceItem ;
    delete pLineNumItem  ;
    delete pKeepItem;
    delete pLRItem  ;
    delete pULItem  ;
    delete pBackItem;
    delete pAdjItem;
    delete pDescItem;
    delete pInetItem;
    delete pDropItem;
    delete pWeightItem;
    delete pULineItem;
    delete pCharFmtItem  ;
    delete pShadItem;
    delete pPostItem;
    delete pNHyphItem;
    delete pLangItem;
    delete pKernItem;
    delete pFontSizeItem ;
    delete pFontItem;
    delete pBlinkItem;
    delete pEscItem;
    delete pCrossedOutItem;
    delete pContourItem  ;
    delete pCharColorItem;
    delete pCasemapItem  ;
    delete pBrushItem  ;
}

sal_Bool    SwSearchProperties_Impl::HasAttributes() const
{
    for(sal_uInt16 i = 0; i < nArrLen; i++)
        if(pValueArr[i])
            return sal_True;
    return sal_False;
}

/*-- 14.12.98 13:07:10    ---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextSearch::SwXTextSearch() :
    pSearchProperties( new SwSearchProperties_Impl),
    pReplaceProperties( new SwSearchProperties_Impl),
    _pMap(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_SEARCH)),
    bAll(sal_False),
    bWord(sal_False),
    bBack(sal_False),
    bExpr(sal_False),
    bCase(sal_False),
    bStyles(sal_False),
    bSimilarity(sal_False),
    bLevRelax(sal_False),
    nLevExchange(2),
    nLevAdd(2),
    nLevRemove(2),
    bIsValueSearch(sal_True)
{
}
/*-- 14.12.98 13:07:12    ---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextSearch::~SwXTextSearch()
{
    delete pSearchProperties;
    delete pReplaceProperties;
}

const Sequence< sal_Int8 > & SwXTextSearch::getUnoTunnelId()
{
    static Sequence< sal_Int8 > aSeq = ::binfilter::CreateUnoTunnelId();
    return aSeq;
}

sal_Int64 SAL_CALL SwXTextSearch::getSomething( const Sequence< sal_Int8 >& rId )
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

OUString SwXTextSearch::getSearchString(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    return sSearchText;
}

void SwXTextSearch::setSearchString(const OUString& rString)
                                        throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    sSearchText = String(rString);
}

OUString SwXTextSearch::getReplaceString(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    return sReplaceText;
}

void SwXTextSearch::setReplaceString(const OUString& rReplaceString) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    sReplaceText = String(rReplaceString);
}

Reference< XPropertySetInfo >  SwXTextSearch::getPropertySetInfo(void) throw( RuntimeException )
{
    static Reference< XPropertySetInfo >  aRef = new SfxItemPropertySetInfo(_pMap);
    return aRef;
}

void SwXTextSearch::setPropertyValue(const OUString& rPropertyName, const Any& aValue)
    throw( UnknownPropertyException, PropertyVetoException,
        lang::IllegalArgumentException, lang::WrappedTargetException, RuntimeException )
{
    SolarMutexGuard aGuard;
    const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                                _pMap, rPropertyName);
    if(pMap)
    {
        if ( pMap->nFlags & PropertyAttribute::READONLY)
            throw PropertyVetoException ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
        sal_Bool bVal = FALSE;
        if(aValue.getValueType() == ::getBooleanCppuType())
            bVal = *(sal_Bool*)aValue.getValue();
        switch(pMap->nWID)
        {
            case WID_SEARCH_ALL :           bAll        = bVal; break;
            case WID_WORDS:                 bWord       = bVal; break;
            case WID_BACKWARDS :            bBack       = bVal; break;
            case WID_REGULAR_EXPRESSION :   bExpr       = bVal; break;
            case WID_CASE_SENSITIVE  :      bCase       = bVal; break;
            //case WID_IN_SELECTION  :      bInSel      = bVal; break;
            case WID_STYLES          :      bStyles     = bVal; break;
            case WID_SIMILARITY      :      bSimilarity = bVal; break;
            case WID_SIMILARITY_RELAX:      bLevRelax   = bVal; break;
            case WID_SIMILARITY_EXCHANGE:   aValue >>= nLevExchange; break;
            case WID_SIMILARITY_ADD:        aValue >>= nLevAdd; break;
            case WID_SIMILARITY_REMOVE :    aValue >>= nLevRemove;break;
            break;
        };
    }
    else
        throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
}

Any SwXTextSearch::getPropertyValue(const OUString& rPropertyName) throw( UnknownPropertyException, lang::WrappedTargetException, RuntimeException )
{
    SolarMutexGuard aGuard;
    Any aRet;

    const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                                _pMap, rPropertyName);
    sal_Bool bSet = sal_False;
    sal_Int16 nSet = 0;
    if(pMap)
    {
        switch(pMap->nWID)
        {
            case WID_SEARCH_ALL :           bSet = bAll; goto SET_BOOL;
            case WID_WORDS:                 bSet = bWord; goto SET_BOOL;
            case WID_BACKWARDS :            bSet = bBack; goto SET_BOOL;
            case WID_REGULAR_EXPRESSION :   bSet = bExpr; goto SET_BOOL;
            case WID_CASE_SENSITIVE  :      bSet = bCase; goto SET_BOOL;
            //case WID_IN_SELECTION  :      bSet = bInSel; goto SET_BOOL;
            case WID_STYLES          :      bSet = bStyles; goto SET_BOOL;
            case WID_SIMILARITY      :      bSet = bSimilarity; goto SET_BOOL;
            case WID_SIMILARITY_RELAX:      bSet = bLevRelax;
SET_BOOL:
            aRet.setValue(&bSet, ::getBooleanCppuType());
            break;
            case WID_SIMILARITY_EXCHANGE:   nSet = nLevExchange; goto SET_UINT16;
            case WID_SIMILARITY_ADD:        nSet = nLevAdd; goto SET_UINT16;
            case WID_SIMILARITY_REMOVE :    nSet = nLevRemove;
SET_UINT16:
            aRet <<= nSet;
            break;
        };
    }
    else
        throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    return aRet;
}

void SwXTextSearch::addPropertyChangeListener(
        const OUString& /*PropertyName*/, const Reference< XPropertyChangeListener > & /*aListener*/)
    throw( UnknownPropertyException, lang::WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SwXTextSearch::removePropertyChangeListener(
        const OUString& /*PropertyName*/, const Reference< XPropertyChangeListener > & /*aListener*/)
    throw( UnknownPropertyException, lang::WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SwXTextSearch::addVetoableChangeListener(
        const OUString& /*PropertyName*/, const Reference< XVetoableChangeListener > & /*aListener*/)
    throw( UnknownPropertyException, lang::WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SwXTextSearch::removeVetoableChangeListener(
        const OUString& /*PropertyName*/, const Reference< XVetoableChangeListener > & /*aListener*/)
    throw( UnknownPropertyException, lang::WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented");
}

sal_Bool SwXTextSearch::getValueSearch(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    return bIsValueSearch;
}

void SwXTextSearch::setValueSearch(sal_Bool ValueSearch_) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    bIsValueSearch = ValueSearch_;
}

Sequence< PropertyValue > SwXTextSearch::getSearchAttributes(void) throw( RuntimeException )
{
    return  pSearchProperties->GetProperties();
}

void SwXTextSearch::setSearchAttributes(const Sequence< PropertyValue >& rSearchAttribs)
    throw( UnknownPropertyException, lang::IllegalArgumentException, RuntimeException )
{
    pSearchProperties->SetProperties(rSearchAttribs);
}

Sequence< PropertyValue > SwXTextSearch::getReplaceAttributes(void)
    throw( RuntimeException )
{
    return pReplaceProperties->GetProperties();
}

void SwXTextSearch::setReplaceAttributes(const Sequence< PropertyValue >& rReplaceAttribs)
    throw( UnknownPropertyException, lang::IllegalArgumentException, RuntimeException )
{
    pReplaceProperties->SetProperties(rReplaceAttribs);
}

void    SwXTextSearch::FillSearchItemSet(SfxItemSet& rSet) const
{
    pSearchProperties->FillItemSet(rSet, bIsValueSearch);
}

void    SwXTextSearch::FillReplaceItemSet(SfxItemSet& rSet) const
{
    pReplaceProperties->FillItemSet(rSet, bIsValueSearch);
}

sal_Bool    SwXTextSearch::HasSearchAttributes() const
{
    return pSearchProperties->HasAttributes();
}

sal_Bool    SwXTextSearch::HasReplaceAttributes() const
{
    return pReplaceProperties->HasAttributes();
}

OUString SwXTextSearch::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXTextSearch");
}

BOOL SwXTextSearch::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.util.SearchDescriptor") == rServiceName ||
            C2U("com.sun.star.util.ReplaceDescriptor") == rServiceName;
}

Sequence< OUString > SwXTextSearch::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.util.SearchDescriptor");
    pArray[1] = C2U("com.sun.star.util.ReplaceDescriptor");
    return aRet;
}

void SwXTextSearch::FillSearchOptions( SearchOptions& rSearchOpt ) const
{
    if( bSimilarity )
    {
        rSearchOpt.algorithmType = SearchAlgorithms_APPROXIMATE;
        rSearchOpt.changedChars = nLevExchange;
        rSearchOpt.deletedChars = nLevRemove;
        rSearchOpt.insertedChars = nLevAdd;
        if( bLevRelax )
            rSearchOpt.searchFlag |= SearchFlags::LEV_RELAXED;
    }
    else if( bExpr )
        rSearchOpt.algorithmType = SearchAlgorithms_REGEXP;
    else
        rSearchOpt.algorithmType = SearchAlgorithms_ABSOLUTE;

    rSearchOpt.searchString = sSearchText;
    rSearchOpt.replaceString = sReplaceText;

    if( !bCase )
        rSearchOpt.transliterateFlags |= TransliterationModules_IGNORE_CASE;
    if( bWord )
        rSearchOpt.searchFlag |= SearchFlags::NORM_WORD_ONLY;
}



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
