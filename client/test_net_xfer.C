static volatile const char *BOINCrcsid="$Id$";
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
