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

#ifndef _HTTP_
#define _HTTP_

#include "net_xfer.h"

struct HTTP_REPLY_HEADER {
    int status;
    int content_length;
};

#define HTTP_OP_GET     1
#define HTTP_OP_POST    2
#define HTTP_OP_PUT     3
#define HTTP_OP_HEAD    4

// represents an HTTP request in progress
//
class HTTP_OP : public NET_XFER {
public:
    HTTP_OP();
    ~HTTP_OP();

    char hostname[256];
    char filename[256];
    char infile[256];
    char outfile[256];
    int content_length;
    int offset;
    HTTP_REPLY_HEADER hrh;
    int http_op_state;     // values below
    int http_op_type;
    int http_op_retval;

    int init_head(char* url);
    int init_get(char* url, char* outfile, int offset=0);
    int init_post(char* url, char* infile, char* outfile);
    int init_put(char* url, char* infile, int offset=0);
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
    int size();
};

#define HTTP_STATE_IDLE             0
#define HTTP_STATE_CONNECTING       1
#define HTTP_STATE_REQUEST_HEADER   2
#define HTTP_STATE_REQUEST_BODY     3
#define HTTP_STATE_REPLY_HEADER     4
#define HTTP_STATE_REPLY_BODY       5
#define HTTP_STATE_DONE             6

extern int read_http_reply_header(int socket, HTTP_REPLY_HEADER&);

#endif
