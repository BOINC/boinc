<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2010 University of California
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


// This file contains support functions for display and editing
// preferences (global and project).
// Preferences are represented in two ways:
// - As a PHP structure (usually called $prefs)
//   This has fields run_if_user_active, etc.
//   The fields "project_specific" is plain XML
// - As XML (usually called $prefs_xml)
//
// This XML has the general structure
// <global_preferences>
//    <mod_time>...</mod_time>
//    <run_if_user_active/>
//    <work_buf_min_days>1.3</work_buf_min_days>
//    ...
//    <venue name="home">
//       <run_if_user_active/>
//       ...
//    </venue>
// </global_preferences>
//
// and
//
// <project_preferences>
//    <resource_share>4</resource_share>
//    [ <allow_beta_work>0|1</allow_beta_work> ]
//    [ <no_cpu>1</no_cpu> ]
//    [ <no_cuda>1</no_cuda> ]
//    [ <no_ati>1</no_ati> ]
//    <project-specific>
//            ... (arbitrary project-specific XML)
//    </project-specific>
//    <home>
//       ...
//    </home>
// </project_preferences>
//
// In addition there are some fields of the user table
// (send_email and show_hosts) that are treated as project preferences

// Various functions are defined below for converting between these forms,
// and also to/from HTML form elements

error_reporting(E_ALL);
ini_set('display_errors', true);
ini_set('display_startup_errors', true);

include_once("../inc/prefs_util.inc");
include_once("../project/project_specific_prefs.inc");

$app_types = get_app_types();

$venues = array("home", "school", "work");

$cpu_prefs = array(
    new PREF_BOOL(
        tra(
            "Suspend work while computer is on battery power? %1 Matters only for portable computers %2",
            "<br><span class=note>",
            "</span>"
        ),
        "run_on_batteries",
        false, true
    ),
    new PREF_BOOL(
        tra("Suspend work while computer is in use?"),
        "run_if_user_active",
        true, true
    ),
    new PREF_BOOL(
        tra(
            "Suspend GPU work while computer is in use? %1 Enforced by version 6.6.21+ %2",
            "<br><span class=note>",
            "</span>"
        ),
        "run_gpu_if_user_active",
        false, true
    ),
    new PREF_NUM(
        tra("'In use' means mouse/keyboard activity in last"),
        "idle_time_to_run",
        new NUM_SPEC(tra("minutes"), 1, 9999, 3)
    ),
    new PREF_NUM(
        tra(
            "Suspend work if no mouse/keyboard activity in last %1 Needed to enter low-power mode on some computers %2",
            "<br><span class=note>",
            "</span>"
        ),
        "suspend_if_no_recent_input",
        new NUM_SPEC(tra("minutes"), 0, 9999, 0)
    ),
    new PREF_NUM(
        tra(
            "Suspend work if CPU usage is above %1 0 means no restriction<br>Enforced by version 6.10.30+ %2",
            "<br><span class=note>",
            "</span>"
        ),
        "suspend_cpu_usage",
        new NUM_SPEC("%", 0, 100, 25)
    ),
    new PREF_HOUR_RANGE(
        tra("Do work only between the hours of %1 No restriction if equal %2",
            "<br><span class=note>",
            "</span>"
        ),
        "start_hour", "end_hour"
    ),
    new PREF_BOOL(
        tra(
            "Leave tasks in memory while suspended? %1 Suspended tasks will consume swap space if 'yes' %2",
            "<br><span class=note>",
            "</span>"
        ),
        "leave_apps_in_memory",
        false
    ),
    new PREF_NUM(
        tra(
            "Switch between tasks every %1 Recommended: 60 minutes %2",
            "<br><span class=note>",
            "</span>"
        ),
        "cpu_scheduling_period_minutes",
        new NUM_SPEC(tra("minutes"), 1, 9999, 60)
    ),
    new PREF_NUM(
        tra("On multiprocessors, use at most"),
        "max_cpus",
        new NUM_SPEC(tra("processors"), 0, 9999, 0)
    ),
    new PREF_NUM(
        tra(
            "On multiprocessors, use at most %1 Enforced by version 6.1+ %2",
            "<br><span class=note>",
            "</span>"
        ),
        "max_ncpus_pct",
        new NUM_SPEC(tra("% of the processors"), 0, 100, 100)
    ),
    new PREF_NUM(
        tra(
            "Use at most %1 Can be used to reduce CPU heat %2",
            "<br><span class=note>",
            "</span>"
        ),
        "cpu_usage_limit",
        new NUM_SPEC(tra("% of CPU time"), 0, 100, 100)
    ),
);

$dp = get_disk_space_config();

$disk_prefs = array(
    new PREF_NUM(
        tra("Disk: use at most"),
        "disk_max_used_gb",
        new NUM_SPEC(tra("GB"), 0, 9999999, $dp->disk_max_used_gb)
    ),
    new PREF_NUM(
        tra("Disk: leave free at least %1 Values smaller than %2 are ignored %3",
            "<br><span class=note>",
            "0.001",
            "</span>"
        ),
        "disk_min_free_gb",
        new NUM_SPEC(tra("GB"), 0.001, 9999999, $dp->disk_min_free_gb)
    ),
    new PREF_NUM(
        tra("Disk: use at most"),
        "disk_max_used_pct",
        new NUM_SPEC(tra("% of total"), 0, 100, $dp->disk_max_used_pct)
    ),
    new PREF_NUM(
        tra("Tasks checkpoint to disk at most every"),
        "disk_interval",
        new NUM_SPEC(tra("seconds"), 0, 9999999, 60)
    ),
    new PREF_NUM(
        tra("Swap space: use at most"),
        "vm_max_used_pct",
        new NUM_SPEC(tra("% of total"), 0, 100, 75)
    ),
    new PREF_NUM(
        tra("Memory: when computer is in use, use at most"),
        "ram_max_used_busy_pct",
        new NUM_SPEC(tra("% of total"), 0, 100, 50)
    ),
    new PREF_NUM(
        tra("Memory: when computer is not in use, use at most"),
        "ram_max_used_idle_pct",
        new NUM_SPEC(tra("% of total"), 0, 100, 90)
    ),
);

$net_prefs = array(
    new PREF_NUM(
        tra(
            "Computer is connected to the Internet about every %1 Leave blank or 0 if always connected. %2 BOINC will try to maintain at least this much work. %3",
            "<br><span class=note>",
            "<br>",
            "</span>"
        ),
        "work_buf_min_days",
        new NUM_SPEC(tra("days"), 0, 10, 0)
    ),
    new PREF_NUM(
        tra("Maintain enough work for an additional"),
        "work_buf_additional_days",
        new NUM_SPEC(tra("days"), 0, 10, .25)
    ),
    new PREF_BOOL(
        tra(
            "Confirm before connecting to Internet? %1 Matters only if you have a modem, ISDN or VPN connection %2",
            "<br><span class=note>",
            "</span>"
        ),
        "confirm_before_connecting",
        false
    ),
    new PREF_BOOL(
        tra(
            "Disconnect when done? %1 Matters only if you have a modem, ISDN or VPN connection %2",
            "<br><span class=note>",
            "</span>"
        ),
        "hangup_if_dialed",
        false
    ),
    new PREF_NUM(
        tra("Maximum download rate:"),
        "max_bytes_sec_down",
        new NUM_SPEC(tra("Kbytes/sec"), 0, 9999999, 0, 1000)
    ),
    new PREF_NUM(
        tra("Maximum upload rate:"),
        "max_bytes_sec_up",
        new NUM_SPEC(tra("Kbytes/sec"), 0, 9999999, 0, 1000)
    ),
    new PREF_HOUR_RANGE(
        tra("Use network only between the hours of"),
        "net_start_hour", "net_end_hour"
    ),
    new PREF_NUM2(
        tra("Transfer at most %1 Enforced by version 6.10.46+ %2",
            "<br><span class=note>",
            "</span>"
        ),
        "daily_xfer_limit_mb",
        "daily_xfer_period_days",
        new NUM_SPEC(tra("Mbytes every"), 0, 9999999, 0),
        new NUM_SPEC(tra("days"), 0, 9999999, 0)
    ),
    new PREF_BOOL(
        tra(
            "Skip image file verification? %1 Check this ONLY if your Internet provider modifies image files (UMTS does this, for example). %2 Skipping verification reduces the security of BOINC. %3",
            "<br><span class=note>",
            "",
            "</span>"
        ),
        "dont_verify_images",
        false
    ),
);

$project_pref_descs = array(
    new PREF_NUM(
        $x = tra(
            "Resource share %1 Determines the proportion of your computer's resources allocated to this project. Example: if you participate in two BOINC projects with resource shares of 100 and 200, the first will get 1/3 of your resources and the second will get 2/3. %2",
            "<br><span class=note>",
            "</span>"
        ),
        "resource_share",
        new NUM_SPEC("", 0, 9999999, 100)
    ),
);

if ($app_types->count > 1) {
    if ($app_types->cpu) {
        $project_pref_descs[] = new PREF_BOOL (
            tra(
                "Use CPU %1 Enforced by version 6.10+ %2",
                "<br><span class=note>",
                "</span>"
            ),
            "no_cpu",
            false,
            true
        );
    }
    if ($app_types->ati) {
        $project_pref_descs[] = new PREF_BOOL (
            tra(
                "Use ATI GPU %1 Enforced by version 6.10+ %2",
                "<br><span class=note>",
                "</span>"
            ),
            "no_ati",
            false,
            true
        );
    }
    if ($app_types->cuda) {
        $project_pref_descs[] = new PREF_BOOL (
            tra(
                "Use NVIDIA GPU %1 Enforced by version 6.10+ %2",
                "<br><span class=note>",
                "</span>"
            ),
            "no_cuda",
            false,
            true
        );
    }
}

if (isset($project_has_beta) && $project_has_beta) {
    $project_pref_descs[] = new PREF_BOOL(
        tra(
            "Run test applications? %1 This helps us develop applications, but may cause jobs to fail on your computer %2",
            "<br><span class=note>",
            "</span>"
        ),
        "allow_beta_work",
        false
    );
}

define("DISK_LIMIT_DESC", tra("Disk and memory usage"));
define("CPU_LIMIT_DESC", tra("Processor usage"));
define("NETWORK_LIMIT_DESC", tra("Network usage"));

// These texts are used in multiple places in prefs_edit.php and add_venue.php
define("PREFS_FORM_DESC1", tra("These preferences apply to all the BOINC projects in which you participate.")."<br><br>");
define("PREFS_FORM_ERROR_DESC",
    tra(
        "%1Unable to update preferences.%2 The values marked in red below were out of range or not numeric.",
        "<strong>",
        "</strong>"
    ).
    "<br><br>"
);

global $text;
global $parse_result;
global $top_parse_result;
global $in_project_specific;
global $venue_name;

// get default settings for disk space usage so the default user
// preferences match the settings used by the scheduler.
// Defaults are set if the tags are missing, they depend on
// which scheduler is running:
// - 'old' has the default hardcoded 
// - 'new' uses config settings
// if running the old scheduler, set <scheduler_disk_space_check_hardcoded>
// in config.xml so the right default is set for minimum free disk space
// 
function get_disk_space_config() {
    global  $config;
    $config = get_config();
    $dp->disk_max_used_gb = parse_config($config, "<default_disk_max_used_gb>");
    $dp->disk_max_used_pct = parse_config($config, "<default_disk_max_used_pct>");
    $dp->disk_min_free_gb = parse_config($config, "<default_disk_min_free_gb>");
    // set some defaults if not found
    if (!$dp->disk_max_used_gb) $dp->disk_max_used_gb = 100;  // 100 gb
    if (!$dp->disk_max_used_pct) $dp->disk_max_used_pct = 50; // 50 percent
    if (!$dp->disk_min_free_gb) $dp->disk_min_free_gb=.001;   // 1 megabyte
    // set mininimum free space scheduler allows 
    // - depends on which scheduler is running
    $dp->new_sched_flag = 1;
    $dp->sched_disk_min_free_gb = $dp->disk_min_free_gb;
    if (parse_config($config, "scheduler_disk_space_check_hardcoded>")) {
        $dp->new_sched_flag = 0;
        $dp->sched_disk_min_free_gb = 0;
    }    
       
    return $dp;
}


function check_venue($x) {
    if ($x == "home") return;
    if ($x == "work") return;
    if ($x == "school") return;
    error_page(tra("bad venue: %1", $x));
}

function check_subset($x) {
    if ($x == "global") return;
    if ($x == "project") return;
    error_page(tra("bad subset: %1", $x));
}

// functions to parse preferences XML into a struct
//
function element_start_project($parser, $name, $attrs) {
    global $top_parse_result;
    global $parse_result;
    global $text;
    global $in_project_specific;
    global $venue_name;

    switch($name) {
    case "venue":
        $venue_name = $attrs["name"];
        $top_parse_result = $parse_result;
        $parse_result = default_prefs_project();
        break;
    case "project_specific":
        $in_project_specific = 1;
        $text = "";
        break;
    default:
        if ($in_project_specific) {
            $text= $text."<$name>";
        } else {
            $text = "";
        }
    }
}

function element_start_global($parser, $name, $attrs) {
    global $top_parse_result;
    global $parse_result;
    global $text;
    global $venue_name;

    switch($name) {
    case "venue":
        $venue_name = $attrs["name"];
        $top_parse_result = $parse_result;
        $parse_result = default_prefs_global();
        break;
    }
    $text = "";
}

function element_end_project($parser, $name) {
    global $text;
    global $parse_result;
    global $in_project_specific;
    global $top_parse_result;
    global $venue_name;
    global $project_pref_descs;

    foreach($project_pref_descs as $p) {
        if ($p->xml_parse($parse_result, $name, $text)) {
            return;
        }
    }
    switch($name) {
    case "venue":
        $top_parse_result->$venue_name = $parse_result;
        $parse_result = $top_parse_result;
        break;
    case "project_specific":
        $parse_result->project_specific = $text;
        $in_project_specific = false;
        break;
    case "project_preferences":
        break;
    default:
        if ($in_project_specific) {
            $text = $text."</$name>";
        } else {
            //echo "Unknown tag: $name\n";
        }
    }
}

function element_end_global($parser, $name) {
    global $text;
    global $parse_result;
    global $top_parse_result;
    global $venue_name;
    global $cpu_prefs;
    global $disk_prefs;
    global $disk_prefs;
    global $net_prefs;

    foreach ($cpu_prefs as $p) {
        if ($p->xml_parse($parse_result, $name, $text)) {
            return;
        }
    }
    foreach ($disk_prefs as $p) {
        if ($p->xml_parse($parse_result, $name, $text)) {
            return;
        }
    }
    foreach ($net_prefs as $p) {
        if ($p->xml_parse($parse_result, $name, $text)) {
            return;
        }
    }
    switch($name) {
    case "venue":
        $top_parse_result->$venue_name = $parse_result;
        $parse_result = $top_parse_result;
        break;
    case "mod_time":
        $parse_result->mod_time = $text;
        break;
    case "global_preferences":
        break;
    default:
        //echo "Unknown tag: $name\n";
    }
}

function char_handler($parser, $x) {
    global $text;
    $text = $text.$x;
}


// state of prefs before parsing; defines prefs for new users
//
function default_prefs_global() {
    global $cpu_prefs;
    global $disk_prefs;
    global $net_prefs;

    $p = null;
    foreach ($cpu_prefs as $pref) {
        $pref->set_default($p);
    }
    foreach ($disk_prefs as $pref) {
        $pref->set_default($p);
    }
    foreach ($net_prefs as $pref) {
        $pref->set_default($p);
    }
    return $p;
}

function default_prefs_project() {
    global $project_pref_descs;

    $p = null;
    foreach($project_pref_descs as $pref) {
        $pref->set_default($p);
    }
    $p->project_specific = project_specific_prefs_default();
    return $p;
}

// parse prefs from XML to a struct
//
function prefs_parse_project($prefs_xml) {
    global $parse_result;
    $parse_result = default_prefs_project();
    $xml_parser = xml_parser_create();
    xml_parser_set_option($xml_parser, XML_OPTION_CASE_FOLDING, 0);
    xml_set_element_handler($xml_parser, "element_start_project", "element_end_project");
    xml_set_character_data_handler($xml_parser, "char_handler");
    xml_parse($xml_parser, $prefs_xml, 1);
    return $parse_result;
}

function prefs_parse_global($prefs_xml) {
    global $parse_result;
    $parse_result = default_prefs_global();
    $xml_parser = xml_parser_create();
    xml_parser_set_option($xml_parser, XML_OPTION_CASE_FOLDING, 0);
    xml_set_element_handler($xml_parser, "element_start_global", "element_end_global");
    xml_set_character_data_handler($xml_parser, "char_handler");
    xml_parse($xml_parser, $prefs_xml, 1);
    return $parse_result;
}

////////////////////////////////////////////
//
// display preference subsets as Columns
//
function row_top($x, $ncols=6, $class="heading") {
    echo "<tr><td class=$class width=35%>$x</td>";
    echo "<td class=$class width=10%><b>Default</b></td>
        <td class=$class width=10%><b>Home</b></td>
        <td class=$class width=10%><b>School</b></td>
        <td class=$class width=10%><b>Work</b></td>";
    echo "<td width=15%><br></td></tr>\n";
}

//
// row_defs - Display a value for all 4 venues in one row
//
function row_defs($pre, $item, $post, $type, $prefs) {
    $gen = $prefs->$item;
    $hom  = (isset($prefs->home) && isset($prefs->home->$item)) ? $prefs->home->$item : "--";
    $schl = (isset($prefs->school) && isset($prefs->school->$item)) ? $prefs->school->$item : "--";
    $wrk  = (isset($prefs->work) && isset($prefs->work->$item)) ? $prefs->work->$item : "--";

    echo "<tr><td class=fieldname>$pre</td>";
    row_field($gen,  $type);
    row_field($hom,  $type);
    row_field($schl, $type);
    row_field($wrk,  $type);
    echo "<td align=left>$post</td></tr>\n";
}

//
// row_field - Display each field value, with selectable display modes
//
function row_field($value, $type) {
    echo "<td class=f_val valign=top>";
    $type = $value === "--" ? "--" : $type;
    switch($type) {
    case "yesno":
        echo $value ?tra("yes"):tra("no");
        break;
    case "noyes":
        echo $value ?tra("no"):tra("yes");
        break;
    case "limit":
        $x = max_bytes_display_mode($value);
        $y = "$x " . BYTE_ABBR;
        echo $x ? "$y" : tra("no limit");
        break;
    case "minutes":
        if ($value) {
            echo $value;
        } else {
            echo '--';
        }
        break;
    default:
        echo $value;
        break;
    }
    echo "</td>";
}

//
// row_links - Display Edit/Add/Remove links for all venues in 1 row
//
function row_links($subset, $prefs) {
    global $g_logged_in_user;
    $tokens = url_tokens($g_logged_in_user->authenticator);
    $pre_add  = "<a href=add_venue.php?venue=";
    $pre_edit  = "<a href=prefs_edit.php?venue=";
    $pre_remove = "<a href=prefs_remove.php?venue=";
    $post_add = "&amp;subset=$subset&cols=1$tokens>".tra("Add")."</a>";
    $post_edit = "&amp;subset=$subset&cols=1$tokens>".tra("Edit")."</a>";
    $post_remove = "&amp;subset=$subset&cols=1$tokens>".tra("Remove")."</a>";
    $gen = "<a href=prefs_edit.php?subset=$subset&amp;cols=1$tokens>".tra("Edit")."</a>";

    $hom  = isset($prefs->home) ? $pre_edit."home".$post_edit : $pre_add."home".$post_add;
    $schl = isset($prefs->school) ? $pre_edit."school".$post_edit : $pre_add."school".$post_add;
    $wrk  = isset($prefs->work) ? $pre_edit."work".$post_edit : $pre_add."work".$post_add;

    echo "<tr><td class=fieldname> </td>";
    echo "<td>$gen</td>";
    echo "<td>$hom</td>";
    echo "<td>$schl</td>";
    echo "<td>$wrk</td>";
    echo "<td><br></td></tr>\n";

    $hom  = isset($prefs->home) ? $pre_remove."home".$post_remove : "<br>";
    $schl = isset($prefs->school) ? $pre_remove."school".$post_remove : "<br>";
    $wrk  = isset($prefs->work) ? $pre_remove."work".$post_remove : "<br>";

    echo "<tr><td class=fieldname> </td>";
    echo "<td> </td>";
    echo "<td>$hom</td>";
    echo "<td>$schl</td>";
    echo "<td>$wrk</td>";
    echo "<td><br></td></tr>\n";
}

// Display all venues as columns next to descriptions
//
function prefs_show_columns_global($prefs) {
    global $cpu_prefs;
    global $disk_prefs;
    global $net_prefs;

    row_top(CPU_LIMIT_DESC);
    foreach ($cpu_prefs as $p) {
        $p->show_cols($prefs);
    }
    row_top(DISK_LIMIT_DESC);
    foreach ($disk_prefs as $p) {
        $p->show_cols($prefs);
    }
    row_top(NETWORK_LIMIT_DESC);
    foreach ($net_prefs as $p) {
        $p->show_cols($prefs);
    }
    row_links("global", $prefs);
}

function prefs_show_global($prefs) {
    global $cpu_prefs;
    global $disk_prefs;
    global $net_prefs;

    row1(CPU_LIMIT_DESC);
    foreach ($cpu_prefs as $p) {
        $p->show($prefs);
    }
    row1(DISK_LIMIT_DESC);
    foreach ($disk_prefs as $p) {
        $p->show($prefs);
    }
    row1(NETWORK_LIMIT_DESC);
    foreach ($net_prefs as $p) {
        $p->show($prefs);
    }
}

function prefs_show_project($prefs, $columns=false) {
    global $project_pref_descs;
    if ($columns) {
        foreach ($project_pref_descs as $p) {
            $p->show_cols($prefs);
        }
    } else {
        foreach ($project_pref_descs as $p) {
            $p->show($prefs);
        }
    }
}

function prefs_show_privacy($user) {
    $x = "";
    if (defined('EMAIL_FROM')) {
        $x = "<br><span class=note>".
            tra("Emails will be sent from %1; make sure your spam filter accepts this address.", EMAIL_FROM).
            "</span>";
    }
    row2(tra("Is it OK for %1 and your team (if any) to email you?", PROJECT)." $x",
        $user->send_email?tra("yes"):tra("no")
    );
    row2(tra("Should %1 show your computers on its web site?", PROJECT), $user->show_hosts?tra("yes"):tra("no"));
}

function prefs_show_project_specific($prefs, $columns=false) {
    if ($columns) {
    $project_specific_prefs = project_specific_prefs_parse($prefs->project_specific);
        $project_specific_prefs->home = isset($prefs->home) ? project_specific_prefs_parse($prefs->home->project_specific) : "";
        $project_specific_prefs->work = isset($prefs->work) ? project_specific_prefs_parse($prefs->work->project_specific) : "";
        $project_specific_prefs->school = isset($prefs->school) ? project_specific_prefs_parse($prefs->school->project_specific) : "";
    } else {
        $project_specific_prefs = project_specific_prefs_parse($prefs->project_specific);
    }
    project_specific_prefs_show($project_specific_prefs, $columns);
}

function subset_name($subset) {
    if ($subset == "global") return tra("Computing");
    return PROJECT;
}

function prefs_display_venue($prefs, $venue, $subset) {
    global $g_logged_in_user;
    $tokens = url_tokens($g_logged_in_user->authenticator);
    $x = false;
    if (isset($prefs->$venue)) $x = $prefs->$venue;

    if ($x) {
        row1(tra("Separate preferences for %1", $venue), 2, "heading");
        echo "<tr><td colspan=2>";
        start_table();
        if ($subset == "global") {
            prefs_show_global($x);
        } else {
            prefs_show_project($x);
            prefs_show_project_specific($x);
        }
        row2("<br>", "<a href=prefs_edit.php?venue=$venue&amp;subset=$subset$tokens>".tra("Edit preferences")."</a> | <a href=prefs_remove.php?venue=$venue&amp;subset=$subset$tokens>".tra("Remove")."</a>");
        end_table();
        echo "</td></tr>\n";
    } else {
        row1("<a href=add_venue.php?venue=$venue&amp;subset=$subset$tokens>".tra("Add separate preferences for %1", $venue)."</a>", 2, "heading");
    }
}

function print_prefs_display_project($user, $columns=false) {
    $project_prefs = prefs_parse_project($user->project_prefs);

    start_table();
    $switch_link = " <font size=\"-2\"><a href=prefs.php?subset=project&cols=". (int)!$columns .">".tra("(Switch View)")."</a></font>";
    if ($columns) {
        row1(tra("Combined preferences").$switch_link, 2, "heading");
        echo "<tr><td colspan=2>";
        start_table();
        prefs_show_privacy($user);
        venue_show($user);
        row_top(tra("Project specific settings"));
        prefs_show_project($project_prefs, true);
        prefs_show_project_specific($project_prefs, true);
        row_links("project", $project_prefs);
        end_table();
        echo "</td></tr>\n";
    } else {
        if (isset($project_prefs->home) || isset($project_prefs->work) || isset($project_prefs->school)) {
            row1(tra("Primary (default) preferences").$switch_link, 2, "heading");
        }
        echo "<tr><td colspan=2>";
        start_table();
        prefs_show_project($project_prefs, false);
        prefs_show_privacy($user);
        venue_show($user);
        prefs_show_project_specific($project_prefs, false);
        $tokens = url_tokens($user->authenticator);
        row2("", "<a href=prefs_edit.php?subset=project$tokens>".tra("Edit %1 preferences", PROJECT)."</a>");
        end_table();
        echo "</td></tr>\n";
        prefs_display_venue($project_prefs, "home", "project");
        prefs_display_venue($project_prefs, "school", "project");
        prefs_display_venue($project_prefs, "work", "project");
    }
    end_table();
}

function print_prefs_display_global($user, $columns=false) {
    $global_prefs = prefs_parse_global($user->global_prefs);

    echo "<span class=note>".
         tra("These apply to all BOINC projects in which you participate.<br>
             On computers attached to multiple projects, the most recently modified preferences will be used.").
         "</span><br><br>";
    if (isset($global_prefs->mod_time)) {
        echo tra("Preferences last modified:")." ".pretty_time_str($global_prefs->mod_time);
    }
    echo "<br><br>";
    $switch_link = " <font size=\"-2\"><a href=prefs.php?subset=global&cols=". (int)!$columns .">".tra("(Switch View)")."</a></font>";
    start_table();
    if ($columns) {
        row1(tra("Combined preferences").$switch_link, 2, "heading");
        echo "<tr><td colspan=2>";
        start_table();
        prefs_show_columns_global($global_prefs);
        end_table();
        echo "</td></tr>\n";

    } else {
        if (isset($global_prefs->home) || isset($global_prefs->work) || isset($global_prefs->school)) {
            row1(tra("Primary (default) preferences").$switch_link, 2, "heading");
        }
        echo "<tr><td colspan=2>";
        start_table();
        prefs_show_global($global_prefs);
        $tokens = url_tokens($user->authenticator);
        row2("<br>", "<a href=prefs_edit.php?subset=global$tokens>".tra("Edit preferences")."</a>");
        end_table();
        echo "</td></tr>\n";

        prefs_display_venue($global_prefs, "home", "global");
        prefs_display_venue($global_prefs, "school", "global");
        prefs_display_venue($global_prefs, "work", "global");
    }
    end_table();
}

function print_prefs_display($user) {
    print_prefs_display_project($user);
    echo "<br><br>\n";
    print_prefs_display_global($user);
}

// This functions is used in prefs_edit.php to be able to display
// the prefs form in case of an error again.
// $error and $project_error should be an object of the form:
// $error->idle_time_to_run=true if an error occurred
// otherwise false
//
function print_prefs_form(
    $action, $subset, $venue, $user, $prefs, $cols, $error=false,
    $project_error=false
){
    if ($action == "add") {
        $script = "add_venue.php";
        $submit_value = tra("Add preferences");
    }
    if ($action == "edit") {
        $script = "prefs_edit.php";
        $submit_value = tra("Update preferences");
    }
    echo "<form action=$script><input type=hidden name=subset value=$subset>
        ".form_tokens($user->authenticator);
    if ($venue) {
        echo "<input type=hidden name=venue value=$venue>\n";
    }
    if ($cols) {
        echo "<input type=hidden name=cols value=$cols>\n";
    }
    
    start_table();
    if ($subset == "global") {
        prefs_form_global($user, $prefs, $error);
    } else {
        prefs_form_project($prefs, $error);
        if (!$venue) {
            prefs_form_privacy($user);
            venue_form($user);
        }
        prefs_form_project_specific($prefs->project_specific, $project_error);
    }

    row2("", "<input type=submit value=\"$submit_value\" name=\"action\">");
    end_table();
    echo "</form>\n";
}

////////////////////////////////////////////
//
// Functions to display preference subsets as forms
//
function prefs_form_global($user, $prefs, $error=false) {
    global $cpu_prefs;
    global $disk_prefs;
    global $net_prefs;

    row1(CPU_LIMIT_DESC);
    foreach ($cpu_prefs as $p) {
        $p->show_form_row($prefs, $error);
    }
    row1(DISK_LIMIT_DESC);
    foreach ($disk_prefs as $p) {
        $p->show_form_row($prefs, $error);
    }
    row1(NETWORK_LIMIT_DESC);
    foreach ($net_prefs as $p) {
        $p->show_form_row($prefs, $error);
    }
}

function prefs_form_privacy($user) {
    $y = prefs_form_radio_buttons("send_email", $user->send_email);
    row2(tra("Is it OK for %1 and your team (if any) to email you?", PROJECT), $y);

    $y = prefs_form_radio_buttons("show_hosts", $user->show_hosts);
    row2(tra("Should %1 show your computers on its web site?", PROJECT), $y);
}

function prefs_form_project($prefs, $error=false) {
    global $project_pref_descs;

    global $app_types;
    foreach ($project_pref_descs as $p) {
        $p->show_form_row($prefs, $error);
    }
}

function prefs_form_project_specific($prefs_xml, $error=false) {
    $prefs = project_specific_prefs_parse($prefs_xml);
    project_specific_prefs_edit($prefs, $error);
}

// returns a set of translated yes/no radio buttons for editing prefs forms
// Example: prefs_form_radio_buttons("allow_beta_work", $user->allow_beta_work);
//
// @param string $name name of the radio buttons
// @param bool $yesno toggles the preset of the buttons; true=yes, false=no
function prefs_form_radio_buttons($name, $yesno) {
    $rb = tra("yes")." <input type=radio name=$name value=yes "
        .($yesno?"checked":"")
        ."> ".tra("no")." <input type=radio name=$name value=no "
        .($yesno?"":"checked")
        .">\n";
    return $rb;
}

function venue_show($user) {
    $venue = $user->venue;
    if ($venue =='') $venue = '---';
    row2(tra("Default computer location"), $venue);
}

function venue_form($user) {
    $n=$h=$w=$s='';
    if ($user->venue == '') $n = 'selected';
    if ($user->venue == 'home') $h = 'selected';
    if ($user->venue == 'work') $w = 'selected';
    if ($user->venue == 'school') $s = 'selected';
    row2(tra("Default computer location"),
        "<select name=default_venue>
        <option value=\"\" $n>---
        <option value=home $h>".tra("Home")."
        <option value=work $w>".tra("Work")."
        <option value=school $s>".tra("School")."
        </select>"
    );
}

function venue_parse_form(&$user) {
    $user->venue = $_GET['default_venue'];
}

function venue_update($user) {
    mysql_query("update user set venue='$user->venue' where id=$user->id");
}

////////////////////////////////////////////
//
// Functions to parse form elements, modifying a preferences structure
// prefs is preferences object to modify
// returns an object with errorvalues or false in success case
//
function prefs_global_parse_form(&$prefs) {
    global $cpu_prefs;
    global $disk_prefs;
    global $net_prefs;

    $error = false;
    foreach ($cpu_prefs as $p) {
        $p->parse_form($prefs, $error);
    }
    foreach ($disk_prefs as $p) {
        $p->parse_form($prefs, $error);
    }
    foreach ($net_prefs as $p) {
        $p->parse_form($prefs, $error);
    }
    return $error;
}

function prefs_resource_parse_form(&$prefs) {
    global $project_pref_descs;
    $error = false;
    foreach ($project_pref_descs as $p) {
        $p->parse_form($prefs, $error);
    }
    return $error;
}

function prefs_privacy_parse_form(&$user) {
    $user->send_email = ($_GET['send_email'] == "yes")?1:0;
    $user->show_hosts = ($_GET['show_hosts'] == "yes")?1:0;
}

// This function parses the project specific prefs form.
// For details look into project/project_specific_prefs.inc
//
function prefs_project_parse_form(&$prefs) {
    $error = false;
    $prefs->project_specific = project_specific_prefs_parse_form($error);
    return $error;
}


////////////////////////////////////////////
//
// convert prefs from structure to XML
//
function global_prefs_make_xml($prefs, $primary=true) {
    global $cpu_prefs;
    global $disk_prefs;
    global $net_prefs;

    // N.B.: each XML entry must end with \n due to the sloppy parsing by the
    // BOINC client!!
    $xml = "";
    if ($primary) {
        $xml = "<global_preferences>\n";
        $now = time();
        $xml = $xml."<mod_time>$now</mod_time>\n";
    }

    foreach ($cpu_prefs as $p) {
        $xml .= $p->xml_string($prefs);
    }
    foreach ($disk_prefs as $p) {
        $xml .= $p->xml_string($prefs);
    }
    foreach ($net_prefs as $p) {
        $xml .= $p->xml_string($prefs);
    }

    if (isset($prefs->home)) {
        $xml = $xml."<venue name=\"home\">\n".global_prefs_make_xml($prefs->home, false)."</venue>\n";
    }
    if (isset($prefs->work)) {
        $xml = $xml."<venue name=\"work\">\n".global_prefs_make_xml($prefs->work, false)."</venue>\n";
    }
    if (isset($prefs->school)) {
        $xml = $xml."<venue name=\"school\">\n".global_prefs_make_xml($prefs->school, false)."</venue>\n";
    }
    if ($primary) {
        $xml = $xml."</global_preferences>\n";
    }
    return $xml;
}

// given a prefs structure, return the corresponding XML string
//
function project_prefs_make_xml($prefs, $primary=true) {
    global $project_pref_descs;
    $xml = "";
    if ($primary) {
        $xml = "<project_preferences>\n";
    }
    foreach ($project_pref_descs as $p) {
        $xml .= $p->xml_string($prefs);
    }
    if ($prefs->project_specific) {
        $x = trim($prefs->project_specific);
        $xml = $xml
            ."<project_specific>\n$x\n</project_specific>\n";
    }
    if (isset($prefs->home)) {
        $xml = $xml."<venue name=\"home\">\n".project_prefs_make_xml($prefs->home, false)."</venue>\n";
    }
    if (isset($prefs->work)) {
        $xml = $xml."<venue name=\"work\">\n".project_prefs_make_xml($prefs->work, false)."</venue>\n";
    }
    if (isset($prefs->school)) {
        $xml = $xml."<venue name=\"school\">\n".project_prefs_make_xml($prefs->school, false)."</venue>\n";
    }
    if ($primary) {
        $xml = $xml."</project_preferences>\n";
    }
    return $xml;
}

////////////////////////////////////////////
//
// Update user's prefs in database, from a given structure
//
function global_prefs_update(&$user, $prefs) {
    $prefs_xml = mysql_real_escape_string(global_prefs_make_xml($prefs));
    $query = "update user set global_prefs='$prefs_xml' where id=$user->id";
    $retval = mysql_query($query);
    if (!$retval) {
        echo tra("Update failed: ").htmlspecialchars($query)."\n";
        echo mysql_error();
        exit();
    }
    $user->global_prefs = $prefs_xml;
    return $retval;
}

function project_prefs_update(&$user, $prefs) {
    $prefs_xml = mysql_real_escape_string(project_prefs_make_xml($prefs));
    $retval = mysql_query("update user set project_prefs='$prefs_xml', send_email=$user->send_email, show_hosts=$user->show_hosts where id=$user->id");
    $user->project_prefs = $prefs_xml;
    return $retval;
}

?>
