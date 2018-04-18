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
require_once("../inc/token.inc");
require_once("../inc/boinc_db.inc");
require_once("../inc/user_util.inc");

//Make sure feature is enabled
$config = get_config();
if ( !parse_bool($config, "enable_delete_account") ) {
    error_page(
        tra("This feature is disabled.  Please contact the project administrator.")
    );
}

//Make sure the token is still valid
$userid = post_int("id");
$token = post_str("token");
if( !is_valid_delete_account_token($userid, $token) ) {
    sleep(LOGIN_FAIL_SLEEP_SEC);
    error_page(
        tra("The token you used has expired or is otherwise not valid.  Please request a new one <a href=\"delete_account_request.php\">here</a>")
    );
}


//Verify password
$user = $user = BoincUser::lookup_id($userid);
$passwd = post_str("passwd");

if( !check_passwd($user, $passwd) ) {
    sleep(LOGIN_FAIL_SLEEP_SEC);
    page_head("Password incorrect");
    echo "The password you entered is incorrect. Please go back and try again.\n";
    page_tail();
    exit;
}

//do account delete

page_head(tra("Account Deleted"));

echo "<p>".tra("Your account has been deleted.  If you want to contribute to ".PROJECT." in the future you will need to create a new account.")."</p>";

page_tail();
?>