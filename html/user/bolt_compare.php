<?php

// actions:
// none
//      form to choose select and xset; OK goes to:
// snap_form
//      if have a snapshot, show its start/end times
//      show form to get new snapshot
// snap_action
//      make new snapshot
// compare(filter, breakdown)
//      show comparison.
//      show form to set or change filter or breakdown.

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

?>
