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

#include "windows_cpp.h"

#ifdef _WIN32
#include "winsock.h"
#include "win_net.h"
#endif
#include <stdio.h>
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
