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

// make_work
//      -wu_name name
//      -result_template filename
//      [ -redundancy n ]
//      [ -cushion n ]
//
// Create WU and result records as needed to maintain a pool of work
// (for testing purposes).
// Makes a new WU for every "redundancy" results.
// Clones the WU of the given name.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "db.h"
#include "crypt.h"
#include "backend_lib.h"
#include "config.h"
#include "parse.h"

#define TRIGGER_FILENAME    "stop_server"

int cushion = 10;
int redundancy = 10;
char wu_name[256], result_template_file[256];

// edit a WU XML doc, replacing one filename by another
// (should appear twice, within <file_info> and <file_ref>)
// Also patch the download URL (redundant)
//
void replace_file_name(
    char* xml_doc, char* filename, char* new_filename, char* download_url
) {
    char buf[MAX_BLOB_SIZE], temp[256], download_path[256],
    new_download_path[256];
    char * p;
  
    sprintf(download_path,"%s/%s", download_url, filename);
    sprintf(new_download_path,"%s/%s", download_url, new_filename);
    strcpy(buf, xml_doc);
    p = strtok(buf,"\n");
    while (p) {
        if (parse_str(p, "<name>", temp, sizeof(temp))) {
            if(!strcmp(filename, temp)) {
                replace_element(xml_doc + (p - buf),"<name>","</name>",new_filename);
            }
        } else if (parse_str(p, "<file_name>", temp, sizeof(temp))) {
            if(!strcmp(filename, temp)) {
                replace_element(xml_doc + (p - buf),"<file_name>","</file_name>",new_filename);
            }
        } else if (parse_str(p, "<url>", temp, sizeof(temp))) {
            if(!strcmp(temp, download_path)) {
                replace_element(xml_doc + (p - buf),"<url>","</url>",new_download_path);
            }
        }
        p = strtok(0, "\n");
    }
}

void check_trigger() {
    FILE* f = fopen(TRIGGER_FILENAME, "r");
    if (!f) return;
    exit(0);
}

void make_work() {
    CONFIG config;
    char * p;
    int retval, start_time=time(0), n, nresults_left;
    char keypath[256], suffix[256], result_template[MAX_BLOB_SIZE];
    char file_name[256], buf[MAX_BLOB_SIZE], pathname[256];
    char new_file_name[256], new_pathname[256], command[256];
    char starting_xml[MAX_BLOB_SIZE], new_buf[MAX_BLOB_SIZE];
    R_RSA_PRIVATE_KEY key;
    WORKUNIT wu;
    int seqno = 0;
   
    retval = config.parse_file();
    if (retval) {
        fprintf(stderr,"make_work: can't read config file\n");
        exit(1);
    }

    retval = db_open(config.db_name, config.db_passwd);
    if (retval) {
        fprintf(stderr,"make_work: can't open db\n");
        exit(1);
    }

    strcpy(wu.name, wu_name);
    retval = db_workunit_lookup_name(wu);
    if (retval) {
        fprintf(stderr,"make_work: can't find wu %s\n", wu_name);
        exit(1);
    }

    strcpy(starting_xml,wu.xml_doc);
    
    sprintf(keypath, "%s/upload_private", config.key_dir);
    retval = read_key_file(keypath, key);
    if (retval) {
        fprintf(stderr,"make_work: can't read key\n");
        exit(1);
    }
    
    retval = read_filename(result_template_file, result_template);
    if (retval) {
        fprintf(stderr,"make_work: can't open result template\n");
        exit(1);
    }
    nresults_left = 0;
    while (true) {
        fflush(stdout);
        retval = db_result_count_server_state(RESULT_SERVER_STATE_UNSENT, n);
        if (retval) {
            fprintf(stderr,"make_work: can't counts results\n");
            exit(1);
        }
        printf("make_work: %d results available to send\n", n);
        if (n > cushion) {
            sleep(1);
            continue;
        }

        // make a new workunit every "redundancy" results
        //
        if (nresults_left == 0) {
            strcpy(buf, starting_xml);
            p = strtok(buf, "\n");
            strcpy(file_name, "");
        
            // make new copies of all the WU's input files
            //
            while (p) {
                if (parse_str(p, "<name>", file_name, sizeof(file_name))) {
                    sprintf(
                        new_file_name, "%s_%d_%d", file_name, start_time, seqno++
                    );
                    sprintf(pathname, "%s/%s", config.download_dir, file_name);
                    sprintf(
                        new_pathname, "%s/%s",config.download_dir, new_file_name
                    );
                    sprintf(command,"cp %s %s",pathname,new_pathname);
                    if (system(command)) {
                        fprintf(stderr, "make_work: ERROR\n");
                        perror(command);
                        exit(1);
                    }
                    strcpy(new_buf, starting_xml);
                    replace_file_name(
                        new_buf, file_name, new_file_name, config.download_url
                    );
                    strcpy(wu.xml_doc, new_buf);
                }
                p = strtok(0, "\n");
            }
            nresults_left = redundancy;
            sprintf(wu.name, "wu_%d_%d", start_time, seqno++);
            wu.id = 0;
            wu.create_time = time(0);
            retval = db_workunit_new(wu);
            wu.id = db_insert_id();
         
        }
        sprintf(suffix, "%d_%d", start_time, seqno++);
        create_result(
            wu, result_template, suffix, key,
            config.upload_url, config.download_url
        );
        printf("make_work: added a result\n");
        nresults_left--;
        check_trigger();
    }
}

int main(int argc, char** argv) {
    bool asynch = false;
    int i;

    unlink(TRIGGER_FILENAME);
    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-asynch")) {
            asynch = true;
        } else if (!strcmp(argv[i], "-cushion")) {
            cushion = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-result_template")) {
            strcpy(result_template_file, argv[++i]);
        } else if (!strcmp(argv[i], "-wu_name")) {
            strcpy(wu_name, argv[++i]);
        }
    }

    if (!strlen(result_template_file)) {
        fprintf(stderr,"make_work: missing -result_template\n");
        exit(1);
    }
    if (!strlen(wu_name)) {
        fprintf(stderr,"make_work: missing -wu_name\n");
        exit(1);
    }

    if (asynch) {
        if (!fork()) {
            make_work();
        }
    } else {
        make_work();
    }
}
