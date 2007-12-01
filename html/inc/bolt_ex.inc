<?php

define('BOLT_MODE_SHOW', 0);
define('BOLT_MODE_SCORE', 1);
define('BOLT_MODE_ANSWER', 2);

$bolt_ex_mode = 0;
$bolt_ex_index = 0;
$bolt_ex_state = 0;
$bolt_ex_score = 0;

function bolt_new_seed() {
    return (float)microtime() * 1000000;
}

function bolt_exclusive_choice($choices) {
    global $bolt_ex_mode;   // input
    global $bolt_ex_index;  // input
    global $bolt_ex_state;  // output if SHOW, else input
    global $bolt_ex_score;  // output if SCORE

    switch ($bolt_ex_mode) {
    case BOLT_MODE_SHOW:
        // Shuffle the answers;
        // record the RNG seed so that we can do the same shuffle later
        //
        $seed = bolt_new_seed();
        srand($seed);
        shuffle($choices);
        $i = 0;
        foreach ($choices as $choice) {
            echo "<br><input name=q_$bolt_ex_index type=radio value=$i> $choice\n";
            $i++;
        }
        $bolt_ex_state = $seed;
        break;
    case BOLT_MODE_SCORE:
        $seed = $bolt_ex_state;
        srand($seed);
        $right_ans = $choices[0];
        shuffle($choices);
        $response = $_GET["q_$bolt_ex_index"];
        if ($choices[$response] == $right_ans) {
            $bolt_ex_score = 1;
        } else {
            $bolt_ex_score = 0;
        }
        echo "FOO";
        break;
    case BOLT_MODE_ANSWER:
        $seed = $bolt_ex_state;
        srand($seed);
        $right_ans = $choices[0];
        shuffle($choices);
        $response = $_GET["q_$bolt_ex_index"];
        $i = 0;
        foreach ($choices as $choice) {
            echo "<br>$choice";
            if ($response == $i) echo "You chose this";
            if ($choice == $right_ans) echo "Right answer";
            $i++;
        }
        break;
    }
    $bolt_ex_index++;
}

function bolt_image_rect($img, $rect) {
    global $bolt_ex_mode;   // input
    global $bolt_ex_index;  // input
    global $bolt_ex_state;  // output if SHOW, else input
    global $bolt_ex_score;  // output if SCORE

    switch ($bolt_ex_mode) {
    case BOLT_MODE_SHOW:
        echo "<img src=$img";
        break;
    }
}

?>
