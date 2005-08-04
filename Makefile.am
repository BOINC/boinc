## -*- mode: make; tab-width: 4 -*-
## $Id$

AUTOMAKE_OPTIONS = foreign

## make sure rebuilding uses the right aclocal-switches
ACLOCAL_AMFLAGS = -I m4

API_SUBDIRS = api lib zip

if ENABLE_SERVER
   SERVER_SUBDIRS = db tools test py sched apps
endif

if ENABLE_CLIENT
   CLIENT_SUBDIRS = client 
endif

if BUILD_CLIENTGUI
   CLIENTGUI_SUBDIRS = clientgui sea
endif

# ORDER MATTERS below.  One must build dependencies FIRST, then things
# that depend on them later
SUBDIRS = m4 $(API_SUBDIRS) $(SERVER_SUBDIRS) $(CLIENT_SUBDIRS) $(CLIENTGUI_SUBDIRS) 

# Putting a directory name recursively copies the entire contents - the
# dist-hook below gets rid of CVS directories.

EXTRA_DIST =                                    \
	win_build                                   \
	doc                                         \
	stripchart                                  \
    INSTALL										


dist-hook:
	rm -rf `find $(distdir) -name .backup -prune -o -name '*~' -o -name CVS`
#	cd $(distdir) && ./_autosetup

# convenience target
client-bin:
	cd client && make client-bin client-bin-gz
