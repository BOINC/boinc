<?php

// express a time difference in readable form
//
function time_diff_str($t1, $t2) {
    if (!$t1 || !$t2) return "---";
    $diff = $t2 - $t1;
    $x = "";
    if ($diff >= 3600*24) {
        $n = (int) ($diff/(3600*24));
        if ($n == 1) {
            $x .= "1 day ";
        } else {
            $x .= "$n days ";
        }
        return $x." ago";
        //$diff -= $n*3600*24;
    }
    if ($diff >= 3600) {
        $n = (int) ($diff/3600);
        if ($n == 1) {
            $x .= "1 hour ";
        } else {
            $x .= $n." hours ";
        }
        return $x." ago";
        //$diff -= $n*3600;
    }
    if ($diff >= 60) {
        $n = (int) ($diff/60);
        if ($n == 1) {
            $x .= "1 minute ";
        } else {
            $x .= $n." minutes ";
        }
        return $x." ago";
    }
    if ($diff == 1) {
        return "1 second ago";
    }
    return $x." seconds ago";
}
?>
