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

// THIS FILE IS DEPRECATED.
// ITS ORIGINAL PURPOSE WAS TO TEST THE FILE XFER LAYER IN ISOLATION,
// BUT THIS IS NO LONGER POSSIBLE

#include "cpp.h"

#include <stdio.h>
#ifdef _WIN32
#include "winsock.h"
#include "win_net.h"
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "http.h"
#include "net_xfer.h"
#include "util.h"

#define UNCONNECTED 0
#define WRITE_WAIT  1
#define HEADER_WAIT 2
#define BODY_WAIT   3

int main() {
    NET_XFER_SET nxs;
    NET_XFER* nxp;
    HTTP_REPLY_HEADER reply_header;
    int n;

#ifdef _WIN32
    NetOpen();
#endif
    nxp = new NET_XFER;
    nxp->init("localhost.localdomain", 80, 1024);
    nxp->net_xfer_state = UNCONNECTED;
    nxs.insert(nxp);

    char* buf = "GET /my_index.html HTTP/1.0\015\012"
        "Host: localhost.localdomain:80\015\012"
        "Accept: */*\015\012"
        "\015\012";

    while (1) {
        nxs.poll();
        switch(nxp->net_xfer_state) {
        case UNCONNECTED:
            if(nxp->is_connected) {
                nxp->net_xfer_state = WRITE_WAIT;
                nxp->want_upload = true;
            }
            break;
        case WRITE_WAIT:
            if (nxp->io_ready) {
#ifdef _WIN32
                n = send(nxp->socket, buf, strlen(buf), 0);
#else
                n = write(nxp->socket, buf, strlen(buf));
#endif
                printf("wrote %d bytes\n", n);
                nxp->net_xfer_state = HEADER_WAIT;
                nxp->want_upload = false;
                nxp->want_download = true;
                nxp->io_ready = false;
            }
            break;
        case HEADER_WAIT:
            if (nxp->io_ready) {
                read_http_reply_header(nxp->socket, reply_header);
                nxp->net_xfer_state = BODY_WAIT;
                nxp->file = fopen("foo", "wb");
                nxp->do_file_io = true;
            }
            break;
        case BODY_WAIT:
            if (nxp->io_done) break;
        }
        if (nxp->io_done) break;
        boinc_sleep(1.);
    }
    nxs.remove(nxp);
    if (nxp->file) {
		fclose(nxp->file);
	}

    return 0;
}

const char *BOINC_RCSID_6a5fc6c730 = "$Id$";
