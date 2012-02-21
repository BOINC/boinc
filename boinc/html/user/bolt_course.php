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
        $score = number_format($result->score*100);
        $x = "<br>Score: $score%
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

    $views = BoltView::enum("user_id=$user->id and course_id=$course->id order by id");
    start_table();

    table_header("ID", "Time", "Duration", "Item", "Mode",
            // "Phase",
            "Action");
    foreach ($views as $view) {
        show_view($view);
    }
    end_table();
}

require_once("../inc/bolt_db.inc");
require_once("../inc/bolt_util.inc");

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
