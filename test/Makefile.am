## $Id$

include $(top_srcdir)/Makefile.incl

# TESTS = test_sanity.php				\
# 	test_uc.php				\
# 	test_concat.php				\
# 	test_1sec.php				\
# 	test_rsc.php				\
# 	test_backend.php

TESTS = test_sanity.py				\
	test_uc.py				\
	test_concat.py				\
	test_1sec.py				\
	test_backend.py

EXTRA_DIST =					\
        *.xml *.php *wu *result *output *input

# TODO: phase out php stuff
noinst_SCRIPTS = 						\
	db_def_to_php db_def_to_py				\
	*.php version.inc.in boinc.py test_*.py version.py.in

BUILT_SOURCES = boinc_db.inc boinc_db.py

boinc_db.inc: ../db/boinc_db.h
	./db_def_to_php < ../db/boinc_db.h > boinc_db.inc

boinc_db.py: ../db/boinc_db.h
	./db_def_to_py < ../db/boinc_db.h > boinc_db.py
