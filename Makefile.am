## -*- mode: make; tab-width: 4 -*-
## $Id$

AUTOMAKE_OPTIONS = foreign

API_SUBDIRS = api lib apps zip

if ENABLE_SERVER
   SERVER_SUBDIRS = db tools test py sched
endif

if ENABLE_CLIENT
   CLIENT_SUBDIRS = client 
endif
if BUILD_CLIENTGUI
   CLIENTGUI_SUBDIRS = clientgui
endif

SUBDIRS = m4 RSAEuro $(SERVER_SUBDIRS) $(CLIENT_SUBDIRS) $(CLIENTGUI_SUBDIRS) $(API_SUBDIRS)

# Putting a directory name recursively copies the entire contents - the
# dist-hook below gets rid of CVS directories.

EXTRA_DIST =                                    \
	win_build                                   \
	doc                                         \
	stripchart                                  \
    INSTALL										


# KARL -- please remove this!!  BA
#
# UNIX2DOS=unix2dos
# UNIX2DOSDIR = /disks/philmor/a/users/quarl/bin 
# UNIX2DOS = /disks/philmor/a/users/quarl/bin/unix2dos

dist-hook:
	rm -rf `find $(distdir) -name .backup -prune -o -name '*~' -o -name CVS`
#	cd $(distdir) && ./_autosetup

# This is a hack!
client-bin:
	cd client && make client-bin client-bin-gz
