## -*- mode: make; tab-width: 4 -*-
## $Id$

AUTOMAKE_OPTIONS = foreign

API_SUBDIRS = api lib boincglut apps

if ENABLE_SERVER
  SERVER_SUBDIRS = db tools test py sched
endif

if ENABLE_CLIENT
  CLIENT_SUBDIRS = client
endif

SUBDIRS = RSAEuro $(SERVER_SUBDIRS) $(CLIENT_SUBDIRS) $(API_SUBDIRS)

# Putting a directory name recursively copies the entire contents - the
# dist-hook below gets rid of CVS directories.

EXTRA_DIST =                                    \
	win_build                                   \
	doc                                         \
	stripchart                                  \
    INSTALL

py:
	[ "$(srcdir)" != . ] && rm -f $@ && $(LN_S) $(srcdir)/$@ $@

UNIX2DOS=unix2dos
UNIX2DOSDIR = /disks/philmor/a/users/quarl/bin

# UNIX2DOS = /disks/philmor/a/users/quarl/bin/unix2dos

dist-hook:
	rm -rf `find $(distdir) -name .backup -prune -o -name '*~'`
	PATH=$PATH:$UNIX2DOSDIR cd $(distdir)/win_build && $(UNIX2DOS) boinc.dsw *.dsp

client-bin:
	cd client && make client-bin client-bin-gz
