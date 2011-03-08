<?php

require_once("../inc/util.inc");

function show_scenarios() {
    page_head();
    $d = open_dir("scenarios");
    while ($f = readdir($d)) {
        if ($f == ".") continue;
        if ($f == "..") continue;
    }
    page_tail();
}

function create_scenario_form() {
    page_head();
    page_tail();
}

function create_scenario() {
    // redirect to show_scenario
}

function show_scenario() {
    page_head();
    page_tail();
}

function simulation_form() {
    page_head();
    page_tail();
}

function simulation_action() {
    //redirect to show_simulation
}

function show_simulation() {
    page_head();
    page_tail();
}

$action = get_str("action", true);

switch ($action) {
case "create_scenario_form":
    create_scenario_form();
    break;
case "create_scenario":
    create_scenario();
    break;
case "show_scenario":
    show_scenario();
    break;
case "simulation_form":
    simulation_form();
    break;
case "simulation_action":
    simulation_action();
    break;
case "show_simulation":
    show_simulation();
    break;
default:
    show_scenarios();
}

?>
