## $Id$

include $(top_srcdir)/Makefile.incl

noinst_PROGRAMS =							\
	cgi feeder show_shmem file_upload_handler			\
        validate_test make_work timeout_check file_deleter		\
        assimilator db_dump update_stats start_servers

noinst_LIBRARIES = libsched.a

EXTRA_PROGRAMS = fcgi
EXTRA_DIST = wd.php wd_nresults_changing.php

# scripts that 'make install' should put in bindir
bin_SCRIPTS = watch_tcp kill_server

LDADD = $(MYSQL_LIBS) -L. -lsched

libsched_a_SOURCES =				\
        sched_shmem.C				\
        sched_util.C				\
        config.C				\
	../db/boinc_db.C			\
        ../lib/util.C				\
        ../lib/crypt.C				\
        ../lib/parse.C				\
        ../lib/shmem.C				\
        ../lib/md5.c				\
        ../lib/md5_file.C			\
	config.h				\
	sched_shmem.h				\
	sched_util.h

cgi_SOURCES =					\
        handle_request.C			\
        main.C					\
        server_types.C				\
	assimilate_handler.h			\
	handle_request.h			\
	main.h					\
	server_types.h

feeder_SOURCES = feeder.C
show_shmem_SOURCES = show_shmem.C
validate_test_SOURCES = validate.C validate_test.C
file_deleter_SOURCES = file_deleter.C
assimilator_SOURCES = assimilator.C assimilate_handler.C
db_dump_SOURCES = db_dump.C
update_stats_SOURCES = update_stats.C
start_servers_SOURCES = start_servers.C

file_upload_handler_SOURCES = file_upload_handler.C
file_upload_handler_DEPENDENCIES = $(LIBRSA)
file_upload_handler_LDADD = $(LDADD) $(RSA_LIBS)

make_work_SOURCES =				\
        make_work.C				\
        ../tools/backend_lib.C			\
        ../tools/process_result_template.C	\
make_work_DEPENDENCIES = $(LIBRSA)
make_work_LDADD = $(LDADD) $(RSA_LIBS)

timeout_check_SOURCES =				\
        timeout_check.C				\
        ../tools/backend_lib.C			\
        ../tools/process_result_template.C

timeout_check_DEPENDENCIES = $(LIBRSA)
timeout_check_LDADD = $(LDADD) $(RSA_LIBS)

fcgi_SOURCES =					\
        handle_request.C			\
        main.C					\
        server_types.C				\
        ../tools/process_result_template.C

fcgi_DEPENDENCIES = $(LIBRSA)
fcgi_CPPFLAGS = -include /usr/local/include/fcgi_stdio.h -D_USING_FCGI_ $(AM_CPPFLAGS)
fcgi_LDADD = $(LDADD) $(RSA_LIBS) -lfcgi -lfcgi++ $(MYSQL_LIBS)
