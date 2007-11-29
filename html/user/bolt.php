<?php

require_once("../inc/bolt_db.inc");
require_once("../inc/util.inc");

page_head("Courses");

$user = get_logged_in_user(true);

$courses = BoltCourse::enum();
start_table();
table_header(
    "Course<br><span class=note>Click to start, resume, or review</span>
    ", "Status"
);
foreach ($courses as $course) {
    $e = $user?BoltEnrollment::lookup($user->id, $course->id):null;
    $start = date_str($e->create_time);
    $ago = time_diff(time() - $e->last_view);
    if ($e) {
        $pct = number_format($e->fraction_done*100, 0);
        $status = "Started $start; last visit $ago ago; $pct% done";
    } else {
        $status = "Not started";
    }
    row2("<a href=bolt_sched.php?course_id=$course->id&action=start>$course->name</a>
        <br><span class=note>$course->description</span>",
        $status
    );
}
end_table();
page_tail();

?>
