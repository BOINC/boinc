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

// HTTP_OP represents an HTTP operation.
// There are variants for GET, POST etc.
// as well as for the data source/sink.

#ifndef _HTTP_
#define _HTTP_

#include "net_xfer.h"

// official HTTP status codes
#define HTTP_STATUS_OK              200
#define HTTP_STATUS_RANGE_REQUEST_ERROR    416
#define HTTP_STATUS_MOVED_PERM      301
#define HTTP_STATUS_MOVED_TEMP      302

struct HTTP_REPLY_HEADER {
    int status;
    int content_length;
    char redirect_location[256];
};

#define HTTP_OP_NONE    0
// For the first 4, data source/sink are files
#define HTTP_OP_GET     1
#define HTTP_OP_POST    2
//#define HTTP_OP_PUT     3
#define HTTP_OP_HEAD    4
#define HTTP_OP_POST2   5
    // a POST operation where the request comes from a combination
    // of a string and a file w/offset,
    // and the reply goes into a memory buffer

class HTTP_OP : public NET_XFER {
public:
    HTTP_OP();
    ~HTTP_OP();

    char hostname[256];
    int port;
    char filename[256];
    char* req1;
    char infile[256];
    char outfile[256];
    int content_length;
    double file_offset;
    char request_header[256];
    HTTP_REPLY_HEADER hrh;
    int http_op_state;     // values below
    int http_op_type;
    int http_op_retval;
    bool use_proxy;
    int proxy_server_port;
    char proxy_server_name[256];

    int init_head(char* url);
    int init_get(char* url, char* outfile, bool del_old_file, double offset=0);
    int init_post(char* url, char* infile, char* outfile);
    int init_post2(
        char* url, char* req1, char* infile, double offset
    );
    //int init_put(char* url, char* infile, int offset=0);
    bool http_op_done();
};

// represents a set of HTTP requests in progress
//
class HTTP_OP_SET {
    vector<HTTP_OP*> http_ops;
    NET_XFER_SET* net_xfers;
public:
    HTTP_OP_SET(NET_XFER_SET*);
    bool poll();
    int insert(HTTP_OP*);
    int remove(HTTP_OP*);
};

#define HTTP_STATE_IDLE             0
#define HTTP_STATE_CONNECTING       1
#define HTTP_STATE_REQUEST_HEADER   2
#define HTTP_STATE_REQUEST_BODY1    3
    // sending the string part of a POST2 operation
#define HTTP_STATE_REQUEST_BODY     4
#define HTTP_STATE_REPLY_HEADER     5
#define HTTP_STATE_REPLY_BODY       6
#define HTTP_STATE_DONE             7

extern int read_http_reply_header(int socket, HTTP_REPLY_HEADER&);

#endif
