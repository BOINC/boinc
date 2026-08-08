
include $(top_srcdir)/Makefile.incl

copyright: $(top_builddir)/COPYRIGHT $(top_builddir)/COPYING
	cat $(top_builddir)/COPYRIGHT $(top_builddir)/COPYING > copyright

$(top_builddir)/stage/etc/init.d/cswboinc-client: $(top_builddir)/stage/etc/init.d/boinc-client
	cp $(top_builddir)/stage/etc/init.d/boinc-client $(top_builddir)/stage/etc/init.d/cswboinc-client

$(top_builddir)/stage/etc/default/boinc-client: boinc-client.conf
	$(INSTALL) -d $(top_builddir)/stage/etc/default
	$(INSTALL) boinc-client.conf $(top_builddir)/stage/etc/default/boinc-client

$(top_builddir)/stage/$(mandir)/man1/boinc_client.1: boinc_client.1
	$(INSTALL) -d $(top_builddir)/stage/$(mandir)/man1
	$(INSTALL) boinc_client.1 $(top_builddir)/stage/$(mandir)/man1/boinc_client.1

$(top_builddir)/stage/$(mandir)/man1/boinc.1: boinc.1
	$(INSTALL) -d $(top_builddir)/stage/$(mandir)/man1
	$(INSTALL) boinc.1 $(top_builddir)/stage/$(mandir)/man1/boinc.1

$(top_builddir)/stage/$(mandir)/man1/boinccmd.1: boinccmd.1
	$(INSTALL) -d $(top_builddir)/stage/$(mandir)/man1
	$(INSTALL) boinccmd.1 $(top_builddir)/stage/$(mandir)/man1/boinccmd.1

all: copyright $(top_builddir)/stage/etc/init.d/cswboinc-client $(top_builddir)/stage/etc/default/boinc-client $(top_builddir)/stage/$(mandir)/man1/boinc_client.1 $(top_builddir)/stage/$(mandir)/man1/boinc.1 $(top_builddir)/stage/$(mandir)/man1/boinccmd.1

