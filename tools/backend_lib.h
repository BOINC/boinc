// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
//
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License.
//
// The Original Code is the Berkeley Open Infrastructure for Network Computing.
//
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved.
//
// Contributor(s):
//

#ifndef H_BACKEND_LIB
#define H_BACKEND_LIB

#include "crypt.h"
#include "boinc_db.h"

extern int add_signatures(char*, R_RSA_PRIVATE_KEY&);
extern int remove_signatures(char*);

extern int process_result_template(
    char* result_template,
    R_RSA_PRIVATE_KEY& key,
    char* base_filename,
    char* upload_url
);

extern int read_file(FILE*, char* buf);
extern int read_filename(const char* path, char* buf, int len);

extern void initialize_result(DB_RESULT&, DB_WORKUNIT&);

extern int create_result(
    int wuid,
    int appid,
    char* wu_name,
    char* result_template_filename, 
    char* suffix,
    R_RSA_PRIVATE_KEY& key, 
    char* upload_url,
    char* query_string=0
);

extern int create_work(
    DB_WORKUNIT& wu,
    const char* wu_template,
    const char* result_template_filename,
    const char* result_template_filepath,
    const char* infile_dir,
    const char** infiles,
    int ninfiles,
    R_RSA_PRIVATE_KEY&,
    const char* upload_url,
    const char* download_url
);

#if 0
extern int create_sequence_group(
    DB_WORKUNIT& wu,
    char* wu_template,
    char* result_template,
    char* infile_dir,
    char** infiles,
    int ninfiles,
    R_RSA_PRIVATE_KEY&,
    char* upload_url,
    char* download_url,
    int nsteps
);
#endif

#endif
