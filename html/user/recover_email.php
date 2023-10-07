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

// If user has an email due to an email change made for their account,
// then they can visit this page with the token and user id to revert
// the change within 7 days of the initial change.
// Note:  The token is removed when it has been used.
//

require_once("../inc/util.inc");
require_once("../inc/token.inc");
require_once("../inc/email.inc");
require_once("../inc/password_compat/password.inc");

check_get_args(array("id", "token"));

$user = get_logged_in_user(false);
//Log out to clear all auth tokens
if ($user) {
    clear_cookie('auth');
    $g_logged_in_user = null;
}

page_head(tra("Recover email address"));

$userid = get_int("id", true);
$token = get_str("token", true);

if (is_valid_token($userid, $token, TOKEN_TYPE_CHANGE_EMAIL)) {
    $tmpuser = BoincUser::lookup_id_nocache($userid);
    // We can only change passwd_hash if we can get the userdata.
    //
    if ($tmpuser) {
        $existing = BoincUser::lookup_email_addr($tmpuser->previous_email_addr);
        if ($existing) {
            echo tra("There is already an account with that email address.")."<br /><br />".tra("Please contact the admin.  Previous email address could not be reverted as another account is using it as their email address.");
        } else {
            echo tra("Email address has been reverted.")."<br /><br />".tra("You need to reset your password:  ")."<a href=\"".secure_url_base()."get_passwd.php\">".secure_url_base()."get_passwd.php</a>";

            $database_passwd_hash = password_hash(random_string() , PASSWORD_DEFAULT);
            //Change previous_email
            $result = $tmpuser->update(
                "email_addr=previous_email_addr, previous_email_addr='', email_addr_change_time=0, passwd_hash='$database_passwd_hash', email_validated=0"
            );
            $result = delete_token($userid, $token, TOKEN_TYPE_CHANGE_EMAIL);
	}
    } else {
	echo tra("Invalid token.");
    }
} else {
    echo tra("Invalid token.");
}

page_tail();

?>
