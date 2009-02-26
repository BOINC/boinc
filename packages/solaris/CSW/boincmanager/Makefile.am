
include $(top_srcdir)/Makefile.incl

copyright: $(top_builddir)/COPYRIGHT $(top_builddir)/COPYING
	cat $(top_builddir)/COPYRIGHT $(top_builddir)/COPYING > copyright

$(top_builddir)/stage/$(mandir)/man1/boinc_manager.1: boinc_manager.1
	$(INSTALL) -d $(top_builddir)/stage/$(mandir)/man1
	$(INSTALL) boinc_manager.1 $(top_builddir)/stage/$(mandir)/man1/boinc_manager.1  

$(top_builddir)/stage/$(mandir)/man1/boincmgr.1: boincmgr.1
	$(INSTALL) -d $(top_builddir)/stage/$(mandir)/man1
	$(INSTALL) boincmgr.1 $(top_builddir)/stage/$(mandir)/man1/boincmgr.1  

all: copyright $(top_builddir)/stage/$(mandir)/man1/boinc_manager.1 $(top_builddir)/stage/$(mandir)/man1/boincmgr.1

