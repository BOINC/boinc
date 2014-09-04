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

// Script to help you purge spam profiles.
//
// To use: do the following query from mysql:
//
// select name, id  from user, profile where user.id=profile.userid and match(response1, response2) against ('Viagra');
// (replace "Viagra" with other keywords)
//
// Then copy the ids into the array below and run this script

error_reporting(E_ALL);
ini_set('display_errors', true);
ini_set('display_startup_errors', true);
$cli_only = true;
require_once("../inc/util_ops.inc");

db_init();

$ids = array(
        9031517,
        9031518,
);

function purge_user($id) {
    _mysql_query("delete from user where id=$id");
    _mysql_query("delete from profile where userid=$id");
    _mysql_query("delete from thread where owner=$id");
    _mysql_query("delete from post where user=$id");
}

function purge_users($ids) {
    foreach ($ids as $id) {
        purge_user($id);
    }
}

// purge_users($ids);

function profile_word($word) {
    $q = "select userid from profile where response1 like '%$word%'";
    echo "$q\n";
    $r = _mysql_query($q);
    while ($x = _mysql_fetch_object($r)) {
        purge_user($x->userid);
    }
}

//profile_word("viagra");

?>
