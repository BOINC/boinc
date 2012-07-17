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
    sched_config.cpp \
    sched_limit.cpp \
    sched_msgs.cpp \
    ../db/boinc_db.cpp \
    ../db/db_base.cpp \
    ../tools/process_result_template.cpp \
    ../tools/process_input_template.cpp \
    ../tools/backend_lib.cpp

lib_LTLIBRARIES = libsched.la
libsched_la_SOURCES = $(libsched_sources)
libsched_la_CFLAGS = $(AM_CPPFLAGS)
libsched_la_CXXFLAGS = $(AM_CPPFLAGS)
libsched_la_LDFLAGS= -L$(libdir) -rpath $(libdir) -version-number $(LIBBOINC_VERSION)
libsched_la_LIBADD= $(SSL_LIBS)

## install only headers that are meant for exporting the API !!
if INSTALL_HEADERS
pkginclude_HEADERS = \
	credit.h \
	sched_config.h \
	sched_limit.h \
	sched_msgs.h \
	sched_util.h \
	../tools/backend_lib.h \
	validate_util.h
endif
# end of "if INSTALL_HEADERS

if ENABLE_FCGI

lib_LTLIBRARIES += libsched_fcgi.la
libsched_fcgi_la_SOURCES = $(libsched_sources)
libsched_fcgi_la_CFLAGS = -D_USING_FCGI_ $(AM_CPPFLAGS)
libsched_fcgi_la_CXXFLAGS = -D_USING_FCGI_ $(AM_CPPFLAGS)
libsched_fcgi_la_LDFLAGS= -L$(libdir) -rpath $(libdir) -version-number $(LIBBOINC_VERSION)
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

cgidir = $(libexecdir)/cgi-bin
scheddir = $(libexecdir)/sched

bin_PROGRAMS = \
    delete_file \
    get_file \
    make_work \
    sched_driver \
    put_file \
    show_shmem \
    wu_check

sched_PROGRAMS = \
    census \
	credit_test \
    db_dump \
    db_purge \
    feeder \
    file_deleter \
    antique_file_deleter \
    message_handler \
    sample_assimilator \
    sample_dummy_assimilator \
    sample_bitwise_validator \
    sample_trivial_validator \
    sample_work_generator \
    single_job_assimilator \
    transitioner \
    trickle_credit \
    trickle_echo \
    update_stats

cgi_PROGRAMS= \
    cgi \
    file_upload_handler

# scripts that 'make install' should put in bindir
bin_SCRIPTS = start stop status

noinst_HEADERS = \
    assimilate_handler.h \
    handle_request.h \
    plan_class_spec.h \
    sched_main.h \
    sched_locality.h \
    sched_score.h \
    sched_send.h \
    sched_shmem.h \
    sched_version.h \
    sched_types.h

EXTRA_DIST = \
    start

cgi_sources = \
	credit.cpp \
    edf_sim.cpp \
    handle_request.cpp \
    hr.cpp \
    hr_info.cpp \
    plan_class_spec.cpp \
    sched_main.cpp \
    sched_array.cpp \
    sched_assign.cpp \
    sched_customize.cpp \
    sched_hr.cpp \
    sched_resend.cpp \
    sched_limit.cpp \
    sched_locality.cpp \
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

db_dump_SOURCES = db_dump.cpp
db_dump_LDADD = $(SERVERLIBS)

db_purge_SOURCES = db_purge.cpp
db_purge_LDADD = $(SERVERLIBS)

trickle_credit_SOURCES = trickle_credit.cpp trickle_handler.cpp
trickle_credit_LDADD = $(SERVERLIBS)

trickle_echo_SOURCES = trickle_echo.cpp trickle_handler.cpp
trickle_echo_LDADD = $(SERVERLIBS)

update_stats_SOURCES = update_stats.cpp
update_stats_LDADD = $(SERVERLIBS)

file_upload_handler_SOURCES = file_upload_handler.cpp
file_upload_handler_LDADD = $(SERVERLIBS)

make_work_SOURCES = make_work.cpp
make_work_LDADD = $(SERVERLIBS)

transitioner_SOURCES = transitioner.cpp
transitioner_LDADD = $(SERVERLIBS)

message_handler_SOURCES = message_handler.cpp
message_handler_LDADD = $(SERVERLIBS)

get_file_SOURCES = get_file.cpp
get_file_LDADD = $(SERVERLIBS)

put_file_SOURCES = put_file.cpp
put_file_LDADD = $(SERVERLIBS)

delete_file_SOURCES = delete_file.cpp
delete_file_LDADD = $(SERVERLIBS)

sched_driver_SOURCES = sched_driver.cpp
sched_driver_LDADD = $(SERVERLIBS)

if ENABLE_FCGI

cgi_PROGRAMS += fcgi \
                fcgi_file_upload_handler

fcgi_SOURCES = $(cgi_sources)
fcgi_CPPFLAGS = -D_USING_FCGI_ $(AM_CPPFLAGS)
fcgi_LDADD = $(SERVERLIBS_FCGI)

fcgi_file_upload_handler_SOURCES = \
    file_upload_handler.cpp \
    sched_config.cpp \
    sched_msgs.cpp 
fcgi_file_upload_handler_CPPFLAGS = -D_USING_FCGI_ $(AM_CPPFLAGS)
fcgi_file_upload_handler_LDADD = $(SERVERLIBS_FCGI)

endif
# end of "if ENABLE_FCGI"
endif 
# end of "if ENABLE_SERVER"

.PHONY: PHONY-start

PHONY-start:
	@test -f start || @LN_S@ $(srcdir)/start start && test -f start

status stop: PHONY-start
	@test -f $@ || @LN_S@ start $@ && test -f $@

CLEANFILES = status stop
