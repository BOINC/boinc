## $Id$

include $(top_srcdir)/Makefile.incl

bin_PROGRAMS = cgi feeder show_shmem file_upload_handler

noinst_PROGRAMS =						\
        validate_test make_work timeout_check file_deleter	\
        assimilator db_dump update_stats start_servers

EXTRA_PROGRAMS = fcgi

cgi_SOURCES =					\
        handle_request.C			\
        main.C					\
        sched_shmem.C				\
        server_types.C				\
        sched_util.C				\
        config.C				\
	../db/boinc_db.C			\
        ../lib/shmem.C				\
        ../lib/util.C				\
        ../lib/parse.C				\
	assimilate_handler.h			\
	config.h				\
	handle_request.h			\
	main.h					\
	sched_shmem.h				\
	sched_util.h				\
	server_types.h

cgi_LDADD = $(MYSQL_LIBS)

feeder_SOURCES =				\
        feeder.C				\
        sched_shmem.C				\
        sched_util.C				\
        config.C				\
	../db/boinc_db.C                        \
        ../lib/parse.C				\
        ../lib/util.C				\
        ../lib/shmem.C

feeder_LDADD = $(MYSQL_LIBS)

show_shmem_SOURCES =				\
        show_shmem.C				\
        sched_shmem.C				\
        sched_util.C				\
        config.C				\
	../db/boinc_db.C                        \
        ../lib/util.C				\
        ../lib/parse.C				\
        ../lib/shmem.C

show_shmem_LDADD = $(MYSQL_LIBS)

file_upload_handler_SOURCES =			\
        file_upload_handler.C			\
        sched_util.C				\
        config.C				\
        ../lib/crypt.C				\
        ../lib/util.C				\
        ../lib/parse.C				\
        ../lib/md5.c				\
        ../lib/md5_file.C

file_upload_handler_DEPENDENCIES = $(LIBRSA)
file_upload_handler_LDADD = $(RSA_LIBS)

validate_test_SOURCES =				\
        validate.C				\
        validate_test.C				\
        sched_util.C				\
        config.C				\
	../db/boinc_db.C                        \
        ../lib/util.C				\
        ../lib/parse.C

validate_test_LDADD = $(MYSQL_LIBS)

file_deleter_SOURCES = 				\
        file_deleter.C				\
        sched_util.C				\
        config.C				\
	../db/boinc_db.C                        \
        ../lib/util.C				\
        ../lib/parse.C

file_deleter_LDADD = $(MYSQL_LIBS)

assimilator_SOURCES = 				\
        assimilator.C				\
        assimilate_handler.C			\
        sched_util.C				\
        config.C				\
	../db/boinc_db.C                        \
        ../lib/util.C				\
        ../lib/parse.C

assimilator_LDADD = $(MYSQL_LIBS)

make_work_SOURCES =					\
        make_work.C					\
        sched_util.C					\
        config.C					\
	../db/boinc_db.C                        \
        ../tools/backend_lib.C				\
        ../tools/process_result_template.C		\
        ../lib/util.C					\
        ../lib/parse.C					\
        ../lib/md5_file.C				\
        ../lib/md5.c					\
        ../lib/crypt.C

make_work_DEPENDENCIES = $(LIBRSA)
make_work_LDADD = $(RSA_LIBS) $(MYSQL_LIBS)

timeout_check_SOURCES =					\
        timeout_check.C					\
        sched_util.C					\
        config.C					\
	../db/boinc_db.C                        \
        ../lib/util.C					\
        ../lib/parse.C					\
        ../lib/md5_file.C				\
        ../lib/md5.c					\
        ../lib/crypt.C					\
        ../tools/backend_lib.C				\
        ../tools/process_result_template.C

timeout_check_DEPENDENCIES = $(LIBRSA)
timeout_check_LDADD = $(RSA_LIBS) $(MYSQL_LIBS)

db_dump_SOURCES = 				\
	../db/boinc_db.C                        \
        ../lib/parse.C				\
        ../lib/util.C				\
        sched_util.C				\
        config.C				\
        db_dump.C

db_dump_LDADD = $(MYSQL_LIBS)

update_stats_SOURCES = 				\
	../db/boinc_db.C                        \
        ../lib/parse.C				\
        ../lib/util.C				\
        sched_util.C				\
        config.C				\
        update_stats.C

update_stats_LDADD = $(MYSQL_LIBS)

start_servers_SOURCES =				\
        ../lib/util.C				\
        ../lib/parse.C				\
        config.C				\
        start_servers.C

fcgi_SOURCES =					\
        handle_request.C			\
        main.C					\
        sched_shmem.C				\
        server_types.C				\
	../db/boinc_db.C                        \
        ../lib/shmem.C				\
        ../lib/parse.C				\
        ../lib/crypt.C				\
        ../lib/md5.c				\
        ../lib/md5_file.C			\
        ../tools/process_result_template.C

fcgi_DEPENDENCIES = $(LIBRSA)
fcgi_CPPFLAGS = -include /usr/local/include/fcgi_stdio.h -D_USING_FCGI_
fcgi_LDADD = $(RSA_LIBS) -lfcgi -lfcgi++ $(MYSQL_LIBS)
