<?php

function news_item($date, $text) {
    echo "<font color=666666>$date</font>
        <br />
        $text
        <br /><br />
    ";
}

function show_news($items, $n) {
    if ($n > count($items)) {
        $n = count($items);
    }
    for ($i=0; $i<$n; $i++) {
        news_item($items[$i][0], $items[$i][1]);
    }
}

function show_old_news($items, $n) {
    for ($i=$n; $i<count($items); $i++) {
        news_item($items[$i][0], $items[$i][1]);
    }
}

?>
