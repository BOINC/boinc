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

#ifndef _BOINC_API_
#define _BOINC_API_

// NOTE: this is required on windows as well as unix, do not add "#ifndef
// _WIN32" -- if something is not working on a particular version of MSVC
// let's find a working solution
#include <string>
using namespace std;

#include "app_ipc.h"

/////////// API BEGINS HERE

extern "C" {
    extern int	boinc_set_error(int exit_code);

    extern int	boinc_init(bool standalone = false);
    extern int	boinc_finish(int);

    extern bool	boinc_is_standalone();

    extern int	boinc_resolve_filename(const char*, char*, int len);

    extern int	boinc_resolve_filename_s(const char*, string&);

    extern int	boinc_parse_init_data_file();
    extern int	boinc_write_init_data_file();
    extern int	boinc_get_init_data(APP_INIT_DATA&);

    extern int	boinc_send_trickle_up(char*);
    extern bool boinc_receive_trickle_down(char* buf, int len);

    extern bool	boinc_time_to_checkpoint();
    extern int	boinc_checkpoint_completed();

    extern int	boinc_fraction_done(double);

    extern int	boinc_wu_cpu_time(double&);
    extern int	boinc_thread_cpu_time(double&, double&);

} // extern "C"

/////////// API ENDS HERE

/////////// IMPLEMENTATION STUFF BEGINS HERE

extern APP_CLIENT_SHM *app_client_shm;

/////////// IMPLEMENTATION STUFF ENDS HERE

// Forward declare implementation functions - POSIX Platform Only.
#ifdef HAVE_SIGNAL_H
extern RETSIGTYPE boinc_catch_signal(int signal);
extern void boinc_quit(int sig);
#endif


#endif
