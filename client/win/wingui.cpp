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

#include "boinc_win.h"

#include "client_msgs.h"
#include "wingui.h"
#include "wingui_mainwindow.h"

void show_message(PROJECT* p, char* msg, int priority) {
	char* x;
    char message[1024];

    strcpy(message, msg);
    while (strlen(message)&&message[strlen(message)-1] == '\n') {
        message[strlen(message)-1] = 0;
    }    
    if (p) {
        x = p->get_project_name();
	} else {
		x = "---";
	}

	if(g_myWnd) {
		g_myWnd->MessageUser(x, message, priority);
	}
    if (priority == MSG_INFO)
        fprintf(stdout, "%s: %s\n", x, message);
    else
        fprintf(stderr, "%s: %s\n", x, message);
}

int add_new_project() {
	return 0;
}

BOOL RequestNetConnect()
{
	if(g_myWnd) {
		return g_myWnd->RequestNetConnect();
	}
	return FALSE;
}
