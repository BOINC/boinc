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

// script to delete spammer accounts, profiles, forum posts, and/or teams.
//
// delete_spammers.php [--days n] [--test] command
//
// commands:
//
// --profiles
//   delete accounts that
//   - have a profile containing a link.
//   - have no hosts
//   - have no message-board posts
//
// --forums
//   delete accounts that
//   - have no hosts
//   - have message-board posts
//   - don't belong to a team
//
// --list filename
//   "filename" contains a list of user IDs, one per line.
//
// --id_range N M
//   delete users with ID N to M inclusive
//
// --teams
//   delete teams (and their owners) where the team
//   - has 0 or 1 members
//   - has no total credit
//   - has description containing a link
//   - is not a BOINC-Wide team
//   and the owner
//   - has no posts
//   - has no hosts
//
// --user_url
//   delete accounts that
//   - have a nonempty URL
//   - have no hosts
//   - have no message-board posts
//
// options:
// --days N
//    Only delete accounts created in last N days
// --test
//    Show what accounts would be deleted, but don't delete them

error_reporting(E_ALL);
ini_set('display_errors', true);
ini_set('display_startup_errors', true);

require_once("../inc/db.inc");
require_once("../inc/profile.inc");
require_once("../inc/forum.inc");
db_init();

$days = 0;
$test = false;

// delete a spammer account, and everything associated with it
//
function do_delete_user($user) {
    global $test;
    $age = (time() - $user->create_time) / 86400;
    echo "deleting user\n";
    echo "   ID: $user->id\n";
    echo "   email: $user->email_addr\n";
    echo "   name: $user->name\n";
    echo "   URL: $user->url\n";
    echo "   age:$age days\n";
    if ($test) {
        echo "   (test mode - nothing deleted)\n";
        return;
    }
    delete_user($user);
}

function delete_list($fname) {
    $f = fopen($fname, "r");
    if (!$f) die("no such file $fname\n");
    while ($s = fgets($f)) {
        $s = trim($s);
        if (!is_numeric($s)) die("bad ID $s\n");
        $user = BoincUser::lookup_id((int)$s);
        if ($user) {
            do_delete_user($user);
        } else {
            echo "no user ID $s\n";
        }
    }
}

function has_link($x) {
    if (strstr($x, "[url")) return true;
    if (strstr($x, "http://")) return true;
    if (strstr($x, "https://")) return true;
    if (strstr($x, "www.")) return true;
    return false;
}

function delete_forums() {
    global $days;
    $prefs = BoincForumPrefs::enum("posts>0");
    foreach ($prefs as $p) {
        $user = BoincUser::lookup_id($p->userid);
        if (!$user) {
            echo "missing user $p->userid\n";
            continue;
        }
        if ($days) {
            if ($user->create_time < time() - $days*86400) continue;
        }
        if ($user->teamid) {
            continue;
        }
        $n = BoincHost::count("userid=$p->userid");
        if ($n) continue;
        do_delete_user($user);
    }
}

function delete_profiles() {
    global $test, $days;
    $profiles = BoincProfile::enum("");
    foreach ($profiles as $p) {
        if (has_link($p->response1) || has_link($p->response2)) {
            $user = BoincUser::lookup_id($p->userid);
            if (!$user) {
                echo "profile has missing user: $p->userid\n";
                continue;
            }

            if ($days) {
                if ($user->create_time < time() - $days*86400) continue;
            }

            $n = BoincHost::count("userid=$p->userid");
            if ($n) continue;
            $n = BoincPost::count("user=$p->userid");
            if ($n) continue;

            do_delete_user($user);
            if ($test) {
                echo "\n$p->userid\n$p->response1\n$p->response2\n";
            }
        }
    }
}

function delete_user_url() {
    global $test, $days;
    $users = BoincUser::enum("url <> ''");
    foreach ($users as $user) {
        if ($days) {
            if ($user->create_time < time() - $days*86400) continue;
        }
        $n = BoincHost::count("userid=$user->id");
        if ($n) continue;
        $n = BoincPost::count("user=$user->id");
        if ($n) continue;
        do_delete_user($user);
    }
}

function delete_banished() {
    global $days;
    $fps = BoincForumPrefs::enum("banished_until>0");
    foreach ($fps as $fp) {
        $user = BoincUser::lookup_id($fp->userid);
        if (!$user) continue;
        if ($user->create_time < time() - $days*86400) continue;
        do_delete_user($user);
    }
}

function delete_teams() {
    global $days, $test;
    $query = "nusers < 2 and seti_id=0 and total_credit=0";
    if ($days) {
        $x = time() - $days*86400;
        $query .= " and create_time > $x";
    }
    $teams = BoincTeam::enum($query);
    foreach ($teams as $team) {
        $n = team_count_members($team->id);
        if ($n > 1) continue;
        if (!has_link($team->description)) continue;
        $user = BoincUser::lookup_id($team->userid);
        if ($user) {
            $n = BoincPost::count("user=$user->id");
            if ($n) continue;
            $n = BoincHost::count("userid=$user->id");
            if ($n) continue;
        }
        if ($test) {
            echo "would delete team:\n";
            echo "   ID: $team->id\n";
            echo "   name: $team->name\n";
            echo "   description: $team->description\n";
        } else {
            $team->delete();
            echo "deleted team ID $team->id name $team->name\n";
            if ($user) do_delete_user($user);
        }
    }
}

echo "Starting: ".strftime('%Y-%m-%d %H:%M %Z')."\n";

for ($i=1; $i<$argc; $i++) {
    if ($argv[$i] == "--test") {
        $test = true;
    } else if ($argv[$i] == "--days") {
        $days = $argv[++$i];
    } else if ($argv[$i] == "--list") {
        delete_list($argv[++$i]);
    } else if ($argv[$i] == "--profiles") {
        delete_profiles();
    } else if ($argv[$i] == "--forums") {
        delete_forums();
    } else if ($argv[$i] == "--id_range") {
        $id1 = $argv[++$i];
        $id2 = $argv[++$i];
        if (!is_numeric($id1) || !is_numeric($id2)) {
            die ("bad args\n");
        }
        if ($id2 < $id1) {
            die("bad args\n");
        }
        for ($i=$id1; $i <= $id2; $i++) {
            $user = BoincUser::lookup_id($i);
            if ($user) {
                echo "deleting user $i\n";
                do_delete_user($user);
            }
        }
    } else if ($argv[$i] == "--banished") {
        delete_banished();
    } else if ($argv[$i] == "--teams") {
        delete_teams();
    } else if ($argv[$i] == "--user_url") {
        delete_user_url();
    } else {
        echo "usage: delete_spammers.php [--days N] [--test] [--list filename] [--profiles] [--forums] [--id_range N M] [--teams] [--user_url]\n";
        exit;
    }
}
echo "Finished: ".strftime('%Y-%m-%d %H:%M %Z')."\n";

?>
