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

// HTTP_OP represents an HTTP operation.
// There are variants for GET and POST,
// and for the data source/sink (see below).

#ifndef _HTTP_
#define _HTTP_

#include "proxy.h"

// official HTTP status codes
#define HTTP_STATUS_OK              200
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
// For the first 4, data source/sink are files
#define HTTP_OP_GET     1
#define HTTP_OP_POST    2
#define HTTP_OP_HEAD    4
#define HTTP_OP_POST2   5
    // a POST operation where the request comes from a combination
    // of a string and a file w/offset,
    // and the reply goes into a memory buffer
    // Used exclusively for file upload

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
    int init_get(const char* url, char* outfile, bool del_old_file, double offset=0);
    int init_post(const char* url, char* infile, char* outfile);
    int init_post2(
        const char* url, char* req1, char* infile, double offset
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
