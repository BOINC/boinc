#!/usr/local/bin/php
<?php

require_once("../html_user/db.inc");
require_once("../html_user/util.inc");

define('MAX_REWARD', 4096);
define('SCALAR', 0.9);

db_init();

$currentTime = time();
$result = mysql_query("SELECT * FROM thread");

while ($thread = mysql_fetch_assoc($result)) {
    $sql = "SELECT * FROM forum WHERE id = " . $thread['forum'];
    $result3 = mysql_query($sql);
    $forum = NULL;
    
    if ($result3) {
        $forum = mysql_fetch_assoc($result3);
    }
                       
    // Helpdesk activiy is scaled linearly by the number of days since the last post.
    if ($forum != NULL && $forum['is_helpdesk']) {
        $timeDiff = $currentTime - $thread['timestamp'];
        $dayDiff = floor($timeDiff / 86400);  // 86400 = # of seconds in a day;
        $points = $thread['sufferers'];
        if ($dayDiff > 0) {
            $points = $points * ($dayDiff * SCALAR);
        }
    // Other forum activity is scaled exponentially based on the recency of the posts.
    } else {
        $sql = "SELECT * FROM post WHERE thread = " . $thread['id'];
        $result2 = mysql_query($sql);
        $points = 0;
    
        while ($post = mysql_fetch_assoc($result2)) {
            $timeDiff = $currentTime - $post['timestamp'];
            $dayDiff = floor($timeDiff / 86400);
        
            $points += (MAX_REWARD / pow(2, $dayDiff));
        }
    }
    
    $sql = "UPDATE thread SET activity = " . ceil($points) . " WHERE id = " . $thread['id'];
    mysql_query($sql);
    
}



?>