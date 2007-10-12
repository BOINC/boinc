## -*- mode: make; tab-width: 4 -*-
## $Id$

include $(top_srcdir)/Makefile.incl

if ENABLE_CLIENT
  CLIENT_MANS = boinc_client.1 boinc_cmd.1
endif

if BUILD_CLIENTGUI
  CLIENTGUI_MANS = boincmgr.1
endif

if ENABLE_SERVER
  SERVER_MANS = 
endif

man_MANS = $(CLIENT_MANS) $(CLIENTGUI_MANS) $(SERVER_MANS)

%.1: %.xml
	$(DOCBOOK2X_MAN) $<

all-local: $(man_MANS)

CLEANFILES = $(man_MANS)
