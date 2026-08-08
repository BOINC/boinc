## -*- mode: makefile; tab-width: 4 -*-
## $Id$

include $(top_srcdir)/Makefile.incl

EXTRA_DIST = db_def_to_py \
	Boinc/__init__.py \
	Boinc/add_util.py \
	Boinc/boinc_db.py \
	Boinc/boinc_project_path.py \
	Boinc/boincxml.py \
	Boinc/configxml.py \
	Boinc/database.py \
	Boinc/db_base.py \
	Boinc/db_mid.py	 \
	Boinc/projectxml.py \
	Boinc/sched_messages.py \
	Boinc/setup_project.py \
	Boinc/tools.py \
	Boinc/util.py

pydir=$(prefix)/usr/share/boinc-server-maker/py/Boinc
py_SCRIPTS=$(EXTRA_DIST)


$(srcdir)/Boinc/boinc_db.py: $(top_srcdir)/db/boinc_db.h $(top_srcdir)/db/boinc_db_types.h $(top_srcdir)/lib/common_defs.h
	cat $^ | $(top_srcdir)/py/db_def_to_py > $@

# all: $(srcdir)/Boinc/boinc_db.py

all-local:
	python setup.py build --build-base=$(top_builddir)/py

install-exec-local:
	python setup.py install --prefix=$(prefix) --root=$(DESTDIR)

clean:
	rm -rf $(top_builddir)/py/lib.*

