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

// Bolt exercise API

// The following is a global var accessed by exercise functions.
//
$bolt_ex = null;
$bolt_ex->mode = 0;     // input: SHOW/SCORE/ANSWER
$bolt_ex->index = 0;    // input: sequence of this exercise in file
$bolt_ex->score = 0;    // input/output: cumulative score (if mode = SCORE)
$bolt_ex->weight = 0;   // input/output: cumulative weight
$bolt_ex->query_string = "";    // user's response (if SCORE or ANSWER)

function weight($w) {
    return array('weight', $w);
}

function exclusive_choice() {
    global $bolt_ex;
    $weight = 1;

    $choices = array();
    $args = func_get_args();
    foreach ($args as $arg) {
        if (is_string($arg)) {
            $choices[] = $arg;
        } else if (is_array($arg)) {
            switch ($arg[0]) {
            case 'weight': $weight = $arg[1]; break;
            default: echo "bad arg to exclusive_choice()";
            }
        } else {
            echo "bad arg to exclusive_choice()";
        }
    }

    parse_str($bolt_ex->query_string);

    switch ($bolt_ex->mode) {
    case BOLT_MODE_SHOW:
        shuffle($choices);
        $i = 0;
        start_table();
        foreach ($choices as $choice) {
            row2($choice, "<input name=q_$bolt_ex->index type=radio value=$i>");
            $i++;
        }
        end_table();
        break;
    case BOLT_MODE_SCORE:
        $right_ans = $choices[0];
        shuffle($choices);
        $key = "q_$bolt_ex->index";
        if (isset($$key)) {
            $response = $$key;
            if ($choices[$response] == $right_ans) {
                $bolt_ex->score += 1;
            }
        }
        $bolt_ex->weight += $weight;
        break;
    case BOLT_MODE_ANSWER:
        $right_ans = $choices[0];
        shuffle($choices);
        $key = "q_$bolt_ex->index";
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
    $bolt_ex->index++;
}

function inclusive_choice() {
    global $bolt_ex;
    $weight = 1;

    $choices = array();
    $args = func_get_args();
    foreach ($args as $arg) {
        if (is_array($arg)) {
            $choices[] = $arg;
        } else if (is_object($arg)) {
            if (get_class($arg) == 'BoltWeight') {
                $weight = $arg->weight;
            } else {
                echo "bad arg to inclusive_choice()";
            }
        } else {
            echo "bad arg to inclusive_choice()";
        }
    }

    parse_str($bolt_ex->query_string);

    switch ($bolt_ex->mode) {
    case BOLT_MODE_SHOW:
        shuffle($choices);
        $i = 0;
        start_table();
        foreach ($choices as $choice) {
            $c = $choice[0];
            row2("<input name=q_".$bolt_ex->index."_$i type=checkbox>", $c);
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
            $key = "q_".$bolt_ex->index."_$i";
            $response = isset($$key);
            $r = $choice[1];
            $correct = ($r && $response) || (!$r && !$response);
            if ($correct) $score += 1./$n;
            $i++;
        }
        $bolt_ex->score += $score;
        $bolt_ex->weight += $weight;
        break;
    case BOLT_MODE_ANSWER:
        $i = 0;
        $n = count($choices);
        shuffle($choices);
        start_table();
        table_header("Choice", "Correct?", "Your answer");
        foreach ($choices as $choice) {
            $c = $choice[0];
            $key = "q_".$bolt_ex->index."_$i";
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
    $bolt_ex->index++;
}

function image_rect($img, $rect) {
    global $bolt_ex;

    parse_str($bolt_ex->query_string);

    switch ($bolt_ex->mode) {
    case BOLT_MODE_SHOW:
        echo "<input type=image name=pic_$bolt_ex->index src=$img>
        ";
        break;
    case BOLT_MODE_SCORE:
        $x = get_int("pic_".$bolt_ex->index."_x");
        $y = get_int("pic_".$bolt_ex->index."_y");
        $right = true;
        if ($x < $rect[0]) $right = false;
        if ($x > $rect[1]) $right = false;
        if ($y < $rect[2]) $right = false;
        if ($y > $rect[3]) $right = false;
        if ($right) {
            $bolt_ex->score += 1;
        }
        $bolt_ex->weight += $weight;
        break;
    case BOLT_MODE_ANSWER:
        $x = get_int("pic_".$bolt_ex->index."_x");
        $y = get_int("pic_".$bolt_ex->index."_y");
        $right = true;
        if ($x < $rect[0]) $right = false;
        if ($x > $rect[1]) $right = false;
        if ($y < $rect[2]) $right = false;
        if ($y > $rect[3]) $right = false;
        $cx = $rect[0];
        $cy = $rect[2];
        $sizex = $rect[1]-$rect[0];
        $sizey = $rect[3]-$rect[2];
        $ax = $x-4;
        $ay = $y-4;
        $color = $right?"green":"red";
        if ($right) {
            echo "The point you selected (shown in green) is correct.";
        } else {
            echo "The point you selected (shown in red) is not correct.";
        }
        echo "
            <div style=\"position:relative; \">
            <span style=\"width:".$sizex."px;height:".$sizey."px;position:absolute;top:".$cy.";left:".$cx.";color:white;border-style:solid;border-width:3px\"><br></span>
            <span style=\"width:8;height:8;position:absolute;top:".$ay.";left:".$ax.";color:$color;border-style:solid;border-width:4px\"><br></span>
            <img src=$img align=left>
            </div>
            <br clear=all>
        ";
        break;
    }
    $bolt_ex->index++;
}

class BoltFitbField {
    public $textarea, $nrows, $ncols;
    function __construct($textarea, $nrows, $ncols) {
        $this->textarea = $textarea;
        $this->nrows = $nrows;
        $this->ncols = $ncols;
    }
}

function field($n) {
    return new BoltFitbField(false, 1, $n);
}

function box($nr, $nc) {
    return new BoltFitbField(true, $nr, $nc);
}

class BoltFitbAnswer {
    public $type;   // 0=constant, 1=regexp, 2=func
    public $ans;
    function __construct($type, $ans) {
        $this->type = $type;
        $this->ans = $ans;
    }
}

function answer($ans) {
    return new BoltFitbAnswer(0, $ans);
}

function answer_regexp($ans) {
    return new BoltFitbAnswer(1, $ans);
}

function answer_func($ans) {
    return new BoltFitbAnswer(2, $ans);
}

function fitb() {
    global $bolt_ex;
    $args = func_get_args();
    $field = new BoltFitbField(false, 1, 20);
    $answer = null;
    foreach ($args as $arg) {
        if (is_array($arg)) {
            $choices[] = $arg;
        } else if (is_object($arg)) {
            if (get_class($arg) == 'BoltFitbField') {
                $field = $arg;
            } else if (get_class($arg) == 'BoltFitbAns') {
                $answer = $arg;
            } else {
                echo "bad arg to fitb()";
            }
        } else {
            echo "bad arg to fitb()";
        }
    }

    switch ($bolt_ex->mode) {
    case BOLT_MODE_SHOW:
        if ($field->textarea) {
            echo "<textarea name=q_".$bolt_ex->index." nrows=$field->nrows ncols=$field->ncols>
                </textarea>
            ";
        } else {
            echo "<input name=q_".$bolt_ex->index." length=$field->ncols>";
        }
        break;
    case BOLT_MODE_SCORE:
        if (!$answer) break;
        $bolt_ex->score = 0;
        $key = "q_".$bolt_ex->index;
        if (isset($$key)) {
            $response = $$key;
        } else {
            $response = "";
        }
        switch ($answer->type) {
        case 0:
            if ($response == $answer->ans) {
                $bolt_ex->score = 1;
            }
            break;
        case 1:
            if (ereg($answer->ans, $response)) {
                $bolt_ex->score = 1;
            }
            break;
        case 2:
            $bolt_ex->score = call_user_func($answer->ans, $response);
            break;
        }
        break;
    case BOLT_MODE_ANSWER:
        $key = "q_".$bolt_ex->index;
        if (isset($$key)) {
            $response = $$key;
        } else {
            $response = "";
        }
        break;
    }
    $bolt_ex->index++;
}

?>
