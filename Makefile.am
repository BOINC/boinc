## $Id$

AUTOMAKE_OPTIONS = foreign

SUBDIRS = RSAEuro db lib api apps client sched tools test

# Putting a directory name recursively copies the entire contents - the
# dist-hook below gets rid of CVS directories.

EXTRA_DIST =								\
	mac_build win_build						\
	doc html_ops html_user						\
	stripchart							\
        INSTALL LICENSE

dist-hook:
	rm -rf `find $(distdir) -name CVS -prune`
	rm -rf `find $(distdir) -name .backup -prune -o -name *~`
	cd $(distdir)/win_build && perl -pi.bak -e 'tr /\r//d' boinc.dsw */*.dsp
