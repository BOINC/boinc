<?php

require_once("../inc/bolt_db.inc");

$c = new BoltCourse();
$c->short_name = 'test_course';
$c->name = 'A test course';
$c->description = 'This course is a demonstration of Bolt';
$c->doc_file = 'bolt_course_sample.php';

if ($c->insert()) {
    echo "all done\n";
} else {
    echo "database error\n";
}

?>
