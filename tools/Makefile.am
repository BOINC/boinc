## $Id$

include $(top_srcdir)/Makefile.incl

bin_PROGRAMS = create_work add country_select

EXTRA_DIST = make_project

# TODO: use libboinc for these:

create_work_SOURCES =				\
	create_work.C				\
        backend_lib.C				\
        process_result_template.C		\
        ../lib/md5_file.C			\
        ../lib/md5.c				\
        ../lib/parse.C				\
        ../lib/util.C				\
	../db/boinc_db.C			\
        ../lib/crypt.C				\
	backend_lib.h

create_work_DEPENDENCIES = $(LIBRSA)
create_work_LDADD = $(RSA_LIBS) $(MYSQL_LIBS)

add_SOURCES =					\
	add.C					\
        backend_lib.C				\
        process_result_template.C		\
        ../lib/md5_file.C			\
        ../lib/md5.c				\
        ../lib/parse.C				\
        ../lib/util.C				\
	../db/boinc_db.C			\
        ../lib/crypt.C				\
	backend_lib.h

add_DEPENDENCIES = $(LIBRSA)
add_LDADD = $(RSA_LIBS) $(MYSQL_LIBS)

country_select_SOURCES = country_select.C ../lib/countries.C
