// There are two possible inputs to this program.
// One is for uploading a completed result, this looks as follows:
//
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
// The return for an uploaded result looks like
//
// <status>0</status>
// or
// <status>2</status>
// <error>bad file size</error>
//
// The other kind of input is a file size request.  This is used
// to determine how much of a file has been uploaded already.
// This kind of request should always be made before starting
// a file upload.
// The input for this looks as follows:
// <file_size_req>result_1234_file</file_size_req>
//
// The return for this information request looks like
//
// <nbytes>1234</nbytes>
//
// Where nbytes will be 0 if the file doesn't exist
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include "parse.h"
#include "crypt.h"

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
        if (parse_str(buf, "<name>", name)) continue;
        if (parse_double(buf, "<max_nbytes>", max_nbytes)) continue;
        //fprintf(stderr, "file_upload_handler: FILE_INFO::parse: unrecognized: %s \n", buf);
    }
    return 1;
}

int print_status(int status, char* message) {
    printf("Content-type: text/plain\n\n<status>%d</status>\n", status);
    if (message) printf("<error>%s</error>\n", message);
#if 0
    fprintf(stderr, "Content-type: text/plain\n\n<status>%d</status>\n", status);
    if (message) fprintf(stderr, "<error>%s</error>\n", message);
#endif
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
    out = fopen(path, "wb");
    if (!out) {
        print_status(-1, "can't open file");
        return -1;
    }

    // TODO: use a 64-bit variant
    retval = fseek(out, (long)offset, SEEK_SET);
    if (retval) {
        fclose(out);
        print_status(-1, "can't fseek file");
        return retval;
    }
    bytes_left = nbytes - offset;
    while (1) {
        m = BLOCK_SIZE;
        if (m > bytes_left) m = (int)bytes_left;
        n = fread(buf, 1, m, in);
        if (n <= 0) {
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
    double nbytes=0, offset=0;
    char path[256],file_name[256];
    FILE_INFO file_info;
    int retval;
    bool is_valid;
    assert(in!=NULL);
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "<file_info>")) {
            retval = file_info.parse(in);
            if (retval) {
		fprintf(stderr, "file_upload_handler: FILE_INFO.parse\n");
		return retval;
	    }
            retval = verify_string(
                file_info.signed_xml, file_info.xml_signature, key, is_valid
            );
            if (retval || !is_valid) {
		fprintf(stderr, "invalid XML signature\n");
                print_status(-1, "invalid XML signature");
                return -1;
            }
            continue;
        }
        // Handle a file size request
        else if (parse_str(buf, "<file_size_req>", file_name)) {
            struct stat sbuf;
            // TODO: put checking here to ensure path doesn't point somewhere bad
            sprintf( path, "%s/%s", BOINC_UPLOAD_DIR, file_name );
            retval = stat( path, &sbuf );
            if (retval && errno != ENOENT) print_status( -1, "cannot open file" );
            else if (retval) printf("Content-type: text/plain\n\n<nbytes>0</nbytes>\n");
            else printf("Content-type: text/plain\n\n<nbytes>%d</nbytes>\n", (int)sbuf.st_size);
            exit(0);
        }
        else if (parse_double(buf, "<offset>", offset)) continue;
        else if (parse_double(buf, "<nbytes>", nbytes)) continue;
        else if (match_tag(buf, "<data>")) {
            if (nbytes == 0) {
		fprintf(stderr, "nbytes missing\n");
                print_status(-1, "nbytes missing");
                return -1;
            }

            // enforce limits in signed XML
            if (nbytes > file_info.max_nbytes) {
		fprintf(stderr, "nbytes too large\n");
                sprintf(buf,
                    "nbytes too large: %f > %f",
                    nbytes, file_info.max_nbytes
                );
                print_status(-1, buf);
                return -1;
            }

            sprintf(path, "%s/%s", BOINC_UPLOAD_DIR, file_info.name);
            retval = copy_socket_to_file(in, path, offset, nbytes);
	    if(retval) {
		fprintf(stderr, "file_upload_handler: copy_socket_to_file\n");
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
    sprintf(buf, "%s/upload_public", BOINC_KEY_DIR);
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

    retval = get_key(key);
    if (retval) {
        fprintf(stderr, "file_upload_handler: can't read key file\n");
        print_status(-1, "can't read key file");
        exit(0);
    }
    
    retval = handle_request(stdin, key);
    if (retval) {
        fprintf(stderr, "file_upload_handler: handle_request: %d\n", retval);
    } else {
        //fprintf(stderr, "file_upload_handler: handle_request: %d\n", retval);
        print_status(0, 0);
    }
    return 0;
}
