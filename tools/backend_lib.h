// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
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
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef H_BACKEND_LIB
#define H_BACKEND_LIB

#include "crypt.h"
#include "sched_config.h"
#include "boinc_db.h"

extern int add_signatures(char*, R_RSA_PRIVATE_KEY&);
extern int remove_signatures(char*);

extern int process_result_template(
    char* result_template,
    R_RSA_PRIVATE_KEY& key,
    char* base_filename,
    SCHED_CONFIG& config
);

extern int read_file(FILE*, char* buf);
extern int read_filename(const char* path, char* buf, int len);

extern void initialize_result(DB_RESULT&, DB_WORKUNIT&);

extern int create_result(
    TRANSITIONER_ITEM&,
    char* result_template_filename, 
    char* suffix,
    R_RSA_PRIVATE_KEY& key, 
    SCHED_CONFIG& config,
    char* query_string=0
);

extern int create_work(
    DB_WORKUNIT& wu,
    const char* wu_template,
    const char* result_template_filename,
    const char* result_template_filepath,
    const char** infiles,
    int ninfiles,
    R_RSA_PRIVATE_KEY&,
    SCHED_CONFIG&
);

#endif
