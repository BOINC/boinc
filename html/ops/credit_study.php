<?php

// Use this script to find an optimal value for fp_benchmark_weight.
// This evaluates the variation for weights 0, .1, ... 1.

require_once("../inc/db.inc");

db_init();

function mean($x) {
    $sum = 0;
    $n = count($x);
    for ($i=0; $i<$n; $i++) {
        $sum += $x[$i];
    }
    return $sum/$n;
}

function spread($x) {
    $sum = 0;
    $n = count($x);
    $m = mean($x);
    for ($i=0; $i<$n; $i++) {
        $d = $x[$i] - $m;
        $sum += $d*$d;
    }
    return sqrt($sum)/$n;
}

function cc($x, $fpw) {
    $cps = $x->p_fpops*$fpw + $x->p_iops*(1-$fpw);
    $cps /= 1e9;
    $cps /= 864;
    $cc = $x->cpu_time * $cps;
    //if ($x->duration_correction_factor) {
    //    $cc /= $x->duration_correction_factor;
    //}
    return $cc;
}

// $x is a vector of objects
//
function handle_wu($x) {
    $cc = array();
    for ($i=0; $i<=12; $i++) {
        array_push($cc, array());
    }
    for ($j=0; $j<count($x); $j++) {
        $y = $x[$j];
        for ($i=0; $i<=12; $i++) {
            $r = $i/10.;
            array_push($cc[$i], cc($y, $r));
        }
    }
    $res = array();
    for ($i=0; $i<=12; $i++) {
        $c = $cc[$i];
        $m = mean($c);
        $s = spread($c);
        $rs = $s/$m;
        array_push($res, $rs);
    }
    return $res;
}

function get_data() {
    $sum = array();
    for ($i=0; $i<=12; $i++) {
        array_push($sum, 0);
    }
    $r1 = mysql_query(
        "select id from workunit where canonical_resultid>0 limit 10000"
    );
    $n = 0;
    while ($wu = mysql_fetch_object($r1)) {
        $x = array();
        $r2 = mysql_query("select * from result where workunitid=$wu->id");
        $found_zero = false;
        while ($result = mysql_fetch_object($r2)) {
            if ($result->granted_credit==0) continue;
            if ($result->claimed_credit==0) $found_zero = true;
            $host = lookup_host($result->hostid);
            $y->claimed_credit = $result->claimed_credit;
            $y->granted_credit = $result->granted_credit;
            $y->cpu_time = $result->cpu_time;
            $y->p_fpops = $host->p_fpops;
            $y->p_iops = $host->p_iops;
            $y->duration_correction_factor = $host->duration_correction_factor;
            $y->id = $result->id;
            array_push($x, $y);
        }
        if (count($x)==0) continue;
        if ($found_zero) continue;
        $res = handle_wu($x);
        for ($i=0; $i<=12; $i++) {
            $sum[$i] += $res[$i];
        }
        $n++;
    }
    for ($i=0; $i<=12; $i++) {
        $r = $sum[$i]/$n;
        echo "$i $r\n";
    }
}


get_data();

?>
