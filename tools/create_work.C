// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/ 
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
//  -rsc_fpops n
//  -rsc_iops n
//  -rsc_memory n
//  -rsc_disk n
//  -dynamic_results
//  -wu_name name
//  -wu_template filename
//  -result_template filename
//  -nresults n
//  -keyfile path
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

#include "db.h"
#include "crypt.h"
#include "backend_lib.h"

int main(int argc, char** argv) {
    APP app;
    WORKUNIT wu;
    int retval;
    char wu_template[MAX_BLOB_SIZE];
    char result_template[MAX_BLOB_SIZE];
    char wu_template_file[256], result_template_file[256];
    char keyfile[256];
    char** infiles;
    int i, ninfiles, nresults;
    R_RSA_PRIVATE_KEY key;
    char* boinc_download_dir = getenv("BOINC_DOWNLOAD_DIR");

    srand(time(NULL));
    if (!boinc_download_dir) {
        printf("must define BOINC_DOWNLOAD_DIR");
        exit(1);
    }
    if (db_open(getenv("BOINC_DB_NAME"))) {
        printf( "Error opening database.\n" );
        exit(0);
    }
    strcpy(wu_template_file, "");
    strcpy(result_template_file, "");
    strcpy(app.name, "");
    strcpy(keyfile, "");
    nresults = 1;
    i = 1;
    ninfiles = 0;
    memset(&wu, 0, sizeof(wu));
    while (i < argc) {
        if (!strcmp(argv[i], "-appname")) {
            i++;
            strcpy(app.name, argv[i]);
        } else if (!strcmp(argv[i], "-wu_name")) {
            i++;
            strcpy(wu.name, argv[i]);
        } else if (!strcmp(argv[i], "-wu_template")) {
            i++;
            strcpy(wu_template_file, argv[i]);
        } else if (!strcmp(argv[i], "-result_template")) {
            i++;
            strcpy(result_template_file, argv[i]);
        } else if (!strcmp(argv[i], "-dynamic_results")) {
            wu.dynamic_results = true;
        } else if (!strcmp(argv[i], "-nresults")) {
            i++;
            nresults = atoi(argv[i]);
        } else if (!strcmp(argv[i], "-rsc_fpops")) {
            i++;
            wu.rsc_fpops = atof(argv[i]);
        } else if (!strcmp(argv[i], "-rsc_iops")) {
            i++;
            wu.rsc_iops = atof(argv[i]);
        } else if (!strcmp(argv[i], "-rsc_memory")) {
            i++;
            wu.rsc_memory = atof(argv[i]);
        } else if (!strcmp(argv[i], "-rsc_disk")) {
            i++;
            wu.rsc_disk = atof(argv[i]);
        } else if (!strcmp(argv[i], "-keyfile")) {
            i++;
            strcpy(keyfile, argv[i]);
        } else if (!strcmp(argv[i], "-wu_name_rand")) {
            i++;
            sprintf(wu.name, "%s_%d", argv[i], rand());
        } else {
            infiles = argv+i;
            ninfiles = argc - i;
            break;
        }
        i++;
    }
    if (!strlen(app.name) || !strlen(wu.name) || !strlen(wu_template_file)
        || !strlen(result_template_file)
    ) {
        printf("bad cmdline\n");
        exit(1);
    }
    retval = db_app_lookup_name(app);
    if (retval) {
        printf("app not found\n");
        exit(1);
    }

    retval = read_filename(wu_template_file, wu_template);
    if (retval) {
        fprintf(stderr, "can't open WU template\n");
        exit(1);
    }

#if 0
    retval = read_file(result_template_file, result_template);
    if (retval) {
        fprintf(stderr, "can't open result template\n");
        exit(1);
    }
#endif

    if (wu.dynamic_results) {
        strcpy(app.result_xml_template, result_template);
        retval = db_app_update(app);
        if (retval) printf("db_app_update: %d\n", retval);
    }

    wu.appid = app.id;


    FILE* fkey = fopen(keyfile, "r");
    rewind(fkey);
    if (!fkey) {
        printf("create_work: can't open key file (%s)\n", keyfile);
        exit(1);
    }
    retval = scan_key_hex(fkey, (KEY*)&key, sizeof(key));
    fclose(fkey);
    if (retval) {
        printf("can't parse key\n");
        exit(1);
    }

    retval = create_work(
        wu,
        wu_template,
        result_template_file,
        nresults,
        boinc_download_dir,
        infiles,
        ninfiles,
        key
    );
    if (retval) fprintf(stderr, "create_work: %d\n", retval);
    db_close();
}
