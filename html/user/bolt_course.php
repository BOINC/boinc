<?php

require_once("../inc/util.inc");

$user = get_logged_in_user();

function mode_name($mode) {
    switch ($mode) {
    case BOLT_MODE_LESSON: return "lesson";
    case BOLT_MODE_SHOW: return "exercise";
    case BOLT_MODE_ANSWER: return "exercise answer";
    case BOLT_MODE_FINISHED: return "course completed";
    default: return "unknown: $mode";
    }
}

function action_name($action) {
    switch ($action) {
    case BOLT_ACTION_NONE: return "None";
    case BOLT_ACTION_NEXT: return "Next";
    case BOLT_ACTION_PREV: return "Previous";
    case BOLT_ACTION_SUBMIT: return "Submit";
    case BOLT_ACTION_QUESTION: return "Question";
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
        <td valign=top>".time_str($view->start_time)."</td>
        <td valign=top>$dur</td>
        <td valign=top>$view->item_name</td>
        <td valign=top>".mode_name($view->mode)." $x</td>
        <td valign=top>".action_name($view->action)."</td>
        </tr>
    ";
}

require_once("../inc/bolt_db.inc");

$course_id = get_int('course_id');
$course = BoltCourse::lookup_id($course_id);
page_head("Your history in $course->name");

$views = BoltView::enum("user_id=$user->id and course_id=$course_id order by id desc");
start_table();

table_header("Time", "Duration", "Item", "Type", "Action");
foreach ($views as $view) {
    show_view($view);
}
end_table();
echo "
    <a href=bolt_sched.php?course_id=$course_id>Resume course</a>
    <p>
";

page_tail();
?>
