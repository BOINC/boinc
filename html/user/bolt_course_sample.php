<?php

function part2() {
    return Sequence(
        name('inner seq'),
        Lesson(
            name('lesson 3'),
            file('bolt_sample_lesson3.php')
        )
    )
}

function refresh($n) {
    switch ($n) {
    case 0: return 7;
    case 1: return 14;
    }
    return 28;
}

function basic_review() {
    return Sequence(
        Lesson(
            name('lesson 1'),
            file('bolt_sample_lesson1.php')
        ),
        Lesson(
            name('lesson 2'),
            file('bolt_sample_lesson2.php')
        )
    );
}

function int_review() {
    return Lesson(
        name('lesson 2'),
        file('bolt_sample_lesson2.php')
    )
}

return Sequence(
    name('course'),
    Lesson(
        name('lesson 1'),
        file('bolt_sample_lesson1.php')
    ),
    Lesson(
        name('lesson 2'),
        file('bolt_sample_lesson2.php')
    )
    Exercise(
        name('exercise 1'),
        refresh_interval(refresh),
        review(.3, basic_review),
        review(.7, int_review),
        file('bolt_sample_exercise1.php')

    )
    part2()
);

?>
