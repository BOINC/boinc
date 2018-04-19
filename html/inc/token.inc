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

// Constants for token durations
define("TOKEN_DURATION_ONE_DAY", "86400");

function create_confirm_delete_account_token($user) {
    $token = random_string();
    $now = time();
    $ret = BoincToken::insert("(token,userid,type,create_time,expire_time) values ('$token', $user->id, '".TOKEN_TYPE_DELETE_ACCOUNT."', $now, $now+".TOKEN_DURATION_ONE_DAY.")");
    if ( !$ret ) {
        return null;
    }
    return $token;
}

function is_valid_delete_account_token($userid, $token) {
    $boincToken = BoincToken::lookup_valid_token($userid, $token, TOKEN_TYPE_DELETE_ACCOUNT);
    if ( $boincToken == null ) {
        return false;
    }
    return true;
}

?>
