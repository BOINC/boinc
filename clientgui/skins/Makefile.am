## -*- mode: makefile; tab-width: 4 -*-

include $(top_srcdir)/Makefile.incl

datadir = $(prefix)/share/boinc-manager/skins

skins_dirs = \
    "Charity Engine" \
    "GridRepublic" \
    "People for a Smarter Planet" \
    "People for a Smarter Planet/graphic" \
    "ProgressThruProcessors" \
    "World Community Grid" \
    "Default"

install-data-hook:
	mydir=$(DESTDIR)$(datadir) ;\
	for sdir in $(skins_dirs) ; do \
	  $(INSTALL) -d "$$mydir/$$sdir" ; \
	  find "$(top_srcdir)/clientgui/skins/$$sdir" -maxdepth 1 -type f -print0 | while read -d $$'\0' file ; do \
	    $(INSTALL_DATA) "$$file" "$$mydir/$$sdir" ; \
	  done ; \
	done
