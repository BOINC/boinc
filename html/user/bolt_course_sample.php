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

require_once("../inc/bolt.inc");

function part2() {
    return sequence(
        name('inner seq'),
        lesson(
            name('lesson 3'),
            filename('bolt_sample_lesson.php?n=3')
        )
    );
}

function basic_review() {
    return sequence(
        lesson(
            name('lesson 1'),
            filename('bolt_sample_lesson.php?n=1')
        ),
        lesson(
            name('lesson 2'),
            filename('bolt_sample_lesson.php?n=2')
        )
    );
}

function int_review() {
    return lesson(
        name('lesson 2'),
        filename('bolt_sample_lesson.php?n=2')
    );
}

function my_rand($student, $unit) {
    return rand();
}

function sample_select() {
    return select(
        name('sample select'),
        valuator('my_rand'),
        lesson(
            name('lesson 1'),
            filename('bolt_sample_lesson.php?n=1')
        ),
        lesson(
            name('lesson 2'),
            filename('bolt_sample_lesson.php?n=2')
        ),
        lesson(
            name('lesson 3'),
            filename('bolt_sample_lesson.php?n=3')
        )
    );
}

function sample_random() {
    return random(
        name('first lessons'),
        number(2),
        lesson(
            name('lesson 1'),
            filename('bolt_sample_lesson.php?n=1')
        ),
        lesson(
            name('lesson 2'),
            filename('bolt_sample_lesson.php?n=2')
        ),
        lesson(
            name('lesson 3'),
            filename('bolt_sample_lesson.php?n=3')
        )
    );
}

function xset_with_review() {
    return exercise_set(
        name('exercise set 1'),
        number(2),
        exercise(
            name('exercise 1'),
            filename('bolt_sample_exercise.php?n=1')
        ),
        exercise(
            name('exercise 2'),
            filename('bolt_sample_exercise.php?n=2')
        ),
        exercise(
            name('exercise 3'),
            filename('bolt_sample_exercise.php?n=3')
        ),
        repeat(.3, basic_review(), REVIEW),
        repeat(.7, int_review(), REVIEW|REPEAT),
        repeat(1, null, REPEAT|NEXT),
        refresh(array(7, 14, 28))
    );
}

function sample_xset() {
    return exercise_set(
        name('sample exercise set'),
        number(1),
        exercise(
            name('exercise 1'),
            filename('bolt_sample_exercise.php?n=1')
        )
    );
}

return sequence(
    name('course'),
    // sample_random(),
    // xset_with_review(),
    sample_select(),
    sample_xset(),
    part2()
);

?>
