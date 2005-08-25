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

#ifndef _FILE_XFER_
#define _FILE_XFER_

// FILE_XFER objects encapsulate the transfer of a file to/from data servers.
// In particular it manages:
// - the choice of data servers
//   TODO: try servers beyond the first one
// - the retry and give-up policies
//   TODO: retry and eventually give up
// - restarting partial transfers
// - upload authentication

#include "client_types.h"
#include "http_curl.h"

class FILE_XFER : public HTTP_OP {
public:
    FILE_INFO* fip;
    char pathname[256];
    char header[4096];
    bool file_size_query;
    bool is_upload;

    FILE_XFER();
    ~FILE_XFER();

    int parse_upload_response(double &offset);
    int init_download(FILE_INFO&);
    int init_upload(FILE_INFO&);
    bool file_xfer_done;
    int file_xfer_retval;
};

class FILE_XFER_SET {
    HTTP_OP_SET* http_ops;
public:
    std::vector<FILE_XFER*> file_xfers;
    FILE_XFER_SET(HTTP_OP_SET*);
    int insert(FILE_XFER*);
    int remove(FILE_XFER*);
    bool poll();
};

#endif
