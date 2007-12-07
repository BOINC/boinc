<?php

$bolt_ex_mode = 0;
$bolt_ex_index = 0;
$bolt_ex_state = 0;
$bolt_ex_score = 0;

function bolt_exclusive_choice($choices) {
    global $bolt_ex_mode;   // input
    global $bolt_ex_index;  // input
    global $bolt_ex_score;  // output if SCORE
    global $bolt_ex_response;   // output if SCORE

    echo "mode: $bolt_ex_mode";
    switch ($bolt_ex_mode) {
    case BOLT_MODE_SHOW:
        shuffle($choices);
        $i = 0;
        start_table();
        foreach ($choices as $choice) {
            row2($choice, "<input name=q_$bolt_ex_index type=radio value=$i>");
            $i++;
        }
        end_table();
        break;
    case BOLT_MODE_SCORE:
        $right_ans = $choices[0];
        shuffle($choices);
        $response = $_GET["q_$bolt_ex_index"];
        if ($choices[$response] == $right_ans) {
            $bolt_ex_score = 1;
        } else {
            $bolt_ex_score = 0;
        }
        $bolt_ex_response = "$bolt_ex_index: $choices[$response]";
        break;
    case BOLT_MODE_ANSWER:
        $right_ans = $choices[0];
        shuffle($choices);
        $response = $_GET["q_$bolt_ex_index"];
        $i = 0;
        echo "response: $response";
        start_table();
        foreach ($choices as $choice) {
            $x = "";
            if ($response == $i) $x .= "(You chose this) ";
            if ($choice == $right_ans) $x .=  "(Right answer)";
            row2($choice, $x);
            $i++;
        }
        end_table();
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
