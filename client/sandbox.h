// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2007 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

extern void kill_via_switcher(int pid);
extern int get_project_gid();
extern int set_to_project_group(const char* path);
extern int switcher_exec(const char* util_filename, const char* cmdline);
extern int client_clean_out_dir(const char*);
extern int delete_project_owned_file(const char* path, bool retry);
extern int remove_project_owned_dir(const char* name);
extern int check_security(int use_sandbox, int isManager);

#define BOINC_PROJECT_GROUP_NAME "boinc_project"

extern bool g_use_sandbox;
