<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2017 University of California
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

// functions for creating and deleting users


include_once("../inc/boinc_db.inc");
include_once("../inc/util.inc");
include_once("../inc/email.inc");
include_once("../inc/recaptchalib.php");

function is_banned_email_addr($email_addr) {
    global $banned_email_domains;
    if (isset($banned_email_domains)) {
        foreach($banned_email_domains as $d) {
            $x = strstr($email_addr, $d);
            if ($x == $d) return true;
        }
    }
    return false;
}

function is_valid_user_name($name, &$reason) {
    if (trim($name) !== $name) {
        $reason = tra("user name cannot have leading or trailing white space");
        return false;
    }
    if (strlen($name) == 0) {
        $reason = tra("user name must be nonempty");
        return false;
    }
    if (sanitize_tags($name) !== $name) {
        $reason = tra("user name may not contain HTML tags");
        return false;
    }
    return true;
}

// the following DB-escapes its args
//
function make_user(
    $email_addr, $name, $passwd_hash,
    $country=null, $postal_code=null, $project_prefs=null, $teamid=0
) {
    if (!is_valid_email_addr($email_addr)) return null;
    if (is_banned_email_addr($email_addr)) return null;

    $authenticator = random_string();
    $cross_project_id = random_string();
    $now = time();
    if (!is_valid_country($country)) return null;

    $email_addr = BoincDb::escape_string($email_addr);
    $name = sanitize_tags($name);
    $name = BoincDb::escape_string($name);
    $passwd_hash = BoincDb::escape_string($passwd_hash);

    $country = BoincDb::escape_string($country);
    $postal_code = sanitize_tags(BoincDb::escape_string($postal_code));

    $uid = BoincUser::insert("(create_time, email_addr, name, authenticator, country, postal_code, total_credit, expavg_credit, expavg_time, project_prefs, teamid,  venue, send_email, show_hosts, posts, seti_id, seti_nresults, seti_last_result_time, seti_total_cpu, has_profile, cross_project_id, passwd_hash, email_validated, donated) values($now, '$email_addr', '$name', '$authenticator', '$country', '$postal_code', 0, 0, unix_timestamp(), '$project_prefs', $teamid, '', 1, 1, 0, 0, 0, 0, 0, 0, '$cross_project_id', '$passwd_hash', 0, 0)");

    if (!$uid) {
        return null;
    }
    $user = BoincUser::lookup_id($uid);
    if (defined('RECORD_USER_IP')) {
        $ip = $_SERVER['REMOTE_ADDR'];
        $ip = BoincDb::escape_string($ip);
        $user->update("venue='$ip'");
    }
    return $user;
}

function make_user_ldap($email_addr, $name) {
    $email_addr = BoincDb::escape_string($email_addr);
    $name = sanitize_tags($name);
    $name = BoincDb::escape_string($name);
    $authenticator = random_string();
    $cross_project_id = random_string();
    $passwd_hash = random_string();
    $now = time();
    $uid = BoincUser::insert("(create_time, email_addr, name, authenticator, country, postal_code, total_credit, expavg_credit, expavg_time, project_prefs, teamid,  send_email, show_hosts, cross_project_id, passwd_hash) values($now, '$email_addr', '$name', '$authenticator', '', '', 0, 0, unix_timestamp(), '', 0, 1, 1, '$cross_project_id', '$passwd_hash')");

    if ($uid) {
        return BoincUser::lookup_id($uid);
    } else {
        return null;
    }
}

function show_error($str) {
    page_head(tra("Can't create account"));
    echo "$str<br>\n";
    echo BoincDb::error();
    echo "<p>".tra("Click your browser's <b>Back</b> button to try again.")."\n</p>\n";
    page_tail();
    exit();
}

// validate POST args and make user.
// If error show error page and exit.
// Else return user object
//
function validate_post_make_user() {
    global $recaptcha_private_key;
    $config = get_config();
    if (parse_bool($config, "disable_account_creation")
        || parse_bool($config, "no_web_account_creation")
    ) {
        error_page("Account creation is disabled");
    }

    if ($recaptcha_private_key) {
        if (!boinc_recaptcha_isValidated($recaptcha_private_key)) {
            show_error(
                tra("Your reCAPTCHA response was not correct. Please try again.")
            );
        }
    }

    // see whether the new account should be pre-enrolled in a team,
    // and initialized with its founder's project prefs
    //
    $teamid = post_int("teamid", true);
    if ($teamid) {
        $team = BoincTeam::lookup_id($teamid);
        $clone_user = BoincUser::lookup_id($team->userid);
        if (!$clone_user) {
            error_page("User $userid not found");
        }
        $project_prefs = $clone_user->project_prefs;
    } else {
        $teamid = 0;
        $project_prefs = "";
    }

    if (defined('INVITE_CODES')) {
        $invite_code = post_str("invite_code");
        if (strlen($invite_code) == 0) {
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
    $user = BoincUser::lookup_email_addr($new_email_addr);
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
        $country = "None";
    }
    if (!is_valid_country($country)) {
        error_page("bad country");
    }

    if (POSTAL_CODE) {
        $postal_code = sanitize_tags(post_str("postal_code", true));
    } else {
        $postal_code = '';
    }

    $user = make_user(
        $new_email_addr, $new_name, $passwd_hash,
        $country, $postal_code, $project_prefs, $teamid
    );
    if (!$user) {
        show_error(
            tra("Couldn't create account").": ".BoincDb::error()
        );
    }

    if (defined('INVITE_CODES')) {
        error_log("Account '$new_email_addr' created using invitation code '$invite_code'");
    }
    return $user;
}

// delete a user and all associated records except
// result
// host
// batch
// team
// user_submit
// user_submit_app
// credited_job
// donation_paypal
// sent_email
//
function delete_user($user) {
    delete_profile($user);
    forum_delete_user($user);
        // deletes post, thread, subscription, forum_preferences, forum_logging
    BoincPrivateMessage::delete_aux("userid=$user->id or senderid=$user->id");
    BoincNotify::delete_aux("userid=$user->id");
    BoincCreditUser::delete_user($user);
    BoincBadgeUser::delete("user_id=$user->id");
    BoincFriend::delete_aux("user_src=$user->id or user_dest=$user->id");
    $user->delete();
}
?>
