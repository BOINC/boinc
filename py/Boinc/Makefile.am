## $Id$

include $(top_srcdir)/Makefile.incl

EXTRA_DIST = db_def_to_py			\
	__init__.py				\
	boinc_db.py				\
	boinc_project_path.py			\
	configxml.py				\
	database.py				\
	db_base.py				\
	db_mid.py				\
	setup_project.py			\
	tools.py				\
	util.py					\
	version.py

$(srcdir)/boinc_db.py: $(top_srcdir)/db/boinc_db.h $(top_srcdir)/lib/result_state.h
	cat $^ | $(srcdir)/db_def_to_py > $@

# all: $(srcdir)/boinc_db.py
