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

// HTTP_OP represents an HTTP operation.
// There are variants for GET and POST,
// and for the data source/sink (see below).

#ifndef _HTTP_
#define _HTTP_

#include "proxy.h"

// official HTTP status codes
#define HTTP_STATUS_OK              200
#define HTTP_STATUS_PARTIAL_CONTENT 206
#define HTTP_STATUS_RANGE_REQUEST_ERROR    416
#define HTTP_STATUS_MOVED_PERM      301
#define HTTP_STATUS_MOVED_TEMP      302
#define HTTP_STATUS_NOT_FOUND       404
#define HTTP_STATUS_PROXY_AUTH_REQ  407

struct HTTP_REPLY_HEADER {
    int http_status;
    int content_length;
    std::string redirect_location;
    std::string recv_buf;

    HTTP_REPLY_HEADER(){}
    ~HTTP_REPLY_HEADER(){}
    void init();
    int read_reply(int socket);
    void parse();
};

#define HTTP_OP_NONE    0
#define HTTP_OP_GET     1
    // data sink is a file (used for file download)
#define HTTP_OP_POST    2
    // data source and sink are files (used for scheduler op)
#define HTTP_OP_HEAD    4
    // no data (used for file upload)
#define HTTP_OP_POST2   5
    // a POST operation where the request comes from a combination
    // of a string and a file w/offset,
    // and the reply goes into a memory buffer.
    // Used for file upload

class HTTP_OP : public PROXY {
public:
    HTTP_OP();
    ~HTTP_OP();

    int port;
    char filename[256];
    char url_hostname[256];
        // the hostname part of the URL.
        // May not be the host we connect to (if using proxy)
    char* req1;
    char infile[256];
    char outfile[256];
    int content_length;
    double file_offset;
    char request_header[4096];
    HTTP_REPLY_HEADER hrh;
    int http_op_state;     // values below
    int http_op_type;
    int http_op_retval;
        // zero if success, or a BOINC error code, or an HTTP status code
////    bool proxy_auth_done;

    int init_head(const char* url);
    int init_get(const char* url, const char* outfile, bool del_old_file, double offset=0);
    int init_post(const char* url, const char* infile, const char* outfile);
    int init_post2(
        const char* url,
        char* req1,     // first part of request.  ALSO USED FOR REPLY
        const char* infile, double offset     // infile is NULL if no file sent
    );
    bool http_op_done();
};

// represents a set of HTTP requests in progress
//
class HTTP_OP_SET {
  std::vector<HTTP_OP*> http_ops;
    NET_XFER_SET* net_xfers;
public:
    HTTP_OP_SET(NET_XFER_SET*);
    bool poll(double);
    int insert(HTTP_OP*);
    int remove(HTTP_OP*);
};

#define HTTP_STATE_IDLE             0
#define HTTP_STATE_CONNECTING       1
#define HTTP_STATE_SOCKS_CONNECT    2
#define HTTP_STATE_REQUEST_HEADER   3
#define HTTP_STATE_REQUEST_BODY1    4
    // sending the string part of a POST2 operation
#define HTTP_STATE_REQUEST_BODY     5
#define HTTP_STATE_REPLY_HEADER     6
#define HTTP_STATE_REPLY_BODY       7
#define HTTP_STATE_DONE             8

extern void parse_url(const char* url, char* host, int &port, char* file);

#endif
