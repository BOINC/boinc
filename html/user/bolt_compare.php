<?php

function choose_select() {
}

function choose_xset() {
}

function show_form() {
    choose_select();
    choose_xset();
}

function show_results() {

}

//if (get_str('submit', true)) {
//    show_results();
//} else {
//    show_form();
//}

// compute the mean and stdev of an array
//
function mean_stdev($array, &$mean, &$stdev) {
    $n = 0;
    $m = 0;
    $m2 = 0;

    foreach ($array as $x) {
        $n++;
        $delta = $x - $m;
        $m += $delta/$n;
        $m2 += $delta*($x-$m);
    }
    $mean = $m;
    $stdev = sqrt($m2/($n-1));
}

// approximate the 90% confidence interval for the mean of an array
//
function conf_int_90($array, &$lo, &$hi) {
    $n = count($array);
    mean_stdev($array, $mean, $stdev);

    // I'm too lazy to compute the t distribution
    $t_90 = 1.7;
    $d = $t_90 * $stdev / sqrt($n);
    $lo = $mean - $d;
    $hi = $mean + $d;
}

$a = array(1,1,1,1,0,1,1,1,3, 1, 1, 1, 1);
//mean_stdev($a, $mean, $stdev);
//echo "mean: $mean stdev: $stdev\n";

conf_int_90($a, $lo, $hi);
echo "lo $lo hi $hi\n";

?>
