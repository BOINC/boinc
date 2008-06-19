<?php

// The forum tables contain items that can become inconsistent
// due to bugs and DB gremlins.
// This script repairs some of them.

require_once("../inc/forum_db.inc");

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
        $num = BoincPost::count("user=$user->id");
        if ($num != $user->posts) {
            echo "user $user->id: $user->posts $num\n";
            $user->update("posts=$num");
        }
    }
}

function update_thread_replies() {
    $threads = BoincThread::enum();
    foreach ($threads as $t) {
        $n = BoincPost::count("thread=$t->id");
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
