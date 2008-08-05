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

// RPC handler for account lookup

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/email.inc");
require_once("../inc/xml.inc");

xml_header();
$retval = db_init_xml();
if ($retval) xml_error($retval);

$email_addr = get_str("email_addr");
$passwd_hash = get_str("passwd_hash", true);

$email_addr = BoincDb::escape_string($email_addr);
$user = BoincUser::lookup("email_addr='$email_addr'");
if (!$user) {
    xml_error(-136);
}

if (!$passwd_hash) {
    echo "<account_out>
    <success/>
</account_out>
";
    exit();
}

$auth_hash = md5($user->authenticator.$user->email_addr);

// if no password set, set password to account key
//
if (!strlen($user->passwd_hash)) {
    $user->passwd_hash = $auth_hash;
    $user->update("passwd_hash='$user->passwd_hash'");
}

// if the given password hash matches (auth+email), accept it
//
if ($user->passwd_hash == $passwd_hash || $auth_hash == $passwd_hash) {
    echo "<account_out>\n";
    echo "<authenticator>$user->authenticator</authenticator>\n";
    echo "</account_out>\n";
} else {
    xml_error(-206);
}

?>

