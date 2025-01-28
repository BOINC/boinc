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

require_once("../inc/common_defs.inc");
require_once("../inc/util.inc");
require_once("../inc/user.inc");
require_once("../inc/user_util.inc");
require_once("../inc/host.inc");
require_once("../inc/friend.inc");
require_once("../inc/boinc_db.inc");
require_once("../inc/submit_util.inc");
require_once("../project/project.inc");

// Constants for different methods of deleting accounts
// These correspond to the value used in the config.xml
// field of <enable_delete_account/>
//
define("DELETE_ACCOUNT_METHOD_OBFUSCATE", 1);
define("DELETE_ACCOUNT_METHOD_WIPE", 2);
define("DELETE_ACCOUNT_METHOD_PROJECT_DEFINED", 3);

// Constant for how long to sleep after invalidating authenticator
// before proceeding with rest of delete
// This is done on the chance that there is an active scheduler request
// in progress
//
if (!defined("DELETE_DELAY")) define("DELETE_DELAY", 2);

function is_delete_account_token_valid($userid, $token) {
    if (!is_valid_token($userid, $token, TOKEN_TYPE_DELETE_ACCOUNT) ) {
        sleep(LOGIN_FAIL_SLEEP_SEC);
        return false;
    }
    return true;
}

// Save the minimal information from the user and their hosts
// so that db_dump can provide the information necessary
// to export the deleted_user and deleted_host files.
// These records are deleted after 60 days by the
// daily ops task "delete_expired_users_and_hosts.php"
//
function insert_deleted_records($user) {
    BoincUserDeleted::insert_user($user);
    BoincHostDeleted::insert_hosts_for_user($user);
}

// This method selects which delete method to utilize.
// Projects can implement their own method
// and make that a third mechanism if they have a need to
//
function delete_account($user) {
    $config = get_config();
    $enable_delete_account = parse_config($config, "<enable_delete_account>");
    if ($enable_delete_account == DELETE_ACCOUNT_METHOD_OBFUSCATE) {
        return obfuscate_account($user);
    } else if ($enable_delete_account == DELETE_ACCOUNT_METHOD_WIPE) {
        return wipe_account($user);
    } else if ($enable_delete_account == DELETE_ACCOUNT_METHOD_PROJECT_DEFINED) {
        return project_delete_account($user);
    }
    return ERR_NO_OPTION;
}

// invalidate the authenticator and then sleep for DELETE_DELAY seconds
// in order to let any active scheduler requests complete.
//
function invalidate_authenticator($user) {
    $x = "deleted_".time()."_".random_string();
    $retval = $user->update("authenticator='$x'");
    if (!$retval) return ERR_DB_NOT_FOUND;
    sleep(DELETE_DELAY);
    return 0;
}

// "obfuscate" an account: leave user record (for DB consistency) but:
//  - set email address and authenticator to "deleted_time_randomstring"
//  - clear name, country, postal_code
//  - remove from team
//  - delete posts, subscriptions, and forum prefs
//  - delete private messages (sent and received)
//  - delete profile and associated image
// for each host:
//  - clear domain_name, last_ip_addr
//
function obfuscate_account($user) {
    $retval = invalidate_authenticator($user);
    if ($retval) return $retval;
    insert_deleted_records($user);
    $x = "deleted_".time()."_".random_string();
    $retval = $user->update("email_addr='$x', authenticator='$x', name='deleted', country='', postal_code='', has_profile=0");
    if (!$retval) return ERR_DB_NOT_FOUND;
    user_quit_team($user);
    forum_delete_user($user);
    pm_delete_user($user);
    anonymize_hosts($user);
    delete_profile($user);
    delete_friends($user);
    return 0;
}


// return true if the result is in progress
//
function is_in_progress($res) {
    return ($res->server_state == RESULT_SERVER_STATE_IN_PROGRESS);
}

// returns true if the result finished successfully but is either
// pending validation or inconclusive
//
function is_over_but_not_validated($res) {
    if ($res->server_state == RESULT_SERVER_STATE_OVER && $res->outcome == RESULT_OUTCOME_SUCCESS &&
            ($res->validate_state == VALIDATE_STATE_INIT || $res->validate_state == VALIDATE_STATE_INCONCLUSIVE) ) {
        return true;
    }
    return false;
}

function transition_workunit($res) {
    $now = time();
    BoincWorkunit::update_aux("transition_time=$now where id=$res->workunitid");
}

// This method handles dissassociating the user from their results.
// It will cancel all in progress or returned, but not yet validated
// results for a user.  For other results, it will set the userid and
// hostid fields to 0
//
function cancel_results_for_user($user) {
    $ress = BoincResult::enum("userid = $user->id");
    $cancel_clause="server_state=".RESULT_SERVER_STATE_OVER.", outcome=".RESULT_OUTCOME_CLIENT_DETACHED.", validate_state=".VALIDATE_STATE_INVALID;
    $set_id_clause="hostid=0, userid=0";
    foreach ($ress as $res) {
        if (is_in_progress($res) || is_over_but_not_validated($res)) {
            $res->update($cancel_clause.", ".$set_id_clause);
            transition_workunit($res);
        } else {
            $res->update($set_id_clause);
        }
    }
}

// This method deletes all rows from the database associated with the user
//
function wipe_account($user) {
    $retval = invalidate_authenticator($user);
    if ($retval) return $retval;

    //insert records into tables for db_dump to announce deletion of user
    //
    insert_deleted_records($user);

    // delete remote submit user
    //
    delete_remote_submit_user($user);  // from submit_util.inc

    // remove user's team records
    //
    user_erase_team_owner($user);      // from team.inc
    user_quit_team($user);        // from team.inc
    user_erase_team_delta($user); // from team.inc

    // Items that do not have logic elsewhere
    // and do not have objects in boinc_db.inc
    //
    $db = BoincDb::get();
    if (!$db) die("no DB connection");
    $db->do_query("delete from credited_job where userid = $user->id");
    $db->do_query("delete from donation_paypal where userid = $user->id");
    $db->do_query("delete from banishment_vote where userid = $user->id");
    $db->do_query("delete from post_ratings where post in ( select id from post where user = $user->id )");
    $db->do_query("delete from post_ratings where user = $user->id");
    $db->do_query("delete from msg_from_host where hostid in (select id from host where userid = $user->id )");
    $db->do_query("delete from msg_to_host where hostid in (select id from host where userid = $user->id )");
    $db->do_query("delete from sent_email where userid = $user->id");

    //It is much faster to update results with single query
    //
    cancel_results_for_user($user);

    BoincHostAppVersion::delete_for_user($user->id);
    BoincHost::delete_for_user($user->id);
    BoincConsent::delete_for_user($user->id);
    BoincTeamAdmin::delete("userid=$user->id");

    // final action
    delete_user($user);  //from user_util.inc
    return 0;
}
