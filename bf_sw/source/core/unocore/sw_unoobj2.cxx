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

#include <rtl/ustrbuf.hxx>
#include <swtypes.hxx>
#include <hintids.hxx>
#include <cmdid.h>
#include <hints.hxx>
#include <bookmrk.hxx>
#include <frmfmt.hxx>

#include <horiornt.hxx>

#include <doc.hxx>

#include <osl/diagnose.h>

#include <ndtxt.hxx>
#include <ndnotxt.hxx>
#include <unocrsr.hxx>
#include <rootfrm.hxx>
#include <flyfrm.hxx>
#include <ftnidx.hxx>
#include <bf_so3/linkmgr.hxx>
#include <docary.hxx>
#include <paratr.hxx>
#include <tools/urlobj.hxx>
#include <pam.hxx>
#include <tools/cachestr.hxx>
#include <shellio.hxx>
#include <swerror.h>
#include <swtblfmt.hxx>
#include <fmthbsh.hxx>
#include <docsh.hxx>
#include <bf_svtools/style.hxx>
#include <docstyle.hxx>
#include <charfmt.hxx>
#include <txtfld.hxx>
#include <fmtfld.hxx>
#include <fmtpdsc.hxx>
#include <pagedesc.hxx>
#include <poolfmt.hrc>
#include <poolfmt.hxx>
#include <viscrs.hxx>
#include <fchrfmt.hxx>
#include <cntfrm.hxx>
#include <pagefrm.hxx>
#include <doctxm.hxx>
#include <tox.hxx>
#include <bf_sfx2/docfilt.hxx>
#include <bf_sfx2/docfile.hxx>
#include <bf_sfx2/fcontnr.hxx>
#include <fmtrfmrk.hxx>
#include <txtrfmrk.hxx>
#include <unoclbck.hxx>
#include <unoobj.hxx>
#include <unomap.hxx>
#include <unoport.hxx>
#include <unomid.h>
#include <unocrsrhelper.hxx>
#include <unosett.hxx>
#include <unoprnms.hxx>
#include <unotbl.hxx>
#include <unodraw.hxx>
#include <unocoll.hxx>
#include <unostyle.hxx>
#include <unofield.hxx>
#include <fmtanchr.hxx>
#include <bf_svx/flstitem.hxx>
#include <bf_svx/unolingu.hxx>
#include <bf_svtools/ctrltool.hxx>
#include <bf_svtools/eitem.hxx>
#include <txtftn.hxx>
#include <section.hxx>
#include <fmtftn.hxx>

#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/style/PageStyleLayout.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <unotools/collatorwrapper.hxx>
#include <com/sun/star/table/TableSortField.hpp>
#include <unoidx.hxx>
#include <unoframe.hxx>
#include <fmthdft.hxx>
#include <osl/mutex.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <fmtflcnt.hxx>
#define _SVSTDARR_USHORTS
#define _SVSTDARR_USHORTSSORT
#define _SVSTDARR_XUB_STRLEN
#include <bf_svtools/svstdarr.hxx>
#include <bf_svx/brshitem.hxx>
#include <bf_svtools/stritem.hxx>
#include <fmtclds.hxx>
#include <rtl/uuid.h>
#include <dcontact.hxx>
#include <flyfrm.hxx>
#include <cntfrm.hxx>
#include <dflyobj.hxx>
#include <crsskip.hxx>
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::drawing;


using rtl::OUString;

void CollectFrameAtNode( SwClient& rClnt, const SwNodeIndex& rIdx,
                            SwDependArr& rFrameArr, BOOL bSort )
{
    // bSortFlag says: TRUE - search AutoContent Flys,
    //                 else search AtContent Flys

    // alle Rahmen, Grafiken und OLEs suchen, die an diesem Absatz
    // gebunden sind
    SvXub_StrLens aSortArr( 8, 8 );
    SwDoc* pDoc = rIdx.GetNode().GetDoc();

    USHORT nChkType = bSort ? FLY_AUTO_CNTNT : FLY_AT_CNTNT;
    const SwSpzFrmFmts& rFmts = *pDoc->GetSpzFrmFmts();
    USHORT nSize = rFmts.Count();
    for ( USHORT i = 0; i < nSize; i++)
    {
        const SwFrmFmt* pFmt = rFmts[ i ];
        const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
        const SwPosition* pAnchorPos;
        if( rAnchor.GetAnchorId() == nChkType &&
            0 != (pAnchorPos = rAnchor.GetCntntAnchor()) &&
                pAnchorPos->nNode == rIdx )
        {
            //jetzt einen SwDepend anlegen und in das Array einfuegen
            SwDepend* pNewDepend = new SwDepend( &rClnt, (SwFrmFmt*)pFmt);
            USHORT nInsPos = rFrameArr.Count();
            if( bSort )
            {
                xub_StrLen nInsertIndex = pAnchorPos->nContent.GetIndex();

                USHORT nEnd = nInsPos;
                for( nInsPos = 0; nInsPos < nEnd; ++nInsPos )
                    if( aSortArr[ nInsPos ] > nInsertIndex )
                        break;
                aSortArr.Insert( nInsertIndex, nInsPos );
            }
            rFrameArr.C40_INSERT( SwDepend, pNewDepend, nInsPos );
        }
    }
}



void SwXTextCursor::insertDocumentFromURL(const OUString& rURL,
    const uno::Sequence< beans::PropertyValue >& aOptions)
    throw( lang::IllegalArgumentException, io::IOException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        String sFilterName;
        String sFilterOption;
        String sPassword;
        OUString uFilterName(C2U(SW_PROP_NAME_STR(UNO_NAME_FILTER_NAME)));
        OUString uFilterOption(C2U(SW_PROP_NAME_STR(UNO_NAME_FILTER_OPTION)));
        OUString uPassword(C2U(SW_PROP_NAME_STR(UNO_NAME_PASSWORD)));
        sal_Bool bIllegalArgument = sal_False;

        for ( int n = 0; n < aOptions.getLength(); ++n )
        {
            // get Property-Value from options
            const beans::PropertyValue &rProp = aOptions.getConstArray()[n];
            uno::Any aValue( rProp.Value );

            // FilterName-Property?
            if ( rProp.Name == uFilterName )
            {
                if ( rProp.Value.getValueType() == ::getCppuType((const OUString*)0))
                {
                    OUString uLclFilterName;
                    rProp.Value >>= uLclFilterName;
                    sFilterName = String(uLclFilterName);
                }
                else if ( rProp.Value.getValueType() != ::getVoidCppuType() )
                    bIllegalArgument = sal_True;
            }
            else if( rProp.Name == uFilterOption )
            {
                if ( rProp.Value.getValueType() == ::getCppuType((const OUString*)0))
                {
                    OUString uLclFilterOption;
                    rProp.Value >>= uLclFilterOption;
                    sFilterOption = String(uLclFilterOption) ;
                }
                else if ( rProp.Value.getValueType() != ::getVoidCppuType() )

                    bIllegalArgument = sal_True;
            }
            else if( rProp.Name == uPassword )
            {
                if ( rProp.Value.getValueType() == ::getCppuType((const OUString*)0))
                {
                    OUString uLclPassword;
                    rProp.Value >>= uLclPassword;
                    sPassword = String(uLclPassword );
                }
                else if ( rProp.Value.getValueType() != ::getVoidCppuType() )
                    bIllegalArgument = sal_True;
            }
            else if(rProp.Value.getValueType() != ::getVoidCppuType())
                bIllegalArgument = sal_True;
        }
        if(bIllegalArgument)
            throw lang::IllegalArgumentException();
        SwUnoCursorHelper::InsertFile(pUnoCrsr, rURL, sFilterName, sFilterOption, sPassword);
    }
    else
        throw uno::RuntimeException();
}

uno::Sequence< beans::PropertyValue > SwXTextCursor::createSortDescriptor(sal_Bool bFromTable)
{
    uno::Sequence< beans::PropertyValue > aRet(5);
    beans::PropertyValue* pArray = aRet.getArray();

    uno::Any aVal;
    sal_Bool bTrue = sal_True;
    aVal.setValue( &bFromTable, ::getCppuBooleanType());
    pArray[0] = beans::PropertyValue(C2U("IsSortInTable"), -1, aVal, beans::PropertyState_DIRECT_VALUE);

    String sSpace(String::CreateFromAscii(" "));
    sal_Unicode uSpace = sSpace.GetChar(0);

    aVal <<= uSpace;
    pArray[1] = beans::PropertyValue(C2U("Delimiter"), -1, aVal, beans::PropertyState_DIRECT_VALUE);

    aVal.setValue( &bTrue, ::getCppuBooleanType());
    pArray[2] = beans::PropertyValue(C2U("IsSortColumns"), -1, aVal, beans::PropertyState_DIRECT_VALUE);

    aVal <<= (INT32)3;
    pArray[3] = beans::PropertyValue(C2U("MaxSortFieldsCount"), -1, aVal, beans::PropertyState_DIRECT_VALUE);

    uno::Sequence< table::TableSortField > aFields(3);
    table::TableSortField* pFields = aFields.getArray();

    Locale aLang( SvxCreateLocale( LANGUAGE_SYSTEM ) );
    // get collator algorithm to be used for the locale
    Sequence < OUString > aSeq( GetAppCollator().listCollatorAlgorithms( aLang ) );
    INT32 nLen = aSeq.getLength();
    DBG_ASSERT( nLen > 0, "list of collator algorithms is empty!");
    OUString aCollAlg;
    if (nLen > 0)
        aCollAlg = aSeq.getConstArray()[0];

    pFields[0].Field = 1;
    pFields[0].IsAscending = sal_True;
    pFields[0].IsCaseSensitive = sal_False;
    pFields[0].FieldType = table::TableSortFieldType_ALPHANUMERIC;
    pFields[0].CollatorLocale = aLang;
    pFields[0].CollatorAlgorithm = aCollAlg;

    pFields[1].Field = 1;
    pFields[1].IsAscending = sal_True;
    pFields[1].IsCaseSensitive = sal_False;
    pFields[1].FieldType = table::TableSortFieldType_ALPHANUMERIC;
    pFields[1].CollatorLocale = aLang;
    pFields[1].CollatorAlgorithm = aCollAlg;

    pFields[2].Field = 1;
    pFields[2].IsAscending = sal_True;
    pFields[2].IsCaseSensitive = sal_False;
    pFields[2].FieldType = table::TableSortFieldType_ALPHANUMERIC;
    pFields[2].CollatorLocale = aLang;
    pFields[2].CollatorAlgorithm = aCollAlg;

    aVal <<= aFields;
    pArray[4] = beans::PropertyValue(C2U("SortFields"), -1, aVal, beans::PropertyState_DIRECT_VALUE);

    return aRet;
}


uno::Sequence< beans::PropertyValue > SwXTextCursor::createSortDescriptor(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    return SwXTextCursor::createSortDescriptor(sal_False);
}

sal_Bool SwXTextCursor::convertSortProperties(
    const uno::Sequence< beans::PropertyValue >& rDescriptor, SwSortOptions& rSortOpt)
{
    sal_Bool bRet = sal_True;
    const beans::PropertyValue* pProperties = rDescriptor.getConstArray();

    rSortOpt.bTable = sal_False;
    rSortOpt.cDeli = ' ';
    rSortOpt.eDirection = SRT_COLUMNS;  //!! UI text may be contrary though !!

    SwSortKey* pKey1 = new SwSortKey;
    pKey1->nColumnId = USHRT_MAX;
    pKey1->bIsNumeric = TRUE;
    pKey1->eSortOrder   = SRT_ASCENDING;

    SwSortKey* pKey2 = new SwSortKey;
    pKey2->nColumnId = USHRT_MAX;
    pKey2->bIsNumeric = TRUE;
    pKey2->eSortOrder   = SRT_ASCENDING;

    SwSortKey* pKey3 = new SwSortKey;
    pKey3->nColumnId = USHRT_MAX;
    pKey3->bIsNumeric = TRUE;
    pKey3->eSortOrder   = SRT_ASCENDING;
    SwSortKey* aKeys[3] = {pKey1, pKey2, pKey3};

    sal_Bool bOldSortdescriptor(sal_False);
    sal_Bool bNewSortdescriptor(sal_False);

    for( int n = 0; n < rDescriptor.getLength(); ++n )
    {
        uno::Any aValue( pProperties[n].Value );
        const OUString& rPropName = pProperties[n].Name;

        // old and new sortdescriptor
        if( COMPARE_EQUAL == rPropName.compareToAscii("IsSortInTable"))
        {
            if ( aValue.getValueType() == ::getBooleanCppuType() )
                rSortOpt.bTable = *(sal_Bool*)aValue.getValue();
            else
                bRet = sal_False;
        }
        else if(COMPARE_EQUAL == rPropName.compareToAscii("Delimiter"))
        {
            sal_Unicode uChar(0);
            if( aValue >>= uChar )
                rSortOpt.cDeli = uChar;
            else
                bRet = sal_False;
        }
        // old sortdescriptor
        else if(COMPARE_EQUAL == rPropName.compareToAscii("SortColumns"))
        {
            bOldSortdescriptor = sal_True;
            if ( aValue.getValueType() == ::getBooleanCppuType() )
            {
                sal_Bool bTemp = *(sal_Bool*)aValue.getValue();
                rSortOpt.eDirection = bTemp ? SRT_COLUMNS : SRT_ROWS;
            }
            else
                bRet = sal_False;
        }
        else if(COMPARE_EQUAL == rPropName.compareToAscii("IsCaseSensitive"))
        {
            bOldSortdescriptor = sal_True;
            if ( aValue.getValueType() == ::getBooleanCppuType() )
            {
                sal_Bool bTemp = *(sal_Bool*)aValue.getValue();
                rSortOpt.bIgnoreCase = !bTemp;
            }
            else
                bRet = sal_False;
        }
        else if(COMPARE_EQUAL == rPropName.compareToAscii("CollatorLocale"))
        {
            bOldSortdescriptor = sal_True;
            Locale aLocale;
            if (aValue >>= aLocale)
                rSortOpt.nLanguage = SvxLocaleToLanguage( aLocale );
            else
                bRet = sal_False;
        }
        else if(COMPARE_EQUAL == rPropName.compareToAscii("CollatorAlgorithm", 17) &&
            rPropName.getLength() == 18 &&
            (rPropName.getStr()[17] >= '0' && rPropName.getStr()[17] <= '9'))
        {
            bOldSortdescriptor = sal_True;
            sal_uInt16 nIndex = rPropName.getStr()[17];
            nIndex -= '0';
            OUString aTxt;
            if ((aValue >>= aTxt) && nIndex < 3)
                aKeys[nIndex]->sSortType = aTxt;
            else
                bRet = sal_False;
        }
        else if(COMPARE_EQUAL == rPropName.compareToAscii("SortRowOrColumnNo", 17) &&
            rPropName.getLength() == 18 &&
            (rPropName.getStr()[17] >= '0' && rPropName.getStr()[17] <= '9'))
        {
            bOldSortdescriptor = sal_True;
            sal_uInt16 nIndex = rPropName.getStr()[17];
            nIndex -= '0';
            sal_Int16 nCol = -1;
            if( aValue.getValueType() == ::getCppuType((const sal_Int16*)0) && nIndex < 3)
                aValue >>= nCol;
            if( nCol >= 0 )
                aKeys[nIndex]->nColumnId = nCol;
            else
                bRet = sal_False;
        }
        else if(0 == rPropName.indexOf(C2U("IsSortNumeric")) &&
            rPropName.getLength() == 14 &&
            (rPropName.getStr()[13] >= '0' && rPropName.getStr()[13] <= '9'))
        {
            bOldSortdescriptor = sal_True;
            sal_uInt16 nIndex = rPropName.getStr()[13];
            nIndex = nIndex - '0';
            if ( aValue.getValueType() == ::getBooleanCppuType() && nIndex < 3 )
            {
                sal_Bool bTemp = *(sal_Bool*)aValue.getValue();
                aKeys[nIndex]->bIsNumeric = bTemp;
            }
            else
                bRet = sal_False;
        }
        else if(0 == rPropName.indexOf(C2U("IsSortAscending")) &&
            rPropName.getLength() == 16 &&
            (rPropName.getStr()[15] >= '0' && rPropName.getStr()[15] <= '9'))
        {
            bOldSortdescriptor = sal_True;
            sal_uInt16 nIndex = rPropName.getStr()[15];
            nIndex -= '0';
            if ( aValue.getValueType() == ::getBooleanCppuType() && nIndex < 3 )
            {
                sal_Bool bTemp = *(sal_Bool*)aValue.getValue();
                aKeys[nIndex]->eSortOrder = bTemp ? SRT_ASCENDING : SRT_DESCENDING;
            }
            else
                bRet = sal_False;
        }
        // new sortdescriptor
        else if(COMPARE_EQUAL == rPropName.compareToAscii("IsSortColumns"))
        {
            bNewSortdescriptor = sal_True;
            if ( aValue.getValueType() == ::getBooleanCppuType() )
            {
                sal_Bool bTemp = *(sal_Bool*)aValue.getValue();
                rSortOpt.eDirection = bTemp ? SRT_COLUMNS : SRT_ROWS;
            }
            else
                bRet = sal_False;
        }
        else if (COMPARE_EQUAL == rPropName.compareToAscii("SortFields"))
        {
            bNewSortdescriptor = sal_True;
            uno::Sequence < table::TableSortField > aFields;
            if ( aValue >>= aFields )
            {
                sal_Int32 nCount(aFields.getLength());
                if (nCount <= 3)
                {
                    table::TableSortField* pFields = aFields.getArray();
                    for (sal_Int32 i = 0; i < nCount; ++i)
                    {
                        rSortOpt.bIgnoreCase = !pFields[i].IsCaseSensitive;
                        rSortOpt.nLanguage = SvxLocaleToLanguage( pFields[i].CollatorLocale );
                        aKeys[i]->sSortType = pFields[i].CollatorAlgorithm;
                        aKeys[i]->nColumnId = pFields[i].Field;
                        aKeys[i]->bIsNumeric = (pFields[i].FieldType == table::TableSortFieldType_NUMERIC);
                        aKeys[i]->eSortOrder = pFields[i].IsAscending ? SRT_ASCENDING : SRT_DESCENDING;
                    }
                }
                else
                    bRet = sal_False;
            }
            else
                bRet = sal_False;
        }
    }

    if (bNewSortdescriptor && bOldSortdescriptor)
    {
        OSL_FAIL("someone tried to set the old deprecated and the new sortdescriptor");
        bRet = sal_False;
    }

    if(pKey1->nColumnId != USHRT_MAX)
        rSortOpt.aKeys.C40_INSERT(SwSortKey, pKey1, rSortOpt.aKeys.Count());
    if(pKey2->nColumnId != USHRT_MAX)
        rSortOpt.aKeys.C40_INSERT(SwSortKey, pKey2, rSortOpt.aKeys.Count());
    if(pKey3->nColumnId != USHRT_MAX)
        rSortOpt.aKeys.C40_INSERT(SwSortKey, pKey3, rSortOpt.aKeys.Count());

    return bRet && rSortOpt.aKeys.Count() > 0;
}

void SwXTextCursor::sort(const uno::Sequence< beans::PropertyValue >& rDescriptor)
        throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwSortOptions aSortOpt;
    SwUnoCrsr* pUnoCrsr = GetCrsr();

    if(pUnoCrsr)
    {
        if(pUnoCrsr->HasMark())
        {
            if(!SwXTextCursor::convertSortProperties(rDescriptor, aSortOpt))
                    throw uno::RuntimeException();
            UnoActionContext aContext( pUnoCrsr->GetDoc() );

            SwPosition* pStart = pUnoCrsr->Start();
            SwPosition* pEnd   = pUnoCrsr->End();

            SwNodeIndex aPrevIdx( pStart->nNode, -1 );
            ULONG nOffset = pEnd->nNode.GetIndex() - pStart->nNode.GetIndex();
            xub_StrLen nCntStt  = pStart->nContent.GetIndex();

            // Selektion wieder setzen
            pUnoCrsr->DeleteMark();
            pUnoCrsr->GetPoint()->nNode.Assign( aPrevIdx.GetNode(), +1 );
            SwCntntNode* pCNd = pUnoCrsr->GetCntntNode();
            xub_StrLen nLen = pCNd->Len();
            if( nLen > nCntStt )
                nLen = nCntStt;
            pUnoCrsr->GetPoint()->nContent.Assign(pCNd, nLen );
            pUnoCrsr->SetMark();

            pUnoCrsr->GetPoint()->nNode += nOffset;
            pCNd = pUnoCrsr->GetCntntNode();
            pUnoCrsr->GetPoint()->nContent.Assign( pCNd, pCNd->Len() );
        }
    }
    else
        throw uno::RuntimeException();
}

void ClientModify(SwClient* pClient, SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    switch( pOld ? pOld->Which() : 0 )
    {
    case RES_REMOVE_UNO_OBJECT:
    case RES_OBJECTDYING:
        if( (void*)pClient->GetRegisteredIn() == ((SwPtrMsgPoolItem *)pOld)->pObject )
            ((SwModify*)pClient->GetRegisteredIn())->Remove(pClient);
        break;

    case RES_FMT_CHG:
        // wurden wir an das neue umgehaengt und wird das alte geloscht?
        if( ((SwFmtChg*)pNew)->pChangedFmt == pClient->GetRegisteredIn() &&
            ((SwFmtChg*)pOld)->pChangedFmt->IsFmtInDTOR() )
            ((SwModify*)pClient->GetRegisteredIn())->Remove(pClient);
        break;
    }
}


Reference< XEnumeration >  SAL_CALL SwXTextCursor::createContentEnumeration(const OUString& rServiceName) throw( RuntimeException )
{
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if( !pUnoCrsr || 0 != rServiceName.compareToAscii("com.sun.star.text.TextContent") )
        throw RuntimeException();

    Reference< XEnumeration > xRet = new SwXParaFrameEnumeration(*pUnoCrsr, PARAFRAME_PORTION_TEXTRANGE);
    return xRet;
}

Reference< XEnumeration >  SwXTextCursor::createEnumeration(void) throw( RuntimeException )
{
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if( !pUnoCrsr  )
        throw RuntimeException();
    Reference<XUnoTunnel> xTunnel(xParentText, UNO_QUERY);
    SwXText* pParentText = 0;
    if(xTunnel.is())
    {
        pParentText = (SwXText*)xTunnel->getSomething(SwXText::getUnoTunnelId());
    }
    DBG_ASSERT(pParentText, "parent is not a SwXText");

    SwUnoCrsr* pNewCrsr = pUnoCrsr->GetDoc()->CreateUnoCrsr(*pUnoCrsr->GetPoint());
    if(pUnoCrsr->HasMark())
    {
        pNewCrsr->SetMark();
        *pNewCrsr->GetMark() = *pUnoCrsr->GetMark();
    }
    CursorType eSetType = eType == CURSOR_TBLTEXT ? CURSOR_SELECTION_IN_TABLE : CURSOR_SELECTION;
    Reference< XEnumeration > xRet = new SwXParagraphEnumeration(pParentText, *pNewCrsr, eSetType);

    return xRet;
}

uno::Type  SwXTextCursor::getElementType(void) throw( RuntimeException )
{
    return ::getCppuType((uno::Reference<XTextRange>*)0);
}

sal_Bool SwXTextCursor::hasElements(void) throw( RuntimeException )
{
    return sal_True;
}

Sequence< OUString > SAL_CALL SwXTextCursor::getAvailableServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.text.TextContent" ));
    return aRet;
}


IMPL_STATIC_LINK( SwXTextCursor, RemoveCursor_Impl,
                  Reference<XInterface>*, pArg )
{
    OSL_ENSURE( pThis != NULL, "no reference?" );
    OSL_ENSURE( pArg != NULL, "no reference?" );

    SwUnoCrsr* pCursor = pThis->GetCrsr();
    if( pCursor != NULL )
    {
        pCursor->Remove( pThis );
        delete pCursor;
    }
    delete pArg;

    return 0;
}

void    SwXTextCursor::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);

    // if the cursor leaves its designated section, it becomes invalid
    if( ( pOld != NULL ) && ( pOld->Which() == RES_UNOCURSOR_LEAVES_SECTION ) )
    {
        // create reference to this object to prevent deletion before
        // the STATIC_LINK is executed. The link will delete the
        // reference.
        Reference<XInterface>* pRef =
            new Reference<XInterface>( static_cast<XServiceInfo*>( this ) );
        Application::PostUserEvent(
            STATIC_LINK( this, SwXTextCursor, RemoveCursor_Impl ), pRef );
    }

    if(!GetRegisteredIn())
        aLstnrCntnr.Disposing();

}

const SwPaM*    SwXTextCursor::GetPaM() const
{
    return GetCrsr() ? GetCrsr() : 0;
}
// -----------------------------------------------------------------------------
SwPaM*  SwXTextCursor::GetPaM()
{
    return GetCrsr() ? GetCrsr() : 0;
}


const SwDoc* SwXTextCursor::GetDoc()const
{
    return   GetCrsr() ? GetCrsr()->GetDoc() : 0;
}

SwDoc* SwXTextCursor::GetDoc()
{
    return   GetCrsr() ? GetCrsr()->GetDoc() : 0;
}


void SwXTextCursor::SetCrsrAttr(SwPaM& rPam, const SfxItemSet& rSet, USHORT nAttrMode)
{
    sal_uInt16 nFlags = SETATTR_APICALL;
    if(nAttrMode & CRSR_ATTR_MODE_DONTREPLACE)
        nFlags |= SETATTR_DONTREPLACE;
    SwDoc* pDoc = rPam.GetDoc();
    //StartEndAction
    UnoActionContext aAction(pDoc);
    SwPaM* pCrsr = &rPam;
    if( pCrsr->GetNext() != pCrsr )     // Ring von Cursorn
    {
        SwPaM *_pStartCrsr = &rPam;
        do
        {
            if( _pStartCrsr->HasMark() && ( (CRSR_ATTR_MODE_TABLE & nAttrMode) ||
                *_pStartCrsr->GetPoint() != *_pStartCrsr->GetMark() ))
                pDoc->Insert(*_pStartCrsr, rSet, nFlags );
        } while( (_pStartCrsr=(SwPaM *)_pStartCrsr->GetNext()) != &rPam );

    }
    else
    {
//          if( !HasSelection() )
//              UpdateAttr();
        pDoc->Insert( *pCrsr, rSet, nFlags );
    }
}

void SwXTextCursor::GetCrsrAttr(SwPaM& rPam, SfxItemSet& rSet, BOOL bCurrentAttrOnly)
{
    static const sal_uInt16 nMaxLookup = 1000;
    SfxItemSet aSet( *rSet.GetPool(), rSet.GetRanges() );
    SfxItemSet *pSet = &rSet;
    SwPaM *_pStartCrsr = &rPam;
    do
    {
        sal_uInt32 nSttNd = _pStartCrsr->GetMark()->nNode.GetIndex(),
                    nEndNd = _pStartCrsr->GetPoint()->nNode.GetIndex();
            xub_StrLen nSttCnt = _pStartCrsr->GetMark()->nContent.GetIndex(),
                    nEndCnt = _pStartCrsr->GetPoint()->nContent.GetIndex();

            if( nSttNd > nEndNd || ( nSttNd == nEndNd && nSttCnt > nEndCnt ))
            {
                sal_uInt32 nTmp = nSttNd; nSttNd = nEndNd; nEndNd = nTmp;
                nTmp = nSttCnt; nSttCnt = nEndCnt; nEndCnt = (sal_uInt16)nTmp;
            }

            if( nEndNd - nSttNd >= nMaxLookup )
            {
                rSet.ClearItem();
                rSet.InvalidateAllItems();
                return;// uno::Any();
            }

            // beim 1.Node traegt der Node die Werte in den GetSet ein (Initial)
            // alle weiteren Nodes werden zum GetSet zu gemergt
            for( ULONG n = nSttNd; n <= nEndNd; ++n )
            {
                SwNode* pNd = rPam.GetDoc()->GetNodes()[ n ];
                switch( pNd->GetNodeType() )
                {
                case ND_TEXTNODE:
                    {
                        xub_StrLen nStt = n == nSttNd ? nSttCnt : 0,
                                nEnd = n == nEndNd ? nEndCnt
                                            : ((SwTxtNode*)pNd)->GetTxt().Len();
                        ((SwTxtNode*)pNd)->GetAttr( *pSet, nStt, nEnd, bCurrentAttrOnly, !bCurrentAttrOnly );
                    }
                    break;
                case ND_GRFNODE:
                case ND_OLENODE:
                    ((SwCntntNode*)pNd)->GetAttr( *pSet );
                    break;

                default:
                    pNd = 0;
                }

                if( pNd )
                {
                    if( pSet != &rSet )
                        rSet.MergeValues( aSet );

                    if( aSet.Count() )
                        aSet.ClearItem();

                }
                pSet = &aSet;
            }
    } while( (_pStartCrsr=(SwPaM *)_pStartCrsr->GetNext()) != &rPam );
}
/******************************************************************
 * SwXParagraphEnumeration
 ******************************************************************/

OUString SwXParagraphEnumeration::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXParagraphEnumeration");
}

BOOL SwXParagraphEnumeration::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.ParagraphEnumeration") == rServiceName;
}

Sequence< OUString > SwXParagraphEnumeration::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.ParagraphEnumeration");
    return aRet;
}

SwXParagraphEnumeration::SwXParagraphEnumeration(SwXText* pParent,
                                                    SwPosition& rPos,
                                                    CursorType eType) :
        xParentText(pParent),
        bFirstParagraph(sal_True),
        eCursorType(eType),
        nEndIndex(rPos.nNode.GetIndex()),
        nFirstParaStart(-1),
        nLastParaEnd(-1)
{
    SwUnoCrsr* pUnoCrsr = pParent->GetDoc()->CreateUnoCrsr(rPos, sal_False);
    pUnoCrsr->Add(this);
}


SwXParagraphEnumeration::SwXParagraphEnumeration(SwXText* pParent,
                                                SwUnoCrsr*  pCrsr,
                                                CursorType eType) :
        SwClient(pCrsr),
        xParentText(pParent),
        bFirstParagraph(sal_True),
        eCursorType(eType),
        nEndIndex(pCrsr->End()->nNode.GetIndex()),
        nFirstParaStart(-1),
        nLastParaEnd(-1)
{
    if(CURSOR_SELECTION == eCursorType || CURSOR_SELECTION_IN_TABLE == eCursorType)
    {
        if(*pCrsr->GetPoint() > *pCrsr->GetMark())
            pCrsr->Exchange();
        nFirstParaStart = pCrsr->GetPoint()->nContent.GetIndex();
        nLastParaEnd = pCrsr->GetMark()->nContent.GetIndex();
        if(pCrsr->HasMark())
            pCrsr->DeleteMark();
    }
}

SwXParagraphEnumeration::~SwXParagraphEnumeration()
{
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
        delete pUnoCrsr;

}

sal_Bool SwXParagraphEnumeration::hasMoreElements(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool bRet = sal_False;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        if(bFirstParagraph)
            bRet = sal_True;
        else
        {
            SwPosition* pStart = pUnoCrsr->Start();
            SwUnoCrsr* pNewCrsr = pUnoCrsr->GetDoc()->CreateUnoCrsr(*pStart, sal_False);
            //man soll hier auch in Tabellen landen duerfen
            if(CURSOR_TBLTEXT != eCursorType && CURSOR_SELECTION_IN_TABLE != eCursorType)
                pNewCrsr->SetRemainInSection( sal_False );

            //was mache ich, wenn ich schon in einer Tabelle stehe?
            SwTableNode* pTblNode = pNewCrsr->GetNode()->FindTableNode();
            if((CURSOR_TBLTEXT != eCursorType && CURSOR_SELECTION_IN_TABLE != eCursorType) && pTblNode)
            {
                pNewCrsr->GetPoint()->nNode = pTblNode->EndOfSectionIndex();
                bRet = pNewCrsr->Move(fnMoveForward, fnGoNode);
            }
            else
                bRet = pNewCrsr->MovePara(fnParaNext, fnParaStart);
            if((CURSOR_SELECTION == eCursorType || CURSOR_SELECTION_IN_TABLE == eCursorType)
                        && nEndIndex < pNewCrsr->Start()->nNode.GetIndex())
                bRet = FALSE;
            delete pNewCrsr;
        }
    }
    return bRet;
}

uno::Any SwXParagraphEnumeration::nextElement(void)
    throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< XTextContent >  aRef;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
         XText* pText = xParentText.get();
        sal_Bool bInTable = sal_False;
        if(!bFirstParagraph)
        {
            //man soll hier auch in Tabellen landen duerfen
            if(CURSOR_TBLTEXT != eCursorType && CURSOR_SELECTION_IN_TABLE != eCursorType)
            {
                pUnoCrsr->SetRemainInSection( sal_False );
                //was mache ich, wenn ich schon in einer Tabelle stehe?
                SwTableNode* pTblNode = pUnoCrsr->GetNode()->FindTableNode();
                if(pTblNode)
                {
                    // wir haben es mit einer Tabelle zu tun - also ans Ende
                    pUnoCrsr->GetPoint()->nNode = pTblNode->EndOfSectionIndex();
                    if(!pUnoCrsr->Move(fnMoveForward, fnGoNode))
                    {
                        throw container::NoSuchElementException();
                    }
                    else
                        bInTable = sal_True;

                }
            }
        }

        if( bFirstParagraph || bInTable || pUnoCrsr->MovePara(fnParaNext, fnParaStart))
        {
            SwPosition* pStart = pUnoCrsr->Start();
            sal_Int32 nFirstContent = bFirstParagraph ? nFirstParaStart : -1;
            sal_Int32 nLastContent = nEndIndex ==  pStart->nNode.GetIndex() ? nLastParaEnd : -1;
            bFirstParagraph = sal_False;
            //steht man nun in einer Tabelle, oder in einem einfachen Absatz?

            SwTableNode* pTblNode = pUnoCrsr->GetNode()->FindTableNode();
            if(CURSOR_TBLTEXT != eCursorType && CURSOR_SELECTION_IN_TABLE != eCursorType && pTblNode)
            {
                // wir haben es mit einer Tabelle zu tun
                SwFrmFmt* pTableFmt = (SwFrmFmt*)pTblNode->GetTable().GetFrmFmt();
                XTextTable* pTable = SwXTextTables::GetObject( *pTableFmt );
                aRef =  (XTextContent*)(SwXTextTable*)pTable;
            }
            else
            {
                SwUnoCrsr* pNewCrsr = pUnoCrsr->GetDoc()->CreateUnoCrsr(*pStart, sal_False);
                aRef =  (XTextContent*)new SwXParagraph((SwXText*)pText, pNewCrsr, nFirstContent, nLastContent);
            }
        }
        else
            throw container::NoSuchElementException();

    }
    else
        throw uno::RuntimeException();

    uno::Any aRet(&aRef, ::getCppuType((uno::Reference<XTextContent>*)0));
    return aRet;
}
void SwXParagraphEnumeration::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
}

/******************************************************************
 * SwXTextRange
 ******************************************************************/
TYPEINIT1(SwXTextRange, SwClient);


const uno::Sequence< sal_Int8 > & SwXTextRange::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::binfilter::CreateUnoTunnelId();
    return aSeq;
}

    //XUnoTunnel
sal_Int64 SAL_CALL SwXTextRange::getSomething(
    const uno::Sequence< sal_Int8 >& rId )
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

OUString SwXTextRange::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXTextRange");
}

BOOL SwXTextRange::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    String sServiceName(rServiceName);
    return sServiceName.EqualsAscii("com.sun.star.text.TextRange") ||
         sServiceName.EqualsAscii("com.sun.star.style.CharacterProperties")||
        sServiceName.EqualsAscii("com.sun.star.style.CharacterPropertiesAsian")||
        sServiceName.EqualsAscii("com.sun.star.style.CharacterPropertiesComplex")||
        sServiceName.EqualsAscii("com.sun.star.style.ParagraphProperties") ||
        sServiceName.EqualsAscii("com.sun.star.style.ParagraphPropertiesAsian") ||
        sServiceName.EqualsAscii("com.sun.star.style.ParagraphPropertiesComplex");
}

Sequence< OUString > SwXTextRange::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(7);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TextRange");
     pArray[1] = C2U("com.sun.star.style.CharacterProperties");
    pArray[2] = C2U("com.sun.star.style.CharacterPropertiesAsian");
    pArray[3] = C2U("com.sun.star.style.CharacterPropertiesComplex");
    pArray[4] = C2U("com.sun.star.style.ParagraphProperties");
    pArray[5] = C2U("com.sun.star.style.ParagraphPropertiesAsian");
    pArray[6] = C2U("com.sun.star.style.ParagraphPropertiesComplex");
    return aRet;
}


SwXTextRange::SwXTextRange(SwPaM& rPam, const uno::Reference< XText > & rxParent) :
    eRangePosition(RANGE_IN_TEXT),
    pDoc(rPam.GetDoc()),
    pBox(0),
    pBoxStartNode(0),
    aObjectDepend(this, 0),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_CURSOR)),
    xParentText(rxParent)
{
    //Bookmark an der anlegen
    _CreateNewBookmark(rPam);
}

SwXTextRange::SwXTextRange(SwFrmFmt& rFmt, SwPaM& rPam) :
    eRangePosition(RANGE_IN_FRAME),
    pDoc(rPam.GetDoc()),
    pBox(0),
    pBoxStartNode(0),
    aObjectDepend(this, &rFmt),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_CURSOR))
{
    //Bookmark an der anlegen
    _CreateNewBookmark(rPam);
}

SwXTextRange::SwXTextRange(SwFrmFmt& rTblFmt, SwTableBox& rTblBox, SwPaM& rPam) :
    eRangePosition(RANGE_IN_CELL),
    pDoc(rPam.GetDoc()),
    pBox(&rTblBox),
    pBoxStartNode(0),
    aObjectDepend(this, &rTblFmt),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_CURSOR))
{
    //Bookmark an der anlegen
    _CreateNewBookmark(rPam);
}

SwXTextRange::SwXTextRange(SwFrmFmt& rTblFmt, const SwStartNode& rStartNode, SwPaM& rPam) :
    eRangePosition(RANGE_IN_CELL),
    pDoc(rPam.GetDoc()),
    pBox(0),
    pBoxStartNode(&rStartNode),
    aObjectDepend(this, &rTblFmt),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_CURSOR))
{
    //Bookmark an der anlegen
    _CreateNewBookmark(rPam);
}

SwXTextRange::SwXTextRange(SwFrmFmt& rTblFmt) :
    eRangePosition(RANGE_IS_TABLE),
    pDoc(rTblFmt.GetDoc()),
    pBox(0),
    pBoxStartNode(0),
    aObjectDepend(this, &rTblFmt),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_CURSOR))
{
}


SwXTextRange::~SwXTextRange()
{
    if(GetBookmark())
        pDoc->DelBookmark( GetBookmark()->GetName() );
}

void    SwXTextRange::_CreateNewBookmark(SwPaM& rPam)
{
    static sal_Int32 nBookmark = 0;
    String sBookmarkName;

    SwBookmark* pBkm = GetBookmark();
    if(pBkm)
    {
        // If a bookmark exists already its name can be resused
        sBookmarkName = pBkm->GetName();
        pDoc->DelBookmark( sBookmarkName );
    }
    else
    {
        // Otherwise we have to create a new name. This is not done
        // using SwDoc::MakeUniqueBookmarkName, because this method
        // starts counting bookmarks beginning with 1. That's required
        // for real bookmarks, but very slow in thsi case there lots
        // of bookmarks requiere any unique name only.
        String sPrefix(C2S("SwXTextPosition"));
        const SwBookmarks& rBookmarks = pDoc->GetBookmarks();
        sal_uInt16 nBookmarks = rBookmarks.Count(), i;
        do
        {
            nBookmark++;
            if( nBookmark < 1 ) // on overwflow restart with 1
                nBookmark = 1;

            sBookmarkName = sPrefix;
            sBookmarkName += String::CreateFromInt32( nBookmark );
            for( i = 0; i < nBookmarks; i++ )
                if( rBookmarks[i]->GetName().Equals( sBookmarkName ) )
                    break;
        }
        while( i < nBookmarks );
    }

    KeyCode aCode;
    String sShortName;
    SwBookmark* pMark = pDoc->MakeBookmark(rPam, aCode,
                sBookmarkName, sShortName, UNO_BOOKMARK);
    pMark->Add(this);
}

void    SwXTextRange::DeleteAndInsert(const String& rText) throw( uno::RuntimeException )
{
    SwBookmark* pBkm = GetBookmark();
    if(pBkm )
    {
        const SwPosition& rPoint = pBkm->GetPos();
        const SwPosition* pMark = pBkm->GetOtherPos();
        SwCursor aNewCrsr( rPoint);
        SwDoc* pLclDoc = aNewCrsr.GetDoc();
        if(pMark)
        {
            aNewCrsr.SetMark();
            *aNewCrsr.GetMark() = *pMark;
        }

        UnoActionContext aAction(aNewCrsr.GetDoc());
        if(aNewCrsr.HasMark())
            pLclDoc->DeleteAndJoin(aNewCrsr);

        if(rText.Len())
        {
            if( !pLclDoc->Insert(aNewCrsr, rText) )
            {
                OSL_FAIL( "Doc->Insert(Str) failed." );
            }
            SwXTextCursor::SelectPam(aNewCrsr, sal_True);
            aNewCrsr.Left(rText.Len(), CRSR_SKIP_CHARS);
        }
        _CreateNewBookmark(aNewCrsr);
    }

}


uno::Reference< XText >  SwXTextRange::getText(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!xParentText.is())
    {
        if(eRangePosition == RANGE_IN_FRAME &&
            aObjectDepend.GetRegisteredIn())
        {
            SwFrmFmt* pFrmFmt = (SwFrmFmt*)aObjectDepend.GetRegisteredIn();
            SwXTextFrame* pxFrm = (SwXTextFrame*)SwClientIter( *pFrmFmt ).
                                                First( TYPE( SwXTextFrame ));
            if(pxFrm)
                ((SwXTextRange*)this)->xParentText = pxFrm;
            else
                ((SwXTextRange*)this)->xParentText = new SwXTextFrame(*pFrmFmt);
            ((SwModify*)aObjectDepend.GetRegisteredIn())->
                    Remove(&((SwXTextRange*)this)->aObjectDepend);

        }
        else if(eRangePosition == RANGE_IN_CELL &&
            aObjectDepend.GetRegisteredIn())
        {
            const SwStartNode* pSttNd = pBoxStartNode ? pBoxStartNode : pBox->GetSttNd();
            const SwTableNode* pTblNode = pSttNd->FindTableNode();
            const SwFrmFmt* pTableFmt = pTblNode->GetTable().GetFrmFmt();
            ((SwXTextRange*)this)->xParentText =
                    pBox ? SwXCell::CreateXCell((SwFrmFmt*)pTableFmt, pBox)
                         : new SwXCell( (SwFrmFmt*)pTableFmt, *pBoxStartNode );
            ((SwModify*)aObjectDepend.GetRegisteredIn())->
                    Remove(&((SwXTextRange*)this)->aObjectDepend);
        }
        else if(eRangePosition == RANGE_IS_TABLE &&
            aObjectDepend.GetRegisteredIn() )
        {
            SwFrmFmt* pTblFmt = (SwFrmFmt*)aObjectDepend.GetRegisteredIn();
            SwDoc* pLclDoc = pTblFmt->GetDoc();
            SwTable* pTable = SwTable::FindTable( pTblFmt );
            SwTableNode* pTblNode = pTable->GetTableNode();
            SwPosition aPosition( *pTblNode );
            Reference< XTextRange >  xRange = SwXTextRange::CreateTextRangeFromPosition(pLclDoc,
                        aPosition, 0);
            xParentText = xRange->getText();
        }
        else
        {
            // jetzt noch alle Faelle finden, die nicht abgedeckt sind
            // (Body, Kopf-/Fusszeilen, Fussnotentext )
            DBG_WARNING("not implemented");
        }
    }
    return xParentText;
}

uno::Reference< XTextRange >  SwXTextRange::getStart(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< XTextRange >  xRet;
    SwBookmark* pBkm = GetBookmark();
    if(!xParentText.is())
        getText();
    if(pBkm)
    {
        SwPaM aPam(pBkm->GetPos());
        xRet = new SwXTextRange(aPam, xParentText);
    }
    else if(eRangePosition == RANGE_IS_TABLE)
    {
        //start und ende sind mit this identisch, wenn es eine Tabelle ist
        xRet = this;
    }
    else
        throw uno::RuntimeException();
    return xRet;
}

uno::Reference< XTextRange >  SwXTextRange::getEnd(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< XTextRange >  xRet;
    SwBookmark* pBkm = GetBookmark();
    if(!xParentText.is())
        getText();
    if(pBkm)
    {
        SwPaM aPam(pBkm->GetOtherPos()? *pBkm->GetOtherPos() : pBkm->GetPos());
        xRet = new SwXTextRange(aPam, xParentText);
    }
    else if(eRangePosition == RANGE_IS_TABLE)
    {
        //start und ende sind mit this identisch, wenn es eine Tabelle ist
        xRet = this;
    }
    else
        throw uno::RuntimeException();
    return xRet;
}

OUString SwXTextRange::getString(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwBookmark* pBkm = GetBookmark();
    OUString sRet;
    //fuer Tabellen gibt es keine Bookmark, also auch keinen Text
    //evtl. koennte man hier die Tabelle als ASCII exportieren?
    if(pBkm && pBkm->GetOtherPos())
    {
        const SwPosition& rPoint = pBkm->GetPos();
        const SwPosition* pMark = pBkm->GetOtherPos();
        SwPaM aCrsr(*pMark, rPoint);
/*      if( rPoint.nNode.GetIndex() ==
            pMark->nNode.GetIndex() )
        {
            SwTxtNode* pTxtNd = aCrsr.GetNode()->GetTxtNode();
            if( pTxtNd )
            {
                sal_uInt16 nStt = aCrsr.Start()->nContent.GetIndex();
                sRet = pTxtNd->GetExpandTxt( nStt,
                        aCrsr.End()->nContent.GetIndex() - nStt );
            }
        }
        else
*/      {
            SwXTextCursor::getTextFromPam(aCrsr, sRet);
        }
    }
    return sRet;
}

void SwXTextRange::setString(const OUString& aString)
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(RANGE_IS_TABLE == eRangePosition)
    {
        //setString in Tabellen kann nicht erlaubt werden
        throw uno::RuntimeException();
    }
    else
        DeleteAndInsert(aString);
}

void SwXTextRange::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    sal_Bool bAlreadyRegisterred = 0 != GetRegisteredIn();
    ClientModify(this, pOld, pNew);
    if(aObjectDepend.GetRegisteredIn())
    {
        ClientModify(&aObjectDepend, pOld, pNew);
        // if the depend was removed then the range must be removed too
        if(!aObjectDepend.GetRegisteredIn() && GetRegisteredIn())
            ((SwModify*)GetRegisteredIn())->Remove(this);
        // or if the range has been removed but the depend ist still
        // connected then the depend must be removed
        else if(bAlreadyRegisterred && !GetRegisteredIn() &&
                aObjectDepend.GetRegisteredIn())
            ((SwModify*)aObjectDepend.GetRegisteredIn())->Remove(&aObjectDepend);
    }
}

sal_Bool    SwXTextRange::GetPositions(SwPaM& rToFill) const
{
    sal_Bool bRet = sal_False;
    SwBookmark* pBkm = GetBookmark();
    if(pBkm)
    {
        *rToFill.GetPoint() = pBkm->GetPos();
        if(pBkm->GetOtherPos())
        {
            rToFill.SetMark();
            *rToFill.GetMark() = *pBkm->GetOtherPos();
        }
        else
            rToFill.DeleteMark();
        bRet = sal_True;
    }
    return bRet;
}

sal_Bool        SwXTextRange::XTextRangeToSwPaM( SwUnoInternalPaM& rToFill,
                            const uno::Reference< XTextRange > & xTextRange)
{
    sal_Bool bRet = sal_False;

    uno::Reference<lang::XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
    SwXTextRange* pRange = 0;
    OTextCursorHelper* pCursor = 0;
    SwXTextPortion* pPortion = 0;
    SwXText* pText = 0;
    if(xRangeTunnel.is())
    {
        pRange = (SwXTextRange*)xRangeTunnel->getSomething(
                                SwXTextRange::getUnoTunnelId());
        pCursor = (OTextCursorHelper*)xRangeTunnel->getSomething(
                                OTextCursorHelper::getUnoTunnelId());
        pPortion = (SwXTextPortion*)xRangeTunnel->getSomething(
                                SwXTextPortion::getUnoTunnelId());
        pText = (SwXText*)xRangeTunnel->getSomething(
                                SwXText::getUnoTunnelId());
    }

    //if it's a text cursor then create a temporary cursor there and re-use the pCursor variable
    Reference< XTextCursor > xTextCursor;
    if(pText)
    {
        xTextCursor = pText->createCursor();
        xTextCursor->gotoEnd(sal_True);
        Reference<XUnoTunnel> xCrsrTunnel( xTextCursor, UNO_QUERY);
        pCursor = (OTextCursorHelper*)xCrsrTunnel->getSomething(
                                                    OTextCursorHelper::getUnoTunnelId());
    }
    if(pRange && pRange->GetDoc() == rToFill.GetDoc())
    {
        bRet = pRange->GetPositions(rToFill);
    }
    else
    {
        SwDoc*      pDoc = pCursor ? pCursor->GetDoc() : NULL;
        if ( !pDoc )
            pDoc = pPortion ? pPortion->GetCrsr()->GetDoc() : NULL;
        const SwPaM* pUnoCrsr = pCursor ? pCursor->GetPaM() : pPortion ? pPortion->GetCrsr() : 0;
        if(pUnoCrsr && pDoc == rToFill.GetDoc())
            {
                DBG_ASSERT((SwPaM*)pUnoCrsr->GetNext() == pUnoCrsr, "was machen wir mit Ringen?" );
                bRet = sal_True;
                *rToFill.GetPoint() = *pUnoCrsr->GetPoint();
                if(pUnoCrsr->HasMark())
                {
                    rToFill.SetMark();
                    *rToFill.GetMark() = *pUnoCrsr->GetMark();
                }
                else
                    rToFill.DeleteMark();
            }
    }
    return bRet;
}
/* --------------------------------------------------
 * Der StartNode muss in einem existierenden Header/Footen liegen
 * --------------------------------------------------*/
sal_Bool lcl_IsStartNodeInFormat(sal_Bool bHeader, SwStartNode* pSttNode,
    const SwFrmFmt* pFrmFmt, SwFrmFmt*& rpFormat)
{
    sal_Bool bRet = sal_False;
    const SfxItemSet& rSet = pFrmFmt->GetAttrSet();
    const SfxPoolItem* pItem;
    SwFrmFmt* pHeadFootFmt;
    if(SFX_ITEM_SET == rSet.GetItemState(bHeader ? RES_HEADER : RES_FOOTER, sal_True, &pItem) &&
            0 != (pHeadFootFmt = bHeader ?
                    ((SwFmtHeader*)pItem)->GetHeaderFmt() :
                                ((SwFmtFooter*)pItem)->GetFooterFmt()))
    {
        const SwFmtCntnt& rFlyCntnt = pHeadFootFmt->GetCntnt();
        const SwNode& rNode = rFlyCntnt.GetCntntIdx()->GetNode();
        const SwStartNode* pCurSttNode = rNode.FindSttNodeByType(
            bHeader ? SwHeaderStartNode : SwFooterStartNode);
        if(pCurSttNode && pCurSttNode == pSttNode)
        {
            bRet = sal_True;
            rpFormat = pHeadFootFmt;
        }
    }
    return bRet;
}

uno::Reference< XTextRange >  SwXTextRange::CreateTextRangeFromPosition(SwDoc* pDoc,
                        const SwPosition& rPos, const SwPosition* pMark)
{
    uno::Reference< XTextRange >  aRet;
    SwUnoCrsr* pNewCrsr = pDoc->CreateUnoCrsr(rPos, sal_False);
    if(pMark)
    {
        pNewCrsr->SetMark();
        *pNewCrsr->GetMark() = *pMark;
    }
    uno::Reference< XText >  xParentText;
    //jetzt besorgen wir uns mal den Parent:
    SwStartNode* pSttNode = rPos.nNode.GetNode().FindStartNode();
    while(pSttNode && pSttNode->IsSectionNode())
    {
        pSttNode = pSttNode->FindStartNode();
    }
    SwStartNodeType eType = pSttNode->GetStartNodeType();
    switch(eType)
    {
        case SwTableBoxStartNode:
        {
            const SwTableNode* pTblNode = pSttNode->FindTableNode();
            SwFrmFmt* pTableFmt = (SwFrmFmt*)pTblNode->GetTable().GetFrmFmt();
            SwTableBox* pBox = pSttNode->GetTblBox();

            if( pBox )
                aRet = new SwXTextRange(*pTableFmt, *pBox, *pNewCrsr);
            else
                aRet = new SwXTextRange(*pTableFmt, *pSttNode, *pNewCrsr);
        }
        break;
        case SwFlyStartNode:
        {
            SwFrmFmt* pFmt;
            if(0 != (pFmt = pSttNode->GetFlyFmt()))
            {
                aRet = new SwXTextRange(*pFmt, *pNewCrsr);

            }
        }
        break;
        case SwHeaderStartNode:
        case SwFooterStartNode:
        {
            sal_Bool bHeader = SwHeaderStartNode == eType;
            sal_uInt16 nPDescCount = pDoc->GetPageDescCnt();
            for(sal_uInt16 i = 0; i < nPDescCount; i++)
            {
                const SwPageDesc& rDesc = pDoc->GetPageDesc( i );
                const SwFrmFmt* pFrmFmtMaster = &rDesc.GetMaster();
                const SwFrmFmt* pFrmFmtLeft = &rDesc.GetLeft();

                SwFrmFmt* pHeadFootFmt = 0;
                if(!lcl_IsStartNodeInFormat(bHeader, pSttNode, pFrmFmtMaster, pHeadFootFmt))
                    lcl_IsStartNodeInFormat(bHeader, pSttNode, pFrmFmtLeft, pHeadFootFmt);

                if(pHeadFootFmt)
                {
                    SwXHeadFootText* pxHdFt = (SwXHeadFootText*)SwClientIter( *pHeadFootFmt ).
                                    First( TYPE( SwXHeadFootText ));
                    xParentText = pxHdFt;
                    if(!pxHdFt)
                        xParentText = new SwXHeadFootText(*pHeadFootFmt, bHeader);
                    break;
                }
            }
        }
        break;
        case SwFootnoteStartNode:
        {
            sal_uInt16 n, nFtnCnt = pDoc->GetFtnIdxs().Count();
            uno::Reference< XFootnote >  xRef;
            for( n = 0; n < nFtnCnt; ++n )
            {
                const SwTxtFtn* pTxtFtn = pDoc->GetFtnIdxs()[ n ];
                const SwFmtFtn& rFtn = pTxtFtn->GetFtn();
                pTxtFtn = rFtn.GetTxtFtn();

                if(pSttNode == pTxtFtn->GetStartNode()->GetNode().
                                    FindSttNodeByType(SwFootnoteStartNode))
                {
                    xParentText = ((SwUnoCallBack*)pDoc->GetUnoCallBack())->
                                                            GetFootnote(rFtn);
                    if(!xParentText.is())
                        xParentText = new SwXFootnote(pDoc, rFtn);
                    break;
                }
            }
        }
        break;
        default:
        {
            // dann ist es der Body-Text
            uno::Reference<frame::XModel> xModel = pDoc->GetDocShell()->GetBaseModel();
            uno::Reference< XTextDocument > xDoc(
                xModel, uno::UNO_QUERY);
            xParentText = xDoc->getText();
        }
    }
    if(!aRet.is())
        aRet = new SwXTextRange(*pNewCrsr, xParentText);
    delete pNewCrsr;
    return aRet;
}


Reference< XEnumeration >  SAL_CALL SwXTextRange::createContentEnumeration(
        const OUString& rServiceName)
                throw( RuntimeException )
{
    SwBookmark* pBkm = GetBookmark();
    if( !pBkm || COMPARE_EQUAL != rServiceName.compareToAscii("com.sun.star.text.TextContent") )
        throw RuntimeException();

    const SwPosition& rPoint = pBkm->GetPos();
    const SwPosition* pMark = pBkm->GetOtherPos();
    SwUnoCrsr* pNewCrsr = pDoc->CreateUnoCrsr(rPoint, FALSE);
    if(pMark && *pMark != rPoint)
    {
        pNewCrsr->SetMark();
        *pNewCrsr->GetMark() = *pMark;
    }
    Reference< XEnumeration > xRet = new SwXParaFrameEnumeration(*pNewCrsr, PARAFRAME_PORTION_TEXTRANGE);
    delete pNewCrsr;
    return xRet;
}

Reference< XEnumeration >  SwXTextRange::createEnumeration(void) throw( RuntimeException )
{
    SwBookmark* pBkm = GetBookmark();
    if( !pBkm  )
        throw RuntimeException();
    const SwPosition& rPoint = pBkm->GetPos();
    const SwPosition* pMark = pBkm->GetOtherPos();
    SwUnoCrsr* pNewCrsr = pDoc->CreateUnoCrsr(rPoint, FALSE);
    if(pMark && *pMark != rPoint)
    {
        pNewCrsr->SetMark();
        *pNewCrsr->GetMark() = *pMark;
    }
    Reference<XUnoTunnel> xTunnel(xParentText, UNO_QUERY);
    SwXText* pParentText = 0;
    if(xTunnel.is())
    {
        pParentText = (SwXText*)xTunnel->getSomething(SwXText::getUnoTunnelId());
    }
    DBG_ASSERT(pParentText, "parent is not a SwXText");
    CursorType eSetType = RANGE_IN_CELL == eRangePosition ? CURSOR_SELECTION_IN_TABLE : CURSOR_SELECTION;
    Reference< XEnumeration > xRet = new SwXParagraphEnumeration(pParentText, *pNewCrsr, eSetType);
    return xRet;
}

uno::Type  SwXTextRange::getElementType(void) throw( RuntimeException )
{
    return ::getCppuType((uno::Reference<XTextRange>*)0);
}

sal_Bool SwXTextRange::hasElements(void) throw( RuntimeException )
{
    return sal_True;
}

Sequence< OUString > SAL_CALL SwXTextRange::getAvailableServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.text.TextContent" ));
    return aRet;
}

Reference< XPropertySetInfo > SAL_CALL SwXTextRange::getPropertySetInfo(  ) throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    static Reference< XPropertySetInfo > xRef =
        aPropSet.getPropertySetInfo();
    return xRef;
}

void SAL_CALL SwXTextRange::setPropertyValue(
    const OUString& rPropertyName, const Any& rValue )
    throw(UnknownPropertyException, PropertyVetoException,
        IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    if(!GetDoc() || !GetBookmark())
        throw RuntimeException();
    SwPaM aPaM(GetDoc()->GetNodes());
    SwXTextRange::GetPositions(aPaM);
    SwXTextCursor::SetPropertyValue(aPaM, aPropSet, rPropertyName, rValue);
}

Any SAL_CALL SwXTextRange::getPropertyValue( const OUString& rPropertyName )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    if(!GetDoc() || !GetBookmark())
        throw RuntimeException();
    SwPaM aPaM(((SwDoc*)GetDoc())->GetNodes());
    SwXTextRange::GetPositions(aPaM);
    return SwXTextCursor::GetPropertyValue(aPaM, aPropSet, rPropertyName);
}

void SAL_CALL SwXTextRange::addPropertyChangeListener(
    const OUString& /*aPropertyName*/, const Reference< XPropertyChangeListener >& /*xListener*/ )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    DBG_WARNING("not implemented");
}

void SAL_CALL SwXTextRange::removePropertyChangeListener(
    const OUString& /*aPropertyName*/, const Reference< XPropertyChangeListener >& /*aListener*/ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    DBG_WARNING("not implemented");
}

void SAL_CALL SwXTextRange::addVetoableChangeListener(
    const OUString& /*PropertyName*/, const Reference< XVetoableChangeListener >& /*aListener*/ )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    DBG_WARNING("not implemented");
}

void SAL_CALL SwXTextRange::removeVetoableChangeListener(
    const OUString& /*PropertyName*/, const Reference< XVetoableChangeListener >& /*aListener*/ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    DBG_WARNING("not implemented");
}

PropertyState SAL_CALL SwXTextRange::getPropertyState( const OUString& rPropertyName )
    throw(UnknownPropertyException, RuntimeException)
{
    SolarMutexGuard aGuard;
    if(!GetDoc() || !GetBookmark())
        throw RuntimeException();
    SwPaM aPaM(((SwDoc*)GetDoc())->GetNodes());
    SwXTextRange::GetPositions(aPaM);
    return SwXTextCursor::GetPropertyState(aPaM, aPropSet, rPropertyName);
}

Sequence< PropertyState > SAL_CALL SwXTextRange::getPropertyStates(
    const Sequence< OUString >& rPropertyName ) throw(UnknownPropertyException, RuntimeException)
{
    SolarMutexGuard aGuard;
    if(!GetDoc() || !GetBookmark())
        throw RuntimeException();
    SwPaM aPaM(((SwDoc*)GetDoc())->GetNodes());
    SwXTextRange::GetPositions(aPaM);
    return SwXTextCursor::GetPropertyStates(aPaM, aPropSet, rPropertyName);
}

void SAL_CALL SwXTextRange::setPropertyToDefault( const OUString& rPropertyName )
    throw(UnknownPropertyException, RuntimeException)
{
    SolarMutexGuard aGuard;
    if(!GetDoc() || !GetBookmark())
        throw RuntimeException();
    SwPaM aPaM(((SwDoc*)GetDoc())->GetNodes());
    SwXTextRange::GetPositions(aPaM);
    SwXTextCursor::SetPropertyToDefault(aPaM, aPropSet, rPropertyName);
}

Any SAL_CALL SwXTextRange::getPropertyDefault( const OUString& rPropertyName )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    if(!GetDoc() || !GetBookmark())
        throw RuntimeException();
    SwPaM aPaM(((SwDoc*)GetDoc())->GetNodes());
    SwXTextRange::GetPositions(aPaM);
    return SwXTextCursor::GetPropertyDefault(aPaM, aPropSet, rPropertyName);
}

/******************************************************************
 * SwXTextRanges
 ******************************************************************/

const uno::Sequence< sal_Int8 > & SwXTextRanges::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::binfilter::CreateUnoTunnelId();
    return aSeq;
}

sal_Int64 SAL_CALL SwXTextRanges::getSomething( const uno::Sequence< sal_Int8 >& rId )
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
/****************************************************************************
 *  Text positions
 *  Bis zum ersten Zugriff auf eine TextPosition wird ein SwCursor gehalten,
 * danach wird ein Array mit uno::Reference< XTextPosition >  angelegt
 *
****************************************************************************/
SV_IMPL_PTRARR(XTextRangeArr, XTextRangeRefPtr);

OUString SwXTextRanges::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXTextRanges");
}

BOOL SwXTextRanges::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.TextRanges") == rServiceName;
}

Sequence< OUString > SwXTextRanges::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TextRanges");
    return aRet;
}

SwXTextRanges::SwXTextRanges() :
    pRangeArr(0)
{

}

SwXTextRanges::~SwXTextRanges()
{
    SwUnoCrsr* pCrsr = GetCrsr();
    delete pCrsr;
    if(pRangeArr)
    {
        pRangeArr->DeleteAndDestroy(0, pRangeArr->Count());
        delete pRangeArr;
    }
}

sal_Int32 SwXTextRanges::getCount(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Int32 nRet = 0;
    SwUnoCrsr* pCrsr = GetCrsr();
    if(pCrsr)
    {
        FOREACHUNOPAM_START(pCrsr)
            nRet++;
        FOREACHUNOPAM_END()
    }
    else if(pRangeArr)
        nRet = pRangeArr->Count();
    return nRet;
}

uno::Any SwXTextRanges::getByIndex(sal_Int32 nIndex)
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< XTextRange >  aRef;
    XTextRangeArr* pArr = ((SwXTextRanges*)this)->GetRangesArray();
    if(pArr && pArr->Count() > nIndex)
    {
        XTextRangeRefPtr pRef = pArr->GetObject( USHORT( nIndex ));
        aRef = *pRef;
    }
    else
        throw lang::IndexOutOfBoundsException();
    uno::Any aRet(&aRef, ::getCppuType((uno::Reference<XTextRange>*)0));
    return aRet;
}

uno::Type  SwXTextRanges::getElementType(void) throw( uno::RuntimeException )
{
    return ::getCppuType((uno::Reference<XTextRange>*)0);
}

sal_Bool SwXTextRanges::hasElements(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    return getCount() > 0;
}

XTextRangeArr*  SwXTextRanges::GetRangesArray()
{
    SwUnoCrsr* pCrsr = GetCrsr();
    if(!pRangeArr && pCrsr)
    {
        pRangeArr = new XTextRangeArr();
        FOREACHUNOPAM_START(pCrsr)

            Reference< XTextRange >* pPtr =
                new Reference<XTextRange>( SwXTextRange::CreateTextRangeFromPosition(PUNOPAM->GetDoc(),
                        *PUNOPAM->GetPoint(), PUNOPAM->GetMark()));
//              new Reference<XTextRange>( SwXTextRange::createTextRangeFromPaM(*PUNOPAM, xParentText));
            if(pPtr->is())
                pRangeArr->Insert(pPtr, pRangeArr->Count());
        FOREACHUNOPAM_END()
        pCrsr->Remove( this );
    }
    return pRangeArr;
}

void    SwXTextRanges::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
}


void SwXTextCursor::SetString(SwCursor& rCrsr, const OUString& rString)
{
    // Start/EndAction
    SwDoc* pDoc = rCrsr.GetDoc();
    UnoActionContext aAction(pDoc);
    String aText(rString);
    xub_StrLen nTxtLen = aText.Len();
    if(rCrsr.HasMark())
        pDoc->DeleteAndJoin(rCrsr);
    if(nTxtLen)
    {
        //OPT: GetSystemCharSet
        if( !pDoc->Insert(rCrsr, aText) )
        {
            OSL_FAIL( "Doc->Insert(Str) failed." );
        }
        SwXTextCursor::SelectPam(rCrsr, sal_True);
        rCrsr.Left(nTxtLen, CRSR_SKIP_CHARS);
    }
}
/******************************************************************
 * SwXParaFrameEnumeration
 ******************************************************************/
SV_IMPL_PTRARR(SwDependArr, SwDepend*);

OUString SwXParaFrameEnumeration::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXParaFrameEnumeration");
}

BOOL SwXParaFrameEnumeration::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.util.ContentEnumeration") == rServiceName;
}

Sequence< OUString > SwXParaFrameEnumeration::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.util.ContentEnumeration");
    return aRet;
}

SwXParaFrameEnumeration::SwXParaFrameEnumeration(const SwUnoCrsr& rUnoCrsr,
                                                sal_uInt8 nParaFrameMode,
                                                SwFrmFmt* pFmt)
{
    SwDoc* pDoc = rUnoCrsr.GetDoc();
    SwUnoCrsr* pUnoCrsr = pDoc->CreateUnoCrsr(*rUnoCrsr.GetPoint(), sal_False);
    if(rUnoCrsr.HasMark())
    {
        pUnoCrsr->SetMark();
        *pUnoCrsr->GetMark() = *rUnoCrsr.GetMark();
    }
    pUnoCrsr->Add(this);

    if(PARAFRAME_PORTION_PARAGRAPH == nParaFrameMode)
        ::binfilter::CollectFrameAtNode( *this, rUnoCrsr.GetPoint()->nNode,
                                aFrameArr, FALSE );
    else if(pFmt)
    {
        //jetzt einen SwDepend anlegen und in das Array einfuegen
        SwDepend* pNewDepend = new SwDepend(this, pFmt);
        aFrameArr.C40_INSERT(SwDepend, pNewDepend, aFrameArr.Count());
    }
    else if((PARAFRAME_PORTION_CHAR == nParaFrameMode) ||
            (PARAFRAME_PORTION_TEXTRANGE == nParaFrameMode))
    {
        if(PARAFRAME_PORTION_TEXTRANGE == nParaFrameMode)
        {
            //created from any text range
            if(pUnoCrsr->HasMark())
            {
                if(pUnoCrsr->Start() != pUnoCrsr->GetPoint())
                    pUnoCrsr->Exchange();
                do
                {
                    FillFrame(*pUnoCrsr);
                    pUnoCrsr->Right(1, CRSR_SKIP_CHARS);
                }
                while(*pUnoCrsr->GetPoint() < *pUnoCrsr->GetMark());
            }
        }
        FillFrame(*pUnoCrsr);
    }
}

SwXParaFrameEnumeration::~SwXParaFrameEnumeration()
{
    aFrameArr.DeleteAndDestroy(0, aFrameArr.Count());
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    delete pUnoCrsr;
}
/* ---------------------------------------------------------------------------
    Description: Search for a FLYCNT text attribute at the cursor point
                and fill the frame into the array
 ---------------------------------------------------------------------------*/
void SwXParaFrameEnumeration::FillFrame(SwUnoCrsr& rUnoCrsr)
{
    // search for objects at the cursor - anchored at/as char
    SwTxtAttr* pTxtAttr = rUnoCrsr.GetNode()->GetTxtNode()->GetTxtAttr(
                    rUnoCrsr.GetPoint()->nContent, RES_TXTATR_FLYCNT);
    if(pTxtAttr)
    {
        const SwFmtFlyCnt& rFlyCnt = pTxtAttr->GetFlyCnt();
        SwFrmFmt* pFrmFmt = rFlyCnt.GetFrmFmt();
        //jetzt einen SwDepend anlegen und in das Array einfuegen
        SwDepend* pNewDepend = new SwDepend(this, pFrmFmt);
        aFrameArr.C40_INSERT(SwDepend, pNewDepend, aFrameArr.Count());
    }
}

sal_Bool SwXParaFrameEnumeration::hasMoreElements(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!GetCrsr())
        throw uno::RuntimeException();
    return xNextObject.is() ? sal_True : CreateNextObject();
}

uno::Any SwXParaFrameEnumeration::nextElement(void)
    throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!GetCrsr())
        throw uno::RuntimeException();
    if(!xNextObject.is() && aFrameArr.Count())
    {
        CreateNextObject();
    }
    if(!xNextObject.is())
        throw container::NoSuchElementException();
    uno::Any aRet(&xNextObject, ::getCppuType((uno::Reference<XTextContent>*)0));
    xNextObject = 0;
    return aRet;
}

sal_Bool SwXParaFrameEnumeration::CreateNextObject()
{
    if(!aFrameArr.Count())
        return sal_False;
    SwDepend* pDepend = aFrameArr.GetObject(0);
    aFrameArr.Remove(0);
    SwFrmFmt* pFormat = (SwFrmFmt*)pDepend->GetRegisteredIn();
    delete pDepend;
    // the format should be valid her otherwise the client
    // would have been removed in ::Modify
    // check for a shape first
    SwClientIter aIter(*pFormat);
    SwDrawContact* pContact = (SwDrawContact*)
                                            aIter.First(TYPE(SwDrawContact));
    if(pContact)
       {
          SdrObject* pSdr = pContact->GetMaster();
        if(pSdr)
        {
            xNextObject = uno::Reference< XTextContent >(pSdr->getUnoShape(), uno::UNO_QUERY);
        }
       }
    else
    {
        const SwNodeIndex* pIdx = pFormat->GetCntnt().GetCntntIdx();
        DBG_ASSERT(pIdx, "where is the index?");
        const SwNode* pNd = GetCrsr()->GetDoc()->GetNodes()[ pIdx->GetIndex() + 1 ];

        FlyCntType eType;
        if(!pNd->IsNoTxtNode())
            eType = FLYCNTTYPE_FRM;
        else if(pNd->IsGrfNode())
            eType = FLYCNTTYPE_GRF;
        else
            eType = FLYCNTTYPE_OLE;

        uno::Reference< container::XNamed >  xFrame = SwXFrames::GetObject(*pFormat, eType);
        xNextObject = uno::Reference< XTextContent >(xFrame, uno::UNO_QUERY);
    }

    return xNextObject.is();
}


void    SwXParaFrameEnumeration::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    switch( pOld ? pOld->Which() : 0 )
    {
    case RES_REMOVE_UNO_OBJECT:
    case RES_OBJECTDYING:
        if( (void*)GetRegisteredIn() == ((SwPtrMsgPoolItem *)pOld)->pObject )
            ((SwModify*)GetRegisteredIn())->Remove(this);
        break;

    case RES_FMT_CHG:
        // wurden wir an das neue umgehaengt und wird das alte geloscht?
        if( ((SwFmtChg*)pNew)->pChangedFmt == GetRegisteredIn() &&
            ((SwFmtChg*)pOld)->pChangedFmt->IsFmtInDTOR() )
            ((SwModify*)GetRegisteredIn())->Remove(this);
        break;
    }
    if(!GetRegisteredIn())
    {
        aFrameArr.DeleteAndDestroy(0, aFrameArr.Count());
        xNextObject = 0;
    }
    else
    {
        //dann war es vielleicht ein Frame am Absatz?
        for(sal_uInt16 i = aFrameArr.Count(); i; i--)
        {
            SwDepend* pDepend = aFrameArr.GetObject(i-1);
            if(!pDepend->GetRegisteredIn())
            {
                delete pDepend;
                aFrameArr.Remove(i-1);
            }
        }
    }
}
// -----------------------------------------------------------------------------
IMPLEMENT_FORWARD_XINTERFACE2(SwXTextCursor,SwXTextCursor_Base,OTextCursorHelper)


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
