## $Id$

include $(top_srcdir)/Makefile.incl

TESTS = test_sanity.py				\
	test_uc.py				\
	test_concat.py				\
	test_1sec.py				\
	test_abort.py				\
	test_rsc.py				\
	test_exit.py				\
	test_signal.py				\
	test_backend.py				\
	test_masterurl_failure.py

# TODO: phase out php stuff

EXTRA_DIST =							\
        *.xml *wu *result *output *input			\
	test*.py						\
	db_def_to_php *.php version.inc.in boinc_db.inc 	\
	test*.inc init.inc boinc_db.inc

# BUILT_SOURCES = boinc_db.inc boinc_db.py

$(TESTS): ../py/version.py $(top_srcdir)/py/boinc_db.py

boinc_db.inc: ../db/boinc_db.h ../lib/result_state.h
	cat $^ | ./db_def_to_php > $@
