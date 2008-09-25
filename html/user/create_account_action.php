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

include_once("../inc/boinc_db.inc");
include_once("../inc/util.inc");
include_once("../inc/email.inc");
include_once("../inc/user.inc");

function show_error($str) {
    page_head("Can't create account");
    echo "$str<br>\n";
    echo BoincDb::error();
    echo "<p>Click your browser's <b>Back</b> button to try again.\n<p>\n";
    page_tail();
    exit();
}

$config = get_config();
if (parse_bool($config, "disable_account_creation")) {
    page_head("Account creation is disabled");
    echo "
        <h3>Account creation is disabled</h3>
        Sorry, this project has disabled the creation of new accounts.
        Please try again later.
    ";
    exit();
}

// see whether the new account should be pre-enrolled in a team,
// and initialized with its founder's project prefs
//
$teamid = post_int("teamid", true);
if ($teamid) {
    $team = lookup_team($teamid);
    $clone_user = lookup_user_id($team->userid);
    if (!$clone_user) {
        echo "User $userid not found";
        exit();
    }
    $project_prefs = $clone_user->project_prefs;
} else {
    $teamid = 0;
    $project_prefs = "";
}

if(defined('INVITE_CODES')) {
    $invite_code = post_str("invite_code");
    if (strlen($invite_code)==0) {
        show_error(tra("You must supply an invitation code to create an account."));
    }
    if (!preg_match(INVITE_CODES, $invite_code)) {
        show_error(tra("The invitation code you gave is not valid."));
    }
} 

$new_name = post_str("new_name");
if (strlen($new_name)==0) {
    show_error("You must supply a name for your account");
}
if ($new_name != strip_tags($new_name)) {
    show_error("HTML tags not allowed in name");
}

$new_email_addr = post_str("new_email_addr");
$new_email_addr = strtolower($new_email_addr);
if (!is_valid_email_addr($new_email_addr)) {
    show_error("Invalid email address:
        you must enter a valid address of the form
        name@domain"
    );
}
$user = lookup_user_email_addr($new_email_addr);
if ($user) {
    show_error("There's already an account with that email address.");
}

$passwd = post_str("passwd");
$passwd2 = post_str("passwd2");
if ($passwd != $passwd2) {
    show_error("New passwords are different");
}

$min_passwd_length = parse_config($config, "<min_passwd_length>");
if (!$min_passwd_length) $min_passwd_length = 6;

if (!is_ascii($passwd)) {
    show_error("Passwords may only include ASCII characters.");
}

if (strlen($passwd)<$min_passwd_length) {
    show_error(
        "New password is too short:
        minimum password length is $min_passwd_length characters."
    );
}

$passwd_hash = md5($passwd.$new_email_addr);

$country = post_str("country");
if ($country == "") {
    $country = "International";
}
if (!is_valid_country($country)) {
    echo "bad country";
    exit();
}

$postal_code = post_str("postal_code", true);

$user = make_user(
    $new_email_addr, $new_name, $passwd_hash,
    $country, $postal_code, $project_prefs, $teamid
);
if (!$user) {
    show_error("Couldn't create account");
}

if(defined('INVITE_CODES')) {
    error_log("Account '$new_email_addr' created using invitation code '$invite_code'");
}

// In success case, redirect to a fixed page so that user can
// return to it without getting "Repost form data" stuff

$next_url = post_str('next_url', true);
if ($next_url) {
    Header("Location: $next_url");
} else {
    Header("Location: home.php");
    send_cookie('init', "1", true);
    send_cookie('via_web', "1", true);
}
send_cookie('auth', $user->authenticator, true);

?>
