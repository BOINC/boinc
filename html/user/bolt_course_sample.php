<?php

return new BoltSeq(
    'course',
    array(
        new BoltLesson('lesson 1', 'bolt_sample_lesson1.php'),
        new BoltLesson('lesson 2', 'bolt_sample_lesson2.php'),
        new BoltExercise('exercise 1', 'bolt_sample_exercise1.php'),
        new BoltSeq(
            'inner seq',
            array(
                new BoltLesson('lesson 3', 'bolt_sample_lesson3.php')
            )
        )
    )
);

?>
