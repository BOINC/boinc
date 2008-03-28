#!/usr/bin/env php
<?php

require_once("../inc/forum_db.inc");

define('MAX_REWARD', 4096);
define('SCALAR', 0.9);
set_time_limit(0);

echo date(DATE_RFC822), ": Starting\n";

$now = time();
$threads = BoincThread::enum();
foreach ($threads as $thread) {
    $forum = BoincForum::lookup_id($thread->forum);
    $category = BoincCategory::lookup_id($forum->category);
    if ($category->is_helpdesk) {
        $diff = ($now - $thread->create_time)/86400;
        $activity = ($thread->sufferers+1)/$diff;
        echo "help $diff $activity\n";
    } else {
        $posts = BoincPost::enum("thread=$thread->id");
        $activity = 0;
    
        foreach ($posts as $post) {
            $diff = $now - $post->timestamp;
            $diff /= 7*86400;
            $activity += pow(2, -$diff);
        }
        echo "forum $activity\n";
    }
    $thread->update("activity=$activity");
    
}

echo date(DATE_RFC822), ": Finished\n";

?>
