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

// gui_test: test program for BOINC GUI RPCs.
// Does a single RPC and shows results

#ifdef _WIN32
#include "stdafx.h"
#include "win_net.h"
#endif

#ifndef _WIN32
#include <stdio.h>
#endif

#include "gui_rpc_client.h"
#include "error_numbers.h"

#ifdef _WIN32

int WinsockInitialize()
{
    WSADATA wsdata;
    return WSAStartup( MAKEWORD( 1, 1 ), &wsdata);
}

int WinsockCleanup()
{
    return WSACleanup();
}

#endif

int main(int argc, char** argv) {

    RPC_CLIENT rpc;
    unsigned int i;
    vector<MESSAGE_DESC> message_descs;

#ifdef _WIN32
    if ( WinsockInitialize() != 0 ) {
        printf(
            "BOINC Core Client Error Message\n"
            "Failed to initialize the Windows Sockets interface\n"
            "Terminating Application...\n"
        );
        return ERR_IO;
    }
#endif

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

#ifdef _WIN32
    if ( WinsockCleanup() != 0 ) {
        printf(
            "BOINC Core Client Error Message\n"
            "Failed to cleanup the Windows Sockets interface\n"
        );
        return ERR_IO;
    }
#endif

    return 0;
}

