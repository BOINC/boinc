<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2013 University of California
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

// PHP interfaces to some of BOINC's Web RPCs

// my PHP currently doesn't support file_get_contents(https://...)
// so do it with Curl
//
function fetch_url($url) {
    if (0) {
        return file_get_contents($url);
    } else {
        $ch = curl_init();
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
        curl_setopt($ch, CURLOPT_URL, $url);
        curl_setopt($ch, CURLOPT_HEADER, 0);

        $x = curl_exec($ch);
        curl_close($ch);
        //echo "curl return: $x\n";
        return $x;
    }
}

function lookup_account(
    $project_url,
    $email_addr,
    $passwd_hash
) {
    $url = $project_url."/lookup_account.php?email_addr=".urlencode($email_addr)."&passwd_hash=$passwd_hash";
    $reply = fetch_url($url);
    if (!$reply) {
        return array(null, -1, "HTTP error to $url");
    }
    $r = @simplexml_load_string($reply);
    if (!$r) {
        return array(null, -1, "Can't parse reply XML:\n$reply");
    }
    $auth = (string)$r->authenticator;
    if ($auth) {
        return array($auth, 0, null);
    } else {
        return array(null, (int)$r->error_num, (string)$r->error_msg);
    }
}

// return (authenticator, errmsg)
//
function create_account(
    $project_url,
    $email_addr,
    $passwd_hash,
    $user_name
) {
    $url = $project_url."/create_account.php?email_addr=".urlencode($email_addr)."&passwd_hash=$passwd_hash&user_name=".urlencode($user_name);

    $reply = fetch_url($url);
    if (!$reply) return array(null, -1, "HTTP error to $url");

    $r = @simplexml_load_string($reply);
    if (!$r) {
        // old server code returns PHP warnings, which break XML.
        // do ad-hoc parsing instead
        //
        $auth = parse_element($reply, "<authenticator>");
        if ($auth) {
            return array($auth, 0, null);
        }
        $error_num = parse_element($reply, "<error_num>");
        $error_msg = parse_element($reply, "<error_msg>");
        if ($error_num) {
            return array(null, $error_num, $error_msg);
        }
        return array(null, -1, "Can't parse reply XML:\n$reply");
    }
    $auth = (string)$r->authenticator;
    if ($auth) {
        return array($auth, 0, null);
    } else {
        return array(null, (int)$r->error_num, (string)$r->error_msg);
    }
}

function example1() {
    $x = create_account(
        "http://isaac.ssl.berkeley.edu/test/",
        "john@a.b.c",
        "12345678901234567890123456789012",
        "John Doe"
    );
    print_r($x);
}

function example2() {
    $x = lookup_account(
        "http://isaac.ssl.berkeley.edu/test/",
        "davea@ssl.berkeley.edu",
        "xxx"
    );
    print_r($x);
}

//example2();
?>
