<?php

// express a time difference in readable form
//
function time_diff_str($t1, $t2) {
    if (!$t1 || !$t2) return "---";
    $diff = $t2 - $t1;
    if ($diff<0){
	$pre="In "; 
	$post=""; 
	$diff=-$diff;
    } else {
	$pre="";
	$post=" ago";
    }
    $x = "";


    if ($diff >= 3600*24) {
        $n = (int) ($diff/(3600*24));
        if ($n == 1) {
            $x .= "1 day ";
        } else {
            $x .= "$n days ";
        }
        return $pre.$x.$post;
        //$diff -= $n*3600*24;
    } elseif ($diff >= 3600) {
        $n = (int) ($diff/3600);
        if ($n == 1) {
            $x .= "1 hour ";
        } else {
            $x .= $n." hours ";
        }
        return $pre.$x.$post;
        //$diff -= $n*3600;
    } elseif ($diff >= 60) {
        $n = (int) ($diff/60);
        if ($n == 1) {
            $x .= "1 minute ";
        } else {
            $x .= $n." minutes ";
        }
        return $pre.$x.$post;
    } elseif($diff > 1 || $diff==0) {
        return $pre."$diff seconds".$post;
    } elseif($diff == 1){
        return $pre."$diff seconds".$post;
    }    
}
?>
