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
//
// columns:
// name
// type
// breakdown category (if using breakdown)
// nviews (as number)
// outcome (as color-coded bar graph: green=next, yellow=back, red=none)
// time (bar graph of log(t))
// score (bar graph)
//
// what's shown:
// lessons: nviews, outcome, time
// exercise: nviews, outcome, time, score
// exercise answer: nviews, outcome, time
// exercise set: score
//
// When breakdown is used, each of above has N lines
// Total, followed by each breakdown category

require_once("../inc/util.inc");
require_once("../inc/bolt_db.inc");
require_once("../inc/bolt_cat.inc");
require_once("../inc/bolt_util.inc");
require_once("../inc/bolt.inc");

// the following are to minimize argument passing

$snap = null;
$course_id = 0;
$top_unit = null;
$filter = null;
$filter_cat = null;
$breakdown = null;
$breakdown_cat = null;

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
    $x = "";
    for ($i=0; $i<$level; $i++) {
        $x .= "&nbsp;&nbsp;&nbsp;";
    }
    return $x;
}

// used to filter arrays of views or xset_results
//
function filter_array($array) {
    global $snap, $filter, $filter_cat, $breakdown, $breakdown_cat;

    if (!$filter) return $array;
    $x = array();
    foreach ($array as $y) {
        $u = $snap->users[$y->user_id];
        if ($filter->category($u) != $filter_cat) {
            continue;
        }
        if ($breakdown && $breakdown_cat) {
            if ($breakdown->category($u) != $breakdown_cat) {
                continue;
            }
        }
        $x[] = $y;
    }
    return $x;
}

function avg_score($array) {
    $sum = 0;
    $n = count($array);
    if ($n ==0) return 0;
    foreach ($array as $a) {
        $sum += $a->score;
    }
    return $sum/count($array);
}

function avg_time($views) {
    $sum = 0;
    $n = 0;
    foreach ($views as $v) {
        if ($v->start_time && $v->end_time) {
            $sum += $v->end_time - $v->start_time;
            $n++;
        }
    }
    if ($n ==0) return 0;
    return $sum/$n;
}

function outcomes($views) {
    $x = array();
    $x[0] = 0;
    $x[1] = 0;
    $x[2] = 0;

    foreach ($views as $v) {
        switch ($v->action) {
        case BOLT_ACTION_NONE: $x[0]++; break;
        case BOLT_ACTION_NEXT: $x[1]++; break;
        default: $x[2]++; break;
        }
    }
    return $x;
}

function get_views($unit, $mode) {
    global $snap;

    $y = array();
    if (array_key_exists($unit->name, $snap->views)) {
        $a = filter_array($snap->views[$unit->name]);
        foreach ($a as $x) {
            if ($x->mode == $mode) $y[] = $x;
        }
    }
    return $y;
}

function get_results($unit) {
    global $snap;

    if (array_key_exists($unit->name, $snap->results)) {
        return filter_array($snap->results[$unit->name]);
    }
    return array();
}

function get_xset_results($unit) {
    global $snap;

    if (array_key_exists($unit->name, $snap->xset_results)) {
        return filter_array($snap->xset_results[$unit->name]);
    }
    return array();
}

function show_unit_row($unit, $class, $level, $is_answer) {
    global $breakdown, $breakdown_cat;

    $a = $is_answer?" (answer)":"";
    echo "<tr>
        <td>".spaces($level)."$unit->name</td>
        <td>$class $a</td>
    ";
    if ($breakdown) {
        if ($breakdown_cat) {
            echo "<td>$breakdown_cat</td>\n";
        } else {
            echo "<td>Total</td>\n";
        }
    }
    switch ($class) {
    case "BoltLesson":
        $views = get_views($unit, BOLT_MODE_LESSON);
        $n = count($views);
        $out = outcomes($views);
        $t = avg_time($views);
        echo "<td>$n</td>";
        echo outcome_graph($out);
        echo time_graph($t);
        echo empty_cell();
        break;
    case "BoltExercise":
        $views = get_views($unit, $is_answer?BOLT_MODE_ANSWER:BOLT_MODE_SHOW);
        $results = get_results($unit);
        $n = count($views);
        $out = outcomes($views);
        $t = avg_time($views);
        $score = avg_score($results);
        echo "<td>$n</td>";
        echo outcome_graph($out);
        echo time_graph($t);
        echo score_graph($score);
        break;
    case "BoltExerciseSet":
        $xr = get_xset_results($unit);
        $score = avg_score($xr);
        echo score_graph($score);
        break;
    default:
    }
    echo "</tr>\n";
}

function show_unit($unit, $level) {
    global $snap, $filter, $filter_cat, $breakdown, $breakdown_cat;

    $class = get_class($unit);
    $breakdown_cat = null;
    show_unit_row($unit, $class, $level, false);
    if ($breakdown) {
        foreach ($breakdown->categories() as $c) {
            $breakdown_cat = $c;
            show_unit_row($unit, $class, $level, false);
        }
    }

    // if exercise, show answer page views
    //
    if ($class == "BoltExercise") {
        $breakdown_cat = null;
        show_unit_row($unit, $class, $level, true);
        if ($breakdown) {
            foreach ($breakdown->categories() as $c) {
                $breakdown_cat = $c;
                show_unit_row($unit, $class, $level, $true);
            }
        }
    }
}

function show_unit_recurse($unit, $level) {
    show_unit($unit, $level);
    if ($unit->is_item) return;
    foreach ($unit->units as $u) {
        show_unit_recurse($u, $level+1);
    }
}

function show_map() {
    global $snap, $course_id, $top_unit, $filter, $filter_cat, $breakdown;

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
    show_unit_recurse($top_unit, 0);
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
