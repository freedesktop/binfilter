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

#include <vcl/outdev.hxx>

#include <editobj2.hxx>
#include <editeng.hxx>
#include <fontitem.hxx>
#include <cscoitem.hxx>

#include "itemdata.hxx"

#include <flditem.hxx>
#include <lrspitem.hxx>
#include <tstpitem.hxx>
#include <bulitem.hxx>
#include <numitem.hxx>
#include <brshitem.hxx>
#include <bf_svtools/intitem.hxx>

#include <tools/tenccvt.hxx>
#include <bf_tools/string.hxx>
namespace binfilter {

DBG_NAME( EE_EditTextObject )
DBG_NAME( XEditAttribute )

XEditAttribute* MakeXEditAttribute( SfxItemPool& rPool, const SfxPoolItem& rItem, USHORT nStart, USHORT nEnd )
{
    // das neue Attribut im Pool anlegen
    const SfxPoolItem& rNew = rPool.Put( rItem );

    XEditAttribute* pNew = new XEditAttribute( rNew, nStart, nEnd );
    return pNew;
}

XEditAttribute::XEditAttribute( const SfxPoolItem& rAttr, USHORT nS, USHORT nE )
{
    DBG_CTOR( XEditAttribute, 0 );
    pItem = &rAttr;
    nStart = nS;
    nEnd = nE;
}

XEditAttribute::~XEditAttribute()
{
    DBG_DTOR( XEditAttribute, 0 );
    pItem = 0;    // Gehoert dem Pool.
}

XEditAttribute* XEditAttributeList::FindAttrib( USHORT nWhich, USHORT nChar ) const
{
    for ( USHORT n = Count(); n; )
    {
        XEditAttribute* pAttr = GetObject( --n );
        if( ( pAttr->GetItem()->Which() == nWhich ) && ( pAttr->GetStart() <= nChar ) && ( pAttr->GetEnd() > nChar ) )
            return pAttr;
    }
    return NULL;
}

ContentInfo::ContentInfo( SfxItemPool& rPool ) : aParaAttribs( rPool, EE_PARA_START, EE_CHAR_END )
{
    eFamily = SFX_STYLE_FAMILY_PARA;
    pTempLoadStoreInfos = NULL;
}

// Richtiger CopyCTOR unsinning, weil ich mit einem anderen Pool arbeiten muss!
ContentInfo::ContentInfo( const ContentInfo& rCopyFrom, SfxItemPool& rPoolToUse )
    : aParaAttribs( rPoolToUse, EE_PARA_START, EE_CHAR_END )
{
    pTempLoadStoreInfos = NULL;
    // So sollten die Items im richtigen Pool landen!
    aParaAttribs.Set( rCopyFrom.GetParaAttribs() );
    aText = rCopyFrom.GetText();
    aStyle = rCopyFrom.GetStyle();
    eFamily = rCopyFrom.GetFamily();

    // Attribute kopieren...
    for ( USHORT n = 0; n < rCopyFrom.GetAttribs().Count(); n++  )
    {
        XEditAttribute* pAttr = rCopyFrom.GetAttribs().GetObject( n );
        XEditAttribute* pMyAttr = MakeXEditAttribute( rPoolToUse, *pAttr->GetItem(), pAttr->GetStart(), pAttr->GetEnd() );
        aAttribs.Insert( pMyAttr, aAttribs.Count()  );
    }
}

ContentInfo::~ContentInfo()
{
    for ( USHORT nAttr = 0; nAttr < aAttribs.Count(); nAttr++ )
    {
        XEditAttribute* pAttr = aAttribs.GetObject(nAttr);
        // Item aus Pool entfernen!
        aParaAttribs.GetPool()->Remove( *pAttr->GetItem() );
        delete pAttr;
    }
    aAttribs.Remove( 0, aAttribs.Count() );
}

void ContentInfo::CreateLoadStoreTempInfos()
{
    delete pTempLoadStoreInfos;
    pTempLoadStoreInfos = new LoadStoreTempInfos;
}

void ContentInfo::DestroyLoadStoreTempInfos()
{
    delete pTempLoadStoreInfos;
    pTempLoadStoreInfos = NULL;
}

EditTextObject::EditTextObject( USHORT n)
{
    DBG_CTOR( EE_EditTextObject, 0 );
    nWhich = n;
}

EditTextObject::EditTextObject( const EditTextObject& r )
{
    DBG_CTOR( EE_EditTextObject, 0 );
    nWhich = r.nWhich;
}

EditTextObject::~EditTextObject()
{
    DBG_DTOR( EE_EditTextObject, 0 );
}

SfxItemSet EditTextObject::GetParaAttribs( USHORT ) const
{
    OSL_FAIL( "V-Methode direkt vom EditTextObject!" );
    return SfxItemSet( *(SfxItemPool*)NULL );
}

EditTextObject* EditTextObject::Create( SvStream& rIStream, SfxItemPool* pGlobalTextObjectPool )
{
    ULONG nStartPos = rIStream.Tell();

    // Ertmal sehen, was fuer ein Object...
    USHORT nLclWhich;
    rIStream >> nLclWhich;

    sal_uInt32 nStructSz;
    rIStream >> nStructSz;

    DBG_ASSERT( ( nLclWhich == 0x22 /*EE_FORMAT_BIN300*/ ) || ( nLclWhich == EE_FORMAT_BIN ), "CreateTextObject: Unbekanntes Objekt!" );

    if ( rIStream.GetError() )
        return NULL;

    EditTextObject* pTxtObj = NULL;
    switch ( nLclWhich )
    {
        case 0x22 /*BIN300*/:        pTxtObj = new BinTextObject( 0 );
                                    ((BinTextObject*)pTxtObj)->CreateData300( rIStream );
                                    break;
        case EE_FORMAT_BIN:            pTxtObj = new BinTextObject( pGlobalTextObjectPool );
                                    pTxtObj->CreateData( rIStream );
                                    break;
        default:
        {
            // Wenn ich das Format nicht kenne, ueberlese ich den Inhalt:
            rIStream.SetError( EE_READWRITE_WRONGFORMAT );
        }
    }

    // Sicherstellen, dass der Stream an der richtigen Stelle hinterlassen wird.
    ULONG nFullSz = sizeof( nLclWhich ) + sizeof( nStructSz ) + nStructSz;
    rIStream.Seek( nStartPos + nFullSz );
    return pTxtObj;
}


BinTextObject::BinTextObject( SfxItemPool* pP ) :
    EditTextObject( EE_FORMAT_BIN )
{
    nVersion = 0;
    nMetric = 0xFFFF;
    nUserType = 0;
    nObjSettings = 0;
    pPortionInfo = 0;
    if ( pP )
    {
        pPool =    pP;
        bOwnerOfPool = FALSE;
    }
    else
    {
        pPool =    EditEngine::CreatePool();
        bOwnerOfPool =    TRUE;
    }
    bVertical = FALSE;
    bStoreUnicodeStrings = FALSE;
    nScriptType = 0;
}

BinTextObject::BinTextObject( const BinTextObject& r ) :
    EditTextObject( r )
{
    nVersion = r.nVersion;
    nMetric = r.nMetric;
    nUserType = r.nUserType;
    nObjSettings = r.nObjSettings;
    bVertical = r.bVertical;
    nScriptType = r.nScriptType;
    pPortionInfo = NULL;    // PortionInfo nicht kopieren
    bStoreUnicodeStrings = FALSE;
    if ( !r.bOwnerOfPool )
    {
        // Dann den Pool mitverwenden
        pPool =    r.pPool;
        bOwnerOfPool = FALSE;
    }
    else
    {
        pPool =    EditEngine::CreatePool();
        bOwnerOfPool =    TRUE;

    }

    if ( bOwnerOfPool && pPool && r.pPool )
        pPool->SetDefaultMetric( r.pPool->GetMetric( DEF_METRIC ) );

    for ( USHORT n = 0; n < r.aContents.Count(); n++ )
    {
        ContentInfo* pOrg = r.aContents.GetObject( n );
        DBG_ASSERT( pOrg, "NULL-Pointer in ContentList!" );
        ContentInfo* pNew = new ContentInfo( *pOrg, *pPool );
        aContents.Insert( pNew, aContents.Count() );
    }
}

BinTextObject::~BinTextObject()
{
    ClearPortionInfo();
    DeleteContents();
    if ( bOwnerOfPool )
    {
        delete pPool;
    }
}

USHORT BinTextObject::GetUserType() const
{
    return nUserType;
}

void BinTextObject::SetUserType( USHORT n )
{
    nUserType = n;
}



BOOL BinTextObject::IsVertical() const
{
    return bVertical;
}

void BinTextObject::SetVertical( BOOL b )
{
    if ( b != bVertical )
    {
        bVertical = b;
        ClearPortionInfo();
    }
}


void BinTextObject::SetScriptType( USHORT nType )
{
    nScriptType = nType;
}


void BinTextObject::DeleteContents()
{
    for ( USHORT n = 0; n < aContents.Count(); n++ )
    {
        ContentInfo* p = aContents.GetObject( n );
        DBG_ASSERT( p, "NULL-Pointer in ContentList!" );
        delete p;
    }
    aContents.Remove( 0, aContents.Count() );
}

EditTextObject*    BinTextObject::Clone() const
{
    return new BinTextObject( *this );
}

XEditAttribute* BinTextObject::CreateAttrib( const SfxPoolItem& rItem, USHORT nStart, USHORT nEnd )
{
    return MakeXEditAttribute( *pPool, rItem, nStart, nEnd );
}

void BinTextObject::DestroyAttrib( XEditAttribute* pAttr )
{
    pPool->Remove( *pAttr->GetItem() );
    delete pAttr;
}

ContentInfo* BinTextObject::CreateAndInsertContent()
{
    ContentInfo* pC = new ContentInfo( *pPool );
    aContents.Insert( pC, aContents.Count() );
    return pC;
}

USHORT BinTextObject::GetParagraphCount() const
{
    return aContents.Count();
}

XubString BinTextObject::GetText( USHORT nPara ) const
{
    DBG_ASSERT( nPara < aContents.Count(), "BinTextObject::GetText: Absatz existiert nicht!" );
    if ( nPara < aContents.Count() )
    {
        ContentInfo* pC = aContents[ nPara ];
        return pC->GetText();
    }
    return XubString();
}

void BinTextObject::Insert( const EditTextObject& rObj, USHORT nDestPara )
{
    DBG_ASSERT( rObj.Which() == EE_FORMAT_BIN, "UTO: Unbekanntes Textobjekt" );

    const BinTextObject& rBinObj = (const BinTextObject&)rObj;

    if ( nDestPara > aContents.Count() )
        nDestPara = aContents.Count();

    const USHORT nParas = rBinObj.GetContents().Count();
    for ( USHORT nP = 0; nP < nParas; nP++ )
    {
        ContentInfo* pC = rBinObj.GetContents()[ nP ];
        ContentInfo* pNew = new ContentInfo( *pC, *GetPool() );
        aContents.Insert( pNew, nDestPara+nP );
    }
    ClearPortionInfo();
}




void BinTextObject::ClearPortionInfo()
{
    if ( pPortionInfo )
    {
        for ( USHORT n = pPortionInfo->Count(); n; )
            delete pPortionInfo->GetObject( --n );
        delete pPortionInfo;
        pPortionInfo = NULL;
    }
}




void BinTextObject::MergeParaAttribs( const SfxItemSet& rAttribs, USHORT nStart, USHORT nEnd )
{
    BOOL bChanged = FALSE;

    for ( USHORT nPara = GetContents().Count(); nPara; )
    {
        ContentInfo* pC = GetContents().GetObject( --nPara );

        for ( USHORT nW = nStart; nW <= nEnd; nW++ )
        {
            if ( ( pC->GetParaAttribs().GetItemState( nW, FALSE ) != SFX_ITEM_ON )
                    && ( rAttribs.GetItemState( nW, FALSE ) == SFX_ITEM_ON ) )
            {
                pC->GetParaAttribs().Put( rAttribs.Get( nW ) );
                bChanged = TRUE;
            }
        }
    }

    if ( bChanged )
        ClearPortionInfo();
}



BOOL BinTextObject::HasField( TypeId aType ) const
{
    USHORT nParagraphs = GetContents().Count();
    for ( USHORT nPara = 0; nPara < nParagraphs; nPara++ )
    {
        ContentInfo* pC = GetContents().GetObject( nPara );
        USHORT nAttrs = pC->GetAttribs().Count();
        for ( USHORT nAttr = 0; nAttr < nAttrs; nAttr++ )
        {
            XEditAttribute* pAttr = pC->GetAttribs()[nAttr];
            if ( pAttr->GetItem()->Which() == EE_FEATURE_FIELD )
            {
                if ( !aType )
                    return TRUE;

                const SvxFieldData* pFldData = ((const SvxFieldItem*)pAttr->GetItem())->GetField();
                if ( pFldData && pFldData->IsA( aType ) )
                    return TRUE;
            }
        }
    }
    return FALSE;
}

SfxItemSet BinTextObject::GetParaAttribs( USHORT nPara ) const
{
    ContentInfo* pC = GetContents().GetObject( nPara );
    return pC->GetParaAttribs();
}





void BinTextObject::GetStyleSheet( USHORT nPara, XubString& rName, SfxStyleFamily& rFamily ) const
{
    if ( nPara < aContents.Count() )
    {
        ContentInfo* pC = aContents[ nPara ];
        rName = pC->GetStyle();
        rFamily = pC->GetFamily();
    }
}

BOOL BinTextObject::ImpChangeStyleSheets(
                    const XubString& rOldName, SfxStyleFamily eOldFamily,
                    const XubString& rNewName, SfxStyleFamily eNewFamily )
{
    const USHORT nParagraphs = GetContents().Count();
    BOOL bChanges = FALSE;

    for ( USHORT nPara = 0; nPara < nParagraphs; nPara++ )
    {
        ContentInfo* pC = GetContents().GetObject( nPara );
        if ( pC->GetFamily() == eOldFamily )
        {
            if ( pC->GetStyle() == rOldName )
            {
                pC->GetStyle() = rNewName;
                pC->GetFamily() = eNewFamily;
                bChanges = TRUE;
            }
        }
    }
    return bChanges;
}

BOOL BinTextObject::ChangeStyleSheets(
                    const XubString& rOldName, SfxStyleFamily eOldFamily,
                    const XubString& rNewName, SfxStyleFamily eNewFamily )
{
    BOOL bChanges = ImpChangeStyleSheets( rOldName, eOldFamily, rNewName, eNewFamily );
    if ( bChanges )
        ClearPortionInfo();

    return bChanges;
}

void BinTextObject::ChangeStyleSheetName( SfxStyleFamily eFamily,
                const XubString& rOldName, const XubString& rNewName )
{
    ImpChangeStyleSheets( rOldName, eFamily, rNewName, eFamily );
}

void BinTextObject::CreateData( SvStream& rIStream )
{
    rIStream >> nVersion;

    // Das Textobject wurde erstmal mit der aktuellen Einstellung
    // von pTextObjectPool erzeugt.
    BOOL bOwnerOfCurrent = bOwnerOfPool;
    rIStream >> bOwnerOfPool;

    if ( bOwnerOfCurrent && !bOwnerOfPool )
    {
        // Es wurde ein globaler Pool verwendet, mir jetzt nicht uebergeben,
        // aber ich brauche ihn!
        OSL_FAIL( "Man gebe mir den globalen TextObjectPool!" );
        return;
    }
    else if ( !bOwnerOfCurrent && bOwnerOfPool )
    {
        // Es soll ein globaler Pool verwendet werden, aber dieses
        // Textobject hat einen eigenen.
        pPool =    EditEngine::CreatePool();
    }

    if ( bOwnerOfPool )
        GetPool()->Load( rIStream );

    // CharSet, in dem gespeichert wurde:
    USHORT nCharSet;
    rIStream >> nCharSet;

    rtl_TextEncoding eSrcEncoding = GetSOLoadTextEncoding( (rtl_TextEncoding)nCharSet );

    // Die Anzahl der Absaetze...
    USHORT nParagraphs;
    rIStream >> nParagraphs;

    // Die einzelnen Absaetze...
    for ( ULONG nPara = 0; nPara < nParagraphs; nPara++ )
    {
        ContentInfo* pC = CreateAndInsertContent();

        // Der Text...
        rtl::OString aByteString = read_lenPrefixed_uInt8s_ToOString<sal_uInt16>(rIStream);
        pC->GetText() = rtl::OStringToOUString(aByteString, eSrcEncoding);

        // StyleName und Family...
        pC->GetStyle() = rIStream.ReadUniOrByteString( eSrcEncoding );
        USHORT nStyleFamily;
        rIStream >> nStyleFamily;
        pC->GetFamily() = (SfxStyleFamily)nStyleFamily;

        // Absatzattribute...
        pC->GetParaAttribs().Load( rIStream );

        // Die Anzahl der Attribute...
        USHORT nAttribs;
        rIStream >> nAttribs;

        // Und die einzelnen Attribute
        // Items als Surregate => immer 8 Byte pro Attrib
        // Which = 2; Surregat = 2; Start = 2; End = 2;
        USHORT nAttr;
        for ( nAttr = 0; nAttr < nAttribs; nAttr++ )
        {
            USHORT nLclWhich, nStart, nEnd;
            const SfxPoolItem* pItem;

            rIStream >> nLclWhich;
            nLclWhich = pPool->GetNewWhich( nLclWhich );
            pItem = pPool->LoadSurrogate( rIStream, nLclWhich, 0 );
            rIStream >> nStart;
            rIStream >> nEnd;
            if ( pItem )
            {
                if ( pItem->Which() == EE_FEATURE_NOTCONV )
                {
                    pC->GetText().SetChar( nStart, ByteString::ConvertToUnicode( aByteString[nStart], ((SvxCharSetColorItem*)pItem)->GetCharSet() ) );
                }
                else
                {
                    XEditAttribute* pAttr = new XEditAttribute( *pItem, nStart, nEnd );
                    pC->GetAttribs().Insert( pAttr, pC->GetAttribs().Count() );

                    if ( ( nLclWhich >= EE_FEATURE_START ) && ( nLclWhich <= EE_FEATURE_END ) )
                    {
                        // Convert CH_FEATURE to CH_FEATURE_OLD
                        DBG_ASSERT( (BYTE) aByteString[nStart] == CH_FEATURE_OLD, "CreateData: CH_FEATURE expected!" );
                        if ( (BYTE) aByteString[nStart] == CH_FEATURE_OLD )
                            pC->GetText().SetChar( nStart, CH_FEATURE );
                    }
                }
            }
        }

        // Symbol-Conversion neccessary?
        // All Strings are converted with the SourceCharSet in CreateData()...

        // Keep old ByteString, maybe Symbol-Conversion neccessary, will be
        // checked in FinishLoad(), I need the StyleSheetPool for this...
        if ( pC->GetStyle().Len() && ( pC->GetParaAttribs().GetItemState( EE_CHAR_FONTINFO ) != SFX_ITEM_ON ) )
        {
            pC->CreateLoadStoreTempInfos();
            pC->GetLoadStoreTempInfos()->aOrgString_Load = aByteString;
        }

        // But check for paragraph and character symbol attribs here,
        // FinishLoad will not be called in OpenOffice Calc, no StyleSheets...

        BOOL bSymbolPara = FALSE;
        if ( pC->GetParaAttribs().GetItemState( EE_CHAR_FONTINFO ) == SFX_ITEM_ON )
        {
            const SvxFontItem& rFontItem = (const SvxFontItem&)pC->GetParaAttribs().Get( EE_CHAR_FONTINFO );
            if ( rFontItem.GetCharSet() == RTL_TEXTENCODING_SYMBOL )
            {
                pC->GetText() = rtl::OStringToOUString(aByteString, RTL_TEXTENCODING_SYMBOL);
                bSymbolPara = TRUE;
            }
        }

        for ( nAttr = pC->GetAttribs().Count(); nAttr; )
        {
            XEditAttribute* pAttr = pC->GetAttribs().GetObject( --nAttr );
            if ( pAttr->GetItem()->Which() == EE_CHAR_FONTINFO )
            {
                const SvxFontItem& rFontItem = (const SvxFontItem&)*pAttr->GetItem();
                if ( ( !bSymbolPara && ( rFontItem.GetCharSet() == RTL_TEXTENCODING_SYMBOL ) )
                      || ( bSymbolPara && ( rFontItem.GetCharSet() != RTL_TEXTENCODING_SYMBOL ) ) )
                {
                    // Not correctly converted
                    rtl::OString aPart(aByteString.copy(pAttr->GetStart(), pAttr->GetEnd()-pAttr->GetStart()));
                    rtl::OUString aNew(rtl::OStringToOUString(aPart, rFontItem.GetCharSet()));
                    pC->GetText().Erase( pAttr->GetStart(), pAttr->GetEnd()-pAttr->GetStart() );
                    pC->GetText().Insert( aNew, pAttr->GetStart() );
                }

                // #88414# Convert StarMath and StarBats to StarSymbol
                FontToSubsFontConverter hConv = CreateFontToSubsFontConverter( rFontItem.GetFamilyName(), FONTTOSUBSFONT_IMPORT | FONTTOSUBSFONT_ONLYOLDSOSYMBOLFONTS );
                if ( hConv )
                {
                    SvxFontItem aNewFontItem( rFontItem );
                    aNewFontItem.GetFamilyName() = GetFontToSubsFontName( hConv );

                    pC->GetAttribs().Remove( nAttr );
                    XEditAttribute* pNewAttr = CreateAttrib( aNewFontItem, pAttr->GetStart(), pAttr->GetEnd() );
                    pC->GetAttribs().Insert( pNewAttr, nAttr );
                    DestroyAttrib( pAttr );

                    for ( USHORT nChar = pNewAttr->GetStart(); nChar < pNewAttr->GetEnd(); nChar++ )
                    {
                        sal_Unicode cOld = pC->GetText().GetChar( nChar );
                        DBG_ASSERT( cOld >= 0xF000, "cOld not converted?!" );
                        sal_Unicode cConv = ConvertFontToSubsFontChar( hConv, cOld );
                        if ( cConv )
                            pC->GetText().SetChar( nChar, cConv );
                    }

                    DestroyFontToSubsFontConverter( hConv );
                }
            }
        }


        // #88414# Convert StarMath and StarBats to StarSymbol
        // Maybe old symbol font as paragraph attribute?
        if ( pC->GetParaAttribs().GetItemState( EE_CHAR_FONTINFO ) == SFX_ITEM_ON )
        {
            const SvxFontItem& rFontItem = (const SvxFontItem&)pC->GetParaAttribs().Get( EE_CHAR_FONTINFO );
            FontToSubsFontConverter hConv = CreateFontToSubsFontConverter( rFontItem.GetFamilyName(), FONTTOSUBSFONT_IMPORT | FONTTOSUBSFONT_ONLYOLDSOSYMBOLFONTS );
            if ( hConv )
            {
                SvxFontItem aNewFontItem( rFontItem );
                aNewFontItem.GetFamilyName() = GetFontToSubsFontName( hConv );
                pC->GetParaAttribs().Put( aNewFontItem );

                for ( USHORT nChar = 0; nChar < pC->GetText().Len(); nChar++ )
                {
                    if ( !pC->GetAttribs().FindAttrib( EE_CHAR_FONTINFO, nChar ) )
                    {
                        sal_Unicode cOld = pC->GetText().GetChar( nChar );
                        DBG_ASSERT( cOld >= 0xF000, "cOld not converted?!" );
                        sal_Unicode cConv = ConvertFontToSubsFontChar( hConv, cOld );
                        if ( cConv )
                            pC->GetText().SetChar( nChar, cConv );
                    }
                }

                DestroyFontToSubsFontConverter( hConv );
            }
        }
    }

    // Ab 400 auch die DefMetric:
    if ( nVersion >= 400 )
    {
        USHORT nTmpMetric;
        rIStream >> nTmpMetric;
        if ( nVersion >= 401 )
        {
            // In der 400 gab es noch einen Bug bei Textobjekten mit eigenem
            // Pool, deshalb erst ab 401 auswerten.
            nMetric = nTmpMetric;
            if ( bOwnerOfPool && pPool && ( nMetric != 0xFFFF ) )
                pPool->SetDefaultMetric( (SfxMapUnit)nMetric );
        }
    }

    if ( nVersion >= 600 )
    {
        rIStream >> nUserType;
        rIStream >> nObjSettings;
    }

    if ( nVersion >= 601 )
    {
        rIStream >> bVertical;
    }

    if ( nVersion >= 602 )
    {
        rIStream >> nScriptType;

        BOOL bUnicodeStrings;
        rIStream >> bUnicodeStrings;
        if ( bUnicodeStrings )
        {
            for ( USHORT nPara = 0; nPara < nParagraphs; nPara++ )
            {
                ContentInfo* pC = GetContents().GetObject( nPara );
                USHORT nL;

                // Text
                rIStream >> nL;
                if ( nL )
                {
                    pC->GetText().AllocBuffer( nL );
                    rIStream.Read( pC->GetText().GetBufferAccess(), nL*sizeof(sal_Unicode) );
                    pC->GetText().ReleaseBufferAccess( (USHORT)nL );
                }

                // StyleSheetName
                rIStream >> nL;
                if ( nL )
                {
                    pC->GetStyle().AllocBuffer( nL );
                    rIStream.Read( pC->GetStyle().GetBufferAccess(), nL*sizeof(sal_Unicode) );
                    pC->GetStyle().ReleaseBufferAccess( (USHORT)nL );
                }
            }
        }
    }


    // Ab 500 werden die Tabs anders interpretiert: TabPos + LI, vorher nur TabPos.
    // Wirkt nur wenn auch Tab-Positionen eingestellt wurden, nicht beim DefTab.
    if ( nVersion < 500 )
    {
        for ( USHORT n = 0; n < aContents.Count(); n++ )
        {
            ContentInfo* pC = aContents.GetObject( n );
            const SvxLRSpaceItem& rLRSpace = (const SvxLRSpaceItem&) pC->GetParaAttribs().Get( EE_PARA_LRSPACE );
            if ( rLRSpace.GetTxtLeft() && ( pC->GetParaAttribs().GetItemState( EE_PARA_TABS ) == SFX_ITEM_ON ) )
            {
                const SvxTabStopItem& rTabs = (const SvxTabStopItem&) pC->GetParaAttribs().Get( EE_PARA_TABS );
                SvxTabStopItem aNewTabs( 0, 0, SVX_TAB_ADJUST_LEFT, EE_PARA_TABS );
                for ( USHORT t = 0; t < rTabs.Count(); t++ )
                {
                    const SvxTabStop& rT = rTabs[ t ];
                    aNewTabs.Insert( SvxTabStop( rT.GetTabPos() - rLRSpace.GetTxtLeft(),
                                rT.GetAdjustment(), rT.GetDecimal(), rT.GetFill() ) );
                }
                pC->GetParaAttribs().Put( aNewTabs );
            }
        }
    }
}

USHORT BinTextObject::GetVersion() const
{
    return nVersion;
}

void BinTextObject::FinishLoad( SfxStyleSheetPool* pStyleSheetPool )
{
    BOOL bCreateNumBulletItem = nVersion && ( nVersion < 501 );
    for ( USHORT nPara = GetContents().Count(); nPara; )
    {
        ContentInfo* pC = GetContents().GetObject( --nPara );

        if( GetUserType() == 0x0003 ) // !! OUTLINERMODE_OUTLINEOBJECT !!
        {
            if ( pC->GetParaAttribs().GetItemState( EE_PARA_NUMBULLET ) == SFX_ITEM_ON )
            {
                SvxNumBulletItem* pNumBullet = (SvxNumBulletItem*) &pC->GetParaAttribs().Get( EE_PARA_NUMBULLET );
                if( pNumBullet->GetNumRule()->GetNumRuleType() != SVX_RULETYPE_PRESENTATION_NUMBERING )
                {
                    pNumBullet->GetNumRule()->SetNumRuleType( SVX_RULETYPE_PRESENTATION_NUMBERING );
                    pC->GetParaAttribs().Put( *pNumBullet, EE_PARA_NUMBULLET );
                }
            }
        }

        if ( bCreateNumBulletItem )
        {
            BOOL bBulletInPara = pC->GetParaAttribs().GetItemState( EE_PARA_BULLET ) == SFX_ITEM_ON;
            BOOL bLRSpaceInPara = pC->GetParaAttribs().GetItemState( EE_PARA_LRSPACE ) == SFX_ITEM_ON;

            // Nur wenn ein Attribut hart gesetzt, ansonsten muss es in den Vorlagen
            // richtig konvertiert sein.

            if ( bBulletInPara || bLRSpaceInPara )
            {
                const SvxBulletItem* pBullet = NULL;
                const SvxLRSpaceItem* pLRSpace = NULL;
                SvxNumBulletItem* pNumBullet = NULL;
                SfxStyleSheet* pStyle = NULL;

                if ( pC->GetStyle().Len() )
                    pStyle = (SfxStyleSheet*)pStyleSheetPool->Find( pC->GetStyle(), pC->GetFamily() );

                const SfxUInt16Item& rLevel = (const SfxUInt16Item&) pC->GetParaAttribs().Get( EE_PARA_OUTLLEVEL );
                USHORT nLevel = rLevel.GetValue();

                if ( !pStyle || bBulletInPara )
                    pBullet = (const SvxBulletItem*) &pC->GetParaAttribs().Get( EE_PARA_BULLET );
                else
                    pBullet = (const SvxBulletItem*) &pStyle->GetItemSet().Get( EE_PARA_BULLET );

                if ( !pStyle || bLRSpaceInPara )
                    pLRSpace = (const SvxLRSpaceItem*) &pC->GetParaAttribs().Get( EE_PARA_LRSPACE );
                else
                    pLRSpace = (const SvxLRSpaceItem*) &pStyle->GetItemSet().Get( EE_PARA_LRSPACE );

                if ( !pStyle || ( pC->GetParaAttribs().GetItemState( EE_PARA_NUMBULLET ) == SFX_ITEM_ON ) )
                    pNumBullet = (SvxNumBulletItem*) &pC->GetParaAttribs().Get( EE_PARA_NUMBULLET );
                else
                    pNumBullet = (SvxNumBulletItem*) &pStyle->GetItemSet().Get( EE_PARA_NUMBULLET );

                SvxNumBulletItem aNumBullet( *pNumBullet );
                EditEngine::ImportBulletItem( aNumBullet, nLevel, pBullet, pLRSpace );
                pC->GetParaAttribs().Put( aNumBullet );

                if ( bLRSpaceInPara )
                    pC->GetParaAttribs().ClearItem( EE_PARA_LRSPACE );
            }
        }

        // Symbol-Convertion because of StyleSheet?
        if ( pStyleSheetPool && pC->GetLoadStoreTempInfos() && pC->GetLoadStoreTempInfos()->aOrgString_Load.Len() )
        {
            SfxStyleSheet* pStyle = (SfxStyleSheet*)pStyleSheetPool->Find( pC->GetStyle(), pC->GetFamily() );
            if ( pStyle )
            {
                const SvxFontItem& rFontItem = (const SvxFontItem&)pStyle->GetItemSet().Get( EE_CHAR_FONTINFO );
                if ( rFontItem.GetCharSet() == RTL_TEXTENCODING_SYMBOL )
                {
                    String aConverted( pC->GetLoadStoreTempInfos()->aOrgString_Load, RTL_TEXTENCODING_SYMBOL );

                    // Replace only Parts without hard font attribute, other symbol encoding
                    // is already done in CreateData()...

                    USHORT nLastEnd = 0;
                    for ( USHORT nAttr = 0; nAttr < pC->GetAttribs().Count(); nAttr++ )
                    {
                        XEditAttribute* pAttr = pC->GetAttribs().GetObject( nAttr );
                        if ( pAttr->GetItem()->Which() == EE_CHAR_FONTINFO )
                        {
                            if ( nLastEnd < pAttr->GetStart() )
                            {
                                USHORT nLen = pAttr->GetStart() - nLastEnd;
                                pC->GetText().Erase( nLastEnd, nLen );
                                pC->GetText().Insert( aConverted, nLastEnd, nLen, nLastEnd );
                            }
                            nLastEnd = pAttr->GetEnd();

                        }
                    }
                    if ( nLastEnd < pC->GetText().Len() )
                    {
                        USHORT nLen = pC->GetText().Len() - nLastEnd;
                        pC->GetText().Erase( nLastEnd, nLen );
                        pC->GetText().Insert( aConverted, nLastEnd, nLen, nLastEnd );
                    }
                }
            }
        }

        pC->DestroyLoadStoreTempInfos();

        // MT 07/00: EE_PARA_BULLET no longer needed
        pC->GetParaAttribs().ClearItem( EE_PARA_BULLET );
    }
}

void BinTextObject::AdjustImportedLRSpaceItems( BOOL bTurnOfBullets )
{
    for ( USHORT nPara = GetContents().Count(); nPara; )
    {
        ContentInfo* pC = GetContents().GetObject( --nPara );
        if ( !bTurnOfBullets )
        {
            for ( USHORT n = 0; n <=1; n++ )
            {
                USHORT nItemId = n ? EE_PARA_LRSPACE : EE_PARA_OUTLLRSPACE;
                if ( pC->GetParaAttribs().GetItemState( nItemId ) == SFX_ITEM_ON )
                {
                    const SvxLRSpaceItem& rItem = (const SvxLRSpaceItem&) pC->GetParaAttribs().Get( nItemId );
                    if ( !rItem.GetTxtFirstLineOfst() )
                    {
                        SvxLRSpaceItem aNewItem( rItem );
                        aNewItem.SetTxtFirstLineOfst( (short) -1200 ); // Outliner im Draw mit MAP100TH_MM
                        if( pC->GetParaAttribs().GetItemState( EE_PARA_NUMBULLET ) == SFX_ITEM_ON )
                        {
                            const SvxNumBulletItem& rNumBullet = (const SvxNumBulletItem&) pC->GetParaAttribs().Get( EE_PARA_NUMBULLET );
                            const SfxUInt16Item& rLevel = (const SfxUInt16Item&) pC->GetParaAttribs().Get( EE_PARA_OUTLLEVEL );
                            const SvxNumberFormat* pFmt = rNumBullet.GetNumRule()->Get( rLevel.GetValue() );
                            if ( pFmt )
                                aNewItem.SetTxtFirstLineOfst( pFmt->GetFirstLineOffset() );
                        }
                        pC->GetParaAttribs().Put( aNewItem );
                    }
                }
            }
        }

        // Wurden in alten Versionen vom Outliner mal am Absatz gesetzt, soll nicht mehr sein
        if ( !bTurnOfBullets && ( nVersion < 500 ) )
        {
            for ( USHORT nW = EE_CHAR_START; nW <= EE_CHAR_END; nW++  )
                pC->GetParaAttribs().ClearItem( nW );
        }

        if ( bTurnOfBullets )
        {
            SfxUInt16Item aBulletState( EE_PARA_BULLETSTATE, 0 );
            pC->GetParaAttribs().Put( aBulletState );
        }
    }
}



#define CHARSETMARKER    0x9999

void BinTextObject::CreateData300( SvStream& rIStream )
{
    // Fuer Aufwaertskompatibilitaet.

    // Erst den Pool laden...
    // Ist in der 300 immer gespeichert worden!
    GetPool()->Load( rIStream );

    // Die Anzahl der Absaetze...
    sal_uInt32 nParagraphs;
    rIStream >> nParagraphs;

    // Die einzelnen Absaetze...
    for ( sal_uInt32 nPara = 0; nPara < nParagraphs; nPara++ )
    {
        ContentInfo* pC = CreateAndInsertContent();

        // Der Text...
        pC->GetText() = rIStream.ReadUniOrByteString(
            rIStream.GetStreamCharSet() );

        // StyleName und Family...
        pC->GetStyle() = rIStream.ReadUniOrByteString(
            rIStream.GetStreamCharSet() );
        USHORT nStyleFamily;
        rIStream >> nStyleFamily;
        pC->GetFamily() = (SfxStyleFamily)nStyleFamily;

        // Absatzattribute...
        pC->GetParaAttribs().Load( rIStream );

        // Die Anzahl der Attribute...
        sal_uInt32 nAttribs;
        rIStream >> nAttribs;

        // Und die einzelnen Attribute
        // Items als Surregate => immer 8 Byte pro Attrib
        // Which = 2; Surregat = 2; Start = 2; End = 2;
        for ( sal_uInt32 nAttr = 0; nAttr < nAttribs; nAttr++ )
        {
            USHORT nLclWhich, nStart, nEnd;
            const SfxPoolItem* pItem;

            rIStream >> nLclWhich;
            nLclWhich = pPool->GetNewWhich( nLclWhich );
            pItem = pPool->LoadSurrogate( rIStream, nLclWhich, 0 );
            rIStream >> nStart;
            rIStream >> nEnd;
            if ( pItem )
            {
                XEditAttribute* pAttr = new XEditAttribute( *pItem, nStart, nEnd );
                pC->GetAttribs().Insert( pAttr, pC->GetAttribs().Count() );
            }
        }
    }

    // Prueffen, ob ein Zeichensatz gespeichert wurde
    USHORT nCharSetMarker;
    rIStream >> nCharSetMarker;
    if ( nCharSetMarker == CHARSETMARKER )
    {
        USHORT nCharSet;
        rIStream >> nCharSet;
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
