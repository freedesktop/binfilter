/*************************************************************************
 *
 *  $RCSfile: redline.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hjs $ $Date: 2003-10-01 12:23:53 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _REDLINE_HXX
#define _REDLINE_HXX

#ifndef _DATETIME_HXX //autogen
#include <tools/datetime.hxx>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#define _SVSTDARR_USHORTS
#include <svtools/svstdarr.hxx>

#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _REDLENUM_HXX
#include <redlenum.hxx>
#endif

class SfxItemSet;

class SwRedlineExtraData
{
    SwRedlineExtraData( const SwRedlineExtraData& );
    SwRedlineExtraData& operator=( const SwRedlineExtraData& );

protected:
    SwRedlineExtraData() {}

public:
    virtual ~SwRedlineExtraData();
     virtual SwRedlineExtraData* CreateNew() const = 0;

//STRIP001  virtual void Accept( SwPaM& rPam ) const;
//STRIP001  virtual void Reject( SwPaM& rPam ) const;
    virtual int operator == ( const SwRedlineExtraData& ) const;
};

//STRIP001 class SwRedlineExtraData_FmtColl : public SwRedlineExtraData
//STRIP001 {
//STRIP001  String sFmtNm;
//STRIP001  SfxItemSet* pSet;
//STRIP001  USHORT nPoolId;
//STRIP001 public:
//STRIP001  SwRedlineExtraData_FmtColl( const String& rColl, USHORT nPoolFmtId,
//STRIP001                              const SfxItemSet* pSet = 0 );
//STRIP001  virtual ~SwRedlineExtraData_FmtColl();
//STRIP001  virtual SwRedlineExtraData* CreateNew() const;
//STRIP001  virtual void Reject( SwPaM& rPam ) const;
//STRIP001  virtual int operator == ( const SwRedlineExtraData& ) const;

//STRIP001  void SetItemSet( const SfxItemSet& rSet );
//STRIP001 };

//STRIP001 class SwRedlineExtraData_Format : public SwRedlineExtraData
//STRIP001 {
//STRIP001  SvUShorts aWhichIds;
//STRIP001
//STRIP001  SwRedlineExtraData_Format( const SwRedlineExtraData_Format& rCpy );
//STRIP001
//STRIP001 public:
//STRIP001  SwRedlineExtraData_Format( const SfxItemSet& rSet );
//STRIP001  virtual ~SwRedlineExtraData_Format();
//STRIP001  virtual SwRedlineExtraData* CreateNew() const;
//STRIP001  virtual void Reject( SwPaM& rPam ) const;
//STRIP001  virtual int operator == ( const SwRedlineExtraData& ) const;
//STRIP001 };


class SwRedlineData
{
    friend class SwRedline;
    SwRedlineData* pNext;       // Verweis auf weitere Daten
    SwRedlineExtraData* pExtraData;

    String sComment;
    DateTime aStamp;
    SwRedlineType eType;
    USHORT nAuthor, nSeqNo;

public:
    SwRedlineData( SwRedlineType eT, USHORT nAut );
    SwRedlineData( const SwRedlineData& rCpy, BOOL bCpyNext = TRUE );

    // fuer sw3io: pNext/pExtraData gehen in eigenen Besitz ueber!
    SwRedlineData( SwRedlineType eT, USHORT nAut, const DateTime& rDT,
                   const String& rCmnt, SwRedlineData* pNxt,
                    SwRedlineExtraData* pExtraData = 0 );

    ~SwRedlineData();

    int operator==( const SwRedlineData& rCmp ) const
        {
            return nAuthor == rCmp.nAuthor &&
                    eType == rCmp.eType &&
                    sComment == rCmp.sComment &&
                    (( !pNext && !rCmp.pNext ) ||
                        ( pNext && rCmp.pNext && *pNext == *rCmp.pNext )) &&
                    (( !pExtraData && !rCmp.pExtraData ) ||
                        ( pExtraData && rCmp.pExtraData &&
                            *pExtraData == *rCmp.pExtraData ));
        }
    int operator!=( const SwRedlineData& rCmp ) const
        {   return !operator==( rCmp ); }

    SwRedlineType GetType() const
        { return SwRedlineType( eType & REDLINE_NO_FLAG_MASK); }
    SwRedlineType GetRealType() const       { return eType; }
    USHORT GetAuthor() const                { return nAuthor; }
    const String& GetComment() const        { return sComment; }
    const DateTime& GetTimeStamp() const    { return aStamp; }
    inline const SwRedlineData* Next() const{ return pNext; }

    void SetTimeStamp( const DateTime& rDT)
        { aStamp = rDT; aStamp.SetSec( 0 ); aStamp.Set100Sec( 0 ); }
    void SetComment( const String& rS )     { sComment = rS; }
    void SetAutoFmtFlag()
        { eType = SwRedlineType( eType | REDLINE_FORM_AUTOFMT ); }
    int CanCombine( const SwRedlineData& rCmp ) const
        {
            return nAuthor == rCmp.nAuthor &&
                    eType == rCmp.eType &&
                    sComment == rCmp.sComment &&
                    GetTimeStamp() == rCmp.GetTimeStamp() &&
                    (( !pNext && !rCmp.pNext ) ||
                        ( pNext && rCmp.pNext &&
                        pNext->CanCombine( *rCmp.pNext ))) &&
                    (( !pExtraData && !rCmp.pExtraData ) ||
                        ( pExtraData && rCmp.pExtraData &&
                            *pExtraData == *rCmp.pExtraData ));
        }

    // ExtraData wird kopiert, der Pointer geht also NICHT in den Besitz
    // des RedlineObjectes!
    void SetExtraData( const SwRedlineExtraData* pData );
    const SwRedlineExtraData* GetExtraData() const { return pExtraData; }

    // fuers UI-seitige zusammenfassen von Redline-Actionen. Wird z.Z. nur
    // fuers Autoformat mit Redline benoetigt. Der Wert != 0 bedeutet dabei,
    // das es noch weitere geben kann!
    USHORT GetSeqNo() const                     { return nSeqNo; }
    void SetSeqNo( USHORT nNo )                 { nSeqNo = nNo; }
};


class SwRedline : public SwPaM
{
    SwRedlineData* pRedlineData;
    SwNodeIndex* pCntntSect;
    BOOL bDelLastPara : 1;
    BOOL bIsLastParaDelete : 1;
    BOOL bIsVisible : 1;

//STRIP001  void MoveToSection();
//STRIP001  void CopyToSection();
//STRIP001  void DelCopyOfSection();
//STRIP001  void MoveFromSection();

public:
    SwRedline( SwRedlineType eType, const SwPaM& rPam );
    SwRedline( SwRedlineType eTyp, const SwPosition& rPos );
    SwRedline( const SwRedlineData& rData, const SwPaM& rPam );
    SwRedline( const SwRedlineData& rData, const SwPosition& rPos );
    // fuer sw3io: pData geht in eigenen Besitz ueber!
    SwRedline(SwRedlineData* pData, const SwPosition& rPos, BOOL bVsbl,
               BOOL bDelLP, BOOL bIsPD) :
        SwPaM( rPos ), pRedlineData( pData ), pCntntSect( 0 ),
        bDelLastPara( bDelLP ), bIsLastParaDelete( bIsPD ), bIsVisible( bVsbl )
    {}
    SwRedline( const SwRedline& );
    virtual ~SwRedline();

    SwNodeIndex* GetContentIdx() const { return pCntntSect; }
    // fuers Undo
     void SetContentIdx( const SwNodeIndex* );

    BOOL IsVisible() const { return bIsVisible; }
    BOOL IsDelLastPara() const { return bDelLastPara; }
    BOOL IsLastParaDelete() const { return bIsLastParaDelete; }

    // das BOOL besagt, ob nach dem setzen der Pos kein Bereich mehr
    // aufgespannt ist. -> TRUE, ansonten Bereich und FALSE
    void SetStart( const SwPosition& rPos, SwPosition* pSttPtr = 0 )
    {
        if( !pSttPtr ) pSttPtr = Start();
        *pSttPtr = rPos;
    }
    void SetEnd( const SwPosition& rPos, SwPosition* pEndPtr = 0 )
    {
        if( !pEndPtr ) pEndPtr = End();
        *pEndPtr = rPos;
    }
    // liegt eine gueltige Selektion vor?
//STRIP001  BOOL HasValidRange() const;

    USHORT GetAuthor() const { return pRedlineData->GetAuthor(); }
    const DateTime& GetTimeStamp() const { return pRedlineData->GetTimeStamp(); }
    SwRedlineType GetType() const { return pRedlineData->GetType(); }
    SwRedlineType GetRealType() const { return pRedlineData->GetRealType(); }
    const SwRedlineData& GetRedlineData() const { return *pRedlineData; }
    int operator==( const SwRedlineData& rCmp ) const
        { return *pRedlineData == rCmp; }
    int operator!=( const SwRedlineData& rCmp ) const
        { return *pRedlineData != rCmp; }
    void SetAutoFmtFlag()               { pRedlineData->SetAutoFmtFlag(); }

//STRIP001  USHORT GetStackCount() const;
//STRIP001  USHORT GetAuthor( USHORT nPos ) const;
    const String& GetAuthorString( USHORT nPos = 0 ) const;
//STRIP001  const DateTime& GetTimeStamp( USHORT nPos ) const;
//STRIP001  SwRedlineType GetRealType( USHORT nPos ) const;
//STRIP001  SwRedlineType GetType( USHORT nPos ) const
//STRIP001      { return SwRedlineType( GetRealType( nPos ) & REDLINE_NO_FLAG_MASK); }
//STRIP001  const String& GetComment( USHORT nPos ) const;

    const String& GetComment() const    { return pRedlineData->GetComment(); }
    void SetComment( const String& rS ) { pRedlineData->SetComment( rS ); }

    // ExtraData wird kopiert, der Pointer geht also NICHT in den Besitz
    // des RedlineObjectes!
    void SetExtraData( const SwRedlineExtraData* pData )
        { pRedlineData->SetExtraData( pData ); }
    const SwRedlineExtraData* GetExtraData() const
        { return pRedlineData->GetExtraData(); }

    // fuers UI-seitige zusammenfassen von Redline-Actionen. Wird z.Z. nur
    // fuers Autoformat mit Redline benoetigt. Der Wert != 0 bedeutet dabei,
    // das es noch weitere geben kann!
    USHORT GetSeqNo() const             { return pRedlineData->GetSeqNo(); }
    void SetSeqNo( USHORT nNo )         { pRedlineData->SetSeqNo( nNo ); }

    // Beim Hide/ShowOriginal wird 2 mal ueber die Liste gelaufen, damit
    //  die Del-Redlines per Copy und Delete versteckt werden. Beim Move
    //  wird sonst die Attributierung falsch behandelt.
    // Alle anderen Aufrufer muessen immer 0 angeben.
//STRIP001  void CallDisplayFunc( USHORT nLoop = 0 );
    void Show( USHORT nLoop = 0 );
    void Hide( USHORT nLoop = 0 );
//STRIP001  void ShowOriginal( USHORT nLoop = 0 );

    // calculates the intersection with text node number nNdIdx
//STRIP001     void CalcStartEnd( USHORT nNdIdx, USHORT& nStart, USHORT& nEnd ) const;

//STRIP001     void InvalidateRange();     // das Layout anstossen

    BOOL IsOwnRedline( const SwRedline& rRedl ) const
        { return GetAuthor() == rRedl.GetAuthor(); }
    BOOL CanCombine( const SwRedline& rRedl ) const;

//STRIP001  void PushData( const SwRedline& rRedl, BOOL bOwnAsNext = TRUE );
//STRIP001  BOOL PopData();

     int operator==( const SwRedline& ) const;
    int operator<( const SwRedline& ) const;
};

#endif
