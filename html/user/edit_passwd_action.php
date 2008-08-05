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
require_once("../inc/user.inc");

$auth = post_str("auth", true);
$email_addr = strtolower(post_str("email_addr", true));

$old_passwd = post_str("old_passwd", true);
$passwd = post_str("passwd");
$passwd2 = post_str("passwd2");

if ($passwd != $passwd2) {
    error_page("New passwords are different");
}

$config = get_config();
$min_passwd_length = parse_config($config, "<min_passwd_length>");
if (!$min_passwd_length) $min_passwd_length = 6;

if (!is_ascii($passwd)) {
    error_page("Passwords may only include ASCII characters.");
}

if (strlen($passwd)<$min_passwd_length) {
    error_page(
        "New password is too short:
        minimum password length is $min_passwd_length characters."
    );
}
if ($auth) {
    $user = lookup_user_auth($auth);
    if (!$user) {
        error_page("Invalid account key");
    }
} else {
    $user = lookup_user_email_addr($email_addr);
    if (!$user) {
        error_page("No account with that email address was found");
    }
    $passwd_hash = md5($old_passwd.$email_addr);
    if ($user->passwd_hash != $passwd_hash) {
        error_page("Invalid password");
    }
}

page_head("Change password");
$passwd_hash = md5($passwd.$user->email_addr);
$result = $user->update("passwd_hash='$passwd_hash'");
if ($result) {
    echo "Your password has been changed.";
} else {
    echo "
        We can't update your password due to a database problem.
        Please try again later.
    ";
}

page_tail();
?>
