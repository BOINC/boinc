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
include_once("../inc/recaptchalib.inc");
require_once("../inc/password_compat/password.inc");
require_once("../inc/consent.inc");

// Password hash has old format.
// Update user record with new format
//
function do_passwd_rehash($user, $passwd_hash) {
    $database_passwd_hash = password_hash($passwd_hash, PASSWORD_DEFAULT);
    $result = $user->update(" passwd_hash='$database_passwd_hash' ");
}

// return true if the passwd hash (old format)
// matches the user's passwd hash (possibly new format)
//
function check_passwd_hash($user, $passwd_hash) {
    if (password_verify($passwd_hash, $user->passwd_hash)) {
        // on valid login, rehash password to upgrade hash overtime
        // as the defaults change.
        //
        if (password_needs_rehash($user->passwd_hash, PASSWORD_DEFAULT)) {
            do_passwd_rehash($user, $passwd_hash);
        }
    } else if ($passwd_hash == $user->passwd_hash) {
        // user record has old format.  Change to new.
        //
        do_passwd_rehash($user, $passwd_hash);
    } else {
        return false;
    }
    return true;
}

function check_passwd_ui($user, $passwd) {
    $passwd_hash = md5($passwd.$user->email_addr);
    if(!check_passwd_hash($user, $passwd_hash)) {
        sleep(LOGIN_FAIL_SLEEP_SEC);
        page_head("Password incorrect");
        echo "The password you entered is incorrect. Please go back and try again.\n";
        page_tail();
        exit;
    }
}

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
    if (filter_var($name, FILTER_SANITIZE_SPECIAL_CHARS) !== $name) {
        $reason = tra("user name may not contain special characters");
        return false;
    }
    if (is_numeric($name)) {
        $reason = tra("user name may not be a number");
        return false;
    }
    return true;
}

function default_show_hosts() {
    $config = get_config();
    // If enable privacy by default is TRUE, then show_hosts' default
    // is FALSE.
    return parse_bool($config, "enable_privacy_by_default") ? 0 : 1;
}

// the following DB-escapes its args
//
function make_user(
    $email_addr, $name, $passwd_hash,
    $country=null, $postal_code=null, $project_prefs=null, $teamid=0
) {
    if (!is_valid_email_syntax($email_addr)) return null;
        // caller generally has already checked
    if (is_banned_email_addr($email_addr)) return null;

    $authenticator = random_string();
    $cross_project_id = random_string();
    $now = time();
    if (!is_valid_country($country)) return null;

    $email_addr = BoincDb::escape_string($email_addr);
    $name = sanitize_tags($name);
    $name = BoincDb::escape_string($name);
    $database_passwd_hash = password_hash($passwd_hash, PASSWORD_DEFAULT);

    $country = BoincDb::escape_string($country);
    $postal_code = sanitize_tags(BoincDb::escape_string($postal_code));

    $show_hosts = default_show_hosts();
    $uid = BoincUser::insert("(create_time, email_addr, name, authenticator, country, postal_code, total_credit, expavg_credit, expavg_time, project_prefs, teamid,  venue, send_email, show_hosts, posts, seti_id, seti_nresults, seti_last_result_time, seti_total_cpu, has_profile, cross_project_id, passwd_hash, email_validated, donated) values($now, '$email_addr', '$name', '$authenticator', '$country', '$postal_code', 0, 0, unix_timestamp(), '$project_prefs', $teamid, '', 1, $show_hosts, 0, 0, 0, 0, 0, 0, '$cross_project_id', '$database_passwd_hash', 0, 0)");

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

    $show_hosts = default_show_hosts();
    $uid = BoincUser::insert("(create_time, email_addr, name, authenticator, country, postal_code, total_credit, expavg_credit, expavg_time, project_prefs, teamid,  send_email, show_hosts, cross_project_id, passwd_hash) values($now, '$email_addr', '$name', '$authenticator', '', '', 0, 0, unix_timestamp(), '', 0, 1, $show_hosts, '$cross_project_id', '$passwd_hash')");

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
    $config = get_config();
    if (parse_bool($config, "disable_account_creation")
        || parse_bool($config, "no_web_account_creation")
    ) {
        error_page("Account creation is disabled");
    }

    if (recaptcha_private_key()) {
        if (!boinc_recaptcha_isValidated(recaptcha_private_key())) {
            show_error(
                tra("Your reCAPTCHA response was not correct. Please try again.")
            );
        }
    }

    // Check if consent to terms of use has been given.
    //
    $myconsent = FALSE;
    list($checkct, $ctid) = check_consent_type(CONSENT_TYPE_ENROLL);
    if ($checkct and check_termsofuse()) {
        $agree = post_str("agree_to_terms_of_use", true);
        if (!$agree) {
            error_page(tra("You have not agreed to our terms of use. Please agree to the terms of use by navigating back to the previous page, in order to create your account"));
        }
        $myconsent = TRUE;
    }

    // see whether the new account should be pre-enrolled in a team,
    // and initialized with its founder's project prefs
    //
    $teamid = post_int("teamid", true);
    if ($teamid) {
        $team = BoincTeam::lookup_id($teamid);
        $clone_user = BoincUser::lookup_id($team->userid);
        if (!$clone_user) {
            error_page("User $team->userid not found");
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
    if (UNIQUE_USER_NAME) {
        $u = BoincUser::lookup_name($new_name);
        if ($u) {
            page_head("That name is in use");
            echo "<p>The following user names are taken;
                please go back and use a different one.<p>
            ";
            $users = BoincUser::enum(
                sprintf("name like '%s%%'", $new_name)
            );
            foreach ($users as $u){
                echo "<p>$u->name\n";
            }
            page_tail();
            exit;
        }
    }

    $new_email_addr = strtolower(post_str("new_email_addr"));
    if (!is_valid_email_syntax($new_email_addr)) {
        show_error(tra("Invalid email address: please enter a valid address of the form name@xxx.yyy"));
    }
    if (!is_valid_email_sfs($new_email_addr)) {
        show_error("$new_email_addr was flagged by stopforumspam.com");
    }

    $user = BoincUser::lookup_email_addr($new_email_addr);
    if ($user) {
        show_error(tra("There's already an account with that email address."));
    }

    $tmpuser = BoincUser::lookup_prev_email_addr($new_email_addr);
    if ($tmpuser) {
        show_error(tra("There's already an account with that email address."));
    }

    $passwd = post_str("passwd");

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

    $country = "";
    if (USER_COUNTRY) {
        $country = post_str("country", true);
        if ($country && !is_valid_country($country)) {
            error_page("bad country");
        }
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

    if ($myconsent) {
        $rc = consent_to_a_policy($user, $ctid, 1, 0, 'Webreg');
        if (!$rc) {
            show_error(tra("database error, please contact site administrators"));
        }
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
    BoincToken::delete_for_user($user->id);
    $user->delete();
}
?>
