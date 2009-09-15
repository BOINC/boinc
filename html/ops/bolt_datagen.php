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

// Generate synthetic usage data for a Bolt course (for debugging analytics),
// or to clear out existing data.
// See comments below for how to use this.

require_once("../inc/bolt_db.inc");
require_once("../inc/bolt_util.inc");
require_once("../inc/bolt.inc");
require_once("../inc/bolt_cat.inc");
require_once("../inc/util_ops.inc");

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

$rand_max = (float)getrandmax();

// uniform random from (0, 1)
//
function frand() {
    global $rand_max;
    return (float)rand()/$rand_max;
}

// uniform random from (a-d, a+d)
//
function urand($a, $d) {
    $f = frand()*2 - 1;
    return $a + $f*$d;
}

// Generate data for unit comparison.
// Random N students, randomly assigned to select alternatives.
// Score is A +- B.
// Score for alternative X is C +- D
//
function compare_gen(
    $select_name,       // name of select unit
    $xset_name,         // name of exercise set
    $n,                 // number of records to create
    $a1, $d1,           // avg and dev of default score
    $x,                 // name of chosen alternative
    $a2, $d2            // avg and dev of score for chosen alternative
) {
    global $course;
    $top_unit = require_once($course->doc_file());
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
        BoltSelectFinished::insert("(user_id, course_id, end_time, name, selected_unit) values ($uid, $course->id, $t1, '$select_name', '$child->name')");

        if ($child->name == $x) {
            $score = urand($a2, $d2);
        } else {
            $score = urand($a1, $d1);
        }
        BoltXsetResult::insert("(create_time, user_id, course_id, start_time, end_time, name, score) values ($t2, $uid, $course->id, $t2, $t2, '$xset_name', $score)");
    }
}

// generate data for course map.
// $n: number of students
// $sb1: default student behavior; components:
//      $attr: attrition rate
//      $score_mean
//      $score_dev
//      $time_mean
//      $time_dev
// $sb2: student behavior for selected group
// $sel, $sel_cat: categorization and category of selected group
//
function map_gen($n, $sb1, $sb2, $sel, $sel_cat) {
    global $course;

    $top_unit = require_once($course->doc_file());
    for ($i=0; $i<$n; $i++) {
        $uid = random_student();
        $user = BoincUser::lookup_id($uid);
        BoltUser::lookup($user);
        $sb = ($sel->categorize($user) == $sel_cat)?$sb2:$sb1;
        map_gen_uid($uid, $top_unit, $sb);
    }
}

function map_gen_uid($uid, $unit, $sb) {
    global $course, $now;
    $class = get_class($unit);
    switch ($class) {
    case 'BoltExercise':
        return map_gen_ex($uid, $unit, $sb);
    case 'BoltLesson':
        return map_gen_lesson($uid, $unit, $sb);
    default:
        $ret = false;
        foreach ($unit->units as $u) {
            $ret = map_gen_uid($uid, $u, $sb);
            if (!$ret) break;
        }
        if ($ret && $class == 'BoltExerciseSet') {
            $score = urand($sb->score_mean, $sb->score_dev);
            BoltXsetResult::insert("(create_time, user_id, course_id, name, score) values ($now, $uid, $course->id, '$unit->name', $score)");
        }
        return $ret;
    }
}

function map_gen_ans($uid, $unit, $sb) {
    global $course, $now;
    if (frand() < $sb->attr) {
        $action = BOLT_ACTION_NONE;
        $t = $now;
    } else {
        $action = BOLT_ACTION_NEXT;
        $t = $now + urand($sb->time_mean, $sb->time_dev);
    }
    $mode = BOLT_MODE_ANSWER;
    BoltView::insert("(user_id, course_id, item_name, mode, action, start_time, end_time) values ($uid, $course->id, '$unit->name', $mode, $action, $now, $t)");
    return ($action == BOLT_ACTION_NEXT);
}

function map_gen_ex($uid, $unit, $sb) {
    global $course, $now;
    if (frand() < $sb->attr) {
        $mode = BOLT_MODE_SHOW;
        $action = BOLT_ACTION_NONE;
        BoltView::insert("(user_id, course_id, item_name, mode, action, start_time) values ($uid, $course->id, '$unit->name', $mode, $action, $now)");
        return false;
    } else {
        $t = $now + urand($sb->time_mean, $sb->time_dev);
        $mode = BOLT_MODE_SHOW;
        $action = BOLT_ACTION_NEXT;
        BoltView::insert("(user_id, course_id, item_name, mode, action, start_time, end_time) values ($uid, $course->id, '$unit->name', $mode, $action, $now, $t)");
        $score = urand($sb->score_mean, $sb->score_dev);
        BoltResult::insert("(create_time, user_id, course_id, item_name, score) values ($now, $uid, $course->id, '$unit->name', $score)");
        return map_gen_ans($uid, $unit, $sb);
    }
}

function map_gen_lesson($uid, $unit, $sb) {
    global $course, $now;
    if (frand() < $sb->attr) {
        $action = BOLT_ACTION_NONE;
        $t = $now;
    } else {
        $action = BOLT_ACTION_NEXT;
        $t = $now + urand($sb->time_mean, $sb->time_dev);
    }
    $mode = BOLT_MODE_LESSON;
    BoltView::insert("(user_id, course_id, item_name, action, mode, start_time, end_time) values ($uid, $course->id, '$unit->name', $action, $mode, $now, $t)");
    return ($action == BOLT_ACTION_NEXT);
}

function clear() {
    global $course;
    $db = BoltDb::get();
    $db->do_query("delete from DBNAME.bolt_view where course_id=$course->id");
    $db->do_query("delete from DBNAME.bolt_result where course_id=$course->id");
    $db->do_query("delete from DBNAME.bolt_xset_result where course_id=$course->id");
    $db->do_query("delete from DBNAME.bolt_select_finished where course_id=$course->id");
}

// put your course ID here:

$course = BoltCourse::lookup_id(4);
if (!$course) error_page("no such course");
$now = time();

if (1) {
    clear();
}

if (1) {
    compare_gen(
        'Conifer/deciduous alternative',        // select name
        'Intro exercises',      // xset name
        50,                     // # of records to create
        .5, .3,                 // mean and dev of default score
        'conifer_decid2.php',  // name of chosen alternative
        .8, .2                  // mean and dev of score for that alternative
    );
}

if (0) {
    $sb1->attr = 0.1;
    $sb1->time_mean = 20;
    $sb1->time_dev = 10;
    $sb1->score_mean = 0.8;
    $sb1->score_dev = 0.2;

    $sb2->attr = 0.3;
    $sb2->time_mean = 30;
    $sb2->time_dev = 10;
    $sb2->score_mean = 0.5;
    $sb2->score_dev = 0.2;

    map_gen(50, $sb1, $sb2, new CatSex(), "Female");
}

?>
