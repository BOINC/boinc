<?php

// generate synthetic usage data for a Bolt course (for debugging analytics)

require_once("../inc/bolt_db.inc");
require_once("../inc/bolt_util.inc");

// generate a random student
//
function random_student() {
    $email = "test_".rand();
    $uid = BoincUser::insert("(email_addr) values ('$email')");
    if (!$uid) {
        echo "can't add user";
        exit();
    }
    $sex = rand(1,2);
    $birth_year = rand(1938, 1998);
    BoltUser::insert("(user_id, sex, birth_year) values ($uid, $sex, $birth_year)");
    return $uid;
}

function rand_score($a, $d) {
    $f = (float)rand()/(float)getrandmax();
    $f = $f*2 - 1;
    return $a + $f*$d;
}

// Generate data for unit comparison.
// Random N students, randomly assigned to select alternatives.
// Score is A +- B.
// Score for alternative X is C +- D
//
function compare_gen(
    $course_id,
    $select_name,       // name of select unit
    $xset_name,         // name of exercise set
    $n,                 // number of records to create
    $a1, $d1,           // avg and dev of default score
    $x,                 // name of chosen alternative
    $a2, $d2            // avg and dev of score for chosen alternative
) {

    $course = BoltCourse::lookup_id($course_id);
    if (!$course) error_page("no such course");
    $top_unit = require_once("../user/$course->doc_file");
    $select_unit = lookup_unit($top_unit, $select_name);
    if (!$select_unit) error_page("no such select unit");
    if (!lookup_unit($top_unit, $xset_name)) error_page("no such xset");
    $m = count($select_unit->units);
    for ($i=0; $i<$n; $i++) {
        $uid = random_student();
        $t1 = time();
        $t2 = $t1+1;

        $j = rand(0, $m-1);
        $child = $select_unit->units[$j];
        BoltSelectFinished::insert("(user_id, course_id, end_time, name, selected_unit) values ($uid, $course_id, $t1, '$select_name', '$child->name')");

        if ($child->name == $x) {
            $score = rand_score($a2, $d2);
        } else {
            $score = rand_score($a1, $d1);
        }
        BoltXsetResult::insert("(create_time, user_id, course_id, start_time, end_time, name, score) values ($t2, $uid, $course_id, $t2, $t2, '$xset_name', $score)");
    }
}

compare_gen(
    2, 'sample select', 'exercise set 1', 10, .5, .1, 'lesson 1', .8, .1
);

?>
