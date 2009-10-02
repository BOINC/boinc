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

$skip_auth_ops = true;

require_once("../inc/boinc_db.inc");
require_once("../inc/util_ops.inc");
require_once("../inc/email.inc");
require_once("../inc/user.inc");

// check for email/password case
//
$email_addr = strtolower(post_str("email_addr", true));
$passwd = stripslashes(post_str("passwd", true));

if ($email_addr && $passwd) {
    $user = lookup_user_email_addr($email_addr);
    if (!$user) {
        error_page("No account found with email address $email_addr");
    }
    $passwd_hash = md5($passwd.$email_addr);
    if ($passwd_hash != $user->passwd_hash) {
        error_page("Login failed");
    }
    $authenticator = $user->authenticator;
    $next_url = $_POST["next_url"];
    if (strlen($next_url) == 0) $next_url = "index.php";
    Header("Location: $next_url");
    $perm = $_POST['stay_logged_in'];
    send_cookie('auth', $authenticator, $perm, true);
}
?>
