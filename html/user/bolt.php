<?php

require_once("../inc/bolt_db.inc");
require_once("../inc/util.inc");

page_head("Courses");

$user = get_logged_in_user(true);

$courses = BoltCourse::enum();
foreach ($courses as $course) {
    $e = BoltEnrollment::lookup($user->id, $course->id);
    echo "$course->name <a href=bolt_sched.php?course_id=$course->id&action=start>start</a>
    ";
    if ($e) {
        echo "<a href=bolt_sched.php?course_id=$course->id>resume</a>
        ";
    }
    echo "
        <dd>$course->description
    ";
}
page_tail();

?>
