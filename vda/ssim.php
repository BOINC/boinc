<?php

// controller script for storage simulator
//
// usage: ssim.php infile
//
// format of infile
// policy policy_file_1
// ...
// policy policy_file_n
// host_life_mean x1 ... xn
// connect_interval x
// mean_xfer_rate x

// output graphs:
// infile_ft.png: fault tolerance level vs time
// infile_du.png: disk usage vs time
// infile_ub.png: upload BW vs time
// infile_db.png: download BW vs time

function parse_input_file($filename) {
    $x = null;
    $x->name = $filename;
    $x->policy = array();
    $x->policy_name = array();
    $f = fopen($filename, "r");
    if (!$f) die("no file $filename\n");
    while (($buf = fgets($f)) !== false) {
        $w = explode(" ", $buf);
        switch ($w[0]) {
        case "policy":
            $pfile = trim($w[1]);
            $x->policy[] = $pfile;
            $g = fopen($pfile, "r");
            $name = fgets($g);
            fclose($g);
            $x->policy_name[] = trim($name);
            break;
        case "host_life_mean":
            $x->host_life_mean = array();
            for ($i=1; $i<count($w); $i++) {
                $x->host_life_mean[] = 86400 * (double)($w[$i]);
            }
            break;
        case "connect_interval":
            $x->connect_interval = (double)($w[1]);
            break;
        case "mean_xfer_rate":
            $x->mean_xfer_rate = (double)($w[1]);
            break;
        case "file_size":
            $x->file_size = (double)($w[1]);
            break;
        }
    }
    fclose($f);
    return $x;
}

function make_graph($input, $prefix, $title, $index) {
    $gp_filename = $input->name."_$prefix.gp";
    $f = fopen($gp_filename, "w");
    fprintf($f, "set terminal png small size 1024, 768
set yrange[0:]
set ylabel \"$title\"
set xlabel \"Mean host lifetime (days)\"
plot ");
    $n = sizeof($input->policy);
    for ($i=0; $i<$n; $i++) {
        $p = $input->policy[$i];
        $pname = $input->policy_name[$i];
        $fname = $input->name."_$p.dat";
        fprintf($f, "\"$fname\" using 1:$index title \"$pname\" with lines");
        if ($i < $n-1) {
            fprintf($f, ", \\");
        }
        fprintf($f, "\n");
    }
    fclose($f);
    $png_filename = $input->name."_$prefix.png";
    $cmd = "gnuplot < $gp_filename > $png_filename";
    echo "$cmd\n";
    system($cmd);
}

function parse_output_file($fname) {
    $f = fopen($fname, "r");
    if (!$f) {
        die("no output file $fname\n");
    }
    $ft = (int)fgets($f);
    $du = (double)fgets($f);
    $ul = (double)fgets($f);
    $dl = (double)fgets($f);
    return array($ft, $du, $ul, $dl);
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
        $cmd = "ssim --policy $p --host_life_mean $hlm --connect_interval $input->connect_interval --mean_xfer_rate $input->mean_xfer_rate --file_size $input->file_size > /dev/null";
        echo "$cmd\n";
        system($cmd);
        list($ft, $du, $ub, $db) = parse_output_file("summary.txt");
        $hlmd = $hlm/86400;
        $du_rel = $du/$input->file_size;
        fprintf($datafile, "$hlmd $ft $du_rel $ub $db\n");
    }
    fclose($datafile);
}

make_graph($input, "ft", "Fault tolerance", 2);
make_graph($input, "du", "Relative disk usage", 3);
make_graph($input, "ub", "Upload bandwidth (Mbps)", 4);
make_graph($input, "db", "Download bandwidth (Mbps)", 5);

?>
