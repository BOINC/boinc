#!/usr/bin/env php
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

// script to delete spammer accounts and profiles.
//
// delete_spammers.php --list filename
//   "filename" contains a list of user IDs, one per line.
//
// delete_spammers.php --auto
//   delete accounts that
//   - have no hosts
//   - have no message-board posts
//   - have a profile containing a link.
// NOTE: use this with caution.  See delete_auto() below.
// Run it in test mode first.

require_once("../inc/db.inc");
db_init();

function del($id) {
    echo "deleting user $id\n";
    $q = "delete from profile where userid=$id";
    mysql_query($q);
    $q = "delete from user where id=$id";
    mysql_query($q);
}

function delete_list($fname) {
    $f = fopen($fname, "r");
    if (!$f) die("no such file $fname\n");
    while ($s = fgets($f)) {
        $s = trim($s);
        if (!is_numeric($s)) die("bad ID $s\n");
        del($s);
    }
}

function has_link($x) {
    if (strstr($x, "[url")) return true;
    if (strstr($x, "http://")) return true;
    if (strstr($x, "https://")) return true;
    return false;
}

function delete_auto() {
    $profiles = BoincProfile::enum("");
    foreach ($profiles as $p) {
        if (has_link($p->response1) || has_link($p->response2)) {
            $user = BoincUser::lookup_id($p->userid);
            if (!$user) {
                echo "profile has missing user: %p->userid\n";
                continue;
            }

            // uncomment the following to delete only recent accounts
            //
            //if ($user->create_time < time() - 60*86400) continue;

            $n = BoincHost::count("userid=$p->userid");
            if ($n) continue;
            $n = BoincPost::count("user=$p->userid");
            if ($n) continue;

            // By default, show profile but don't delete anything
            // Change 0 to 1 if you want to actually delete
            //
            if (0) {
                del($user->id);
            } else {
                echo "------------\n$p->userid\n$p->response1\n$p->response2\n";
            }
        }
    }
}

for ($i=1; $i<$argc; $i++) {
    if ($argv[$i] == "--list") {
        delete_list($argv[++$i]);
    } else if ($argv[$i] == "--auto") {
        delete_auto();
    }
}

?>
