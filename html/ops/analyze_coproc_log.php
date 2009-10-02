<?php

// Parse a log file of completed GPU jobs and generate some stats.
// Log format:
//
// time validated
// time received
// result ID
// host ID
// user ID
// team ID
// claimed credit
// granted credit
// coprocessor description
//
// This was developed for SETI@home but might be useful for other projects;
// you'll need to add code to your validator to generate the log file

// outputs:
// - user leaderboard
// - host leaderboard
// - team leaderboard
// - daily total
// - breakdown by desc (number, credit)

ini_set ("memory_limit", "1G");
set_time_limit(0);

$cli_only = true;
require_once("../inc/util_ops.inc");

$users = array();
$hosts = array();
$teams = array();
$days = array();
$descs = array();

function add_to_array(&$array, $id, $credit) {
    if (array_key_exists($id, $array)) {
        $array[$id]->nresults++;
        $array[$id]->credit += $credit;
    } else {
        $x = null;
        $x->nresults = 1;
        $x->credit = $credit;
        $array[$id] = $x;
    }
}

function compare($x, $y) {
    if ($x->credit > $y->credit) return -1;
    if ($x->credit < $y->credit) return 1;
    return 0;
}

function write_array($array, $file, $n) {
    if ($n) {
        $array = array_slice($array, 0, $n, true);
    }
    $f = fopen($file, "w");
    fwrite($f, serialize($array));
    fclose($f);
}

$f = fopen("cuda_result_log", "r");
$i = 0;
while (!feof($f)) {
    $str = fgets($f);
    list($val_time, $rec_time, $resultid, $hostid, $userid, $teamid, $claimed, $granted, $desc) = sscanf($str, "%d %d %d %d %d %d %f %f %s");
    add_to_array($hosts, $hostid, $granted);
    add_to_array($users, $userid, $granted);
    add_to_array($teams, $teamid, $granted);
    add_to_array($descs, $desc, $granted);
    if ($rec_time) {
        $day = date("Y n j", $rec_time);
        add_to_array($days, $day, $granted);
    }
    $i++;
    if ($i % 10000 == 0) echo "$i\n";
}

uasort($hosts, "compare");
uasort($users, "compare");
uasort($teams, "compare");
uasort($descs, "compare");
write_array($hosts, "cuda_hosts.dat", 100);
write_array($users, "cuda_users.dat", 100);
write_array($teams, "cuda_teams.dat", 100);
write_array($descs, "cuda_models.dat", 0);
write_array($days, "cuda_days.dat", 0);

?>
