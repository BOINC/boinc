## $Id$

include $(top_srcdir)/Makefile.incl

noinst_PROGRAMS =                               \
    cgi                                         \
    feeder                                      \
    db_dump                                     \
    db_purge                                    \
    file_deleter                                \
    file_upload_handler                         \
    make_work                                   \
    show_shmem                                  \
    transitioner                                \
    message_handler                             \
    update_stats                                \
    assimilator_placeholder			\
    sample_dummy_assimilator                    \
    validator_placeholder                       \
    sample_bitwise_validator                    \
    sample_trivial_validator                    \
    get_file					\
    send_file					\
    request_file_list				\
    delete_file					\
    wu_check

lib_LIBRARIES = libsched.a

EXTRA_PROGRAMS = fcgi \
		 fcgi_file_upload_handler

# scripts that 'make install' should put in bindir
bin_SCRIPTS = start stop status

LDADD = -L. -lsched $(MYSQL_LIBS) $(BOINC_LIB) $(PTHREAD_LIBS)

LIB_SCHED = libsched.a

libsched_a_SOURCES =                            \
    sched_shmem.C                               \
    sched_util.C                                \
    sched_config.C                              \
    sched_msgs.C                                \
    ../db/boinc_db.C                            \
    ../db/db_base.C                             \
    ../lib/msg_log.C                            \
    ../tools/process_result_template.C          \
    ../tools/backend_lib.C 

EXTRA_DIST =    \
    assimilate_handler.h                        \
    fcgiapp.h                                   \
    fcgi_stdio.h                                \
    handle_request.h                            \
    main.h                                      \
    sched_locality.h                            \
    sched_send.h                                \
    sched_shmem.h                               \
    server_types.h                              \
    start                                       \
    validate_util.h


cgi_SOURCES = \
    handle_request.C \
    main.C \
    sched_array.C \
    sched_hr.C \
    sched_resend.C \
    sched_send.C \
    sched_locality.C \
    sched_timezone.C \
    server_types.C \
    ../lib/synch.C


## install header-files with prefix-subdir BOINC/ to avoid name-conflicts
includedir = ${prefix}/include/BOINC/

## install only headers that are meant for exporting the API !!
include_HEADERS = 	\
	sched_config.h	\
	sched_msgs.h	\
	sched_util.h	\
	../tools/backend_lib.h


cgi_DEPENDENCIES = $(LIB_SCHED)

feeder_SOURCES = \
    feeder.C \
    ../lib/synch.C

feeder_DEPENDENCIES = $(LIB_SCHED)

wu_check_SOURCES = wu_check.C
wu_check_DEPENDENCIES = $(LIB_SCHED)

show_shmem_SOURCES = show_shmem.C
show_shmem_DEPENDENCIES = $(LIB_SCHED)

file_deleter_SOURCES = file_deleter.C
file_deleter_DEPENDENCIES = $(LIB_SCHED)

sample_bitwise_validator_SOURCES = validator.C sample_bitwise_validator.C validate_util.C validate_util.h
sample_bitwise_validator_DEPENDENCIES = $(LIB_SCHED)

sample_trivial_validator_SOURCES = validator.C sample_trivial_validator.C validate_util.C validate_util.h
sample_trivial_validator_DEPENDENCIES = $(LIB_SCHED)

validator_placeholder_SOURCES = validator.C validator_placeholder.C validate_util.C validate_util.h
validator_placeholder_DEPENDENCIES = $(LIB_SCHED)

sample_dummy_assimilator_SOURCES = assimilator.C sample_dummy_assimilator.C validate_util.C validate_util.h
sample_dummy_assimilator_DEPENDENCIES = $(LIB_SCHED)

assimilator_placeholder_SOURCES = assimilator.C assimilator_placeholder.C validate_util.C validate_util.h
assimilator_placeholder_DEPENDENCIES = $(LIB_SCHED)

db_dump_SOURCES = db_dump.C
db_dump_DEPENDENCIES = $(LIB_SCHED)

db_purge_SOURCES = db_purge.C
db_purge_DEPENDENCIES = $(LIB_SCHED)

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

message_handler_SOURCES = message_handler.C
message_handler_DEPENDENCIES = $(LIBRSA) $(LIB_SCHED)
message_handler_LDADD = $(LDADD) $(RSA_LIBS)

request_file_list_SOURCES = request_file_list.C
request_file_list_DEPENDENCIES = $(LIBRSA) $(LIB_SCHED)
request_file_list_LDADD = $(LDADD) $(RSA_LIBS)

get_file_SOURCES = get_file.C
get_file_DEPENDENCIES = $(LIBRSA) $(LIB_SCHED)
get_file_LDADD = $(LDADD) $(RSA_LIBS)

send_file_SOURCES = send_file.C
send_file_DEPENDENCIES = $(LIBRSA) $(LIB_SCHED)
send_file_LDADD = $(LDADD) $(RSA_LIBS)

delete_file_SOURCES = delete_file.C
delete_file_DEPENDENCIES = $(LIBRSA) $(LIB_SCHED)
delete_file_LDADD = $(LDADD) $(RSA_LIBS)

fcgi_SOURCES =                                  \
    handle_request.C                            \
    main.C                                      \
    sched_send.C                                \
    sched_resend.C 				\
    sched_array.C 				\
    sched_hr.C 					\
    server_types.C                              \
    ../lib/synch.C                              \
    sched_shmem.C                               \
    sched_util.C                                \
    sched_config.C                              \
    sched_msgs.C                                \
    sched_locality.C                            \
    sched_timezone.C                            \
    ../db/boinc_db.C                            \
    ../db/db_base.C                             \
    ../lib/util.C                               \
    ../lib/crypt.C                              \
    ../lib/filesys.C                            \
    ../lib/parse.C                              \
    ../lib/base64.C                             \
    ../lib/shmem.C                              \
    ../lib/md5.c                                \
    ../lib/md5_file.C                           \
    ../lib/msg_log.C                            \
    ../tools/process_result_template.C          \
    ../tools/backend_lib.C

fcgi_DEPENDENCIES = $(LIBRSA) $(LIB_SCHED)
fcgi_CPPFLAGS = -include fcgi_stdio.h -D_USING_FCGI_ $(AM_CPPFLAGS)
fcgi_LDADD = $(LDADD) $(RSA_LIBS) -lfcgi $(MYSQL_LIBS)

fcgi_file_upload_handler_SOURCES = 		\
    file_upload_handler.C			\
    ../lib/parse.C				\
    ../lib/crypt.C
fcgi_file_upload_handler_DEPENDENCIES = $(LIBRSA) $(LIB_SCHED)
fcgi_file_upload_handler_CPPFLAGS = -include fcgi_stdio.h -D_USING_FCGI_ $(AM_CPPFLAGS)
fcgi_file_upload_handler_LDADD = $(LDADD) $(RSA_LIBS) -lfcgi

.PHONY: PHONY-start

PHONY-start:
	@test -f start || @LN_S@ $(srcdir)/start start && test -f start

status stop: PHONY-start
	@test -f $@ || @LN_S@ start $@ && test -f $@
