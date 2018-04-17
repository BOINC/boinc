<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
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
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

require_once("../inc/util.inc");
require_once("../inc/account.inc");
require_once("../inc/user_util.inc");
require_once("../inc/email.inc");
require_once("../inc/password_compat/password.inc");

$user = get_logged_in_user();

$config = get_config();
if ( !parse_bool($config, "enable_delete_account") ) {
    error_page(
        tra("This feature is disabled.  Please contact the project administrator.")
    );
}

//Verify password
$user = get_logged_in_user();
$passwd = post_str("passwd");

if( !check_passwd($user, $passwd) ) {
    sleep(LOGIN_FAIL_SLEEP_SEC);
    page_head("Password incorrect");
    echo "The password you entered is incorrect. Please go back and try again.\n";
    page_tail();
    exit;
}

send_confirm_delete_email($user);


page_head(tra("Confirmation Email Sent"));

echo "<p>".tra("The email to confirm your request to delete your account has been sent.")."</p>";

page_tail();
?>