<?php

define("CREDIT_HALF_LIFE", 86400*7);

// update a credit average.
// this must match the function in lib/util.C

function update_average($now, $work_start_time, $work, &$avg, &$avg_time) {
    if ($avg_time) {
        $diff = $now - $avg_time;
        if ($diff <=0) $diff = 3600;
        $diff_days = $diff/86400;
        $weight = exp(-$diff*M_LN2/CREDIT_HALF_LIFE);
        $avg *= $weight;
        $avg += (1-$weight)*($work/$diff_days);
    } else if ($work) {
        $dd = ($now - $work_start_time)/86400;
        $avg = $work/$dd;
    }
    $avg_time = $now;
}

?>
