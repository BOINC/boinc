#! /usr/bin/env php
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

// General-purpose watchdog script.
// Run this from crontab.
// We use the mod time of a file "watchdog_exec_time"
// to keep track of the last time we ran.

// BOINC uses a number of "error log files".
// If any error log file has been updated since the last time we ran,
// sound the alarm.

// TODO: to detect file system full errors,
// have this program attempt to create/read a file.

$cli_only = true;
require_once("../inc/util_ops.inc");

function sound_alarm($x) {
    //echo "alarm: $x\n";
    mail(SYS_ADMIN_EMAIL, "BOINC problem", $x);
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
