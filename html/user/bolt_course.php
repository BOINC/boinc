<?php

require_once("../inc/util.inc");

$user = get_logged_in_user();

function phase_name($phase) {
    switch ($phase) {
    case BOLT_PHASE_STUDY: return "study";
    case BOLT_PHASE_REVIEW: return "review";
    case BOLT_PHASE_REFRESH: return "refresh";
    default: return "unknown phase: $phase";
    }
}

function mode_name($mode) {
    switch ($mode) {
    case BOLT_MODE_LESSON: return "lesson";
    case BOLT_MODE_SHOW: return "exercise";
    case BOLT_MODE_ANSWER: return "answer page";
    case BOLT_MODE_FINISHED: return "course completed";
    default: return "unknown mode: $mode";
    }
}

function action_name($action) {
    switch ($action) {
    case BOLT_ACTION_NONE: return "None";
    case BOLT_ACTION_NEXT: return "Next";
    case BOLT_ACTION_PREV: return "Previous";
    case BOLT_ACTION_SUBMIT: return "Submit";
    case BOLT_ACTION_QUESTION: return "Question";
    case BOLT_ACTION_COURSE_HOME: return "Course home";
    default: return "unknown: $action";
    }
}

function show_view($view) {
    if ($view->end_time) {
        $d = $view->end_time - $view->start_time;
        $dur = "$d seconds";
    } else {
        $dur = "---";
    }

    if ($view->result_id) {
        $result = BoltResult::lookup_id($view->result_id);
        $qs = str_replace("action=answer", "action=answer_page", $result->response);
        $x = "<br>Score: $result->score
            <br><a href=bolt_sched.php?$qs>Answer page</a>";
    }
    echo "<tr>
        <td valign=top>$view->id</td>
        <td valign=top>".time_str($view->start_time)."</td>
        <td valign=top>$dur</td>
        <td valign=top>$view->item_name</td>
        <td valign=top>".mode_name($view->mode)." $x</td>
    ";
        //<td valign=top>".phase_name($view->phase)."</td>
    echo "
        <td valign=top>".action_name($view->action)."</td>
        </tr>
    ";
}

function show_views() {
    global $user;
    global $course;

    $views = BoltView::enum("user_id=$user->id and course_id=$course->id order by id desc");
    start_table();

    table_header("ID", "Time", "Duration", "Item", "Mode",
            // "Phase",
            "Action");
    foreach ($views as $view) {
        show_view($view);
    }
    end_table();
}

function show_refresh($r) {
    echo "<tr>
        <td>".time_str($r->create_time)."</td>
        <td>$r->name
            <a href=bolt_sched.php?course_id=$r->course_id&refresh_id=$r->id&action=start>Start</a>
            <a href=bolt_sched.php?course_id=$r->course_id&refresh_id=$r->id&action=resume>Resume</a>
        </td>
        <td>".time_str($r->due_time)."</td>
        </tr>
    ";
}

function show_refreshes() {
    global $user;
    global $course;

    $refreshes = BoltRefreshRec::enum("user_id=$user->id and course_id=$course->id");
    start_table();
    table_header("Created", "Unit", "Due");
    foreach ($refreshes as $r) {
        show_refresh($r);
    }
    end_table();
}

require_once("../inc/bolt_db.inc");

$course_id = get_int('course_id');
$course = BoltCourse::lookup_id($course_id);
if (!$course) error_page("No such course");
page_head("Your history in $course->name");

show_views();
show_refreshes();

echo "
    <a href=bolt_sched.php?course_id=$course_id&action=resume>Resume course</a>
    <p>
";

page_tail();
?>
