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
// Based on code by Lucian Wischik

#include <afxwin.h>

enum TScrMode {smNone,smConfig,smPassword,smPreview,smSaver};

int WINAPI WinMain(HINSTANCE h,HINSTANCE,LPSTR,int)
{
	int BOINC_GFX_MODE_MSG;
	UINT oldval;
	TScrMode ScrMode=smNone;

	/*char *c=GetCommandLine();
	if (*c=='\"') {c++; while (*c!=0 && *c!='\"') c++;} else {while (*c!=0 && *c!=' ') c++;}
	if (*c!=0) c++;
	while (*c==' ') c++;
	HWND hwnd=NULL;
	if (*c==0) {ScrMode=smConfig; hwnd=NULL;}
	else
	{ if (*c=='-' || *c=='/') c++;
    if (*c=='p' || *c=='P' || *c=='l' || *c=='L')
    { c++; while (*c==' ' || *c==':') c++;
      if ((strcmp(c,"scrprev")==0) || (strcmp(c,"ScrPrev")==0) || (strcmp(c,"SCRPREV")==0)) hwnd=CheckForScrprev();
      else hwnd=(HWND)atoi(c);
      ScrMode=smPreview;
    }
    else if (*c=='s' || *c=='S') {ScrMode=smSaver; }
    else if (*c=='c' || *c=='C') {c++; while (*c==' ' || *c==':') c++; if (*c==0) hwnd=GetForegroundWindow(); else hwnd=(HWND)atoi(c); ScrMode=smConfig;}
    else if (*c=='a' || *c=='A') {c++; while (*c==' ' || *c==':') c++; hwnd=(HWND)atoi(c); ScrMode=smPassword;}
	}
	if (ScrMode==smPassword) ChangePassword(hwnd);
	if (ScrMode==smConfig) DialogBox(hInstance,MAKEINTRESOURCE(DLG_CONFIG),hwnd,ConfigDialogProc);
	if (ScrMode==smSaver || ScrMode==smPreview) DoSaver(hwnd);*/

	// Set a flag in the system to indicate that we're in screensaver mode
	//if (ScrMode==smSaver)
		SystemParametersInfo(SPI_SCREENSAVERRUNNING,1,&oldval,0);

	BOINC_GFX_MODE_MSG = RegisterWindowMessage( "BOINC_GFX_MODE" );

	PostMessage(HWND_BROADCAST, BOINC_GFX_MODE_MSG, 0, 0);

	// Unset the system screensaver flag
	//if (ScrMode==smSaver)
		SystemParametersInfo(SPI_SCREENSAVERRUNNING,0,&oldval,0);

	return 0;
}
