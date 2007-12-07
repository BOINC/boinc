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
require_once("../inc/bolt_util.inc");
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

function finalize_view($user, $view_id, $action) {
    if (!$view_id) return null;
    $view = BoltView::lookup_id($view_id);
    if ($view && $view->user_id == $user->id && !$view->end_time) {
        $now = time();
        $view->update("end_time=$now, action=$action");
        return $view;
    }
    return null;
}

function default_mode($item) {
    return $item->is_exercise()?BOLT_MODE_SHOW:BOLT_MODE_LESSON;
}

function create_view($user, $course, $item, $mode) {
    $now = time();
    return BoltView::insert("(user_id, course_id, item_name, start_time, mode) values ($user->id, $course->id, '$item->name', $now, $mode)");
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

function show_item(
    $item, $frac_done, $user, $course, $e, $view_id, $mode
) {
    global $bolt_ex_mode;
    global $bolt_ex_index;

    $now = time();
    $e->update("last_view=$now, fraction_done=$frac_done");

    if ($item->is_exercise()) {
        $bolt_ex_mode = $mode;
        $bolt_ex_index = 0;
        switch ($mode) {
        case BOLT_MODE_SHOW:
            echo "
                <form action=bolt_sched.php>
                <input type=hidden name=view_id value=$view_id>
                <input type=hidden name=course_id value=$course->id>
                <input type=hidden name=action value=answer>
            ";
            srand($view_id);
            require($item->filename);
            echo "<p><input type=submit value=OK>";
            break;
        case BOLT_MODE_ANSWER:
            require($item->filename);
            echo "<p><a href=bolt_sched.php?course_id=$course->id&action=next&view_id=$view_id>Next</a>";
            break;
        }
    } else {
        require_once($item->filename);
        echo "<p><a href=bolt_sched.php?course_id=$course->id&action=next&view_id=$view_id>Next</a>";
    }

    echo "<p>Fraction done: $frac_done
        <a href=bolt_course.php?course_id=$course->id>Course history</a>
    ";
}

function start_course($user, $course, $course_doc) {
    BoltEnrollment::delete($user->id, $course->id);
    $iter = new BoltIter($course_doc);
    $iter->at();

    $now = time();
    print_r($iter->state);
    $state = $iter->encode_state();
    BoltEnrollment::insert("(create_time, user_id, course_id, state) values ($now, $user->id, $course->id, '$state')");
    $e = BoltEnrollment::lookup($user->id, $course->id);
    $mode = default_mode($iter->item);
    $view_id = create_view($user, $course, $iter->item, $mode);
    show_item($iter->item, 0, $user, $course, $e, $view_id, $mode);
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
    $view = finalize_view($user, $view_id, BOLT_ACTION_NEXT);
    $iter = get_next_item($e, $course_doc);
    if (!$iter->item) {
        page_head("Done with course");
        echo "All done!";
        page_tail();
        exit();
    }
    $mode = default_mode($iter->item);
    $view_id = create_view($user, $course, $iter->item, $mode);
    show_item($iter->item, $iter->frac_done, $user, $course, $e, $view_id, $mode);
    break;
case 'answer':          // submit answer in exercise
    $view = finalize_view($user, $view_id, BOLT_ACTION_SUBMIT);
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
    $bolt_ex_response = "";
    srand($view_id);
    ob_start();     // turn on output buffering
    require($item->filename);
    ob_end_clean();

    $result_id = BoltResult::insert(
        "(view_id, score, response)
        values ($view->id, $bolt_ex_score, '$bolt_ex_response')"
    );
    $view->update("result_id=$result_id");
    srand($view_id);
    $view_id = create_view($user, $course, $item, BOLT_MODE_ANSWER);
    show_item(
        $iter->item, $iter->frac_done, $user, $course, $e,
        $view_id, BOLT_MODE_ANSWER
    );
    break;
default:
    $iter = get_current_item($e, $course_doc);
    $mode = default_mode($iter->item);
    $view_id = create_view($user, $course, $iter->item, $mode);
    show_item($iter->item, $iter->frac_done, $user, $course, $e, $view_id, $mode);
    break;
}

?>
