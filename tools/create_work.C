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

// Create a workunit.
// Input files must be in the download dir.
// See the docs for a description of WU and result template files
// This program must be run in the project's root directory,
// and there must be a valid config.xml file there
//
// create_work
//  -appname name
//  -wu_name name
//  -wu_template filename       relative to project root; usually in templates/
//  -result_template filename   relative to project root; usually in templates/
//  [ -config_dir path ]
//  [ -batch n ]
//            the following can be supplied in WU template; see defaults below
//  [ -rsc_fpops_est n ]
//  [ -rsc_fpops_bound n ]
//  [ -rsc_memory_bound n ]
//  [ -rsc_disk_bound n ]
//  [ -delay_bound x ]
//  [ -min_quorum x ]
//  [ -target_nresults x ]
//  [ -max_error_results x ]
//  [ -max_total_results x ]
//  [ -max_success_results x ]
//  infile1 infile2 ...
//

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include "boinc_db.h"
#include "crypt.h"
#include "backend_lib.h"
#include "sched_config.h"

int main(int argc, char** argv) {
    DB_APP app;
    DB_WORKUNIT wu;
    int retval;
    char wu_template[LARGE_BLOB_SIZE];
    char wu_template_file[256], result_template_file[256], result_template_path[1024];
    char keyfile[256];
    char** infiles = NULL;
    int i, ninfiles;
    R_RSA_PRIVATE_KEY key;
    char download_dir[256], db_name[256], db_passwd[256],db_user[256],db_host[256];
    char buf[256];
    SCHED_CONFIG config;

    strcpy(result_template_file, "");
    strcpy(app.name, "");
    strcpy(db_passwd, "");
    strcpy(keyfile, "");
    char* config_dir = ".";
    i = 1;
    ninfiles = 0;
    wu.clear();

    // defaults (in case not in WU template)

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

    while (i < argc) {
        if (!strcmp(argv[i], "-appname")) {
            strcpy(app.name, argv[++i]);
        } else if (!strcmp(argv[i], "-wu_name")) {
            strcpy(wu.name, argv[++i]);
        } else if (!strcmp(argv[i], "-wu_template")) {
            strcpy(wu_template_file, argv[++i]);
        } else if (!strcmp(argv[i], "-result_template")) {
            strcpy(result_template_file, argv[++i]);
        } else if (!strcmp(argv[i], "-batch")) {
            wu.batch = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-config_dir")) {
            config_dir = argv[++i];
        } else if (!strcmp(argv[i], "-batch")) {
            wu.batch = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-priority")) {
            wu.priority = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-rsc_fpops_est")) {
            wu.rsc_fpops_est = atof(argv[++i]);
        } else if (!strcmp(argv[i], "-rsc_fpops_bound")) {
            wu.rsc_fpops_bound = atof(argv[++i]);
        } else if (!strcmp(argv[i], "-rsc_memory_bound")) {
            wu.rsc_memory_bound = atof(argv[++i]);
        } else if (!strcmp(argv[i], "-rsc_disk_bound")) {
            wu.rsc_disk_bound = atof(argv[++i]);
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
    CHKARG_STR(wu.name              , "need -wu_name");
    CHKARG_STR(wu_template_file     , "need -wu_template");
    CHKARG_STR(result_template_file , "need -result_template");
#undef CHKARG
#undef CHKARG_STR

    retval = config.parse_file(config_dir);
    if (retval) {
        fprintf(stderr, "Can't parse config file: %d\n", retval);
        exit(1);
    } else {
        strcpy(db_name, config.db_name);
        strcpy(db_passwd, config.db_passwd);
        strcpy(db_user, config.db_user);
        strcpy(db_host, config.db_host);
        strcpy(download_dir, config.download_dir);
        sprintf(keyfile, "%s/upload_private", config.key_dir);
    }

    retval = boinc_db.open(db_name, db_host, db_user, db_passwd);
    if (retval) {
        fprintf(stderr, "create_work: error opening database: %d\n", retval );
        exit(1);
    }
    sprintf(buf, "where name='%s'", app.name);
    retval = app.lookup(buf);
    if (retval) {
        fprintf(stderr, "create_work: app not found\n");
        exit(1);
    }

    retval = read_filename(wu_template_file, wu_template, sizeof(wu_template));
    if (retval) {
        fprintf(stderr, "create_work: can't open WU template: %d\n", retval);
        exit(1);
    }

    wu.appid = app.id;

    retval = read_key_file(keyfile, key);
    if (retval) {
        fprintf(stderr, "create_work: can't read key: %d", retval);
        exit(1);
    }

    strcpy(result_template_path, "./");
    strcat(result_template_path, result_template_file);
    retval = create_work(
        wu,
        wu_template,
        result_template_file,
        result_template_path,
        const_cast<const char **>(infiles),
        ninfiles,
        key,
        config
    );
    if (retval) {
        fprintf(stderr, "create_work: %d\n", retval);
        exit(1);
    }
    boinc_db.close();
}

const char *BOINC_RCSID_3865dbbf46 = "$Id$";
