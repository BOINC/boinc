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

require_once("../inc/util.inc");
require_once("../inc/user.inc");
require_once("../inc/host.inc");
require_once("../inc/friend.inc");
require_once("../inc/boinc_db.inc");
require_once("../project/project.inc");

// Constants for different methods of deleting accounts
// These correspond to the value used in the config.xml 
// field of <enable_delete_account/>
define("DELETE_ACCOUNT_METHOD_OBFUSCATE", 1);
define("DELETE_ACCOUNT_METHOD_WIPE", 2);
define("DELETE_ACCOUNT_METHOD_PROJECT_DEFINED", 3);

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
function delete_account($user) {
    $config = get_config();
    $enable_delete_account = parse_config($config, "<enable_delete_account>");
    if ( $enable_delete_account == DELETE_ACCOUNT_METHOD_OBFUSCATE ) {
        return obfuscate_account($user);
    } else if ( $enable_delete_account == DELETE_ACCOUNT_METHOD_WIPE ) {
        return wipe_account($user);
    } else if ( $enable_delete_account == DELETE_ACCOUNT_METHOD_PROJECT_DEFINED ) {
        return project_delete_account($user);
    } else {
        error_page(
            tra("This feature is configured incorrectly.  Please contact the project administrator.")
        );
    }
}

// "obfuscate" an account: leave user record (for DB consistency) but:
//  - set email address and authenticator to "deleted_pid_time"
//  - clear name, country, postal_code
//  - remove from team
//  - delete posts, subscriptions, and forum prefs
//  - delete private messages (sent and received)
//  - delete profile and associated image
// for each host:
//  - clear domain_name, last_ip_addr
//
function obfuscate_account($user) {
    BoincUserDeleted::insert_user($user);
    BoincHostDeleted::insert_hosts_for_user($user);
    $x = "deleted_".time()."_".random_string();
    $retval = $user->update("email_addr='$x', authenticator='$x', name='deleted', country='', postal_code='', has_profile=0");
    if (!$retval) return false;
    user_quit_team($user);
    forum_delete_user($user);
    pm_delete_user($user);
    anonymize_hosts($user);
    delete_profile($user);
    delete_friends($user);
    return true;
}

// This method deletes all rows from the database associated with the user
function wipe_account($user) {
    error_page(
        tra("Not Implemented Yet")
    );
}