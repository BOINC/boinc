#!/usr/bin/env php

<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2024 University of California
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
// -----------------------------------------------

// delete_spammers.php [--test] [--min_days n] [--max_days n] command
//
// script to delete spammer accounts, profiles, forum posts, and/or teams.
// The various options delete different categories of spammers.
//
// USE WITH CARE.  You don't want to delete legit accounts.
// Run with --test and examine the results first.

// In this context, 'spam' is text that advertises something
// (e.g. viagra, porn sites etc.) and contains hyperlinks.
// Spammers can put such text in
// - profiles
// - team URLs or descriptions
// - account URLs
// - forum posts
//
// All the above can legitimately contain links,
// so to decide what's spam we look at attributes of the user:
// - whether they've attached a client to the project
//      Most spammers haven't, so they have no hosts.
//      Note: legit users might create an account just to participate
//      in the project forums (e.g. Science United users).
//      So we generally need to check for forum activity.
// - whether they've been granted any credit
//      This is more stringent.
//      But we need to take into account that it may take a month or two
//      to get credit because of validation
//
// When we identify spam, we delete everything associated with that user:
// - profile
// - forum stuff: post, thread, subscriptions etc.
// - private messages
// - friend links
// - badges

// options:
// --min_days N
//    Only delete accounts created at least N days ago
// --max_days N
//    Only delete accounts created at most N days ago
// --test
//    Show what accounts would be deleted, but don't delete them
//
// commands:
//
// --profiles
//   delete accounts that
//   - have a profile containing a link.
//   - have no hosts
//   - have no message-board posts
//
// --user_url
//   delete accounts that
//   - have a nonempty URL
//   - have no hosts
//   - have no message-board posts
//   Use for spammers who create accounts with commercial URLs.
//
// --user_null
//   delete accounts that
//   - have no hosts
//   - have no message-board posts
//   - don't belong to a team
//   Spammers may create accounts and attempt to create a profile but fail.
//   This cleans up those accounts.
//
// --forums
//   delete accounts that
//   - have no hosts
//   - have message-board posts containing links or URLs
//   - don't belong to a team (avoid deleting BOINC-wide team owners)
//   Use this for spammers who create accounts and post spam.
//   Don't use this for non-computing projects (like BOINC message boards).
//   In fact, don't use this in general:
//   it will delete users who join to participate in forums.
//
// --profiles_strict
//  delete accounts that have a profile and no message-board posts.
//  For the BOINC message boards.
//
// --list filename
//   "filename" contains a list of user IDs, one per line.
//
// --id_range N M
//   delete users with ID N to M inclusive
// --id N
//   delete user N
//
// --teams
//   delete teams (and their owners and members) where the team
//   - has no total credit
//   - has description containing a link, or a URL
//   - is not a BOINC-Wide team
//   and the owner and members
//   - have no posts
//   - have no hosts
//
// --all (recommended for BOINC projects)
//   Does: --teams --user_url --profiles
//   Doesn't do --forums (see comments above).
//   Can use moderators for that.

error_reporting(E_ALL);
ini_set('display_errors', true);
ini_set('display_startup_errors', true);
ini_set('memory_limit', '4G');

require_once("../inc/db.inc");
require_once("../inc/profile.inc");
require_once("../inc/forum.inc");
require_once("../inc/user_util.inc");
db_init();

$min_days = 0;
$max_days = 0;
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
        $n = count(BoincHost::enum("userid=$user->id"));
        $m = count(BoincPost::enum("user=$user->id"));
        echo "   $n hosts\n";
        echo "   $m posts\n";
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
    if (strstr($x, "<a ")) return true;
    if (strstr($x, "[url")) return true;
    if (strstr($x, "http://")) return true;
    if (strstr($x, "https://")) return true;
    if (strstr($x, "www.")) return true;
    return false;
}

// delete users with
// - no hosts
// - no team
// - posts contain links and/or URLs
//
function delete_forums() {
    global $min_days, $max_days;

    // if they've posted, they'll have forum prefs.
    // This is faster than enumerating all users
    //
    $prefs = BoincForumPrefs::enum("posts>0");
    $count = 0;
    foreach ($prefs as $p) {
        $user = BoincUser::lookup_id($p->userid);
        if (!$user) {
            echo "missing user $p->userid\n";
            continue;
        }
        if ($min_days) {
            if ($user->create_time > time() - $min_days*86400) continue;
        }
        if ($max_days) {
            if ($user->create_time < time() - $max_days*86400) continue;
        }
        if ($user->teamid) {
            continue;
        }
        $h = BoincHost::count("userid=$p->userid");
        if ($h) continue;

        // posts with at least 2 URLs
        $n = BoincPost::count("user=$user->id and (content like '%http%http%')");
        if (!$n) continue;
        do_delete_user($user);
        $count++;
    }
    echo "delete_forums(): deleted $count users\n";
}

function delete_profiles() {
    global $test, $min_days, $max_days;
    $profiles = BoincProfile::enum("");
    $n = 0;
    foreach ($profiles as $p) {
        if (has_link($p->response1) || has_link($p->response2)) {
            $user = BoincUser::lookup_id($p->userid);
            if (!$user) {
                echo "profile has missing user: $p->userid\n";
                continue;
            }

            if ($min_days) {
                if ($user->create_time > time() - $min_days*86400) continue;
            }
            if ($max_days) {
                if ($user->create_time < time() - $max_days*86400) continue;
            }

            $m = BoincHost::count("userid=$p->userid");
            if ($m) continue;
            $m = BoincPost::count("user=$p->userid");
            if ($m) continue;

            do_delete_user($user);
            if ($test) {
                echo "\n$p->userid\n$p->response1\n$p->response2\n";
            }
            $n++;
        }
    }
    echo "delete_profiles(): deleted $n users\n";
}

function delete_profiles_strict() {
    global $test;
    $profiles = BoincProfile::enum("");
    foreach ($profiles as $p) {
        $user = BoincUser::lookup_id($p->userid);
        if (!$user) {
            echo "profile has missing user: $p->userid\n";
            continue;
        }
        $n = BoincPost::count("user=$p->userid");
        if ($n) continue;
        do_delete_user($user);
        if ($test) {
            echo "\n$p->userid\n$p->response1\n$p->response2\n";
        }
    }
}

function delete_users($no_hosts, $no_posts, $no_teams, $have_url) {
    global $test, $min_days, $max_days;
    $db = BoincDb::get();
    $query = "select a.* from user a ";
    if ($no_hosts) {
        $query .= " left join host c on c.userid=a.id ";
    }
    if ($no_posts) {
        $query .= " left join post b on a.id=b.user ";
    }
    if ($no_teams) {
        $query .= " left join team d on a.id=d.userid ";
    }
    $query .= " where true ";
    if ($no_hosts) {
        $query .= " and c.userid is null ";
    }
    if ($no_posts) {
        $query .= " and b.user is null ";
    }
    if ($no_teams) {
        $query .= " and d.userid is null ";
    }
    if ($min_days) {
        $t = time() - $min_days*86400;
        $query .= " and a.create_time < $t ";
    }
    if ($max_days) {
        $t = time() - $max_days*86400;
        $query .= " and a.create_time > $t ";
    }

    $result = $db->do_query($query);
    $n = 0;
    while ($u = $result->fetch_object()) {
        $user = BoincUser::lookup_id($u->id);
        if (!$user) {
            continue;
        }
        if ($have_url) {
            if (!strlen($user->url)) continue;
        }
        do_delete_user($user);
        $n++;
    }
    echo "deleted $n users\n";
}

function delete_banished() {
    global $min_days, $max_days;
    $fps = BoincForumPrefs::enum("banished_until>0");
    foreach ($fps as $fp) {
        $user = BoincUser::lookup_id($fp->userid);
        if (!$user) continue;
        if ($user->create_time > time() - $min_days*86400) continue;
        if ($user->create_time < time() - $max_days*86400) continue;
        do_delete_user($user);
    }
}

function delete_teams() {
    global $min_days, $max_days, $test;
    $query = "nusers < 2 and seti_id=0 and total_credit=0";
    if ($min_days) {
        $x = time() - $min_days*86400;
        $query .= " and create_time < $x";
    }
    if ($max_days) {
        $x = time() - $max_days*86400;
        $query .= " and create_time > $x";
    }
    $teams = BoincTeam::enum($query);
    $count = 0;
    foreach ($teams as $team) {
        $founder = null;
        if ($team->userid) {
            $founder = BoincUser::lookup_id($team->userid);
        }

        // delete teams with no founder
        if (!$founder) {
            delete_team($team, []);
            $count++;
            continue;
        }

        $n = team_count_members($team->id);
        if ($n > 1) continue;
        if (!has_link($team->description) && !$team->url) continue;

        // get list of team members
        //
        $users = BoincUser::enum("teamid = $team->id");

        // add team founder if not member
        //
        if ($founder->teamid != $team->id) {
            $users[] = $founder;
        }

        // if any of these users has signs of life, skip team
        //
        $life = false;
        foreach ($users as $user) {
            if ($user->seti_nresults) {
                // for SETI@home
                $life = true;
                break;
            }
            $n = BoincPost::count("user=$user->id");
            if ($n) {
                $life = true;
                break;
            }
            $n = BoincHost::count("userid=$user->id");
            if ($n) {
                $life = true;
                break;
            }
        }
        if ($life) {
            continue;
        }

        $count++;
        delete_team($team, $users);
    }
    echo "deleted $count teams\n";
}

function delete_team($team, $users) {
    global $test;
    if ($test) {
        echo "would delete team:\n";
        echo "   ID: $team->id\n";
        echo "   name: $team->name\n";
        echo "   description: $team->description\n";
        echo "   URL: $team->url\n";
        foreach ($users as $user) {
            echo "would delete user $user->id: $user->email_addr:\n";
        }
    } else {
        $team->delete();
        echo "deleted team ID $team->id name $team->name\n";
        foreach ($users as $user) {
            do_delete_user($user);
        }
    }
}

function delete_user_id($id) {
    $user = BoincUser::lookup_id($id);
    if ($user) {
        echo "deleting user $id\n";
        do_delete_user($user);
    } else {
        echo "no such user\n";
    }
}

function delete_user_id_range($id1, $id2) {
    for ($i=$id1; $i <= $id2; $i++) {
        $user = BoincUser::lookup_id($i);
        if ($user) {
            echo "deleting user $i\n";
            do_delete_user($user);
        }
    }
}

// this is for cleaning up BOINC-wide teams
//
function delete_team_id_range($id1, $id2) {
    for ($i=$id1; $i <= $id2; $i++) {
        echo "deleting team $i\n";
        $team = BoincTeam::lookup_id($i);
        if ($team) {
            $team->delete();
            $user = BoincUser::lookup_id($team->userid);
            if ($user) $user->delete();
        }
    }
}

echo "Starting: ".date(DATE_RFC2822)."\n";

// get settings first
//
for ($i=1; $i<$argc; $i++) {
    if ($argv[$i] == "--test") {
        $test = true;
    } else if ($argv[$i] == "--min_days") {
        $min_days = $argv[++$i];
    } else if ($argv[$i] == "--max_days") {
        $max_days = $argv[++$i];
    } else if ($argv[$i] == "--days") {     // deprecated
        $max_days = $argv[++$i];
    }
}

// then do actions
//
for ($i=1; $i<$argc; $i++) {
    if ($argv[$i] == "--list") {
        delete_list($argv[++$i]);
    } else if ($argv[$i] == "--profiles") {
        delete_profiles();
    } else if ($argv[$i] == "--profiles_strict") {
        delete_profiles_strict();
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
        delete_user_id_range($id1, $id2);
    } else if ($argv[$i] == "--id") {
        $id = $argv[++$i];
        if (!is_numeric($id)) {
            die ("bad arg\n");
        }
        delete_user_id($id);
    } else if ($argv[$i] == "--team_id_range") {
        $id1 = $argv[++$i];
        $id2 = $argv[++$i];
        if (!is_numeric($id1) || !is_numeric($id2)) {
            die ("bad args\n");
        }
        if ($id2 < $id1) {
            die("bad args\n");
        }
        delete_team_id_range($id1, $id2);
    } else if ($argv[$i] == "--banished") {
        delete_banished();
    } else if ($argv[$i] == "--teams") {
        delete_teams();
    } else if ($argv[$i] == "--user_url") {
        delete_users(true, true, false, true);
    } else if ($argv[$i] == "--user_null") {
        delete_users(true, true, true, false);
    } else if ($argv[$i] == "--all") {
        delete_profiles();
        delete_teams();
        delete_users(true, true, false, true);
    }
}
echo "Finished: ".date(DATE_RFC2822)."\n";

?>
