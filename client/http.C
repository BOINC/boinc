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
#include "util.h"
#include "message.h"

#include "http.h"

#define HTTP_BLOCKSIZE  16384

// Breaks a HTTP url down into its server and file path components
//
void parse_url(char* url, char* host, int &port, char* file) {
    char* p;
    char buf[256];

    if (strncmp(url, "http://", 7) == 0) {
        safe_strcpy(buf, url+7);
    } else {
        safe_strcpy(buf, url);
    }
    p = strchr(buf, '/');
    if (p) {
        strcpy(file, p+1);
        *p = 0;
    } else {
        strcpy(file, "");
    }
    p=strchr(buf,':');
    if (p) {
        port = atol(p+1);
        *p = 0;
    } else {
        port=80;
    }
    strcpy(host, buf);
}

// Prints an HTTP 1.1 GET request header into buf
// Hopefully there won't be chunked transfers in a GET
//
static void http_get_request_header(
    char* buf, char* host, int port, char* file, double offset
) {
    char offset_info[256];
    if (offset) sprintf( offset_info, "Range: bytes=%.0f-\015\012", offset );
    sprintf(buf,
        "GET %s HTTP/1.0\015\012"
        "User-Agent: BOINC client\015\012"
        "Host: %s:%d\015\012"
        "%s"
        "Connection: close\015\012"
        "Accept: */*\015\012"
        "\015\012",
        file, host, port, offset?offset_info:""
    );
}

// Prints an HTTP 1.1 HEAD request header into buf
//
static void http_head_request_header(char* buf, char* host, int port, char* file) {
    sprintf(buf,
        "HEAD %s HTTP/1.0\015\012"
        "User-Agent: BOINC client\015\012"
        "Host: %s:%d\015\012"
        "Connection: close\015\012"
        "Accept: */*\015\012"
        "\015\012",
        file, host, port
    );
}

// Prints an HTTP 1.0 POST request header into buf
// Use HTTP 1.0 so we don't have to deal with chunked transfers
//
static void http_post_request_header(
    char* buf, char* host, int port, char* file, int size
) {
    sprintf(buf,
        "POST %s HTTP/1.0\015\012"
        "Pragma: no-cache\015\012"
        "Cache-Control: no-cache\015\012"
        "Host: %s:%d\015\012"
        "Connection: close\015\012"
        "Content-Type: application/octet-stream\015\012"
        "Content-Length: %d\015\012"
        "\015\012",
        file, host, port, size
    );
}

// Parse an http reply header into the header struct
//
int read_http_reply_header(int socket, HTTP_REPLY_HEADER& header) {
    int i, n;
    char buf[1024], *p;

    ScopeMessages scope_messages(log_messages, ClientMessages::DEBUG_HTTP);

    memset(buf, 0, sizeof(buf));
    header.content_length = 0;
    header.status = 404;        // default to failure
    for (i=0; i<1024; i++) {
        n = recv(socket, buf+i, 1, 0);
        if (strstr(buf, "\r\n\r\n") || strstr(buf, "\n\n")) {
            // scope_messages.printf("read_http_reply_header(): reply header on socket %d:\n", socket);
            scope_messages.printf_multiline(buf, "read_http_reply_header(): header: ");
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
                header.redirect_location[n] = 0;
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
    for (i=0; i<len-1; i++) {
        n = recv(socket, buf+i, 1, 0);
        if (n != 1) break;
    }
    buf[i] = 0;
    return 0;
}

HTTP_OP::HTTP_OP() {
    strcpy(hostname, "");
    strcpy(filename, "");
    req1 = NULL;
    strcpy(infile, "");
    strcpy(outfile, "");
    content_length = 0;
    file_offset = 0;
    strcpy(request_header, "");
    http_op_state = HTTP_STATE_IDLE;
    http_op_type = HTTP_OP_NONE;
    http_op_retval = 0;
    use_http_proxy = false;
    proxy_server_port = 0;
    strcpy(proxy_server_name, "");
}

HTTP_OP::~HTTP_OP() {
}

// Initialize HTTP HEAD operation
//
int HTTP_OP::init_head(char* url) {
    char proxy_buf[256];
    parse_url(url, hostname, port, filename);
    NET_XFER::init(use_http_proxy?proxy_server_name:hostname, use_http_proxy?proxy_server_port:port, HTTP_BLOCKSIZE);
    http_op_type = HTTP_OP_HEAD;
    http_op_state = HTTP_STATE_CONNECTING;
    if (use_http_proxy) {
        sprintf( proxy_buf, "http://%s:%d/%s", hostname, port, filename );
    } else {
        sprintf( proxy_buf, "/%s", filename );
    }
    http_head_request_header(request_header, hostname, port, proxy_buf);
    return 0;
}

// Initialize HTTP GET operation
//
int HTTP_OP::init_get(char* url, char* out, bool del_old_file, double off) {
    char proxy_buf[256];

    if (del_old_file) {
        unlink(out);
    }
    file_offset = off;
    parse_url(url, hostname, port, filename);
    NET_XFER::init(use_http_proxy?proxy_server_name:hostname, use_http_proxy?proxy_server_port:port, HTTP_BLOCKSIZE);
    safe_strcpy(outfile, out);
    http_op_type = HTTP_OP_GET;
    http_op_state = HTTP_STATE_CONNECTING;
    if (use_http_proxy) {
        sprintf( proxy_buf, "http://%s:%d/%s", hostname, port, filename );
    } else {
        sprintf( proxy_buf, "/%s", filename );
    }
    http_get_request_header(request_header, hostname, port, proxy_buf, (int)file_offset);
    return 0;
}

// Initialize HTTP POST operation
//
int HTTP_OP::init_post(char* url, char* in, char* out) {
    int retval;
    double size;
    char proxy_buf[256];

    ScopeMessages scope_messages(log_messages, ClientMessages::DEBUG_HTTP);

    parse_url(url, hostname, port, filename);
    NET_XFER::init(use_http_proxy?proxy_server_name:hostname, use_http_proxy?proxy_server_port:port, HTTP_BLOCKSIZE);
    safe_strcpy(infile, in);
    safe_strcpy(outfile, out);
    retval = file_size(infile, size);
    if (retval) return retval;
    content_length = (int)size;
    http_op_type = HTTP_OP_POST;
    http_op_state = HTTP_STATE_CONNECTING;
    if (use_http_proxy) {
        sprintf( proxy_buf, "http://%s:%d/%s", hostname, port, filename );
    } else {
        sprintf( proxy_buf, "/%s", filename );
    }
    http_post_request_header(
        request_header, hostname, port, proxy_buf, content_length
        );
    scope_messages.printf("HTTP_OP::init_post(): %p io_done %d\n", this, io_done);
    return 0;
}

// Initialize HTTP POST operation
//
int HTTP_OP::init_post2(
    char* url, char* r1, char* in, double offset
) {
    int retval;
    double size;
    char proxy_buf[256];

    parse_url(url, hostname, port, filename);
    NET_XFER::init(use_http_proxy?proxy_server_name:hostname, use_http_proxy?proxy_server_port:port, HTTP_BLOCKSIZE);
    req1 = r1;
    if (in) {
        safe_strcpy(infile, in);
        file_offset = offset;
        retval = file_size(infile, size);
        if (retval) {
            printf("HTTP::init_post2: couldn't get file size\n");
            return retval;
        }
        content_length = (int)size - (int)offset;
    }
    content_length += strlen(req1);
    http_op_type = HTTP_OP_POST2;
    http_op_state = HTTP_STATE_CONNECTING;
    if (use_http_proxy) {
        sprintf( proxy_buf, "http://%s:%d/%s", hostname, port, filename );
    } else {
        sprintf( proxy_buf, "/%s", filename );
    }
    http_post_request_header(
        request_header, hostname, port, proxy_buf, content_length
    );
    return 0;
}

// Returns true if the HTTP operation is complete
//
bool HTTP_OP::http_op_done() {
    return (http_op_state == HTTP_STATE_DONE);
}

HTTP_OP_SET::HTTP_OP_SET(NET_XFER_SET* p) {
    net_xfers = p;
}

// Adds an HTTP_OP to the set
//
int HTTP_OP_SET::insert(HTTP_OP* ho) {
    int retval;

    retval = net_xfers->insert(ho);
    if (retval) return retval;
    http_ops.push_back(ho);
    return 0;
}

bool HTTP_OP_SET::poll() {
    unsigned int i;
    HTTP_OP* htp;
    int n, retval;
    bool action = false;

    ScopeMessages scope_messages(log_messages, ClientMessages::DEBUG_HTTP);

    for (i=0; i<http_ops.size(); i++) {
        htp = http_ops[i];
        switch(htp->http_op_state) {
        case HTTP_STATE_CONNECTING:
            if (htp->error) {
                htp->http_op_state = HTTP_STATE_DONE;
                htp->http_op_retval = ERR_CONNECT;
                break;
            }
            if (htp->is_connected) {
                htp->http_op_state = HTTP_STATE_REQUEST_HEADER;
                htp->want_upload = true;
                action = true;
            }
            break;
        case HTTP_STATE_REQUEST_HEADER:
            if (htp->io_ready) {
                action = true;
                n = send(
                    htp->socket, htp->request_header,
                    strlen(htp->request_header), 0
                    );
                scope_messages.printf("HTTP_OP_SET::poll(): wrote HTTP header to socket %d: %d bytes\n",
                                      htp->socket, n);
                scope_messages.printf_multiline(htp->request_header, "HTTP_OP_SET::poll(): request header: ");
                htp->io_ready = false;
                switch(htp->http_op_type) {
                case HTTP_OP_POST:
                    htp->http_op_state = HTTP_STATE_REQUEST_BODY;
                    htp->file = fopen(htp->infile, "rb");
                    if (!htp->file) {
                        msg_printf(NULL, MSG_ERROR, "HTTP_OP_SET::poll(): no input file %s\n", htp->infile);
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

                if (htp->infile && strlen(htp->infile) > 0) {
                    htp->file = fopen(htp->infile, "rb");
                    if (!htp->file) {
                        msg_printf(NULL, MSG_ERROR, "HTTP_OP_SET::poll(): no input2 file %s\n", htp->infile);
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
                scope_messages.printf("HTTP_OP_SET::poll(): finished sending request body\n");
                htp->http_op_state = HTTP_STATE_REPLY_HEADER;
                if (htp->file) {
                    fclose(htp->file);
                    htp->file = 0;
                }
                htp->do_file_io = false;
                htp->want_upload = false;
                htp->want_download = true;
                htp->io_ready = false;
                htp->io_done = false;
            }
            // TODO: intentional no break here?
        case HTTP_STATE_REPLY_HEADER:
            if (htp->io_ready) {
                action = true;
                scope_messages.printf("HTTP_OP_SET::poll(): got reply header; %p io_done %d\n", htp, htp->io_done);
                read_http_reply_header(htp->socket, htp->hrh);

                // TODO: handle all kinds of redirects here

                if (htp->hrh.status == HTTP_STATUS_MOVED_PERM || htp->hrh.status == HTTP_STATUS_MOVED_TEMP) {
                    htp->close_socket();
                    switch (htp->http_op_type) {
                        case HTTP_OP_HEAD:
                            htp->init_head(htp->hrh.redirect_location);
                            break;
                        case HTTP_OP_GET:
                            htp->init_get(htp->hrh.redirect_location, htp->outfile, false);
                            break;
                        case HTTP_OP_POST:
                            htp->init_post(htp->hrh.redirect_location, htp->infile, htp->outfile);
                            break;
                        case HTTP_OP_POST2:
                            htp->init_post2(htp->hrh.redirect_location, htp->req1, htp->infile, htp->file_offset);
                            break;
                    }

                    // Open connection to the redirected server
                    // TODO: handle return value here
                    //
                    htp->open_server();
                    break;
                }
                if ((htp->hrh.status/100)*100 != HTTP_STATUS_OK) {
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
                    retval = unlink(htp->outfile);
                    // no error check here because file need not already exist
                    //
                    // fall through
                case HTTP_OP_GET:
                    htp->http_op_state = HTTP_STATE_REPLY_BODY;

                    htp->file = fopen(htp->outfile, "ab");
                    if (!htp->file) {
                        msg_printf(NULL, MSG_ERROR,
                            "HTTP_OP_SET::poll(): can't open output file %s\n",
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
                    htp->io_done = true;
                    break;
                }
            }
            break;
        case HTTP_STATE_REPLY_BODY:
            if (htp->error) {
                action = true;
                scope_messages.printf("HTTP_OP_SET::poll(): net_xfer returned error %d\n", htp->error);
                htp->http_op_state = HTTP_STATE_DONE;
                htp->http_op_retval = htp->error;
            }
            else if (htp->io_done) {
                action = true;
                switch(htp->http_op_type) {
                case HTTP_OP_POST2:
                    read_reply(htp->socket, htp->req1, 256);
                    // parse reply here?
                    break;
                default:
                    fclose(htp->file);
                    htp->file = 0;
                    break;
                }
                scope_messages.printf("HTTP_OP_SET::poll(): got reply body\n");
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

    net_xfers->remove(p);

    iter = http_ops.begin();
    while (iter != http_ops.end()) {
        if (*iter == p) {
            http_ops.erase(iter);
            return 0;
        }
        iter++;
    }
    // fprintf(stdout, "HTTP_OP_SET::remove(): not found\n");
    return 1;
}
