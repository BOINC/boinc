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

// RPC handler for account creation

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/email.inc");
require_once("../inc/xml.inc");
require_once("../inc/user.inc");
require_once("../inc/team.inc");

xml_header();

$retval = db_init_xml();
if ($retval) xml_error($retval);

$config = get_config();
if (parse_bool($config, "disable_account_creation")) {
    xml_error(-208);
}

if(defined('INVITE_CODES')) {
    $invite_code = get_str("invite_code");
    if (!preg_match(INVITE_CODES, $invite_code)) {
        xml_error(-209);
    }
} 

$email_addr = get_str("email_addr");
$email_addr = strtolower($email_addr);
$passwd_hash = get_str("passwd_hash");
$user_name = get_str("user_name");
$team_name = get_str("team_name", true);

if (!is_valid_email_addr($email_addr)) {
    xml_error(-205);
}
if (is_banned_email_addr($email_addr)) {
    xml_error(-205);
}

if (strlen($passwd_hash) != 32) {
    xml_error(-1, "password hash length not 32");
}

$user = lookup_user_email_addr($email_addr);
if ($user) {
    if ($user->passwd_hash != $passwd_hash) {
        xml_error(-137);
    } else {
        $authenticator = $user->authenticator;
    }
} else {
    $user = make_user($email_addr, $user_name, $passwd_hash, 'International');
    if (!$user) {
        xml_error(-137);
    }
    
    if(defined('INVITE_CODES')) {
        error_log("Account for '$email_addr' created using invitation code '$invite_code'");
    }
}

if ($team_name) {
    $team_name = BoincDb::escape_string($team_name);
    $team = BoincTeam::lookup("name='$team_name'");
    if ($team && $team->joinable) {
        user_join_team($team, $user);
    }
}

echo " <account_out>\n";
echo "   <authenticator>$user->authenticator</authenticator>\n";
echo "</account_out>\n";

?>
