<?php

// a set of scheduling policies
//
class POLICY {
    public $name;
    public $existing_jobs_only;
    public $cpu_sched_rr_only;
    public $include_empty_projects;
    public $server_uses_workload;

    function __construct($name) {
        $this->name = $name;
        $this->duration = 86400;
        $this->delta = 60;
        $this->rec_half_life = 10*86400;
        $this->existing_jobs_only = false;
        $this->cpu_sched_rr_only = false;
        $this->include_empty_projects = false;
        $this->server_uses_workload = false;
    }
}

// a set of figures of merit
//
class RESULT {
    public $name;
    public $wasted_frac;
    public $idle_frac;
    public $share_violation;
    public $monotony;
    public $rpcs;

    function __construct($n) {
        $this->name = $n;
        $this->wasted_frac = 0;
        $this->idle_frac = 0;
        $this->share_violation = 0;
        $this->monotony = 0;
        $this->rpcs = 0;
    }
    function write($f) {
        if (is_numeric($this->name)) {
            fprintf($f, "%d", $this->name);
        } else {
            fprintf($f, "\"%s\"", $this->name);
        }
        fprintf($f,
            " wf %f if %f sv %f m %f r %f\n",
            $this->wasted_frac,
            $this->idle_frac,
            $this->share_violation,
            $this->monotony,
            $this->rpcs
        );
    }
    function add($dir) {
        $f = fopen("$dir/results.dat", "r");
        fscanf($f, "wf %f if %f sv %f m %f r %f", $wf, $if, $sv, $m, $r);
        $this->wasted_frac += $wf;
        $this->idle_frac += $if;
        $this->share_violation += $sv;
        $this->monotony += $m;
        $this->rpcs += $r;
    }
    function scale($n) {
        $this->wasted_frac /= $n;
        $this->idle_frac /= $n;
        $this->share_violation /= $n;
        $this->monotony /= $n;
        $this->rpcs /= $n;
    }
}

// do a simulation
//
function do_sim($in, $out, $policy) {
    $args = "";
    if ($policy->existing_jobs_only) $args .= " --existing_jobs_only";
    if ($policy->cpu_sched_rr_only) $args .= " --cpu_sched_rr_only";
    if ($policy->include_empty_projects) $args .= " --include_empty_projects";
    if ($policy->server_uses_workload) $args .= " --server_uses_workload";
    $args .= " --duration $policy->duration";
    $args .= " --delta $policy->delta";
    $args .= " --rec_half_life $policy->rec_half_life";

    $cmd = "./sim $args --infile_prefix $in/ --outfile_prefix $out/";
    //echo "cmd: $cmd\n"; die;
    system($cmd, $ret);
    if ($ret) {
        die("command failed ($ret): $cmd\n");
    }
}

// display N results (usually 2) as bar graphs
//
function write_gp_bar($fname, $title, $data_fname) {
    $f = fopen($fname, "w");
    $s = <<<EOT
set terminal png small size 320, 240
set title "$title"
set style fill pattern
set style histogram clustered
set yrange[0:1]
plot \
    "$data_fname" u 3:xtic(1) t "Wasted" w histograms, \
    "" u 5 t "Idle" w histograms, \
    "" u 7 t "Share" w histograms, \
    "" u 9 t "Monotony" w histograms, \
    "" u 11 t "RPCs" w histograms
EOT;
    fwrite($f, $s);
    fclose($f);
}

// display N results as line graphs, one for each figure of merit
//
function write_gp_line($fname, $title, $data_fname) {
    $f = fopen($fname, "w");
    $s = <<<EOT
set terminal png small size 320, 240
set title "$title"
set xlabel "foobar"
set format x "%e"
set style data linesp
set yrange[0:1]
plot \
    "$data_fname" u 3:xtic(1) t "Wasted" , \
    "" u 5 t "Idle" , \
    "" u 7 t "Share" , \
    "" u 9 t "Monotony" , \
    "" u 11 t "RPCs" 
EOT;
    fwrite($f, $s);
    fclose($f);
}

// display 2 groups of N results as line graphs.
// Show only 1 figure of merit ($col).
//
function write_gp_line2(
    $fname, $title, $xlabel, $lab1, $lab2, $data1, $data2, $col
) {
    $f = fopen($fname, "w");
    $s = <<<EOT
set terminal png small size 320, 240
set title "$title"
set xlabel "$xlabel"
set format x "%e"
set style data linesp
set yrange[0:1]
set xtic rotate
plot "$data1" u $col:xtic(1) t "$lab1" , "$data2" u $col:xtic(1) t "$lab2" 

EOT;
    fwrite($f, $s);
    fclose($f);
}

function graph_2_results($title, $dir, $r1, $r2) {
    $data_fname = "$dir/cr.dat";
    $f = fopen($data_fname, "w");
    $r1->write($f);
    $r2->write($f);
    fclose($f);
    $gp_fname = "$dir/cr.gp";
    $png_fname = "$dir/cr.png";
    write_gp_bar($gp_fname, $title, $data_fname);
    system("gnuplot < $gp_fname > $png_fname");
}

function graph_n_results1($title, $dir, $results) {
    $data_fname = "$dir/cr.dat";
    $f = fopen($data_fname, "w");
    foreach ($results as $r) {
        $r->write($f);
    }
    fclose($f);
    $gp_fname = "$dir/cr.gp";
    $png_fname = "$dir/cr.png";
    write_gp_line($gp_fname, $title, $data_fname);
    system("gnuplot < $gp_fname > $png_fname");
}

function graph_n_results2(
    $title, $xlabel, $lab1, $lab2, $dir, $results1, $results2, $col
) {
    for ($i=0; $i<2; $i++) {
        $data_fname = "$dir/cr_$i.dat";
        $f = fopen($data_fname, "w");
        $rr = $i?$results2:$results1;
        foreach ($rr as $r) {
            $r->write($f);
        }
        fclose($f);
    }
    $gp_fname = "$dir/cr.gp";
    $png_fname = "$dir/cr.png";
    write_gp_line2(
        $gp_fname, $title, $xlabel, $lab1, $lab2,
        "$dir/cr_0.dat", "$dir/cr_1.dat", $col
    );
    system("gnuplot < $gp_fname > $png_fname");
}

// create output dir, do a simulation, accumulate results
//
function do_sim_aux($out_dir, $scenario, $policy, $pname, $sum) {
    $sim_out_dir = $out_dir."/".$pname."_".$scenario;
    @mkdir($sim_out_dir);
    do_sim($scenario, $sim_out_dir, $policy);
    $sum->add($sim_out_dir);
    return $sum;
}

// substitute a param into scenario and run sim
//
function do_sim_scenario_param($out_dir, $scenario, $policy, $param, $sum) {
    $sim_out_dir = $out_dir."/".$scenario."_".$policy->name."_".$param;
    @mkdir($sim_out_dir);
    $cs_template_fname = "$scenario/client_state_template.xml";
    $cs_fname = "$scenario/client_state.xml";
    $cs = file_get_contents($cs_template_fname);
    if (!$cs) die("no file $cs_template_fname");
    $cs = str_replace("PARAM", $param, $cs);
    file_put_contents($cs_fname, $cs);
    do_sim($scenario, $sim_out_dir, $policy);
    $r = new RESULT($param);
    $sum->add($sim_out_dir);
    return $sum;
}

// do a sim with a policy parameter
//
function do_sim_policy_param($out_dir, $scenario, $policy, $param, $sum) {
    $sim_out_dir = $out_dir."/".$scenario."_".$policy->name."_".$param;
    @mkdir($sim_out_dir);
    do_sim($scenario, $sim_out_dir, $policy);
    $sum->add($sim_out_dir);
    return $sum;
}

?>
