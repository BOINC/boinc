## -*- mode: makefile; tab-width: 4 -*-

## $Id: Makefile.am 18938 2009-08-28 18:25:26Z davea $

include $(top_srcdir)/Makefile.incl

if ENABLE_CLIENT
  CLIENT_MANS = boinc.1 boinccmd.1
endif

if ENABLE_MANAGER
  CLIENTGUI_MANS = boincmgr.1
endif

if ENABLE_SERVER
  SERVER_MANS = appmgr.8
endif

man_MANS = $(CLIENT_MANS) $(CLIENTGUI_MANS) $(SERVER_MANS)

SUFFIXES = .xml .1 .8

.xml.1:
	$(DOCBOOK2X_MAN) $<

.xml.8:
	$(DOCBOOK2X_MAN) $<

all-local: $(man_MANS)

CLEANFILES = $(man_MANS)
