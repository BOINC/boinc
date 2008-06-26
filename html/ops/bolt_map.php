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

require_once("../util.inc");
require_once("../inc/bolt_db.inc");
require_once("../inc/bolt_cat.inc");
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
     $dur = get_int('dur');
     $s = write_map_snapshot($course_id, $dur);
     map_aux($select_name, $xset_name, $s);

}

function map_aux($snap) {
}

function show_map($unit, $level) {
    for ($i=0; $i<$level; $i++) echo '  ';
    echo "$unit->name: $unit->nviews\n";
    if ($unit->is_item) return;
    foreach ($unit->units as $u) {
        show_map($u, $level+1);
    }
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
    show_map($top_unit, 0);
    break;
default:
    error_page("Unknown action $action");
}

?>
