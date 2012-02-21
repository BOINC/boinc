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
require_once("../inc/xml.inc");
require_once("../inc/team.inc");
require_once("../inc/email.inc");

// do a very cursory check that the given text is valid;
// for now, just make sure it has the given start and end tags,
// and at least one \n in the middle.
// Ideally, we'd like to check that it's valid XML
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

$auth = get_str("account_key");
$user = lookup_user_auth($auth);
if (!$user) {
    xml_error(-136);
}

$name = BoincDb::escape_string(get_str("name", true));
$country = get_str("country", true);
if ($country && !is_valid_country($country)) {
    xml_error(-1, "invalid country");
}
$country = BoincDb::escape_string($country);
$postal_code = BoincDb::escape_string(get_str("postal_code", true));
$global_prefs = BoincDb::escape_string(get_str("global_prefs", true));
$project_prefs = BoincDb::escape_string(get_str("project_prefs", true));

// Do processing on project prefs so that we don't overwrite project-specific
// settings if AMS has no idea about them

if (stripos($project_prefs, "<project_specific>") === false) {
    // AMS request does not contain project specific prefs, preserve original
    $orig_project_specific = stristr($user->project_prefs, "<project_specific>");
    $orig_project_specific = substr($orig_project_specific, 0, stripos($orig_project_specific, "</project_specific>") + 19)."\n";
    $project_prefs = str_ireplace("<project_preferences>", "<project_preferences>\n".$orig_project_specific, $project_prefs);
}

$url = BoincDb::escape_string(get_str("url", true));
$send_email = BoincDb::escape_string(get_str("send_email", true));
$show_hosts = BoincDb::escape_string(get_str("show_hosts", true));
$teamid = get_int("teamid", true);
$venue = BoincDb::escape_string(get_str("venue", true));
$email_addr = get_str("email_addr", true);
if ($email_addr) {
    if (!is_valid_email_addr($email_addr)) {
        xml_error(-205, "Invalid email address");
    }
    if (is_banned_email_addr($email_addr)) {
        xml_error(-205, "Invalid email address");
    }
    $email_addr = strtolower(BoincDb::escape_string($email_addr));
}
$password_hash = BoincDb::escape_string(get_str("password_hash", true));

$query = "";
if ($name) {
    $query .= " name='$name', ";
}
if ($country) {
    $query .= " country='$country', ";
}
if ($postal_code) {
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
        xml_error(-112, "Invalid project preferences: $x");
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
        $team = lookup_team($teamid);
        if ($team && $team->joinable) {
            user_join_team($team, $user);
        }
    }
}

if ($venue) {
    $query .= " venue='$venue', ";
}
if ($email_addr && $email_addr!=$user->email_addr) {
    $old_email_addr = $user->email_addr;
    $query .= " email_addr='$email_addr', ";
}
if ($password_hash) {
    $query .= " passwd_hash='$password_hash', ";
}

if (strlen($query)) {
    // the seti_id=seti_id is to make the query valid,
    // since $query ends with a comma at this point
    //
    $query = "$query seti_id=seti_id";
    $result = $user->update($query);
    if ($result) {
        success("");
    } else {
        xml_error(-1, "database error: ".BoincDb::error());
    }
} else {
    success("");
}

?>
