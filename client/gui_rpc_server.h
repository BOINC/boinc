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

class GUI_RPC_CONN {
public:
    int sock;
    FILE* fout;
    char buffer[262144];
    GUI_RPC_CONN(int);
    ~GUI_RPC_CONN();
    int handle_rpc();
};

class GUI_RPC_CONN_SET {
    vector<GUI_RPC_CONN*> gui_rpcs;
    int insert(GUI_RPC_CONN*);
    int lsock;
public:
    bool poll();
    int init();
};

#define GUI_RPC_PORT 31416
