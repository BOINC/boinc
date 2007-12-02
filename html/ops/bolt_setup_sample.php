<?php

require_once("../inc/bolt_db.inc");

$short_name = 'test_course';
$name = 'Test course';
$description = 'This course is a demonstration of Bolt';
$doc_file = 'bolt_course_sample.php';
$now = time();

if (BoltCourse::insert("(create_time, short_name, name, description, doc_file) values ($now, '$short_name', '$name', '$description', '$doc_file')")) {
    echo "all done\n";
} else {
    echo "database error\n";
}

?>
