// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2011 University of California
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

// The following is an example trickle handler.
// It echoes whatever the app sent us, as a trickle-down message.
// Replace it with your own function.
//
// Note: you're passed the host ID (in mfh.hostid).
// From this you can get the HOST and USER records with
// DB_HOST host;
// DB_USER user;
// host.lookup_id(mfh.hostid);
// user.lookup_id(host.userid);

#include "error_numbers.h"
#include "str_replace.h"

#include "trickle_handler.h"

#include <ctime>

int handle_trickle_init(int, char**) {
    return 0;
}

int handle_trickle(MSG_FROM_HOST& mfh) {
    int retval;

    printf(
        "got trickle-up \n%s\n\n",
        mfh.xml
    );
    DB_MSG_TO_HOST mth;
    mth.clear();
    mth.create_time = time(0);
    mth.hostid = mfh.hostid;
    safe_strcpy(mth.variety, mfh.variety);
    mth.handled = false;
    sprintf(mth.xml,
        "<trickle_down>\n"
        "%s"
        "</trickle_down>\n",
        mfh.xml
    );
    retval = mth.insert();
    if (retval) {
        printf("insert failed: %s\n", boincerror(retval));
    }
    return 0;
}

