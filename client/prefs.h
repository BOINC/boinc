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

#ifndef _PREFS_
#define _PREFS_

#include <vector>
#include "client_types.h"

// Implementation notes on preferences:
//
// Preferences are edited and stored on BOINC servers.
// The native representation of preferences is XML.
// The client maintains the preferences (in XML form)
// and mod time in the state file and in memory.
// It includes these items in each scheduler request message.
// A scheduler reply message may contain a more recent set of preferences.
//
// Some preference items are used by the core client.
// The XML is parsed into a structure containing these items.
//
// Preferences may include project-specific elements.
// When an application is run, the core client checks for project-specific
// preferences and puts them in a file "prefs.xml" in the slot directory.

// The following structure is a parsed version of the prefs file
//
struct PREFS {
    int mod_time;
    vector<PROJECT*> projects;
    bool dont_run_on_batteries;
    bool dont_run_if_user_active;
    bool confirm_before_connecting;
    double high_water_days;
    double low_water_days;
    double disk_max_used_gb;
    double disk_max_used_pct;
    double disk_min_free_gb;

    PREFS();
    int parse(FILE*);
    int parse_file();
    bool looks_reasonable(PROJECT& project);
};

extern int write_initial_prefs(char* master_url, char* authenticator);
extern int initialize_prefs();
    // the implementation is different between CLI and GUI

#endif
