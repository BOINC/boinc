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

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/email.inc");
require_once("../project/project.inc");

check_get_args(array());

function email_sent_message($email_addr) {
    if (defined('EMAIL_FROM')) {
        $email_from = EMAIL_FROM;
    } else {
        $email_from = secure_url_base();
    }

    page_head("Email sent");
    echo "
        <p>
        Instructions for resetting your password have been emailed to $email_addr.
        <p>
        If the email doesn't arrive in a few minutes, check your spam folder.
    ";
    if (defined('MAIL_PASSWD_EXTRA')) {
        echo MAIL_PASSWD_EXTRA;
    }
}

$email_addr = strtolower(post_str("email_addr"));
$email_addr = sanitize_email($email_addr);
if (!strlen($email_addr)) {
    error_page("no address given");
}
$user = BoincUser::lookup_email_addr($email_addr);

if (!$user) {
    page_head("No such user");
    echo "There is no account with email address $email_addr. <br>
        Try reentering your email address.<p>
    ";
} else {
    if (substr($user->authenticator, 0, 1) == 'x') {
        page_head("Account currently disabled");
        echo "This account has been administratively disabled.";
    } else {
        $user->email_addr = $email_addr;
        $retval = send_auth_email($user);
        if ($retval) {
            email_sent_message($email_addr);
        } else {
            page_head("Email failed");
            echo "Can't send email to $user->email_addr";
        }
    }
}

page_tail();

?>
