## -*- mode: makefile; tab-width: 4 -*-
## $Id$

include $(top_srcdir)/Makefile.incl

noinst_PROGRAMS = \
    census \
    cgi \
    db_dump \
    db_purge \
    delete_file \
    feeder \
    file_deleter \
    file_upload_handler \
    get_file \
    make_work \
    message_handler \
	pymw_assimilator \
    request_file_list \
    sample_assimilator \
    sample_dummy_assimilator \
    sample_bitwise_validator \
    sample_trivial_validator \
    sample_work_generator \
    single_job_assimilator \
    sched_driver \
    send_file \
    show_shmem \
    transitioner \
    trickle_handler \
    update_stats \
    wu_check

lib_LIBRARIES = libsched.a

EXTRA_PROGRAMS = fcgi \
		 fcgi_file_upload_handler

# scripts that 'make install' should put in bindir
bin_SCRIPTS = start stop status

LDADD = $(LIBSCHED) $(LIBBOINC) $(MYSQL_LIBS) $(PTHREAD_LIBS) $(RSA_LIBS)
AM_CXXFLAGS = $(MYSQL_CFLAGS)

libsched_a_SOURCES = \
    sched_shmem.C \
    sched_util.C \
    sched_config.C \
	sched_msgs.C \
    ../db/boinc_db.C \
    ../db/db_base.C \
	../lib/msg_log.C \
    ../tools/process_result_template.C \
    ../tools/backend_lib.C

EXTRA_DIST = \
    assimilate_handler.h \
    fcgiapp.h \
    fcgi_stdio.h \
    handle_request.h \
    main.h \
    sched_locality.h \
    sched_send.h \
    sched_shmem.h \
    server_types.h \
    start


cgi_SOURCES = \
    edf_sim.C \
    handle_request.C \
    hr.C \
    hr_info.C \
    main.C \
    sched_array.C \
    sched_assign.C \
    sched_hr.C \
    sched_resend.C \
    sched_locality.C \
    sched_result.C \
    sched_plan.C \
    sched_send.C \
    sched_timezone.C \
    server_types.C \
    time_stats_log.C \
    ../lib/synch.C

census_SOURCES = \
    census.C \
    hr.C \
    hr_info.C

## install only headers that are meant for exporting the API !!
pkginclude_HEADERS = \
	sched_config.h \
	sched_msgs.h \
	sched_util.h \
	../tools/backend_lib.h \
	validate_util.h


feeder_SOURCES = \
    feeder.C \
    hr.C \
    hr_info.C \
    ../lib/synch.C

wu_check_SOURCES = wu_check.C

show_shmem_SOURCES = show_shmem.C

file_deleter_SOURCES = file_deleter.C

sample_bitwise_validator_SOURCES = validator.C sample_bitwise_validator.C validate_util.C validate_util.h validate_util2.C

sample_trivial_validator_SOURCES = validator.C sample_trivial_validator.C validate_util.C validate_util.h validate_util2.C

sample_dummy_assimilator_SOURCES = assimilator.C sample_dummy_assimilator.C validate_util.C validate_util.h

sample_assimilator_SOURCES = assimilator.C sample_assimilator.C validate_util.C validate_util.h

pymw_assimilator_SOURCES = assimilator.C pymw_assimilator.C validate_util.C validate_util.h 
pymw_assimilator_DEPENDENCIES = $(LIB_SCHED) 

single_job_assimilator_SOURCES = assimilator.C single_job_assimilator.C validate_util.C validate_util.h

sample_work_generator_SOURCES = sample_work_generator.C

db_dump_SOURCES = db_dump.C

db_purge_SOURCES = db_purge.C

trickle_handler_SOURCES = trickle_handler.C

update_stats_SOURCES = update_stats.C

file_upload_handler_SOURCES = file_upload_handler.C

make_work_SOURCES = make_work.C

transitioner_SOURCES = transitioner.C

message_handler_SOURCES = message_handler.C

request_file_list_SOURCES = request_file_list.C

get_file_SOURCES = get_file.C

send_file_SOURCES = send_file.C

delete_file_SOURCES = delete_file.C

sched_driver_SOURCES = sched_driver.C

# N.B.: the FCGI scheduler can't use lib/libboinc.a;
# it needs to have specially compiled versions of everything
# because its stdio is different
fcgi_SOURCES = \
    handle_request.C \
    hr.C \
    hr_info.C \
    main.C \
    sched_array.C \
    sched_assign.C \
    sched_config.C \
    sched_hr.C \
    sched_locality.C \
    sched_msgs.C \
    sched_resend.C \
    sched_result.C \
    sched_plan.C \
    sched_send.C \
    sched_shmem.C \
    sched_timezone.C \
    sched_util.C \
    server_types.C \
	time_stats_log.C \
    edf_sim.C \
    ../db/boinc_db.C \
    ../db/db_base.C \
    ../lib/base64.C \
    ../lib/coproc.C \
    ../lib/crypt.C \
    ../lib/filesys.C \
    ../lib/md5.c \
    ../lib/md5_file.C \
    ../lib/miofile.C \
    ../lib/msg_log.C \
    ../lib/parse.C \
    ../lib/shmem.C \
    ../lib/synch.C \
    ../lib/str_util.C \
    ../lib/util.C \
    ../tools/process_result_template.C \
    ../tools/backend_lib.C

fcgi_CPPFLAGS = -include fcgi_stdio.h -D_USING_FCGI_ $(AM_CPPFLAGS)
fcgi_LDADD = -lfcgi $(MYSQL_LIBS)

fcgi_file_upload_handler_SOURCES = \
    file_upload_handler.C \
    sched_config.C \
    sched_msgs.C \
    ../lib/miofile.C \
    ../lib/msg_log.C \
    ../lib/parse.C \
    ../lib/crypt.C

fcgi_file_upload_handler_CPPFLAGS = -include fcgi_stdio.h -D_USING_FCGI_ $(AM_CPPFLAGS)
fcgi_file_upload_handler_LDADD = $(LDADD) -lfcgi

.PHONY: PHONY-start

PHONY-start:
	@test -f start || @LN_S@ $(srcdir)/start start && test -f start

status stop: PHONY-start
	@test -f $@ || @LN_S@ start $@ && test -f $@

CLEANFILES = status stop
