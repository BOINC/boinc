## $Id$

include $(top_srcdir)/Makefile.incl

TESTS = test_sanity.py				\
	test_uc.py

#	test_concat.py
#	test_1sec.py
#	test_abort.py
#	test_rsc.py
#	test_exit.py
#	test_signal.py
#	test_masterurl_failure.py


# TODO: phase out php stuff

EXTRA_DIST =						\
        *.xml *wu *result *output *input		\
	test*.py cgiserver.py fake_php.py testproxy	\
	db_def_to_php *.php version.inc.in boinc_db.inc	\
	test*.inc init.inc boinc_db.inc 

$(TESTS): $(top_srcdir)/py/Boinc/version.py $(top_srcdir)/py/Boinc/boinc_db.py

$(srcdir)/boinc_db.inc: $(top_srcdir)/db/boinc_db.h $(top_srcdir)/lib/result_state.h
	cat $^ | $(srcdir)/db_def_to_php > $@
