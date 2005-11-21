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
// ITS ORIGINAL PURPOSE WAS TO TO TEST THE HTTP LAYER IN ISOLATION,
// BUT THIS IS NO LONGER POSSIBLE

#include "cpp.h"

#ifdef _WIN32
#include "winsock.h"
#include "win_net.h"
#else
#include "config.h"
#endif
#include <cstdio>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "http.h"
#include "log_flags.h"
#include "net_xfer.h"
#include "util.h"

int main() {
    NET_XFER_SET nxs;
    HTTP_OP_SET hos(&nxs);
    HTTP_OP *op1=0, *op2=0, *op3=0;
    int retval;

#ifdef _WIN32
    NetOpen();
#endif
    log_flags.http_debug = true;
    log_flags.net_xfer_debug = true;

#if 0
    op1 = new HTTP_OP;
    retval = op1->init_get("http://localhost.localdomain/my_index.html", "test_out1", true);
    if (retval) {
        printf("init_post: %d\n", retval);
        exit(1);
    }
    hos.insert(op1);

    op2 = new HTTP_OP;
    retval = op2->init_post("http://localhost.localdomain/test-cgi/cgi", "test_in1", "test_out2");
    if (retval) {
        printf("init_post: %d\n", retval);
        exit(1);
    }
    hos.insert(op2);
#endif
    op3 = new HTTP_OP;
    retval = op3->init_head("http://localhost.localdomain/my_index2.html");
    if (retval) {
        printf("init_post: %d\n", retval);
        exit(1);
    }
    hos.insert(op3);

    while (1) {
	nxs.poll();
	hos.poll();
	if (op1 && op1->http_op_done()) {
	    printf("op1 done; status %d\n", op1->hrh.status);
	    hos.remove(op1);
            op1 = 0;
	}
	if (op2 && op2->http_op_done()) {
	    printf("op2 done; status %d\n", op2->hrh.status);
	    hos.remove(op2);
            op2 = 0;
	}
	if (op3 && op3->http_op_done()) {
	    printf("op3 done; status %d\n", op3->hrh.status);
	    hos.remove(op3);
            op3 = 0;
	}
	if (!op1 && !op2 && !op3) break;
	boinc_sleep(1.);
    }
    printf("all done\n");

    return 0;
}

const char *BOINC_RCSID_4cff6b442d = "$Id$";
