## -*- mode: makefile; tab-width: 4 -*-
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

EXTRA_DIST = 	log_flags.xml \
		1sec_wu concat_wu uc_exit_wu uc_multiple_download_wu uc_sig_wu uc_wu ucs_wu 	\
		1sec_result abort_result concat_result uc_multiple_uploads_result uc_result 	\
		input small_input 								\
		boinc_path_config.py cgiserver.py fake_php.py test_1sec.py test_abort.py test_backend.py test_concat.py \
		test_exit.py test_masterurl_failure.py test_rsc.py test_sanity.py test_sched_moved.py test_signal.py test_uc.py testbase.py \
		testproxy db_def_to_php db_def_to_py 						\
		gen_keys.php		   test_limit.php	       test_suite.php 		\
		make_project.php	   test_loop.php	       test_time.php 		\
		make_project_ap.php	   test_masterurl_failure.php  test_uc.php 		\
		make_project_sah.php	   test_mdownload_backoff.php  test_uc_slow.php 	\
		test_1sec.php		   test_pers.php	       test_uc_win.php  	\
		test_abort.php		   test_prefs.php	       test_upload_backoff.php 	\
		test_api.php		   test_rsc.php		       test_upload_resume.php  	\
		test_backend.php	   test_sanity.php	       test_water.php   	\
		test_concat.php		   test_sched_failure.php  				\
		test_download_backoff.php  test_sticky.php  					\
		boinc_db.inc  init.inc	test.inc  test_uc.inc  version.inc


$(TESTS): $(top_srcdir)/py/Boinc/version.py $(top_srcdir)/py/Boinc/boinc_db.py

$(srcdir)/boinc_db.inc: $(top_srcdir)/db/boinc_db.h $(top_srcdir)/lib/result_state.h
	cat $^ | $(srcdir)/db_def_to_php > $@
