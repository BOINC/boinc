## $Id$

AUTOMAKE_OPTIONS = foreign

SUBDIRS = RSAEuro db lib api apps client sched tools test py

# Putting a directory name recursively copies the entire contents - the
# dist-hook below gets rid of CVS directories.

EXTRA_DIST =								\
	mac_build win_build						\
	doc html_ops html_user						\
	stripchart							\
        INSTALL

py html_user html_ops:
	[ "$srcdir" != . ] && ln -s $srcdir/$@ $@

all-local: html_user html_ops

UNIX2DOS = /disks/philmor/a/users/quarl/bin/unix2dos

dist-hook:
	rm -rf `find $(distdir) -name CVS -prune`
	rm -rf `find $(distdir) -name .backup -prune -o -name *~`
	cd $(distdir)/win_build && $(UNIX2DOS) boinc.dsw *.dsp

client-bin:
	cd client && make client-bin client-bin-gz
