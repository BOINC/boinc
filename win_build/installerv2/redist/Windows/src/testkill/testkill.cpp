// testkill.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "terminate.h"

int _tmain(int argc, _TCHAR* argv[])
{
	return TerminateProcessEx( tstring(_T("boinc.exe")) );;
}

