#!/usr/local/bin/php
<?php

require_once("../html_user/db.inc");
require_once("../html_user/util.inc");

define('MAX_REWARD', 4096);

db_init();

$currentTime = time();
$result = mysql_query("SELECT * FROM thread");

while ($thread = mysql_fetch_assoc($result)) {
    $sql = "SELECT * FROM post WHERE thread = " . $thread['id'];
    $result2 = mysql_query($sql);
    $points = 0;
    
    while ($post = mysql_fetch_assoc($result2)) {
        $timeDiff = $currentTime - $post['timestamp'];
        $dayDiff = floor($timeDiff / 86400);  // 86400 = # of seconds in a day;
        
        $points += (MAX_REWARD / pow(2, $dayDiff));
    }
    
    $sql = "UPDATE thread SET activity = " . ceil($points) . " WHERE id = " . $thread['id'];
    mysql_query($sql);
}



?>