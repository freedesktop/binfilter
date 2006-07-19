#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: kz $ $Date: 2006-07-19 14:54:05 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

EXTERNAL_WARNINGS_NOT_ERRORS := TRUE

PRJ=..$/..$/..$/..
BFPRJ=..$/..$/..

PRJNAME=binfilter
TARGET=sw_text

#GEN_HID=TRUE
#GEN_HID_OTHER=TRUE
NO_HIDS=TRUE

AUTOSEG=true

# --- Settings -----------------------------------------------------

#.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  $(PRJ)$/inc$/bf_sw$/swpre.mk
.INCLUDE :  settings.mk
#.INCLUDE :  $(PRJ)$/inc$/sw.mk
.INCLUDE :  $(PRJ)$/inc$/bf_sw$/sw.mk
INC+= -I$(PRJ)$/inc$/bf_sw
INCEXT=s:\solar\inc\hm

.IF "$(mydebug)" != ""
CDEFS+=-Dmydebug
.ENDIF

.IF "$(GUI)$(COM)" == "WINMSC"
LIBFLAGS=/NOI /NOE /PAGE:512
.ENDIF

# --- Files --------------------------------------------------------

CXXFILES = \
                sw_atrstck.cxx \
                sw_frmcrsr.cxx \
        sw_frmform.cxx \
        sw_frminf.cxx \
        sw_frmpaint.cxx \
        sw_guess.cxx \
        sw_inftxt.cxx \
        sw_itradj.cxx \
        sw_itratr.cxx \
        sw_itrcrsr.cxx \
        sw_itrform2.cxx \
        sw_itrpaint.cxx \
        sw_itrtxt.cxx \
        sw_porexp.cxx \
        sw_porfld.cxx \
        sw_porfly.cxx \
        sw_porglue.cxx \
        sw_porlay.cxx \
        sw_porlin.cxx \
        sw_pormulti.cxx \
        sw_porref.cxx \
        sw_porrst.cxx \
        sw_portox.cxx \
        sw_portxt.cxx \
        sw_redlnitr.cxx \
        sw_txtcache.cxx \
        sw_txtdrop.cxx \
        sw_txtfld.cxx \
        sw_txtfly.cxx \
        sw_txtfrm.cxx \
        sw_txtftn.cxx \
        sw_txthyph.cxx \
        sw_txtinit.cxx \
        sw_txtpaint.cxx \
        sw_txttab.cxx \
        sw_widorp.cxx \
        sw_blink.cxx \
        sw_noteurl.cxx \
        sw_wrong.cxx

.IF "$(product)$(cap)" == ""
CXXFILES += \
        sw_txtio.cxx
.ENDIF



SLOFILES =  \
                $(SLO)$/sw_atrstck.obj \
        $(SLO)$/sw_frmcrsr.obj \
        $(SLO)$/sw_frmform.obj \
        $(SLO)$/sw_frminf.obj \
        $(SLO)$/sw_frmpaint.obj \
        $(SLO)$/sw_guess.obj \
        $(SLO)$/sw_inftxt.obj \
        $(SLO)$/sw_itradj.obj \
        $(SLO)$/sw_itratr.obj \
        $(SLO)$/sw_itrcrsr.obj \
        $(SLO)$/sw_itrform2.obj \
        $(SLO)$/sw_itrpaint.obj \
        $(SLO)$/sw_itrtxt.obj \
        $(SLO)$/sw_porexp.obj \
        $(SLO)$/sw_porfld.obj \
        $(SLO)$/sw_porfly.obj \
        $(SLO)$/sw_porglue.obj \
        $(SLO)$/sw_porlay.obj \
        $(SLO)$/sw_porlin.obj \
        $(SLO)$/sw_pormulti.obj \
        $(SLO)$/sw_porref.obj \
        $(SLO)$/sw_porrst.obj \
        $(SLO)$/sw_portox.obj \
        $(SLO)$/sw_portxt.obj \
        $(SLO)$/sw_redlnitr.obj \
        $(SLO)$/sw_txtcache.obj \
        $(SLO)$/sw_txtdrop.obj \
        $(SLO)$/sw_txtfld.obj \
        $(SLO)$/sw_txtfly.obj \
        $(SLO)$/sw_txtfrm.obj \
        $(SLO)$/sw_txtftn.obj \
        $(SLO)$/sw_txthyph.obj \
        $(SLO)$/sw_txtinit.obj \
        $(SLO)$/sw_txtpaint.obj \
        $(SLO)$/sw_txttab.obj \
        $(SLO)$/sw_widorp.obj \
        $(SLO)$/sw_blink.obj \
        $(SLO)$/sw_noteurl.obj \
        $(SLO)$/sw_wrong.obj

.IF "$(product)$(cap)" == ""
SLOFILES +=  \
        $(SLO)$/sw_txtio.obj
.ENDIF

.IF "$(CPUNAME)" == "SPARC"
.IF "$(OS)" == "NETBSD"
NOOPTFILES = \
    $(SLO)$/sw_txtftn.obj
.ENDIF
.ENDIF

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

