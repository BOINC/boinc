#include <afxwin.h>

int WINAPI WinMain(HINSTANCE h,HINSTANCE,LPSTR,int)
{
	int BOINC_GFX_MODE_MSG;

	BOINC_GFX_MODE_MSG = RegisterWindowMessage( "BOINC_GFX_MODE" );

	PostMessage(HWND_BROADCAST, BOINC_GFX_MODE_MSG, 0, 0);

	return 0;
}
