<?php
// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2018 University of California
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
// along with BOINC.  If not, see <https://www.gnu.org/licenses/>.

// Constants for 
define("DELETE_ACCOUNT_METHOD_OBFUSCATE", 1);
define("DELETE_ACCOUNT_METHOD_WIPE", 2);

$config = get_config();
if ( !parse_bool($config, "enable_delete_account") ) {
    error_page(
        tra("This feature is disabled.  Please contact the project administrator.")
    );
}

function check_delete_account_token($userid, $token) {
    if( !is_valid_token($userid, $token, TOKEN_TYPE_DELETE_ACCOUNT) ) {
        sleep(LOGIN_FAIL_SLEEP_SEC);
        error_page(
            tra("The token you used has expired or is otherwise not valid.  Please request a new one <a href=\"delete_account_request.php\">here</a>")
        );
    }
}

/*
* This method selects which delete method to utilize.  Projects can implement their own method
* and make that a third mechanism if they have a need to
*/
function delete_account($userid) {
    $enable_delete_account = parse_int($config, "enable_delete_account");
    if ( $enable_delete_account == DELETE_ACCOUNT_METHOD_OBFUSCATE ) {
        return obfuscate_account($userid);
    } else if ( $enable_delete_account == DELETE_ACCOUNT_METHOD_WIPE ) {
        return wipe_account($userid);
    } else {
        error_page(
            tra("This feature is configured incorrectly.  Please contact the project administrator.")
        );
    }
}

// This method obfuscates all personal information but
// leaves critical db records in place
function obfuscate_account($userid) {
    
}

// This method deletes all rows from the database associated with the user
function wipe_account($userid) {
    
}