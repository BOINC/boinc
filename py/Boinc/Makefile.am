## $Id$

include $(top_srcdir)/Makefile.incl

EXTRA_DIST = boinc.py boinc_db.py version.py.in db_def_to_py

$(srcdir)/boinc_db.py: $(top_srcdir)/db/boinc_db.h $(top_srcdir)/lib/result_state.h
	cat $^ | $(srcdir)/db_def_to_py > $@

# all: $(srcdir)/boinc_db.py
