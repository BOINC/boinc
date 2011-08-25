<?php
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

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/email.inc");
require_once("../inc/user.inc");

check_get_args(array());

$user = get_logged_in_user();

$email_addr = strtolower(post_str("email_addr"));
$passwd = post_str("passwd", true);

page_head(tra("Change email address of account"));

if (!is_valid_email_addr($email_addr)) {
    echo tra("New email address '%1' is invalid.", $email_addr);
} else if (is_banned_email_addr($email_addr)) {
    echo tra("New email address '%1' is invalid.", $email_addr);
} else if ($email_addr == $user->email_addr) {
    echo tra("New email address is same as existing address. Nothing is changed.");
} else {
    $existing = lookup_user_email_addr($email_addr);
    if ($existing) {
        echo tra("There's already an account with that email address");
    } else {
        $passwd_hash = md5($passwd.$user->email_addr);

        // deal with the case where user hasn't set passwd
        // (i.e. passwd is account key)
        //
        if ($passwd_hash != $user->passwd_hash) {
            $passwd = $user->authenticator;
            $passwd_hash = md5($passwd.$user->email_addr);
        }
        if ($passwd_hash != $user->passwd_hash) {
            echo tra("Invalid password.");
        } else {
            $passwd_hash = md5($passwd.$email_addr);
            $email_addr = BoincDb::escape_string($email_addr);
            $result = $user->update(
                "email_addr='$email_addr', passwd_hash='$passwd_hash', email_validated=0"
            );
            if ($result) {
                echo tra("The email address of your account is now %1.", $email_addr);
                if (defined("SHOW_NONVALIDATED_EMAIL_ADDR")) {
                    echo "<p>".tra("Please %1validate this email address%2.", "<a href=validate_email_addr.php>", "</a>")."\n";
                }
            } else {
                echo tra("We can't update your email address due to a database problem.  Please try again later.");
            }
        }
    }
}

page_tail();
?>
