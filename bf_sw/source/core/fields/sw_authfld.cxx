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

#define _SVSTDARR_STRINGSDTOR
#define _SVSTDARR_USHORTS
#define _SVSTDARR_LONGS
#define _SVSTDARR_ULONGS

#include <hintids.hxx>

#include <bf_svx/unolingu.hxx>

#include <authfld.hxx>
#include <expfld.hxx>

#include <osl/diagnose.h>

#include <pam.hxx>
#include <cntfrm.hxx>
#include <tox.hxx>
#include <txmsrt.hxx>
#include <doctxm.hxx>
#include <fmtfld.hxx>
#include <txtfld.hxx>
#include <ndtxt.hxx>

#include <horiornt.hxx>

#include <doc.hxx>
#include <unofldmid.h>
#include <unoprnms.hxx>
namespace binfilter {

extern String& GetString( const ::com::sun::star::uno::Any& rAny, String& rStr );

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::rtl;

#define C2U(cChar) ::rtl::OUString::createFromAscii(cChar)

typedef SwAuthEntry* SwAuthEntryPtr;
SV_DECL_PTRARR_DEL( SwAuthDataArr, SwAuthEntryPtr, 5, 5 )
SV_IMPL_PTRARR( SwAuthDataArr, SwAuthEntryPtr )


typedef SwTOXSortKey* TOXSortKeyPtr;
SV_DECL_PTRARR_DEL( SortKeyArr, TOXSortKeyPtr, 5, 5 )
SV_IMPL_PTRARR( SortKeyArr, TOXSortKeyPtr )



 SwAuthEntry::SwAuthEntry(const SwAuthEntry& rCopy)
    : nRefCount(0)
 {
    for(USHORT i = 0; i < AUTH_FIELD_END; i++)
        aAuthFields[i] = rCopy.aAuthFields[i];
 }
// --------------------------------------------------------
BOOL    SwAuthEntry::operator==(const SwAuthEntry& rComp)
{
    for(USHORT i = 0; i < AUTH_FIELD_END; i++)
        if(aAuthFields[i] != rComp.aAuthFields[i])
            return FALSE;
    return TRUE;
}
// --------------------------------------------------------
SwAuthorityFieldType::SwAuthorityFieldType(SwDoc* pDoc)
    : SwFieldType( RES_AUTHORITY )
    , m_pDoc(pDoc)
    , m_pDataArr(new SwAuthDataArr )
    , m_pSequArr(new SvLongs(5, 5))
    , m_pSortKeyArr(new SortKeyArr(3, 3))
    , m_cPrefix('[')
    , m_cSuffix(']')
    , m_bIsSequence(FALSE)
    , m_bSortByDocument(TRUE)
    , m_eLanguage((LanguageType)GetAppLanguage())
{
}

SwAuthorityFieldType::SwAuthorityFieldType( const SwAuthorityFieldType& rFType)
    : SwFieldType( RES_AUTHORITY )
    , m_pDataArr(new SwAuthDataArr )
    , m_pSequArr(new SvLongs(5, 5))
    , m_pSortKeyArr(new SortKeyArr(3, 3))
    , m_cPrefix(rFType.m_cPrefix)
    , m_cSuffix(rFType.m_cSuffix)
    , m_bIsSequence(rFType.m_bIsSequence)
    , m_bSortByDocument(rFType.m_bSortByDocument)
    , m_eLanguage(rFType.m_eLanguage)
    , m_sSortAlgorithm(rFType.m_sSortAlgorithm)
{
    for(USHORT i = 0; i < rFType.m_pSortKeyArr->Count(); i++)
        m_pSortKeyArr->Insert((*rFType.m_pSortKeyArr)[i], i);
}


SwAuthorityFieldType::~SwAuthorityFieldType()
{
//  DBG_ASSERT(!m_pDataArr->Count(), "Array is not empty");
    m_pSortKeyArr->DeleteAndDestroy(0, m_pSortKeyArr->Count());
    delete m_pSortKeyArr;
    delete m_pSequArr;
    delete m_pDataArr;
}

SwFieldType*    SwAuthorityFieldType::Copy()  const
{
    return new SwAuthorityFieldType(m_pDoc);
}

void    SwAuthorityFieldType::RemoveField(long nHandle)
{
#ifdef DBG_UTIL
    BOOL bRemoved = FALSE;
#endif
    for(USHORT j = 0; j < m_pDataArr->Count(); j++)
    {
        SwAuthEntry* pTemp = m_pDataArr->GetObject(j);
        long nRet = (long)(void*)pTemp;
        if(nRet == nHandle)
        {
#ifdef DBG_UTIL
            bRemoved = TRUE;
#endif
            pTemp->RemoveRef();
            if(!pTemp->GetRefCount())
            {
                m_pDataArr->DeleteAndDestroy(j, 1);
                //re-generate positions of the fields
                DelSequenceArray();
            }
            break;
        }
    }
#ifdef DBG_UTIL
    DBG_ASSERT(bRemoved, "Field unknown" );
#endif
}

long    SwAuthorityFieldType::AddField(const String& rFieldContents)
{
    long nRet = 0;
    SwAuthEntry* pEntry = new SwAuthEntry;
    for( USHORT i = 0; i < AUTH_FIELD_END; ++i )
        pEntry->SetAuthorField( (ToxAuthorityField)i,
                        rFieldContents.GetToken( i, TOX_STYLE_DELIMITER ));

    for(USHORT j = 0; j < m_pDataArr->Count() && pEntry; j++)
    {
        SwAuthEntry* pTemp = m_pDataArr->GetObject(j);
        if(*pTemp == *pEntry)
        {
            DELETEZ(pEntry);
            nRet = (long)(void*)pTemp;
            pTemp->AddRef();
        }
    }
    //if it is a new Entry - insert
    if(pEntry)
    {
        nRet = (long)(void*)pEntry;
        pEntry->AddRef();
        m_pDataArr->Insert(pEntry, m_pDataArr->Count());
        //re-generate positions of the fields
        DelSequenceArray();
    }
    return nRet;
}

BOOL SwAuthorityFieldType::AddField(long nHandle)
{
    BOOL bRet = FALSE;
    for( USHORT j = 0; j < m_pDataArr->Count(); j++ )
    {
        SwAuthEntry* pTemp = m_pDataArr->GetObject(j);
        long nTmp = (long)(void*)pTemp;
        if( nTmp == nHandle )
        {
            bRet = TRUE;
            pTemp->AddRef();
            //re-generate positions of the fields
            DelSequenceArray();
            break;
        }
    }
    DBG_ASSERT(bRet, "::AddField(long) failed");
    return bRet;
}

const SwAuthEntry*  SwAuthorityFieldType::GetEntryByHandle(long nHandle) const
{
    const SwAuthEntry* pRet = 0;
    for(USHORT j = 0; j < m_pDataArr->Count(); j++)
    {
        const SwAuthEntry* pTemp = m_pDataArr->GetObject(j);
        long nTmp = (long)(void*)pTemp;
        if( nTmp == nHandle )
        {
            pRet = pTemp;
            break;
        }
    }
    OSL_ENSURE( pRet, "invalid Handle" );
    return pRet;
}
/*-----------------------------------------------------------------------
    Description:    appends a new entry (if new) and returns the array position

  -----------------------------------------------------------------------*/
USHORT  SwAuthorityFieldType::AppendField( const SwAuthEntry& rInsert )
{
    USHORT nRet = 0;
    for( nRet = 0; nRet < m_pDataArr->Count(); ++nRet )
    {
        SwAuthEntry* pTemp = m_pDataArr->GetObject( nRet );
        if( *pTemp == rInsert )
        {
            break;
            //ref count unchanged
        }
    }

    //if it is a new Entry - insert
    if( nRet == m_pDataArr->Count() )
        m_pDataArr->Insert( new SwAuthEntry( rInsert ), nRet );

    return nRet;
}


long    SwAuthorityFieldType::GetHandle(USHORT nPos)
{
    long nRet = 0;
    if( nPos < m_pDataArr->Count() )
    {
        SwAuthEntry* pTemp = m_pDataArr->GetObject(nPos);
        nRet = (long)(void*)pTemp;
    }
    return nRet;
}

USHORT  SwAuthorityFieldType::GetEntryCount() const
{
    return m_pDataArr->Count();
}

USHORT  SwAuthorityFieldType::GetSequencePos(long nHandle)
{
    //find the field in a sorted array of handles,
    if(m_pSequArr->Count() && m_pSequArr->Count() != m_pDataArr->Count())
        DelSequenceArray();
    if(!m_pSequArr->Count())
    {
        SwTOXSortTabBases aSortArr;
        SwClientIter aIter( *this );

        SwTOXInternational aIntl(m_eLanguage, 0, m_sSortAlgorithm);

        for( SwFmtFld* pFmtFld = (SwFmtFld*)aIter.First( TYPE(SwFmtFld) );
                                pFmtFld; pFmtFld = (SwFmtFld*)aIter.Next() )
        {
            const SwTxtFld* pTxtFld = pFmtFld->GetTxtFld();
            if(!pTxtFld || !pTxtFld->GetpTxtNode())
                continue;

            const SwTxtNode& rFldTxtNode = pTxtFld->GetTxtNode();
            SwPosition aFldPos(rFldTxtNode);
            SwDoc& rDoc = *(SwDoc*)rFldTxtNode.GetDoc();
            SwCntntFrm *pFrm = rFldTxtNode.GetFrm();
            const SwTxtNode* pTxtNode = 0;
            if(pFrm && !pFrm->IsInDocBody())
                pTxtNode = GetBodyTxtNode( rDoc, aFldPos, *pFrm );
            //if no text node could be found or the field is in the document
            //body the directly available text node will be used
            if(!pTxtNode)
                pTxtNode = &rFldTxtNode;
            pTxtNode->GetIndex();
            if( pTxtNode->GetTxt().Len() && pTxtNode->GetFrm() &&
                pTxtNode->GetNodes().IsDocNodes() )
            {
                SwTOXAuthority* pNew = new SwTOXAuthority( *pTxtNode,
                                                            *pFmtFld, aIntl );

                for(short i = 0; i < aSortArr.Count(); ++i)
                {
                    SwTOXSortTabBase* pOld = aSortArr[i];
                    if(*pOld == *pNew)
                    {
                        //only the first occurrence in the document
                        //has to be in the array
                        if(*pOld < *pNew)
                            DELETEZ(pNew);
                        else // remove the old content
                            aSortArr.DeleteAndDestroy( i, 1 );
                        break;
                    }
                }
                //if it still exists - insert at the correct position
                if(pNew)
                {
                    short j;
                    for(j = 0; j < aSortArr.Count(); ++j)
                    {
                        SwTOXSortTabBase* pOld = aSortArr[j];
                        if(*pNew < *pOld)
                            break;
                    }
                    aSortArr.Insert(pNew, j );
                }
            }
        }

        for(USHORT i = 0; i < aSortArr.Count(); i++)
        {
            const SwTOXSortTabBase& rBase = *aSortArr[i];
            SwFmtFld& rFmtFld = ((SwTOXAuthority&)rBase).GetFldFmt();
            SwAuthorityField* pAFld = (SwAuthorityField*)rFmtFld.GetFld();
            m_pSequArr->Insert(pAFld->GetHandle(), i);
        }
        aSortArr.DeleteAndDestroy(0, aSortArr.Count());
    }
    //find nHandle
    USHORT nRet = 0;
    for(USHORT i = 0; i < m_pSequArr->Count(); i++)
    {
        if((*m_pSequArr)[i] == nHandle)
        {
            nRet = i + 1;
            break;
        }
    }
    return nRet;
}

BOOL    SwAuthorityFieldType::QueryValue( Any& rVal, BYTE nMId ) const
{
    nMId &= ~CONVERT_TWIPS;
    switch( nMId )
    {
    case FIELD_PROP_PAR1:
    case FIELD_PROP_PAR2:
        {
            OUString sVal;
            sal_Unicode uRet = FIELD_PROP_PAR1 == nMId ? m_cPrefix : m_cSuffix;
            if(uRet)
                sVal = OUString(uRet);
            rVal <<= sVal;
        }
        break;
    case FIELD_PROP_PAR3:
        rVal <<= OUString(GetSortAlgorithm());
        break;

    case FIELD_PROP_BOOL1:
    case FIELD_PROP_BOOL2:
        {
            sal_Bool bVal = FIELD_PROP_BOOL1 == nMId ? m_bIsSequence: m_bSortByDocument;
            rVal.setValue(&bVal, ::getBooleanCppuType());
        }
        break;

    case FIELD_PROP_LOCALE:
        rVal <<= SvxCreateLocale(GetLanguage());
        break;

    case FIELD_PROP_PROP_SEQ:
        {
            Sequence<PropertyValues> aRet(m_pSortKeyArr->Count());
            PropertyValues* pValues = aRet.getArray();
            OUString sProp1( C2U(SW_PROP_NAME_STR(UNO_NAME_SORT_KEY)) ),
                     sProp2( C2U(SW_PROP_NAME_STR(UNO_NAME_IS_SORT_ASCENDING)));
            for(sal_uInt16 i = 0; i < m_pSortKeyArr->Count(); i++)
            {
                const SwTOXSortKey* pKey = (*m_pSortKeyArr)[i];
                pValues[i].realloc(2);
                PropertyValue* pValue = pValues[i].getArray();
                pValue[0].Name = sProp1;
                pValue[0].Value <<= sal_Int16(pKey->eField);
                pValue[1].Name = sProp2;
                pValue[1].Value.setValue(&pKey->bSortAscending, ::getBooleanCppuType());
            }
            rVal <<= aRet;
        }
        break;
    default:
        OSL_FAIL("illegal property");
    }
    return TRUE;
}

BOOL    SwAuthorityFieldType::PutValue( const Any& rAny, BYTE nMId )
{
    nMId &= ~CONVERT_TWIPS;
    sal_Bool bRet = TRUE;
    String sTmp;
    switch( nMId )
    {
    case FIELD_PROP_PAR1:
    case FIELD_PROP_PAR2:
    {
        ::binfilter::GetString( rAny, sTmp );
        sal_Unicode uSet = sTmp.GetChar(0);
        if( FIELD_PROP_PAR1 == nMId )
            m_cPrefix = uSet;
        else
            m_cSuffix = uSet;
    }
    break;
    case FIELD_PROP_PAR3:
        SetSortAlgorithm( ::binfilter::GetString( rAny, sTmp ));
        break;

    case FIELD_PROP_BOOL1:
        m_bIsSequence = *(sal_Bool*)rAny.getValue();
        break;
    case FIELD_PROP_BOOL2:
        m_bSortByDocument = *(sal_Bool*)rAny.getValue();
        break;

    case FIELD_PROP_LOCALE:
        {
            Locale aLocale;
            if( 0 != (bRet = rAny >>= aLocale ))
                SetLanguage( SvxLocaleToLanguage( aLocale ));
        }
        break;

    case FIELD_PROP_PROP_SEQ:
        {
            Sequence<PropertyValues> aSeq;
            if( 0 != (bRet = rAny >>= aSeq) )
            {
                m_pSortKeyArr->DeleteAndDestroy(0, m_pSortKeyArr->Count());
                const PropertyValues* pValues = aSeq.getConstArray();
                for(sal_Int32 i = 0; i < aSeq.getLength() && i < USHRT_MAX / 4; i++)
                {
                    const PropertyValue* pValue = pValues[i].getConstArray();
                    SwTOXSortKey* pSortKey = new SwTOXSortKey;
                    for(sal_Int32 j = 0; j < pValues[i].getLength(); j++)
                    {
                        if(pValue[j].Name.equalsAsciiL(SW_PROP_NAME(UNO_NAME_SORT_KEY)))
                        {
                            sal_Int16 nVal = -1; pValue[j].Value >>= nVal;
                            if(nVal >= 0 && nVal < AUTH_FIELD_END)
                                pSortKey->eField = (ToxAuthorityField) nVal;
                            else
                                bRet = FALSE;
                        }
                        else if(pValue[j].Name.equalsAsciiL(SW_PROP_NAME(UNO_NAME_IS_SORT_ASCENDING)))
                        {
                            pSortKey->bSortAscending = *(sal_Bool*)pValue[j].Value.getValue();
                        }
                    }
                    m_pSortKeyArr->Insert(pSortKey, m_pSortKeyArr->Count());
                }
            }
        }
        break;
    default:
        OSL_FAIL("illegal property");
    }
    return bRet;
}

void SwAuthorityFieldType::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew )
{
    //re-generate positions of the fields
    DelSequenceArray();
    SwModify::Modify( pOld, pNew );
}

USHORT SwAuthorityFieldType::GetSortKeyCount() const
{
    return m_pSortKeyArr->Count();
}

const SwTOXSortKey*  SwAuthorityFieldType::GetSortKey(USHORT nIdx) const
{
    SwTOXSortKey* pRet = 0;
    if(m_pSortKeyArr->Count() > nIdx)
        pRet = (*m_pSortKeyArr)[nIdx];
    DBG_ASSERT(pRet, "Sort key not found");
    return pRet;
}

void SwAuthorityFieldType::SetSortKeys(USHORT nKeyCount, SwTOXSortKey aKeys[])
{
    m_pSortKeyArr->DeleteAndDestroy(0, m_pSortKeyArr->Count());
    USHORT nArrIdx = 0;
    for(USHORT i = 0; i < nKeyCount; i++)
        if(aKeys[i].eField < AUTH_FIELD_END)
            m_pSortKeyArr->Insert(new SwTOXSortKey(aKeys[i]), nArrIdx++);
}


SwAuthorityField::SwAuthorityField( SwAuthorityFieldType* pType1,
                                    const String& rFieldContents )
    : SwField(pType1)
{
    nHandle = pType1->AddField( rFieldContents );
}

SwAuthorityField::SwAuthorityField( SwAuthorityFieldType* pType2,
                                                long nSetHandle )
    : SwField( pType2 ),
    nHandle( nSetHandle )
{
    pType2->AddField( nHandle );
}

SwAuthorityField::~SwAuthorityField()
{
    ((SwAuthorityFieldType* )GetTyp())->RemoveField(nHandle);
}

String  SwAuthorityField::Expand() const
{
     SwAuthorityFieldType* pAuthType = (SwAuthorityFieldType*)GetTyp();
     String sRet;
     if(pAuthType->GetPrefix())
         sRet.Assign(pAuthType->GetPrefix());

     if( pAuthType->IsSequence() )
     {
         sRet += String::CreateFromInt32( pAuthType->GetSequencePos( nHandle ));
     }
     else
     {
         const SwAuthEntry* pEntry = pAuthType->GetEntryByHandle(nHandle);
         //TODO: Expand to: identifier, number sequence, ...
         if(pEntry)
             sRet += pEntry->GetAuthorField(AUTH_FIELD_IDENTIFIER);
     }
     if(pAuthType->GetSuffix())
         sRet += pAuthType->GetSuffix();
     return sRet;
}

SwField* SwAuthorityField::Copy() const
{
    SwAuthorityFieldType* pAuthType = (SwAuthorityFieldType*)GetTyp();
    return new SwAuthorityField(pAuthType, nHandle);
}

const String&   SwAuthorityField::GetFieldText(ToxAuthorityField eField) const
{
    SwAuthorityFieldType* pAuthType = (SwAuthorityFieldType*)GetTyp();
    const SwAuthEntry* pEntry = pAuthType->GetEntryByHandle( nHandle );
    return pEntry->GetAuthorField( eField );
}

void    SwAuthorityField::SetPar1(const String& rStr)
{
    SwAuthorityFieldType* pType3 = (SwAuthorityFieldType* )GetTyp();
    pType3->RemoveField(nHandle);
    nHandle = pType3->AddField(rStr);
}

const char* aFieldNames[] =
{
    "Identifier",
    "BibiliographicType",
    "Address",
    "Annote",
    "Author",
    "Booktitle",
    "Chapter",
    "Edition",
    "Editor",
    "Howpublished",
    "Institution",
    "Journal",
    "Month",
    "Note",
    "Number",
    "Organizations",
    "Pages",
    "Publisher",
    "School",
    "Series",
    "Title",
    "Report_Type",
    "Volume",
    "Year",
    "URL",
    "Custom1",
    "Custom2",
    "Custom3",
    "Custom4",
    "Custom5",
    "ISBN"
};

BOOL    SwAuthorityField::QueryValue( Any& rAny, BYTE nMId ) const
{
    nMId &= ~CONVERT_TWIPS;
    if(!GetTyp())
        return FALSE;
    const SwAuthEntry* pAuthEntry = ((SwAuthorityFieldType*)GetTyp())->GetEntryByHandle(nHandle);
    if(!pAuthEntry)
        return FALSE;
    Sequence <PropertyValue> aRet(AUTH_FIELD_END);
    PropertyValue* pValues = aRet.getArray();
    for(sal_Int16 i = 0; i < AUTH_FIELD_END; i++)
    {
        pValues[i].Name = C2U(aFieldNames[i]);
        const String& rField = pAuthEntry->GetAuthorField((ToxAuthorityField) i);
        if(i == AUTH_FIELD_AUTHORITY_TYPE)
            pValues[i].Value <<= sal_Int16(rField.ToInt32());
        else
            pValues[i].Value <<= OUString(rField);
    }
    rAny <<= aRet;
    return FALSE;
}

sal_Int16 lcl_Find(const OUString& rFieldName)
{
    for(sal_Int16 i = 0; i < AUTH_FIELD_END; i++)
        if(!rFieldName.compareToAscii(aFieldNames[i]))
            return i;
    return -1;
}
//----------------------------------------------------------------------------
BOOL    SwAuthorityField::PutValue( const Any& rAny, BYTE nMId )
{
    nMId &= ~CONVERT_TWIPS;
    if(!GetTyp() || !((SwAuthorityFieldType*)GetTyp())->GetEntryByHandle(nHandle))
        return FALSE;

    Sequence <PropertyValue> aParam;
    if(!(rAny >>= aParam))
        return FALSE;

    String sToSet;
    sToSet.Fill(AUTH_FIELD_ISBN, TOX_STYLE_DELIMITER);
    const PropertyValue* pParam = aParam.getConstArray();
    for(sal_Int32 i = 0; i < aParam.getLength(); i++)
    {
        sal_Int16 nFound = lcl_Find(pParam[i].Name);
        if(nFound >= 0)
        {
            OUString sContent;
            if(AUTH_FIELD_AUTHORITY_TYPE == nFound)
            {
                sal_Int16 nVal(0);
                pParam[i].Value >>= nVal;
                sContent = OUString::valueOf((sal_Int32)nVal);
            }
            else
                pParam[i].Value >>= sContent;
            sToSet.SetToken(nFound, TOX_STYLE_DELIMITER, sContent);
        }
    }

    ((SwAuthorityFieldType*)GetTyp())->RemoveField(nHandle);
    nHandle = ((SwAuthorityFieldType*)GetTyp())->AddField(sToSet);

    return FALSE;
}

SwFieldType* SwAuthorityField::ChgTyp( SwFieldType* pFldTyp )
{
    SwAuthorityFieldType* pSrcTyp = (SwAuthorityFieldType*)GetTyp(),
                        * pDstTyp = (SwAuthorityFieldType*)pFldTyp;
    if( pSrcTyp != pDstTyp )
    {

        const SwAuthEntry* pEntry = pSrcTyp->GetEntryByHandle( nHandle );
        USHORT nHdlPos = pDstTyp->AppendField( *pEntry );
        pSrcTyp->RemoveField( nHandle );
        nHandle = pDstTyp->GetHandle( nHdlPos );
        pDstTyp->AddField( nHandle );
        SwField::ChgTyp( pFldTyp );
    }
    return pSrcTyp;
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
