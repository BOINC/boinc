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
	db_def_to_php db_def_to_py				\
	*.php version.inc.in boinc.py test_*.py version.py.in	\
	boinc_db.inc boinc_db.py

BUILT_SOURCES = boinc_db.inc boinc_db.py

$(TESTS): version.py boinc_db.py

boinc_db.inc: ../db/boinc_db.h ../lib/result_state.h
	cat $^ | ./db_def_to_php > $@

boinc_db.py: ../db/boinc_db.h ../lib/result_state.h
	cat $^ | ./db_def_to_py > $@
