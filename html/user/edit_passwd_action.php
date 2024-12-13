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
require_once("../inc/user.inc");
require_once("../inc/password_compat/password.inc");

check_get_args(array());

$user = get_logged_in_user();

$token = post_str("token");
if ($token != $user->login_token) {
    error_page("bad token");
}
if (time() - $user->login_token_time > 86400) {
    error_page("expired token");
}

$passwd = post_str("passwd");

$config = get_config();
$min_passwd_length = parse_config($config, "<min_passwd_length>");
if (!$min_passwd_length) $min_passwd_length = 6;

if (!is_ascii($passwd)) {
    error_page(tra("Passwords may only include ASCII characters."));
}

if (strlen($passwd) < $min_passwd_length) {
    error_page(tra("New password is too short: minimum password length is %1 characters.", $min_passwd_length));
}

$passwd_hash = md5($passwd.$user->email_addr);
$database_passwd_hash = password_hash($passwd_hash, PASSWORD_DEFAULT);
$result = $user->update(" passwd_hash='$database_passwd_hash' ");
if (!$result) {
    error_page(tra("We can't update your password due to a database problem. Please try again later."));
}

page_head(tra("Change password"));
echo tra("Your password has been changed.");
page_tail();

?>
