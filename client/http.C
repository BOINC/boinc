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

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#define SHUT_WR SD_SEND
#endif

#ifndef _WIN32
#include <cstring>
#include <sstream>
#include <algorithm>

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#include <cerrno>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#endif

#include "error_numbers.h"
#include "filesys.h"
#include "util.h"
#include "client_msgs.h"

#include "http.h"
#include "base64.h"

#define HTTP_BLOCKSIZE  16384

using std::string;
using std::istringstream;
using std::vector;
using std::getline;

// Breaks a HTTP url down into its server and file path components
//
void parse_url(const char* url, char* host, int &port, char* file) {
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
        "User-Agent: BOINC client (%s %d.%02d)\015\012"
        "Host: %s:%d\015\012"
        "%s"
        "Connection: close\015\012"
        "Accept: */*\015\012"
        "\015\012",
        file,
        HOSTTYPE, BOINC_MAJOR_VERSION, BOINC_MINOR_VERSION,
        host, port, offset?offset_info:""
    );
}

static void http_get_request_header_proxy(
    char* buf, char* host, int port, char* file, double offset, char* encstr
) {
    char offset_info[256];
    if (offset) sprintf( offset_info, "Range: bytes=%.0f-\015\012", offset );
    sprintf(buf,
        "GET %s HTTP/1.0\015\012"
        "User-Agent: BOINC client (%s %d.%02d)\015\012"
        "Host: %s:%d\015\012"
        "%s"
        "Connection: close\015\012"
        "Accept: */*\015\012"
        "Proxy-Authorization: Basic %s\015\012"
        "\015\012",
        file,
        HOSTTYPE, BOINC_MAJOR_VERSION, BOINC_MINOR_VERSION,
        host, port, offset?offset_info:"", encstr
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

static void http_head_request_header_proxy(char* buf, char* host, int port, char* file, char* encstr
) {
    sprintf(buf,
        "HEAD %s HTTP/1.0\015\012"
        "User-Agent: BOINC client\015\012"
        "Host: %s:%d\015\012"
        "Connection: close\015\012"
        "Accept: */*\015\012"
        "Proxy-Authorization: Basic %s\015\012"
        "\015\012",
        file, host, port, encstr
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

static void http_post_request_header_proxy(
    char* buf, char* host, int port, char* file, int size, char* encstr
) {
    sprintf(buf,
        "POST %s HTTP/1.0\015\012"
        "Pragma: no-cache\015\012"
        "Cache-Control: no-cache\015\012"
        "Host: %s:%d\015\012"
        "Connection: close\015\012"
        "Content-Type: application/octet-stream\015\012"
        "Content-Length: %d\015\012"
        "Proxy-Authorization: Basic %s\015\012"
        "\015\012",
        file, host, port, size, encstr
    );
}

void HTTP_REPLY_HEADER::init() {
    http_status = 500;
    content_length = 0;
    redirect_location.erase();
    recv_buf.erase();
}

void HTTP_REPLY_HEADER::parse() {
    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_HTTP);

    istringstream h(recv_buf);
    string line, w;

    if (getline(h, line)) {
        istringstream iline(line);

        iline >> w;
        if (!starts_with(w,"HTTP/")) {
            scope_messages.printf("HTTP_REPLY_HEADER::parse(): not HTTP\n");
            return;
        }
        iline >> http_status;
        scope_messages.printf("HTTP_REPLY_HEADER::parse(): status=%d\n", http_status);
    }

    while (getline(h, line)) {
        istringstream iline(line);

        iline >> w;
        downcase_string(w);
        if (w == "content-length:") {
            iline >> content_length;
            scope_messages.printf("HTTP_REPLY_HEADER::parse(): content_length=%d\n", content_length);
        } else if (w == "location:") {
            iline >> redirect_location;
            scope_messages.printf("HTTP_REPLY_HEADER::parse(): redirect_location=%s\n", redirect_location.c_str());
        }
    }
}

const unsigned int MAX_HEADER_SIZE = 1024;

// Read an HTTP reply header into the header struct
// Read one character at a time so we don't read anything beyond the header.
// Parse the reply header if it's complete.
// Returns:
//      1 if not done yet
//      0 if done (header.http_status indicates status)
//
int HTTP_REPLY_HEADER::read_reply(int socket) {
    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_HTTP);

    while (recv_buf.size() < MAX_HEADER_SIZE) {
        char c;
        errno = 0;
        int n = recv(socket, &c, 1, 0);
        if (n != 1) {
            scope_messages.printf(
                "HTTP_REPLY_HEADER::read_reply(): recv() on socket %d returned %d errno %d sockerr %d\n",
                socket, n, errno, get_socket_error(socket)
            );
        }
        if (n == -1 && errno == EAGAIN) {
            return 1;
        }

        // if n is zero, we've reached EOF (and that's an error)
        //
        if (n != 1) {
            break;
        }

        if (c == '\r') continue;
        recv_buf += c;

        if (ends_with(recv_buf, "\n\n")) {
            scope_messages.printf_multiline(recv_buf.c_str(),
                "HTTP_REPLY_HEADER::read_reply(): header: "
            );
            parse();
            return 0;
        }
    }

    // error occurred; status will be 500 (from constructor)
    //
    scope_messages.printf(
        "HTTP_REPLY_HEADER::read_reply(): returning error (recv_buf=%s)\n",
        recv_buf.c_str()
    );
    return 0;
}

// Read the contents of the socket into buf
// Read until EOF from socket, so only call this if you're
// sure the message is small
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
    strcpy(url_hostname, "");
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
}

HTTP_OP::~HTTP_OP() {
}

// Initialize HTTP HEAD operation
//
int HTTP_OP::init_head(const char* url) {
    char proxy_buf[256];
    parse_url(url, url_hostname, port, filename);
    PROXY::init(url_hostname, port);
    NET_XFER::init(get_proxy_server_name(url_hostname),get_proxy_port(port), HTTP_BLOCKSIZE);
    http_op_type = HTTP_OP_HEAD;
    http_op_state = HTTP_STATE_CONNECTING;
    if (pi.use_http_proxy) {
        sprintf(proxy_buf, "http://%s:%d/%s", url_hostname, port, filename);
    } else {
        sprintf(proxy_buf, "/%s", filename);
    }
    if (!pi.use_http_auth){
        http_head_request_header(
            request_header, url_hostname, port, proxy_buf
        );
    } else {
        char id_passwd[512];
        string encstr = "";
        memset(id_passwd,0,sizeof(id_passwd));
        strcpy(id_passwd,pi.http_user_name);
        strcat(id_passwd,":");
        strcat(id_passwd,pi.http_user_passwd);
        encstr = r_base64_encode(id_passwd,strlen(id_passwd));
        http_head_request_header_proxy(
            request_header, url_hostname, port, proxy_buf, (char*)encstr.c_str()
        );
    }
    return 0;
}

// Initialize HTTP GET operation
//
int HTTP_OP::init_get(const char* url, char* out, bool del_old_file, double off) {
    char proxy_buf[256];

    if (del_old_file) {
        unlink(out);
    }
    file_offset = off;
    parse_url(url, url_hostname, port, filename);
    PROXY::init(url_hostname, port);
    NET_XFER::init(get_proxy_server_name(url_hostname),get_proxy_port(port), HTTP_BLOCKSIZE);
    safe_strcpy(outfile, out);
    if (off != 0){
        bytes_xferred = off;
    }
    http_op_type = HTTP_OP_GET;
    http_op_state = HTTP_STATE_CONNECTING;
    if (pi.use_http_proxy) {
        sprintf(proxy_buf, "http://%s:%d/%s", url_hostname, port, filename);
    } else {
        sprintf(proxy_buf, "/%s", filename);
    }
    if (!pi.use_http_auth) {
        http_get_request_header(request_header, url_hostname, port, proxy_buf, (int)file_offset);
    } else {
        char id_passwd[512];
        string encstr = "";
        memset(id_passwd,0,sizeof(id_passwd));
        strcpy(id_passwd,pi.http_user_name);
        strcat(id_passwd,":");
        strcat(id_passwd,pi.http_user_passwd);
        encstr = r_base64_encode(id_passwd,strlen(id_passwd));
        http_get_request_header_proxy(
            request_header, url_hostname,
            port, proxy_buf, (int)file_offset, (char*)encstr.c_str()
        );
    }
    return 0;
}

// Initialize HTTP POST operation
//
int HTTP_OP::init_post(const char* url, char* in, char* out) {
    int retval;
    double size;
    char proxy_buf[256];

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_HTTP);

    parse_url(url, url_hostname, port, filename);

    PROXY::init(url_hostname, port);
    NET_XFER::init(get_proxy_server_name(url_hostname),get_proxy_port(port), HTTP_BLOCKSIZE);
    safe_strcpy(infile, in);
    safe_strcpy(outfile, out);
    retval = file_size(infile, size);
    if (retval) return retval;
    content_length = (int)size;
    http_op_type = HTTP_OP_POST;
    http_op_state = HTTP_STATE_CONNECTING;
    if (pi.use_http_proxy) {
        sprintf(proxy_buf, "http://%s:%d/%s", url_hostname, port, filename);
    } else {
        sprintf(proxy_buf, "/%s", filename);
    }

    if (!pi.use_http_auth) {
        http_post_request_header(
            request_header, url_hostname, port, proxy_buf, content_length
        );
    } else {
        char id_passwd[512];
        string encstr = "";
        memset(id_passwd,0,sizeof(id_passwd));
        strcpy(id_passwd,pi.http_user_name);
        strcat(id_passwd,":");
        strcat(id_passwd,pi.http_user_passwd);
        encstr = r_base64_encode(id_passwd,strlen(id_passwd));
        http_post_request_header_proxy(
            request_header, url_hostname, port, proxy_buf, content_length,
            (char*)encstr.c_str()
        );
    }
    scope_messages.printf("HTTP_OP::init_post(): %p io_done %d\n", this, io_done);
    return 0;
}

// Initialize HTTP POST operation
//
int HTTP_OP::init_post2(
    const char* url, char* r1, char* in, double offset
) {
    int retval;
    double size;
    char proxy_buf[256];

    parse_url(url, url_hostname, port, filename);
    PROXY::init(url_hostname, port);
    NET_XFER::init(get_proxy_server_name(url_hostname),get_proxy_port(port), HTTP_BLOCKSIZE);
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
    if (pi.use_http_proxy) {
        sprintf(proxy_buf, "http://%s:%d/%s", url_hostname, port, filename);
    } else {
        sprintf(proxy_buf, "/%s", filename);
    }
    if (!pi.use_http_auth) {
        http_post_request_header(
            request_header, url_hostname, port, proxy_buf, content_length
        );
    } else {
        char id_passwd[512];
        string encstr = "";
        memset(id_passwd,0,sizeof(id_passwd));
        strcpy(id_passwd,pi.http_user_name);
        strcat(id_passwd,":");
        strcat(id_passwd,pi.http_user_passwd);
        encstr = r_base64_encode(id_passwd,strlen(id_passwd));
        http_post_request_header_proxy(
            request_header, url_hostname, port, proxy_buf, content_length,
            (char*)encstr.c_str()
        );
    }
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

bool HTTP_OP_SET::poll(double) {
    unsigned int i;
    HTTP_OP* htp;
    int n, retval;
    bool action = false;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_HTTP);

    for (i=0; i<http_ops.size(); i++) {
        htp = http_ops[i];
        if (htp->error) {
            htp->http_op_state = HTTP_STATE_DONE;
            htp->http_op_retval = htp->error;
            action = true;
            continue;
        }
        switch(htp->http_op_state) {
        case HTTP_STATE_CONNECTING:
            if (htp->is_connected) {
                htp->http_op_state = HTTP_STATE_SOCKS_CONNECT;
                htp->want_upload = true;
                action = true;
            }
            break;
        case HTTP_STATE_SOCKS_CONNECT:
            // Since the HTTP layer is synchronous with the proxy layer, we
            // call proxy_poll() here instead of in do_something()
            htp->proxy_poll();

            // After negotiation with the proxy is complete, advance to
            // the next step of the HTTP layer
            if (htp->proxy_negotiated()) {
                if (htp->proxy_retval) {
                    htp->http_op_state = HTTP_STATE_DONE;
                    htp->http_op_retval = htp->proxy_retval;
                    switch (htp->proxy_retval) {
                    case ERR_SOCKS_UNKNOWN_FAILURE:
                        msg_printf(NULL, MSG_ERROR, "An unknown SOCKS server error occurred\n");
                        break;
                    case ERR_SOCKS_REQUEST_FAILED:
                        msg_printf(NULL, MSG_ERROR, "The SOCKS server denied access for this computer\n");
                        break;
                    case ERR_SOCKS_BAD_USER_PASS:
                        msg_printf(NULL, MSG_ERROR, "Incorrect SOCKS user name and/or password\n");
                        break;
                    case ERR_SOCKS_UNKNOWN_SERVER_VERSION:
                        msg_printf(NULL, MSG_ERROR, "The SOCKS server is using an unknown version\n");
                        break;
                    case ERR_SOCKS_UNSUPPORTED:
                        msg_printf(NULL, MSG_ERROR, "The SOCKS server is using unsupported features unknown to BOINC\n");
                        break;
                    case ERR_SOCKS_CANT_REACH_HOST:
                        msg_printf(NULL, MSG_ERROR, "The SOCKS server is unable to contact the host\n");
                        break;
                    case ERR_SOCKS_CONN_REFUSED:
                        msg_printf(NULL, MSG_ERROR, "The connection from the SOCKS server to the host was refused\n");
                        break;
                    }
                } else {
                    htp->http_op_state = HTTP_STATE_REQUEST_HEADER;
                }
            }
            break;
        case HTTP_STATE_REQUEST_HEADER:
            if (htp->io_ready) {
                action = true;
                n = send(
                    htp->socket, htp->request_header,
                    strlen(htp->request_header), 0
                    );
                scope_messages.printf(
                    "HTTP_OP_SET::poll(): wrote HTTP header to socket %d: %d bytes\n",
                    htp->socket, n
                );
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
                    htp->hrh.init();
                    htp->http_op_state = HTTP_STATE_REPLY_HEADER;
                    htp->want_upload = false;
                    htp->want_download = true;

                    // We don't need to write to the socket anymore.
                    // Close the read fd on the receiving side.
                    // This is needed by the scheduler "use_file" mechanism
                    // NOTE: this is commented out because
                    // - it seems to cause problems on all platforms
                    // in Windows it exercises a problem with Norton Firewall
#ifndef _WIN32
                    //shutdown(htp->socket, SHUT_WR);
#endif
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
                htp->hrh.init();
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

                // We don't need to write to the socket anymore.
                // Close the read fd on the receiving side.
                // This is needed by the scheduler "use_file" mechanism
                // NOTE: this is commented out because
                // - it seems to cause problems on all platforms
                // in Windows it exercises a problem with Norton Firewall
#ifndef _WIN32
                //shutdown(htp->socket, SHUT_WR);
#endif
            }
            break;
        case HTTP_STATE_REPLY_HEADER:
            if (htp->io_ready) {
                action = true;
                scope_messages.printf(
                    "HTTP_OP_SET::poll(): reading reply header; io_ready %d io_done %d\n",
                    htp->io_ready, htp->io_done
                );
                if (htp->hrh.read_reply(htp->socket)) {
                    // reading header, not done yet
                    htp->io_ready = false;
                    break;
                }

                // here we've read and parsed the header.
                // hth.http_status tells us the HTTP status

                if (htp->hrh.http_status == HTTP_STATUS_MOVED_PERM || htp->hrh.http_status == HTTP_STATUS_MOVED_TEMP) {
                    htp->close_socket();
                    retval = 0;
                    const char* new_url = htp->hrh.redirect_location.c_str();
                    if (!new_url || !strlen(new_url)) {
                        htp->http_op_state = HTTP_STATE_DONE;
                        htp->http_op_retval = retval;
                        break;
                    }
                    switch (htp->http_op_type) {
                    case HTTP_OP_HEAD:
                        htp->init_head(new_url);
                        break;
                    case HTTP_OP_GET:
                        htp->init_get(new_url, htp->outfile, false);
                        break;
                    case HTTP_OP_POST:
                        htp->init_post(new_url, htp->infile, htp->outfile);
                        break;
                    case HTTP_OP_POST2:
                        htp->init_post2(new_url, htp->req1, htp->infile, htp->file_offset);
                        break;
                    }

                    // Open connection to the redirected server
                    //
                    retval = htp->open_server();
                    if (retval) {
                        htp->http_op_state = HTTP_STATE_DONE;
                        htp->http_op_retval = retval;
                    }
                    break;
                }

                if (htp->hrh.http_status == HTTP_STATUS_PROXY_AUTH_REQ) {
                    htp->close_socket();
                    htp->http_op_state = HTTP_STATE_DONE;
                    htp->http_op_retval = htp->hrh.http_status;
                    msg_printf(NULL, MSG_ERROR, "HTTP_OP_SET::poll(): Proxy Authentication Failed\n");
                    break;
                }

                if ((htp->hrh.http_status/100)*100 != HTTP_STATUS_OK) {
                    htp->http_op_state = HTTP_STATE_DONE;
                    htp->http_op_retval = htp->hrh.http_status;
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

                    htp->file = boinc_fopen(htp->outfile, "ab");
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
                    htp->io_done = false;
                    break;
                }
            }
            break;
        case HTTP_STATE_REPLY_BODY:
            switch (htp->http_op_type) {
            case HTTP_OP_POST2:
                if (htp->io_ready) {
                    action = true;
                    read_reply(htp->socket, htp->req1, 256);
                    scope_messages.printf("HTTP_OP_SET::poll(): got reply body\n");
                    htp->http_op_state = HTTP_STATE_DONE;
                    htp->http_op_retval = 0;
                }
                break;
            default:
                if (htp->io_done) {
                    action = true;
                    fclose(htp->file);
                    htp->file = 0;
                    scope_messages.printf("HTTP_OP_SET::poll(): got reply body\n");
                    htp->http_op_retval = 0;
                    if (htp->hrh.content_length) {
                        if ((htp->bytes_xferred-htp->file_offset) != htp->hrh.content_length) {
                            htp->http_op_retval = ERR_IO;
                        }
                    }
                    htp->http_op_state = HTTP_STATE_DONE;
                    break;
                }
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
    msg_printf(NULL, MSG_ERROR, "HTTP_OP_SET::remove(): not found\n");
    return ERR_NOT_FOUND;
}

const char *BOINC_RCSID_57f273bb60 = "$Id$";
