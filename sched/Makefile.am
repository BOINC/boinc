## $Id$

include $(top_srcdir)/Makefile.incl

noinst_PROGRAMS = \
    assimilator \
	cgi feeder \
    db_dump \
    file_deleter \
    file_upload_handler	\
    make_work \
    show_shmem \
    transitioner \
    trickle_handler \
    update_stats \
    validate_test \
    validate_trivial \
    wu_check

noinst_LIBRARIES = libsched.a

EXTRA_PROGRAMS = fcgi
EXTRA_DIST = wd.php wd_nresults_changing.php watch_tcp start grep_logs

# scripts that 'make install' should put in bindir
bin_SCRIPTS = watch_tcp start stop status

LDADD = -L. -lsched $(MYSQL_LIBS)

LIB_SCHED = libsched.a

libsched_a_SOURCES =				\
    sched_shmem.C				\
    sched_util.C				\
    sched_config.C				\
	sched_msgs.C			    \
	../db/boinc_db.C			\
	../db/db_base.C				\
    ../lib/util.C				\
    ../lib/crypt.C				\
    ../lib/filesys.C				\
    ../lib/parse.C				\
    ../lib/base64.C				\
    ../lib/shmem.C				\
    ../lib/md5.c				\
    ../lib/md5_file.C			\
	../lib/msg_log.C			\
	../tools/process_result_template.C	\
    ../tools/backend_lib.C			\
	sched_config.h				\
	sched_shmem.h				\
	sched_util.h

cgi_SOURCES =					\
    handle_request.C			\
    main.C					\
    sched_send.C					\
    server_types.C				\
    ../lib/synch.C

cgi_DEPENDENCIES = $(LIB_SCHED)

feeder_SOURCES = \
    feeder.C \
    ../lib/synch.C

feeder_DEPENDENCIES = $(LIB_SCHED)

wu_check_SOURCES = wu_check.C
wu_check_DEPENDENCIES = $(LIB_SCHED)

show_shmem_SOURCES = show_shmem.C
show_shmem_DEPENDENCIES = $(LIB_SCHED)

validate_test_SOURCES = validate.C validate_test.C validate_util.C validate_util.h
validate_test_DEPENDENCIES = $(LIB_SCHED)

validate_trivial_SOURCES = validate.C validate_trivial.C validate_util.C validate_util.h
validate_trivial_DEPENDENCIES = $(LIB_SCHED)

file_deleter_SOURCES = file_deleter.C
file_deleter_DEPENDENCIES = $(LIB_SCHED)

assimilator_SOURCES = assimilator.C assimilate_handler.C
assimilator_DEPENDENCIES = $(LIB_SCHED)

db_dump_SOURCES = db_dump.C
db_dump_DEPENDENCIES = $(LIB_SCHED)

update_stats_SOURCES = update_stats.C
update_stats_DEPENDENCIES = $(LIB_SCHED)

file_upload_handler_SOURCES = file_upload_handler.C
file_upload_handler_DEPENDENCIES = $(LIBRSA) $(LIB_SCHED)
file_upload_handler_LDADD = $(LDADD) $(RSA_LIBS)

make_work_SOURCES = make_work.C
make_work_DEPENDENCIES = $(LIBRSA) $(LIB_SCHED)
make_work_LDADD = $(LDADD) $(RSA_LIBS)

transitioner_SOURCES = transitioner.C
transitioner_DEPENDENCIES = $(LIBRSA) $(LIB_SCHED)
transitioner_LDADD = $(LDADD) $(RSA_LIBS)

fcgi_SOURCES =					\
        handle_request.C			\
        main.C					\
        sched_send.C					\
        server_types.C  \
        ../lib/synch.C

fcgi_DEPENDENCIES = $(LIBRSA) $(LIB_SCHED)
fcgi_CPPFLAGS = -include /usr/local/include/fcgi_stdio.h -D_USING_FCGI_ $(AM_CPPFLAGS)
fcgi_LDADD = $(LDADD) $(RSA_LIBS) -lfcgi -lfcgi++ $(MYSQL_LIBS)

.PHONY: PHONY-start

PHONY-start:
	@test -f start || ln -s $(srcdir)/start start && test -f start

status stop: PHONY-start
	@test -f $@ || ln -s start $@ && test -f $@
