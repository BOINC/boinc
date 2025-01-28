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

// Handler for an RPC (used by account managers)
// to change attributes of an account.
// This probably shouldn't exist.

require_once("../inc/boinc_db.inc");
require_once("../inc/xml.inc");
require_once("../inc/team.inc");
require_once("../inc/email.inc");
require_once("../inc/password_compat/password.inc");
require_once("../inc/user_util.inc");

// do a very cursory check that the given text is valid;
// for now, just make sure it has the given start and end tags,
// and at least one \n in the middle.
// Ideally, we'd like to check that it's valid XML
// WHY NOT USE simplexml_load_string()??
//
function bad_xml($text, $start, $end) {
    $text = trim($text);
    if (strstr($text, $start) != $text) {
        return "No start tag";
    }
    if (strstr($text, $end) != $end) {
        return "No end tag";
    }
    if (!strstr($text, "\n")) {
        return "No CR";
    }
    return "";
}

function success($x) {
    echo "<am_set_info_reply>
    <success/>
    $x
</am_set_info_reply>
";
}

xml_header();
$retval = db_init_xml();
if ($retval) xml_error($retval);

$config = get_config();

$auth = post_str("account_key", true);
if ($auth) {
    $name = post_str("name", true);
    $country = post_str("country", true);
    $postal_code = post_str("postal_code", true);
    $global_prefs = post_str("global_prefs", true);
    $project_prefs = post_str("project_prefs", true);
    $url = post_str("url", true);
    $send_email = post_str("send_email", true);
    $show_hosts = post_str("show_hosts", true);
    $teamid = post_int("teamid", true);
    $venue = post_str("venue", true);
    $email_addr = post_str("email_addr", true);
    $password_hash = post_str("password_hash", true);
    $consent_name = post_str("consent_name", true);
    $consent_flag = post_str("consent_flag", true);
    $consent_not_required = post_str("consent_not_required", true);
    $consent_source = post_str("consent_source", true);
} else {
    $auth = get_str("account_key");
    $name = get_str("name", true);
    $country = get_str("country", true);
    $postal_code = get_str("postal_code", true);
    $global_prefs = get_str("global_prefs", true);
    $project_prefs = get_str("project_prefs", true);
    $url = get_str("url", true);
    $send_email = get_str("send_email", true);
    $show_hosts = get_str("show_hosts", true);
    $teamid = get_int("teamid", true);
    $venue = get_str("venue", true);
    $email_addr = get_str("email_addr", true);
    $password_hash = get_str("password_hash", true);
    $consent_name = get_str("consent_name", true);
    $consent_flag = get_str("consent_flag", true);
    $consent_not_required = get_str("consent_not_required", true);
    $consent_source = get_str("consent_source", true);
}

$user = BoincUser::lookup_auth($auth);
if (!$user) {
    xml_error(ERR_DB_NOT_FOUND);
}

$name = BoincDb::escape_string($name);
if (!USER_COUNTRY) {
    $country = "";
}
if (!is_valid_country($country)) {
    xml_error(-1, "invalid country");
}
$country = BoincDb::escape_string($country);
$postal_code = BoincDb::escape_string($postal_code);
$global_prefs = BoincDb::escape_string($global_prefs);
$project_prefs = BoincDb::escape_string($project_prefs);

// Do processing on project prefs so that we don't overwrite project-specific
// settings if AMS has no idea about them

if (stripos($project_prefs, "<project_specific>") === false) {
    // AMS request does not contain project specific prefs, preserve original
    $orig_project_specific = stristr($user->project_prefs, "<project_specific>");
    $orig_project_specific = substr($orig_project_specific, 0, stripos($orig_project_specific, "</project_specific>") + 19)."\n";
    $project_prefs = str_ireplace("<project_preferences>", "<project_preferences>\n".$orig_project_specific, $project_prefs);
}

if (!USER_URL) {
    $url = "";
}
$url = BoincDb::escape_string($url);
$send_email = BoincDb::escape_string($send_email);
$show_hosts = BoincDb::escape_string($show_hosts);
$venue = BoincDb::escape_string($venue);
$send_changed_email_to_user = false;
if ($email_addr) {
    $email_addr = strtolower($email_addr);
}
if ($email_addr && $email_addr != $user->email_addr) {
    // is addr already in use?
    //
    $tmpuser = BoincUser::lookup_email_addr($email_addr);
    if ($tmpuser) {
        xml_error(ERR_DB_NOT_UNIQUE, "There's already an account with that email address.");
    }

    // did another account use this addr previously?
    // WHY CHECK THIS???
    //
    $tmpuser = BoincUser::lookup_prev_email_addr($email_addr);
    if ($tmpuser) {
        xml_error(ERR_DB_NOT_UNIQUE, "Email address is already in use");
    }

    // see if email addr was changed too recently
    //
    if ($user->email_addr_change_time + 604800 > time()) {
        xml_error(ERR_BAD_EMAIL_ADDR, "Email address was changed within the past 7 days, please look for an email to $user->previous_email_addr if this email change is incorrect.");
    }

    if (!is_valid_email_syntax($email_addr)) {
        xml_error(ERR_BAD_EMAIL_ADDR, "Invalid email address");
    }
    if (!is_valid_email_sfs($email_addr)) {
        xml_error(ERR_BAD_EMAIL_ADDR, "email address flagged by stopforumspam.com");
    }
    if (is_banned_email_addr($email_addr)) {
        xml_error(ERR_BAD_EMAIL_ADDR, "Invalid email address");
    }
    $email_addr = strtolower(BoincDb::escape_string($email_addr));
}
$password_hash = BoincDb::escape_string($password_hash);

$query = "";
if ($name) {
    $query .= " name='$name', ";
}
if ($country) {
    $query .= " country='$country', ";
}
if (POSTAL_CODE && $postal_code) {
    $query .= " postal_code='$postal_code', ";
}
if ($global_prefs) {
    $global_prefs = str_replace("\\r\\n", "\n", $global_prefs);
    $x = bad_xml($global_prefs, "<global_preferences>", "</global_preferences>");
    if ($x) {
        error("Invalid global preferences: $x");
    }
    $query .= " global_prefs='$global_prefs', ";
}
if ($project_prefs) {
    $project_prefs = str_replace("\\r\\n", "\n", $project_prefs);
    $x = bad_xml($project_prefs, "<project_preferences>", "</project_preferences>");
    if ($x) {
        xml_error(ERR_XML_PARSE, "Invalid project preferences: $x");
    }
    $query .= " project_prefs='$project_prefs', ";
}
if ($url) {
    $query .= " url='$url', ";
}
if ($send_email != null) {
    $query .= " send_email='$send_email', ";
}
if ($show_hosts != null) {
    $query .= " show_hosts='$show_hosts', ";
}

if (!is_null($teamid)) {
    if ($teamid==0) {
        user_quit_team($user);
    } else {
        $team = BoincTeam::lookup_id_nocache($teamid);
        if ($team && $team->joinable) {
            user_join_team($team, $user);
        }
    }
}

if ($venue) {
    $query .= " venue='$venue', ";
}

// Check to see if email_addr is different then what user->email-addr has
// If it is different, then update the database and trigger sending an
// email to the user that the email address has been changed.
//
if ($email_addr && $email_addr != $user->email_addr) {
    $user->previous_email_addr = $user->email_addr;
    $user->email_addr_change_time = time();
    $user->email_addr = $email_addr;
    $query .= " email_addr='$user->email_addr', ";
    if ($user->previous_email_addr) {
        $query .= " previous_email_addr='$user->previous_email_addr', email_addr_change_time=$user->email_addr_change_time, ";
        $send_changed_email_to_user = true;
    }
}
if ($password_hash) {
    $database_passwd_hash = password_hash($password_hash, PASSWORD_DEFAULT);
    $query .= " passwd_hash='$database_passwd_hash', ";
}

if (strlen($query)) {
    // the seti_id=seti_id is to make the query valid,
    // since $query ends with a comma at this point
    //
    $query = "$query seti_id=seti_id";
    $result = $user->update($query);
    if ($result) {
        if ($send_changed_email_to_user) {
            send_changed_email($user);
        }
    } else {
        xml_error(-1, "database error: ".BoincDb::error());
    }
}

// If all four consent parameters must be given to add to the consent
// table. If one or more of these consent_xyz parameters are NOT
// present, the RPC will still return 'success', even though the
// consent table is not updated.
if ( (isset($consent_name) and isset($consent_flag) and isset($consent_not_required) and isset($consent_source)) ) {
    list($checkct, $ctid) = check_consent_type($consent_name);
    if ($checkct) {

        // Check to see if latest consent of this name is already
        // given.
        $cr= BoincConsent::lookup("userid={$user->id} AND consent_type_id='${ctid}' ORDER BY consent_time DESC LIMIT 1");
        if ( (($cr) and ($cr->consent_flag!=$consent_flag)) or
             (!$cr) ) {

            $rc = consent_to_a_policy($user, $ctid, $consent_flag, $consent_not_required, $consent_source, time());
            if (!$rc) {
                xml_error(-1, "database error: ".BoincDb::error());
            }
        }

    }
}


// The equivalent of a 'return 0'.
success("");

?>
