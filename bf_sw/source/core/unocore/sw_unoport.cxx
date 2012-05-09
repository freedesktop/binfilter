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

#include <cmdid.h>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

#include <osl/diagnose.h>

#include <unocrsrhelper.hxx>
#include <unoport.hxx>
#include <unoobj.hxx>
#include <unomap.hxx>
#include <unoprnms.hxx>
#include <unocrsr.hxx>
#include <unomid.h>
#include <txtatr.hxx>
#include <txtfld.hxx>
#include <ndtxt.hxx>

#include <horiornt.hxx>

#include <doc.hxx>
#include <fmtflcnt.hxx>
#include <fmtfld.hxx>
#include <frmfmt.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;

using rtl::OUString;


/******************************************************************
 * SwXTextPortion
 ******************************************************************/


SwFmtFld*   SwXTextPortion::GetFldFmt(sal_Bool bInit)
{
    SwFmtFld* pRet = 0;
    // initial wird es immer gesucht, danach nur noch, wenn es bereits existierte
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr && (bInit || pFmtFld))
    {
        SwTxtNode *pNode = pUnoCrsr->GetPoint()->nNode.GetNode().GetTxtNode();
        SwTxtFld *pTxtFld = 0;
        if( pNode )
            pTxtFld = pNode->GetTxtFld( pUnoCrsr->Start()->nContent );
        if(pTxtFld)
            pFmtFld = pRet = (SwFmtFld*)&pTxtFld->GetFld();
    }
    return pRet;
}

SwXTextPortion::SwXTextPortion(const SwUnoCrsr* pPortionCrsr,
        uno::Reference< XText > & rParent,
        SwTextPortionType eType) :
    aLstnrCntnr( (XTextRange*)this),
    aPropSet(aSwMapProvider.GetPropertyMap(
                (PORTION_REDLINE_START == eType ||
                 PORTION_REDLINE_END   == eType) ?
                    PROPERTY_MAP_REDLINE_PORTION : PROPERTY_MAP_TEXTPORTION_EXTENSIONS)),
    xParentText(rParent),
    pRubyText(0),
    pRubyStyle(0),
    pRubyAdjust(0),
    pRubyIsAbove(0),
    pFmtFld(0),
    aFrameDepend(this, 0),
    pFrameFmt(0),
    ePortionType(eType),
    nControlChar(0),
    bIsCollapsed(FALSE)
{
    SwUnoCrsr* pUnoCrsr = pPortionCrsr->GetDoc()->CreateUnoCrsr(*pPortionCrsr->GetPoint());
    if(pPortionCrsr->HasMark())
    {
        pUnoCrsr->SetMark();
        *pUnoCrsr->GetMark() = *pPortionCrsr->GetMark();
    }
    pUnoCrsr->Add(this);
    // erst nach ->Add()
    if(ePortionType == PORTION_FIELD)
        GetFldFmt(sal_True);
//  else if(ePortionType == PORTION_FRAME)
//      ...;
}

SwXTextPortion::SwXTextPortion(const SwUnoCrsr* pPortionCrsr, uno::Reference< XText > & rParent,
                        SwFrmFmt& rFmt ) :
    aLstnrCntnr( (XTextRange*)this),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXTPORTION_EXTENSIONS)),
    xParentText(rParent),
    pRubyText(0),
    pRubyStyle(0),
    pRubyAdjust(0),
    pRubyIsAbove(0),
    pFmtFld(0),
    aFrameDepend(this, &rFmt),
    pFrameFmt(&rFmt),
    ePortionType(PORTION_FRAME),
    nControlChar(0),
    bIsCollapsed(FALSE)
{
    SwUnoCrsr* pUnoCrsr = pPortionCrsr->GetDoc()->CreateUnoCrsr(*pPortionCrsr->GetPoint());
    if(pPortionCrsr->HasMark())
    {
        pUnoCrsr->SetMark();
        *pUnoCrsr->GetMark() = *pPortionCrsr->GetMark();
    }
    pUnoCrsr->Add(this);
}


SwXTextPortion::~SwXTextPortion()
{
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    delete pUnoCrsr;
    delete pRubyText;
    delete pRubyStyle;
    delete pRubyAdjust;
    delete pRubyIsAbove;
}

uno::Reference< XText >  SwXTextPortion::getText(void) throw( uno::RuntimeException )
{
    return xParentText;
}

uno::Reference< XTextRange >  SwXTextPortion::getStart(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< XTextRange >  xRet;
    SwUnoCrsr* pUnoCrsr = ((SwXTextPortion*)this)->GetCrsr();
    if(pUnoCrsr)
    {
        SwPaM aPam(*pUnoCrsr->Start());
        uno::Reference< XText >  xParent = getText();
        xRet = new SwXTextRange(aPam, xParent);
    }
    else
        throw uno::RuntimeException();
    return xRet;
}

uno::Reference< XTextRange >  SwXTextPortion::getEnd(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< XTextRange >  xRet;
    SwUnoCrsr* pUnoCrsr = ((SwXTextPortion*)this)->GetCrsr();
    if(pUnoCrsr)
    {
        SwPaM aPam(*pUnoCrsr->End());
        uno::Reference< XText >  xParent = getText();
        xRet = new SwXTextRange(aPam, xParent);
    }
    else
        throw uno::RuntimeException();
    return xRet;
}

OUString SwXTextPortion::getString(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    String aTxt;
    SwUnoCrsr* pUnoCrsr = ((SwXTextPortion*)this)->GetCrsr();
    if(pUnoCrsr)
    {
        //TextPortions liegen immer innerhalb eines Absatzes
        SwTxtNode* pTxtNd = pUnoCrsr->GetNode()->GetTxtNode();
        if( pTxtNd )
        {
            xub_StrLen nStt = pUnoCrsr->Start()->nContent.GetIndex();
            aTxt = pTxtNd->GetExpandTxt( nStt,
                    pUnoCrsr->End()->nContent.GetIndex() - nStt );
        }
    }
    else
        throw uno::RuntimeException();
    return aTxt;
}

void SwXTextPortion::setString(const OUString& aString) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
        SwXTextCursor::SetString(*pUnoCrsr, aString);
    else
        throw uno::RuntimeException();
}

uno::Reference< beans::XPropertySetInfo >  SwXTextPortion::getPropertySetInfo(void) throw( uno::RuntimeException )
{
    //! PropertySetInfo for text portion extensions
    static uno::Reference< beans::XPropertySetInfo >
            xTxtPorExtRef = SfxItemPropertySet( aSwMapProvider.GetPropertyMap(
                    PROPERTY_MAP_TEXTPORTION_EXTENSIONS) ).getPropertySetInfo();
    //! PropertySetInfo for redline portions
    static uno::Reference< beans::XPropertySetInfo >
            xRedlPorRef = SfxItemPropertySet( aSwMapProvider.GetPropertyMap(
                    PROPERTY_MAP_REDLINE_PORTION) ).getPropertySetInfo();

    return (PORTION_REDLINE_START == ePortionType ||
            PORTION_REDLINE_END   == ePortionType) ? xRedlPorRef : xTxtPorExtRef;
}

void SwXTextPortion::setPropertyValue(const OUString& rPropertyName,
    const uno::Any& aValue)
    throw( beans::UnknownPropertyException,
        beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwUnoCrsr* pUnoCrsr = ((SwXTextPortion*)this)->GetCrsr();
    if(pUnoCrsr)
    {
        SwXTextCursor::SetPropertyValue(*pUnoCrsr, aPropSet, rPropertyName, aValue);
    }
    else
        throw uno::RuntimeException();
}


void SwXTextPortion::GetPropertyValues( const OUString *pPropertyNames,
                                   uno::Any *pValues,
                                      sal_Int32 nLength )
{
    SwUnoCrsr* pUnoCrsr = ((SwXTextPortion*)this)->GetCrsr();
    if(pUnoCrsr)
    {
        SfxItemSet *pSet = 0;
        const SfxItemPropertyMap*   pMap = aPropSet.getPropertyMap();
        for(sal_Int32 nProp = 0; nProp < nLength; nProp++)
        {
            pMap = SfxItemPropertyMap::GetByName(pMap, pPropertyNames[nProp]);
            if(pMap)
            {
                switch(pMap->nWID)
                {
                    case FN_UNO_TEXT_PORTION_TYPE:
                    {
                        const char* pRet;
                        switch (ePortionType)
                        {
                        case PORTION_TEXT:          pRet = "Text";break;
                        case PORTION_FIELD:         pRet = "TextField";break;
                        case PORTION_FRAME:         pRet = "Frame";break;
                        case PORTION_FOOTNOTE:      pRet = "Footnote";break;
                        case PORTION_CONTROL_CHAR:  pRet = "ControlCharacter";break;
                        case PORTION_REFMARK_START:
                        case PORTION_REFMARK_END:   pRet = SW_PROP_NAME_STR(UNO_NAME_REFERENCE_MARK);break;
                        case PORTION_TOXMARK_START:
                        case PORTION_TOXMARK_END:   pRet = SW_PROP_NAME_STR(UNO_NAME_DOCUMENT_INDEX_MARK);break;
                        case PORTION_BOOKMARK_START :
                        case PORTION_BOOKMARK_END : pRet = SW_PROP_NAME_STR(UNO_NAME_BOOKMARK);break;
                        case PORTION_REDLINE_START:
                        case PORTION_REDLINE_END:   pRet = "Redline";break;
                        case PORTION_RUBY_START:
                        case PORTION_RUBY_END:      pRet = "Ruby";break;
                        default:
                            pRet = 0;
                        }

                        OUString sRet;
                        if( pRet )
                            sRet = C2U( pRet );
                        pValues[nProp] <<= sRet;
                    }
                    break;
                    case FN_UNO_CONTROL_CHARACTER:
                    {
                        if(PORTION_CONTROL_CHAR == ePortionType)
                            pValues[nProp] <<= (sal_Int16) nControlChar;
                    }
                    break;
                    case FN_UNO_DOCUMENT_INDEX_MARK:
                        pValues[nProp] <<= xTOXMark;
                    break;
                    case FN_UNO_REFERENCE_MARK:
                        pValues[nProp] <<= xRefMark;
                    break;
                    case FN_UNO_BOOKMARK:
                        pValues[nProp] <<= xBookmark;
                    break;
                    case FN_UNO_FOOTNOTE:
                        pValues[nProp] <<= xFootnote;
                    break;
                    case FN_UNO_IS_COLLAPSED:
                    {
                        switch (ePortionType)
                        {
                            case PORTION_REFMARK_START:
                            case PORTION_BOOKMARK_START :
                            case PORTION_TOXMARK_START:
                            case PORTION_REFMARK_END:
                            case PORTION_TOXMARK_END:
                            case PORTION_BOOKMARK_END :
                            case PORTION_REDLINE_START :
                            case PORTION_REDLINE_END :
                            case PORTION_RUBY_START:
                            case PORTION_RUBY_END:
                                pValues[nProp].setValue(&bIsCollapsed, ::getBooleanCppuType());
                            break;
                            default:
                            break;
                        }
                    }
                    break;
                    case FN_UNO_IS_START:
                    {
                        BOOL bStart = TRUE, bPut = TRUE;
                        switch (ePortionType)
                        {
                            case PORTION_REFMARK_START:
                            case PORTION_BOOKMARK_START:
                            case PORTION_TOXMARK_START:
                            case PORTION_REDLINE_START:
                            case PORTION_RUBY_START:
                            break;

                            case PORTION_REFMARK_END:
                            case PORTION_TOXMARK_END:
                            case PORTION_BOOKMARK_END:
                            case PORTION_REDLINE_END:
                            case PORTION_RUBY_END:
                                bStart = FALSE;
                            break;
                            default:
                                bPut = FALSE;
                        }
                        if(bPut)
                            pValues[nProp].setValue(&bStart, ::getBooleanCppuType());
                    }
                    break;
                    case RES_TXTATR_CJK_RUBY:
                    {
                        Any* pToSet = 0;
                        switch(pMap->nMemberId)
                        {
                            case MID_RUBY_TEXT :    pToSet = pRubyText;     break;
                            case MID_RUBY_ADJUST :  pToSet = pRubyAdjust;   break;
                            case MID_RUBY_CHARSTYLE:pToSet = pRubyStyle;    break;
                            case MID_RUBY_ABOVE :   pToSet = pRubyIsAbove;  break;
                        }
                        if(pToSet)
                            pValues[nProp] = *pToSet;
                    }
                    break;
                    default:
                        PropertyState eTemp;
                        BOOL bDone = SwUnoCursorHelper::getCrsrPropertyValue(
                                            pMap, *pUnoCrsr, &(pValues[nProp]), eTemp );
                        if(!bDone)
                        {
                            if(!pSet)
                            {
                                   pSet = new SfxItemSet(pUnoCrsr->GetDoc()->GetAttrPool(),
                                    RES_CHRATR_BEGIN,   RES_PARATR_NUMRULE,
                                    RES_UNKNOWNATR_CONTAINER, RES_UNKNOWNATR_CONTAINER,
                                    RES_TXTATR_UNKNOWN_CONTAINER, RES_TXTATR_UNKNOWN_CONTAINER,
                                    RES_FILL_ORDER,     RES_FRMATR_END -1,
                                    0L);
                                SwXTextCursor::GetCrsrAttr(*pUnoCrsr, *pSet);
                            }
                            pValues[nProp] = aPropSet.getPropertyValue(*pMap, *pSet);
                        }
                }
                pMap++;
            }
            else
                throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + pPropertyNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );
        }
        delete pSet;
    }
    else
        throw uno::RuntimeException();
}

uno::Any SwXTextPortion::getPropertyValue(
    const OUString& rPropertyName)
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    SolarMutexGuard aGuard;
    Any aAny;
    GetPropertyValues( &rPropertyName, &aAny, 1 );
    return aAny;
}

void SwXTextPortion::setPropertyValues(
    const Sequence< OUString >& rPropertyNames,
    const Sequence< Any >& aValues )
        throw(PropertyVetoException, IllegalArgumentException,
            WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    SwUnoCrsr* pUnoCrsr = ((SwXTextPortion*)this)->GetCrsr();
    if(pUnoCrsr)
    {
        const OUString* pPropertyNames = rPropertyNames.getConstArray();
        const Any* pValues = aValues.getConstArray();
        const SfxItemPropertyMap*   pMap = aPropSet.getPropertyMap();
        OUString sTmp;
        for(sal_Int32 nProp = 0; nProp < rPropertyNames.getLength(); nProp++)
        {
            pMap = SfxItemPropertyMap::GetByName(pMap, pPropertyNames[nProp]);
            if (!pMap)
                throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + pPropertyNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );
            if ( pMap->nFlags & PropertyAttribute::READONLY)
                throw PropertyVetoException ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + pPropertyNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );

            SwXTextCursor::SetPropertyValue( *pUnoCrsr, aPropSet, sTmp, pValues[nProp], pMap);
        }
    }
    else
        throw uno::RuntimeException();
}

Sequence< Any > SwXTextPortion::getPropertyValues(
    const Sequence< OUString >& rPropertyNames )
        throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    Sequence< Any > aValues(rPropertyNames.getLength());

    GetPropertyValues( rPropertyNames.getConstArray(),
                       aValues.getArray(),
                       rPropertyNames.getLength() );
    return aValues;
}

void SwXTextPortion::addPropertiesChangeListener(
    const Sequence< OUString >& /*aPropertyNames*/,
    const Reference< XPropertiesChangeListener >& /*xListener*/ )
        throw(RuntimeException)
{}

void SwXTextPortion::removePropertiesChangeListener(
    const Reference< XPropertiesChangeListener >& /*xListener*/ )
        throw(RuntimeException)
{}

void SwXTextPortion::firePropertiesChangeEvent(
    const Sequence< OUString >& /*aPropertyNames*/,
    const Reference< XPropertiesChangeListener >& /*xListener*/ )
        throw(RuntimeException)
{}

void SwXTextPortion::addPropertyChangeListener(
    const OUString& /*PropertyName*/,
    const Reference< XPropertyChangeListener > & /*aListener*/)
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SwXTextPortion::removePropertyChangeListener(
        const OUString& /*PropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SwXTextPortion::addVetoableChangeListener(
        const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SwXTextPortion::removeVetoableChangeListener(
        const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}

beans::PropertyState SwXTextPortion::getPropertyState(const OUString& rPropertyName)
            throw( beans::UnknownPropertyException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    beans::PropertyState eRet = beans::PropertyState_DEFAULT_VALUE;
    SwUnoCrsr* pUnoCrsr = ((SwXTextPortion*)this)->GetCrsr();
    if(pUnoCrsr)
    {
        if(GetTextPortionType() == PORTION_RUBY_START &&
           !rPropertyName.compareToAscii( RTL_CONSTASCII_STRINGPARAM("Ruby") ))
            eRet = beans::PropertyState_DIRECT_VALUE;
        else
            eRet = SwXTextCursor::GetPropertyState(*pUnoCrsr, aPropSet, rPropertyName);
    }
    else
        throw RuntimeException();
    return eRet;
}

uno::Sequence< beans::PropertyState > SwXTextPortion::getPropertyStates(
        const uno::Sequence< OUString >& rPropertyNames)
        throw( beans::UnknownPropertyException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwUnoCrsr* pUnoCrsr = ((SwXTextPortion*)this)->GetCrsr();
    if(!pUnoCrsr)
        throw RuntimeException();
    Sequence< PropertyState > aRet = SwXTextCursor::GetPropertyStates(*pUnoCrsr, aPropSet, rPropertyNames, SW_PROPERTY_STATE_CALLER_SWX_TEXT_PORTION);

    if(GetTextPortionType() == PORTION_RUBY_START)
    {
        const OUString* pNames = rPropertyNames.getConstArray();
        PropertyState* pStates = aRet.getArray();
        for(sal_Int32 nProp = 0; nProp < rPropertyNames.getLength();nProp++)
        {
           if(!pNames[nProp].compareToAscii( RTL_CONSTASCII_STRINGPARAM("Ruby") ))
                pStates[nProp] = beans::PropertyState_DIRECT_VALUE;
        }
    }
    return aRet;
}

void SwXTextPortion::setPropertyToDefault(const OUString& rPropertyName)
                throw( beans::UnknownPropertyException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwXTextCursor::SetPropertyToDefault(*pUnoCrsr, aPropSet, rPropertyName);
    }
    else
        throw uno::RuntimeException();
}

uno::Any SwXTextPortion::getPropertyDefault(const OUString& rPropertyName)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    uno::Any aRet;
    SwUnoCrsr* pUnoCrsr = ((SwXTextPortion*)this)->GetCrsr();
    if(pUnoCrsr)
    {
        aRet = SwXTextCursor::GetPropertyDefault(*pUnoCrsr, aPropSet, rPropertyName);
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

OUString SwXTextPortion::getPresentation(sal_Bool /*bShowCommand*/) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwUnoCrsr* pUnoCrsr = ((SwXTextPortion*)this)->GetCrsr();
    String sRet;
    SwFmtFld* pFmt = 0;
    if(pUnoCrsr && 0 != (pFmt = GetFldFmt()))
    {
        const SwField* pField = pFmt->GetFld();
        OSL_FAIL("bShowCommand auswerten!");
        sRet = pField->Expand();
    }
    return sRet;
}

void SwXTextPortion::attach(const uno::Reference< XTextRange > & /*xTextRange*/)
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    // SwXTextPortion cannot be created at the factory therefore
    // they cannot be attached
    throw uno::RuntimeException();
}

uno::Reference< XTextRange >  SwXTextPortion::getAnchor(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< XTextRange >  aRet;
    SwUnoCrsr* pUnoCrsr = ((SwXTextPortion*)this)->GetCrsr();
    if(pUnoCrsr)
        aRet = new SwXTextRange(*pUnoCrsr, xParentText);
    else
        throw uno::RuntimeException();
    return aRet;
}

void SwXTextPortion::dispose(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwUnoCrsr* pUnoCrsr = ((SwXTextPortion*)this)->GetCrsr();
    if(pUnoCrsr)
    {
        setString(aEmptyStr);
        pUnoCrsr->Remove(this);
    }
    else
        throw uno::RuntimeException();
}

void SwXTextPortion::addEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!GetRegisteredIn())
        throw uno::RuntimeException();
    aLstnrCntnr.AddListener(aListener);
}

void SwXTextPortion::removeEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!GetRegisteredIn() || !aLstnrCntnr.RemoveListener(aListener))
        throw uno::RuntimeException();
}

uno::Reference< container::XEnumeration >  SwXTextPortion::createContentEnumeration(const OUString& /*aServiceName*/)
        throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(!pUnoCrsr)
        throw uno::RuntimeException();
    uno::Reference< container::XEnumeration >  xRet = new SwXParaFrameEnumeration(*pUnoCrsr, PARAFRAME_PORTION_CHAR, pFrameFmt);
    return xRet;

}

const uno::Sequence< sal_Int8 > & SwXTextPortion::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::binfilter::CreateUnoTunnelId();
    return aSeq;
}

sal_Int64 SwXTextPortion::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
            return (sal_Int64)this;
    }
    return 0;
}

uno::Sequence< OUString > SwXTextPortion::getAvailableServiceNames(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TextContent");
    return aRet;
}

OUString SwXTextPortion::getImplementationName(void) throw( uno::RuntimeException )
{
    return C2U("SwXTextPortion");
}

sal_Bool SwXTextPortion::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(!pUnoCrsr)
        throw uno::RuntimeException();

    sal_Bool bRet = sal_False;
    if(!rServiceName.compareToAscii("com.sun.star.text.TextPortion") ||
            !rServiceName.compareToAscii("com.sun.star.style.CharacterProperties") ||
            !rServiceName.compareToAscii("com.sun.star.style.CharacterPropertiesAsian") ||
            !rServiceName.compareToAscii("com.sun.star.style.CharacterPropertiesComplex") ||
            !rServiceName.compareToAscii("com.sun.star.style.ParagraphProperties") ||
            !rServiceName.compareToAscii("com.sun.star.style.ParagraphPropertiesAsian") ||
            !rServiceName.compareToAscii("com.sun.star.style.ParagraphPropertiesComplex"))
        bRet = sal_True;
    else if(COMPARE_EQUAL == rServiceName.compareToAscii("com.sun.star.text.TextField"))
        bRet = 0 != GetFldFmt();
    else
    {
        SwFrmFmt* pCurFrmFmt = pFrameFmt;
        if(!pCurFrmFmt && !pUnoCrsr->HasMark() && pUnoCrsr->GetCntntNode()->Len())
        {
            // hier die zeichengebundenen am Cursor suchen - hier kann es nur einen geben
            SwTxtAttr* pTxtAttr = pUnoCrsr->GetNode()->GetTxtNode()->GetTxtAttr(
                            pUnoCrsr->GetPoint()->nContent, RES_TXTATR_FLYCNT);
            DBG_ASSERT(pTxtAttr, "Hier muss doch ein Fly-Attribut zu finden sein!");
            if(pTxtAttr)
            {
                const SwFmtFlyCnt& rFlyCnt = pTxtAttr->GetFlyCnt();
                pCurFrmFmt = rFlyCnt.GetFrmFmt();
            }
        }
        if(pCurFrmFmt)
        {
            const SwNodeIndex* pIdx = pCurFrmFmt->GetCntnt().GetCntntIdx();
            const SwNode* pNd = GetCrsr()->GetDoc()->GetNodes()[ pIdx->GetIndex() + 1 ];

            if((!pNd->IsNoTxtNode() && COMPARE_EQUAL == rServiceName.compareToAscii("com.sun.star.text.TextFrame"))||
                (pNd->IsGrfNode() && COMPARE_EQUAL == rServiceName.compareToAscii("com.sun.star.text.TextGraphicObject")) ||
                (pNd->IsOLENode() && COMPARE_EQUAL == rServiceName.compareToAscii("com.sun.star.text.TextEmbeddedObject")))
                bRet = sal_True;
        }
    }
    return bRet;
}

uno::Sequence< OUString > SwXTextPortion::getSupportedServiceNames(void)
                                                throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(!pUnoCrsr)
        throw uno::RuntimeException();
    sal_Bool bField = 0 != GetFldFmt();
    sal_uInt16 nCount = bField ? 8 : 7;
    uno::Sequence< OUString > aRet(nCount);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TextPortion");
    pArray[1] = C2U("com.sun.star.style.CharacterProperties");
    pArray[2] = C2U("com.sun.star.style.CharacterPropertiesAsian");
    pArray[3] = C2U("com.sun.star.style.CharacterPropertiesComplex");
    pArray[4] = C2U("com.sun.star.style.ParagraphProperties");
    pArray[5] = C2U("com.sun.star.style.ParagraphPropertiesAsian");
    pArray[6] = C2U("com.sun.star.style.ParagraphPropertiesComplex");
    if(bField)
        pArray[7] = C2U("com.sun.star.text.TextField");
    else
    {
        SwFrmFmt* pCurFrmFmt = pFrameFmt;
        if(!pCurFrmFmt && !pUnoCrsr->HasMark())
        {
            // hier die zeichengebundenen am Cursor suchen - hier kann es nur einen geben
            SwTxtAttr* pTxtAttr = pUnoCrsr->GetNode()->GetTxtNode()->GetTxtAttr(
                            pUnoCrsr->GetPoint()->nContent, RES_TXTATR_FLYCNT);
            // if any - it could also be an empty paragraph
            if(pTxtAttr)
            {
                const SwFmtFlyCnt& rFlyCnt = pTxtAttr->GetFlyCnt();
                pCurFrmFmt = rFlyCnt.GetFrmFmt();
            }
        }
        if(pCurFrmFmt)
        {
            aRet.realloc(8);
            pArray = aRet.getArray();
            const SwNodeIndex* pIdx = pCurFrmFmt->GetCntnt().GetCntntIdx();
            const SwNode* pNd = GetCrsr()->GetDoc()->GetNodes()[ pIdx->GetIndex() + 1 ];
            if(!pNd->IsNoTxtNode())
                pArray[7] = C2U("com.sun.star.text.TextFrame");
            else if(pNd->IsGrfNode())
                pArray[7] = C2U("com.sun.star.text.TextGraphicObject");
            else
                pArray[7] = C2U("com.sun.star.text.TextEmbeddedObject");
        }
    }
    return aRet;
}

void SwXTextPortion::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!aFrameDepend.GetRegisteredIn())
        pFrameFmt = 0;
}

SwXRubyPortion::SwXRubyPortion(const SwUnoCrsr* pPortionCrsr,
                    SwTxtRuby& rAttr,
                    Reference< XText > & rParent,
                    sal_Bool bEnd   ) :
        SwXTextPortion(pPortionCrsr, rParent, bEnd ? PORTION_RUBY_END : PORTION_RUBY_START  )
{
    if(!bEnd)
    {
        const SfxPoolItem& rItem = rAttr.GetAttr();
        pRubyText = new Any;
        pRubyStyle = new Any;
        pRubyAdjust = new Any;
        pRubyIsAbove = new Any;
        rItem.QueryValue(*pRubyText, MID_RUBY_TEXT);
        rItem.QueryValue(*pRubyStyle, MID_RUBY_CHARSTYLE);
        rItem.QueryValue(*pRubyAdjust, MID_RUBY_ADJUST);
        rItem.QueryValue(*pRubyIsAbove, MID_RUBY_ABOVE);
    }
}

SwXRubyPortion::~SwXRubyPortion()
{
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
