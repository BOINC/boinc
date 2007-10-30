<?php

function bolt_exclusive_choice($choices) {
    foreach ($choices as $choice) {
        echo "<br><input name=q type=radio> $choice";
    }
}
?>
