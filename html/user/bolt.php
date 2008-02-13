<?php

require_once("../inc/bolt_db.inc");
require_once("../inc/util.inc");

page_head("Courses");

$user = get_logged_in_user(false);
if ($user) {
    BoltUser::lookup($user);
}

$courses = BoltCourse::enum();
start_table();
table_header(
    "Course", "Status"
);
foreach ($courses as $course) {
    if ($course->hidden && !($user->bolt->flags&BOLT_FLAG_SHOW_ALL)) {
        continue;
    }
    $e = $user?BoltEnrollment::lookup($user->id, $course->id):null;
    if ($e) {
        $start = date_str($e->create_time);
        $view = BoltView::lookup_id($e->last_view_id);
        $ago = time_diff(time() - $view->start_time);
        $pct = number_format($view->fraction_done*100, 0);
        $status = "Started $start
            <br>Last visit: $ago ago
            <br>$pct% done
        ";
        if ($view->fraction_done < 1) {
            $status .= "<br><a href=bolt_sched.php?course_id=$course->id&action=resume>Resume</a>
            ";
        }
        $status .= "<br><a href=bolt_sched.php?course_id=$course->id&action=start>Restart</a>
            <br><a href=bolt_course.php?course_id=$course->id>History</a>
        ";
    } else {
        $status = "
            <a href=bolt_sched.php?course_id=$course->id&action=start>Start</a>
        ";
    }
    row2("<b>$course->name</b>
        <br><span class=note>$course->description</span>",
        $status
    );
}
end_table();
page_tail();

?>
