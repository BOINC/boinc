// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
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

// command-line version of the BOINC core client

// This file contains no GUI-related code,
// and is not included in the source code for Mac or Win GUI clients

#ifdef WIN32
#include <afxwin.h>
#include "stackwalker.h"
#include "win_service.h"
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#include "client_state.h"
#include "error_numbers.h"
#include "file_names.h"
#include "log_flags.h"
#include "prefs.h"
#include "util.h"
#include "message.h"

// dummies
void create_curtain(){}
void delete_curtain(){}
void guiOnBenchmarksBegin(){}
void guiOnBenchmarksEnd(){}

void CLIENT_STATE::approve_executables() {
    unsigned int i;
    char buf[256];

    for (i=0; i<file_infos.size(); i++) {
        FILE_INFO* fip = file_infos[i];
        if (fip->approval_pending) {
            printf(
                "BOINC has received the executable file %s\n"
                "from project %s.\n"
                "Its MD5 checksum is %s\n"
                "Should BOINC accept this file (y/n)? ",
                fip->name,
                fip->project->project_name,
                fip->md5_cksum
            );
            scanf("%s", buf);
            if (buf[0]=='y' || buf[0]=='Y') {
                fip->approval_pending = false;
                set_client_state_dirty("approve_executables");
            } else {
                msg_printf(fip->project, MSG_INFO, "User rejected executable: %s", fip->name);
                fip->status = ERR_USER_REJECTED;
            }
        }
    }
}

// This gets called when the client fails to add a project
//
void project_add_failed(PROJECT* project) {
    if (project->scheduler_urls.size()) {
        printf(
            "BOINC failed to log in to %s.\n "
            "Please check your account ID and try again.\n",
            project->master_url
        );
    } else {
        printf(
            "BOINC couldn't get main page for %s.\n"
            "Please check the URL and try again.\n",
            project->master_url
        );
    }
    gstate.detach_project(project);
    exit(1);
}

// Display a message to the user.
// Depending on the priority, the message may be more or less obtrusive
//
void show_message(PROJECT *p, char* msg, int priority) {
    char* x;
    char message[1024];
    char event_message[2048];

    strcpy(message, msg);
    if (message[strlen(message)-1] == '\n') {
        message[strlen(message)-1] = 0;
    }

    if (p) {
        x = p->get_project_name();
    } else {
        x = "BOINC";
    }
    switch (priority) {
    case MSG_ERROR:
        fprintf(stderr, "%s [%s] %s\n", timestamp(), x, message);
        printf("%s [%s] %s\n", timestamp(), x, message);
		if (gstate.executing_as_windows_service)
		{
#if defined(_WIN32) && defined(_CONSOLE)
		    _stprintf(event_message, TEXT("%s [%s] %s\n"), timestamp(), x, message);
			LogEventErrorMessage(event_message);
#endif
		}
		break;
    case MSG_WARNING:
        printf("%s [%s] %s\n", timestamp(), x, message);
		if (gstate.executing_as_windows_service)
		{
#if defined(_WIN32) && defined(_CONSOLE)
		    _stprintf(event_message, TEXT("%s [%s] %s\n"), timestamp(), x, message);
			LogEventWarningMessage(event_message);
#endif
		}
		break;
    case MSG_INFO:
        printf("%s [%s] %s\n", timestamp(), x, message);
		if (gstate.executing_as_windows_service)
		{
#if defined(_WIN32) && defined(_CONSOLE)
		    _stprintf(event_message, TEXT("%s [%s] %s\n"), timestamp(), x, message);
			LogEventInfoMessage(event_message);
#endif
		}
		break;
    }
}

// Prompt user for project URL and authenticator,
// and create an account file
//
int add_new_project() {
    PROJECT project;

    printf("Enter the URL of the project: ");
    scanf("%s", project.master_url);
    printf(
        "You should have already registered with the project\n"
        "and received an account key by email.\n"
        "Paste the account key here: "
    );
    scanf("%s", project.authenticator);

    project.tentative = true;
    project.write_account_file();
    return 0;
}

void quit_client(int a) {
    gstate.requested_exit = true;
}

void susp_client(int a) {
    gstate.active_tasks.suspend_all();
    msg_printf(NULL, MSG_INFO, "Suspending activity - user request");
#ifndef _WIN32
	signal(SIGTSTP, SIG_DFL);
    raise(SIGTSTP);
#endif
}

void resume_client(int a) {
    gstate.active_tasks.unsuspend_all();
    msg_printf(NULL, MSG_INFO, "Resuming activity");
}

#ifdef _WIN32
BOOL WINAPI ConsoleControlHandler ( DWORD dwCtrlType ){
	BOOL bReturnStatus = FALSE;
	switch( dwCtrlType ){
        case CTRL_C_EVENT:
			if(gstate.activities_suspended)
                resume_client(NULL);
			else
                susp_client(NULL);
			bReturnStatus =  TRUE;
            break;
		case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_LOGOFF_EVENT:
        case CTRL_SHUTDOWN_EVENT:
			quit_client(NULL);
            bReturnStatus =  TRUE;
            break;
	}
	return bReturnStatus;
}
#endif

int boinc_execution_engine(int argc, char** argv) {

#ifdef WIN32
	InitAllocCheck();
#endif

	int retval;
    double dt;

    setbuf(stdout, 0);
    if (lock_file(LOCK_FILE_NAME)) {
        fprintf(stderr, "Another copy of BOINC is already running\n");
        exit(1);
    }

// Unix/Linux console controls
#ifndef WIN32
    // Handle quit signals gracefully
    signal(SIGHUP, quit_client);
    signal(SIGINT, quit_client);
    signal(SIGQUIT, quit_client);
#ifdef SIGPWR
    signal(SIGPWR, quit_client);
#endif
    signal(SIGTSTP, susp_client);
    signal(SIGCONT, resume_client);
#endif

// Windows console controls
#ifdef WIN32
    if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleControlHandler, TRUE)){
        fprintf(stderr, "Failed to register the console control handler\n");
        exit(1);
	} else {
		printf(
			"\nTo pause/resume tasks hit CTRL-C, to exit hit CTRL-BREAK\n"
		);
	}
#endif

	read_log_flags();
    gstate.parse_cmdline(argc, argv);
    gstate.parse_env_vars();
    retval = gstate.init();
    if (retval) exit(retval);


    while (1) {
        if (!gstate.do_something()) {
            dt = dtime();
            gstate.net_sleep(1.);
            dt = dtime() - dt;
            log_messages.printf(ClientMessages::DEBUG_TIME, "SLEPT %f SECONDS\n", dt);
            fflush(stdout);
        }

        // check for executables files that need approval;
        // in the GUI version this is done in event loop
        //
        gstate.approve_executables();

        if (gstate.time_to_exit()) {
            msg_printf(NULL, MSG_INFO, "Time to exit");
            break;
        }
        if (gstate.requested_exit) {
            msg_printf(NULL, MSG_INFO, "Exit requested by signal");
            break;
        }
    }
    gstate.cleanup_and_exit();

#ifdef WIN32
	DeInitAllocCheck();
#endif

	return 0;
}


#if defined(_WIN32) && defined(_CONSOLE)

//
// On Windows, we support running as a Windows Service which requires
//     a different startup method
//
int main(int argc, char** argv) {

	int iRetVal = 0;

	SERVICE_TABLE_ENTRY dispatchTable[] =
    {
		{ TEXT(SZSERVICENAME), (LPSERVICE_MAIN_FUNCTION)service_main },
		{ NULL, NULL }
	};

    if ( (argc > 1) &&
         ((*argv[1] == '-') || (*argv[1] == '/')) )
    {
        if ( _stricmp( "install", argv[1]+1 ) == 0 )
        {
            CmdInstallService();
        }
        else if ( _stricmp( "uninstall", argv[1]+1 ) == 0 )
        {
            CmdUninstallService();
        }
 		else if ( _stricmp( "win_service", argv[1]+1 ) == 0 )
        {
			printf( "\nStartServiceCtrlDispatcher being called.\n" );
			printf( "This may take several seconds.  Please wait.\n" );

			if (!StartServiceCtrlDispatcher(dispatchTable))
				LogEventErrorMessage(TEXT("StartServiceCtrlDispatcher failed."));
        }
		else
		{
			iRetVal = boinc_execution_engine(argc, argv);
		}
    }
	else
	{
		iRetVal = boinc_execution_engine(argc, argv);
	}

    return iRetVal;
}

#elif

//
// For platforms other than windows just treat it as a console application
//
int main(int argc, char** argv) {
	return boinc_execution_engine(argc, argv);
}

#endif


