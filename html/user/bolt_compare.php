<?php

// actions:
// (none)
//      form to choose select and xset; OK goes to:
// snap_form
//      if have a snapshot, show its start/end times
//      show form to get new snapshot
// snap_action
//      make new snapshot
// compare(filter, breakdown)
//      show comparison.
//      show form to set or change filter or breakdown.

require_once("../inc/util.inc");
require_once("../inc/bolt_db.inc");
require_once("../inc/bolt_util.inc");

// show comparison results for a given select/xset pair.
//
function show_comparison($ss, $filter, $breakdown) {
}


function show_results() {
}

// get names of units of a given type

function units_of_type($unit, $type) {
    $names = array();
    if (get_class($unit) == $type) {
        $names[] = $unit->name;
    }
    if (is_subclass_of($unit, "BoltSet")) {
        foreach ($unit->units as $u) {
            $n = units_of_type($u, $type);
            $names = array_merge($names, $n);
        }
    }
    return array_unique($names);
}

// show a menu of select units
//
function choose_select($top_unit) {
    echo "<select name=select_name>
        <option selected> ---
    ";
    $names = units_of_type($top_unit, "BoltSelect");
    foreach ($names as $n) {
        echo "<option> $n";
    }
    echo "</select>";
}

// show a menu of exercise sets
//
function choose_xset($top_unit) {
    echo "<select name=xset_name>
        <option selected> ---
    ";
    $names = units_of_type($top_unit, "BoltExerciseSet");
    foreach ($names as $n) {
        echo "<option> $n";
    }
    echo "</select>";
}

function compare_aux($snap) {
    print_r($snap);
    // for each select alternative, build an array of xset scores
    //
    $a = array();
    foreach ($snap->recs as $uid=>$x) {
        //$a[$x->select_finished_name][] = $x->xset_result->score;
    }

    foreach ($a as $name => $scores) {
        conf_int_90($scores, $lo, $hi);
        $n = count($scores);
        echo "
            <br>$name: lo $lo hi $hi ($count results)
        ";
    }
}

function compare($select_name, $exset_name) {
}

function show_snap_form($top_unit) {
    global $course_id;
    $select_name = get_str('select_name');
    $xset_name = get_str('xset_name');
    page_head("Data snapshot");
    $s = read_compare_snapshot($course_id, $select_name, $xset_name);

    if ($s) {
        $end = date_str($s->time);
        echo "
            A data snapshot exists for the $s->dur days prior to $end.
        ";
        button(
            "bolt_compare.php?action=compare&course_id=$course_id",
            "Use this snapshot"
        );
    } else {
        echo "There is currently no snapshot.";
    }
    echo "
        <form action=bolt_compare.php>
        <input type=hidden name=action value=snap_action>
        <input type=hidden name=course_id value=$course_id>
        <input type=hidden name=select_name value=\"$select_name\">
        <input type=hidden name=xset_name value=\"$xset_name\">
        Create a new snapshot using data from the last
        <input name=dur> days.
        <input type=submit value=OK>
        </form>
    ";
    page_tail();
}

function snap_action() {
    global $course_id;
    $select_name = get_str('select_name');
    $xset_name = get_str('xset_name');
    $dur = get_int('dur');
    $s = write_compare_snapshot($course_id, $select_name, $xset_name, $dur);
    compare_aux($s);
}

function show_choice($top_unit) {
    global $course_id;
    page_head("Unit comparison");
    echo "
        <form action=bolt_compare.php>
        <input type=hidden name=course_id value=$course_id>
        This tool lets you compare alternative lessons.
        These lessons must be included in a 'select' unit,
        typically with a random selection function.
        This must be followed by an exercise set
        that tests for the concepts in the lessons.
        <p>
        Please choose a select unit
    ";
    choose_select($top_unit);
    echo "
        and an exercise set
    ";
    choose_xset($top_unit);
    echo "
        <input type=hidden name=action value=snap_form>
        <p>
        <input type=submit value=OK>
    ";
}

$course_id = get_int('course_id');
$course = BoltCourse::lookup_id($course_id);
$top_unit = require_once($course->doc_file);

$action = get_str('action', true);
switch ($action) {
case "": show_choice($top_unit); break;
case "snap_form": show_snap_form($top_unit); break;
case "snap_action": snap_action(); break;
case "compare": show_compare(); break;
default: error_page("Unknown action $action");
}

?>
