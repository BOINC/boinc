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

require_once("sim_util.inc");

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
