## $Id$

include $(top_srcdir)/Makefile.incl

bin_PROGRAMS = create_work old_add country_select sign_executable

EXTRA_DIST = make_project add update_version

# TODO: use libboinc for these:

create_work_SOURCES =           \
    create_work.C               \
    backend_lib.C               \
    process_result_template.C   \
    ../lib/md5_file.C           \
    ../lib/md5.c                \
    ../lib/parse.C              \
    ../lib/util.C               \
    ../db/boinc_db.C            \
    ../db/db_base.C             \
    ../lib/crypt.C              \
    ../sched/sched_config.C     \
    backend_lib.h

create_work_DEPENDENCIES = $(LIBRSA)
create_work_LDADD = $(RSA_LIBS) $(MYSQL_LIBS)

## deprecated
old_add_SOURCES =				\
    add.C					\
    backend_lib.C				\
    process_result_template.C			\
    ../lib/md5_file.C				\
    ../lib/md5.c				\
    ../lib/parse.C				\
    ../lib/util.C				\
    ../db/boinc_db.C				\
    ../db/db_base.C				\
    ../lib/crypt.C				\
    ../sched/sched_config.C			\
    backend_lib.h

old_add_DEPENDENCIES = $(LIBRSA)
old_add_LDADD = $(RSA_LIBS) $(MYSQL_LIBS)

sign_executable_SOURCES =			\
        sign_executable.C			\
        ../lib/md5_file.C			\
        ../lib/md5.c				\
        ../lib/crypt.C

sign_executable_DEPENDENCIES = $(LIBRSA)
sign_executable_LDADD = $(RSA_LIBS) $(MYSQL_LIBS)

country_select_SOURCES = country_select.C ../lib/countries.C
