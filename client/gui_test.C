// gui_test: test program for BOINC GUI RPCs.
// Does a single RPC and shows results

#include <stdio.h>

#include "gui_rpc_client.h"

main(int argc, char** argv) {
    RPC_CLIENT rpc;
    unsigned int i;

    rpc.init("gui_rpc");
    rpc.get_state();
    rpc.print();
}
