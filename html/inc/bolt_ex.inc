<?php

function bolt_exclusive_choice($choices) {
    global $bolt_mode;
    global $bolt_index;

    switch ($bolt_mode) {
    case BOLT_MODE_SHOW:
        foreach ($choices as $choice) {
            echo "<br><input name=q_$bolt_index type=radio> $choice";
        }
        break;
    case BOLT_MODE_SCORE:
        $resp = $_GET["q_$bolt_index"];

        break;
    case BOLT_MODE_ANSWER:
        break;
    }
}
?>
