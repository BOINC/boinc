<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2014 University of California
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

// This changes the user email, and then sets previous_email_addr to the old
// user email address for recovery when needed.  It triggers emails to both
// addresses for a way to revert it to the previous address.
//

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/email.inc");
require_once("../inc/user_util.inc");
require_once("../inc/password_compat/password.inc");

check_get_args(array());

$user = get_logged_in_user();

$email_addr = strtolower(post_str("email_addr"));
$passwd = post_str("passwd", true);

page_head(tra("Change email address of account"));

if (!is_valid_email_syntax($email_addr)) {
    echo tra("New email address '%1' is invalid.", $email_addr);
} else if (!is_valid_email_sfs($email_addr)) {
    echo tra("Email address '%1' is flagged by stopforumspam.com.", $email_addr);
} else if (is_banned_email_addr($email_addr)) {
    echo tra("New email address '%1' is banned.", $email_addr);
} else if ($email_addr == $user->email_addr) {
    echo tra("New email address is same as existing address. Nothing is changed.");
} else if ($user->email_addr_change_time + 604800 > time()) {
    echo tra("Email address was changed within the past 7 days, please look for an email to $user->previous_email_addr if this email change is incorrect.");
} else {
    $existing = BoincUser::lookup_email_addr($email_addr);
    if ($existing) {
        echo tra("There's already an account with that email address");
    } else {
        $passwd_hash = md5($passwd.$user->email_addr);

        // deal with the case where user hasn't set passwd
        // (i.e. passwd is account key)
        //
        if ($passwd_hash != $user->passwd_hash && !password_verify($passwd_hash, $user->passwd_hash)) {
            $passwd = $user->authenticator;
            $passwd_hash = md5($passwd.$user->email_addr);
        }
        if ($passwd_hash != $user->passwd_hash && !password_verify($passwd_hash, $user->passwd_hash)) {
            echo tra("Invalid password.");
        } else {
            $passwd_hash = md5($passwd.$email_addr);
            $database_passwd_hash = password_hash($passwd_hash , PASSWORD_DEFAULT);
            $email_addr = BoincDb::escape_string($email_addr);
            $user->email_addr_change_time = time();
            $result = $user->update(
                "email_addr='$email_addr', previous_email_addr='$user->email_addr', email_addr_change_time=$user->email_addr_change_time, passwd_hash='$database_passwd_hash', email_validated=0"
            );
            $user->previous_email_addr = $user->email_addr;
            $user->email_addr = $email_addr;
            if ($result) {
                echo tra("The email address of your account is now %1.", $email_addr);
                if (defined("SHOW_NONVALIDATED_EMAIL_ADDR")) {
                    echo "<p>".tra("Please %1 validate this email address %2.", "<a href=validate_email_addr.php>", "</a>")."\n";
                }
                send_changed_email($user);
            } else {
                echo tra("We can't update your email address due to a database problem.  Please try again later.");
            }
        }
    }
}

page_tail();
?>
