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

// The forum tables contain items that can become inconsistent
// due to bugs and DB gremlins.
// This script repairs some of them.

ini_set("memory_limit", "1024M");

$cli_only = true;
require_once("../inc/forum_db.inc");
require_once("../inc/util_ops.inc");

function update_thread_timestamps() {
    $threads = BoincThread::enum();
    foreach ($threads as $thread) {
        $q = "select max(timestamp) as foo from post where thread=$thread->id";
        $r2 = mysql_query($q);
        $m = mysql_fetch_object($r2);
        echo "id: $thread->id; min: $m->foo\n";
        mysql_free_result($r2);
        $n = $m->foo;
        if ($n) {
            $q = "update thread set timestamp=$n where id=$thread->id";
            mysql_query($q);
        }
    }
}

function update_user_posts() {
    $users = BoincUser::enum();
    foreach ($users as $user) {
        BoincForumPrefs::lookup($user);
        $num = BoincPost::count("user=$user->id");
        if ($num != $user->prefs->posts) {
            echo "user $user->id: $user->posts $num\n";
            $user->prefs->update("posts=$num");
        }
    }
}

function update_thread_replies() {
    $threads = BoincThread::enum();
    foreach ($threads as $t) {
        $n = BoincPost::count("thread=$t->id and hidden=0");
        $n--;
        if ($t->replies != $n) {
            $t->update("replies=$n");
            echo "updated thread $t->id; $t->replies -> $n\n";
        }
    }
}

//update_thread_replies(); exit();

echo "You must uncomment a function call in the script\n";
?>
