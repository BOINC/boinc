## -*- mode: makefile; tab-width: 4 -*-
## $Id$

AUTOMAKE_OPTIONS = foreign

include Makefile.incl

## make sure rebuilding uses the right aclocal-switches
ACLOCAL_AMFLAGS = -I m4

if ENABLE_LIBRARIES
    API_SUBDIRS = api lib
if ENABLE_SERVER
    API_SUBDIRS += zip
endif
## boinc_zip needed for wrapper
if ENABLE_APPS
    API_SUBDIRS += zip
endif
if ENABLE_BOINCZIP
    API_SUBDIRS += zip
endif
endif

if ENABLE_SERVER
   SERVER_SUBDIRS = db py sched apps tools vda html
## once contained samples/example_app which breaks out-of-source-tree builds
endif

if ENABLE_CLIENT
   CLIENT_SUBDIRS = client client/scripts
endif

if ENABLE_MANAGER
   CLIENTGUI_SUBDIRS = clientgui locale clientgui/res clientgui/skins
if BUILD_X11_SCREENSAVER
   CLIENTGUI_SUBDIRS += clientscr
endif
endif

if ENABLE_APPS
   APPS_SUBDIRS = samples
endif

# ORDER MATTERS below.  One must build dependencies FIRST, then things
# that depend on them later
SUBDIRS = m4 $(API_SUBDIRS) $(SERVER_SUBDIRS) $(CLIENT_SUBDIRS) $(CLIENTGUI_SUBDIRS) $(APPS_SUBDIRS) doc

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
    svn_version.h \
    project_specific_defines.h
endif

# Define files for distclean command
DISTCLEANFILES =                                \
   clientgui/res/Makefile                       \
   packages/generic/sea/Makefile

# Define files for maintainer-clean command
MAINTAINERCLEANFILES =                          \
    config.h.in                                 \
    Makefile.in                                 \
    aclocal.m4                                  \
    autom4te.cache/*                            \
    compile                                     \
    config.guess                                \
    config.sub                                  \
    configure                                   \
    depcomp                                     \
    install-sh                                  \
    ltmain.sh                                   \
    m4/libtool.m4                               \
    m4/ltoptions.m4                             \
    m4/ltsugar.m4                               \
    m4/ltversion.m4                             \
    m4/lt~obsolete.m4                           \
    missing                                     \
    test-driver

# svn_version.h should always be rebuilt.
BUILT_SOURCES = svn_version.h
svn_version.h: generate_svn_version.sh
	cd $(srcdir) && sh generate_svn_version.sh
.PHONY: svn_version.h

# Add a stage target for staging a distribution

clean-generic:
	rm -rf stage *.gcov $(BUILT_SOURCES)

stage: all
	if [ ! -d stage ] ; then mkdir stage ; fi
	rm -rf stage/*
	$(MAKE) DESTDIR=$(PWD)/stage install

dist-hook:
	rm -rf `find $(distdir) -name .backup -prune -o -name '*~' -o -name CVS -name Makefile`
#	cd $(distdir) && ./_autosetup

distclean-local:
	cd packages/solaris/CSW && make distclean
