ACLOCAL_AMFLAGS = -I cf
AUTOMAKE_OPTIONS = foreign 1.7

SUBDIRS = include common doc

if WITH_BOINC
SUBDIRS += boinc
endif

if WITH_CLGR
SUBDIRS += clgr
endif

if WITH_CONDOR
SUBDIRS += condor
endif

if WITH_LOCAL
SUBDIRS += local
endif

gtk-doc.make cf/gtk-doc.m4:
	gtkdocize --copy
