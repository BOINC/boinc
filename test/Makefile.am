## $Id$

include $(top_srcdir)/Makefile.incl

TESTS = test_sanity.php				\
	test_uc.php				\
	test_concat.php				\
	test_1sec.php				\
	test_rsc.php				\
	test_backend.php

# test_sticky.php

EXTRA_DIST =					\
        *.xml *.php *wu *result *output		\
        *input *.inc *.in

boinc_db.inc: ../db/boinc_db.h
	./db_def_to_php < ../db/boinc_db.h > boinc_db.inc
