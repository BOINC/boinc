// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifdef _WIN32
#include "boinc_win.h"
#endif
#include "parse.h"
#include "error_numbers.h"
#include "client_msgs.h"
#include "file_names.h"
#include "filesys.h"

#include "client_state.h"

#include "acct_mgr.h"

int ACCT_MGR::do_rpc(std::string url, std::string name, std::string password) {
    int retval;
    char buf[256];

    if (state != ACCT_MGR_STATE_IDLE) return 0;

    sprintf(buf, "%s?name=%s&password=%s", url.c_str(), name.c_str(), password.c_str());
    http_op.set_proxy(&gstate.proxy_info);
    boinc_delete_file(ACCT_MGR_REPLY_FILENAME);
    retval = http_op.init_get(buf, ACCT_MGR_REPLY_FILENAME, true);
    if (retval) return retval;
    retval = http_ops->insert(&http_op);
    if (retval) return retval;
    state = ACCT_MGR_STATE_BUSY;
    return 0;
}

bool ACCT_MGR::poll(double now) {
    if (state == ACCT_MGR_STATE_IDLE) return false;
    static double last_time=0;
    if (now-last_time < 10) return false;
    last_time = now;

    if (http_op.http_op_state == HTTP_STATE_DONE) {
        if (http_op.http_op_retval == 0) {
            FILE* f = fopen(ACCT_MGR_REPLY_FILENAME, "r");
            if (f) {
                MIOFILE mf;
                mf.init_file(f);
                parse(mf);
                fclose(f);
                handle_reply();
            }
        } else {
            msg_printf(NULL, MSG_ERROR,
                "Account manager RPC failed"
            );
        }
        state = ACCT_MGR_STATE_IDLE;
    }
    return true;
}

int ACCOUNT::parse(MIOFILE& in) {
	char buf[256];
	while (in.fgets(buf, sizeof(buf))) {
		if (match_tag(buf, "</account>")) {
			if (url.length() && authenticator.length()) return 0;
			return ERR_XML_PARSE;
		}
		if (parse_str(buf, "<url>", url)) continue;
		if (parse_str(buf, "<authenticator>", authenticator)) continue;
	}
	return ERR_XML_PARSE;
}

ACCOUNT::ACCOUNT() {}
ACCOUNT::~ACCOUNT() {}

ACCT_MGR::ACCT_MGR() {
    state = ACCT_MGR_STATE_IDLE;
}

int ACCT_MGR::parse(MIOFILE& in) {
    char buf[256];
    int retval;

    accounts.clear();
	while (in.fgets(buf, sizeof(buf))) {
        if (match_tag(buf, "<accounts>")) return 0;
		if (match_tag(buf, "<account>")) {
			ACCOUNT account;
			retval = account.parse(in);
			if (!retval) accounts.push_back(account);
		}
	}
    return ERR_XML_PARSE;
}

void ACCT_MGR::handle_reply() {
	unsigned int i;

	for (i=0; i<accounts.size(); i++) {
		ACCOUNT& acct = accounts[i];
		PROJECT* pp = gstate.lookup_project(acct.url.c_str());
		if (pp) {
			if (strcmp(pp->authenticator, acct.authenticator.c_str())) {
				msg_printf(NULL, MSG_ERROR,
					"You're attached to project %s with a different account"
				);
			}
		} else {
			gstate.add_project(acct.url.c_str(), acct.authenticator.c_str());
		}
	}
}
