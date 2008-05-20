/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: saveopt.hxx,v $
 * $Revision: 1.4 $
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
#ifndef INCLUDED_SVTOOLS_SAVEOPT_HXX
#define INCLUDED_SVTOOLS_SAVEOPT_HXX

#include <sal/types.h>
#include <bf_svtools/options.hxx>

namespace binfilter
{

struct SvtLoadSaveOptions_Impl;
class  SvtSaveOptions: public Options
{
    SvtLoadSaveOptions_Impl*    pImp;

public:

    enum EOption
    {
        E_AUTOSAVETIME,
        E_USEUSERDATA,
        E_BACKUP,
        E_AUTOSAVE,
        E_AUTOSAVEPROMPT,
        E_DOCINFSAVE,
        E_SAVEWORKINGSET,
        E_SAVEDOCWINS,
        E_SAVEDOCVIEW,
        E_SAVERELINET,
        E_SAVERELFSYS,
        E_SAVEUNPACKED,
        E_DOPRETTYPRINTING,
        E_WARNALIENFORMAT,
        E_LOADDOCPRINTER
    };
                            SvtSaveOptions();
                            virtual ~SvtSaveOptions();

    sal_Bool                IsBackup() const;

    sal_Bool                IsSaveDocView() const;

    sal_Bool                IsSaveRelINet() const;

    sal_Bool                IsSaveRelFSys() const;

    sal_Bool                IsSaveUnpacked() const;

    sal_Bool                IsLoadUserSettings() const;

    sal_Bool                IsPrettyPrinting( ) const;
};

}
#endif

