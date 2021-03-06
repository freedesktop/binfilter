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

#include "swrect.hxx"
#include <hintids.hxx>
#include <bf_svx/paperinf.hxx>

//Statt uiparam.hxx selbst definieren, das spart keys
#include <bf_svx/dialogs.hrc>
#include <sal/macros.h>
#define ITEMID_FONTLIST         SID_ATTR_CHAR_FONTLIST

#include <sot/storinfo.hxx>
#include <bf_svtools/ctrltool.hxx>
#include <bf_svtools/lingucfg.hxx>
#include <bf_sfx2/docfile.hxx>
#include <bf_sfx2/printer.hxx>
#include <bf_svx/asiancfg.hxx>
#include <bf_sfx2/request.hxx>
#include <bf_svtools/intitem.hxx>
#include <bf_svx/adjitem.hxx>

#include <com/sun/star/document/UpdateDocMode.hpp>

#include <rtl/logfile.hxx>
#include <bf_sfx2/docfilt.hxx>
#include <bf_svx/xtable.hxx>
#ifndef _SVX_DRAWITEM_HXX //autogen
#define ITEMID_COLOR_TABLE SID_COLOR_TABLE
#include <bf_svx/drawitem.hxx>
#endif
#include <bf_svx/fontitem.hxx>
#include <bf_svx/flstitem.hxx>
#include <bf_svx/tstpitem.hxx>
#include <bf_svx/langitem.hxx>
#include <bf_svx/colritem.hxx>
#include <bf_svx/hyznitem.hxx>

#include <bf_offmgr/app.hxx>

#include <prtopt.hxx>

#include <osl/diagnose.h>

#include <wdocsh.hxx>
#include <swmodule.hxx>

#include <horiornt.hxx>

#include <doc.hxx>
#include <docstyle.hxx>
#include <sw3io.hxx>        // I/O, Hausformat
#include <shellio.hxx>
#include <usrpref.hxx>
#include <fontcfg.hxx>
#include <poolfmt.hxx>
#include <globdoc.hxx>
#include <ndole.hxx>
#include <unotxdoc.hxx>
#include <linkenum.hxx>

#include <swerror.h>
#include <globals.hrc>

// #107253#
#include <swlinguconfig.hxx>
namespace binfilter {


using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::rtl;
#define C2U(cChar) OUString::createFromAscii(cChar)




/*--------------------------------------------------------------------
    Beschreibung: Document laden
 --------------------------------------------------------------------*/


/*N*/ sal_Bool SwDocShell::InitNew( SvStorage * pStor )
/*N*/ {
/*N*/   RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDocShell::InitNew" );
/*N*/
/*N*/   sal_Bool bRet = SfxInPlaceObject::InitNew( pStor );
/*N*/   OSL_ENSURE( GetMapUnit() == MAP_TWIP, "map unit is not twip!" );
/*N*/   sal_Bool bHTMLTemplSet = sal_False;
/*N*/   if( bRet )
/*N*/   {
/*N*/       AddLink();      // pDoc / pIo ggf. anlegen
/*N*/
/*N*/         sal_Bool bWeb = ISA( SwWebDocShell );
/*N*/         if ( bWeb )
/*?*/           bHTMLTemplSet = FALSE;//Styles aus HTML.vor
/*N*/       else if( ISA( SwGlobalDocShell ) )
/*?*/           GetDoc()->SetGlobalDoc();       // Globaldokument
/*N*/
/*N*/       if ( GetCreateMode() ==  SFX_CREATE_MODE_EMBEDDED )
/*N*/       {
                SvEmbeddedObject* pLclObj = this;
                const Size aSz( lA4Width - 2 * lMinBorder, 6 * MM50 );
                SwRect aVis( Point( DOCUMENTBORDER, DOCUMENTBORDER ), aSz );
                pLclObj->SetVisArea( aVis.SVRect() );
                pDoc->SetBrowseMode( TRUE );
/*N*/       }
        // set forbidden characters if necessary
/*N*/         SvxAsianConfig aAsian;
/*N*/         pDoc->SetKernAsianPunctuation(!aAsian.IsKerningWesternTextOnly());
/*N*/         pDoc->SetCharCompressType((SwCharCompressType)aAsian.GetCharDistanceCompression());
/*N*/         pDoc->SetPrintData(*SW_MOD()->GetPrtOptions(bWeb));

/*N*/         SubInitNew();

        // fuer alle

/*N*/       SwStdFontConfig* pStdFont = SW_MOD()->GetStdFontConfig();

/*N*/       String sEntry;
/*N*/         USHORT aFontWhich[] =
/*N*/         {   RES_CHRATR_FONT,
/*N*/             RES_CHRATR_CJK_FONT,
/*N*/             RES_CHRATR_CTL_FONT
/*N*/         };
/*N*/         USHORT aFontIds[] =
/*N*/         {
/*N*/             FONT_STANDARD,
/*N*/             FONT_STANDARD_CJK,
/*N*/             FONT_STANDARD_CTL
/*N*/         };
/*M*/         USHORT nFontTypes[] =
/*M*/         {
/*M*/             DEFAULTFONT_LATIN_TEXT,
/*M*/             DEFAULTFONT_CJK_TEXT,
/*M*/             DEFAULTFONT_CTL_TEXT
/*M*/         };
/*M*/         USHORT aLangTypes[] =
/*N*/         {
/*N*/             RES_CHRATR_LANGUAGE,
/*N*/             RES_CHRATR_CJK_LANGUAGE,
/*N*/             RES_CHRATR_CTL_LANGUAGE
/*N*/         };
/*N*/
/*N*/         for(USHORT i = 0; i < 3; i++)
/*N*/         {
/*N*/             USHORT nFontWhich = aFontWhich[i];
/*N*/             USHORT nFontId = aFontIds[i];
/*N*/             SvxFontItem* pFontItem = 0;
/*N*/             if(!pStdFont->IsFontDefault(nFontId))
/*N*/             {
/*?*/                 sEntry = pStdFont->GetFontFor(nFontId);
/*?*/                 const SfxFont* pFnt = NULL;
/*?*/                 pFnt = new SfxFont( FAMILY_DONTKNOW, sEntry, PITCH_DONTKNOW,
/*?*/                                     ::osl_getThreadTextEncoding() );
/*?*/                 pFontItem = new SvxFontItem(pFnt->GetFamily(), pFnt->GetName(),
/*?*/                                     aEmptyStr, pFnt->GetPitch(), pFnt->GetCharSet(),
                                          nFontWhich);
/*?*/                 delete (SfxFont*) pFnt;
/*?*/             }
/*N*/             else
/*M*/             {
/*N*/                 const SvxLanguageItem& rLang = (const SvxLanguageItem&)pDoc->GetDefault( aLangTypes[i] );
/*N*/               // #107782# OJ use korean language if latin was used
/*N*/               LanguageType eLanguage = rLang.GetLanguage();
/*N*/               if ( i == 0 )
/*N*/               {
/*N*/                   LanguageType eUiLanguage = Application::GetSettings().GetUILanguage();
/*N*/                   switch( eUiLanguage )
/*N*/                   {
/*N*/                       case LANGUAGE_KOREAN:
/*N*/                       case LANGUAGE_KOREAN_JOHAB:
/*N*/                           eLanguage = eUiLanguage;
/*N*/                       break;
/*N*/                   }
/*N*/               }
/*N*/
/*N*/               Font aLangDefFont = OutputDevice::GetDefaultFont(
/*N*/                     nFontTypes[i],
/*N*/                     eLanguage,
/*N*/                     DEFAULTFONT_FLAGS_ONLYONE );
/*N*/                 pFontItem = new SvxFontItem(aLangDefFont.GetFamily(), aLangDefFont.GetName(),
/*N*/                                     aEmptyStr, aLangDefFont.GetPitch(), aLangDefFont.GetCharSet(), nFontWhich);
/*M*/             }
/*N*/             pDoc->SetDefault(*pFontItem);
/*N*/           if( !bHTMLTemplSet )
/*N*/           {
/*N*/               SwTxtFmtColl *pColl = pDoc->GetTxtCollFromPool(RES_POOLCOLL_STANDARD);
/*N*/               pColl->ResetAttr(nFontWhich);
/*N*/           }
/*N*/             delete pFontItem;
/*N*/         }
/*M*/         USHORT aFontIdPoolId[] =
/*M*/         {
/*M*/             FONT_OUTLINE,       RES_POOLCOLL_HEADLINE_BASE,
/*M*/             FONT_LIST,          RES_POOLCOLL_NUMBUL_BASE,
/*M*/             FONT_CAPTION,       RES_POOLCOLL_LABEL,
/*M*/             FONT_INDEX,         RES_POOLCOLL_REGISTER_BASE,
/*M*/             FONT_OUTLINE_CJK,   RES_POOLCOLL_HEADLINE_BASE,
/*M*/             FONT_LIST_CJK,      RES_POOLCOLL_NUMBUL_BASE,
/*M*/             FONT_CAPTION_CJK,   RES_POOLCOLL_LABEL,
/*M*/             FONT_INDEX_CJK,     RES_POOLCOLL_REGISTER_BASE,
/*M*/             FONT_OUTLINE_CTL,   RES_POOLCOLL_HEADLINE_BASE,
/*M*/             FONT_LIST_CTL,      RES_POOLCOLL_NUMBUL_BASE,
/*M*/             FONT_CAPTION_CTL,   RES_POOLCOLL_LABEL,
/*M*/             FONT_INDEX_CTL,     RES_POOLCOLL_REGISTER_BASE
/*M*/         };
/*N*/
/*N*/         USHORT nFontWhich = RES_CHRATR_FONT;
/*M*/         for(USHORT nIdx = 0; nIdx < 24; nIdx += 2)
/*M*/         {
/*M*/             if(nIdx == 8)
/*M*/                 nFontWhich = RES_CHRATR_CJK_FONT;
/*M*/             else if(nIdx == 16)
/*M*/                 nFontWhich = RES_CHRATR_CTL_FONT;
/*M*/             if(!pStdFont->IsFontDefault(aFontIdPoolId[nIdx]))
/*M*/             {
/*M*/                 sEntry = pStdFont->GetFontFor(aFontIdPoolId[nIdx]);
/*M*/                 const SfxFont* pFnt = NULL;
/*M*/                 pFnt = new SfxFont( FAMILY_DONTKNOW, sEntry, PITCH_DONTKNOW,
/*M*/                                    ::osl_getThreadTextEncoding() );
/*M*/                 SwTxtFmtColl *pColl = pDoc->GetTxtCollFromPool(aFontIdPoolId[nIdx + 1]);
/*M*/                 if( !bHTMLTemplSet ||
/*M*/                     SFX_ITEM_SET != pColl->GetAttrSet().GetItemState(
/*M*/                                                   nFontWhich, sal_False ) )
/*M*/                 {
/*M*/                   pColl->SetAttr(SvxFontItem(pFnt->GetFamily(), pFnt->GetName(),
/*M*/                                  aEmptyStr, pFnt->GetPitch(), pFnt->GetCharSet(), nFontWhich));
/*M*/                 }
/*M*/                 delete (SfxFont*) pFnt;
/*M*/             }
/*M*/         }
/*N*/     }
/*N*/
    /* #106748# If the default frame direction of a document is RTL
         the default adjusment is to the right. */
/*N*/      if( !bHTMLTemplSet &&
/*N*/       FRMDIR_HORI_RIGHT_TOP == GetDefaultFrameDirection(GetAppLanguage()) )
/*N*/         pDoc->SetDefault( SvxAdjustItem(SVX_ADJUST_RIGHT) );
/*N*/
/*N*/   return bRet;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung:   Ctor mit SfxCreateMode ?????
 --------------------------------------------------------------------*/


/*N*/ SwDocShell::SwDocShell(SfxObjectCreateMode eMode)
/*N*/ : SfxObjectShell ( eMode )
/*N*/ , pDoc(0)
/*N*/ , pIo(0)
/*N*/ , pBasePool(0)
/*N*/ , pFontList(0)
/*N*/ , pView( 0 )
/*N*/ , nUpdateDocMode(document::UpdateDocMode::ACCORDING_TO_CONFIG)
/*N*/ {
/*N*/   RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDocShell::SwDocShell" );
/*N*/     Init_Impl();
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: Ctor / Dtor
 --------------------------------------------------------------------*/


/*?*/ SwDocShell::SwDocShell( SwDoc *pD, SfxObjectCreateMode eMode )
/*?*/ : SfxObjectShell ( eMode )
/*?*/ , pDoc(pD)
/*?*/ , pIo(0)
/*?*/ , pBasePool(0)
/*?*/ , pFontList(0)
/*?*/ , pView( 0 )
/*?*/ , nUpdateDocMode(document::UpdateDocMode::ACCORDING_TO_CONFIG)
/*?*/ {
/*?*/   RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDocShell::SwDocShell" );
/*?*/     Init_Impl();
/*?*/ }

/*--------------------------------------------------------------------
    Beschreibung:   Dtor
 --------------------------------------------------------------------*/


/*N*/  SwDocShell::~SwDocShell()
/*N*/ {
/*N*/   RemoveLink();
/*N*/   delete pIo;
/*N*/   delete pFontList;
/*N*/
/*N*/   // wir als BroadCaster werden auch unser eigener Listener
/*N*/   // (fuer DocInfo/FileNamen/....)
/*N*/   EndListening( *this );
/*N*/   SvxColorTableItem* pColItem = (SvxColorTableItem*)GetItem(SID_COLOR_TABLE);
/*N*/   // wird nur die DocInfo fuer den Explorer gelesen, ist das Item nicht da
/*N*/   if(pColItem)
/*N*/   {
/*N*/       XColorTable* pTable = pColItem->GetColorTable();
/*N*/       // wurde eine neue Table angelegt, muss sie auch geloescht werden.
/*N*/       if((void*)pTable  != (void*)(OFF_APP())->GetStdColorTable())
/*?*/           delete pTable;
/*N*/   }
/*N*/ }


/*N*/ void  SwDocShell::Init_Impl()
/*N*/ {
/*N*/   SetShell(this);
/*N*/     SetPool(&SW_MOD()->GetPool());
/*N*/   SetBaseModel(new SwXTextDocument(this));
/*N*/   // wir als BroadCaster werden auch unser eigener Listener
    // (fuer DocInfo/FileNamen/....)
/*N*/   StartListening( *this );
    //position of the "Automatic" style filter for the stylist (app.src)
/*N*/     SetAutoStyleFilterIndex(3);

    // set map unit to twip
/*N*/   SetMapUnit( MAP_TWIP );
/*N*/ }
/*--------------------------------------------------------------------
    Beschreibung: AddLink
 --------------------------------------------------------------------*/


/*N*/ void SwDocShell::AddLink()
/*N*/ {
/*N*/   if( !pDoc )
/*N*/   {
/*N*/       SwDocFac aFactory;
/*N*/       pDoc = aFactory.GetDoc();
/*N*/       pDoc->AddLink();
/*N*/       pDoc->SetHTMLMode( ISA(SwWebDocShell) );
/*N*/   }
/*N*/   else
/*?*/       pDoc->AddLink();
/*N*/   pDoc->SetDocShell( this );      // am Doc den DocShell-Pointer setzen
/*N*/   uno::Reference< text::XTextDocument >  xDoc(GetBaseModel(), uno::UNO_QUERY);
/*N*/   ((SwXTextDocument*)xDoc.get())->Reactivate(this);
/*N*/
/*N*/   if( !pIo )
/*N*/       pIo = new Sw3Io( *pDoc );
/*N*/
/*N*/   SetPool(&pDoc->GetAttrPool());

    // am besten erst wenn eine sdbcx::View erzeugt wird !!!
/*N*/   pDoc->SetOle2Link(LINK(this, SwDocShell, Ole2ModifiedHdl));
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung:   neue FontList erzeugen Aenderung Drucker
 --------------------------------------------------------------------*/


/*N*/ void SwDocShell::UpdateFontList()
/*N*/ {
/*N*/   OSL_ENSURE(pDoc, "Kein Doc keine FontList");
/*N*/   if( pDoc )
/*N*/   {
/*N*/       SfxPrinter* pPrt = pDoc->GetPrt();
/*N*/       delete pFontList;
/*N*/
/*N*/       if( pPrt && pPrt->GetDevFontCount() && !pDoc->IsBrowseMode() )
/*N*/           pFontList = new FontList( pPrt );
/*N*/       else
/*N*/           pFontList = new FontList( Application::GetDefaultDevice() );
/*N*/
/*N*/       PutItem( SvxFontListItem( pFontList, SID_ATTR_CHAR_FONTLIST ) );
/*N*/   }
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: RemoveLink
 --------------------------------------------------------------------*/


/*N*/ void SwDocShell::RemoveLink()
/*N*/ {
/*N*/   // Uno-Object abklemmen
/*N*/   uno::Reference< text::XTextDocument >  xDoc(GetBaseModel(), uno::UNO_QUERY);
/*N*/   ((SwXTextDocument*)xDoc.get())->Invalidate();
/*N*/   aFinishedTimer.Stop();
/*N*/   if(pDoc)
/*N*/   {
/*N*/       DELETEZ(pBasePool);
/*N*/       sal_Int8 nRefCt = pDoc->RemoveLink();
/*N*/       pDoc->SetOle2Link(Link());
/*N*/       pDoc->SetDocShell( 0 );
/*N*/       if( !nRefCt )
/*?*/           delete pDoc;
/*N*/       pDoc = 0;       // wir haben das Doc nicht mehr !!
/*N*/   }
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: Laden, Default-Format
 --------------------------------------------------------------------*/


/*N*/ sal_Bool  SwDocShell::Load(SvStorage* pStor)
/*N*/ {
/*N*/   RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDocShell::Load" );
/*N*/   sal_Bool bRet = sal_False;
/*N*/   sal_Bool bXML = pStor->GetVersion() >= SOFFICE_FILEFORMAT_60;
/*N*/   if( SfxInPlaceObject::Load( pStor ))
/*N*/   {
/*N*/       RTL_LOGFILE_CONTEXT_TRACE( aLog, "after SfxInPlaceObject::Load" );
/*N*/       if( pDoc )              // fuer Letzte Version !!
/*?*/           RemoveLink();       // das existierende Loslassen
/*N*/
/*N*/       AddLink();      // Link setzen und Daten updaten !!
/*N*/
/*N*/       // Das Laden
/*N*/       // fuer MD
/*N*/       if( bXML )
/*N*/       {
/*?*/           OSL_ENSURE( !pBasePool, "wer hat seinen Pool nicht zerstoert?" );
/*?*/           pBasePool = new SwDocStyleSheetPool( *pDoc,
/*?*/                           SFX_CREATE_MODE_ORGANIZER == GetCreateMode() );
/*?*/             if(GetCreateMode() != SFX_CREATE_MODE_ORGANIZER)
/*?*/             {
/*?*/                 SfxMedium* pLclMedium = GetMedium();
/*?*/                 SFX_ITEMSET_ARG( pLclMedium->GetItemSet(), pUpdateDocItem, SfxUInt16Item, SID_UPDATEDOCMODE, sal_False);
/*?*/                 nUpdateDocMode = pUpdateDocItem ? pUpdateDocItem->GetValue() : ::com::sun::star::document::UpdateDocMode::NO_UPDATE;
/*?*/             }
/*?*/
/*N*/       }
/*N*/
/*N*/       sal_uInt32 nErr = ERR_SWG_READ_ERROR;
/*N*/       switch( GetCreateMode() )
/*N*/       {
/*N*/       case SFX_CREATE_MODE_ORGANIZER:
/*?*/           break;
/*N*/       case SFX_CREATE_MODE_INTERNAL:
/*N*/       case SFX_CREATE_MODE_EMBEDDED:
            // SfxProgress unterdruecken, wenn man Embedded ist
/*N*/           SW_MOD()->SetEmbeddedLoadSave( sal_True );
            // kein break;
/*N*/       case SFX_CREATE_MODE_STANDARD:
/*N*/       case SFX_CREATE_MODE_PREVIEW:
/*N*/           {
/*N*/               Reader *pReader = bXML ? 0 /*ReadXML*/ : ReadSw3;
                    OSL_ENSURE( !bXML, "ReadXML removed");

/*N*/               if( pReader )
/*N*/               {
/*N*/                   // die DocInfo vom Doc am DocShell-Medium setzen
/*N*/                   RTL_LOGFILE_CONTEXT_TRACE( aLog, "before ReadDocInfo" );
/*N*/                   if( !bXML )
/*N*/                   {
/*N*/                       SfxDocumentInfo aInfo;
/*N*/                       aInfo.Load( pStor );
/*N*/                       pDoc->DocInfoChgd( aInfo );
/*N*/                   }
/*N*/                   SwReader aRdr( *pStor, aEmptyStr, pDoc );
/*N*/                   RTL_LOGFILE_CONTEXT_TRACE( aLog, "before Read" );
/*N*/                   nErr = aRdr.Read( *pReader );
/*N*/                   RTL_LOGFILE_CONTEXT_TRACE( aLog, "after Read" );

                    // If a XML document is loaded, the global doc/web doc
                    // flags have to be set, because they aren't loaded
                    // by this formats.
/*?*/                   if( ISA( SwWebDocShell ) )
/*?*/                   {
/*?*/                       if( !pDoc->IsHTMLMode() )
/*?*/                           pDoc->SetHTMLMode( TRUE );
/*?*/                   }
/*N*/                   if( ISA( SwGlobalDocShell ) )
/*N*/                   {
/*N*/                       if( !pDoc->IsGlobalDoc() )
/*?*/                           pDoc->SetGlobalDoc( TRUE );
/*N*/                   }
/*N*/               }
/*N*/ #ifdef DBG_UTIL
/*N*/               else
/*?*/                   OSL_ENSURE( !this, "ohne Sw3Reader geht nichts" );
/*N*/ #endif
/*N*/           }
/*N*/           break;

/*N*/       default:
/*N*/ #ifdef DBG_UTIL
/*?*/           OSL_ENSURE( !this, "Load: new CreateMode?" );
/*N*/ #endif
/*N*/           break;
/*N*/       }
/*N*/
/*N*/       if( !bXML )
/*N*/       {
/*N*/           OSL_ENSURE( !pBasePool, "wer hat seinen Pool nicht zerstoert?" );
/*N*/           pBasePool = new SwDocStyleSheetPool( *pDoc,
/*N*/                           SFX_CREATE_MODE_ORGANIZER == GetCreateMode() );
/*N*/       }
/*N*/       UpdateFontList();
/*N*/       InitDraw();
/*N*/
/*N*/       SetError( nErr );
/*N*/       bRet = !IsError( nErr );

        // StartFinishedLoading rufen.
/*N*/       if( bRet && !pDoc->IsInLoadAsynchron() &&
/*N*/           GetCreateMode() == SFX_CREATE_MODE_STANDARD )
/*N*/           StartLoadFinishedTimer();

        // SfxProgress unterdruecken, wenn man Embedded ist
/*N*/       SW_MOD()->SetEmbeddedLoadSave( sal_False );

/*N*/       if( pDoc->IsGlobalDoc() && !pDoc->IsGlblDocSaveLinks() )
/*N*/       {
/*N*/           // dann entferne alle nicht referenzierte OLE-Objecte
/*N*/           SvStorageInfoList aInfoList;
/*N*/           pStor->FillInfoList( &aInfoList );
/*N*/
/*N*/           // erstmal alle nicht "OLE-Objecte" aus der Liste entfernen
                sal_uInt32 n;
/*N*/           for( n = aInfoList.size(); n; )
/*N*/           {
/*N*/               const String& rName = aInfoList[ --n ].GetName();
/*N*/               // in ndole.cxx wird dieser PreFix benutzt
/*N*/               if( 3 != rName.Match( String::CreateFromAscii("Obj") ))
                    {
                        SvStorageInfoList::iterator it = aInfoList.begin();
                        ::std::advance( it, n );
                        aInfoList.erase( it );
                    }
/*N*/           }
/*N*/
/*N*/           // dann alle referenzierten Object aus der Liste entfernen
/*N*/           SwClientIter aIter( *(SwModify*)pDoc->GetDfltGrfFmtColl() );
/*N*/           for( SwCntntNode* pNd = (SwCntntNode*)aIter.First( TYPE( SwCntntNode ) );
/*N*/                   pNd; pNd = (SwCntntNode*)aIter.Next() )
/*N*/           {
/*N*/               SwOLENode* pOLENd = pNd->GetOLENode();
/*N*/               if( pOLENd )
/*N*/               {
/*N*/                   const String& rOLEName = pOLENd->GetOLEObj().GetName();
/*N*/                   for( n = aInfoList.size(); n; )
/*N*/                   {
/*N*/                       const String& rName = aInfoList[ --n ].GetName();
/*N*/                       if( rOLEName == rName )
/*N*/                       {
                                SvStorageInfoList::iterator it = aInfoList.begin();
                                ::std::advance( it, n );
                                aInfoList.erase( it );
/*N*/                           break;
/*N*/                       }
/*N*/                   }
/*N*/               }
/*N*/           }
            // und jetzt haben wir alle Objecte, die nicht mehr
            // referenziert werden
/*N*/           SvPersist* p = this;
/*N*/           for( n = aInfoList.size(); n; )
/*N*/           {
/*?*/               const String& rName = aInfoList[ --n ].GetName();
/*?*/               SvInfoObjectRef aRef( p->Find( rName ) );
/*?*/               if( aRef.Is() )
/*?*/                   p->Remove( &aRef );
/*N*/           }
/*N*/       }
/*N*/   }
/*N*/   return bRet;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


/*?*/ sal_Bool  SwDocShell::LoadFrom(SvStorage* pStor)
/*?*/ {
/*?*/   RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDocShell::LoadFrom" );
/*?*/   sal_Bool bRet = sal_False;
/*?*/   if( pDoc )
/*?*/       RemoveLink();
/*?*/
/*?*/   AddLink();      // Link setzen und Daten updaten !!
/*?*/
/*?*/   do {        // middle check loop
/*?*/       sal_uInt32 nErr = ERR_SWG_READ_ERROR;
/*?*/       const String& rNm = pStor->GetName();
/*?*/       String aStreamName;
/*?*/       sal_Bool bXML = pStor->GetVersion() >= SOFFICE_FILEFORMAT_60;
/*?*/       if( bXML )
/*?*/           aStreamName = String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM("styles.xml"));
/*?*/       else
/*?*/           aStreamName = SfxStyleSheetBasePool::GetStreamName();
/*?*/       if( pStor->IsStream( aStreamName ) )
/*?*/       {
/*?*/           // Das Laden
/*?*/           if( bXML )
/*?*/           {
                            OSL_ASSERT("ReadXML removed");
/*?*/           }
/*?*/       }
/*?*/       else
/*?*/       {
/*?*/           // sollte es sich um eine 2. Vrolage handeln ??
/*?*/           if( SotStorage::IsStorageFile( rNm ) )
/*?*/               break;
/*?*/
/*?*/           const SfxFilter* pFltr = SwIoSystem::GetFileFilter(rNm);
/*?*/           if( !pFltr || !pFltr->GetUserData().EqualsAscii( FILTER_SWG ))
/*?*/               break;
/*?*/       }
/*?*/
/*?*/       if( !bXML )
/*?*/       {
/*?*/           OSL_ENSURE( !pBasePool, "wer hat seinen Pool nicht zerstoert?" );
/*?*/           pBasePool = new SwDocStyleSheetPool( *pDoc,
/*?*/                               SFX_CREATE_MODE_ORGANIZER == GetCreateMode() );
/*?*/       }
/*?*/
/*?*/       SetError( nErr );
/*?*/       bRet = !IsError( nErr );
/*?*/
/*?*/   } while( sal_False );
/*?*/
/*?*/   SfxObjectShell::LoadFrom( pStor );
/*?*/   pDoc->ResetModified();
/*?*/   return bRet;
/*?*/ }


/*M*/ void SwDocShell::SubInitNew()
/*M*/ {
/*M*/   OSL_ENSURE( !pBasePool, "wer hat seinen Pool nicht zerstoert?" );
/*M*/   pBasePool = new SwDocStyleSheetPool( *pDoc,
/*M*/                   SFX_CREATE_MODE_ORGANIZER == GetCreateMode() );
/*M*/   UpdateFontList();
/*M*/   InitDraw();
/*M*/
/*M*/     pDoc->SetLinkUpdMode( GLOBALSETTING );
/*M*/   pDoc->SetFldUpdateFlags( AUTOUPD_GLOBALSETTING );
/*M*/
/*M*/   sal_Bool bWeb = ISA(SwWebDocShell);
/*M*/
/*M*/   sal_uInt16 nRange[] =   {
/*N*/         RES_PARATR_ADJUST, RES_PARATR_ADJUST,
/*M*/                           RES_CHRATR_COLOR, RES_CHRATR_COLOR,
/*M*/                           RES_CHRATR_LANGUAGE, RES_CHRATR_LANGUAGE,
/*M*/                             RES_CHRATR_CJK_LANGUAGE, RES_CHRATR_CJK_LANGUAGE,
/*M*/                             RES_CHRATR_CTL_LANGUAGE, RES_CHRATR_CTL_LANGUAGE,
/*M*/                           0, 0, 0  };
/*M*/   if(!bWeb)
/*M*/   {
/*M*/         nRange[ SAL_N_ELEMENTS(nRange) - 3 ] = RES_PARATR_TABSTOP;
/*M*/         nRange[ SAL_N_ELEMENTS(nRange) - 2 ] = RES_PARATR_HYPHENZONE;
/*M*/   }
/*M*/   SfxItemSet aDfltSet( pDoc->GetAttrPool(), nRange );
/*M*/
/*M*/     //! get lingu options without loading lingu DLL
/*M*/     SvtLinguOptions aLinguOpt;
/*N*/
/*N*/   // #107253# Replaced SvtLinguConfig with SwLinguConfig wrapper with UsageCount
/*N*/     SwLinguConfig().GetOptions( aLinguOpt );
/*M*/
/*M*/     sal_Int16   nVal = aLinguOpt.nDefaultLanguage,
/*M*/                 eCJK = aLinguOpt.nDefaultLanguage_CJK,
/*M*/                 eCTL = aLinguOpt.nDefaultLanguage_CTL;
/*M*/   aDfltSet.Put( SvxLanguageItem( nVal, RES_CHRATR_LANGUAGE ) );
/*M*/     aDfltSet.Put( SvxLanguageItem( eCJK, RES_CHRATR_CJK_LANGUAGE ) );
/*M*/     aDfltSet.Put( SvxLanguageItem( eCTL, RES_CHRATR_CTL_LANGUAGE ) );
/*M*/
/*M*/     if(!bWeb)
/*M*/   {
/*M*/       SvxHyphenZoneItem aHyp( (SvxHyphenZoneItem&) pDoc->GetDefault(
/*M*/                                                       RES_PARATR_HYPHENZONE) );
/*M*/         aHyp.GetMinLead()   = aLinguOpt.nHyphMinLeading;
/*M*/         aHyp.GetMinTrail()  = aLinguOpt.nHyphMinTrailing;
/*M*/
/*M*/       aDfltSet.Put( aHyp );
/*M*/
/*M*/       sal_uInt16 nNewPos = SW_MOD()->GetUsrPref(FALSE)->GetDefTab();
/*M*/       if( nNewPos )
/*M*/           aDfltSet.Put( SvxTabStopItem( 1, nNewPos,
/*M*/                                           SVX_TAB_ADJUST_DEFAULT ) );
/*M*/   }
/*M*/     aDfltSet.Put( SvxColorItem( Color( COL_AUTO ), RES_CHRATR_COLOR ) );
/*N*/
/*M*/   pDoc->SetDefault( aDfltSet );
/*M*/   pDoc->ResetModified();
/*M*/ }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
