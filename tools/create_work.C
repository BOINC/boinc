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

// create_work
//  -appname name
//  -wu_name name
//  -wu_template filename
//  -result_template filename
//  [ -db_name x ]          // read the following from config.xml if available
//  [ -db_passwd x ]
//  [ -upload_url x ]
//  [ -download_url x ]
//  [ -download_dir x ]
//  [ -keyfile path ]
//  [ -rsc_fpops_est n ]        // see defaults below
//  [ -rsc_fpops_bound n ]
//  [ -rsc_memory_bound n ]
//  [ -rsc_disk_bound n ]
//  [ -delay_bound x ]
//  [ -min_quorum x ]
//  [ -target_nresults x ]
//  [ -max_error_results x ]
//  [ -max_total_results x ]
//  [ -max_success_results x ]
//  [ -sequence n ]
//  infile1 infile2 ...
//
// Create a workunit and results.
// Input files must be in the download dir.
// template-doc is an XML WU description file, with the following macros:
// INFILEn  gets replaced by the name of input file n
// MD5n     gets replaced by the MD5 checksum of input file n
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <values.h>

#include "boinc_db.h"
#include "crypt.h"
#include "backend_lib.h"
#include "sched_config.h"

int main(int argc, char** argv) {
    DB_APP app;
    DB_WORKUNIT wu;
    int retval;
    char wu_template[MAX_BLOB_SIZE];
    char wu_template_file[256], result_template_file[256];
    char keyfile[256];
    char** infiles = NULL;
    int i, ninfiles, sequence=0;
    R_RSA_PRIVATE_KEY key;
    char download_dir[256], db_name[256], db_passwd[256];
    char upload_url[256], download_url[256];
    char buf[256];
    SCHED_CONFIG config;

    strcpy(result_template_file, "");
    strcpy(app.name, "");
    strcpy(db_passwd, "");
    strcpy(keyfile, "");
    i = 1;
    ninfiles = 0;
    wu.clear();

    wu.min_quorum = 2;
    wu.target_nresults = 5;
    wu.max_error_results = 10;
    wu.max_total_results = 20;
    wu.max_success_results = 10;
    wu.rsc_fpops_est = 1e9;
    wu.rsc_fpops_bound =  1e10;
    wu.rsc_memory_bound = 1e8;
    wu.rsc_disk_bound = 1e8;
    wu.delay_bound = 100000;

    retval = config.parse_file();
    if (retval) {
        printf("No configure file found\n");
    } else {
        strcpy(db_name, config.db_name);
        strcpy(db_passwd, config.db_passwd);
        strcpy(download_url, config.download_url);
        strcpy(download_dir, config.download_dir);
        strcpy(upload_url, config.upload_url);
        sprintf(keyfile, "%s/upload_private", config.key_dir);
    }

    while (i < argc) {
        if (!strcmp(argv[i], "-appname")) {
            strcpy(app.name, argv[++i]);
        } else if (!strcmp(argv[i], "-db_name")) {
            strcpy(db_name, argv[++i]);
        } else if (!strcmp(argv[i], "-db_passwd")) {
            strcpy(db_passwd, argv[++i]);
        } else if (!strcmp(argv[i], "-upload_url")) {
            strcpy(upload_url, argv[++i]);
        } else if (!strcmp(argv[i], "-download_url")) {
            strcpy(download_url, argv[++i]);
        } else if (!strcmp(argv[i], "-download_dir")) {
            strcpy(download_dir, argv[++i]);
        } else if (!strcmp(argv[i], "-wu_name")) {
            strcpy(wu.name, argv[++i]);
        } else if (!strcmp(argv[i], "-wu_template")) {
            strcpy(wu_template_file, argv[++i]);
        } else if (!strcmp(argv[i], "-result_template")) {
            strcpy(result_template_file, argv[++i]);
        } else if (!strcmp(argv[i], "-rsc_fpops_est")) {
            wu.rsc_fpops_est = atof(argv[++i]);
        } else if (!strcmp(argv[i], "-rsc_fpops_bound")) {
            wu.rsc_fpops_bound = atof(argv[++i]);
        } else if (!strcmp(argv[i], "-rsc_memory_bound")) {
            wu.rsc_memory_bound = atof(argv[++i]);
        } else if (!strcmp(argv[i], "-rsc_disk_bound")) {
            wu.rsc_disk_bound = atof(argv[++i]);
        } else if (!strcmp(argv[i], "-keyfile")) {
            strcpy(keyfile, argv[++i]);
        } else if (!strcmp(argv[i], "-delay_bound")) {
            wu.delay_bound = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-min_quorum")) {
            wu.min_quorum = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-target_nresults")) {
            wu.target_nresults = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-max_error_results")) {
            wu.max_error_results = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-max_total_results")) {
            wu.max_total_results = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-max_success_results")) {
            wu.max_success_results = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-sequence")) {
            sequence = atoi(argv[++i]);
        } else {
            if (!strncmp("-",argv[i],1)) {
                fprintf(stderr, "create_work: bad argument '%s'\n", argv[i]);
                exit(1);
            }
            infiles = argv+i;
            ninfiles = argc - i;
            break;
        }
        i++;
    }

#define CHKARG(x,m) do { if (!(x)) { fprintf(stderr, "create_work: bad command line: "m"\n"); exit(1); } } while (0)
#define CHKARG_STR(v,m) CHKARG(strlen(v),m)

    CHKARG_STR(app.name             , "need -appname");
    CHKARG_STR(wu.name              , "need -wuname");
    CHKARG_STR(wu_template_file     , "need -wu_template");
    CHKARG_STR(result_template_file , "need -result_template");
    CHKARG(wu.delay_bound           , "need -delay_bound");
    CHKARG(wu.min_quorum            , "need -min_quorum");
    CHKARG(wu.target_nresults       , "need -target_nresults");
    CHKARG(wu.max_error_results     , "need -max_error_results");
    CHKARG(wu.max_total_results     , "need -max_total_results");
    CHKARG(wu.max_success_results   , "need -max_success_results");
#undef CHKARG
#undef CHKARG_STR

    if (boinc_db.open(db_name, db_passwd)) {
        fprintf(stderr, "create_work: error opening database.\n" );
        exit(0);
    }
    sprintf(buf, "where name='%s'", app.name);
    retval = app.lookup(buf);
    if (retval) {
        fprintf(stderr, "create_work: app not found\n");
        exit(1);
    }

    //fprintf(stderr, "wu_template = %s\n", wu_template);
    retval = read_filename(wu_template_file, wu_template);
    if (retval) {
        fprintf(stderr, "create_work: can't open WU template\n");
        exit(1);
    }

    wu.appid = app.id;

    retval = read_key_file(keyfile, key);
    if (retval) {
        fprintf(stderr, "create_work: can't read key");
        exit(1);
    }

    if (sequence) {
        retval = create_sequence_group(
            wu,
            wu_template,
            result_template_file,
            download_dir,
            infiles,
            ninfiles,
            key,
            upload_url,
            download_url,
            sequence
        );
    } else {
        retval = create_work(
            wu,
            wu_template,
            result_template_file,
            download_dir,
            const_cast<const char **>(infiles),
            ninfiles,
            key,
            upload_url,
            download_url
        );
        if (retval) fprintf(stderr, "create_work: %d\n", retval);
    }
    boinc_db.close();
}
