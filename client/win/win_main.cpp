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

// Main program for the Windows command-line client

#include <afxwin.h>

#include "win_net.h"

#ifdef WIN_CLI
extern int main(int argc, char** argv);

int WINAPI WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR Args, int WinMode )
{
	LPWSTR command_line;
	LPWSTR *args;
	char* argv[100];
	int i, argc;

	NetOpen();
	command_line = GetCommandLineW();
	args = CommandLineToArgvW(command_line, &argc);

	// uh, why did MS have to "improve" on char*?

	for (i=0; i<argc; i++) {
		argv[i] = (char*)args[i];
	}
	main(argc, argv);

	return 0;
}
#endif
