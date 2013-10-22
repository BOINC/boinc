## -*- mode: makefile; tab-width: 4 -*-
## $Id$

include $(top_srcdir)/Makefile.incl

## the following headers need to be installed
if INSTALL_HEADERS
pkginclude_HEADERS =  boinc_db.h db_base.h boinc_db_types.h
endif

## other stuff to go into the distribution
extradir = $(prefix)/share/boinc-server-maker/db
dist_extra_DATA = constraints.sql schema.sql

dist_extra_SCRIPTS = init_db
