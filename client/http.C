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

#include "windows_cpp.h"

#include <string.h>

#ifdef _WIN32
#include "winsock.h"
#endif
#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "error_numbers.h"
#include "filesys.h"
#include "log_flags.h"
#include "http.h"

#define HTTP_BLOCKSIZE  4096

// Breaks a url down into its server and file path components
// TODO: deal with alternate protocols (ftp, gopher, etc) or disallow
// them and parse accordingly
//
static void parse_url(char* url, char* host, char* file) {
    char* p;
    char buf[256];
    if(url==NULL) {
        fprintf(stderr, "error: parse_url: unexpected NULL pointer url\n");
    }
    if(host==NULL) {
        fprintf(stderr, "error: parse_url: unexpected NULL pointer host\n");
    }
    if(file==NULL) {
        fprintf(stderr, "error: parse_url: unexpected NULL pointer file\n");
    }
    if (strncmp(url, "http://", 7) == 0) strcpy(buf, url+7);
    else strcpy(buf, url);
    p = strchr(buf, '/');
    if (p) {
        strcpy(file, p+1);
        *p = 0;
    } else {
        strcpy(file, "");
    }
    strcpy(host, buf);
}

// Prints an HTTP 1.1 GET request header into buf
// Hopefully there won't be chunked transfers in a GET
//
static void http_get_request_header(
    char* buf, char* host, char* file, double offset
) {
    if(buf==NULL) {
        fprintf(stderr, "error: http_get_request_header: unexpected NULL pointer buf\n");
    }
    if(host==NULL) {
        fprintf(stderr, "error: http_get_request_header: unexpected NULL pointer host\n");
    }
    if(file==NULL) {
        fprintf(stderr, "error: http_get_request_header: unexpected NULL pointer file\n");
    }
    if(offset<0) {
        fprintf(stderr, "error: http_get_request_header: negative offset\n");
    }
    if (offset) {
        sprintf(buf,
            "GET /%s HTTP/1.1\015\012"
            "User-Agent: BOINC client\015\012"
            "Host: %s:80\015\012"
            "Range: bytes=%.0f-\015\012"
            "Connection: close\015\012"
            "Accept: */*\015\012"
            "\015\012",
            file, host, offset
        );
    } else {
        sprintf(buf,
            "GET /%s HTTP/1.1\015\012"
            "User-Agent: BOINC client\015\012"
            "Host: %s:80\015\012"
            "Connection: close\015\012"
            "Accept: */*\015\012"
            "\015\012",
            file,
            host
        );
    }
}

// Prints an HTTP 1.1 HEAD request header into buf
//
static void http_head_request_header(char* buf, char* host, char* file) {
    if(buf==NULL) {
        fprintf(stderr, "error: http_head_request_header: unexpected NULL pointer buf\n");
    }
    if(host==NULL) {
        fprintf(stderr, "error: http_head_request_header: unexpected NULL pointer host\n");
    }
    if(file==NULL) {
        fprintf(stderr, "error: http_head_request_header: unexpected NULL pointer file\n");
    }
    sprintf(buf,
        "HEAD /%s HTTP/1.1\015\012"
        "User-Agent: BOINC client\015\012"
        "Host: %s:80\015\012"
        "Connection: close\015\012"
        "Accept: */*\015\012"
        "\015\012",
        file, host
    );
}

// Prints an HTTP 1.0 POST request header into buf
// Use HTTP 1.0 so we don't have to deal with chunked transfers
//
static void http_post_request_header(
    char* buf, char* host, char* file, int size
) {
    if(buf==NULL) {
        fprintf(stderr, "error: http_post_request_header: unexpected NULL pointer buf\n");
    }
    if(host==NULL) {
        fprintf(stderr, "error: http_post_request_header: unexpected NULL pointer host\n");
    }
    if(file==NULL) {
        fprintf(stderr, "error: http_post_request_header: unexpected NULL pointer file\n");
    }
    if(size<0) {
        fprintf(stderr, "error: http_post_request_header: negative size\n");
    }
    sprintf(buf,
        "POST /%s HTTP/1.0\015\012"
        "Pragma: no-cache\015\012"
        "Cache-Control: no-cache\015\012"
        "Host: %s:80\015\012"
        //"Connection: close\015\012"
        "Content-Type: application/octet-stream\015\012"
        "Content-Length: %d\015\012"
        "\015\012",
        file, host, size
    );
}

#if 0
// Do we still need this?
//
void http_put_request_header(
    char* buf, char* host, char* file, int size, int offset
) {
    if(buf==NULL) {
        fprintf(stderr, "error: http_put_request_header: unexpected NULL pointer buf\n");
    }
    if(host==NULL) {
        fprintf(stderr, "error: http_put_request_header: unexpected NULL pointer host\n");
    }
    if(file==NULL) {
        fprintf(stderr, "error: http_put_request_header: unexpected NULL pointer file\n");
    }
    if(size<0) {
        fprintf(stderr, "error: http_put_request_header: negative size\n");
    }
    if(offset<0) {
        fprintf(stderr, "error: http_put_request_header: negative offset\n");
    }
    if (offset) {
        sprintf(buf,
            "PUT /%s HTTP/1.1\015\012"
            "Pragma: no-cache\015\012"
            "Cache-Control: no-cache\015\012"
            "Host: %s:80\015\012"
            "Range: bytes=%d-\015\012"
            "Connection: close\015\012"
            "Content-Type: application/octet-stream\015\012"
            "Content-Length: %d\015\012"
            "\015\012",
            file, host, offset, size
        );
    } else {
        sprintf(buf,
            "PUT /%s HTTP/1.1\015\012"
            "Pragma: no-cache\015\012"
            "Cache-Control: no-cache\015\012"
            "Host: %s:80\015\012"
            "Connection: close\015\012"
            "Content-Type: application/octet-stream\015\012"
            "Content-Length: %d\015\012"
            "\015\012",
            file,
            host, size
        );
    }
}
#endif

// Parse an http reply header into the header struct
//
int read_http_reply_header(int socket, HTTP_REPLY_HEADER& header) {
    int i, n;
    char buf[1024], *p;
    if(socket<0) {
        fprintf(stderr, "error: read_http_reply_header: negative socket\n");
        return ERR_NEG;
    }
    memset(buf, 0, sizeof(buf));
    header.content_length = 0;
    header.status = 404;        // default to failure
    for (i=0; i<1024; i++) {
        n = recv(socket, buf+i, 1, 0);
        if (strstr(buf, "\r\n\r\n") || strstr(buf, "\n\n")) {
            if (log_flags.http_debug) printf("reply header:\n%s", buf);
            p = strchr(buf, ' ');
            if (p) {
                header.status = atoi(p+1);
            }
            p = strstr(buf, "Content-Length: ");
            if (p) {
                header.content_length = atoi(p+strlen("Content-Length: "));
            }
            p = strstr(buf, "Location: ");
            if (p) {
                // TODO: Is there a better way to do this?
                n = 0;
                p += strlen( "Location: " );

                while (p[n] != '\n' && p[n] != '\r') {
                    header.redirect_location[n] = p[n];
                    n++;
                }
                p[n] = '\0';
            }
            return 0;
        }
    }
    return 1;
}

// Read the contents of the socket into buf
//
static int read_reply(int socket, char* buf, int len) {
    int i, n;
    if(socket<0) {
        fprintf(stderr, "error: read_reply: negative socket\n");
        return ERR_NEG;
    }
    if(buf==NULL) {
        fprintf(stderr, "error: read_reply: unexpected NULL pointer buf\n");
        return ERR_NULL;
    }
    if(len<0) {
        fprintf(stderr, "error: read_reply: negative len\n");
        return ERR_NEG;
    }
    for (i=0; i<len-1; i++) {
        n = recv(socket, buf+i, 1, 0);
        if (n != 1) break;
    }
    buf[i] = 0;
    return 0;
}

HTTP_OP::HTTP_OP() {
    http_op_state = HTTP_STATE_IDLE;
    strcpy(hostname,"");
    strcpy(filename,"");
    req1 = NULL;
    strcpy(infile,"");
    strcpy(outfile,"");
    content_length = 0;
    file_offset = 0;
    strcpy(request_header,"");
    http_op_state = HTTP_STATE_IDLE;
    http_op_type = HTTP_OP_NONE;
    http_op_retval = 0;
}

HTTP_OP::~HTTP_OP() {
}

// Initialize HTTP HEAD operation to url
//
int HTTP_OP::init_head(char* url) {
    if(url==NULL) {
        fprintf(stderr, "error: HTTP_OP.init_head: unexpected NULL pointer url\n");
        return ERR_NULL;
    }
    parse_url(url, hostname, filename);
    NET_XFER::init(hostname, 80, HTTP_BLOCKSIZE);
    http_op_type = HTTP_OP_HEAD;
    http_op_state = HTTP_STATE_CONNECTING;
    http_head_request_header(request_header, hostname, filename);
    return 0;
}

// Initialize HTTP GET operation to url
//
int HTTP_OP::init_get(char* url, char* out, bool del_old_file, double off) {
    if(url==NULL) {
        fprintf(stderr, "error: HTTP_OP.init_get: unexpected NULL pointer url\n");
        return ERR_NULL;
    }
    if(out==NULL) {
        fprintf(stderr, "error: HTTP_OP.init_get: unexpected NULL pointer out\n");
        return ERR_NULL;
    }
    if(off<0) {
        fprintf(stderr, "error: HTTP_OP.init_get: negative off\n"); 
        return ERR_NEG;
    }
    if (del_old_file) {
        unlink(out);
    }
    file_offset = off;
    parse_url(url, hostname, filename);
    NET_XFER::init(hostname, 80, HTTP_BLOCKSIZE);
    strcpy(outfile, out);
    http_op_type = HTTP_OP_GET;
    http_op_state = HTTP_STATE_CONNECTING;
    http_get_request_header(request_header, hostname, filename, (int)file_offset);
    return 0;
}

// Initialize HTTP POST operation to url
//
int HTTP_OP::init_post(char* url, char* in, char* out) {
    int retval;
    if(url==NULL) {
        fprintf(stderr, "error: HTTP_OP.init_post: unexpected NULL pointer url\n");
        return ERR_NULL;
    }
    if(in==NULL) {
        fprintf(stderr, "error: HTTP_OP.init_post: unexpected NULL pointer in\n");
        return ERR_NULL;
    }
    if(out==NULL) {
        fprintf(stderr, "error: HTTP_OP.init_post: unexpected NULL pointer out\n");
	return ERR_NULL;
    }
    parse_url(url, hostname, filename);
    NET_XFER::init(hostname, 80, HTTP_BLOCKSIZE);
    strcpy(infile, in);
    strcpy(outfile, out);
    retval = file_size(infile, content_length);
    if (retval) return retval;
    http_op_type = HTTP_OP_POST;
    http_op_state = HTTP_STATE_CONNECTING;
    http_post_request_header(
        request_header, hostname, filename, content_length
    );
    return 0;
}

// Initialize HTTP POST operation to url including file offset
//
int HTTP_OP::init_post2(
    char* url, char* r1, char* in, double offset
) {
    int retval;
    if(url==NULL) {
        fprintf(stderr, "error: HTTP_OP.init_post2: unexpected NULL pointer url\n");
        return ERR_NULL;
    }
    if(r1==NULL) {
	fprintf(stderr, "error: HTTP_OP.init_post2: unexpected NULL pointer r1\n");
        return ERR_NULL;
    }
    if(offset<0) {
        fprintf(stderr, "error: HTTP_OP.init_post2: negative offset\n");
        return ERR_NEG;
    }
    parse_url(url, hostname, filename);
    NET_XFER::init(hostname, 80, HTTP_BLOCKSIZE);
    req1 = r1;
    if (in) {
        strcpy(infile, in);
        file_offset = offset;
        retval = file_size(infile, content_length);
        if (retval) return retval;
        content_length -= (int)offset;
    }
    content_length += strlen(req1);
    http_op_type = HTTP_OP_POST2;
    http_op_state = HTTP_STATE_CONNECTING;
    http_post_request_header(
        request_header, hostname, filename, content_length
    );
    return 0;
}

#if 0
// Is this still needed?
int HTTP_OP::init_put(char* url, char* in, int off) {
    int retval;

    offset = off;
    parse_url(url, hostname, filename);
    NET_XFER::init(hostname, 80, HTTP_BLOCKSIZE);
    strcpy(infile, in);
    retval = file_size(infile, content_length);
    if (retval) return retval;
    http_op_type = HTTP_OP_PUT;
    http_op_state = HTTP_STATE_CONNECTING;
    http_put_request_header(
        request_header, hostname, filename, content_length, offset
    );
    return 0;
}
#endif

// Returns true if the HTTP operation is complete
//
bool HTTP_OP::http_op_done() {
    return (http_op_state == HTTP_STATE_DONE);
}

HTTP_OP_SET::HTTP_OP_SET(NET_XFER_SET* p) {
    if(p==NULL) {
        fprintf(stderr, "error: HTTP_OP_SET: unexpected NULL pointer p\n");
    }
    net_xfers = p;
}

// Inserts an HTTP_OP into the set
//
int HTTP_OP_SET::insert(HTTP_OP* ho) {
    int retval;
    if(ho==NULL) {
        fprintf(stderr, "error: HTTP_OP_SET.insert: unexpected NULL pointer ho\n");
        return ERR_NULL;
    }
    retval = net_xfers->insert(ho);
    if (retval) return retval;
    http_ops.push_back(ho);
    return 0;
}

// Runs through the set of HTTP_OP objects in the set and decides
// what operation is appropriate for each.  This section needs more
// thorough documentation, as it is fairly complex
//
bool HTTP_OP_SET::poll() {
    unsigned int i;
    HTTP_OP* htp;
    int n;
    bool action = false;

    for (i=0; i<http_ops.size(); i++) {
        htp = http_ops[i];
        switch(htp->http_op_state) {
        case HTTP_STATE_CONNECTING:
            // If the op is in the connecting state, and we notice it is done
            // connecting, move it to the HTTP_STATE_REQUEST_HEADER state
            if (htp->is_connected) {
                htp->http_op_state = HTTP_STATE_REQUEST_HEADER;
                htp->want_upload = true;
                action = true;
            }
            break;
        case HTTP_STATE_REQUEST_HEADER:
            if (htp->io_ready) {
                action = true;
                n = send(htp->socket, htp->request_header, strlen(htp->request_header), 0);
                if (log_flags.http_debug) {
                    printf("wrote HTTP header: %d bytes\n", n);
                }
                htp->io_ready = false;
                switch(htp->http_op_type) {
                case HTTP_OP_POST:
                //case HTTP_OP_PUT:
                    htp->http_op_state = HTTP_STATE_REQUEST_BODY;
                    htp->file = fopen(htp->infile, "r");
                    if (!htp->file) {
                        fprintf(stderr, "HTTP_OP: no input file %s\n", htp->infile);
                        htp->io_done = true;
                        htp->http_op_retval = ERR_FOPEN;
                        htp->http_op_state = HTTP_STATE_DONE;
                        break;
                    }
                    htp->do_file_io = true;
                    break;
                case HTTP_OP_GET:
                case HTTP_OP_HEAD:
                    htp->http_op_state = HTTP_STATE_REPLY_HEADER;
                    htp->want_upload = false;
                    htp->want_download = true;
                    break;
                case HTTP_OP_POST2:
                    htp->http_op_state = HTTP_STATE_REQUEST_BODY1;
                    break;
                }
            }
            break;
        case HTTP_STATE_REQUEST_BODY1:
            if (htp->io_ready) {
                action = true;
                n = send(htp->socket, htp->req1, strlen(htp->req1), 0);
                htp->http_op_state = HTTP_STATE_REQUEST_BODY;
                // If there's a file we also want to send, then start transferring
                // it, otherwise, go on to the next step
                if (htp->infile && strlen(htp->infile) > 0) {
                    htp->file = fopen(htp->infile, "r");
                    if (!htp->file) {
                        fprintf(stderr, "HTTP_OP: no input2 file %s\n", htp->infile);
                        htp->io_done = true;
                        htp->http_op_retval = ERR_FOPEN;
                        htp->http_op_state = HTTP_STATE_DONE;
                        break;
                    }
                    fseek(htp->file, (long)htp->file_offset, SEEK_SET);
                    htp->do_file_io = true;
                } else {
                    htp->io_done = true;
                    htp->do_file_io = false;
                }
                htp->io_ready = false;
            }
            break;
        case HTTP_STATE_REQUEST_BODY:
            if (htp->io_done) {
                action = true;
                if (log_flags.http_debug) {
                    printf("finished sending request body\n");
                }
                htp->http_op_state = HTTP_STATE_REPLY_HEADER;
                fclose(htp->file);
                htp->file = 0;
                htp->do_file_io = false;
                htp->want_upload = false;
                htp->want_download = true;
                htp->io_ready = false;
            }
        case HTTP_STATE_REPLY_HEADER:
            if (htp->io_ready) {
                action = true;
                if (log_flags.http_debug) printf("got reply header\n");
                read_http_reply_header(htp->socket, htp->hrh);
                // TODO: handle all kinds of redirects here
                if (htp->hrh.status == 301 || htp->hrh.status == 302) {
                    // Close the old socket
                    htp->close_socket();
                    switch (htp->http_op_type) {
                        case HTTP_OP_HEAD:
                            htp->init_head( htp->hrh.redirect_location );
                            break;
                        case HTTP_OP_GET:
                            // *** Not sure if delete_old_file should be true
                            htp->init_get( htp->hrh.redirect_location, htp->outfile, true );
                            break;
                        case HTTP_OP_POST:
                            htp->init_post( htp->hrh.redirect_location, htp->infile, htp->outfile );
                            break;
                        case HTTP_OP_POST2:
                            // TODO: Change offset to correct value
                            htp->init_post2( htp->hrh.redirect_location, htp->req1, htp->infile, 0 );
                            break;
                    }
                    // Open connection to the redirected server
                    htp->open_server();
                    break;
                }
                if (htp->hrh.status/100 != 2) {
                    htp->http_op_state = HTTP_STATE_DONE;
                    htp->http_op_retval = htp->hrh.status;
                    break;
                }
                switch (htp->http_op_type) {
                case HTTP_OP_HEAD:
                    htp->http_op_state = HTTP_STATE_DONE;
                    htp->http_op_retval = 0;
                    break;
                case HTTP_OP_POST:
                    unlink(htp->outfile);
                case HTTP_OP_GET:
                    htp->http_op_state = HTTP_STATE_REPLY_BODY;
                  
                    // TODO:
                    // Append to a file if it already exists, otherwise
                    // create a new one.  init_get should have already
                    // deleted the file if necessary
                    htp->file = fopen(htp->outfile, "a");
                    if (!htp->file) {
                        fprintf(stderr,
                            "HTTP_OP: can't open output file %s\n",
                            htp->outfile
                        );
                        htp->io_done = true;
                        htp->http_op_state = HTTP_STATE_DONE;
                        htp->http_op_retval = ERR_FOPEN;
                        break;
                    }
                    htp->do_file_io = true;
                    break;
                case HTTP_OP_POST2:
                    htp->http_op_state = HTTP_STATE_REPLY_BODY;
                    htp->io_ready = false;
                    break;
#if 0
                case HTTP_OP_PUT:
                    htp->http_op_state = HTTP_STATE_DONE;
                    htp->http_op_retval = 0;
#endif
                }
            }
            break;
        case HTTP_STATE_REPLY_BODY:
            if (htp->io_done) {
                switch(htp->http_op_type) {
                case HTTP_OP_POST2:
                    read_reply(htp->socket, htp->req1, 256);
                    fprintf( stderr, "%s\n", htp->req1 );
                    // parse reply here?
                    break;
                default:
                    action = true;
                    fclose(htp->file);
                    htp->file = 0;
                    break;
                }
                if (log_flags.http_debug) printf("got reply body\n");
                htp->http_op_state = HTTP_STATE_DONE;
                htp->http_op_retval = 0;
            }
            break;
        }
    }
    return action;
}

// Remove an HTTP_OP from the set
//
int HTTP_OP_SET::remove(HTTP_OP* p) {
    vector<HTTP_OP*>::iterator iter;
    if(p==NULL) {
        fprintf(stderr, "error: HTTP_OP_SET.remove: unexpected NULL pointer p\n");
        return ERR_NULL;
    }
    net_xfers->remove(p);

    iter = http_ops.begin();
    while (iter != http_ops.end()) {
    	if (*iter == p) {
	    http_ops.erase(iter);
	    return 0;
	}
	iter++;
    }
    fprintf(stderr, "HTTP_OP_SET::remove(): not found\n");
    return 1;
}

// Returns the size of the set of HTTP_OP objects
//
int HTTP_OP_SET::size() {
    return http_ops.size();
}
