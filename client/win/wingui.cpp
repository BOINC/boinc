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

// include header

#include "wingui.h"
#include "wingui_mainwindow.h"

void show_message(char* message, char* priority) {
	if(g_myWnd) {
		g_myWnd->MessageUser("BOINC", message, priority);
	}
}

void show_project_message(PROJECT* project, char* message, char* priority) {
	if(g_myWnd) {
		if(strcmp(project->project_name, "")) {
			g_myWnd->MessageUser(project->project_name, message, priority);
		} else {
			g_myWnd->MessageUser(project->master_url, message, priority);
		}
	}
}

int get_initial_project() {
	return 0;
}

void GetByteString(double nbytes, CString* str) {
	double xTera = (1024.0*1024*1024*1024);
	double xGiga = (1024.0*1024*1024);
	double xMega = (1024.0*1024);
	double xKilo = (1024.0);
    if (nbytes >= xTera) {
        str->Format("%0.2f TB", nbytes/xTera);
    } else if (nbytes >= xGiga) {
        str->Format("%0.2f GB", nbytes/xGiga);
    } else if (nbytes >= xMega) {
        str->Format("%0.2f MB", nbytes/xMega);
    } else if (nbytes >= xKilo) {
        str->Format("%0.2f KB", nbytes/xKilo);
    } else {
        str->Format("%0.0f bytes", nbytes);
    }
}

BOOL RequestNetConnect()
{
	if(g_myWnd) {
		return g_myWnd->RequestNetConnect();
	}
	return FALSE;
}
