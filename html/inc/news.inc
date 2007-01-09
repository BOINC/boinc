<?php

function news_item($date, $text, $title="") {
    echo "<p><span class=news_date>$date</span> $title
        <br>$text.
    ";
}

function show_news($items, $n) {
    if ($n > count($items)) {
        $n = count($items);
    }
    for ($i=0; $i<$n; $i++) {
        news_item($items[$i][0], $items[$i][1], $items[$i][2]);
    }
}

function show_old_news($items, $n) {
    $tot = count($items);
    for ($i=$n; $i<count($items); $i++) {
        $j = $tot-$i;
        echo "<a name=$j></a>\n";
        news_item($items[$i][0], $items[$i][1]);
    }
}

?>
