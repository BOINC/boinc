#include <windows.h>
#include "win_util.h"

#define OS_UNKNOWN          0
#define OS_WIN95            1
#define OS_WINNT            2

int OSVersion;

//////////
// Function:    UtilInitOSVersion
// arguments:	void
// returns:		int indicating error
// function:	sets global variable "OSVersion" to the current OS (Win95/NT/Unknown)
int UtilInitOSVersion( void )
{
	OSVERSIONINFO osinfo;

	osinfo.dwOSVersionInfoSize = sizeof(osinfo);
	if (!GetVersionEx( &osinfo ))
		return FALSE;

	if (osinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		OSVersion = OS_WIN95;
	else if ( osinfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
		OSVersion = OS_WINNT;
	else
		OSVersion = OS_UNKNOWN;

	return TRUE;

}

//////////
// Function:    UtilGetRegKey
// arguments:	name: name of key, keyval: where to store value of key
// returns:		int indicating error
// function:	reads string value in specified key
int UtilSetRegKey(char *name, DWORD value)
{
	LONG error;
	HKEY boinc_key;

	if ( OSVersion == OS_WIN95 ) {
		error = RegOpenKeyEx( HKEY_LOCAL_MACHINE, "SOFTWARE\\BOINC",  
			0, KEY_ALL_ACCESS, &boinc_key );
		if ( error != ERROR_SUCCESS ) return -1;
	} else if ( OSVersion == OS_WINNT ) {
		error = RegOpenKeyEx( HKEY_CURRENT_USER, "SOFTWARE\\BOINC",  
			0, KEY_ALL_ACCESS, &boinc_key );
		if ( error != ERROR_SUCCESS ) return -1;
	} else {
		return -1;
	}

	error = RegSetValueEx( boinc_key, name, 0,
		REG_DWORD, (CONST BYTE *)&value, 4 );

	RegCloseKey( boinc_key );

	return 0;
}

//////////
// Function:    UtilGetRegKey
// arguments:	name: name of key, keyval: where to store value of key
// returns:		int indicating error
// function:	reads string value in specified key
int UtilGetRegKey(char *name, DWORD &keyval)
{
	LONG error;
	DWORD type = REG_DWORD;
	DWORD size = sizeof( DWORD );
	char str[256];
	DWORD value;
	HKEY boinc_key;

	strcpy( str, "SOFTWARE\\BOINC\\" );
	strcat( str, name );

	if ( OSVersion == OS_WIN95 ) {
		error = RegOpenKeyEx( HKEY_LOCAL_MACHINE, "SOFTWARE\\BOINC",  
			0, KEY_ALL_ACCESS, &boinc_key );
		if ( error != ERROR_SUCCESS ) return -1;
	} else if ( OSVersion == OS_WINNT ) {
		error = RegOpenKeyEx( HKEY_CURRENT_USER, "SOFTWARE\\BOINC",  
			0, KEY_ALL_ACCESS, &boinc_key );
		if ( error != ERROR_SUCCESS ) return -1;
	} else {
		return -1;
	}

	error = RegQueryValueEx( boinc_key, name, NULL,
		&type, (BYTE *)&value, &size );

	keyval = value;

	RegCloseKey( boinc_key );

	if ( error != ERROR_SUCCESS ) return -1;

	return 0;
}

//////////
// Function:    UtilGetRegStr
// arguments:	name: name of key, str: where to store value of key
// returns:		int indicating error
// function:	reads string value in specified key
int UtilGetRegStr(char *name, char *str)
{
	LONG error;
	DWORD type = REG_SZ;
	DWORD size = 128;
	HKEY boinc_key;

	if ( OSVersion == OS_WIN95 ) {
		error = RegOpenKeyEx( HKEY_LOCAL_MACHINE, "SOFTWARE\\BOINC",  
			0, KEY_ALL_ACCESS, &boinc_key );
		if ( error != ERROR_SUCCESS ) return -1;
	} else if ( OSVersion == OS_WINNT ) {
		error = RegOpenKeyEx( HKEY_CURRENT_USER, "SOFTWARE\\BOINC",  
			0, KEY_ALL_ACCESS, &boinc_key );
		if ( error != ERROR_SUCCESS ) return -1;
	} else {
		return -1;
	}

	error = RegQueryValueEx( boinc_key, name, NULL,
		&type, (BYTE*)str, &size );

	RegCloseKey( boinc_key );

	if ( error != ERROR_SUCCESS ) return -1;

	return 0;
}

