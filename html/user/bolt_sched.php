<?php

// Bolt scheduler.
// GET args:
//   course_id: course ID
//   action: see commands below

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
    global $user;
    $sex = get_int('sex');
    $birth_year = get_int('birth_year');
    $user->bolt->update("sex=$sex, birth_year=$birth_year");
}

$course_doc = require_once($course->doc_file);

// The user clicked something on a view page.
// Make a record of it, and of the time
//
function finalize_view($user, $view_id, $action) {
    if (!$view_id) return null;
    $view = BoltView::lookup_id($view_id);
    if (!$view) {
        error_page("no view");
    }
    if ($view->user_id != $user->id) {
        error_page("wrong user");
    }
    if (!$view->end_time) {
        $now = time();
        $view->update("end_time=$now, action=$action");
    }
    return $view;
}

function default_mode($item) {
    return $item->is_exercise()?BOLT_MODE_SHOW:BOLT_MODE_LESSON;
}

// A page is being shown to the user; make a record of it
//
function create_view($user, $course, $iter, $mode, $prev_view_id) {
    $now = time();
    $item = $iter->item;
    if (!$item) {
        $item = null;
        $item->name = '--end--';
    }
    $state = $iter->encode_state();
    if ($user->bolt->debug) {
        echo "<pre>Ending state: "; print_r($iter->state); echo "</pre>\n";
    }
    return BoltView::insert("(user_id, course_id, item_name, start_time, mode, state, fraction_done, prev_view_id) values ($user->id, $course->id, '$item->name', $now, $mode, '$state', $iter->frac_done, $prev_view_id)");
}

// show a page saying the course has been completed
//
function show_finished_page($course, $view_id, $prev_view_id) {
    page_head(null);
    if (function_exists('bolt_header')) bolt_header("Course completed");
    echo "Congratulations - you have completed this course.";
    $prev = "<a href=bolt_sched.php?course_id=$course->id&action=prev&view_id=$view_id><< Prev</a>";
    echo "
        <p>
        <center>
        <table width=60%><tr>
            <td width=33% align=left>$prev</td>
            <td width=33% align=center><a href=bolt.php>Up</a></td>
            <td width=33% align=right></td>
        </table>
        </center>
    ";
    if (function_exists('bolt_footer')) bolt_footer();
}

// show an item (lesson, exercise, answer page)
//
function show_item($iter, $user, $course, $view_id, $prev_view_id, $mode) {
    global $bolt_ex_mode;
    global $bolt_ex_index;
    global $bolt_ex_score;
    global $bolt_query_string;

    $item = $iter->item;
    page_head(null);
    if (function_exists('bolt_header')) bolt_header($item->title);
    $bolt_query_string = $item->query_string;

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
            if (function_exists('bolt_divide')) bolt_divide();
            $next = "<input type=submit value=OK></form>";
            break;
        case BOLT_MODE_ANSWER:
            require($item->filename);
            if (function_exists('bolt_divide')) bolt_divide();
            $next = "<a href=bolt_sched.php?course_id=$course->id&action=next&view_id=$view_id>Next >></a>";
            $score_pct = number_format($bolt_ex_score*100);
            echo "Score: $score_pct%";
            break;
        }
    } else {
        require_once($item->filename);
        if (function_exists('bolt_divide')) bolt_divide();
        $next = "<a href=bolt_sched.php?course_id=$course->id&action=next&view_id=$view_id>Next >></a>";
    }

    if ($prev_view_id) {
        $prev = "<a href=bolt_sched.php?course_id=$course->id&action=prev&view_id=$view_id><< Prev</a>";
    } else {
        $prev = "";
    }

    echo "
        <p>
        <center>
        <table width=60%><tr>
            <td width=33% align=left>$prev</td>
            <td width=33% align=center><a href=bolt_sched.php?course_id=$course->id&action=course_home&view_id=$view_id>Up</a></td>
            <td width=33% align=right>$next</td>
        </table>
        </center>
    ";
    if (function_exists('bolt_footer')) bolt_footer();

    $e = new BoltEnrollment();
    $e->user_id = $user->id;
    $e->course_id = $course->id;
    $e->update("last_view_id=$view_id");
}

// Show the student the results of an old exercise; no navigation items
//
function show_answer_page($iter, $score) {
    global $bolt_ex_mode;
    global $bolt_ex_index;
    global $bolt_query_string;

    $bolt_ex_mode = BOLT_MODE_ANSWER;
    $bolt_ex_index = 0;

    $item = $iter->item;
    page_head(null);
    if (function_exists('bolt_header')) bolt_header($item->title);
    $bolt_query_string = $item->query_string;
    require_once($item->filename);
    if (function_exists('bolt_divide')) bolt_divide();
    $score_pct = number_format($score*100);
    echo "Score: $score_pct%";
    if (function_exists('bolt_footer')) bolt_footer();
}

function start_course($user, $course, $course_doc) {
    BoltEnrollment::delete($user->id, $course->id);
    $iter = new BoltIter($course_doc);
    $iter->at();

    $now = time();
    $mode = default_mode($iter->item);
    $view_id = create_view($user, $course, $iter, $mode, 0);
    BoltEnrollment::insert("(create_time, user_id, course_id, last_view_id) values ($now, $user->id, $course->id, $view_id)");
    show_item($iter, $user, $course, $view_id, 0, $mode);
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
case 'prev':
    $view = finalize_view($user, $view_id, BOLT_ACTION_PREV);
    if ($view->prev_view_id) {
        $view = BoltView::lookup_id($view->prev_view_id);
        $iter = new BoltIter($course_doc);
        $iter->decode_state($view->state);
        $iter->at();
        $mode = default_mode($iter->item);
        $view_id = create_view(
            $user, $course, $iter, $mode, $view->prev_view_id
        );
        show_item($iter, $user, $course, $view_id, $view->prev_view_id, $mode);
    } else {
        error_page("At start of course");
    }
    break;
case 'next':            // "next" button in lesson or exercise answer page
    $view = finalize_view($user, $view_id, BOLT_ACTION_NEXT);

    $iter = new BoltIter($course_doc);
    $iter->decode_state($view->state);

    if ($user->bolt->debug) {
        echo "<pre>Initial state: "; print_r($iter->state); echo "</pre>\n";
    }

    $iter->next();

    if ($user->bolt->debug) {
        echo "<pre>Item: "; print_r($iter->item); echo "</pre>\n";
    }
    if ($iter->item) {
        $state = $iter->encode_state();
        $mode = default_mode($iter->item);
        $view_id = create_view($user, $course, $iter, $mode, $view->id);
        show_item($iter, $user, $course, $view_id, $view->id, $mode);
    } else {
        $iter->frac_done = 1;
        $fin_view_id = create_view($user, $course, $iter, BOLT_MODE_FINISHED, $view_id);
        $e->update("last_view_id=$fin_view_id");
        show_finished_page($course, $fin_view_id, $view->id);
    }
    break;
case 'answer':          // submit answer in exercise
    $view = finalize_view($user, $view_id, BOLT_ACTION_SUBMIT);
    if ($user->bolt->debug) {
        echo "<pre>State: $view->state</pre>\n";
    }
    $iter = new BoltIter($course_doc);
    $iter->decode_state($view->state);
    $iter->at();

    if ($user->bolt->debug) {
        echo "<pre>Item: "; print_r($iter->item); echo "</pre>\n";
    }
    $item = $iter->item;
    if (!$item->is_exercise()) {
        print_r($item);
        error_page("expected an exercise");
    }
    if ($view->item_name != $item->name) {
        error_page("unexpected name");
    }

    // compute the score

    $bolt_ex_query_string = $_SERVER['QUERY_STRING'];
    $bolt_ex_mode = BOLT_MODE_SCORE;
    $bolt_ex_index = 0;
    $bolt_ex_score = 0;
    $bolt_query_string = $item->query_string;
    srand($view_id);
    ob_start();     // turn on output buffering
    require($item->filename);
    ob_end_clean();

    $bolt_ex_score /= $bolt_ex_index;

    // make a record of the result

    $qs = BoltDb::escape_string($_SERVER['QUERY_STRING']);
    $result_id = BoltResult::insert(
        "(view_id, score, response)
        values ($view->id, $bolt_ex_score, '$qs')"
    );
    $view->update("result_id=$result_id");

    // show the answer page

    srand($view_id);
    $view_id = create_view($user, $course, $iter, BOLT_MODE_ANSWER, $view->id);
    show_item($iter, $user, $course, $view_id, $view->id, BOLT_MODE_ANSWER);
    break;
case 'answer_page':
    $view = BoltView::lookup_id($view_id);
    $iter = new BoltIter($course_doc);
    $iter->decode_state($view->state);
    $iter->at();
    if ($iter->item->name != $view->item_name) {
        error_page("Exercise no longer exists in course");
    }
    $result = BoltResult::lookup_id($view->result_id);
    srand($view_id);
    $bolt_ex_query_string = $result->response;
    show_answer_page($iter, $result->score);
    break;
case 'course_home':
    $view = finalize_view($user, $view_id, BOLT_ACTION_COURSE_HOME);
    Header("Location: bolt.php");
    break;
default:
    $view = $e?BoltView::lookup_id($e->last_view_id):null;
    if (!$view) {
        start_course($user, $course, $course_doc);
        break;
    }
    if ($view->mode == BOLT_MODE_FINISHED) {
        show_finished_page($course, $view->id, $view->prev_view_id);
        break;
    }
    $iter = new BoltIter($course_doc);
    $iter->decode_state($view->state);
    $iter->at();
    $mode = $view->mode;
    if ($view->item_name == $iter->item->name && ($mode == BOLT_MODE_ANSWER)) {
        // if we're returning to an answer page,
        // we need to look up the user's responses and the score.
        //
        $view_orig = BoltView::lookup_id($view->prev_view_id);
        $result = BoltResult::lookup_id($view_orig->result_id);
        srand($view_orig->id);
        echo "reshow: $result->response";
        $bolt_ex_query_string = $result->response;
        $bolt_ex_score = $result->score;
        $bolt_ex_index = 0;
        $view_id = create_view($user, $course, $iter, $mode, $view_orig->id);
        show_item($iter, $user, $course, $view_id, $view_orig->id, $mode);
    } else {
        $view_id = create_view($user, $course, $iter, $mode, $view->id);
        show_item($iter, $user, $course, $view_id, $view->id, $mode);
    }
    break;
}

?>
