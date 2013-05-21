## -*- mode: makefile; tab-width: 4 -*-
include $(top_srcdir)/Makefile.incl

scheddir = $(prefix)/lib/boinc-server-maker/sched
toolsdir = $(prefix)/lib/boinc-server-maker/tools
toolbindir = $(prefix)/lib/boinc-server-maker/tools

toolbin_PROGRAMS = create_work sign_executable dir_hier_path dir_hier_move cancel_jobs

dist_toolbin_SCRIPTS = \
	    boinc_submit \
		dbcheck_files_exist \
		demo_submit \
		demo_query \
		grep_logs \
		make_project \
		parse_config \
		update_versions \
		upgrade \
		vote_monitor \
		xadd

dist_sched_SCRIPTS = \
		cleanlogs.sh \
		makelog.sh \
		run_in_ops

dist_tools_SCRIPTS = \
    appmgr \
    db_query \
    manage_privileges \
    run_in_ops \
    stage_file \
    watch_tcp \
    project.xml

AM_CXXFLAGS += $(MYSQL_CFLAGS)
AM_LDFLAGS += -static

cancel_jobs_SOURCES = cancel_jobs.cpp
cancel_jobs_LDADD = $(SERVERLIBS)

create_work_SOURCES = create_work.cpp
create_work_LDADD = $(SERVERLIBS)

dir_hier_path_SOURCES = dir_hier_path.cpp
dir_hier_path_LDADD = $(SERVERLIBS)

dir_hier_move_SOURCES = dir_hier_move.cpp
dir_hier_move_LDADD = $(SERVERLIBS)

sign_executable_SOURCES = sign_executable.cpp
sign_executable_LDADD = $(SERVERLIBS)
