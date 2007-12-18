<?php

$bolt_ex_mode = 0;
$bolt_ex_index = 0;
$bolt_ex_state = 0;
$bolt_ex_score = 0;

function bolt_exclusive_choice($choices) {
    global $bolt_ex_mode;   // input
    global $bolt_ex_index;  // input
    global $bolt_ex_score;  // incremental output if SCORE
    global $bolt_ex_response;   // output if SCORE

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
        $key = "q_$bolt_ex_index";
        if (array_key_exists($key, $_GET)) {
            $response = $_GET[$key];
            if ($choices[$response] == $right_ans) {
                $bolt_ex_score += 1;
            }
            $bolt_ex_response = "$bolt_ex_index: $choices[$response]";
        } else {
            $bolt_ex_response = "";
        }
        break;
    case BOLT_MODE_ANSWER:
        $right_ans = $choices[0];
        shuffle($choices);
        $key = "q_$bolt_ex_index";
        if (array_key_exists($key, $_GET)) {
            $response = $_GET[$key];
        } else {
            $response = -1;
        }
        $i = 0;
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

function bolt_inclusive_choice($choices) {
    global $bolt_ex_mode;   // input
    global $bolt_ex_index;  // input
    global $bolt_ex_score;  // incremental output if SCORE
    global $bolt_ex_response;   // output if SCORE

    switch ($bolt_ex_mode) {
    case BOLT_MODE_SHOW:
        shuffle($choices);
        $i = 0;
        start_table();
        foreach ($choices as $choice) {
            $c = $choice[0];
            row2("<input name=q_".$bolt_ex_index."_$i type=checkbox>", $c);
            $i++;
        }
        end_table();
        break;
    case BOLT_MODE_SCORE:
        $i = 0;
        $n = count($choices);
        $score = 0;
        shuffle($choices);
        foreach ($choices as $choice) {
            $key = "q_".$bolt_ex_index."_$i";
            $response = array_key_exists($key, $_GET);
            $r = $choice[1];
            $correct = ($r && $response) || (!$r && !$response);
            if ($correct) $score += 1./$n;
            $i++;
        }
        $bolt_ex_response = "$bolt_ex_index: $choices[$response]";
        $bolt_ex_score += $score;
        break;
    case BOLT_MODE_ANSWER:
        $i = 0;
        $n = count($choices);
        shuffle($choices);
        start_table();
        table_header("Choice", "Correct?", "Your answer");
        foreach ($choices as $choice) {
            $c = $choice[0];
            $key = "q_".$bolt_ex_index."_$i";
            $response = array_key_exists($key, $_GET);
            $r = $choice[1];
            $correct = ($r && $response) || (!$r && !$response);
            table_row($c, $r?"yes":"no", $response?"yes":"no");
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
    global $bolt_ex_score;  // incremental output if SCORE

    switch ($bolt_ex_mode) {
    case BOLT_MODE_SHOW:
        echo "<img src=$img";
        break;
    }
}

?>
