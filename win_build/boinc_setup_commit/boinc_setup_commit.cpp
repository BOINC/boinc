// boinc_setup_commit.cpp : Defines the entry point for the application.
//

#include <windows.h>
#include <string>

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	// execute the boinc as a new process and don't wait for it to finish.

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

	std::string boincDir = lpCmdLine;
	std::string boincExePath = boincDir + "\\boinc_gui.exe";
	char commandLine[] = "boinc_gui.exe -min";

	CreateProcess(boincExePath.c_str(), commandLine, NULL, NULL, FALSE, 0, NULL, boincDir.c_str(), &si, &pi);
    // Close process and thread handles. 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );

	// Don't wait for process to exit!  That's the whole point.
	return 0;
}

const char *BOINC_RCSID_d2f0340771 = "$Id$";
