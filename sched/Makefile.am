## -*- mode: makefile; tab-width: 4 -*-
## $Id$

include $(top_srcdir)/Makefile.incl

AM_CPPFLAGS += $(MYSQL_CFLAGS) $(PTHREAD_CFLAGS)
AM_LDFLAGS += -static

if ENABLE_LIBRARIES

libsched_sources = \
    credit.cpp \
    sched_shmem.cpp \
    sched_util.cpp \
    sched_util_basic.cpp \
    sched_config.cpp \
    sched_limit.cpp \
    ../db/boinc_db.cpp \
    ../db/db_base.cpp \
    ../tools/process_result_template.cpp \
    ../tools/process_input_template.cpp \
    ../tools/backend_lib.cpp

lib_LTLIBRARIES = libsched.la
libsched_la_SOURCES = $(libsched_sources)
libsched_la_CFLAGS = $(AM_CPPFLAGS)
libsched_la_CXXFLAGS = $(AM_CPPFLAGS)
libsched_la_LDFLAGS= -version-number $(LIBBOINC_VERSION)
libsched_la_LIBADD= $(SSL_LIBS)

## install only headers that are meant for exporting the API !!
if INSTALL_HEADERS
pkginclude_HEADERS = \
	credit.h \
	sched_config.h \
	sched_limit.h \
	sched_util.h \
	sched_util_basic.h \
	../tools/backend_lib.h \
	validate_util.h
endif
# end of "if INSTALL_HEADERS

if ENABLE_FCGI

lib_LTLIBRARIES += libsched_fcgi.la
libsched_fcgi_la_SOURCES = $(libsched_sources)
libsched_fcgi_la_CFLAGS = -D_USING_FCGI_ $(AM_CPPFLAGS)
libsched_fcgi_la_CXXFLAGS = -D_USING_FCGI_ $(AM_CPPFLAGS)
libsched_fcgi_la_LDFLAGS= -version-number $(LIBBOINC_VERSION)
libsched_fcgi_la_LIBADD=

endif
# end of "if ENABLE_FCGI"

# Some OSs may not prefix libraries with lib.
# For example OS2
if OS_OS2
LIBSCHED_STATIC=sched.${LIBEXT}
LIBSCHED_FCGI_STATIC=sched_fcgi.${LIBEXT}
else
LIBSCHED_STATIC=libsched.${LIBEXT}
LIBSCHED_FCGI_STATIC=libsched_fcgi.${LIBEXT}
endif


if BUILD_STATIC_LIBS
all_local = $(LIBSCHED_STATIC)
if ENABLE_FCGI
all_local += $(LIBSCHED_FCGI_STATIC)
endif
endif

all-local: $(all_local)

$(LIBSCHED_STATIC): libsched.la
	rm -f $(LIBSCHED_STATIC)
	$(LN) .libs/$(LIBSCHED_STATIC) .

$(LIBSCHED_FCGI_STATIC): libsched_fcgi.la
	rm -f $(LIBSCHED_FCGI_STATIC)
	$(LN) .libs/$(LIBSCHED_FCGI_STATIC) .


endif
# end of "if ENABLE_LIBRARIES

if ENABLE_SERVER

schedcgidir = $(libexecdir)/boinc-server-maker/sched
schedsharedir = $(libexecdir)/boinc-server-maker/sched
schedbindir = $(libexecdir)/boinc-server-maker/sched

schedbin_PROGRAMS = \
    adjust_user_priority \
    delete_file \
    get_file \
    make_work \
    put_file \
    sched_driver \
    show_shmem \
    wu_check

schedshare_PROGRAMS = \
    antique_file_deleter \
    census \
    credit_test \
    db_dump \
    db_purge \
    feeder \
    file_deleter \
    message_handler \
    sample_assimilator \
    sample_bitwise_validator \
    sample_dummy_assimilator \
    sample_substr_validator \
    sample_trivial_validator \
    sample_work_generator \
	script_assimilator \
	script_validator \
    single_job_assimilator \
    size_regulator \
    transitioner \
    trickle_credit \
    trickle_deadline \
    trickle_echo \
    update_stats

schedcgi_PROGRAMS= \
    cgi \
    file_upload_handler

schedshare_DATA = \
    db_dump_spec.xml \
    assimilator.py \
    pymw_assimilator.py

# scripts that 'make install' should put in bindir
schedshare_SCRIPTS = start stop status transitioner_catchup.php

noinst_HEADERS = \
    assimilate_handler.h \
    handle_request.h \
    plan_class_spec.h \
    sched_main.h \
    sched_locality.h \
    sched_nci.h \
    sched_score.h \
    sched_send.h \
    sched_shmem.h \
    sched_version.h \
    sched_types.h

EXTRA_DIST = \
    start

cgi_sources = \
    buda.cpp \
	credit.cpp \
    edf_sim.cpp \
    handle_request.cpp \
    hr.cpp \
    hr_info.cpp \
    plan_class_spec.cpp \
    sched_array.cpp \
    sched_assign.cpp \
    sched_check.cpp \
    sched_customize.cpp \
    sched_files.cpp \
    sched_host.cpp \
    sched_hr.cpp \
    sched_keyword.cpp \
    sched_limit.cpp \
    sched_locality.cpp \
    sched_main.cpp \
    sched_nci.cpp \
    sched_resend.cpp \
    sched_result.cpp \
    sched_score.cpp \
    sched_send.cpp \
    sched_timezone.cpp \
	../vda/sched_vda.cpp \
    sched_version.cpp \
    sched_types.cpp \
    time_stats_log.cpp

cgi_SOURCES = $(cgi_sources)
cgi_LDADD = $(SERVERLIBS)

census_SOURCES = \
    census.cpp \
    hr.cpp \
    hr_info.cpp
census_LDADD = $(SERVERLIBS)

credit_test_SOURCES = \
	credit_test.cpp
credit_test_LDADD = $(SERVERLIBS)

feeder_SOURCES = \
    feeder.cpp \
    hr.cpp \
    hr_info.cpp \
    ../lib/synch.cpp
feeder_LDADD = $(SERVERLIBS)

wu_check_SOURCES = wu_check.cpp
wu_check_LDADD = $(SERVERLIBS)

show_shmem_SOURCES = show_shmem.cpp
show_shmem_LDADD = $(SERVERLIBS)

file_deleter_SOURCES = file_deleter.cpp
file_deleter_LDADD = $(SERVERLIBS)

antique_file_deleter_SOURCES = antique_file_deleter.cpp
antique_file_deleter_LDADD = $(SERVERLIBS)

VALIDATOR_SOURCES = \
	credit.cpp \
	validator.cpp \
	validate_util.cpp \
	validate_util2.cpp

sample_bitwise_validator_SOURCES = $(VALIDATOR_SOURCES) \
	sample_bitwise_validator.cpp
sample_bitwise_validator_LDADD = $(SERVERLIBS)

sample_substr_validator_SOURCES = $(VALIDATOR_SOURCES) \
	sample_substr_validator.cpp
sample_substr_validator_LDADD = $(SERVERLIBS)

sample_trivial_validator_SOURCES = $(VALIDATOR_SOURCES) \
	sample_trivial_validator.cpp
sample_trivial_validator_LDADD = $(SERVERLIBS)

ASSIMILATOR_SOURCES = \
	assimilator.cpp \
	validate_util.cpp

sample_dummy_assimilator_SOURCES = $(ASSIMILATOR_SOURCES) \
	sample_dummy_assimilator.cpp
sample_dummy_assimilator_LDADD = $(SERVERLIBS)

sample_assimilator_SOURCES = $(ASSIMILATOR_SOURCES) \
	sample_assimilator.cpp
sample_assimilator_LDADD = $(SERVERLIBS)

single_job_assimilator_SOURCES = $(ASSIMILATOR_SOURCES) \
	single_job_assimilator.cpp
single_job_assimilator_LDADD = $(SERVERLIBS)

sample_work_generator_SOURCES = sample_work_generator.cpp
sample_work_generator_LDADD = $(SERVERLIBS)

script_assimilator_SOURCES = $(ASSIMILATOR_SOURCES) \
	script_assimilator.cpp
script_assimilator_LDADD = $(SERVERLIBS)

script_validator_SOURCES = $(VALIDATOR_SOURCES) \
	script_validator.cpp
script_validator_LDADD = $(SERVERLIBS)

db_dump_SOURCES = db_dump.cpp
db_dump_LDADD = $(SERVERLIBS) -lz

db_purge_SOURCES = db_purge.cpp
db_purge_LDADD = $(SERVERLIBS) -lz

trickle_credit_SOURCES = trickle_credit.cpp trickle_handler.cpp
trickle_credit_LDADD = $(SERVERLIBS)

trickle_deadline_SOURCES = trickle_deadline.cpp trickle_handler.cpp
trickle_deadline_LDADD = $(SERVERLIBS)

trickle_echo_SOURCES = trickle_echo.cpp trickle_handler.cpp
trickle_echo_LDADD = $(SERVERLIBS)

update_stats_SOURCES = update_stats.cpp
update_stats_LDADD = $(SERVERLIBS)

file_upload_handler_SOURCES = file_upload_handler.cpp sched_config.cpp sched_util_basic.cpp sched_limit.cpp
file_upload_handler_LDADD = $(FUHLIBS)

make_work_SOURCES = make_work.cpp
make_work_LDADD = $(SERVERLIBS)

transitioner_SOURCES = transitioner.cpp
transitioner_LDADD = $(SERVERLIBS)

size_regulator_SOURCES = size_regulator.cpp
size_regulator_LDADD = $(SERVERLIBS)

message_handler_SOURCES = message_handler.cpp
message_handler_LDADD = $(SERVERLIBS)

get_file_SOURCES = get_file.cpp
get_file_LDADD = $(SERVERLIBS)

put_file_SOURCES = put_file.cpp
put_file_LDADD = $(SERVERLIBS)

delete_file_SOURCES = delete_file.cpp
delete_file_LDADD = $(SERVERLIBS)

adjust_user_priority_SOURCES = adjust_user_priority.cpp
adjust_user_priority_LDADD = $(SERVERLIBS)

sched_driver_SOURCES = sched_driver.cpp
sched_driver_LDADD = $(SERVERLIBS)

if ENABLE_FCGI

schedcgi_PROGRAMS += fcgi fcgi_file_upload_handler

fcgi_SOURCES = $(cgi_sources)
fcgi_CPPFLAGS = -D_USING_FCGI_ $(AM_CPPFLAGS)
fcgi_LDADD = $(SERVERLIBS_FCGI)

fcgi_file_upload_handler_SOURCES = file_upload_handler.cpp sched_config.cpp sched_util_basic.cpp sched_limit.cpp
fcgi_file_upload_handler_CPPFLAGS = -D_USING_FCGI_ $(AM_CPPFLAGS)
fcgi_file_upload_handler_LDADD = $(FUHLIBS_FCGI)

endif
# end of "if ENABLE_FCGI"
endif
# end of "if ENABLE_SERVER"

.PHONY: PHONY-start

PHONY-start:
	@test -f start || @LN_S@ $(srcdir)/start start && test -f start

status stop: PHONY-start
	@test -f $@ || @LN_S@ start $@ && test -f $@

CLEANFILES = status stop *.gcno *.gcda *.gcov *.a
