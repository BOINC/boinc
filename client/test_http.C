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

#include <stdio.h>
#include <unistd.h>

#include "http.h"
#include "net_xfer.h"

int main() {
    NET_XFER_SET nxs;
    HTTP_OP_SET hos(&nxs);
    HTTP_OP *op1, *op2;
    int n;

    op1 = new HTTP_OP;
    op2 = new HTTP_OP;
    op1->init_get("http://localhost.localdomain/my_index.html", "test_out1");
    op2->init_post("http://localhost.localdomain/test-cgi/cgi", "test_in1", "test_out2");

    hos.insert(op1);
    hos.insert(op2);

    while (1) {
	nxs.poll(100000, n);
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
	if (!op1 && !op2) break;
	boinc_sleep(1);
    }
    printf("all done\n");
}
