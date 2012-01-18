<?php

// controller script for storage simulator
//
// usage: ssim.php infile
//
// format of infile
// policy policyfile1
// ...
// policy policyfilen
// host_life_mean x1 ... xn
// connect_interval x
// mean_xfer_rate x

function parse_input_file($filename) {
    $x = null;
    $x->name = $filename;
    $x->policy = array();
    $f = fopen($filename, "r");
    if (!$f) die("no file $filename\n");
    while (($buf = fgets($f)) !== false) {
        $w = explode(" ", $buf);
        switch ($w[0]) {
        case "policy":
            $x->policy[] = trim($w[1]);
            break;
        case "host_life_mean":
            $x->host_life_mean = array();
            for ($i=1; $i<count($w); $i++) {
                $x->host_life_mean[] = (double)($w[$i]);
            }
            break;
        case "connect_interval":
            $x->connect_interval = (double)($w[1]);
            break;
        case "mean_xfer_rate":
            $x->mean_xfer_rate = (double)($w[1]);
            break;
        }
    }
    fclose($f);
    return $x;
}

function make_graph($input, $title, $index) {
    $gp_filename = $input->name."_$prefix.gp";
    $f = fopen($gp_filename, "w");
    fprintf($f, "set terminal png small size 1024, 768
set title \"$title\"
set yrange[0:]
plot ");
    $n = sizeof($input->policy);
    $i = 0;
    foreach ($input->policy as $p) {
        $fname = $input->name."_$p.dat";
        fprintf($f, "plot $fname using 1:$index title \"$p\" with lines");
        if ($i < $n-1) {
            fprintf($f, ", \\");
        }
        $i++;
    }
    fclose($f);
    $png_filename = $input->name."_$prefix.png";
    $cmd = "gnuplot < $gp_filename > $png_filename";
    system($cmd);
}

if ($argc != 2) {
    die("usage: ssim.php infile\n");
}
$input = parse_input_file($argv[1]);
foreach ($input->policy as $p) {
    $datafile = fopen($input->name."_$p.dat", "w");
    if (!file_exists($p)) {
        die("no policy file '$p'\n");
    }
    foreach ($input->host_life_mean as $hlm) {
        $cmd = "ssim --policy_file $p --host_life_mean $hlm --connect_interval $input->connect_interval --mean_xfer_rate $input->mean_xfer_rate";
        //system($cmd);
        echo "$cmd\n";
        list($du, $ul, $dl, $ft) = parse_output_file("summary.txt");
        printf($datafile, "$hlm $du $ul $dl $ft\n");
    }
}

exit();

make_graph($input, "du", "Disk usage", 2);
make_graph($input, "ub", "Upload bandwidth", 3);
make_graph($input, "db", "Download bandwidth", 4);
make_graph($input, "ft", "Fault tolerance", 5);

?>
