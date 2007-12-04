<?php

require_once("../inc/bolt.inc");

function part2() {
    return sequence(
        name('inner seq'),
        lesson(
            name('lesson 3'),
            filename('bolt_sample_lesson3.php')
        )
    );
}

function refresh() {
    return array(0, 14, 28);
}

function basic_review() {
    return sequence(
        lesson(
            name('lesson 1'),
            filename('bolt_sample_lesson1.php')
        ),
        lesson(
            name('lesson 2'),
            filename('bolt_sample_lesson2.php')
        )
    );
}

function int_review() {
    return lesson(
        name('lesson 2'),
        filename('bolt_sample_lesson2.php')
    );
}

return sequence(
    name('course'),
    lesson(
        name('lesson 1'),
        filename('bolt_sample_lesson1.php')
    ),
    lesson(
        name('lesson 2'),
        filename('bolt_sample_lesson2.php')
    ),
    reviewed_exercise(
        exercise(
            name('exercise 1'),
            filename('bolt_sample_exercise1.php')
        ),
        refresh_interval(refresh()),
        review(.3, basic_review()),
        review(.7, int_review()),
    ),
    part2()
);

?>
