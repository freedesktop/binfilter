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

#ifndef _SVX_FMPAGE_HXX
#define _SVX_FMPAGE_HXX

#include <bf_svtools/bf_solar.h>

#include <bf_svx/svdpage.hxx>

namespace com { namespace sun { namespace star {
    namespace container {
        class XNameContainer;
    }
}}}

namespace binfilter {

class StarBASIC;
class FmFormModel;
class FmFormPageImpl;   // haelt die Liste aller Forms

class SfxJSArray;
class HelpEvent;

class FmFormPage : public SdrPage
{
    friend class FmFormObj;
    FmFormPageImpl* pImpl;

public:
    TYPEINFO();

    FmFormPage(FmFormModel& rModel,StarBASIC*, bool bMasterPage=sal_False);
    ~FmFormPage();

    using SdrPage::NbcInsertObject;
    using SdrPage::ReplaceObject;

    virtual void    ReadData(const SdrIOHeader& rHead, SvStream& rIn);
    virtual void    SetModel(SdrModel* pNewModel);

    virtual SdrPage* Clone() const;

    virtual void    InsertObject(SdrObject* pObj, ULONG nPos = CONTAINER_APPEND,
                                    const SdrInsertReason* pReason=NULL);

    virtual SdrObject* RemoveObject(ULONG nObjNum);

    // Zugriff auf alle Formulare
    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer>& GetForms() const;

    FmFormPageImpl*  GetImpl() const {return pImpl;}

public:
    String              aPageName;
    StarBASIC*          pBasic;

    const String&       GetName() const { return aPageName; }
    void                SetName( const String& rName ) { aPageName = rName; }
    StarBASIC*          GetBasic() const { return pBasic; }
};

}//end of namespace binfilter
#endif          // _SVX_FMPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
