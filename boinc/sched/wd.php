#! /usr/local/bin/php
<?php

// General-purpose watchdog script.
// Run this from crontab.
// We use the mod time of a file "watchdog_exec_time"
// to keep track of the last time we ran.

// BOINC uses a number of "error log files".
// If any error log file has been updated since the last time we ran,
// sound the alarm.

// TODO: to detect file system full errors,
// have this program attempt to create/read a file.

function sound_alarm($x) {
    //echo "alarm: $x\n";
    mail("davea@ssl.berkeley.edu", "BOINC problem", $x);
    mail("eheien@ssl.berkeley.edu", "BOINC problem", $x);
}

function check_log_file($file, $last_time) {
    $t = filemtime($file);
    if ($t == false) {
        sound_alarm("log file ".$file." missing");
    } else if ($t > $last_time) {
        $lines = file($file);
        $last_line = $lines[count($lines)-1];
        sound_alarm($last_line);
    }
}

    $last_time = filemtime("watchdog_exec_time");
    if (!$last_time) {
        sound_alarm("Couldn't find watchdog_exec_time");
    }
    touch("watchdog_exec_time");

    check_log_file("error_log", $last_time);
?>
