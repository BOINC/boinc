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
require_once("../inc/bolt_ex.inc");
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

function update_info() {
    $sex = get_int('sex');
    $birth_year = get_int('birth_year');
    $user->bolt->update("sex=$sex, birth_year=$birth_year");
}


$course_doc = require_once($course->doc_file);

function finalize_view($user, $view_id) {
    if (!$view_id) return null;
    $view = BoltView::lookup_id($view_id);
    if ($view && $view->user_id == $user->id && !$view->end_time) {
        $now = time();
        $view->update("end_time=$now");
    }
    return $view;
}

function start_course($user, $course, $course_doc) {
    $iter = new BoltIter($course_doc);
    $iter->at();

    $now = time();
    print_r($iter->state);
    $state = $iter->encode_state();
    BoltEnrollment::insert("(create_time, user_id, course_id, state) values ($now, $user->id, $course->id, '$state')");
    $e = BoltEnrollment::lookup($user->id, $course->id);
    show_item($iter->item, 0, $user, $course, $e);
}

function get_current_item($e, $course_doc) {
    $frac_done = 0;
    $iter = new BoltIter($course_doc);
    $iter->decode_state($e->state);
    $iter->at();
    return $iter;
}

function get_next_item($e, $course_doc) {
    $iter = new BoltIter($course_doc);
    $iter->decode_state($e->state);
    $iter->next();
    $state = $iter->encode_state();
    $e->update("state='$state'");
    return $iter;
}

function show_item($item, $frac_done, $user, $course, $e) {
    $now = time();
    $e->update("last_view=$now, fraction_done=$frac_done");
    $view_id = BoltView::insert("(user_id, course_id, item_name, start_time) values ($user->id, $course->id, '$item->name', $now)");

    if ($item->is_exercise()) {
        $bolt_ex_mode = BOLT_MODE_SHOW;
        $bolt_ex_index = 0;
        echo "
            <form action=bolt_sched.php>
            <input type=hidden name=view_id value=$view_id>
            <input type=hidden name=course_id value=$course->id>
            <input type=hidden name=action value=answer>
        ";
        require($item->filename);
        echo "<p><input type=submit value=OK>
        ";
    } else {
        require_once($item->filename);
        echo "<p><a href=bolt_sched.php?course_id=$course->id&action=next&view_id=$view_id>Next</a>";
    }

    echo "<p>Fraction done: $frac_done";
}

$e = BoltEnrollment::lookup($user->id, $course_id);
switch ($action) {
case 'start':
    if (info_incomplete($user)) {
        request_info($user, $course);
        exit();
    }
    if ($e) {
        page_head("Confirm start");
        echo "You are already enrolled in $course->name.
            Are you sure you want to start from the beginning?
        ";
        show_button(
            "bolt_sched.php?action=start_confirm&course_id=$course->id",
            "Yes",
            "Start this course from the beginning"
        );
        page_tail();
        exit();
    }
case 'start_confirm':
    start_course($user, $course, $course_doc);
    break;
case 'update_info':
    update_info();
    start_course($user, $course, $course_doc);
case 'next':            // "next" button in lesson or exercise answer page
    $view = finalize_view($user, $view_id);
    $e = BoltEnrollment::lookup($user->id, $course_id);
    $iter = get_next_item($e, $course_doc);
    if (!$iter->item) {
        page_head("Done with course");
        echo "All done!";
        page_tail();
        exit();
    }
    show_item($iter->item, $iter->frac_done, $user, $course, $e);
    break;
case 'answer':          // submit answer in exercise
    $view = finalize_view($user, $view_id);
    $iter = get_current_item($e, $course_doc);
    $item = $iter->item;
    if (!$item->is_exercise()) {
        error_page("expected an exercise");
    }
    if ($view->item_name != $item->name) {
        error_page("unexpected name");
    }
    $bolt_ex_mode = BOLT_MODE_SCORE;
    $bolt_ex_index = 0;
    $bolt_ex_score = 0;
    ob_start();     // turn on output buffering
    require($item->filename);
    ob_end_clean();
    echo "score: $bolt_ex_score";
    $bolt_ex_mode = BOLT_MODE_ANSWER;
    $bolt_ex_index = 0;
    require($item->filename);
    break;
}

?>
