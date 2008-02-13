<?php

// Bolt exercise API

$bolt_ex_mode = 0;
$bolt_ex_index = 0;
$bolt_ex_state = 0;
$bolt_ex_score = 0;
$bolt_ex_query_string = "";

function exclusive_choice($choices) {
    global $bolt_ex_mode;   // input
    global $bolt_ex_index;  // input
    global $bolt_ex_score;  // incremental output if SCORE
    global $bolt_ex_query_string;

    parse_str($bolt_ex_query_string);

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
        if (isset($$key)) {
            $response = $$key;
            if ($choices[$response] == $right_ans) {
                $bolt_ex_score += 1;
            }
        }
        break;
    case BOLT_MODE_ANSWER:
        $right_ans = $choices[0];
        shuffle($choices);
        $key = "q_$bolt_ex_index";
        if (isset($$key)) {
            $response = $$key;
        } else {
            $response = -1;
        }
        $i = 0;
        start_table();
        foreach ($choices as $choice) {
            $x = "<td><br></td>";
            if ($response == $i) {
                if ($choice == $right_ans) {
                    $x = "<td bgcolor=#88ff88>Right</a>";
                } else {
                    $x = "<td bgcolor=#ff8888>You chose this answer</a>";
                }
            } else {
                if ($choice == $right_ans) {
                    $x = "<td>Right answer</td>";
                }
            }
            echo "<tr><td width=50% class=fieldname>$choice $x </tr>";
            $i++;
        }
        end_table();
        break;
    }
    $bolt_ex_index++;
}

function inclusive_choice($choices) {
    global $bolt_ex_mode;   // input
    global $bolt_ex_index;  // input
    global $bolt_ex_score;  // incremental output if SCORE
    global $bolt_ex_query_string;

    parse_str($bolt_ex_query_string);

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
            $response = isset($$key);
            $r = $choice[1];
            $correct = ($r && $response) || (!$r && !$response);
            if ($correct) $score += 1./$n;
            $i++;
        }
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
            $response = isset($$key);
            $r = $choice[1];
            $correct = ($r && $response) || (!$r && !$response);
            $color = $correct?"#88ff88":"#ff8888";
            table_row($c, $r?"yes":"no",
                array($response?"yes":"no", "bgcolor=$color")
            );
            $i++;
        }
        end_table();
        break;
    }
    $bolt_ex_index++;
}

function image_rect($img, $rect) {
    global $bolt_ex_mode;   // input
    global $bolt_ex_index;  // input
    global $bolt_ex_state;  // output if SHOW, else input
    global $bolt_ex_score;  // incremental output if SCORE

    parse_str($bolt_ex_query_string);

    switch ($bolt_ex_mode) {
    case BOLT_MODE_SHOW:
        echo "<img src=$img";
        break;
    }
}

?>
