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

#include <windows.h>
#include <string.h>
#include <stdio.h>

#include "win_net.h"


int NetOpen( void )
{
    WSADATA wsdata;
    WORD    wVersionRequested;
	int rc, addrlen = 16;

	// TODO: return if already open

	// TODO: Handle permission logic here

    wVersionRequested = MAKEWORD(1,1);
    rc = WSAStartup(wVersionRequested, &wsdata);

    if (rc) 
	{
        //printf("WSAStartup failed: error code = %d\n", rc);
        return -1; //CANT_CONNECT;
    }


	return 0;
}


void NetClose( void )
{
 	WSACleanup();
}



