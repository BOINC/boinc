#!/usr/local/bin/php
<?php

define('NUM_USERS', 200);
define('NUM_TEAMS', 24);
define('BASE_UID', 1000001);
define('BASE_TID', 300);

// Users
$fd = fopen("test_user_flat", "w");

for ($i = 0; $i < NUM_USERS; $i++) {
    $line = "" . (BASE_UID + $i)."|"."1234"."|"."user$i@$i.com|Johhny #$i|0|".(2451405.039016203+$i)."|2|3|4|5|6|7|1|United States|".(94704 + $i)."|1|2|3|".(BASE_TID + ($i%NUM_TEAMS))."|password|www.jb.org|0|0|\n";
    fwrite($fd, $line);
    shell_exec("cp 1000001.jpg ".(BASE_UID+$i).".jpg");
    shell_exec("cp 1000001.jpg sm_".(BASE_UID+$i).".jpg");
}

fclose($fd);

// Every other user will have a profile.
$fd = fopen("test_feedback_flat", "w");

for ($i = 0; $i < (NUM_USERS / 2); $i++) {
    $line = "".$i."|".(BASE_UID + ($i*2))."|0|0.00|0.00|JBK||||||||".(BASE_UID + $i*2).".jpg|Hello, I'm user ID# ".(BASE_UID + $i*2)."|I have no opinions|1|1|1|1|something|\n";
    fwrite($fd, $line);
}

fclose($fd);

// Teams
$fd = fopen("test_teams_flat", "w");

for ($i = 0; $i < NUM_TEAMS ; $i++) {

    $line = "".(BASE_TID + ($i%NUM_TEAMS))."|".(BASE_UID + $i)."|Team #$i|Name LC|A test team.|jb$i.teams.org|12|23|12345.11|".($i*10)."|1|<h1> Team $i! </h1>|\n";
    fwrite($fd, $line);
}

fclose($fd);
?>