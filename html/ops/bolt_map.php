<?php

// actions:
// (none)
//      if have a snapshot, show start/end
//      show form to get new snapshot
// snap_action
//      make new snapshot and go to...
// map
//      show a map;
//      show form to set or change filter or breakdown

require_once("../inc/util.inc");
require_once("../inc/bolt_db.inc");
require_once("../inc/bolt_cat.inc");
require_once("../inc/bolt_util.inc");
require_once("../inc/bolt.inc");

function show_snap_form() {
    global $course_id;
    page_head("Data snapshot");
    $s = read_map_snapshot($course_id);

    if ($s) {
        $end = date_str($s->time);
        echo "
            A data snapshot exists for the $s->dur days prior to $end.
        ";
        show_button(
            "bolt_map.php?action=map&course_id=$course_id",
            "Use this snapshot",
            "Use this snapshot"
        );
    } else {
        echo "There is currently no snapshot.";
    }
    echo "
        <form action=bolt_map.php>
        <input type=hidden name=action value=snap_action>
        <input type=hidden name=course_id value=$course_id>
        Create a new snapshot using data from the last
        <input name=dur> days.
        <input type=submit value=OK>
        </form>
    ";
    page_tail();
}

function snap_action() {
    global $course_id;
    global $top_unit;

    $dur = get_int('dur');
    $s = write_map_snapshot($course_id, $dur);
    show_map();
}

function show_unit($snap, $unit) {
    $class = get_class($unit);
    echo "<li> $unit->name ($class); ";
    if ($unit->is_item) {
        if (array_key_exists($unit->name, $snap->views)) {
            $n = count($snap->views[$unit->name]);
        } else {
            $n = 0;
        }
        echo "$n views";
    }
    if ($class == "BoltExercise") {
        if (array_key_exists($unit->name, $snap->results)) {
            $rs = $snap->results[$unit->name];
            $sum = 0;
            $n = count($rs);
            foreach ($rs as $r) {
                $sum += $r->score;
            }
            $avg = $sum/$n;
            echo " avg score: $avg ($n)";
        }
    }
    if ($class == "BoltExerciseSet") {
        if (array_key_exists($unit->name, $snap->xset_results)) {
            $xrs = $snap->xset_results[$unit->name];
            $sum = 0;
            $n = count($xrs);
            foreach ($xrs as $xr) {
                $sum += $xr->score;
            }
            $avg = $sum/$n;
            echo " avg score: $avg ($n)";
        }
    }
    echo "\n";
}

function show_unit_recurse($snap, $unit) {
    show_unit($snap, $unit);
    if ($unit->is_item) return;
    foreach ($unit->units as $u) {
        echo "<ul>\n";
        show_unit_recurse($snap, $u);
        echo "</ul>\n";
    }
}

function show_map() {
    global $course_id;
    global $top_unit;

    page_head("Course map");
    $snap = read_map_snapshot($course_id);
    show_unit_recurse($snap, $top_unit);
    page_tail();
}

$course_id = get_int('course_id');
$course = BoltCourse::lookup_id($course_id);
if (!$course) error_page("no course");
$top_unit = require_once("../user/$course->doc_file");

$action = get_str('action', true);
switch ($action) {
case "":
    show_snap_form();
    break;
case "snap_action":
    snap_action();
    break;
case "map":
    show_map();
    break;
default:
    error_page("Unknown action $action");
}

?>
