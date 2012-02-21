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

// Use this script to find an optimal value for fp_benchmark_weight,
// i.e. the weight which minimizes the variation between
// claimed credit and granted credit
// over the validated WUs currently in the database.
//
// Evaluates the variation for weights 0, .1, ... 1.
//

$cli_only = true;
require_once("../inc/util_ops.inc");

db_init();

function mean($x) {
    $sum = 0;
    $n = count($x);
    for ($i=0; $i<$n; $i++) {
        $sum += $x[$i];
    }
    return $sum/$n;
}

function normalized_variance($x) {
    $sum = 0;
    $n = count($x);
    $m = mean($x);
    for ($i=0; $i<$n; $i++) {
        $d = $x[$i] - $m;
        $sum += $d*$d;
        //echo "$x[$i] ";
    }
    //echo "\n";
    $nv = sqrt($sum)/($n*$m);
    //echo "nresults $n mean $m sum $sum nv $nv\n";
    return $nv;
}

// returns the claimed credit for a given result/host and FP weight
//
function cc($x, $fpw) {
    $cps = $x->p_fpops*$fpw + $x->p_iops*(1-$fpw);
    $cps /= 1e9;
    $cps /= 864;
    $cc = $x->cpu_time * $cps;
    return $cc;
}

// $x is an array of result/host objects;
// return the variance among claimed credits given an FP weight
//
function fpw_var($results, $fpw) {
    $cc = array();
    foreach ($results as $r) {
        $cc[] = cc($r, $fpw);
    }
    return normalized_variance($cc);
}

// scan WUs for which credit has been granted,
// and for which there at least 2 valid results.
// For each of these, compute the variance among claimed credits
// given various FP weights (0, .1, ... 1).
// Maintain the sum of these in an array
//
function get_data() {
    $nwus = 4000;

    $sum = array();
    for ($i=0; $i<=10; $i++) {
        $sum[] = 0;
    }
    $r1 = mysql_query(
        "select id from workunit where canonical_resultid>0 limit $nwus"
    );
    $n = 0;
    while ($wu = mysql_fetch_object($r1)) {
        $results = array();
        $r2 = mysql_query("select * from result where workunitid=$wu->id");
        $found_zero = false;
        while ($result = mysql_fetch_object($r2)) {
            if ($result->granted_credit==0) continue;   // skip invalid
            $host = lookup_host($result->hostid);
            $r = null;
            $r->cpu_time = $result->cpu_time;
            $r->p_fpops = $host->p_fpops;
            $r->p_iops = $host->p_iops;
            $results[] = $r;
        }
        //echo "Wu $wu->id -------------\n";
        if (count($results)<2) continue;
        for ($i=0; $i<=10; $i++) {
            $fpw = $i/10.;
            $sum[$i] += fpw_var($results, $fpw);
        }
        $n++;
    }
    echo "This script recommends value for <fp_benchmark_weight> in config.xml.
It does this by finding the value that minimizes the variance
among claimed credit for workunits currently in your database.
It examines at most $nwus WUs (edit the script to change this).

Number of workunits analyzed: $n

";
    for ($i=0; $i<=10; $i++) {
        $fpw = $i/10.;
        $r = $sum[$i]/$n;
        echo "FP weight $fpw: variance is $r\n";
        if ($i == 0) {
            $best = $r;
            $fbest = $fpw;
        } else {
            if ($r < $best) {
                $best = $r;
                $fbest = $fpw;
            }
        }
    }

    echo "
Recommended value: $fbest
";
}

get_data();

?>
