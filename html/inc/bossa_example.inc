<?php

// two results are compatible if neither found an ellipse,
// or they both did and centers are within 20 pixels
//
function bossa_example_compare($r1, $r2) {
    if ($r1->have_ellipse) {
        if ($r2->have_ellipse) {
            $dx = ($r1->cx - $r2->cx);
            $dy = ($r1->cy - $r2->cy);
            $dsq = $dx*$dx + $dy*$dy;
            return ($dsq < 400);
        } else return false;
    } else {
        return !$r2->have_ellipse;
    }
}

// handle a completed job with the given consensus set
//
function bossa_example_handle($bj, $c) {
    $res = $c[0];
    if ($res->have_ellipse) {
        $res->cx = 0;
        $res->cy = 0;
        foreach ($c as $r) {
            $res->cx += $r->cx;
            $res->cy += $r->cy;
        }
        $res->cx /= count($c);
        $res->cy /= count($c);
    }

    $info = json_decode($bj->info);
    $info->result = $res;
    $i = json_encode($info);
    $bj->update("info='$i'");
}

?>
