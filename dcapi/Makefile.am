ACLOCAL_AMFLAGS = -I cf
AUTOMAKE_OPTIONS = foreign 1.7

SUBDIRS = include common

if WITH_BOINC
SUBDIRS += boinc
endif

if WITH_CLGR
SUBDIRS += clgr
endif

if WITH_LOCAL
SUBDIRS += local
endif
