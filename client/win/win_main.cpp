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

#ifdef _WIN32

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <process.h>
#include <pbt.h>

#include "win_net.h"

//////////////////////////////////////////////////////////////////////////////////////
//
// Function    : WinMain
//
//
///////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv);

int WINAPI WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR Args, int WinMode )
{
	char *command_line;
	char **argv;
	int argc;

	NetOpen();
	//command_line = GetCommandLine();
	//argv = CommandLineToArgvW( command_line, &argc );
	main( argc, argv );

	return 0;
}


#endif
