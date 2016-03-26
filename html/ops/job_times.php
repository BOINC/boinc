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

// tool for estimating FLOPS per job for a given app

error_reporting(E_ALL);
ini_set('display_errors', true);
ini_set('display_startup_errors', true);

require_once("../inc/db.inc");
require_once("../inc/util_ops.inc");

db_init();

$hist = array();
$quantum = 1e10;
$mean = 0;
$count = 0;
$varsum = 0;

function handle_result($result) {
    global $hist;
    global $quantum;
    global $mean;
    global $count;
    global $varsum;

    $flops = $result->elapsed_time * $result->flops_estimate;
    //printf("%e<br>\n", $flops);
    $n = (int) ($flops/$quantum);
    if (!array_key_exists($n, $hist)) {
        $hist[$n] = 0;
    }
    $hist[$n]++;
    $count++;
    $delta = $flops - $mean;
    $mean += $delta/$count;
    $varsum += $delta*($flops-$mean);
}

function show_stats() {
    global $mean;
    global $count;
    global $varsum;
    global $sum;

    $stdev = sqrt($varsum/($count-1));
    printf("mean: %e<br>stdev: %e<br>samples: %d", $mean, $stdev, $count);
}

function show_stats_hist() {    //deprecated
    global $hist;
    global $quantum;

    $sum = 0;
    $n = 0;
    foreach ($hist as $i=>$v) {
        $sum  += $quantum*$i*$v;
        $n += $v;
    }
    $mean = $sum/$n;
    echo "mean: ";
    printf("%e", $mean);

    $sum = 0;
    foreach ($hist as $i=>$v) {
        $d = ($mean - $quantum*$i);
        $sum  += $d*$d*$v;
    }
    $stdev = sqrt($sum/$n);
    echo "<br>stdev: ";
    printf("%e", $stdev);
}

function show_as_xml() {
    global $hist;
    global $quantum;
    echo "<pre>";
    foreach ($hist as $i=>$v) {
        echo "&lt;bin>
    &lt;value>";
        printf("%e", $quantum*$i);
        echo "&lt;/value>
    &lt;count>$v&lt;/count>
&lt;/bin>
";
    }
    echo "</pre>";
}

function show_as_table() {
    global $quantum;
    global $hist;

    echo "<table width=600 border=0 cellborder=0 cellpadding=0>";
    $keys = array_keys($hist);
    $start = reset($keys);
    $end = end($keys);

    $max = $hist[$start];
    foreach ($hist as $v) {
        if ($v > $max) $max = $v;
    }

    for ($i=$start; $i<=$end; $i++) {
        if (array_key_exists($i, $hist)) {
            $w = 600*$hist[$i]/$max;
        } else {
            $w = 0;
        }
        $f = $i*$quantum;
        echo "<tr><td><font size=-2>";
        printf("%e ", $f);
        echo "</font></td><td><img vspace=0 src=https://boinc.berkeley.edu/colors/000000.gif height=10 width=$w></td></tr>\n";
    }
    echo "</table>";
}

function version_select($appid) {
    $x = "<select name=app_version_id>
        <option value=0>All
    ";
    $avs = BoincAppVersion::enum("appid=$appid");
    $avs = current_versions($avs);
    foreach ($avs as $av) {
        $platform = BoincPlatform::lookup_id($av->platformid);
        $n = $platform->name;
        if (strlen($av->plan_class)) {
            $n .= " ($av->plan_class)";
        }
        $x .= "<option value=$av->id> $n\n";
    }
    $x .= "</select>\n";
    return $x;
}

function analyze($appid, $app_version_id, $nresults) {
    global $hist;

    $clause = $app_version_id?" and app_version_id = $app_version_id ":"";

    $query = "select id, server_state, outcome, elapsed_time, flops_estimate from result where server_state=5 and appid=$appid and outcome=1 and validate_state=1 $clause order by id desc limit $nresults";
    $r = _mysql_query($query);

    $n = 0;
    while ($result = _mysql_fetch_object($r)) {
        handle_result($result);
        $n++;
    }

    if (!$n) {
        echo "No done results for that app";
        exit;
    }

    ksort($hist);
    show_stats($hist);
    echo "<hr>\n";
    show_as_table();
    echo "<hr>\n";
    show_as_xml();
}

function show_app_select() {
    admin_page_head("Show FLOPS distribution");
    echo "Select an application:
        <form action=job_times.php>
    ";
    $apps = BoincApp::enum("deprecated=0");

    foreach($apps as $app) {
        echo "<br><input type=radio name=appid value=$app->id>
            $app->user_friendly_name
        ";
    }
    echo "<br><br><input class=\"btn btn-default\" type=submit value=OK>";
    admin_page_tail();
}

function show_form($appid) {
    admin_page_head("Show FLOPS distribution");
    echo "
        <form method=get action=job_times.php>
        <input type=hidden name=appid value=$appid>
    ";
    start_table();
    row2("App version:", version_select($appid));
    row2("Resolution:<br><p class=\"text-muted\">(if you see only one bar, use a smaller value)</p>", "<input name=quantum value=1e12>");
    row2("Sample size (# of jobs):", "<input name=nresults value=1000>");
    row2("", "<input class=\"btn btn-default\" type=submit name=submit value=OK>");
    end_table();
    echo "
        </form>
    ";
    admin_page_tail();
}

if (get_str('submit', true)=='OK') {
    set_time_limit(0);
    $appid = get_int('appid');
    $app_version_id = get_int('app_version_id');
    $quantum = (double)(get_str('quantum'));
    $nresults = get_int('nresults');
    analyze($appid, $app_version_id, $nresults);
} else {
    $appid = get_int('appid', true);
    if ($appid) {
        show_form($appid);
    } else {
        show_app_select();
    }
}

?>
