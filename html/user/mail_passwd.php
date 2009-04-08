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
require_once("../project/project.inc");

function email_sent_message($email_addr) {
    if (defined('EMAIL_FROM')) {
        $email_from = EMAIL_FROM;
    } else {
        $email_from = URL_BASE;
    }

    page_head("Email sent");
    echo "
        Instructions have been emailed to $email_addr.
        <p>
        If the email doesn't arrive in a few minutes,
        your ISP may be blocking it as spam.
        In this case please contact your ISP and
        ask them to not block email from $email_from.
    ";
}

$email_addr = strtolower(post_str("email_addr"));
if (!strlen($email_addr)) {
    error_page("no address given");
}
$user = lookup_user_email_addr($email_addr);

if (!$user) {
    page_head("No such user");
    echo "There is no user with email address $email_addr. <br>
        Try reentering your email address.<p>
    ";
} else {
	if (substr($user->authenticator, 0, 1) == 'x') {
		page_head("Account Currently Disabled");
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
