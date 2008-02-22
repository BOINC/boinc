<?php
require_once("../inc/bolt_db.inc");
require_once("../inc/bolt.inc");

// return true if the given state is still in the course
//
function consistent($view, $state, $unit) {
    while (1) {
        if ($unit->name == $view->item_name) return true;
        if ($unit->is_item) {
            echo "ERROR: $unit->name $view->item_name <pre>";
            print_r($state);
            print_r($view);
            return null;
        }
        $unit = $unit->get_child($state);
        if (!$unit) return null;
    }
}

function add_to_totals($view, $state, &$unit) {
    $u = $unit;
    while (1) {
        $u->nviews++;
        if ($u->is_item) return;
        $u = $u->get_child($state);
        if (!$u) {
            echo "ERROR - NOT ITEM";
        }
    }
}

function init_totals(&$unit) {
    $unit->nviews = 0;
    if ($unit->is_item) return;
    foreach ($unit->units as $u) {
        init_totals($u);
    }
}

function show_totals($unit, $level) {
    for ($i=0; $i<$level; $i++) echo '  ';
    echo "$unit->name: $unit->nviews\n";
    if ($unit->is_item) return;
    foreach ($unit->units as $u) {
        show_totals($u, $level+1);
    }
}

// scan the course's views and total up counts
//
function count_views($course, &$top_unit) {
    $views = BoltView::enum("course_id=$course->id");
    init_totals($top_unit);
    foreach ($views as $view) {
        $state = json_decode($view->state, true);
        if (consistent($view, $state, $top_unit)) {
            add_to_totals($view, $state, $top_unit);
        }
    }
}

$course_id = get_int('course_id');
$course = BoltCourse::lookup_id($course_id);
if (!$course) error_page("no course");
$top_unit = require_once("../user/$course->doc_file");

echo "<pre>";
count_views($course, $top_unit);
show_totals($top_unit, 0);
echo "</pre>";

?>
