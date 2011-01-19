<?php

// multi-purpose script to do BOINC client simulations and graph the results
//
// terms:
// "policy" - a combination of scheduling policies (see POLICY below)
// "scenario" - a combination of host characteristics,
//      project characteristics, and preferences,
//      described by a set of files (client_state.xml etc.).
//      A scenario can have a parameter,
//      macro-subsituted into client_state_template.xml
// "result" - the vector of figures of merit (see RESULT below)
//      resulting from a simulation
// 
// The script provides 2 functions:
//
// compare_policies()
//      Compare 2 policies over a set of scenarios.
//      Show the average results as side-by-side bar graphs
//
// compare_scenario_params()
//      For a given policy and a set of parameterized scenarios,
//      show the average results as a function of the parameter.
//      Show the figures of merit as line graphs.

$duration = 864000;     // sim duration
$delta = 60;            // time step
$rec_half_life = 10*86400;

// a set of scheduling policies
//
class POLICY {
    public $name;
    public $use_rec;
    public $use_hyst_fetch;
    public $cpu_sched_rr_only;
    public $server_uses_workload;

    function __construct($name) {
        $this->name = $name;
        $this->use_rec = false;
        $this->use_hyst_fetch = false;
        $this->cpu_sched_rr_only = false;
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
    global $duration, $delta, $rec_half_life;

    $args = "";
    if ($policy->use_rec) $args .= " --use_rec";
    if ($policy->use_hyst_fetch) $args .= " --use_hyst_fetch";
    if ($policy->cpu_sched_rr_only) $args .= " --cpu_sched_rr_only";
    if ($policy->server_uses_workload) $args .= " --server_uses_workload";
    $args .= " --duration $duration --delta $delta --rec_half_life $rec_half_life";

    $cmd = "sim $args --infile_prefix $in/ --outfile_prefix $out/";
    echo "cmd: $cmd\n";
    system($cmd);
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

///////////// THE TOP-LEVEL FUNCTIONS ////////////

// Compare 2 policies for a set of scenarios, and show the average results.
// Outputs are stored in the given directory.
// Subdirectories policy_scenario/ store individual sim outputs.
//
function compare_policies($title, $set, $p1, $p2, $out_dir) {
    $sum1 = new RESULT($p1->name);
    $sum2 = new RESULT($p2->name);
    @mkdir($out_dir);
    foreach ($set as $s) {
        $sum1 = do_sim_aux($out_dir, $s, $p1, "p1", $sum1);
        $sum2 = do_sim_aux($out_dir, $s, $p2, "p2", $sum2);
    }
    $sum1->scale(count($set));
    $sum2->scale(count($set));
    graph_2_results($title, $out_dir, $sum1, $sum2);
}

// For a given policy and a set of parameterized scenarios,
// show the average results as a function of the parameter.
// Outputs are stored in the given directory.
// Subdirectories scenario_arg/ store individual sim outputs
//
function compare_scenario_params1($title, $set, $policy, $lo, $hi, $inc, $out_dir) {
    @mkdir($out_dir);
    $results = array();
    for ($x = $lo; $x <= $hi; $x += $inc) {
        $sum = new RESULT($x);
        foreach ($set as $s) {
            $sum = do_sim_scenario_param($out_dir, $s, $policy, $x, $sum);
        }
        $results[] = $sum;
    }
    graph_n_results1($title, $out_dir, $results);
}

// same, but compare 2 policies and graph only wasted CPU
//
function compare_scenario_params2($title, $xlabel, $lab1, $lab2, $set, $p1, $p2, $lo, $hi, $inc, $out_dir) {
    @mkdir($out_dir);
    $rr1 = array();
    $rr2 = array();
    for ($x = $lo; $x <= $hi; $x += $inc) {
        $sum = new RESULT($x);
        foreach ($set as $s) {
            $sum = do_sim_scenario_param($out_dir, $s, $p1, $x, $sum);
        }
        $rr1[] = $sum;
        $sum = new RESULT($x);
        foreach ($set as $s) {
            $sum = do_sim_scenario_param($out_dir, $s, $p2, $x, $sum);
        }
        $rr2[] = $sum;
    }
    graph_n_results2($title, $xlabel, $lab1, $lab2, $out_dir, $rr1, $rr2, 3);
}

// compare two policies, varying rec_half_life
//
function compare_policy_params($title, $xlabel, $lab1, $lab2, $set, $p1, $p2, $lo, $hi, $inc, $out_dir) {
    global $rec_half_life;

    @mkdir($out_dir);
    $rr1 = array();
    $rr2 = array();
    for ($x = $lo; $x <= $hi; $x += $inc) {
        $rec_half_life = $x;
        $sum = new RESULT($x);
        foreach ($set as $s) {
            $sum1 = do_sim_policy_param($out_dir, $s, $p1, $x, $sum);
        }
        $rr1[] = $sum;
        $sum = new RESULT($x);
        foreach ($set as $s) {
            $sum1 = do_sim_policy_param($out_dir, $s, $p2, $x, $sum);
        }
        $rr2[] = $sum;
    }
    graph_n_results2($title, $xlabel, $lab1, $lab2, $out_dir, $rr1, $rr2, 7);
}

///////////// EXAMPLE USAGES ////////////

// compare REC and debt scheduling for a set of scenarios
//
if (0) {
    $p1 = new POLICY("JS_LOCAL");
    $p2 = new POLICY("JS_GLOBAL");
    $p2->use_rec = true;
    compare_policies("Scenario 2", array("scen2"), $p1, $p2, "test1");
}

if (0) {
    $p = new POLICY("");
    $p->use_rec = true;
    $lo = 2e9;
    $hi = 1e10;
    $inc = 1e9;
    compare_scenario_params1("Scenario 3", array("s3"), $p, $lo, $hi, $inc, "test2");
}

if (1) {
    $p1 = new POLICY("WF_ORIG");
    $p2 = new POLICY("WF_HYST");
    $p2->use_hyst_fetch = true;
    compare_policies("Scenario 4", array("scen4"), $p1, $p2, "test3");
}

if (0) {
    $p1 = new POLICY("JS_LOCAL");
    $p2 = new POLICY("JS_GLOBAL");
    $p2->use_rec = true;
    compare_policies("Scenario 4", array("scen4"), $p1, $p2, "test4");
}

if (0) {
    $duration = 50* 86400;
    $p1 = new POLICY("JS_LOCAL");
    $p2 = new POLICY("JS_GLOBAL");
    $p2->use_rec = true;
    compare_policies("Scenario 3", array("scen3"), $p1, $p2, "test5");
}

// compare WRR and EDF
if (0) {
    $p1 = new POLICY("JS_LOCAL");
    $p1->use_rec = true;
    $p1->use_hyst_fetch = true;
    $p1->cpu_sched_rr_only = true;
    $p2 = new POLICY("JS_WRR");
    $p2->use_rec = true;
    $p2->use_hyst_fetch = true;
    $lo = 1000;
    $hi = 2000;
    $inc = 100;
    compare_scenario_params2("Wasted processing", "Latency bound", "JS_WRR", "JS_LOCAL", array("scen5"), $p1, $p2, $lo, $hi, $inc, "test6");
}

// compare rec and debt over a range of rec_half_life
if (0) {
    $p1 = new POLICY("JS_LOCAL");
    $p2 = new POLICY("JS_GLOBAL");
    $p2->use_rec = true;
    $lo = 5*86400;
    $hi = 40*86400;
    $inc = 5*86400;
    $duration = 50* 86400;
    compare_policy_params("Resource share violation", "REC half-life", "JS_LOCAL", "JS_GLOBAL", array("scen3"), $p1, $p2, $lo, $hi, $inc, "test7");
}

if (0) {
    $p1 = new POLICY("JS_LOCAL + WF_ORIG");
    $p2 = new POLICY("JS_GLOBAL + WF_HYSTERESIS");
    $p2->use_rec = true;
    compare_policies("Scenario 4", array("scen4"), $p1, $p2, "test8");
}
?>
