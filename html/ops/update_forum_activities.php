#!/usr/local/bin/php
<?php

require_once("../inc/forum.inc");

define('MAX_REWARD', 4096);
define('SCALAR', 0.9);
set_time_limit(0);

db_init();

$now = time();
$result = mysql_query("select * FROM thread");

while ($thread = mysql_fetch_object($result)) {
    $forum = getForum($thread->forum);
    $category = getCategory($forum->category);
    if ($category->is_helpdesk) {
        $diff = ($now - $thread->create_time)/86400;
        $activity = ($thread->sufferers+1)/$diff;
        echo "help $diff $activity\n";
    } else {
        $sql = "select * from post where thread=$thread->id";
        $result2 = mysql_query($sql);
        $activity = 0;
    
        while ($post = mysql_fetch_object($result2)) {
            $diff = $now - $post->timestamp;
            $diff /= 7*86400;
            $activity += pow(2, -$diff);
        }
        echo "forum $activity\n";
    }
    
    $sql = "update thread set activity=$activity where id=$thread->id";
    mysql_query($sql);
    
}



?>
