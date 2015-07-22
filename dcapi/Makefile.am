ACLOCAL_AMFLAGS = -I cf
AUTOMAKE_OPTIONS = foreign 1.7

SUBDIRS = include common doc

EXTRA_DIST = _autosetup.sh

if WITH_BOINC
SUBDIRS += boinc
endif

if WITH_CONDOR
SUBDIRS += condor
endif

if WITH_LOCAL
SUBDIRS += local
endif

if HAVE_JAVA
SUBDIRS += java
endif

aclocaldir = $(datadir)/aclocal
dist_aclocal_DATA = dcapi.m4
