## -*- mode: makefile; tab-width: 4 -*-
## $Id$

AUTOMAKE_OPTIONS = foreign

include Makefile.incl

## make sure rebuilding uses the right aclocal-switches
ACLOCAL_AMFLAGS = -I m4

if ENABLE_LIBRARIES
   API_SUBDIRS = api lib zip
endif

if ENABLE_SERVER
   SERVER_SUBDIRS = db test py sched apps tools samples/example_app vda
endif

if ENABLE_CLIENT
   CLIENT_SUBDIRS = client client/scripts
endif

if ENABLE_MANAGER
   CLIENTGUI_SUBDIRS = clientgui clientgui/res locale
#if BUILD_X11_SCREENSAVER
#   CLIENTGUI_SUBDIRS += clientscr
#endif
endif

# ORDER MATTERS below.  One must build dependencies FIRST, then things
# that depend on them later
SUBDIRS = m4 $(API_SUBDIRS) $(SERVER_SUBDIRS) $(CLIENT_SUBDIRS) $(CLIENTGUI_SUBDIRS) doc

# Putting a directory name recursively copies the entire contents - the
# dist-hook below gets rid of CVS directories.

EXTRA_DIST =                                    \
	win_build                                   \
	doc                                         \
	stripchart                                  \
    INSTALL										

if INSTALL_HEADERS
pkginclude_HEADERS = \
    version.h \
    svn_version.h
endif

# svn_version.h should always be rebuilt.

BUILT_SOURCES = svn_version.h
svn_version.h: generate_svn_version.sh
	cd $(srcdir) && sh generate_svn_version.sh
.PHONY: svn_version.h

# Add a stage target for staging a distribution

clean-generic:
	rm -rf stage

stage: all
	if [ ! -d stage ] ; then mkdir stage ; fi
	rm -rf stage/*
	$(MAKE) DESTDIR=$(PWD)/stage install

dist-hook:
	rm -rf `find $(distdir) -name .backup -prune -o -name '*~' -o -name CVS`
#	cd $(distdir) && ./_autosetup
