include $(top_srcdir)/Makefile.incl

bin_PROGRAMS = create_work sign_executable dir_hier_path dir_hier_move

EXTRA_DIST = make_project xadd update_versions dbcheck_files_exist upgrade makelog.sh cleanlogs.sh vote_monitor

LDADD = $(LIBSCHED) $(LIBBOINC) $(RSA_LIBS) $(MYSQL_LIBS)
AM_CXXFLAGS = $(MYSQL_CFLAGS)

create_work_SOURCES =           \
    create_work.C

sign_executable_SOURCES =	\
    sign_executable.C

dir_hier_path_SOURCES = \
    dir_hier_path.C

dir_hier_move_SOURCES = \
    dir_hier_move.C
