## $Id$

include $(top_srcdir)/Makefile.incl

bin_PROGRAMS = create_work sign_executable dir_hier_path dir_hier_move

EXTRA_DIST = make_project add xadd update_versions dbcheck_files_exist upgrade

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

sign_executable_SOURCES =			\
        sign_executable.C			\
        ../lib/md5_file.C			\
        ../lib/md5.c				\
        ../lib/crypt.C

sign_executable_DEPENDENCIES = $(LIBRSA)
sign_executable_LDADD = $(RSA_LIBS) $(MYSQL_LIBS)

dir_hier_path_SOURCES = \
    dir_hier_path.C \
    ../sched/sched_config.C     \
    ../lib/parse.C \
    ../lib/util.C

dir_hier_move_SOURCES = \
    dir_hier_move.C \
    ../sched/sched_config.C     \
    ../lib/parse.C \
    ../lib/filesys.C \
    ../lib/util.C
