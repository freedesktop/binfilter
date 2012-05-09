#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#***********************************************************************/

PRJ = ../..
PRJNAME = binfilter
TARGET = test_binfilter

ENABLE_EXCEPTIONS = TRUE
VISIBILITY_HIDDEN = TRUE

.INCLUDE: settings.mk

CFLAGSCXX += $(CPPUNIT_CFLAGS)

.IF "$(OS)" == "IOS"
CFLAGSCXX += -DCPPUNIT_PLUGIN_EXPORTED_NAME=cppunitTest_$(TARGET)
.ENDIF

SHL1IMPLIB = i$(SHL1TARGET)
SHL1OBJS = $(SLO)/test_binfilter.obj
SHL1RPATH = NONE
SHL1STDLIBS = $(TESTLIB) $(UNOTESTLIB) $(CPPULIB) $(CPPUNITLIB) $(SALLIB)
SHL1TARGET = test_binfilter
SHL1USE_EXPORTS = name
DEF1NAME = $(SHL1TARGET)

SLOFILES = $(SHL1OBJS)

.INCLUDE: target.mk

.IF "$(OS)" == "WNT"
my_file = file:///
.ELSE
my_file = file://
.END

ALLTAR: test

test_components = \
    component/framework/util/fwk \
    component/sfx2/util/sfx \
    ucb1 \
    ucpfile1

#Make a services.rdb with the services we know we need to get up and running
$(MISC)/$(TARGET)/services.input : makefile.mk
    $(MKDIRHIER) $(@:d)
    echo \
        '<list>$(test_components:^"<filename>":+".component</filename>")</list>' \
        > $@

$(MISC)/$(TARGET)/services.rdb .ERRREMOVE : makefile.mk $(MISC)/$(TARGET)/services.input
    $(MKDIRHIER) $(@:d)
    $(XSLTPROC) --nonet --stringparam prefix $(SOLARXMLDIR)/ -o $@.tmp \
        $(SOLARENV)/bin/packcomponents.xslt $(MISC)/$(TARGET)/services.input
    cat $(MISC)/$@.tmp | sed 's|/program/|/|g' | sed 's|BRAND_BASE_DIR|SOLVER_DIR|g' > $@

test_owncomponents = \
    bf_migratefilter \
    bf_wrapper

$(MISC)/$(TARGET)/ownservices.input : makefile.mk
    $(MKDIRHIER) $(@:d)
    echo \
        '<list>$(test_owncomponents:^"<filename>":+".component</filename>")</list>' \
        > $@

$(MISC)/$(TARGET)/ownservices.rdb .ERRREMOVE : makefile.mk $(MISC)/$(TARGET)/ownservices.input
    $(MKDIRHIER) $(@:d)
    $(XSLTPROC) --nonet --stringparam prefix $(PWD)/$(MISC)/ -o $@.tmp \
        $(SOLARENV)/bin/packcomponents.xslt $(MISC)/$(TARGET)/ownservices.input
    cat $(MISC)/$@.tmp | sed 's|/program/|/|g' > $@

$(DLLDEST)/legacy_binfilters.rdb .ERRREMOVE : makefile.mk $(MISC)/legacy_binfilters.rdb
    $(MKDIRHIER) $(@:d)
    cat $(MISC)/legacy_binfilters.rdb | sed 's|/program/|/|g' > $@

#Tweak things so that we use the .res files in the solver
STAR_RESOURCEPATH:=$(PWD)/$(BIN)$(PATH_SEPERATOR)$(SOLARBINDIR)
.EXPORT : STAR_RESOURCEPATH

test .PHONY: $(SHL1TARGETN) $(MISC)/$(TARGET)/services.rdb $(MISC)/$(TARGET)/ownservices.rdb $(DLLDEST)/legacy_binfilters.rdb
    @echo ----------------------------------------------------------
    @echo - start unit test \#1 on library $(SHL1TARGETN)
    @echo ----------------------------------------------------------
    $(CPPUNITTESTER) $(SHL1TARGETN) --headless --invisible \
        '-env:UNO_TYPES=$(my_file)$(SOLARBINDIR)/udkapi.rdb $(my_file)$(SOLARBINDIR)$/types.rdb' \
        '-env:UNO_SERVICES=$(my_file)$(SOLARXMLDIR)/ure/services.rdb $(my_file)$(PWD)/$(MISC)/$(TARGET)/services.rdb $(my_file)$(PWD)/$(MISC)/$(TARGET)/ownservices.rdb'\
        -env:URE_INTERNAL_LIB_DIR="$(my_file)$(SOLARSHAREDBIN)" \
        -env:SOLVER_DIR="$(my_file)$(SOLARSHAREDBIN)" \
        -env:BRAND_BASE_DIR="$(my_file)$(PWD)/$(DLLDEST)"
