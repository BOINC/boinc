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
//
// to use this, symlink to it from the html/user dir of a BOINC project,
// create an apache-writable dir called "scenarios" there,
// and symlink from html/inc to sim_util.inc

require_once("../inc/util.inc");
require_once("../inc/sim_util.inc");

ini_set ("memory_limit", "1G");
set_time_limit(0);

function get_comments($dir) {
    $d = "$dir/comments";
    if (!is_dir($d)) return null;
    $dd = opendir($d);
    $x = "";
    while (false !== ($f = readdir($dd))) {
        if ($f == ".") continue;
        if ($f == "..") continue;
        if (strlen($x)) $x .= "<hr>\n";
        $userid = (int)file_get_contents("$d/$f/userid");
        $user = BoincUser::lookup_id($userid);
        $comment = file_get_contents("$d/$f/comment");
        $date = date_str(filemtime("$d/$f/comment"));
        $x .= "By $user->name ($date):<br>$comment\n";
    }
    return $x;
}

function nsims($scen) {
    $d = opendir("scenarios/$scen/simulations");
    $n = 0;
    while (false !== ($f = readdir($d))) {
        if ($f == ".") continue;
        if ($f == "..") continue;
        $n++;
    }
    return $n;
}

function show_scenario_summary($f) {
    $desc = file_get_contents("scenarios/$f/description");
    $userid = (int)file_get_contents("scenarios/$f/userid");
    $user = BoincUser::lookup_id($userid);
    $date = date_str(filemtime("scenarios/$f"));
    $nsims = nsims($f);
    echo "<tr>
        <td><a href=sim_web.php?action=show_scenario&name=$f>$f</a></td>
        <td>$user->name</td>
        <td>$date</td>
        <td>$nsims</td>
        <td>$desc</td>
        </tr>
    ";
}

// show existing scenarios, "create scenario" button
//
function show_scenarios() {
    page_head("The BOINC Client Emulator");
    echo "
        The BOINC Client Emulator (BCE)
        <b>emulates</b> a BOINC client attached to one or more projects.
        It predicts, in a few seconds,
        what the latest BOINC client will do over a period of day or months.
        By reporting problem situations to BOINC developers,
        you can help us fix bugs and improve performance.
        <h3>Scenarios</h3>
        The inputs to BCE, called <b>scenarios</b>,
        describe a particular computer and the project to which it's attached.
        A scenario consists of:
        <ul>
        <li> <b>client_state.xml</b>: describes the host and projects.
          Any projects that don't currently have tasks are ignored.
        <li> <b>global_prefs.xml</b> and <b>global_prefs_override.xml</b>:
            computing preferences (optional).
        <li> <b>cc_config.xml</b>: options such as GPU exclusions
        </ul>
        You create a scenario by uploading these files
        using the <b>Create a scenario</b> button below.
        You can use the files from a running BOINC client
        to emulate that client.
        <p>
        You can modify these files, or create new ones,
        to study hypothetical scenarios
        (e.g. hosts with a large number of CPUs,
        hosts attached to a large number of projects,
        projects with very short or long jobs, and so on).
        See <a href=http://boinc.berkeley.edu/trac/wiki/ClientSim>The
        BCE documentation</a> for details.
        <h3>Simulations</h3>
        You can run <b>simulations</b> based on existing scenarios
        (including scenarios created by other people).
        The parameters of a simulation include
        <ul>
        <li> The duration and time resolution of the simulation.
        <li> Log flags
        </ul>
        The outputs of a simulation include
        <ul>
        <li> A 'time line' showing CPU and GPU usage.
        <li> The client's message log
        <li> graphs of scheduling-related data (REC).
        <li> A summary of several <b>figures of merit</b>, including
            <ul>
            <li>idle fraction: the fraction of processing power that was unused
            <li>wasted fraction: the fraction of processing power
                that was 'wasted' on jobs that missed their deadline
            <li>resource share violation: a measure [0..1] of the extent
                to which resource shares were disobeyed.
            <li>monotony: a measure [0..1] of the extent to which
                the client ran jobs of a single project for long periods
            </ul>
        </ul>
        <h3>Comments and reports</h3>
        When you examine the results of a simulation,
        you may find places where the BOINC client
        made bad scheduling or work-fetch decisions.
        Or you find may places where the simulator doesn't
        seem to be working correctly.
        In such cases, please add a <b>comment</b> to the simulation,
        indicating the nature of the problem
        and the simulation time when it occurred.
        <p>
        Please post to the <a href=http://lists.ssl.berkeley.edu/mailman/listinfo/boinc_dev>boinc_dev</a> email list if
        <ul>
        <li> a simulation reveals a bug or bad scheduling decision
        <li> you have problems using BCE
        <li> BCE doesn't do more or less the same thing as your client
        <li> you have suggestions for new features.
        </ul>
        <hr>
    ";
    show_button(
        "sim_web.php?action=create_scenario_form",
        "Create a scenario", "Create a new scenario"
    );
    echo "
        <h3>Existing scenarios</h3>
    ";
    start_table();
    echo "
        <tr>
            <th>ID<br><span class=note>Click to see simulations</span></th>
            <th>Who</th>
            <th>When</th>
            <th># Simulations</th>
            <th>Description</th>
        </tr>
    ";
    $d = opendir("scenarios");
    while (false !== ($f = readdir($d))) {
        if ($f === ".") continue;
        if ($f === "..") continue;
        show_scenario_summary($f);
    }
    echo "</table>\n";
    page_tail();
}

// show form for creating a scenario
//
function create_scenario_form() {
    get_logged_in_user();
    page_head("Create a scenario");
    echo "
        To create a scenario:
        choose the input files,
        enter a short description, and click OK
        (items with * are required).
        <form action=sim_web.php method=post enctype=\"multipart/form-data\">
        <input type=hidden name=action value=create_scenario>
        <table>
    ";
    row2("* client_state.xml", "<input name=client_state type=file>");
    row2("global_prefs.xml", "<input name=global_prefs type=file>");
    row2("global_prefs_override.xml", "<input name=global_prefs_override type=file>");
    row2("cc_config.xml", "<input name=cc_config type=file>");
    row2("* Description", "<textarea name=description cols=40></textarea>");
    row2("", "<input type=submit value=OK>");
    echo "
        </table>
        </form>
    ";
    page_tail();
}

// create a subdir $dir/N for the first available N
//
function create_dir_seqno($dir) {
    $i = -1;
    $d = opendir($dir);
    while (false !== ($f = readdir($d))) {
        $j = -1;
        $n = sscanf($f, "%d", $j);
        if ($n == 1 && $j >= 0) {
            if ($j > $i) {
                $i = $j;
            }
        }
    }
    $i++;
    $p = "$dir/$i";
    mkdir($p);
    return "$i";
}

// choose scenario name
// create dir, put files there
// create meta-data file
// redirect to show scenario
//
function create_scenario() {
    $user = get_logged_in_user();
    $csname = $_FILES['client_state']['tmp_name'];
    if (!is_uploaded_file($csname)) {
        error_page("You must specify a client_state.xml file.");
    }
    $desc = post_str("description", true);
    if (!strlen($desc)) {
        error_page("You must supply a description.");
    }
    $desc = strip_tags($desc);
    $sname = create_dir_seqno("scenarios");
    $d = "scenarios/$sname";
    move_uploaded_file($csname, "$d/client_state.xml");
    $gp = $_FILES['global_prefs']['tmp_name'];
    if (is_uploaded_file($gp)) {
        move_uploaded_file($gp, "$d/global_prefs.xml");
    }
    $gpo = $_FILES['global_prefs_override']['tmp_name'];
    if (is_uploaded_file($gpo)) {
        move_uploaded_file($gpo, "$d/global_prefs_override.xml");
    }
    $ccc = $_FILES['cc_config']['tmp_name'];
    if (is_uploaded_file($ccc)) {
        move_uploaded_file($ccc, "$d/cc_config.xml");
    }
    file_put_contents("$d/userid", "$user->id");
    file_put_contents("$d/description", $desc);
    mkdir("$d/simulations");
    header("Location: sim_web.php?action=show_scenario&name=$sname");
}

function show_simulation_summary($scen, $sim) {
    $dir = "scenarios/$scen/simulations/$sim";
    $userid = (int)file_get_contents("$dir/userid");
    $user = BoincUser::lookup_id($userid);
    $date = date_str(filemtime($dir));

    echo "<tr>
        <td><a href=sim_web.php?action=show_simulation&scen=$scen&sim=$sim>$sim</a></td>
        <td>$user->name</td>
        <td>$date</td>
        <td><pre>".file_get_contents("$dir/inputs.txt")."
        <td><pre>".file_get_contents("$dir/results.txt")."
        <td>".get_comments($dir)."
        </tr>
    ";
}

// show:
// links to files
// list of existing simulations
// link for new simulation
//
function show_scenario() {
    $name = get_str("name");
    $d = "scenarios/$name";
    if (!is_dir($d)) {
        error_page("No such scenario");
    }
    page_head("Scenario $name");
    $desc = file_get_contents("scenarios/$name/description");
    $userid = (int)file_get_contents("scenarios/$name/userid");
    $user = BoincUser::lookup_id($userid);
    $date = date_str(filemtime("scenarios/$name"));
    start_table();
    row2("Creator", $user->name);
    row2("When", $date);
    row2("Description", $desc);
    $x = "<a href=$d/client_state.xml>client_state.xml</a>";
    if (file_exists("$d/global_prefs.xml")) {
        $x .= "<br><a href=$d/global_prefs.xml>global_prefs.xml</a>\n";
    }
    if (file_exists("$d/global_prefs_override.xml")) {
        $x .= "<br><a href=$d/global_prefs_override.xml>global_prefs_override.xml</a>\n";
    }
    if (file_exists("$d/cc_config.xml")) {
        $x .= "<br><a href=$d/cc_config.xml>cc_config.xml</a>\n";
    }
    row2("Input files", $x);
    end_table();
    show_button("sim_web.php?action=simulation_form_short&scen=$name",
        "Do new simulation",
        "Do new simulation"
    );
    echo "<h3>Simulations</h3>";
    start_table();
    echo "<tr>
            <th>ID<br><span class=note>Click for details</span></th>
            <th>Who</th>
            <th>When</th>
            <th>Parameters</th>
            <th>Results</th>
            <th>Comments</th>
        </tr>
    ";
    $s = opendir("$d/simulations");
    while (false !== ($f = readdir($s))) {
        if (!is_numeric($f)) continue;
        show_simulation_summary($name, $f);
    }
    end_table();

    page_tail();
}

function log_flag_boxes() {
    return "
        <input type=checkbox name=cpu_sched_debug> CPU scheduling debug
        <br> <input type=checkbox name=rr_simulation> Round-robin simulation info
        <br> <input type=checkbox name=rrsim_detail> Round-robin simulation details
        <br> <input type=checkbox name=work_fetch_debug> Work fetch debug
    ";
}

function simulation_form_short() {
    $scen = get_str("scen");
    page_head("Do simulation");
    start_table();
    echo "
        <form action=sim_web.php method=post enctype=\"multipart/form-data\">
        <input type=hidden name=action value=simulation_action>
        <input type=hidden name=scen value=$scen>
        <input type=hidden name=rec_half_life_days value=10>
        <input type=hidden name=existing_jobs_only value=0>
        <input type=hidden name=use_hyst_fetch value=1>
        <input type=hidden name=cpu_sched_rr_only value=0>
        <input type=hidden name=server_uses_workload value=0>
    ";
    row2("Duration", "<input name=duration value=86400> seconds");
    row2("Time step", "<input name=delta value=60> seconds");
    row2("Log flags", log_flag_boxes());
    row2("", "<input type=submit value=OK>");
    echo "</form>\n";
    end_table();
    page_tail();
}

function simulation_form() {
    $scen = get_str("scen");
    page_head("Do simulation");
    start_table();
    echo "
        <form action=sim_web.php method=post enctype=\"multipart/form-data\">
        <input type=hidden name=action value=simulation_action>
        <input type=hidden name=scen value=$scen>
    ";
    row2("Duration", "<input name=duration value=86400> seconds");
    row2("Time step", "<input name=delta value=60> seconds");
    row2("Half life of average-credit decay", "<input name=rec_half_life_days value=10> days");
    row2("cc_config.xml", "<input type=file name=cc_config>");
    row2("Existing jobs only?
        <br><span class=note>If checked, simulate only the
        jobs in the client state file.
        Otherwise, simulate an infinite stream of jobs
        modeled after those in the state file.",
        "<input type=checkbox name=existing_jobs_only>"
    );
    row2("Use hysteresis work fetch?
        <br><span class=note>If checked, use 6.14 work fetch policies.
        Tthe client will wait
        until the work buffer falls below X, then fill it to X+Y.
        Otherwise it will keep it filled to X+Y.</span>",
        "<input type=checkbox name=use_hyst_fetch checked>"
    );
    row2("Scheduler does detailed deadline check?
        <br><span class=note>If checked, the scheduler's deadline
        decisions will use a detailed EDF simulation
        rather than an approximation.</span>",
        "<input type=checkbox name=server_uses_workload>"
    );
    row2("Client uses pure Round-robin?
        <br><span class=note>If checked, CPU scheduling will
        use a simple round-robin policy.</span>",
        "<input type=checkbox name=cpu_sched_rr_only>"
    );
    row2("", "<input type=submit value=OK>");
    echo "</form>\n";
    end_table();
    page_tail();
}

// create directory for simulation,
// run simulation,
// redirect to simulation page
//
function simulation_action() {
    $user = get_logged_in_user();
    $scen = post_str("scen");
    if (!is_dir("scenarios/$scen")) {
        error_page("no such scenario");
    }
    $sim_dir = "scenarios/$scen/simulations";
    $sim_name = create_dir_seqno($sim_dir);
    $sim_path = "$sim_dir/$sim_name";
    $policy = new POLICY("");
    $policy->duration = (double)post_str("duration");
    $policy->delta = (double)post_str("delta");
    $policy->rec_half_life = (double)post_str("rec_half_life_days")*86400;
    $policy->existing_jobs_only = post_str("existing_jobs_only", true);
    $policy->use_hyst_fetch = post_str("use_hyst_fetch", true);
    $policy->cpu_sched_rr_only = post_str("cpu_sched_rr_only", true);
    $policy->server_uses_workload = post_str("server_uses_workload", true);
    file_put_contents("$sim_path/userid", "$user->id");

    $x = "<log_flags>\n";
    if (post_str("cpu_sched_debug", true)) {
        $x .= "<cpu_sched_debug/>\n";
    }
    if (post_str("rr_simulation", true)) {
        $x .= "<rr_simulation/>\n";
    }
    if (post_str("rrsim_detail", true)) {
        $x .= "<rrsim_detail/>\n";
    }
    if (post_str("work_fetch_debug", true)) {
        $x .= "<work_fetch_debug/>\n";
    }
    $x .= "</log_flags>\n";
    file_put_contents("$sim_path/log_flags.xml", $x);

    do_sim("scenarios/$scen", $sim_path, $policy);
    header("Location: sim_web.php?action=show_simulation&scen=$scen&sim=$sim_name");
}

// show links to files in simulation directory
//
function show_simulation() {
    $scen = get_str("scen");
    $sim = get_str("sim");
    $dir = "scenarios/$scen/simulations/$sim";
    if (!is_dir($dir)) {
        error_page("No such simulation");
    }
    page_head("Simulation $sim");
    start_table();
    $userid = (int)file_get_contents("$dir/userid");
    $user = BoincUser::lookup_id($userid);
    $date = date_str(filemtime($dir));

    row2("Scenario", "<a href=sim_web.php?action=show_scenario&name=$scen>$scen</a>");
    row2("Who", $user->name);
    row2("When", $date);
    row2("Parameters", "<pre>".file_get_contents("$dir/inputs.txt")."</pre>");
    row2("Results", "<pre>".file_get_contents("$dir/results.txt")."</pre>");

    $x = file_get_contents("$dir/index.html");
    $x = str_replace("<h3>Output files</h3>", "", $x);
    $x = str_replace("href=", "href=scenarios/$scen/simulations/$sim/", $x);
    row2("Output files", $x);
    $x = get_comments($dir);
    if ($x) {
        row2("Comments", $x);
    }
    echo "<form action=sim_web.php>
        <input type=hidden name=scen value=$scen>
        <input type=hidden name=sim value=$sim>
        <input type=hidden name=action value=add_comment>
    ";
    row2("<input type=submit value=\"Add comment\">",
        "<textarea name=comment></textarea>"
    );
    echo "</form>";
    end_table();
    page_tail();
}

function add_comment() {
    $user = get_logged_in_user();
    $scen = get_str("scen");
    $sim = get_str("sim");
    $dir = "scenarios/$scen/simulations/$sim";
    if (!is_dir($dir)) {
        error_page("No such simulation");
    }
    $cdir = "$dir/comments";
    @mkdir($cdir);
    $c = create_dir_seqno($cdir);
    $p = "$cdir/$c";
    file_put_contents("$p/userid", "$user->id");
    file_put_contents("$p/comment", get_str("comment"));

    header("Location: sim_web.php?action=show_simulation?scen=$scen&sim=$sim");
}

$action = get_str("action", true);
if (!$action) $action = post_str("action", true);

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
case "simulation_form_short":
    simulation_form_short();
    break;
case "simulation_action":
    simulation_action();
    break;
case "show_simulation":
    show_simulation();
    break;
case "add_comment":
    add_comment();
    break;
default:
    show_scenarios();
}

?>
