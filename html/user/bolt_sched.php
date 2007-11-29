<?php

// Bolt scheduler.  GET args:
// course_id: course ID
// action:
//      'start' or none: show current (or first) item,
//          and prompt for user info if any missing
//      'next': go to next lesson
// answers:
//      JSON represenation of exercise answers

require_once("../inc/bolt.inc");
require_once("../inc/bolt_db.inc");
require_once("../inc/util.inc");

$user = get_logged_in_user();
BoltUser::lookup($user);
$course_id = get_int('course_id');
$view_id = get_int('view_id', true);
$action = get_str('action', true);

$course = BoltCourse::lookup_id($course_id);
if (!$course) {
    error_page("no such course");
}

if ($action == 'update_info') {
    $sex = get_int('sex');
    $birth_year = get_int('birth_year');
    $user->bolt->update("sex=$sex, birth_year=$birth_year");
    $action = "";
}

if ($action == 'start' && info_incomplete($user)) {
    request_info($user, $course);
    exit();
}

$course_doc = require_once($course->doc_file);

if ($view_id) {
    $view = BoltView::lookup_id($view_id);
    if ($view && $view->user_id == $user->id && !$view->end_time) {
        $now = time();
        $view->update("end_time=$now");
    }
}

$frac_done = 0;
$e = BoltEnrollment::lookup($user->id, $course_id);
if ($e) {
    $iter = new BoltIter($course_doc);
    $iter->stack = json_decode($e->state);
    if ($action == 'next') {
        $item = $iter->next($frac_done);
        $state = json_encode($iter->stack);
        $e->update("state='$state'");
    } else if ($action == 'start') {
        $iter->stack = null;
        $item = $iter->at($frac_done);
        $state = json_encode($iter->stack);
        $e->update("state='$state'");
    } else {
        $item = $iter->at();
    }
} else {
    $iter = new BoltIter($course_doc);
    $item = $iter->at();

    $now = time();
    $state = json_encode($iter->stack);
    BoltEnrollment::insert("(create_time, user_id, course_id, state) values ($now, $user->id, $course_id, '$state')");
    $e = BoltEnrollment::lookup($user->id, $course_id);
}

if (!$item) {
    page_head("Done with course");
    echo "All done!";
    page_tail();
    exit();
}

$now = time();
$e->update("last_view=$now, fraction_done=$frac_done");
$view_id = BoltView::insert("(user_id, course_id, item_name, start_time) values ($user->id, $course_id, '$item->name', $now)");

require_once($item->filename);

echo "<p><a href=bolt_sched.php?course_id=$course_id&action=next&view_id=$view_id>Next</a>";

echo "<p>Fraction done: $frac_done";
?>
