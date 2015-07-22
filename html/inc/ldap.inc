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

// LDAP authentication.
// returns (user, error_msg) where user is an object with
// $user->name
// $user->email_addr
//
// your project.inc must define LDAP_HOST and LDAP_BASE_DN
//
function ldap_auth($uid, $passwd) {
    $ad = @ldap_connect(LDAP_HOST);
    if (!$ad) {
        return array(null, "Can't connect to LDAP server");
    }
    ldap_set_option($ad, LDAP_OPT_PROTOCOL_VERSION, 3);
    ldap_set_option($ad, LDAP_OPT_REFERRALS, 0);
    $rn = "uid=$uid,".LDAP_BASE_DN;
    $r = @ldap_bind($ad, $rn, $passwd);
    if (!$r) {
        return array(null, "Invalid credentials");
    }
    $x = ldap_search($ad, $rn, "(objectclass=*)");
    $x = ldap_get_entries($ad, $x);

    $user = new StdClass;
    $x = $x[0];
    $name = $x["cn"];
    $user->name = $name[0];
    $email = $x["mail"];
    $user->email_addr = $email[0];
    return array($user, null);
}

// for LDAP-authenticated users, we store LDAP:uid in user.email_addr
//
function ldap_email_string($uid) {
    return "LDAP:$uid";
}

function is_ldap_email($x) {
    return (substr($x, 0, 5) == "LDAP:");
}

function ldap_email_to_uid($x) {
    return substr($x, 5);
}

function example_usage() {
    list($user, $error_msg) = ldap_auth("davea", "xxx");
    if ($error_msg) {
        echo "error: $error_msg\n";
    } else {
        echo "name: $user->name; email: $user->email_addr\n";
    }
}

?>
