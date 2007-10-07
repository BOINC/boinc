## -*- mode: make; tab-width: 4 -*-
## $Id$

include $(top_srcdir)/Makefile.incl

if HAVE_DOCBOOK2X_MAN
man_MANS = \
    boinc_client.1 \
    boinc_cmd.1 \
    boincmgr.1
else
man_MANS = 
endif

%.1: %.xml
	$(DOCBOOK2X_MAN) $<

all-local: $(man_MANS)

CLEANFILES = $(man_MANS)
