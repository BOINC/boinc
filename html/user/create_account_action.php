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
include_once("../inc/recaptchalib.php");

function show_error($str) {
    page_head(tra("Can't create account"));
    echo "$str<br>\n";
    echo BoincDb::error();
    echo "<p>".tra("Click your browser's <b>Back</b> button to try again.")."\n</p>\n";
    page_tail();
    exit();
}

$config = get_config();
if (parse_bool($config, "disable_account_creation")
    || parse_bool($config, "no_web_account_creation")
) {
    page_head(tra("Account creation is disabled"));
    echo "
        <h3>".tra("Account creation is disabled")."</h3>
        ".tra("Sorry, this project has disabled the creation of new accounts.
Please try again later.")."
    ";
    exit();
}

$privatekey = parse_config($config, "<recaptcha_private_key>");
  if ($privatekey) {
      $resp = recaptcha_check_answer($privatekey, $_SERVER["REMOTE_ADDR"],
          $_POST["recaptcha_challenge_field"], $_POST["recaptcha_response_field"]
      );
      if (!$resp->is_valid) {
          echo tra("Your reCAPTCHA response was not correct. Please try again.");
          return;
      }
  }

// see whether the new account should be pre-enrolled in a team,
// and initialized with its founder's project prefs
//
$teamid = post_int("teamid", true);
if ($teamid) {
    $team = lookup_team($teamid);
    $clone_user = lookup_user_id($team->userid);
    if (!$clone_user) {
        error_page("User $userid not found");
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
if (!is_valid_user_name($new_name, $reason)) {
    show_error($reason);
}

$new_email_addr = strtolower(post_str("new_email_addr"));
if (!is_valid_email_addr($new_email_addr)) {
    show_error(tra("Invalid email address: you must enter a valid address of the form name@domain"));
}
$user = lookup_user_email_addr($new_email_addr);
if ($user) {
    show_error(tra("There's already an account with that email address."));
}

$passwd = post_str("passwd");
$passwd2 = post_str("passwd2");
if ($passwd != $passwd2) {
    show_error(tra("New passwords are different"));
}

$min_passwd_length = parse_config($config, "<min_passwd_length>");
if (!$min_passwd_length) $min_passwd_length = 6;

if (!is_ascii($passwd)) {
    show_error(tra("Passwords may only include ASCII characters."));
}

if (strlen($passwd)<$min_passwd_length) {
    show_error(
        tra("New password is too short: minimum password length is %1 characters.", $min_passwd_length)
    );
}

$passwd_hash = md5($passwd.$new_email_addr);

$country = post_str("country");
if ($country == "") {
    $country = "International";
}
if (!is_valid_country($country)) {
    error_page("bad country");
}

$postal_code = sanitize_tags(post_str("postal_code", true));

$user = make_user(
    $new_email_addr, $new_name, $passwd_hash,
    $country, $postal_code, $project_prefs, $teamid
);
if (!$user) {
    show_error(tra("Couldn't create account"));
}

if(defined('INVITE_CODES')) {
    error_log("Account '$new_email_addr' created using invitation code '$invite_code'");
}

// In success case, redirect to a fixed page so that user can
// return to it without getting "Repost form data" stuff

$next_url = post_str('next_url', true);
$next_url = sanitize_local_url($next_url);
if ($next_url) {
    Header("Location: ".URL_BASE."$next_url");
} else {
    Header("Location: ".URL_BASE."home.php");
    send_cookie('init', "1", true);
    send_cookie('via_web', "1", true);
}
send_cookie('auth', $user->authenticator, true);

?>
