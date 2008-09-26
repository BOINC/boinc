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
    sched_shmem.cpp \
    sched_util.cpp \
    sched_config.cpp \
	sched_msgs.cpp \
    ../db/boinc_db.cpp \
    ../db/db_base.cpp \
	../lib/msg_log.cpp \
    ../tools/process_result_template.cpp \
    ../tools/backend_lib.cpp

EXTRA_DIST = \
    assimilate_handler.h \
    fcgiapp.h \
    handle_request.h \
    main.h \
    sched_locality.h \
    sched_send.h \
    sched_shmem.h \
    server_types.h \
    start


cgi_SOURCES = \
    edf_sim.cpp \
    handle_request.cpp \
    hr.cpp \
    hr_info.cpp \
    main.cpp \
    sched_array.cpp \
    sched_assign.cpp \
    sched_hr.cpp \
    sched_resend.cpp \
    sched_locality.cpp \
    sched_result.cpp \
    sched_plan.cpp \
    sched_send.cpp \
    sched_timezone.cpp \
    server_types.cpp \
    time_stats_log.cpp \
    ../lib/synch.cpp

census_SOURCES = \
    census.cpp \
    hr.cpp \
    hr_info.cpp

## install only headers that are meant for exporting the API !!
pkginclude_HEADERS = \
	sched_config.h \
	sched_msgs.h \
	sched_util.h \
	../tools/backend_lib.h \
	validate_util.h


feeder_SOURCES = \
    feeder.cpp \
    hr.cpp \
    hr_info.cpp \
    ../lib/synch.cpp

wu_check_SOURCES = wu_check.cpp

show_shmem_SOURCES = show_shmem.cpp

file_deleter_SOURCES = file_deleter.cpp

sample_bitwise_validator_SOURCES = validator.cpp sample_bitwise_validator.cpp validate_util.cpp validate_util.h validate_util2.cpp

sample_trivial_validator_SOURCES = validator.cpp sample_trivial_validator.cpp validate_util.cpp validate_util.h validate_util2.cpp

sample_dummy_assimilator_SOURCES = assimilator.cpp sample_dummy_assimilator.cpp validate_util.cpp validate_util.h

sample_assimilator_SOURCES = assimilator.cpp sample_assimilator.cpp validate_util.cpp validate_util.h

pymw_assimilator_SOURCES = assimilator.cpp pymw_assimilator.cpp validate_util.cpp validate_util.h 
pymw_assimilator_DEPENDENCIES = $(LIB_SCHED) 

single_job_assimilator_SOURCES = assimilator.cpp single_job_assimilator.cpp validate_util.cpp validate_util.h

sample_work_generator_SOURCES = sample_work_generator.cpp

db_dump_SOURCES = db_dump.cpp

db_purge_SOURCES = db_purge.cpp

trickle_handler_SOURCES = trickle_handler.cpp

update_stats_SOURCES = update_stats.cpp

file_upload_handler_SOURCES = file_upload_handler.cpp

make_work_SOURCES = make_work.cpp

transitioner_SOURCES = transitioner.cpp

message_handler_SOURCES = message_handler.cpp

request_file_list_SOURCES = request_file_list.cpp

get_file_SOURCES = get_file.cpp

send_file_SOURCES = send_file.cpp

delete_file_SOURCES = delete_file.cpp

sched_driver_SOURCES = sched_driver.cpp

# N.B.: the FCGI scheduler can't use lib/libboinc.a;
# it needs to have specially compiled versions of everything
# because its stdio is different
fcgi_SOURCES = \
    handle_request.cpp \
    hr.cpp \
    hr_info.cpp \
    main.cpp \
    sched_array.cpp \
    sched_assign.cpp \
    sched_config.cpp \
    sched_hr.cpp \
    sched_locality.cpp \
    sched_msgs.cpp \
    sched_resend.cpp \
    sched_result.cpp \
    sched_plan.cpp \
    sched_send.cpp \
    sched_shmem.cpp \
    sched_timezone.cpp \
    sched_util.cpp \
    server_types.cpp \
	time_stats_log.cpp \
    edf_sim.cpp \
    ../db/boinc_db.cpp \
    ../db/db_base.cpp \
    ../lib/base64.cpp \
    ../lib/boinc_fcgi.cpp \
    ../lib/coproc.cpp \
    ../lib/crypt.cpp \
    ../lib/filesys.cpp \
    ../lib/md5.c \
    ../lib/md5_file.cpp \
    ../lib/miofile.cpp \
    ../lib/msg_log.cpp \
    ../lib/parse.cpp \
    ../lib/shmem.cpp \
    ../lib/synch.cpp \
    ../lib/str_util.cpp \
    ../lib/util.cpp \
    ../tools/process_result_template.cpp \
    ../tools/backend_lib.cpp

fcgi_CPPFLAGS = -D_USING_FCGI_ -include boinc_fcgi.h $(AM_CPPFLAGS)
fcgi_LDADD = -lfcgi $(MYSQL_LIBS)

fcgi_file_upload_handler_SOURCES = \
    file_upload_handler.cpp \
    sched_config.cpp \
    sched_msgs.cpp \
    ../lib/boinc_fcgi.cpp \
    ../lib/miofile.cpp \
    ../lib/msg_log.cpp \
    ../lib/parse.cpp \
    ../lib/crypt.cpp

fcgi_file_upload_handler_CPPFLAGS = -D_USING_FCGI_ -include boinc_fcgi.h $(AM_CPPFLAGS)
fcgi_file_upload_handler_LDADD = $(LDADD) -lfcgi

.PHONY: PHONY-start

PHONY-start:
	@test -f start || @LN_S@ $(srcdir)/start start && test -f start

status stop: PHONY-start
	@test -f $@ || @LN_S@ start $@ && test -f $@

CLEANFILES = status stop
