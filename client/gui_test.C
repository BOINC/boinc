#include <stdio.h>

#include "gui_rpc_client.h"

main() {
    RPC_CLIENT rpc;
    unsigned int i;

    rpc.init("gui_rpc");
    rpc.get_state();
}
