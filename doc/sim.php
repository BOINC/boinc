<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2011 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

// web interface to client simulator

require_once("../inc/util.inc");

function show_scenario_summary($f) {
    echo "<a href=sim.php?action=show_scenario&name=$f>$f</a>";
    readfile("scenarios/$f/description");
    readfile("scenarios/$f/user_name");
    show # sims
}

// show existing scenarios, "create scenario" button
//
function show_scenarios() {
    page_head("Client simulator");
    $d = open_dir("scenarios");
    while ($f = readdir($d)) {
        if ($f == ".") continue;
        if ($f == "..") continue;
        show_scenario_summary($f);
    }
    echo "<a href=sim.php?action=create_scenario>Create new scenario</a>";
    page_tail();
}

// show form for creating a scenario
//
function create_scenario_form() {
    get_logged_in_user();
    page_head("Create scenario");
    echo "<form action=sim.php?action=create_scenario>
        <input name=client_state type=file>
        <p><input name=prefs type=file>
        <p><input name=prefs_override type=file>
        <p><input name=cc_config type=file>
        <p>Description: <input name=description type=textarea>
        <p><input type=submit value=OK>
        </form>
    ";
    page_tail();
}

function create_scenario() {
    get_logged_in_user();
    if (!is_uploaded_file($_FILES['client_state']['tmp_name'])) {

    }

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
