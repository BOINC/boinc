## -*- mode: make; tab-width: 4 -*-
## $Id$

AUTOMAKE_OPTIONS = foreign

SUBDIRS = RSAEuro db lib api apps client sched tools test py

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
