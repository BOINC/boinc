<?php

// script to do simulator runs and graph the output

function do_sim($in, $out, $rec) {
    $prog = $rec?"sim_rec":"sim_debt";
    $cmd = "$prog --infile_prefix $in --outfile_prefix $out";
    system($cmd);
}

function write_gp($fname, $title) {
    $f = fopen($fname, "w");
    $s = <<<EOT
set terminal png small size 640, 480
set title "Scenario 1"
set style fill pattern
set style histogram clustered
set yrange[0:1]
plot \
    "test.dat" u 3:xtic(1) t "Wasted" w histograms, \
    "" u 5 t "Idle" w histograms, \
    "" u 7 t "Share violation" w histograms, \
    "" u 9 t "Monotony" w histograms, \
    "" u 11 t "RPCs" w histograms
EOT;
    fwrite($f, $s);
    fclose($f);
}

// compare REC and debt for the given scenario
//
function compare_rec($in) {
    $rec_out = $in."_rec_";
    $debt_out = $in."_debt_";
    do_sim($in, $rec_out, true);
    do_sim($in, $debt_out, false);

    // concatenate the summary files

    $r = file_get_contents($rec_out."summary.dat");
    $d = file_get_contents($debt_out."summary.dat");   

    $f = fopen($in."_cr.dat", "w");
    fwrite($f, "REC");
    fwrite($f, $r);
    fwrite($f, "debt");
    fwrite($f, $s);
    fclose($f);

    write_gp($in, $in);

    $png_fname = $in."_cr.png";
    system("gnuplot < $in > $png_fname);
}

compare_rec("s1_");

?>
