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

// validate a user's credentials, and log them in by sending a cookie.
// This handles several cases:
//
// 1) login via web form w/ email addr and password
// 2) login via web form w/ authenticator
// 3) login via a URL sent in email (e.g. to reset password)

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/email.inc");
require_once("../inc/user.inc");
require_once("../inc/ldap.inc");
require_once("../inc/user_util.inc");
require_once("../inc/password_compat/password.inc");

check_get_args(array("id", "t", "h", "key"));

// login with email addr / passwd
//
function login_with_email($email_addr, $passwd, $next_url, $perm) {
    $user = BoincUser::lookup_email_addr($email_addr);
    if (!$user) {
        sleep(LOGIN_FAIL_SLEEP_SEC);
        page_head("No such account");
        echo "No account with email address <b>$email_addr</b> exists.
            Please go back and try again.
        ";
        page_tail();
        exit;
    }
    if (substr($user->authenticator, 0, 1) == 'x'){
        sleep(LOGIN_FAIL_SLEEP_SEC);
        error_page("This account has been administratively disabled.");
    }

    // allow authenticator as password
    //
    if ($passwd != $user->authenticator) {
        $passwd_hash = md5($passwd.$email_addr);

        if (!check_passwd_hash($user, $passwd_hash)) {
            sleep(LOGIN_FAIL_SLEEP_SEC);
            page_head("Password incorrect");
            echo "The password you entered is incorrect. Please go back and try again.\n";
            page_tail();
            exit;
        }
    }

    // Intercept next_url if consent has not yet been given
    //
    $next_url = intercept_login($user, $perm, $next_url);
    Header("Location: ".url_base()."$next_url");
}

// email link case
//
function login_via_link($id, $t, $h) {
    $user = BoincUser::lookup_id($id);
    if (!$user) {
        sleep(LOGIN_FAIL_SLEEP_SEC);
        error_page("Invalid user ID.
            Please make sure you visited the complete URL;
            it may have been split across lines by your email reader."
        );
    }
    $x = $id.$user->authenticator.$t;
    $x = md5($x);
    $x = substr($x, 0, 16);
    if ($x != $h) {
        sleep(LOGIN_FAIL_SLEEP_SEC);
        error_page("Invalid authenticator.
            Please make sure you visited the complete URL;
            it may have been split across lines by your email reader."
        );
    }
    if (time() - $t > 86400) {
        sleep(LOGIN_FAIL_SLEEP_SEC);
        error_page("Link has expired;
            go <a href=get_passwd.php>here</a> to
            get a new login link by email."
        );
    }

    // Intercept next_url if consent has not yet been given
    //
    $next_url = intercept_login($user, true, HOME_PAGE);
    Header("Location: ".url_base()."$next_url");
}

function login_with_auth($authenticator, $next_url, $perm) {
    $user = BoincUser::lookup_auth($authenticator);
    if (!$user) {
        sleep(LOGIN_FAIL_SLEEP_SEC);
        page_head("Login failed");
        echo "There is no account with that authenticator.
            Please <a href=get_passwd.php>try again</a>.
        ";
        page_tail();
    } else if (substr($user->authenticator, 0, 1) == 'x'){
        sleep(LOGIN_FAIL_SLEEP_SEC);
        error_page("This account has been administratively disabled.");
    } else {

        // Intercept next_url if consent has not yet been given
        //
        $next_url = intercept_login($user, $perm, $next_url);
        Header("Location: ".url_base()."$next_url");
    }
}

function login_with_ldap($uid, $passwd, $next_url, $perm) {
    list ($ldap_user, $error_msg) = ldap_auth($uid, $passwd);
    if ($error_msg) {
        sleep(LOGIN_FAIL_SLEEP_SEC);
        error_page($error_msg);
    }
    $x = ldap_email_string($uid);
    $user = BoincUser::lookup_email_addr($x);
    if (!$user) {
        // LDAP authentication succeeded but we don't have a user record.
        // Create one.
        //
        $user = make_user_ldap($x, $ldap_user->name);
    }
    if (!$user) {
        error_page("Couldn't create user");
    }
    // Intercept next_url if consent has not yet been given
    //
    $next_url = intercept_login($user, $perm, $next_url);
    Header("Location: ".url_base()."$next_url");
    return;
}

$id = get_int('id', true);
$t = get_int('t', true);
$h = get_str('h', true);
if ($id && $t && $h) {
    login_via_link($id, $t, $h);
    exit;
}

$next_url = post_str("next_url", true);
if ($next_url) {
    $next_url = urldecode($next_url);
    $next_url = sanitize_local_url($next_url);
}
if (!$next_url) {
    $next_url = HOME_PAGE;
}

$perm = false;
if (isset($_POST['stay_logged_in'])) {
    $perm = $_POST['stay_logged_in'];
}

// check for account key case.
// see if key is in URL; if not then check for POST data
//
$authenticator = get_str("key", true);
if (!$authenticator) {
    $authenticator = post_str("authenticator", true);
}
if ($authenticator) {
    login_with_auth($authenticator, $next_url, $perm);
    exit;
}

$email_addr = post_str("email_addr", true);
if ($email_addr) {
    $email_addr = strtolower(sanitize_tags($email_addr));
}
$passwd = post_str("passwd", true);
if ($email_addr && $passwd) {
    if (LDAP_HOST && !is_valid_email_syntax($email_addr)) {
        login_with_ldap($email_addr, $passwd, $next_url, $perm);
    } else {
        login_with_email($email_addr, $passwd, $next_url, $perm);
    }
    exit;
}

error_page("You must supply an email address and password");

?>
