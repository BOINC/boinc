// gui_test: test program for BOINC GUI RPCs.
// Does a single RPC and shows results

#include <stdio.h>

#include "gui_rpc_client.h"

main(int argc, char** argv) {
    RPC_CLIENT rpc;
    unsigned int i;
    vector<MESSAGE_DESC> message_descs;

    rpc.init("gui_rpc");
    rpc.get_state();
    rpc.print();
    rpc.get_messages(20, 0, message_descs);
    for (i=0; i<message_descs.size(); i++) {
        MESSAGE_DESC& md = message_descs[i];
        printf("%s %d %d %s\n",
            md.project.c_str(), md.priority, md.timestamp, md.body.c_str()
        );
    }
}
