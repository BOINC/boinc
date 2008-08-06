// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

#ifndef H_BACKEND_LIB
#define H_BACKEND_LIB

#include <limits.h>

#include "crypt.h"
#include "sched_config.h"
#include "boinc_db.h"

extern int add_signatures(char*, R_RSA_PRIVATE_KEY&);
extern int remove_signatures(char*);

extern int process_result_template(
    char* result_template,
    R_RSA_PRIVATE_KEY& key,
    char* base_filename,
    SCHED_CONFIG&
);

extern int read_file(FILE*, char* buf);
extern int read_filename(const char* path, char* buf, int len);

extern void initialize_result(DB_RESULT&, DB_WORKUNIT&);

extern int create_result(
    WORKUNIT&,
    char* result_template_filename, 
    char* suffix,
    R_RSA_PRIVATE_KEY& key, 
    SCHED_CONFIG& config,
    char* query_string=0,
    int priority_increase=0
);

extern int create_result_ti(
    TRANSITIONER_ITEM&,
    char* result_template_filename, 
    char* suffix,
    R_RSA_PRIVATE_KEY& key, 
    SCHED_CONFIG& config,
    char* query_string=0,
    int priority_increase=0
);

extern int create_work(
    DB_WORKUNIT& wu,
    const char* wu_template,
    const char* result_template_filename,
    const char* result_template_filepath,
    const char** infiles,
    int ninfiles,
    SCHED_CONFIG&,
    const char* command_line = NULL,
    const char* additional_xml = NULL
);

#endif
