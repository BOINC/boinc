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


// The BOINC file upload handler.
// A CGI processor for POST requests.
// Handles two RPC types:

// 1) Get current file size
// Request message format:
// <file_size_req>filename</file_size_req>
//
// Reply message format:
// <status>0</status>
// <nbytes>1234</nbytes>
// Where nbytes is 0 if the file doesn't exist

// 2) Upload file
// Request message format:
// <file_info>
//    ...
// <xml_signature>
//    ...
// </xml_signature>
// </file_info>
// <nbytes>x</nbytes>
// <offset>x</offset>
// <data>
// ... (data)
//
// Reply message format:
// <status>x</status>
// [ <error>bad file size</error> ]
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include "parse.h"
#include "config.h"
#include "crypt.h"

CONFIG config;

#define MAX_FILES 32

struct FILE_INFO {
    char name[256];
    double max_nbytes;
    char* xml_signature;
    char* signed_xml;
    int parse(FILE*);
};

int FILE_INFO::parse(FILE* in) {
    char buf[256];
    int retval;
    assert(in!=NULL);
    memset(this, 0, sizeof(FILE_INFO));
    signed_xml = strdup("");
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</file_info>")) return 0;
        else if (match_tag(buf, "<xml_signature>")) {
            retval = dup_element_contents(in, "</xml_signature>", &xml_signature);
            if (retval) return retval;
            continue;
        }
        strcatdup(signed_xml, buf);
        if (parse_str(buf, "<name>", name, sizeof(name))) continue;
        if (parse_double(buf, "<max_nbytes>", max_nbytes)) continue;
        //fprintf(stderr, "file_upload_handler (%s): FILE_INFO::parse: unrecognized: %s \n", config.user, buf);
    }
    return 1;
}

int print_status(int status, char* message) {
    printf("Content-type: text/plain\n\n<status>%d</status>\n", status);
    if (message) {
        printf("<error>%s</error>\n", message);
        fprintf(stderr,
            "file_upload_handler (%s): status %d: %s>\n",
            config.user_name, status, message
        );
    }
    return 0;
}

#define BLOCK_SIZE  16382

// read from socket, write to file
//
int copy_socket_to_file(FILE* in, char* path, double offset, double nbytes) {
    unsigned char buf[BLOCK_SIZE];
    FILE* out;
    int retval, n, m;
    double bytes_left;
    assert(in!=NULL);
    assert(path!=NULL);
    assert(offset>=0);
    assert(nbytes>=0);
    // printf("path is %s",path);
    out = fopen(path, "ab");
    if (!out) {
  
        print_status(-1, "can't open file");
        return -1;
    }

    // TODO: use a 64-bit variant
    
    rewind(out);
  
    retval = fseek(out, (long)offset, SEEK_CUR);
  
    if (retval) {
        fclose(out);
        print_status(-1, "can't fseek file");
        return retval;
    }
    bytes_left = nbytes - offset;
    if (bytes_left == 0) {
        fprintf(stderr, "file_upload_handler: offset == nbytes!! %f\n", nbytes);
        return 0;
    }
    while (1) {
        m = BLOCK_SIZE;
        if (m > bytes_left) m = (int)bytes_left;
        n = fread(buf, 1, m, in);
        if (n <= 0) {
            fprintf(stderr, "file_upload_handler: fread: asked for %d, return %d\n", m, n);
            print_status(-1, "can't fread socket");
            return -1;
        }
        m = fwrite(buf, 1, n, out);
        if (m != n) {
            print_status(-1, "can't fwrite file");
            return -1;
        }
        bytes_left -= n;
        if (bytes_left == 0) break;
    }
    fclose(out);
    return 0;
}

int handle_request(FILE* in, R_RSA_PUBLIC_KEY& key) {
    char buf[256];
    double nbytes=-1, offset=0;
    char path[256],file_name[256];
    FILE_INFO file_info;
    int retval;
    bool is_valid;
    assert(in!=NULL);
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "<file_info>")) {
            retval = file_info.parse(in);
            if (retval) {
                fprintf(stderr,
                    "file_upload_handler (%s): FILE_INFO.parse\n",
                    config.user_name
                );
                return retval;
            }
            retval = verify_string(
                file_info.signed_xml, file_info.xml_signature, key, is_valid
            );
            if (retval || !is_valid) {
                print_status(-1, "invalid XML signature");
		//     return -1;
            }
            continue;
        }
        // Handle a file size request
        else if (parse_str(buf, "<file_size_req>", file_name, sizeof(file_name))) {
            struct stat sbuf;
            // TODO: check to ensure path doesn't point somewhere bad
            //
            sprintf(path, "%s/%s", config.upload_dir, file_name );
            retval = stat( path, &sbuf );
            if (retval && errno != ENOENT) {
                print_status( -1, "cannot open file" );
            } else if (retval) {
                printf(
                    "Content-type: text/plain\n\n<nbytes>0</nbytes>\n"
                    "<status>0</status>\n"
                );
            } else {
                printf(
                    "Content-type: text/plain\n\n<nbytes>%d</nbytes>\n"
                    "<status>0</status>\n", (int)sbuf.st_size
                );
            }
            exit(0);
        }
        else if (parse_double(buf, "<offset>", offset)) continue;
        else if (parse_double(buf, "<nbytes>", nbytes)) continue;
        else if (match_tag(buf, "<data>")) {
            if (nbytes < 0) {
                print_status(-1, "nbytes missing or negative");
                return -1;
            }

            // enforce limits in signed XML
            if (nbytes > file_info.max_nbytes) {
                sprintf(buf,
                    "nbytes too large: %f > %f",
                    nbytes, file_info.max_nbytes
                );
                print_status(-1, buf);
                return -1;
            }

            // make sure filename is legit
            if (strstr(file_info.name, "..")) {
                fprintf(stderr,
                    "file_upload_handler: .. found in filename: %s\n",
                    file_info.name
                );
                return -1;
            }

            sprintf(path, "%s/%s", config.upload_dir, file_info.name);
            retval = copy_socket_to_file(in, path, offset, nbytes);
            if (retval) {
                fprintf(stderr,
                    "file_upload_handler (%s): copy_socket_to_file %d %s\n",
                    config.user_name, retval, path
                );
            }
            break;
        }
    }
    return 0;
}

int get_key(R_RSA_PUBLIC_KEY& key) {
    FILE* f;
    int retval;
    char buf[256];
    sprintf(buf, "%s/upload_public", config.key_dir);
    f = fopen(buf, "r");
    if (!f) return -1;
    retval = scan_key_hex(f, (KEY*)&key, sizeof(key));
    fclose(f);
    if (retval) return retval;
    return 0;
}

int main() {
    int retval;
    R_RSA_PUBLIC_KEY key;

    retval = config.parse_file();
    if (retval) {
        print_status(-1, "can't read config file");
        exit(0);
    }

    retval = get_key(key);
    if (retval) {
        print_status(-1, "can't read key file");
        exit(0);
    }
    
    retval = handle_request(stdin, key);
    if (retval) {
        fprintf(stderr, "file_upload_handler: handle_request: %d\n", retval);
    } else {
        print_status(0, 0);
    }
    return 0;
}
