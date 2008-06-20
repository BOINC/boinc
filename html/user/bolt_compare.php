<?php

require_once("../inc/bolt_db.inc");

// a "snapshot" is a condensed representation of the results
// for a particular select/xset pair.
// Namely, it's an array whose elements contain
//  bolt_user: the user
//  xset_result: the user's first completion of the xset
//  select_finished: the user's last completion of the select before this

function write_snapshot($course_id, $select_name, $xset_name, $start) {
    $xrs = BoltXsetResult::enum(
        "course_id=$course_id and name='$xset_name' and create_time >= $start"
    );
    $sfs = BoltSelectFinished::enum(
        "course_id=$course_id and name='$select_name' and end_time >= $start"
    );

    // make an array, keyed by user ID, of earliest xset result
    //
    $a = array();
    foreach ($xrs as $xr) {
        $uid = $xr->user_id;
        if (!array_key_exists($uid, $a) || $xr->create_time < $a[$uid]->xr->create_time) {
            $x = null;
            $x->xr = $xr;
            $a[$uid] = $x;
        }
    }

    // now scan select finishes, and for each user find last one before xset
    //
    foreach ($sfs as $sf) {
        $uid = $sf->user_id;
        if (!array_key_exists($uid, $a)) continue;
        $x = $a[$uid];
        $xr = $x->xr;
        if ($sf->end_time > $xr->create_time) continue;
        $s = $x->sf;
        if (!is_set($x->sf) || $sf->create_time > $s.create_time) {
            $x->sf = $sf;
            $a[$uid] = $x;
        }
    }
    $filename = "compare_snapshot_$course_id_$select_name_$xset_name.json";
    $f = fopen($filename, "w");
    fwrite($f, json_encode($a));
    fclose($f);
}

function read_snapshot($course_id, $select_name, $xset_name) {
    $filename = "compare_snapshot_$course_id_$select_name_$xset_name.json";
    $f = fopen($filename, "r");
    $x = fread($f, filesize($filename));
    fclose($f);
    return json_decode($x);
}

// show comparison results for a given select/xset pair.
//
function show_comparison($ss, $filter, $breakdown) {
}

function show_form() {
    choose_select();
    choose_xset();
}

function show_results() {
}

// get names of units of a given type

function units_of_type($unit, $type) {
    $names = array();
    if (is_subclass_of($unit, $type)) {
        $names[] = $unit->name;
    }
    if (is_subclass_of($unit, "BoltSet")) {
        foreach ($unit->units as $u) {
            $n = units_of_type($u);
            $names = array_merge($names, $n);
        }
    }
    return array_unique($names);
}

// show a menu of select units
//
function select_menu($top_unit) {
    echo "<select name=selects>";
    $names = units_of_type($top_unit, "BoltSelect");
    foreach ($names as $n) {
        echo "<option> $n";
    }
    echo "</select>";
}

// show a menu of exercise sets
//
function xset_menu($top_units) {
    echo "<select name=xsets>";
    $names = units_of_type($top_unit, "BoltExSet");
    foreach ($names as $n) {
        echo "<option> $n";
    }
    echo "</select>";
}

function compare($select_name, $exset_name) {
}

//if (get_str('submit', true)) {
//    show_results();
//} else {
//    show_form();
//}

// compute the mean and stdev of an array
//
function mean_stdev($array, &$mean, &$stdev) {
    $n = 0;
    $m = 0;
    $m2 = 0;

    foreach ($array as $x) {
        $n++;
        $delta = $x - $m;
        $m += $delta/$n;
        $m2 += $delta*($x-$m);
    }
    $mean = $m;
    $stdev = sqrt($m2/($n-1));
}

// approximate the 90% confidence interval for the mean of an array
//
function conf_int_90($array, &$lo, &$hi) {
    $n = count($array);
    mean_stdev($array, $mean, $stdev);

    // I'm too lazy to compute the t distribution
    $t_90 = 1.7;
    $d = $t_90 * $stdev / sqrt($n);
    $lo = $mean - $d;
    $hi = $mean + $d;
}

$a = array(1,1,1,1,0,1,1,1,3, 1, 1, 1, 1);
//mean_stdev($a, $mean, $stdev);
//echo "mean: $mean stdev: $stdev\n";

conf_int_90($a, $lo, $hi);
echo "lo $lo hi $hi\n";

?>
