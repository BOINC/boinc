include $(top_srcdir)/Makefile.incl

bin_PROGRAMS = create_work sign_executable dir_hier_path dir_hier_move

EXTRA_DIST = make_project add xadd update_versions dbcheck_files_exist upgrade makelog.sh cleanlogs.sh vote_monitor

# TODO: use libboinc for these:

create_work_SOURCES =           \
    create_work.C ../sched/sched_msgs.C

create_work_LDADD = $(SCHED_LIB) $(BOINC_LIB) $(RSA_LIBS) $(MYSQL_LIBS)

sign_executable_SOURCES =			\
    sign_executable.C ../sched/sched_msgs.C

sign_executable_LDADD = $(BOINC_LIB) $(RSA_LIBS) $(MYSQL_LIBS)

dir_hier_path_SOURCES = \
    dir_hier_path.C ../sched/sched_msgs.C

dir_hier_path_LDADD = $(SCHED_LIB) $(BOINC_LIB) $(MYSQL_LIBS)

dir_hier_move_SOURCES = \
    dir_hier_move.C ../sched/sched_msgs.C

dir_hier_move_LDADD = $(SCHED_LIB) $(BOINC_LIB) $(MYSQL_LIBS)

#updater_SOURCES = updater.C
#updater_LDADD = $(BOINC_LIB)
