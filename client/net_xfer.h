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

#ifndef _NET_XFER_
#define _NET_XFER_

#include <stdio.h>
#include <vector>

// The following classes provide an interface for polling
// (non-blocking) network I/O.

// represents a network connection, either being accessed directly
// or being transferred to/from a file
//
class NET_XFER {
public:
    int net_xfer_state;
            // client-defined; better to define state in parent class
    int socket;
    bool is_connected;
    bool want_download;     // at most one should be true
    bool want_upload;
    bool do_file_io;
        // whether poll() should transfer data to/from file
        // (in which case "file" and blocksize are relevant)
        // or just set io_ready
    bool io_done;           // got error or EOF
    FILE* file;
    bool io_ready;
        // can read or write socket now (used if !do_file_io)
    int error;
    char hostname[256];
    int port;
    int blocksize;

    void init(char* host, int port, int blocksize);
    int get_ip_addr(char *hostname, int &ip_addr);
    int open_server();
    void close_socket();
    int do_xfer(int&);
    void got_error();
};

class NET_XFER_SET {
    vector<NET_XFER*> net_xfers;
public:
    int insert(NET_XFER*);
    int remove(NET_XFER*);
    int poll(int max_bytes, int& bytes_transferred);
    int net_sleep(double);
    int do_select(int max_bytes, int& bytes_transferred, timeval& timeout);
    NET_XFER* lookup_fd(int);   // lookup by fd
};

#endif
