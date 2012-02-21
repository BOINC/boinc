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

require_once("../inc/bolt_util_ops.inc");
require_once("../inc/bolt_db.inc");
require_once("../inc/bolt_cat.inc");
require_once("../inc/bolt.inc");
require_once("../inc/bolt_snap.inc");
require_once("../inc/util_ops.inc");

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

    admin_page_head("Data snapshot");
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
        <input name=dur value=7> days.
        <input type=submit value=OK>
        </form>
    ";
    admin_page_tail();
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
        $x .= "&nbsp;&nbsp;&nbsp;&nbsp;";
    }
    return $x;
}

// filter arrays of anything that has a user_id field
// (view, xset_result, question)
//
function filter_array($array) {
    global $snap, $filter, $filter_cat, $breakdown, $breakdown_cat;

    if (!$filter && !$breakdown) return $array;
    $x = array();
    foreach ($array as $y) {
        if (!array_key_exists($y->user_id, $snap->users)) continue;
        $u = $snap->users[$y->user_id];
        if ($filter && $filter->categorize($u) != $filter_cat) {
            continue;
        }
        if ($breakdown && $breakdown_cat) {
            if ($breakdown->categorize($u) != $breakdown_cat) {
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
        case BOLT_ACTION_SUBMIT: $x[1]++; break;
        default: $x[2]++; break;
        }
    }
    return $x;
}

function get_nquestions($unit, $mode) {
    global $snap;

    if (array_key_exists($unit->name, $snap->questions)) {
        $a = filter_array($snap->questions[$unit->name]);
        $n = 0;
        foreach ($a as $q) {
            if ($q->mode == $mode) $n++;
        }
        return $n;
    }
    return 0;
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

function class_name($class) {
    switch ($class) {
    case "BoltSequence": return "sequence";
    case "BoltSelect": return "select";
    case "BoltLesson": return "lesson";
    case "BoltExercise": return "exercise";
    case "BoltExerciseSet": return "exercise set";
    case "BoltRandom": return "random";
    }
}

$rownum = 0;

function show_unit_row($unit, $class, $level, $is_answer) {
    global $breakdown, $breakdown_cat;
    global $rownum, $course_id;

    $a = $is_answer?" (answer)":"";
    $j = ($rownum++)%2;
    echo "<tr class=row$j>";
    if ($breakdown && $breakdown_cat) {
        echo "
            <td><br></td>
            <td><br></td>
        ";
    } else  {
        $c = class_name($class);
        echo "
            <td><b>".spaces($level)."$unit->name</b></td>
            <td>$c $a</td>
        ";
    }
    if ($breakdown) {
        if ($breakdown_cat) {
            echo "<td>$breakdown_cat</td>\n";
        } else {
            echo "<td>Total</td>\n";
        }
    }
    switch ($class) {
    case "BoltLesson":
        $mode = BOLT_MODE_LESSON;
        $views = get_views($unit, $mode);
        $n = count($views);
        $out = outcomes($views);
        $t = avg_time($views);
        echo "<td>$n</td>";
        $n = get_nquestions($unit, $mode);
        if ($n) {
            echo "<td><a href=bolt_map.php?action=questions&course_id=$course_id&name=$unit->name&mode=$mode".filter_url().">$n</a></td>\n";
        } else {
            echo "<td>0</td>\n";
        }

        echo outcome_graph($out, 200);
        echo empty_cell();
        echo time_graph($t, 200);
        break;
    case "BoltExercise":
        $mode = $is_answer?BOLT_MODE_ANSWER:BOLT_MODE_SHOW;
        $views = get_views($unit, $mode);
        $n = count($views);
        $out = outcomes($views);
        $t = avg_time($views);
        echo "<td>$n</td>";
        $n = get_nquestions($unit, $mode);
        if ($n) {
            echo "<td><a href=bolt_map.php?action=questions&course_id=$course_id&name=$unit->name&mode=$mode>$n</a></td>\n";
        } else {
            echo "<td>0</td>\n";
        }
        echo outcome_graph($out, 200);
        if ($is_answer) {
            echo empty_cell();
        } else {
            $results = get_results($unit);
            $score = avg_score($results);
            echo score_graph($score, 200);
        }
        echo time_graph($t, 200);
        break;
    case "BoltExerciseSet":
        $xr = get_xset_results($unit);
        $n = count($xr);
        echo "<td>$n</td>";
        echo empty_cell();
        echo empty_cell();
        $score = avg_score($xr);
        echo score_graph($score, 200);
        echo empty_cell();
        break;
    default:
        echo empty_cell();
        echo empty_cell();
        echo empty_cell();
        echo empty_cell();
        echo empty_cell();
    }
    echo "</tr>\n";
}

function breakdown_class($class) {
    switch ($class) {
    case "BoltLesson":
    case "BoltExercise":
    case "BoltExerciseSet":
        return true;
    }
    return false;
}

function show_unit($unit, $level) {
    global $snap, $filter, $filter_cat, $breakdown, $breakdown_cat;

    $class = get_class($unit);
    $breakdown_cat = null;
    show_unit_row($unit, $class, $level, false);
    if ($breakdown && breakdown_class($class)) {
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
                show_unit_row($unit, $class, $level, true);
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
    global $course;

    get_filters_from_form();

    admin_page_head("Course map for '$course->name'");
    bolt_style();
    $snap = read_map_snapshot($course_id);
    start_table();
    echo "
        <tr>
            <th>Name</th>
            <th>Type</th>
    ";
    if ($breakdown) {
        echo "<th>Group</th>";
    }
    echo "
            <th>Views</th>
            <th>Questions</th>
            <th>Outcome<br>
                <span class=green>Next</span>
                <span class=yellow>Back</span>
                <span class=red>None</span>
            </th>
            <th>Score</th>
            <th>Time</th>
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
    filter_form($filter?$filter->name():"", $filter_cat);
    echo "</td><td valign=top>";
    breakdown_form($breakdown?$breakdown->name():"");
    echo "
        </td></tr></table>
        <p>
        <input type=submit value=OK>
        </form>
    ";
    admin_page_tail();
}

function show_questions() {
    global $course_id;

    $name = get_str('name');
    $mode = get_int('mode');
    get_filters_from_form();
    $snap = read_map_snapshot($course_id);
    $qs = $snap->questions[$name];
    admin_page_head("Questions about $name");
    start_table();
    echo "<tr>
        <th>When</th>
        <th>Who</th>
        <th>Question</th>
        </tr>
    ";
    foreach ($qs as $q) {
        if ($q->mode != $mode) continue;
        $user = $snap->users[$q->user_id];
        echo "<tr>
            <td>".time_str($q->create_time)."</td>
            <td><a href=student>$user->name</td>
            <td>$q->question</td>
            </tr>
        ";
    }
    end_table();
    admin_page_tail();
}

$course_id = get_int('course_id');
$course = BoltCourse::lookup_id($course_id);
if (!$course) error_page("no course");
$top_unit = require_once($course->doc_file());

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
case "questions":
    show_questions();
    break;
default:
    error_page("Unknown action $action");
}

?>
