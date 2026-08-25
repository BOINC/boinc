# This file is part of BOINC.
# https://boinc.berkeley.edu
# Copyright (C) 2026 University of California
#
# BOINC is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
#
# BOINC is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with BOINC.  If not, see <http://www.gnu.org/licenses/>.
#

include $(top_srcdir)/Makefile.incl

scheddir = $(libexecdir)/boinc-server-maker/sched
toolsdir = $(libexecdir)/boinc-server-maker/tools
toolbindir = $(libexecdir)/boinc-server-maker/tools

toolbin_PROGRAMS = \
	cancel_jobs \
	create_work \
	dir_hier_move \
	dir_hier_path \
	remote_submit_test \
	sign_executable \
	stage_file_native

dist_toolbin_SCRIPTS = \
		batch_collect_assimilate.py \
	    boinc_submit \
		dbcheck_files_exist \
		grep_logs \
		make_project \
		parse_config \
		query_job \
		submit_batch \
		submit_buda \
		submit_job \
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
    manage_privileges \
    run_in_ops \
    stage_file

dist_tools_DATA = \
    project.xml \
    gui_urls.xml

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
sign_executable_LDADD = $(SERVERLIBS_MIN)

stage_file_native_SOURCES = stage_file_native.cpp
stage_file_native_LDADD = $(SERVERLIBS) -lz

remote_submit_test_SOURCES = remote_submit_test.cpp ../lib/remote_submit.cpp
remote_submit_test_LDADD = $(SERVERLIBS) -lcurl

clean: clean-am
	rm -f *.gcno
