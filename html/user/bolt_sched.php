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

// Bolt scheduler.
// GET args:
//   course_id: course ID
//   action: see commands below

require_once("../inc/bolt.inc");
require_once("../inc/bolt_sched.inc");
require_once("../inc/bolt_db.inc");
require_once("../inc/bolt_ex.inc");
require_once("../inc/bolt_util.inc");
require_once("../inc/util.inc");

function debug_show_state($state, $tag) {
    global $user;
    global $refresh;
    if ($user->bolt->flags&BOLT_FLAGS_DEBUG) {
        echo "$tag state: <pre>"; print_r($state); echo "</pre>\n";
        if ($refresh) {
            echo "<p>Refresh ID: $refresh->id<p>";
        }
        echo "<hr>\n";
    }
}
function debug_show_item($item) {
    global $user;
    if ($user->bolt->flags&BOLT_FLAGS_DEBUG) {
        echo "Item:<pre>"; print_r($item); echo "</pre>\n";
        echo "<hr>\n";
    }
}

function update_info() {
    global $user;
    $sex = get_int('sex');
    $birth_year = get_int('birth_year');
    $user->bolt->update("sex=$sex, birth_year=$birth_year");
}

// The user clicked something on a page.
// Look up the view record (the ID is in the URL) and update it
// with the action and the time.
// Return the record.
//
function finalize_view($view_id, $action) {
    global $user;
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
function create_view($iter, $mode, $prev_view_id) {
    global $user;
    global $course;

    $now = time();
    $item = $iter->item;
    if (!$item) {
        $item = null;
        $item->name = '--end--';
    }
    $state = $iter->encode_state();
    debug_show_state($iter->state, "Ending");
    return BoltView::insert("(user_id, course_id, item_name, start_time, mode, state, fraction_done, prev_view_id) values ($user->id, $course->id, '$item->name', $now, $mode, '$state', $iter->frac_done, $prev_view_id)");
}

function page_header() {
    global $course;
    echo "<html><head>
        <title>$course->name</title>
        <link rel=stylesheet type=text/css href=bolt.css>
        </head><body>
    ";
    if (function_exists('bolt_header')) bolt_header();
}

function page_footer() {
    if (function_exists('bolt_footer')) bolt_footer();
    echo "</body></html>";
}

// show a page saying the course has been completed
//
function show_finished_page($view_id, $prev_view_id) {
    global $course;
    global $url_args;

    page_header("Course completed");
    if ($course->bossa_app_id) {
        require_once("../inc/bossa_db.inc");
        $app = BossaApp::lookup_id($course->bossa_app_id);
        echo "
            Congratulations - you have completed the training for $course->name.
            <p>
            You may now
            <a href=bossa_get_job.php?bossa_app_id=$course->bossa_app_id>do work</a>.
        ";
    } else {
        echo "Congratulations - you have completed this course.";
        $links[] = "<a href=bolt_sched.php?$url_args&action=prev&view_id=$view_id><img src=img/prev.gif></a>";
        $up_link = "<a href=bolt_sched.php?$url_args&action=course_home&view_id=$view_id>Course home page</a>";
        show_nav($links, $up_link, $view_id);
    }
    page_footer();
}

function show_refresh_finished() {
    page_header("Refresh completed");
    echo "<a href=bolt.php>Return to courses</a>";
    page_footer();
}

function show_nav($links, $up_link, $view_id) {
    global $course;

    echo "<p><center>
        <table width=60%><tr>
    ";
    foreach ($links as $link) {
        echo "<td align=center>$link</td>\n";
    }
    echo "</tr></table> </center>
        <hr>
        <br>
        <form action=bolt_sched.php>
        <input type=hidden name=course_id value=$course->id>
        <input type=hidden name=action value=question>
        <input type=hidden name=view_id value=$view_id>
        <textarea name=question cols=60 onfocus=\"this.value=''\">Enter question or comment here</textarea>
        <br>
        <input type=submit value=Submit>
        </form>
        <p>
        $up_link
    ";
}

// show an item (lesson, exercise, answer page)
//
function show_item($iter, $view_id, $prev_view_id, $mode, $repeat=null) {
    global $user;
    global $course;
    global $bolt_ex;
    global $refresh;
    global $url_args;

    $item = $iter->item;
    page_header();
    $bolt_query_string = $item->query_string;

    $links = array();
    if ($prev_view_id) {
        $links[] = "<a href=bolt_sched.php?$url_args&action=prev&view_id=$view_id><img src=img/prev.gif></a>";
    }

    $next = "<a href=bolt_sched.php?$url_args&action=next&view_id=$view_id><img src=img/next.gif border=0></a>";

    if ($item->is_exercise()) {
        $bolt_ex->mode = $mode;
        $bolt_ex->index = 0;
        switch ($mode) {
        case BOLT_MODE_SHOW:
            echo "
                <form action=bolt_sched.php>
                <input type=hidden name=view_id value=$view_id>
                <input type=hidden name=course_id value=$course->id>
                <input type=hidden name=action value=answer>
            ";
            if ($refresh) {
                echo "
                    <input type=hidden name=refresh_id value=$refresh->id>
                ";
            }
            srand($view_id);
            require($item->filename);
            if (function_exists('bolt_divide')) bolt_divide();
            $next = "<input type=image src=img/next.gif value=OK></form>";
            break;
        case BOLT_MODE_ANSWER:
            require($item->filename);
            if (function_exists('bolt_divide')) bolt_divide();
            $score_pct = number_format($bolt_ex->score*100);
            echo "Score: $score_pct%";
            break;
        }
    } else {
        require_once($item->filename);
        if (function_exists('bolt_divide')) bolt_divide();
    }

    if ($repeat) {
        $avg = number_format($repeat->avg_score*100, 0);
        echo "<p>Score on this exercise set: $avg%";
        if ($repeat->flags & REVIEW) {
            //echo "<pre>";
            //print_r($repeat);
            //echo "</pre>";
            $name = urlencode($repeat->unit->name);
            $r = "<a href=bolt_sched.php?$url_args&action=review&view_id=$view_id&unit_name=$name>Review, then repeat exercises</a>";
            $links[] = $r;
        }
        if ($repeat->flags & REPEAT) {
            $r = "<a href=bolt_sched.php?$url_args&action=repeat&view_id=$view_id>Repeat exercises</a>";
            $links[] = $r;
        }
        if ($repeat->flags & NEXT) {
            $links[] = $next;
        }
    } else {
        $links[] = $next;
    }

    $up_link = "<a href=bolt_sched.php?$url_args&action=course_home&view_id=$view_id>Course home page</a>";
    show_nav($links, $up_link, $view_id);

    page_footer();

    if ($refresh) {
        $refresh->update("last_view_id=$view_id");
    } else {
        $e = new BoltEnrollment();
        $e->user_id = $user->id;
        $e->course_id = $course->id;
        $e->update("last_view_id=$view_id");
    }
}

// Show the student the results of an old exercise; no navigation items
//
function show_answer_page($iter, $score) {
    global $bolt_ex;

    $bolt_ex->mode = BOLT_MODE_ANSWER;
    $bolt_ex->index = 0;

    $item = $iter->item;
    page_header();
    $bolt_query_string = $item->query_string;
    require_once($item->filename);
    if (function_exists('bolt_divide')) bolt_divide();
    $score_pct = number_format($score*100);
    echo "Score: $score_pct%";
    page_footer();
}

function start_course() {
    global $user;
    global $course;
    global $course_doc;

    BoltEnrollment::delete($user->id, $course->id);
    $iter = new BoltIter($course_doc);
    $iter->at();

    $now = time();
    $mode = default_mode($iter->item);
    $view_id = create_view($iter, $mode, 0);
    BoltEnrollment::insert("(create_time, user_id, course_id, last_view_id) values ($now, $user->id, $course->id, $view_id)");
    show_item($iter, $view_id, 0, $mode);
}

function start_refresh() {
    global $course_doc;
    global $refresh;

    $xset_result = BoltXsetResult::lookup_id($refresh->xset_result_id);
    if (!$xset_result) error_page("Exercise set result not found");
    $view = BoltView::lookup_id($xset_result->view_id);
    if (!$view) error_page("view not found");
    $iter = new BoltIter($course_doc);
    $iter->decode_state($view->state);
    $iter->at();
    $xset = $iter->xset;
    if (!$xset || $xset->name != $xset_result->name) {
        error_page("missing exercise set");
    }
    $xset->restart($iter);
    $iter->at();
    $mode = default_mode($iter->item);
    $view_id = create_view($iter, $mode, 0);
    show_item($iter, $view_id, 0, $mode);
}

function show_next($iter, $view) {
    global $refresh, $user, $course;
    $iter->next();

    if ($refresh) {
        $iter->at();
        if (!$iter->xset) {
            // if we're doing a refresh and are no longer in an xset,
            // we must have finished the refresh
            //
            show_refresh_finished();
            $refresh->update('count=count+1');
            break;
        }
    }

    if ($iter->item) {
        $state = $iter->encode_state();
        $mode = default_mode($iter->item);
        $view_id = create_view($iter, $mode, $view->id);
        show_item($iter, $view_id, $view->id, $mode);
    } else {
        // course finished
        $iter->frac_done = 1;
        $fin_view_id = create_view($iter, BOLT_MODE_FINISHED, $view->id);
        $e = new BoltEnrollment();
        $e->user_id = $user->id;
        $e->course_id = $course->id;
        $e->update("last_view_id=$fin_view_id");
        show_finished_page($fin_view_id, $view->id);
    }
}

$user = get_logged_in_user();
BoltUser::lookup($user);
$course_id = get_int('course_id');
$refresh_id = get_int('refresh_id', true);
$refresh = null;
$url_args = "course_id=$course_id";
if ($refresh_id) {
    $refresh = BoltRefreshRec::lookup_id($refresh_id);
    if (!$refresh) error_page("No such refresh");
    if ($refresh->user_id != $user->id) error_page("Wrong user");
    if ($refresh->course_id != $course_id) error_page("Wrong course");
    $url_args .= "&refresh_id=$refresh_id";
}
$course = BoltCourse::lookup_id($course_id);
if (!$course) {
    error_page("no such course");
}
$view_id = get_int('view_id', true);
$action = get_str('action', true);
$course_doc = require_once($course->doc_file());

switch ($action) {
case 'start':
    if (info_incomplete($user)) {
        request_info($user, $course);
        exit();
    }
    if ($refresh) {
        start_refresh();
        exit();
    }
    $e = BoltEnrollment::lookup($user->id, $course_id);
    if ($e) {
        page_header();
        echo "You are already enrolled in $course->name.
            <p>
            Are you sure you want to start over from the beginning?
            <p>
        ";
        show_button(
            "bolt_sched.php?action=start_confirm&$url_args",
            "Yes",
            "Start this course from the beginning"
        );
        show_button(
            "bolt_sched.php?action=resume&$url_args",
            "Resume",
            "Resume course from current position"
        );
        page_footer();
        exit();
    }
    // fall through
case 'start_confirm':
    start_course();
    break;
case 'update_info':
    update_info();
    start_course();
    break;
case 'prev':
    $view = finalize_view($view_id, BOLT_ACTION_PREV);
    debug_show_state(unserialize($view->state), "Initial");
    if ($view->prev_view_id) {
        $view = BoltView::lookup_id($view->prev_view_id);
        $iter = new BoltIter($course_doc);
        $iter->decode_state($view->state);
        $iter->at();
        $mode = $view->mode;
        if ($mode == BOLT_MODE_ANSWER) {
            $v2 = BoltView::lookup_id($view->prev_view_id);
            $result = BoltResult::lookup_id($v2->result_id);
            srand($v2->id);
            $bolt_ex->score = $result->score;
            $bolt_ex->query_string = $result->response;
        }
        $view_id = create_view($iter, $mode, $view->prev_view_id);
        show_item($iter, $view_id, $view->prev_view_id, $mode);
    } else {
        error_page("At start of course");
    }
    break;
case 'next':            // "next" button in lesson or exercise answer page
    $view = finalize_view($view_id, BOLT_ACTION_NEXT);
    debug_show_state(unserialize($view->state), "Initial");

    $iter = new BoltIter($course_doc);
    $iter->decode_state($view->state);
    show_next($iter, $view);
    break;
case 'answer':          // submit answer in exercise
    $view = finalize_view($view_id, BOLT_ACTION_SUBMIT);
    debug_show_state(unserialize($view->state), "Initial");
    $iter = new BoltIter($course_doc);
    $iter->decode_state($view->state);
    $iter->at();

    debug_show_item($iter->item);
    $item = $iter->item;
    if (!$item->is_exercise()) {
        print_r($item);
        error_page("expected an exercise");
    }
    if ($view->item_name != $item->name) {
        error_page("unexpected name");
    }

    // compute the score

    $bolt_ex->query_string = $_SERVER['QUERY_STRING'];
    $bolt_ex->mode = BOLT_MODE_SCORE;
    $bolt_ex->index = 0;
    $bolt_ex->score = 0;
    $bolt_query_string = $item->query_string;
    srand($view_id);
    ob_start();     // buffer output to avoid showing exercise text
    require($item->filename);
    ob_end_clean();

    $bolt_ex->score /= $bolt_ex->index;

    if ($item->callback) {
        call_user_func(
            $item->callback, $bolt_ex->score, $bolt_ex->query_string
        );
    }

    // make a record of the result

    $qs = BoltDb::escape_string($_SERVER['QUERY_STRING']);
    $now = time();
    $result_id = BoltResult::insert(
        "(create_time, user_id, course_id, view_id, item_name, score, response)
        values ($now, $user->id, $course->id, $view->id, '$view->item_name', $bolt_ex->score, '$qs')"
    );
    $view->update("result_id=$result_id");

    // If this is part of an exercise set, call its callback function
    //
    $repeat = null;
    $xset = $iter->xset;
    if ($xset) {
        $is_last = $xset->xset_record_score(
            $iter, $bolt_ex->score, $view->id, $avg_score, $repeat
        );
        if ($repeat) $repeat->avg_score = $avg_score;
        if ($is_last) {
            // if the exercise set if finished, make or update DB records
            //
            if ($xset->callback) {
                call_user_func($xset->callback, $avg_score);
            }
            $now = time();
            $id = BoltXsetResult::insert("(create_time, user_id, course_id, name, score, view_id) values ($now, $user->id, $course->id, '$xset->name', $avg_score, $view_id)");
            $refresh_intervals = $xset->refresh;
            if ($refresh_intervals) {
                $refresh_rec = BoltRefreshRec::lookup(
                    "user_id=$user->id and course_id=$course->id and name='$xset->name'"
                );
                if ($refresh_rec) {
                    $count = $refresh_rec->count;
                    $n = count($refresh_intervals->intervals);
                    if ($count >= $n) {
                        $count = $n - 1;
                    }
                    $due_time = time() + $refresh_intervals->intervals[$count]*86400;
                    $refresh_rec->update("create_time=$now, xset_result_id=$id, due_time=$due_time");
                } else {
                    $due_time = time() + $refresh_intervals->intervals[0]*86400;
                    BoltRefreshRec::insert(
                        "(user_id, course_id, name, create_time, xset_result_id, due_time, count) values ($user->id, $course->id, '$xset->name', $now, $id, $due_time, 0)"
                    );
                }
            }
        }
    }

    // show the answer page

    if ($item->has_answer_page) {
        srand($view_id);
        $view_id = create_view($iter, BOLT_MODE_ANSWER, $view->id);
        show_item($iter, $view_id, $view->id, BOLT_MODE_ANSWER, $repeat);
    } else {
        show_next($iter, $view);
    }
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
    $bolt_ex->query_string = $result->response;
    show_answer_page($iter, $result->score);
    break;
case 'course_home':
    $view = finalize_view($view_id, BOLT_ACTION_COURSE_HOME);
    Header("Location: bolt.php");
    break;
case 'review':
    // user chose to do review then repeat an exercise set
    //
    $view = finalize_view($view_id, BOLT_ACTION_REVIEW);
    debug_show_state(unserialize($view->state), "Initial");
    $iter = new BoltIter($course_doc);
    $iter->decode_state($view->state);
    $iter->at();
    if (!$iter->xset) {
        echo "NO XSET"; exit;
    }
    $xset = $iter->xset;
    $unit_name = get_str('unit_name');
    $found = $xset->start_review($iter, $unit_name);
    if (!$found) {
        echo "REVIEW UNIT MISSING"; exit;
    }
    $iter->at();
    $mode = default_mode($iter->item);
    $view_id = create_view($iter, $mode, $view->id);
    show_item($iter, $view_id, $view->id, $mode);
    break;
case 'repeat':
    // user chose to repeat an exercise set
    //
    $view = finalize_view($view_id, BOLT_ACTION_REPEAT);
    debug_show_state(unserialize($view->state), "Initial");
    $iter = new BoltIter($course_doc);
    $iter->decode_state($view->state);
    $iter->at();
    if (!$iter->xset) {
        echo "NO XSET"; exit;
    }
    $xset = $iter->xset;
    $xset->restart($iter);
    $iter->at();
    $mode = default_mode($iter->item);
    $view_id = create_view($iter, $mode, $view->id);
    show_item($iter, $view_id, $view->id, $mode);
    break;
case 'resume':
    // user chose to resume a course or refresh
    //
    if ($refresh) {
        if ($refresh->last_view_id) {
            $view = BoltView::lookup_id($refresh->last_view_id);
        } else {
            start_refresh();
            exit();
        }
    } else {
        $view = null;
        $e = BoltEnrollment::lookup($user->id, $course_id);
        if ($e) {
            $view = BoltView::lookup_id($e->last_view_id);
        }
        if (!$view) {
            start_course();
            break;
        }
    }
    if ($view->mode == BOLT_MODE_FINISHED) {
        show_finished_page($view->id, $view->prev_view_id);
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
        $bolt_ex->query_string = $result->response;
        $bolt_ex->score = $result->score;
        $bolt_ex->index = 0;
        $view_id = create_view($iter, $mode, $view_orig->id);
        show_item($iter, $view_id, $view_orig->id, $mode);
    } else {
        $view_id = create_view($iter, $mode, $view->id);
        show_item($iter, $view_id, $view->id, $mode);
    }
    break;
case 'question':
    $view = finalize_view($view_id, BOLT_ACTION_QUESTION);
    debug_show_state(unserialize($view->state), "Initial");
    $now = time();
    $question = BoltDb::escape_string(get_str('question'));
    BoltQuestion::insert("(create_time, user_id, course_id, name, mode, question, state) values ($now, $user->id, $course->id, '$view->item_name', $view->mode, '$question', 0)");
    page_header();
    echo "
        Thanks; we have recorded your question.
        Questions help us improve this course.
        We aren't able to individually respond to all questions.
        Responses are delivered as private messages.
        <p>
        <a href=bolt_sched.php?$url_args&action=resume>Resume course</a>
    ";
    page_footer();
    break;
default:
    error_page("unknown action: $action");
}

?>
