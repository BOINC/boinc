## -*- mode: makefile; tab-width: 4 -*-
## $Id$

include $(top_srcdir)/Makefile.incl

EXTRA_DIST = db_def_to_py				\
	__init__.py					\
	add_util.py					\
	boinc_db.py					\
	boinc_project_path.py				\
	boincxml.py					\
	configxml.py					\
	database.py					\
	db_base.py					\
	db_mid.py					\
	projectxml.py					\
	sched_messages.py				\
	setup_project.py				\
	tools.py					\
	util.py

$(srcdir)/boinc_db.py: $(top_srcdir)/db/boinc_db.h $(top_srcdir)/lib/result_state.h
	cat $^ | $(srcdir)/db_def_to_py > $@

# all: $(srcdir)/boinc_db.py
