<?php

// Bolt scheduler.  POST args:
// course_id: course ID
// action:
//      'show' or none: show current (or first) item
//      'next': go to next lesson
// answers:
//      JSON represenation of exercise answers

require_once("../inc/bolt.inc");
require_once("../inc/bolt_db.inc");
require_once("../inc/util.inc");

$user = get_logged_in_user();
$course_id = get_int('course_id');
$view_id = get_int('view_id', true);
$action = get_str('action', true);

$course = BoltCourse::lookup_id($course_id);
if (!$course) {
    error_page("no such course");
}

$course_doc = require_once($course->doc_file);

if ($view_id) {
    $view = BoltView::lookup_id($view_id);
    if ($view && $view->user_id == $user->id && !$view->end_time) {
        $now = time();
        $view->update("end_time=$now");
    }
}

$e = BoltEnrollment::lookup($user->id, $course_id);

if ($e) {
    $iter = new BoltIter($course_doc);
    $iter->stack = json_decode($e->state);
    if ($action == 'next') {
        $item = $iter->next();
        $state = json_encode($iter->stack);
        $e->update_aux("state='$state' where user_id=$user->id and course_id=$course_id");
    } else if ($action == 'start') {
        $iter->stack = null;
        $item = $iter->at();
        $state = json_encode($iter->stack);
        $e->update_aux("state='$state' where user_id=$user->id and course_id=$course_id");
    } else {
        $item = $iter->at();
    }
} else {
    $iter = new BoltIter($course_doc);
    $item = $iter->at();

    $e = new BoltEnrollment($course);
    $e->user_id = $user->id;
    $e->course_id = $course_id;
    $e->state = json_encode($iter->stack);
    $e->insert();
}

if (!$item) {
    page_head("Done with course");
    echo "All done!";
    page_tail();
    exit();
}

$view = new BoltView();
$view->user_id = $user->id;
$view->course_id = $course_id;
$view->item_name = $item->name;
$view->insert();

require_once($item->filename);

echo "<p><a href=bolt_sched.php?course_id=$course_id&action=next&view_id=$view->id>Next</a>";
?>
