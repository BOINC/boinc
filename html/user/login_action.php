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

$next_url = $_POST["next_url"];
if (strlen($next_url) == 0) $next_url = "home.php";

// check for email/password case
//
$email_addr = strtolower(strip_tags(post_str("email_addr", true)));
$passwd = post_str("passwd", true);

if ($email_addr && $passwd) {
    $user = lookup_user_email_addr($email_addr);
    if (!$user) {
        page_head("No such account");
        echo "No account with email address <b>$email_addr</b> exists.
            Please try again.
        ";
        print_login_form_aux($next_url, null);
        page_tail();
        exit;
    }
	if (substr($user->authenticator, 0, 1) == 'x'){
		//User has been bad so we are going to take away ability to post for awhile.
		error_page("This account has been administratively disabled.");
	}
    $passwd_hash = md5($passwd.$email_addr);
    if ($passwd_hash != $user->passwd_hash) {
        page_head("Password incorrect");
        echo "The password you entered is incorrect. Please try again.\n";
        print_login_form_aux($next_url, null, $email_addr);
        page_tail();
        exit;
    }
    $authenticator = $user->authenticator;
    Header("Location: $next_url");
    $perm = $_POST['stay_logged_in'];
    send_cookie('auth', $authenticator, $perm);
    exit();
}

// check for time/id/hash case.

$id = get_int('id', true);
$t = get_int('t', true);
$h = get_str('h', true);
if ($id && $t && $h) {
    $user = BoincUser::lookup_id($id);
    if (!$user) {
        error_page("Invalid user ID.
            Please make sure you visited the complete URL;
            it may have been split across lines by your email reader."
        );
    }
    $x = $id.$user->authenticator.$t;
    $x = md5($x);
    $x = substr($x, 0, 16);
    if ($x != $h) {
        error_page("Invalid authenticator.
            Please make sure you visited the complete URL;
            it may have been split across lines by your email reader."
        );
    }
    if (time() - $t > 86400) {
        error_page("Link has expired;
            go <a href=get_passwd.php>here</a> to
            get a new login link by email."
        );
    }
    send_cookie('auth', $user->authenticator, true);
    Header("Location: home.php");
    exit();
}

// check for account key case.
// see if key is in URL; if not then check for POST data
//
$authenticator = get_str("key", true);
if (!$authenticator) {
    $authenticator = post_str("authenticator", true);
}
if (!$authenticator) {
    error_page("You must supply an account key");
}

if (substr($user->authenticator, 0, 1) == 'x'){
	//User has been bad so we are going to take away ability to post for awhile.
	error_page("This account has been administratively disabled.");
}
$user = lookup_user_auth($authenticator);
if (!$user) {
    page_head("Login failed");
    echo "There is no account with that authenticator.
        Please <a href=get_passwd.php>try again</a>.
    ";
    page_tail();
} else {
    Header("Location: $next_url");
    $perm = $_POST['stay_logged_in'];
    send_cookie('auth', $authenticator, $perm);
}
?>
