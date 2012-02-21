<?php

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

// "delete" an account:
//  - set email address and authenticator to "deleted_pid_time"
//  - clear name, country, postal_code
//  - remove from team
//  - delete posts, subscriptions, and forum prefs
//  - delete private messages (sent and received)
//  - delete profile and associated image
// for each host:
//  - clear domain_name, last_ip_addr

require_once("../inc/util.inc");
require_once("../inc/host.inc");
require_once("../inc/profile.inc");
require_once("../inc/pm.inc");

check_get_args(array("cmd"));

die("This feature has been disabled.  Please contact project administators.");

function delete_account($user) {
    $x = "deleted_".time()."_".random_string();
    $retval = $user->update("email_addr='$x', authenticator='$x', name='', country='', postal_code='', has_profile=0");
    if (!$retval) return false;
    user_quit_team($user);
    forum_delete_user($user);
    pm_delete_user($user);
    anonymize_hosts($user);
    delete_profile($user);
    return true;
}

$user = get_logged_in_user();

$cmd = get_str("cmd", true);
if ($cmd == "delete") {
    $retval = delete_account($user);
    if (!$retval) {
        error_page(tra("Couldn't delete account"));
    } else {
        page_head(tra("Account deleted"));
        echo tra("Your account has been deleted.");
        page_tail();
    }
} else {
    page_head(tra("Confirm delete account"));
    echo "
        <table><tr><td>
        ".tra("Deleting your account will remove all of your
personal information from our servers,
including your profile and message-board posts.
No jobs will be issued to any computers attached
to this account.")."
        <p>
        <b>".tra("This cannot be undone.
Once your account has been deleted, you cannot get it back.")."
        <p>
        ".tra("Are you sure you want to delete your account?")."</b>
        <p>
    ";
    show_button("delete_account.php?cmd=delete", tra("Yes"), tra("Delete this account"));
    show_button("index.php", tra("No"), tra("Do not delete this account"));
    echo "</td></tr></table>\n";

    page_tail();
}
