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
require_once("../inc/bolt_cat.inc");

function filter_form($sel_name, $sel_cat) {
    global $categorizations;
    $checked = (!$sel_name || $sel_name == "none")?"checked":"";
    echo "
        Filter by:
        <ul>
        <li><input type=radio name=filter value=none $checked> None
    ";
    foreach ($categorizations as $c) {
        $name = $c->name();
        $cats = $c->categories();
        echo "
            <li> $name
            <ul>
        ";
        foreach ($cats as $x) {
            $checked = ($sel_name == $name && $sel_cat == $x) ? "checked":"";
            echo "
                <li> <input type=radio name=filter value=\"$name:$x\" $checked> $x
            ";
        }
        echo "</ul>";
    }
    echo "</ul>";
}

function breakdown_form($sel_name) {
    global $categorizations;
    echo "
        Break down by:
        <ul>
        <li><input type=radio name=breakdown value=none> None
    ";
    foreach ($categorizations as $c) {
        $name = $c->name();
        $checked = ($sel_name == $name)?"checked":"";
        echo "
            <li> <input type=radio name=breakdown value=\"$name\" $checked> $name
        ";
    }
    echo "</ul>";
}

function compare_case(
    $select_unit, $snap, $filter, $filter_cat, $breakdown, $breakdown_cat
) {

    // for each select alternative, build an array of xset scores
    //
    $a = array();
    foreach ($snap->recs as $uid=>$x) {
        if ($filter && $filter->categorize($x->user) != $filter_cat) {
            //echo "<br>$uid rejected by filter ";
            continue;
        }
        if ($breakdown && $breakdown->categorize($x->user) != $breakdown_cat) {
            //echo "<br>$uid rejected by breakdown ";
            continue;
        }
        $z = $x->sf->selected_unit;
        echo "<br>unit: $z ";
        $u = $x->sf->selected_unit;
        $a[$u][] = $x->xr->score;
    }

    foreach ($select_unit->units as $child) {
        if (array_key_exists($child->name, $a)) {
            $scores = $a[$child->name];
            $n = count($scores);
            if ($n < 2) {
                $x = "insufficient data";
            } else {
                conf_int_90($scores, $lo, $hi);
                $x = "($lo, $hi) ($n results)";
            }
        } else {
            $x = "insufficient data";
        }
        echo "
            <p>$child->name: $x
        ";
    }
}

function compare_aux($select_name, $xset_name, $snap) {
    global $top_unit;
    global $course_id;

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

    $select_unit = lookup_unit($top_unit, $select_name);
    if (!$select_unit) error_page("no select unit");

    page_head("Unit comparison");
    echo "
        The following compares the alternatives of the
        <b>$select_name</b> select unit
        with respect to the <b>$xset_name</b> exercise set unit.
    ";

    compare_case($select_unit, $snap, $filter, $filter_cat, null, null);
    if ($breakdown) {
        foreach ($breakdown->categories() as $c) {
            echo "<h3>$c</h3>";
            compare_case($select_unit, $snap, $filter, $filter_cat, $breakdown, $c);
        }
    }

    echo "
        <form action=bolt_compare.php>
        <input type=hidden name=action value=compare>
        <input type=hidden name=course_id value=$course_id>
        <input type=hidden name=select_name value=\"$select_name\">
        <input type=hidden name=xset_name value=\"$xset_name\">
        <table><tr><td>
    ";
    filter_form($filter_name, $filter_cat);
    echo "</td><td>";
    breakdown_form($breakdown_name);
    echo "
        </td></tr></table>
        <p>
        <input type=submit value=OK>
        </form>
    ";
    page_tail();
}

function show_compare() {
    global $course_id;
    $select_name = get_str('select_name');
    $xset_name = get_str('xset_name');
    $s = read_compare_snapshot($course_id, $select_name, $xset_name);
    compare_aux($select_name, $xset_name, $s);
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
        show_button(
            "bolt_compare.php?action=compare&course_id=$course_id&select_name=$select_name&xset_name=$xset_name",
            "Use this snapshot",
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
    compare_aux($select_name, $xset_name, $s);
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
