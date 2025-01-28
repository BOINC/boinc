// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

#ifndef BOINC_ACCT_MGR_H
#define BOINC_ACCT_MGR_H

#include <string>
#include <vector>

#include "str_replace.h"
#include "miofile.h"
#include "parse.h"
#include "keyword.h"
#include "gui_http.h"
#include "client_types.h"

// represents an account manager account to which
// we're attached or potentially attached.
// Info stored in acct_mgr_url.xml and acct_mgr_login.xml
//
// If you add stuff here, add code to
// - ACCT_MGR_INFO::clear()
// - ACCT_MGR_OP::parse()
// - ACCT_MGR_OP::handle_reply()

struct ACCT_MGR_INFO : PROJ_AM {
    // the following used to be std::string but there
    // were mysterious bugs where setting it to "" didn't work
    //

    // Account managers originally authenticated with name/password;
    // e.g. BAM!, Gridrepublic.
    // This has drawbacks, e.g. no way to change password.
    // So we added the option of using a random-string authenticator.
    // If this is present, use it rather than name/passwd.
    //
    char login_name[256];   // unique name (could be email addr)
    char user_name[256];    // non-unique name
    char team_name[256];
    char password_hash[256];
        // md5 of password.lowercase(login_name)
    char authenticator[256];
    char opaque[256];
        // opaque data, from the AM, to be included in future AM requests
    char signing_key[MAX_KEY_LEN];
    char previous_host_cpid[64];
        // the host CPID sent in last RPC
    double next_rpc_time;
    int nfailures;
    bool send_gui_rpc_info;
        // whether to include GUI RPC port and password hash
        // in AM RPCs (used for "farm management")
    bool no_project_notices;
        // if set, don't show notices from projects
    bool send_tasks_all;
    bool send_tasks_active;

    // TODO: get rid of the following here and in the manager
    bool cookie_required;
        // use of cookies are required during initial signup
        // NOTE: This bool gets dropped after the client has
        //   successfully attached to an account manager
    char cookie_failure_url[256];
        // if the cookies could not be detected, provide a
        // link to a website to go to so the user can find
        // what login name and password they have been assigned

    bool password_error;
    bool dynamic;
        // This AM dynamically decides what projects to assign.
        // - send EC in AM RPCs
        // - send starvation info if idle resources
        // - network preferences are those from AM
    USER_KEYWORDS user_keywords;
        // user's yes/no keywords.
        // These are conveyed to projects in scheduler requests

    // vars related to starvation prevention,
    // where we issue a "starved RPC" if a resource has been idle
    // for more than 10 min

    double first_starved;           // start of starvation interval
    double starved_rpc_backoff;     // interval between starved RPCs
    double starved_rpc_min_time;    // earliest time to do a starved RPC

    inline bool using_am() {
        if (!strlen(master_url)) return false;
        if (strlen(authenticator)) return true;
        if (!strlen(login_name)) return false;
        if (!strlen(password_hash)) return false;
        return true;
    }
    inline bool same_am(const char* mu, const char* ln, const char* ph, const char* auth) {
        if (strcmp(mu, master_url)) return false;
        if (!strcmp(auth, authenticator)) return true;
        if (strcmp(ln, login_name)) return false;
        if (strcmp(ph, password_hash)) return false;
        return true;
    }
    inline bool get_no_project_notices() {
        if (!using_am()) return false;
        return no_project_notices;
    }

    ACCT_MGR_INFO();
    int parse_login_file(FILE*);
    int write_info();
    int init();
    void clear();
    bool poll();
};

// stuff after here related to RPCs to account managers

struct OPTIONAL_BOOL {
    bool present;
    bool value;
    inline void init() {present=false;}
    inline void set(bool v) {value=v; present=true;}
};

struct OPTIONAL_DOUBLE {
    bool present;
    double value;
    inline void init() {present=false;}
    inline void set(double v) {value=v; present=true;}
};

// an account entry in reply message
//
struct AM_ACCOUNT {
    std::string url;
    std::string authenticator;

    char url_signature[MAX_SIGNATURE_LEN];
    bool detach;
    bool update;
    bool no_rsc[MAX_RSC];
        // instructions from AM not to use various resources
    OPTIONAL_BOOL dont_request_more_work;
    OPTIONAL_BOOL detach_when_done;
    OPTIONAL_DOUBLE resource_share;
    OPTIONAL_BOOL suspend;
    OPTIONAL_BOOL abort_not_started;

    void handle_no_rsc(const char*, bool);
    int parse(XML_PARSER&);
    AM_ACCOUNT() {
        safe_strcpy(url_signature, "");
        detach = false;
        update = false;
        dont_request_more_work.init();
        detach_when_done.init();
        resource_share.init();
        suspend.init();
        abort_not_started.init();
    }
    ~AM_ACCOUNT() {}
};

struct ACCT_MGR_OP: public GUI_HTTP_OP {
    bool via_gui;
    int error_num;
    ACCT_MGR_INFO ami;
        // a temporary copy while doing RPC.
        // CLIENT_STATE::acct_mgr_info is authoritative
    std::string error_str;
    std::vector<AM_ACCOUNT> accounts;
    double repeat_sec;
    std::string global_prefs_xml;
    char host_venue[256];
    bool got_rss_feeds;
    std::vector<RSS_FEED>rss_feeds;

    int do_rpc(ACCT_MGR_INFO&, bool via_gui);
    int parse(FILE*);
    virtual void handle_reply(int http_op_retval);

    ACCT_MGR_OP(GUI_HTTP* p) {
        gui_http = p;
        via_gui = false;
        error_num = BOINC_SUCCESS;
        repeat_sec = 60.0;
        global_prefs_xml.clear();
        safe_strcpy(host_venue, "");
        got_rss_feeds = false;
    }
    virtual ~ACCT_MGR_OP(){}
};

#endif
