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
// Portions created by the SETI@home project are Copyright (C) 2002, 2003
// University of California at Berkeley. All Rights Reserved.
//
// Contributor(s):
//


// The BOINC file upload handler.
// See doc/upload.html for protocol spec.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include "parse.h"
#include "util.h"
#include "config.h"
#include "sched_util.h"
#include "crypt.h"

CONFIG config;

#define ERR_TRANSIENT   true
#define ERR_PERMANENT   false

#define DEBUG_LEVEL     1

#define STDERR_FILENAME "file_upload_handler.out"

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
        if (match_tag(buf, "<generated_locally/>")) continue;
        if (match_tag(buf, "<upload_when_present/>")) continue;
        if (match_tag(buf, "<url>")) continue;
        write_log(MSG_NORMAL, "FILE_INFO::parse: unrecognized: %s \n", buf);
    }
    return 1;
}

int return_error(bool transient, char* message) {
    printf(
        "Content-type: text/plain\n\n"
        "<data_server_reply>\n"
        "    <status>%d</status>\n"
        "    <message>%s</message>\n"
        "</data_server_reply>\n",
        transient?1:-1,
        message
        );
    write_log(MSG_DEBUG, "Returning error to client: %s (%s)\n", message,
              (transient?"transient":"permanent")
        );
    return 1;
}

int return_success(char* text) {
    printf(
        "Content-type: text/plain\n\n"
        "<data_server_reply>\n"
        "    <status>0</status>\n"
    );
    if (text) {
        printf("    %s\n", text);
    }
    printf("</data_server_reply>\n");
    return 0;
}

#define BLOCK_SIZE  16382

// read from socket, write to file
//
int copy_socket_to_file(FILE* in, char* path, double offset, double nbytes) {
    unsigned char buf[BLOCK_SIZE];
    char buf2[256];
    FILE* out;
    int retval, n, m;
    double bytes_left;

    out = fopen(path, "ab");
    if (!out) {
        return return_error(ERR_TRANSIENT, "can't open file");
    }

    // TODO: use a 64-bit variant

    retval = fseek(out, (long)offset, SEEK_CUR);

    if (retval) {
        fclose(out);
        return return_error(ERR_TRANSIENT, "can't fseek file");
    }
    bytes_left = nbytes - offset;
    if (bytes_left == 0) {
        fclose(out);
        write_log(MSG_DEBUG, "offset == nbytes: %f\n", nbytes);
        return return_success(0);
    }
    while (1) {
        m = BLOCK_SIZE;
        if (m > bytes_left) m = (int)bytes_left;
        n = fread(buf, 1, m, in);
        if (n <= 0) {
            fclose(out);
            sprintf(buf2, "fread: asked for %d, got %d", m, n);
            return return_error(ERR_TRANSIENT, buf2);
        }
        m = fwrite(buf, 1, n, out);
        if (m != n) {
            fclose(out);
            return return_error(ERR_TRANSIENT, "can't fwrite file");
        }
        bytes_left -= n;
        if (bytes_left == 0) break;
    }
    fclose(out);
    return 0;
}

// read from socket, discard data
//
void copy_socket_to_null(FILE* in) {
    unsigned char buf[BLOCK_SIZE];

    while (1) {
        int n = fread(buf, 1, BLOCK_SIZE, in);
        if (n <= 0) return;
    }
}

int handle_file_upload(FILE* in, R_RSA_PUBLIC_KEY& key) {
    char buf[256], path[256];
    FILE_INFO file_info;
    int retval;
    double nbytes=-1, offset=0;
    bool is_valid;

    while (fgets(buf, 256, in)) {
        // TODO: indent
        write_log(MSG_DEBUG, buf);
        if (match_tag(buf, "<file_info>")) {
            retval = file_info.parse(in);
            if (retval) {
                return return_error(ERR_PERMANENT, "FILE_INFO::parse");
            }
            retval = verify_string(
                file_info.signed_xml, file_info.xml_signature, key, is_valid
            );
            if (retval || !is_valid) {
                write_log(MSG_NORMAL,
                    "signed xml:\n%s"
                    "signature:\n%s",
                    file_info.signed_xml, file_info.xml_signature
                );
                return return_error(ERR_PERMANENT, "invalid signature");
            }
            continue;
        }
        else if (parse_double(buf, "<offset>", offset)) continue;
        else if (parse_double(buf, "<nbytes>", nbytes)) continue;
        else if (match_tag(buf, "<data>")) {
            if (nbytes < 0) {
                return return_error(ERR_PERMANENT, "nbytes missing or negative");
            }

            // enforce limits in signed XML
            //
            if (nbytes > file_info.max_nbytes) {
                sprintf(buf,
                    "file size (%d KB) exceeds limit (%d KB)",
                    (int)(nbytes/1024), (int)(file_info.max_nbytes/1024)
                );
                copy_socket_to_null(in);
                return return_error(ERR_PERMANENT, buf);
            }

            // make sure filename is legit
            //
            if (strstr(file_info.name, "..")) {
                sprintf(buf,
                    "file_upload_handler: .. found in filename: %s",
                    file_info.name
                );
                return return_error(ERR_PERMANENT, buf);
            }

            sprintf(path, "%s/%s", config.upload_dir, file_info.name);
            retval = copy_socket_to_file(in, path, offset, nbytes);
            if (!retval) {
                return_success(0);
            }
            break;
        }
    }
    return 0;
}

int handle_get_file_size(char* file_name) {
    struct stat sbuf;
    char path[256], buf[256];
    int retval;

    // TODO: check to ensure path doesn't point somewhere bad
    // Use 64-bit variant
    //
    sprintf(path, "%s/%s", config.upload_dir, file_name );
    retval = stat( path, &sbuf );
    if (retval && errno != ENOENT) {
        write_log(MSG_NORMAL, "handle_get_file_size: %s, returning error\n", file_name);
        return return_error(ERR_TRANSIENT, "cannot open file" );
    } else if (retval) {
        write_log(MSG_NORMAL, "handle_get_file_size: %s, returning zero\n", file_name);
        return return_success("<file_size>0</file_size>");
    } else {
        write_log(MSG_NORMAL, "handle_get_file_size: %s, returning %d\n",
                file_name, (int)sbuf.st_size);
        sprintf(buf, "<file_size>%d</file_size>", (int)sbuf.st_size);
        return return_success(buf);
    }
    return 0;
}

int handle_request(FILE* in, R_RSA_PUBLIC_KEY& key) {
    char buf[256];
    char file_name[256];
    int major;
    bool got_version = false;

    while (fgets(buf, 256, in)) {
        // TODO: indent
        write_log(MSG_DEBUG, buf);
        if (parse_int(buf, "<core_client_major_version>", major)) {
            if (major != MAJOR_VERSION) {
                sprintf(buf,
                    "Core client has major version %d; "
                    "expected %d.",
                    major, MAJOR_VERSION
                );
                return return_error(ERR_PERMANENT, buf);
            } else {
                got_version = true;
            }
        } else if (match_tag(buf, "<file_upload>")) {
            if (!got_version) {
                return return_error(ERR_PERMANENT, "Missing version");
            } else {
                return handle_file_upload(in, key);
            }
        } else if (parse_str(buf, "<get_file_size>", file_name, sizeof(file_name))) {
            if (!got_version) {
                return return_error(ERR_PERMANENT, "Missing version");
            } else {
                return handle_get_file_size(file_name);
            }
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

    if (!freopen(STDERR_FILENAME, "a", stderr)) {
        fprintf(stderr, "Can't redirect stderr\n");
        exit(1);
    }

    set_debug_level(DEBUG_LEVEL);

    retval = config.parse_file();
    if (retval) {
        return_error(ERR_TRANSIENT, "can't read config file");
        exit(1);
    }

    retval = get_key(key);
    if (retval) {
        return_error(ERR_TRANSIENT, "can't read key file");
        exit(1);
    }

    handle_request(stdin, key);
    return 0;
}
