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

#include "stdafx.h"

#include "client_msgs.h"
#include "wingui.h"
#include "wingui_mainwindow.h"

void show_message(PROJECT* p, char* msg, int priority) {
	char* x;
    char message[1024];
    char* time_string = time_to_string(time(0));

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
    if (priority == MSG_INFO) {
        fprintf(stdout, "%s [%s] %s\n", time_string, x, message);
    } else {
        fprintf(stderr, "%s [%s] %s\n", time_string, x, message);
    }
    record_message(p, priority, time(0), message);
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

const char *BOINC_RCSID_d2f8923348 = "$Id$";
