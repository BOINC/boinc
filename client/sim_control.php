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
// compare_params()
//      For a given policy and a set of parameterized scenarios,
//      show the average results as a function of the parameter.
//      Show the figures of merit as line graphs.

$duration = 86400;     // sim duration

// a set of scheduling policies
//
class POLICY {
    public $use_rec;
    public $use_hyst_fetch;
    public $cpu_sched_rr_only;
    public $server_uses_workload;

    function __construct() {
        $this->use_rec = false;
        $this->use_hyst_fetch = false;
        $this->cpu_sched_rr_only = false;
        $this->server_uses_workload = false;
    }

    function name() {
        if ($this->use_rec) return "REC";
        return "debt";
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
            fprintf($f, "%e", $this->name);
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
    global $duration;

    $args = "";
    if ($policy->use_rec) $args .= " --use_rec";
    if ($policy->use_hyst_fetch) $args .= " --use_hyst_fetch";
    if ($policy->cpu_sched_rr_only) $args .= " --cpu_sched_rr_only";
    if ($policy->server_uses_workload) $args .= " --server_uses_workload";
    $args .= " --duration $duration";

    $cmd = "sim $args --infile_prefix $in/ --outfile_prefix $out/";
    echo "cmd: $cmd\n";
    system($cmd);
}

function write_gp_bar($fname, $title, $data_fname) {
    $f = fopen($fname, "w");
    $s = <<<EOT
set terminal png small size 640, 480
set title "Scenario 1"
set style fill pattern
set style histogram clustered
# set yrange[0:1]
plot \
    "$data_fname" u 3:xtic(1) t "Wasted" w histograms, \
    "" u 5 t "Idle" w histograms, \
    "" u 7 t "Share violation" w histograms, \
    "" u 9 t "Monotony" w histograms, \
    "" u 11 t "RPCs" w histograms
EOT;
    fwrite($f, $s);
    fclose($f);
}

function write_gp_line($fname, $title, $data_fname) {
    $f = fopen($fname, "w");
    $s = <<<EOT
set terminal png small size 640, 480
set title "Scenario 1"
set xlabel "foobar"
set format x "%e"
set style data linesp
# set yrange[0:1]
plot \
    "$data_fname" u 3:xtic(1) t "Wasted" , \
    "" u 5 t "Idle" , \
    "" u 7 t "Share violation" , \
    "" u 9 t "Monotony" , \
    "" u 11 t "RPCs" 
EOT;
    fwrite($f, $s);
    fclose($f);
}

function graph_2_results($dir, $r1, $r2) {
    $data_fname = "$dir/cr.dat";
    $f = fopen($data_fname, "w");
    $r1->write($f);
    $r2->write($f);
    fclose($f);
    $gp_fname = "$dir/cr.gp";
    $png_fname = "$dir/cr.png";
    write_gp_bar($gp_fname, "policy comparison", $data_fname);
    system("gnuplot < $gp_fname > $png_fname");
}

function graph_n_results($dir, $results) {
    $data_fname = "$dir/cr.dat";
    $f = fopen($data_fname, "w");
    foreach ($results as $r) {
        $r->write($f);
    }
    fclose($f);
    $gp_fname = "$dir/cr.gp";
    $png_fname = "$dir/cr.png";
    write_gp_line($gp_fname, "policy comparison", $data_fname);
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

function do_sim_param($out_dir, $scenario, $policy, $param, $sum) {
    $sim_out_dir = $out_dir."/".$scenario."_".$param;
    @mkdir($sim_out_dir);
    $cs_template_fname = "$scenario/client_state_template.xml";
    $cs_fname = "$scenario/client_state.xml";
    $cs = file_get_contents($cs_template_fname);
    $cs = str_replace("PARAM", $param, $cs);
    file_put_contents($cs_fname, $cs);
    do_sim($scenario, $sim_out_dir, $policy);
    $r = new RESULT($param);
    $sum->add($sim_out_dir);
    return $sum;
}

///////////// THE TOP-LEVEL FUNCTIONS ////////////

// Compare 2 policies for a set of scenarios, and show the average results.
// Outputs are stored in the given directory.
// Subdirectories policy_scenario/ store individual sim outputs.
//
function compare_policies($set, $p1, $p2, $out_dir) {
    $sum1 = new RESULT($p1->name());
    $sum2 = new RESULT($p2->name());
    @mkdir($out_dir);
    foreach ($set as $s) {
        $sum1 = do_sim_aux($out_dir, $s, $p1, "p1", $sum1);
        $sum2 = do_sim_aux($out_dir, $s, $p2, "p2", $sum2);
    }
    $sum1->scale(count($set));
    $sum2->scale(count($set));
    graph_2_results($out_dir, $sum1, $sum2);
}

// For a given policy and a set of parameterized scenarios,
// show the average results as a function of the parameter.
// Outputs are stored in the given directory.
// Subdirectories scenario_arg/ store individual sim outputs
//
function compare_params($set, $policy, $lo, $hi, $inc, $out_dir) {
    @mkdir($out_dir);
    $results = array();
    for ($x = $lo; $x <= $hi; $x += $inc) {
        $sum = new RESULT($x);
        foreach ($set as $s) {
            $sum = do_sim_param($out_dir, $s, $policy, $x, $sum);
        }
        $results[] = $sum;
    }
    graph_n_results($out_dir, $results);
}

///////////// EXAMPLE USAGES ////////////

// compare REC and debt scheduling for a set of scenarios
//
if (0) {
    $p1 = new POLICY();
    $p2 = new POLICY();
    $p2->use_rec = true;
    compare_policies(array("s2"), $p1, $p2, "test1");
}

// evaluate a policy over a set of parameterized policies
//
if (0) {
    $p = new POLICY();
    $p->use_rec = true;
    $lo = 2e9;
    $hi = 1e10;
    $inc = 1e9;
    compare_params(array("s3"), $p, $lo, $hi, $inc, "test2");
}

?>
