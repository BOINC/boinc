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
//      form to choose select and xset; OK goes to:
// snap_form
//      if have a snapshot, show its start/end times
//      show form to get new snapshot
// snap_action
//      make new snapshot
// compare(filter, breakdown)
//      show comparison.
//      show form to set or change filter or breakdown.

require_once("../inc/bolt_util_ops.inc");
require_once("../inc/bolt_db.inc");
require_once("../inc/bolt.inc");
require_once("../inc/bolt_cat.inc");
require_once("../inc/bolt_snap.inc");
require_once("../inc/util_ops.inc");

$filter = null;
$filter_cat = null;
$breakdown = null;
$breakdown_cat = null;

function compare_case(
    $title, $select_unit, $snap, $filter, $filter_cat, $breakdown, $breakdown_cat
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
        $u = $x->sf->selected_unit;
        $a[$u][] = $x->xr->score;
    }

    if ($title) {
        echo "
            <tr class=bolt_head2><td colspan=2><b>$title</b></td></tr>
        ";
    }
    foreach ($select_unit->units as $child) {
        if (array_key_exists($child->name, $a)) {
            $scores = $a[$child->name];
            $n = count($scores);
            if ($n < 2) {
                $x = compare_bar_insuff($child->name, 600);
            } else {
                conf_int_90($scores, $lo, $hi);
                //$x = "($lo, $hi) ($n results)";
                $x = compare_bar($child->name, $n, 600, $lo, $hi);
            }
        } else {
            $x = compare_bar_insuff($child->name, 600);
        }
        echo $x;
    }
}

function compare_aux($select_name, $xset_name, $snap) {
    global $top_unit, $course_id, $filter, $filter_cat;
    global $breakdown, $breakdown_cat;

    get_filters_from_form();

    $select_unit = lookup_unit($top_unit, $select_name);
    if (!$select_unit) error_page("no select unit");

    admin_page_head("Unit comparison");
    echo "
        <link rel=\"stylesheet\" type=\"text/css\" href=\"".URL_BASE."bolt_admin.css\">
        The following compares the alternatives of
        <b>$select_name</b> with respect to <b>$xset_name</b>.
        <p>
    ";

    echo "<table class=\"bolt_box\">";
    if ($breakdown) echo "<tr class=bolt_head1><td colspan=2>Total</td></tr>";

    compare_case(null, $select_unit, $snap, $filter, $filter_cat, null, null);
    if ($breakdown) {
        echo "<tr class=bolt_head1><td colspan=2>Breakdown by ".$breakdown->name()."</td></tr>";
        foreach ($breakdown->categories() as $c) {
            compare_case($c, $select_unit, $snap, $filter, $filter_cat, $breakdown, $c);
            echo "<p>";
        }
    }
    echo "</table>";

    echo "
        <form action=bolt_compare.php>
        <input type=hidden name=action value=compare>
        <input type=hidden name=course_id value=$course_id>
        <input type=hidden name=select_name value=\"$select_name\">
        <input type=hidden name=xset_name value=\"$xset_name\">
        <table width=600><tr><td>
    ";
    filter_form($filter?$filter->name():"", $filter_cat);
    echo "</td><td>";
    breakdown_form($breakdown?$breakdown->name():"");
    echo "
        </td></tr></table>
        <p>
        <input type=submit value=OK>
        </form>
    ";
    admin_page_tail();
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
    admin_page_head("Data snapshot");
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
        <input name=dur value=7> days.
        <input type=submit value=OK>
        </form>
    ";
    admin_page_tail();
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
    admin_page_head("Unit comparison");
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
    admin_page_tail();
}

$course_id = get_int('course_id');
$course = BoltCourse::lookup_id($course_id);
$top_unit = require_once($course->doc_file());

$action = get_str('action', true);
switch ($action) {
case "": show_choice($top_unit); break;
case "snap_form": show_snap_form($top_unit); break;
case "snap_action": snap_action(); break;
case "compare": show_compare(); break;
default: error_page("Unknown action $action");
}

?>
