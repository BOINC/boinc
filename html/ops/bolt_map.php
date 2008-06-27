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

// things to show:
// all:
//      # views (as number)
// items:
//      fraction of dead-ends
//          as bar graph, red=dead-end, green=next, yellow=back
//      seconds spent
//          (for exercises, show answer sheet separately)
//          bar graph of log(t)
//      exercises:
//          average score (bar graph)
// exercise sets:
//      average score (bar graph)

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

function spaces($level) {
    for ($i=0; $i<level; $i++) {
        echo "&nbsp;&nbsp;";
    }
}

function filter_array($array, $snap, $filter, $filter_cat) {
    if (!$filter) return $array;
    $x = array();
    foreach ($array as $y) {
        $u = $snap->users[$y->user_id];
        if ($filter->category($u) == $filter_cat) {
            $x[] = $y;
        }
    }
    return $x;
}

function avg_score($array) {
    $sum = 0;
    foreach ($array as $a) {
        $sum += $a->score;
    }
    return $sum/count($array);
}

function show_unit($snap, $unit, $level, $filter, $filter_cat, $breakdown) {
    $class = get_class($unit);
    echo "<tr>
        <td>".spaces($level)."$unit->name</td>
        <td>$class</td>
    ";
    if ($unit->is_item) {
        if (array_key_exists($unit->name, $snap->views)) {
            $a = filter_array(
                $snap->views[$unit->name], $snap, $filter, $filter_cat
            );
            $n = count($a);
        } else {
            $n = 0;
        }
        echo "<td>$n</td>";
    } else {
        echo "<td><br></td>";
    }
    if ($class == "BoltExercise") {
        if (array_key_exists($unit->name, $snap->results)) {
            $a = filter_array(
                $snap->results[$unit->name], $snap, $filter, $filter_cat
            );
            $avg = avg_score($a);
            $n = count($a);
            bolt_bar($avg);
        }
    } else if ($class == "BoltExerciseSet") {
        if (array_key_exists($unit->name, $snap->xset_results)) {
            $a = filter_array(
                $snap->xset_results[$unit->name], $snap, $filter, $filter_cat
            );
            $avg = avg_score($a);
            bolt_bar($avg);
        }
    } else {
        echo "<td><br></td>";
    }
    echo "\n";
}

function show_unit_recurse(
    $snap, $unit, $level, $filter, $filter_cat, $breakdown
) {
    show_unit($snap, $unit, $level, $filter, $filter_cat, $breakdown);
    if ($unit->is_item) return;
    foreach ($unit->units as $u) {
        show_unit_recurse($snap, $u, $level+1, $filter, $filter_cat, $breakdown);
    }
}

function show_map() {
    global $course_id;
    global $top_unit;

    $breakdown_name = get_str('breakdown', true);
    if ($breakdown_name) {
        $breakdown = lookup_categorization($breakdown_name);
        if (!$breakdown) error_page("unknown breakdown $breakdown_name");
    } else {
        $breakdown = null;
    }
    $filter_info = get_str('filter', true);
    if ($filter_info && $filter_info != "none") {
        $arr = explode(":", $filter_info);
        $filter_name = $arr[0];
        $filter_cat = $arr[1];
        $filter = lookup_categorization($filter_name);
        if (!$filter) error_page("unknown filter $filter_name");
    } else {
        $filter_name = "";
        $filter_cat = "";
        $filter = null;
    }

    page_head("Course map");
    $snap = read_map_snapshot($course_id);
    echo "
        <table class=bolt_box>
        <tr>
            <th>Name</th>
            <th>Type</th>
            <th>Views</th>
            <th>Outcome</th>
            <th>Time</th>
            <th>Score</th>
        </tr>
    ";
    show_unit_recurse($snap, $top_unit, $filter, $filter_cat, $breakdown);
    echo "
        </table>
        <form action=bolt_map.php>
        <input type=hidden name=action value=map>
        <input type=hidden name=course_id value=$course_id>
        <table width=600><tr><td valign=top>
    ";
    filter_form($filter_name, $filter_cat);
    echo "</td><td valign=top>";
    breakdown_form($breakdown_name);
    echo "
        </td></tr></table>
        <p>
        <input type=submit value=OK>
        </form>
    ";
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
