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
//  -download_dir x
//  -rsc_fpops n
//  -rsc_iops n
//  -rsc_memory n
//  -rsc_disk n
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
    char wu_template_file[256], result_template_file[256];
    char keyfile[256];
    char** infiles;
    int i, ninfiles, nresults;
    R_RSA_PRIVATE_KEY key;
    char download_dir[256], db_name[256], db_passwd[256];
    char upload_url[256], download_url[256];

    strcpy(result_template_file, "");
    strcpy(app.name, "");
    strcpy(db_passwd, "");
    strcpy(keyfile, "");
    nresults = 1;
    i = 1;
    ninfiles = 0;
    memset(&wu, 0, sizeof(wu));
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
        } else if (!strcmp(argv[i], "-nresults")) {
            nresults = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-rsc_fpops")) {
            wu.rsc_fpops = atof(argv[++i]);
        } else if (!strcmp(argv[i], "-rsc_iops")) {
            wu.rsc_iops = atof(argv[++i]);
        } else if (!strcmp(argv[i], "-rsc_memory")) {
            wu.rsc_memory = atof(argv[++i]);
        } else if (!strcmp(argv[i], "-rsc_disk")) {
            wu.rsc_disk = atof(argv[++i]);
        } else if (!strcmp(argv[i], "-keyfile")) {
            strcpy(keyfile, argv[++i]);
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
        fprintf(stderr, "create_work: bad cmdline\n");
        exit(1);
    }
    if (db_open(db_name, db_passwd)) {
        fprintf(stderr, "create_work: error opening database.\n" );
        exit(0);
    }
    retval = db_app_lookup_name(app);
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

    retval = create_work(
        wu,
        wu_template,
        result_template_file,
        nresults,
        download_dir,
        infiles,
        ninfiles,
        key,
        upload_url,
        download_url
    );
    if (retval) fprintf(stderr, "create_work: %d\n", retval);
    db_close();
}
