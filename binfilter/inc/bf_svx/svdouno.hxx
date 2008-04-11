/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: svdouno.hxx,v $
 * $Revision: 1.5 $
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

#ifndef _SVDOUNO_HXX
#define _SVDOUNO_HXX

#ifndef _COM_SUN_STAR_AWT_XCONTROLMODEL_HPP_
#include <com/sun/star/awt/XControlModel.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROL_HPP_
#include <com/sun/star/awt/XControl.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _SVDORECT_HXX
#include <bf_svx/svdorect.hxx>
#endif
namespace binfilter {

//************************************************************
//   Vorausdeklarationen
//************************************************************

class SdrView;
class SdrControlEventListenerImpl;

//************************************************************
//   SdrUnoObj
//************************************************************

class SdrUnoObj : public SdrRectObj
{
    friend class                SdrPageView;
    friend class                SdrControlEventListenerImpl;

    SdrControlEventListenerImpl*    pEventListener;

    String                      aUnoControlModelTypeName;
    String                      aUnoControlTypeName;
    BOOL                        bOwnUnoControlModel;

protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > xUnoControlModel; // kann auch von aussen gesetzt werden

private:

public:
    TYPEINFO();

//  UNICODE: SdrUnoObj(BOOL bOwnsModel = TRUE, const String& rModelName = "");
    SdrUnoObj(const String& rModelName, BOOL bOwnsModel = TRUE);
    SdrUnoObj(const String& rModelName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxSFac,
        BOOL bOwnsModel = TRUE);
    virtual ~SdrUnoObj();

    virtual void SetPage(SdrPage* pNewPage);
    virtual void SetModel(SdrModel* pModel);



    virtual void NbcMove(const Size& rSize);
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    virtual void NbcSetSnapRect(const Rectangle& rRect);
    virtual void NbcSetLogicRect(const Rectangle& rRect);
    virtual void NbcSetLayer(SdrLayerID nLayer);

    virtual void WriteData(SvStream& rOut) const;
    virtual void ReadData(const SdrObjIOHeader& rHead, SvStream& rIn);


    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > GetUnoControlModel() const {return xUnoControlModel;}
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > GetUnoControl(const OutputDevice* pOut) const;

    const String& GetUnoControlModelTypeName() const { return aUnoControlTypeName; }
    const String& GetUnoControlTypeName() const { return aUnoControlTypeName; }

    void SetUnoControlModel(::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > xModel);
    void VisAreaChanged(const OutputDevice* pOut=NULL);
};

}//end of namespace binfilter
#endif          // _SVDOUNO_HXX

