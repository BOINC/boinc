<?php
// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2018 University of California
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
// along with BOINC.  If not, see <https://www.gnu.org/licenses/>.

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");

// Constants for valid token types
define("TOKEN_TYPE_DELETE_ACCOUNT", "D");
define("TOKEN_TYPE_CHANGE_EMAIL", "E");
define("TOKEN_TYPE_LOGIN_INTERCEPT", "L");

// Constants for token durations
define("TOKEN_DURATION_TWO_HOURS", 7200);
define("TOKEN_DURATION_ONE_DAY", 86400);
define("TOKEN_DURATION_ONE_WEEK", 604800);

function create_token($userid, $type, $duration) {
    $token = random_string();
    $now = time();
    $expiration = $now + $duration;
    $type = BoincDb::escape_string($type);
    $ret = BoincToken::insert("(token,userid,type,create_time,expire_time) values ('$token', $userid, '$type', $now, $expiration)");
    if ( !$ret ) {
        return null;
    }
    return $token;
}

function delete_token($userid, $token, $type) {
    $token = BoincDb::escape_string($token);
    $type = BoincDb::escape_string($type);
    $result = BoincToken::delete_token("userid = $userid and token = '$token' and type = '$type'");
    return $result;
}

function is_valid_token($userid, $token, $type) {
    $boincToken = BoincToken::lookup_valid_token($userid, $token, $type);
    if ( $boincToken == null ) {
        return false;
    }
    return true;
}

?>
