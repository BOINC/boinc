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

include_once("../project/project_specific_prefs.inc");

$app_types = get_app_types();

// strings describing various preference fields
//
define("CPU_LIMIT_DESC", tra("Processor usage"));
define("SUSPEND_WHILE_ON_BATTERIES_DESC",
    tra(
        "Suspend work while computer is on battery power? %1(matters only for portable computers)%2",
        "<br><span class=note>",
        "</span>"
    )
);
define("RUN_IF_USER_ACTIVE_DESC",
    tra("Suspend work while computer is in use?")
);
define("RUN_GPU_IF_USER_ACTIVE_DESC",
    tra(
        "Suspend GPU work while computer is in use? %1Enforced by version 6.7+%2",
        "<br><span class=note>",
        "</span>"
    )
);
define("IDLE_TIME_TO_RUN_DESC", tra("'In use' means mouse/keyboard activity in last"));
define("SUSPEND_IF_NO_RECENT_INPUT_DESC",
    tra(
        "Suspend work if no mouse/keyboard activity in last %1(Needed to enter low-power mode on some computers)%2",
        "<br><span class=note>",
        "</span>"
    )
);
define("SUSPEND_CPU_USAGE_DESC",
    tra(
        "Suspend work if CPU usage is above %1 0 means no restriction<br>Enforced by version 6.10.30+%2",
        "<br><span class=note>",
        "</span>"
    )
);
define("START_END_DESC", tra("Do work only between the hours of"));
define("START_END_DESC2", "<br><span class=note>".tra("(no restriction if equal)")."</span>");
define("LEAVE_APPS_IN_MEMORY_DESC",
    tra(
        "Leave tasks in memory while suspended? %1(suspended tasks will consume swap space if 'yes')%2",
        "<br><span class=note>",
        "</span>"
    )
);
define("CPU_SCHEDULING_DESC",
    tra(
        "Switch between tasks every %1(recommended: 60 minutes)%2",
        "<br><span class=note>",
        "</span>"
    )
);
define("CONFIRM_BEFORE_CONNECTING_DESC",
    tra(
        "Confirm before connecting to Internet? %1(matters only if you have a modem, ISDN or VPN connection)%2",
        "<br><span class=note>",
        "</span>"
    )
);
define("HANGUP_IF_DIALED_DESC",
    tra(
        "Disconnect when done? %1(matters only if you have a modem, ISDN or VPN connection)%2",
        "<br><span class=note>",
        "</span>"
    )
);
define("WORK_BUF_MIN_DAYS_DESC",
    tra(
        "Computer is connected to the Internet about every %1(Leave blank or 0 if always connected.%2 BOINC will try to maintain at least this much work.)%3",
        "<br><span class=note>",
        "<br>",
        "</span>"
    )
);
define("WORK_BUF_ADDITIONAL_DAYS_DESC",
    tra("Maintain enough work for an additional")
);
define("MAX_NCPUS_PCT_DESC",
    tra(
        "On multiprocessors, use at most %1 Enforced by version 6.1+%2",
        "<br><span class=note>",
        "</span>"
    )
);
define("MAX_NCPUS_PCT_DESC2", tra("% of the processors"));
define("MAX_CPUS_DESC", tra("On multiprocessors, use at most"));
define("MAX_CPUS_DESC2", tra("processors"));
define("USE_AT_MOST", tra("Use at most"));
define("USE_AT_MOST2",
    tra(
        "Use at most %1 (Can be used to reduce CPU heat)%2",
        "<br><span class=note>",
        "</span>"
    )
);
define("USE_AT_MOST3", tra("Use at most"));
define("CPU_USAGE_LIMIT_DESC2", tra("percent of CPU time"));
define("DISK_INTERVAL_DESC", tra("Tasks checkpoint to disk at most every"));
define("DISK_LIMIT_DESC", tra("Disk and memory usage"));
define("DISK_MIN_FREE_GB_DESC", tra("Leave at least"));
define("DISK_SCHED_MIN_FREE_DESC",
    "<br><span class=note>".tra("(Values smaller than %s are ignored)")."</span>"
);
define("DISK_MAX_USED_PCT_DESC2", tra("% of total disk space"));
define("VM_MAX_USED_PCT_DESC2", tra("% of page file (swap space)"));
define("RAM_MAX_USED_BUSY_PCT_DESC2", tra("% of memory when computer is in use"));
define("RAM_MAX_USED_IDLE_PCT_DESC2", tra("% of memory when computer is not in use"));
define("NETWORK_LIMIT_DESC", tra("Network usage"));
define("MAX_BYTES_SEC_DOWN_DESC", tra("Maximum download rate:"));
define("MAX_BYTES_SEC_UP_DESC", tra("Maximum upload rate:"));
define("NET_START_END_DESC", tra("Use network only between the hours of"));
define("DONT_VERIFY_IMAGES_DESC",
    tra(
        "Skip image file verification? %1 Check this ONLY if your Internet provider modifies image files (UMTS does this, for example). %2 Skipping verification reduces the security of BOINC.%3",
        "<br><span class=note>",
        "<br>",
        "</span>"
    )
);
define('ATI_SELECT_DESC',
    tra(
        "Use ATI GPU if present %1(enforced by 6.10+ clients)%2",
        "<br><span class=note>",
        "</span>"
    )
);
define('CUDA_SELECT_DESC',
    tra(
        "Use NVIDIA GPU if present %1(enforced by 6.10+ clients)%2",
        "<br><span class=note>",
        "</span>"
    )
);
define('CPU_SELECT_DESC',
    tra(
        "Use CPU %1(enforced by 6.10+ clients)%2",
        "<br><span class=note>",
        "</span>"
    )
);


define("BYTE_CONVERSION", 1000.0);
define("BYTE_UNITS", tra("Kbytes/sec"));
define("BYTE_ABBR", tra("KB/s"));
define("ALLOW_BETA_WORK",
    tra(
        "Run test applications? %1This helps us develop applications, but may cause jobs to fail on your computer%2",
        "<br><span class=note>",
        "</span>"
    )
);

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
global $disk_prefs;


// get default settings for disk space usage so the default user
// preferences match the settings used by the scheduler.
// Defaults are set if the tags are missing, they depend on
// which scheduler is running:
// - 'old' has the default hardcoded 
// - 'new' uses config settings
// if running the old scheduler, set <scheduler_disk_space_check_hardcoded>
// in config.xml so the right default is set for minimum free disk space
// 
$disk_prefs = null;
function get_disk_space_config() {
    global  $disk_prefs;
    global  $config;
    if ($disk_prefs == null) {
        $config = get_config();
        $disk_prefs->disk_max_used_gb = parse_config($config, "<default_disk_max_used_gb>");
        $disk_prefs->disk_max_used_pct = parse_config($config, "<default_disk_max_used_pct>");
        $disk_prefs->disk_min_free_gb = parse_config($config, "<default_disk_min_free_gb>");
        // set some defaults if not found
        if (!$disk_prefs->disk_max_used_gb) $disk_prefs->disk_max_used_gb = 100;  // 100 gb
        if (!$disk_prefs->disk_max_used_pct) $disk_prefs->disk_max_used_pct = 50; // 50 percent
        if (!$disk_prefs->disk_min_free_gb) $disk_prefs->disk_min_free_gb=.001;   // 1 megabyte
        // set mininimum free space scheduler allows 
        // - depends on which scheduler is running
        $disk_prefs->new_sched_flag = 1;
        $disk_prefs->sched_disk_min_free_gb = $disk_prefs->disk_min_free_gb;
        if (parse_config($config, "scheduler_disk_space_check_hardcoded>")) {
            $disk_prefs->new_sched_flag = 0;
            $disk_prefs->sched_disk_min_free_gb = 0;
        }    
    }
       
    return $disk_prefs;
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

// functions to convert between max_bytes_sec_* as stored in the
// database and max_bytes_sec_* as displayed/entered on the web
// pages. Currently max_bytes_sec_* is stored in bytes and
// displayed/entered in Kbytes.
//
function max_bytes_display_mode($db_bytes) {
    $disp_bytes = 0;
    if ($db_bytes) {
        $disp_bytes = $db_bytes / BYTE_CONVERSION;
    }
    return $disp_bytes;
}

function max_bytes_db_mode($disp_bytes) {
    $db_bytes = 0;
    if ($disp_bytes) {
        $db_bytes = $disp_bytes * BYTE_CONVERSION;
    }
    return $db_bytes;
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

    switch($name) {
    case "venue":
        $top_parse_result->$venue_name = $parse_result;
        $parse_result = $top_parse_result;
        break;
    case "project_specific":
        $parse_result->project_specific = $text;
        $in_project_specific = false;
        break;
    case "resource_share":
        $parse_result->resource_share = $text;
        break;
    case "allow_beta_work":
        $parse_result->allow_beta_work = $text;
        break;
    case "no_cpu":
        $parse_result->no_cpu = $text;
        break;
    case "no_cuda":
        $parse_result->no_cuda = $text;
        break;
    case "no_ati":
        $parse_result->no_ati = $text;
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

function get_bool() {
    global $text;
    return (trim($text) != '0');
}

function element_end_global($parser, $name) {
    global $text;
    global $parse_result;
    global $top_parse_result;
    global $venue_name;

    switch($name) {
    case "venue":
        $top_parse_result->$venue_name = $parse_result;
        $parse_result = $top_parse_result;
        break;
    case "run_on_batteries":
        // deprecated, but parse anyway
        $parse_result->suspend_while_on_batteries = !get_bool();
        break;
    case "suspend_while_on_batteries":
        $parse_result->suspend_while_on_batteries = get_bool();
        break;
    case "run_if_user_active":
        $parse_result->suspend_if_user_active = !get_bool();
        break;
    case "run_gpu_if_user_active":
        $parse_result->suspend_gpu_if_user_active = !get_bool();
        break;
    case "idle_time_to_run":
        $parse_result->idle_time_to_run = $text;
        break;
    case "suspend_if_no_recent_input":
        $parse_result->suspend_if_no_recent_input = $text;
        break;
    case "suspend_cpu_usage":
        $parse_result->suspend_cpu_usage = $text;
        break;
    case "start_hour":
        $parse_result->start_hour = $text;
        break;
    case "end_hour":
        $parse_result->end_hour = $text;
        break;
    case "leave_apps_in_memory":
        $parse_result->leave_apps_in_memory = get_bool();
        break;
    case "cpu_scheduling_period_minutes":
        $parse_result->cpu_scheduling_period_minutes = $text;
        break;
    case "confirm_before_connecting":
        $parse_result->confirm_before_connecting = get_bool();
        break;
    case "hangup_if_dialed":
        $parse_result->hangup_if_dialed = get_bool();
        break;
    case "work_buf_min_days":
        $parse_result->work_buf_min_days = $text;
        break;
    case "work_buf_additional_days":
        $parse_result->work_buf_additional_days = $text;
        break;
    case "max_cpus":
        $parse_result->max_cpus = $text;
        break;
    case "max_ncpus_pct":
        $parse_result->max_ncpus_pct = $text;
        break;
    case "cpu_usage_limit":
        if (!$text) $text=100;
        $parse_result->cpu_usage_limit = $text;
        break;
    case "disk_interval":
        $parse_result->disk_interval = $text;
        break;
    case "disk_max_used_gb":
        $parse_result->disk_max_used_gb = $text;
        break;
    case "disk_max_used_pct":
        $parse_result->disk_max_used_pct = $text;
        break;
    case "disk_min_free_gb":
        $parse_result->disk_min_free_gb = $text;
        break;
    case "vm_max_used_pct":
        $parse_result->vm_max_used_pct = $text;
        break;
    case "ram_max_used_busy_pct":
        $parse_result->ram_max_used_busy_pct = $text;
        break;
    case "ram_max_used_idle_pct":
        $parse_result->ram_max_used_idle_pct = $text;
        break;
    case "max_bytes_sec_down":
        $parse_result->max_bytes_sec_down = $text;
        break;
    case "max_bytes_sec_up":
        $parse_result->max_bytes_sec_up = $text;
        break;
    case "net_start_hour":
        $parse_result->net_start_hour = $text;
        break;
    case "net_end_hour":
        $parse_result->net_end_hour = $text;
        break;
    case "dont_verify_images":
        $parse_result->dont_verify_images = get_bool();
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
    $p = null;
    $p->suspend_while_on_batteries = true;
    $p->suspend_if_user_active = false;
    $p->suspend_gpu_if_user_active = true;
    $p->idle_time_to_run = 3;
    $p->suspend_if_no_recent_input = 0;
    $p->suspend_cpu_usage = 25;
    $p->start_hour = 0;
    $p->end_hour = 0;
    $p->leave_apps_in_memory = false;
    $p->cpu_scheduling_period_minutes = 60;
    $p->confirm_before_connecting = false;
    $p->hangup_if_dialed = false;
    $p->work_buf_min_days = 0;
    $p->work_buf_additional_days = 0.25;
    $p->max_cpus = 16;
    $p->max_ncpus_pct = 100;
    $p->cpu_usage_limit = 100;
    $p->disk_interval = 60;
    $dp = get_disk_space_config();
    $p->disk_max_used_gb = $dp->disk_max_used_gb;
    $p->disk_max_used_pct = $dp->disk_max_used_pct;
    $p->disk_min_free_gb = $dp->disk_min_free_gb;
    $p->vm_max_used_pct = 75;
    $p->ram_max_used_busy_pct = 50;
    $p->ram_max_used_idle_pct = 90;
    $p->max_bytes_sec_down = 0;
    $p->max_bytes_sec_up = 0;
    $p->net_start_hour = 0;
    $p->net_end_hour = 0;
    $p->dont_verify_images = false;
    return $p;
}

function default_prefs_project() {
    $p = null;
    $p->resource_share = 100;
    $p->no_cpu = false;
    $p->no_cuda = false;
    $p->no_ati = false;
    $p->allow_beta_work = false;
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
    //echo "AFTER INIT: <pre>";
    //var_dump($parse_result);
    //echo "</pre>";
    $xml_parser = xml_parser_create();
    xml_parser_set_option($xml_parser, XML_OPTION_CASE_FOLDING, 0);
    xml_set_element_handler($xml_parser, "element_start_global", "element_end_global");
    xml_set_character_data_handler($xml_parser, "char_handler");
    xml_parse($xml_parser, $prefs_xml, 1);
    //echo "AFTER PARSE: <pre>";
    //var_dump($parse_result);
    //echo "</pre>";
    return $parse_result;
}

function hour_str($x) {
    return "$x:00";
}

function hour_select($x, $name) {
    $s = "";
    $s = $s. "<select name=$name>\n";
    for ($i=0; $i<24; $i++) {
        $y = hour_str($i);
        $sel = ($x == $i)?"selected":"";
        $s = $s."<option value=$i $sel> $y";
    }
    $s = $s."</select>\n";
    return $s;
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
// hour_range - Produce range string for given venue
//
function hour_range($venue, $type) {
    if ($type == "run") {
        $s_hour = $venue->start_hour;
        $e_hour = $venue->end_hour;
    }
    if ($type == "net") {
        $s_hour = $venue->net_start_hour;
        $e_hour = $venue->net_end_hour;
    }
    if ($s_hour == $e_hour) {
        $x = "--";
    } else {
        $s = hour_str($s_hour);
        $e = hour_str($e_hour);
        $x = "$s<br>to $e";
    }
    return $x;
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
// row_hours - Display start & end hours for all venues in 1 row
//
function row_hours($pre, $type, $global_prefs) {
    $gen = hour_range($global_prefs, $type);
    $hom  = isset($global_prefs->home)   ? hour_range($global_prefs->home, $type)   : "--";
    $schl = isset($global_prefs->school) ? hour_range($global_prefs->school, $type) : "--";
    $wrk  = isset($global_prefs->work)   ? hour_range($global_prefs->work, $type)   : "--";

    echo "<tr><td class=fieldname>$pre</td>";
    row_field($gen, "");
    row_field($hom, "");
    row_field($schl, "");
    row_field($wrk, "");
    echo "<td class=f_val><br></td></tr>\n";
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


//
// prefs_show_columns_global - Display all venues as columns next to descriptions
//
function prefs_show_columns_global($prefs) {
    row_top(CPU_LIMIT_DESC);
    row_defs(SUSPEND_WHILE_ON_BATTERIES_DESC, "suspend_while_on_batteries", "", "yesno", $prefs);
    row_defs(RUN_IF_USER_ACTIVE_DESC, "suspend_if_user_active", "", "yesno", $prefs);
    row_defs(RUN_GPU_IF_USER_ACTIVE_DESC, "suspend_gpu_if_user_active", "", "yesno", $prefs);
    row_defs(IDLE_TIME_TO_RUN_DESC, "idle_time_to_run", tra("minutes"), "", $prefs);

    row_defs(SUSPEND_IF_NO_RECENT_INPUT_DESC, "suspend_if_no_recent_input", tra("minutes"), "minutes", $prefs);
    row_defs(SUSPEND_CPU_USAGE_DESC, "suspend_cpu_usage", "%", "", $prefs);

    row_hours(START_END_DESC, "run", $prefs);

    row_defs(LEAVE_APPS_IN_MEMORY_DESC, "leave_apps_in_memory", "", "yesno", $prefs);
    row_defs(CPU_SCHEDULING_DESC, "cpu_scheduling_period_minutes", tra("minutes"), "", $prefs);
    row_defs(MAX_CPUS_DESC, "max_cpus", MAX_CPUS_DESC2, "", $prefs);
    row_defs(MAX_NCPUS_PCT_DESC, "max_ncpus_pct", MAX_NCPUS_PCT_DESC2, "", $prefs);
    row_defs(USE_AT_MOST2, "cpu_usage_limit", CPU_USAGE_LIMIT_DESC2, "", $prefs);

    row_top(DISK_LIMIT_DESC);
    row_defs(USE_AT_MOST, "disk_max_used_gb", tra("GB disk space"), "", $prefs);
    row_defs(DISK_MIN_FREE_GB_DESC, "disk_min_free_gb", tra("GB disk space free"), "", $prefs);
    row_defs(USE_AT_MOST, "disk_max_used_pct", DISK_MAX_USED_PCT_DESC2, "", $prefs);
    row_defs(DISK_INTERVAL_DESC, "disk_interval", tra("seconds"), "", $prefs);
    row_defs(USE_AT_MOST, "vm_max_used_pct", VM_MAX_USED_PCT_DESC2, "", $prefs);
    row_defs(USE_AT_MOST3, "ram_max_used_busy_pct", RAM_MAX_USED_BUSY_PCT_DESC2, "", $prefs);
    row_defs(USE_AT_MOST3, "ram_max_used_idle_pct", RAM_MAX_USED_IDLE_PCT_DESC2, "", $prefs);

    row_top(NETWORK_LIMIT_DESC);
    row_defs(WORK_BUF_MIN_DAYS_DESC, "work_buf_min_days", tra("days"), "", $prefs);
    row_defs(WORK_BUF_ADDITIONAL_DAYS_DESC, "work_buf_additional_days", tra("days"), "", $prefs);
    row_defs(CONFIRM_BEFORE_CONNECTING_DESC, "confirm_before_connecting", "", "yesno", $prefs);
    row_defs(HANGUP_IF_DIALED_DESC, "hangup_if_dialed", "", "yesno", $prefs);
    row_defs(MAX_BYTES_SEC_DOWN_DESC, "max_bytes_sec_down", "", "limit", $prefs);
    row_defs(MAX_BYTES_SEC_UP_DESC, "max_bytes_sec_up", "", "limit", $prefs);
    row_hours(NET_START_END_DESC, "net", $prefs);
    row_defs(DONT_VERIFY_IMAGES_DESC, "dont_verify_images", "", "yesno", $prefs);

    row_links("global", $prefs);
}

function show_double($x) {
    if ($x) return $x;
    return '---';
}

function prefs_show_global($prefs) {
    row1(CPU_LIMIT_DESC);
    row2(SUSPEND_WHILE_ON_BATTERIES_DESC, $prefs->suspend_while_on_batteries?tra("yes"):tra("no"));
    row2(RUN_IF_USER_ACTIVE_DESC, $prefs->suspend_if_user_active?tra("yes"):tra("no"));
    row2(RUN_GPU_IF_USER_ACTIVE_DESC, $prefs->suspend_gpu_if_user_active?tra("yes"):tra("no"));
    row2(IDLE_TIME_TO_RUN_DESC, tra("%1 minutes", $prefs->idle_time_to_run));

    $x = show_double($prefs->suspend_if_no_recent_input);
    row2(SUSPEND_IF_NO_RECENT_INPUT_DESC, tra("%1 minutes", $x));
    $x = show_double($prefs->suspend_cpu_usage);
    row2(SUSPEND_CPU_USAGE_DESC, "$x %");

    if ($prefs->start_hour == $prefs->end_hour) {
        $x = tra("(no restriction)");
    } else {
        $s = hour_str($prefs->start_hour);
        $e = hour_str($prefs->end_hour);
        $x = "$s ".tra("and")." $e";
    }
    row2(START_END_DESC, $x);
    row2(LEAVE_APPS_IN_MEMORY_DESC, $prefs->leave_apps_in_memory?tra("yes"):tra("no"));
    row2(CPU_SCHEDULING_DESC, tra("%1 minutes", $prefs->cpu_scheduling_period_minutes));
    $x = $prefs->max_cpus?$prefs->max_cpus:'---';
    row2(MAX_CPUS_DESC, "$x ".MAX_CPUS_DESC2);
    $x = $prefs->max_ncpus_pct?$prefs->max_ncpus_pct:'---';
    row2(MAX_NCPUS_PCT_DESC, "$x ".MAX_NCPUS_PCT_DESC2);
    row2(USE_AT_MOST2, "$prefs->cpu_usage_limit ".CPU_USAGE_LIMIT_DESC2);
    row1(DISK_LIMIT_DESC);
    row2(USE_AT_MOST, tra("%1 GB disk space", $prefs->disk_max_used_gb));

    $dp = get_disk_space_config();

    $msg = null;
    if ($dp->new_sched_flag) {
        $msg = sprintf(DISK_SCHED_MIN_FREE_DESC, $dp->sched_disk_min_free_gb);
    }
    row2(DISK_MIN_FREE_GB_DESC.$msg, tra("%1 GB disk space free", $prefs->disk_min_free_gb));
    row2(USE_AT_MOST, "$prefs->disk_max_used_pct".DISK_MAX_USED_PCT_DESC2);
    row2(DISK_INTERVAL_DESC, "$prefs->disk_interval seconds");
    row2(USE_AT_MOST, "$prefs->vm_max_used_pct".VM_MAX_USED_PCT_DESC2);
    row2(USE_AT_MOST3, "$prefs->ram_max_used_busy_pct".RAM_MAX_USED_BUSY_PCT_DESC2);
    row2(USE_AT_MOST3, "$prefs->ram_max_used_idle_pct".RAM_MAX_USED_IDLE_PCT_DESC2);
    row1(NETWORK_LIMIT_DESC);
    row2(WORK_BUF_MIN_DAYS_DESC, tra("%1 days", $prefs->work_buf_min_days));
    row2(WORK_BUF_ADDITIONAL_DAYS_DESC, tra("%1 days", $prefs->work_buf_additional_days));
    row2(CONFIRM_BEFORE_CONNECTING_DESC, $prefs->confirm_before_connecting?tra("yes"):tra("no"));
    row2(HANGUP_IF_DIALED_DESC, $prefs->hangup_if_dialed?tra("yes"):tra("no"));
    $x = max_bytes_display_mode($prefs->max_bytes_sec_down);
    $y = "$x " . BYTE_ABBR;
    row2(MAX_BYTES_SEC_DOWN_DESC, $x?"$y":tra("no limit"));
    $x = max_bytes_display_mode($prefs->max_bytes_sec_up);
    $y = "$x " . BYTE_ABBR;
    row2(MAX_BYTES_SEC_UP_DESC, $x?"$y":tra("no limit"));
    if ($prefs->net_start_hour == $prefs->net_end_hour) {
        $x = tra("(no restriction)");
    } else {
        $s = hour_str($prefs->net_start_hour);
        $e = hour_str($prefs->net_end_hour);
        $x = "$s ".tra("and")." $e";
    }
    row2(NET_START_END_DESC, $x);
    row2(DONT_VERIFY_IMAGES_DESC, $prefs->dont_verify_images?tra("yes"):tra("no"));
}

function prefs_show_resource($prefs, $columns=false) {
    global $app_types;
    $x = tra(
        "Resource share %1If you participate in multiple BOINC projects, this is the proportion of your resources used by %2%3",
         "<br><span class=note>",
         PROJECT,
         "</span>"
    );
    if ($columns) {
        row_defs($x, "resource_share", "", "" ,$prefs);
        if ($app_types->count > 1) {
            if ($app_types->cpu) {
                row_defs(CPU_SELECT_DESC, "no_cpu", "", "noyes", $prefs);
            }
            if ($app_types->cuda) {
                row_defs(CUDA_SELECT_DESC, "no_cuda", "", "noyes", $prefs);
            }
            if ($app_types->ati) {
                row_defs(ATI_SELECT_DESC, "no_ati", "", "noyes", $prefs);
            }
        }
    } else {
        row2($x, $prefs->resource_share);
        if ($app_types->count > 1) {
            if ($app_types->cpu) {
                row2(CPU_SELECT_DESC, $prefs->no_cpu?tra("no"):tra("yes"));
            }
            if ($app_types->cuda) {
                row2(CUDA_SELECT_DESC, $prefs->no_cuda?tra("no"):tra("yes"));
            }
            if ($app_types->ati) {
                row2(ATI_SELECT_DESC, $prefs->no_ati?tra("no"):tra("yes"));
            }
        }
    }
}

function prefs_show_beta($prefs, $columns=false) {
    if ($columns) {
        row_defs(ALLOW_BETA_WORK, "allow_beta_work", "", "yesno" ,$prefs);
    } else {
        row2(ALLOW_BETA_WORK, $prefs->allow_beta_work?tra("yes"):tra("no"));
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

function prefs_show_project($prefs, $columns=false) {
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
    global $project_has_beta;
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
            prefs_show_resource($x);
            if ($project_has_beta) prefs_show_beta($x);
            prefs_show_project($x);
        }
        row2("<br>", "<a href=prefs_edit.php?venue=$venue&amp;subset=$subset$tokens>".tra("Edit preferences")."</a> | <a href=prefs_remove.php?venue=$venue&amp;subset=$subset$tokens>".tra("Remove")."</a>");
        end_table();
        echo "</td></tr>\n";
    } else {
        row1("<a href=add_venue.php?venue=$venue&amp;subset=$subset$tokens>".tra("Add separate preferences for %1", $venue)."</a>", 2, "heading");
    }
}

function print_prefs_display_project($user, $columns=false) {
    global $project_has_beta;
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
        prefs_show_resource($project_prefs, true);
        if ($project_has_beta) prefs_show_beta($project_prefs, true);
        prefs_show_project($project_prefs, true);
        row_links("project", $project_prefs);
        end_table();
        echo "</td></tr>\n";
    } else {
        if (isset($project_prefs->home) || isset($project_prefs->work) || isset($project_prefs->school)) {
            row1(tra("Primary (default) preferences").$switch_link, 2, "heading");
        }
        echo "<tr><td colspan=2>";
        start_table();
        prefs_show_resource($project_prefs, false);
        if ($project_has_beta) prefs_show_beta($project_prefs, false);
        prefs_show_privacy($user);
        venue_show($user);
        prefs_show_project($project_prefs, false);
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
    global $project_has_beta;

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
        prefs_form_resource($prefs, $error);
        if ($project_has_beta) prefs_form_beta($prefs, $error);
        if (!$venue) {
            prefs_form_privacy($user);
            venue_form($user);
        }
        prefs_form_project($prefs->project_specific, $project_error);
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

    row1(CPU_LIMIT_DESC);
    $y = prefs_form_radio_buttons("suspend_while_on_batteries", $prefs->suspend_while_on_batteries);
    row2(SUSPEND_WHILE_ON_BATTERIES_DESC, $y);

    $y = prefs_form_radio_buttons("suspend_if_user_active", $prefs->suspend_if_user_active);
    row2(RUN_IF_USER_ACTIVE_DESC, $y);

    $y = prefs_form_radio_buttons("suspend_gpu_if_user_active", $prefs->suspend_gpu_if_user_active);
    row2(RUN_GPU_IF_USER_ACTIVE_DESC, $y);

    $y = "<input size=5 name=idle_time_to_run value='$prefs->idle_time_to_run'> ".tra("minutes");
    $show_error = false;
    if (isset($error->idle_time_to_run)) $show_error= true;
    row2(IDLE_TIME_TO_RUN_DESC, $y, $show_error);

    $y = "<input size=5 name=suspend_if_no_recent_input value='$prefs->suspend_if_no_recent_input'> ".tra("minutes");
    $show_error = false;
    if (isset($error->suspend_if_no_recent_input)) $show_error= true;
    row2(SUSPEND_IF_NO_RECENT_INPUT_DESC, $y, $show_error);

    $y = "<input size=5 name=suspend_cpu_usage value='$prefs->suspend_cpu_usage'> % ";
    $show_error = false;
    if (isset($error->suspend_cpu_usage)) $show_error= true;
    row2(SUSPEND_CPU_USAGE_DESC, $y, $show_error);

    $x = START_END_DESC.START_END_DESC2;
    $y = hour_select($prefs->start_hour, "start_hour").tra("and").hour_select($prefs->end_hour, "end_hour");
    row2($x, $y);

    $x = LEAVE_APPS_IN_MEMORY_DESC;
    $y = prefs_form_radio_buttons("leave_apps_in_memory", $prefs->leave_apps_in_memory);
    row2($x, $y);

    $y = "<input size=5 name=cpu_scheduling_period_minutes value='$prefs->cpu_scheduling_period_minutes'> ".tra("minutes");
    $show_error = false;
    if (isset($error->cpu_scheduling_period_minutes)) $show_error= true;
    row2(CPU_SCHEDULING_DESC, $y, $show_error);

    $show_error = false;
    if (isset($error->max_cpus)) $show_error= true;
    row2(MAX_CPUS_DESC,
        "<input size=4 name=max_cpus value=$prefs->max_cpus> ".MAX_CPUS_DESC2,
        $show_error
    );

    $show_error = false;
    if (isset($error->max_ncpus_pct)) $show_error= true;
    row2(MAX_NCPUS_PCT_DESC,
        "<input size=4 name=max_ncpus_pct value=$prefs->max_ncpus_pct> ".MAX_NCPUS_PCT_DESC2,
        $show_error
    );

    $show_error = false;
    if (isset($error->cpu_usage_limit)) $show_error= true;
    row2(USE_AT_MOST2,
        "<input size=4 name=cpu_usage_limit value=$prefs->cpu_usage_limit> ".CPU_USAGE_LIMIT_DESC2,
        $show_error
    );

    row1(DISK_LIMIT_DESC);

    $show_error = false;
    if (isset($error->disk_max_used_gb)) $show_error= true;
    row2(USE_AT_MOST,
        "<input size=7 name=disk_max_used_gb value='$prefs->disk_max_used_gb'> ".tra("Gbytes"),
        $show_error
    );

    $dp = get_disk_space_config();

    $msg=null;
    if ($dp->new_sched_flag) {
        $msg = sprintf(DISK_SCHED_MIN_FREE_DESC, $dp->sched_disk_min_free_gb);
    } 

    $show_error = false;
    if (isset($error->disk_min_free_gb)) $show_error= true;
    row2(DISK_MIN_FREE_GB_DESC.$msg,
        "<input size=7 name=disk_min_free_gb value='$prefs->disk_min_free_gb'> ".tra("Gbytes free"),
        $show_error
    );

    $show_error = false;
    if (isset($error->disk_max_used_pct)) $show_error= true;
    row2(USE_AT_MOST,
        "<input size=5 name=disk_max_used_pct value='$prefs->disk_max_used_pct'> ".DISK_MAX_USED_PCT_DESC2,
        $show_error
    );

    $show_error = false;
    if (isset($error->disk_interval)) $show_error= true;
    row2(DISK_INTERVAL_DESC,
        "<input size=6 name=disk_interval value=$prefs->disk_interval> ".tra("seconds"),
        $show_error
    );

    $show_error = false;
    if (isset($error->vm_max_used_pct)) $show_error= true;
    row2(USE_AT_MOST,
        "<input size=5 name=vm_max_used_pct value='$prefs->vm_max_used_pct'> ".VM_MAX_USED_PCT_DESC2,
        $show_error
    );

    $show_error = false;
    if (isset($error->ram_max_used_busy_pct)) $show_error= true;
    row2(USE_AT_MOST3,
        "<input size=5 name=ram_max_used_busy_pct value='$prefs->ram_max_used_busy_pct'> ".RAM_MAX_USED_BUSY_PCT_DESC2,
        $show_error
    );

    $show_error = false;
    if (isset($error->ram_max_used_idle_pct)) $show_error= true;
    row2(USE_AT_MOST3,
        "<input size=5 name=ram_max_used_idle_pct value='$prefs->ram_max_used_idle_pct'> ".RAM_MAX_USED_IDLE_PCT_DESC2,
        $show_error
    );

    row1(NETWORK_LIMIT_DESC);
    $x = WORK_BUF_MIN_DAYS_DESC;
    $y = "<input size=5 name=work_buf_min_days value='$prefs->work_buf_min_days'> ".tra("days");
    $show_error = false;
    if (isset($error->work_buf_min_days)) $show_error= true;
    row2($x, $y, $show_error);

    $x = WORK_BUF_ADDITIONAL_DAYS_DESC;
    $y = "<input size=5 name=work_buf_additional_days value='$prefs->work_buf_additional_days'> ".tra("days");
    $show_error = false;
    if (isset($error->work_buf_additional_days)) $show_error= true;
    row2($x, $y, $show_error);

    $x = CONFIRM_BEFORE_CONNECTING_DESC;
    $y = prefs_form_radio_buttons("confirm_before_connecting", $prefs->confirm_before_connecting);
    row2($x, $y);

    $x = HANGUP_IF_DIALED_DESC;
    $y = prefs_form_radio_buttons("hangup_if_dialed", $prefs->hangup_if_dialed);
    row2($x, $y);

    $d = max_bytes_display_mode($prefs->max_bytes_sec_down);
    $dt = $d?"$d":"";
    $u = max_bytes_display_mode($prefs->max_bytes_sec_up);
    $ut = $u?"$u":"";

    $show_error = false;
    if (isset($error->max_bytes_sec_down)) $show_error= true;
    row2(MAX_BYTES_SEC_DOWN_DESC,
        "<input size=7 name=max_bytes_sec_down value='$dt'> " . BYTE_UNITS,
        $show_error
    );

    $show_error = false;
    if (isset($error->max_bytes_sec_up)) $show_error= true;
    row2(MAX_BYTES_SEC_UP_DESC,
        "<input size=7 name=max_bytes_sec_up value='$ut'> " . BYTE_UNITS,
        $show_error
    );

    $x = NET_START_END_DESC.START_END_DESC2;
    $y = hour_select($prefs->net_start_hour, "net_start_hour").tra("and").hour_select($prefs->net_end_hour, "net_end_hour");
    row2($x, $y);
    $x = DONT_VERIFY_IMAGES_DESC;
    $y = prefs_form_radio_buttons("dont_verify_images", $prefs->dont_verify_images);
    row2($x, $y);

}

function prefs_form_privacy($user) {
    $y = prefs_form_radio_buttons("send_email", $user->send_email);
    row2(tra("Is it OK for %1 and your team (if any) to email you?", PROJECT), $y);

    $y = prefs_form_radio_buttons("show_hosts", $user->show_hosts);
    row2(tra("Should %1 show your computers on its web site?", PROJECT), $y);
}

function prefs_form_resource($prefs, $error=false) {
    global $app_types;
    $show_error = false;
    if (isset($error->resource_share)) $show_error= true;
    row2(
        "<strong>".tra("Resource share").":</strong>
        <br><span class=note>".
        tra("Determines the proportion of your computer's resources
            allocated to this project. Example: if you participate in two
            BOINC projects with resource shares of 100 and 200, the first
            will get 1/3 of your resources and the second will get 2/3.").
        "</span>",
        "<input name=resource_share value='$prefs->resource_share'>",
        $show_error
    );
    if ($app_types->count > 1) {
        if ($app_types->cpu) {
            $checked = $prefs->no_cpu?"":"checked";
            $y = "<input type=checkbox name=use_cpu $checked>";
            row2(CPU_SELECT_DESC, $y);
        }
        if ($app_types->cuda) {
            $checked = $prefs->no_cuda?"":"checked";
            $y = "<input type=checkbox name=use_cuda $checked>";
            row2(CUDA_SELECT_DESC, $y);
        }
        if ($app_types->ati) {
            $checked = $prefs->no_ati?"":"checked";
            $y = "<input type=checkbox name=use_ati $checked>";
            row2(ATI_SELECT_DESC, $y);
        }
    }
}

function prefs_form_beta($prefs, $error=false) {
    $y = prefs_form_radio_buttons("allow_beta_work", $prefs->allow_beta_work);
    row2(ALLOW_BETA_WORK, $y);
}

function prefs_form_project($prefs_xml, $error=false) {
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
    $error = false;
    $suspend_while_on_batteries = $_GET["suspend_while_on_batteries"];
    $suspend_if_user_active = $_GET["suspend_if_user_active"];
    $suspend_gpu_if_user_active = $_GET["suspend_gpu_if_user_active"];
    $idle_time_to_run = $_GET["idle_time_to_run"];
    $suspend_if_no_recent_input = $_GET["suspend_if_no_recent_input"];
    $suspend_cpu_usage = $_GET["suspend_cpu_usage"];
    $start_hour = $_GET["start_hour"];
    $end_hour = $_GET["end_hour"];
    $leave_apps_in_memory = $_GET["leave_apps_in_memory"];
    $cpu_scheduling_period_minutes = $_GET["cpu_scheduling_period_minutes"];
    $max_cpus = $_GET["max_cpus"];
    $max_ncpus_pct = $_GET["max_ncpus_pct"];
    $cpu_usage_limit = $_GET["cpu_usage_limit"];
    $disk_max_used_gb = $_GET["disk_max_used_gb"];
    $disk_min_free_gb = $_GET["disk_min_free_gb"];
    $disk_max_used_pct = $_GET["disk_max_used_pct"];
    $disk_interval = $_GET["disk_interval"];
    $vm_max_used_pct = $_GET["vm_max_used_pct"];
    $ram_max_used_busy_pct = $_GET["ram_max_used_busy_pct"];
    $ram_max_used_idle_pct = $_GET["ram_max_used_idle_pct"];
    $work_buf_min_days = $_GET["work_buf_min_days"];
    $work_buf_additional_days = $_GET["work_buf_additional_days"];
    $confirm_before_connecting = $_GET["confirm_before_connecting"];
    $hangup_if_dialed = $_GET["hangup_if_dialed"];
    $max_bytes_sec_down = $_GET["max_bytes_sec_down"];
    $max_bytes_sec_up = $_GET["max_bytes_sec_up"];
    $net_start_hour = $_GET["net_start_hour"];
    $net_end_hour = $_GET["net_end_hour"];
    $dont_verify_images = $_GET["dont_verify_images"];
    // Verification of all user input values
    if (!verify_numeric($idle_time_to_run, 1)) $error->idle_time_to_run = true;
    if (!verify_numeric($suspend_if_no_recent_input, 0)) $error->suspend_if_no_recent_input = true;
    if (!verify_numeric($suspend_cpu_usage, 0, 100)) $error->suspend_cpu_usage = true;
    if (!verify_numeric($start_hour, 0)) $error->start_hour = true;
    if (!verify_numeric($end_hour, 0)) $error->end_hour = true;
    if (!verify_numeric($cpu_scheduling_period_minutes, 1)) $error->cpu_scheduling_period_minutes = true;;
    if (!verify_numeric($max_cpus, 0)) $error->max_cpus = true;
    if (!verify_numeric($max_ncpus_pct, 0, 100)) $error->max_ncpus_pct = true;
    if (!verify_numeric($cpu_usage_limit, 0, 100)) $error->cpu_usage_limit = true;
    if (!verify_numeric($disk_max_used_gb, 0)) $error->disk_max_used_gb = true;
    if (!verify_numeric($disk_min_free_gb, 0.001)) $error->disk_min_free_gb = true;
    if (!verify_numeric($disk_max_used_pct, 0, 100)) $error->disk_max_used_pct = true;
    if (!verify_numeric($disk_interval, 0)) $error->disk_interval = true;
    if (!verify_numeric($vm_max_used_pct, 0, 100)) $error->vm_max_used_pct = true;
    if (!verify_numeric($ram_max_used_busy_pct, 0, 100)) $error->ram_max_used_busy_pct = true;
    if (!verify_numeric($ram_max_used_idle_pct, 0, 100)) $error->ram_max_used_idle_pct = true;
    if (!verify_numeric($work_buf_min_days, 0, 10)) $error->work_buf_min_days = true;
    if (!verify_numeric($work_buf_additional_days, 0, 10)) $error->work_buf_additional_days = true;
    if (!verify_numeric($max_bytes_sec_down, '')) $error->max_bytes_sec_down = true;
    if (!verify_numeric($max_bytes_sec_up, '')) $error->max_bytes_sec_up = true;
    if (!verify_numeric($net_start_hour, 0)) $error->net_start_hour = true;
    if (!verify_numeric($net_end_hour, 0)) $error->net_end_hour = true;
    // Modify all values within the supplied Object
    $prefs->suspend_while_on_batteries = ($suspend_while_on_batteries == "yes");
    $prefs->suspend_if_user_active = ($suspend_if_user_active == "yes");
    $prefs->suspend_gpu_if_user_active = ($suspend_gpu_if_user_active == "yes");
    $prefs->idle_time_to_run = $idle_time_to_run;
    $prefs->suspend_if_no_recent_input = $suspend_if_no_recent_input;
    $prefs->suspend_cpu_usage = $suspend_cpu_usage;
    $prefs->start_hour = $start_hour;
    $prefs->end_hour = $end_hour;
    $prefs->leave_apps_in_memory = ($leave_apps_in_memory == "yes");
    $prefs->cpu_scheduling_period_minutes = $cpu_scheduling_period_minutes;
    $prefs->max_cpus = $max_cpus;
    $prefs->max_ncpus_pct = $max_ncpus_pct;
    $prefs->cpu_usage_limit = $cpu_usage_limit;
    $prefs->disk_max_used_gb = $disk_max_used_gb;
    $prefs->disk_min_free_gb = $disk_min_free_gb;
    $prefs->disk_max_used_pct = $disk_max_used_pct;
    $prefs->disk_interval = $disk_interval;
    $prefs->vm_max_used_pct = $vm_max_used_pct;
    $prefs->ram_max_used_busy_pct = $ram_max_used_busy_pct;
    $prefs->ram_max_used_idle_pct = $ram_max_used_idle_pct;
    $prefs->work_buf_min_days = $work_buf_min_days;
    $prefs->work_buf_additional_days = $work_buf_additional_days;
    $prefs->confirm_before_connecting = ($confirm_before_connecting == "yes");
    $prefs->hangup_if_dialed = ($hangup_if_dialed == "yes");
    $prefs->max_bytes_sec_down = max_bytes_db_mode($max_bytes_sec_down);
    $prefs->max_bytes_sec_up = max_bytes_db_mode($max_bytes_sec_up);
    $prefs->net_start_hour = $net_start_hour;
    $prefs->net_end_hour = $net_end_hour;
    $prefs->dont_verify_images = ($dont_verify_images == "yes");

    return $error;
}

// This functions parses and verifies the resource value
// @param object &$prefs reference to object that stores prefs
// @return object an object with errorvalues or false in success case
function prefs_resource_parse_form(&$prefs) {
    global $app_types;
    $error = false;
    $resource_share = $_GET['resource_share'];
    if (!verify_numeric($resource_share, 0)) $error->resource_share = true;
    $prefs->resource_share = $resource_share;

    if ($app_types->count > 1) {
        if ($app_types->cpu) $prefs->no_cpu = ($_GET['use_cpu'])?0:1;
        if ($app_types->cuda) $prefs->no_cuda = ($_GET['use_cuda'])?0:1;
        if ($app_types->ati) $prefs->no_ati = ($_GET['use_ati'])?0:1;
    }
    return $error;
}

function prefs_beta_parse_form(&$prefs) {
    $prefs->allow_beta_work = ($_GET['allow_beta_work'] == "yes")?1:0;
}

function prefs_privacy_parse_form(&$user) {
    $user->send_email = ($_GET['send_email'] == "yes")?1:0;
    $user->show_hosts = ($_GET['show_hosts'] == "yes")?1:0;
}

// This function parses the project specific prefs form.
// For details look into project/project_specific_prefs.inc
// @param object &$prefs refernce to preference object
// @return object an object with errorvalues or false in success case
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
    // N.B.: each XML entry must end with \n due to the sloppy parsing by the
    // BOINC client!!
    $xml = "";
    if ($primary) {
        $xml = "<global_preferences>\n";
        $now = time();
        $xml = $xml."<mod_time>$now</mod_time>\n";
    }

    $x = $prefs->suspend_while_on_batteries?0:1;
    $xml = $xml."<run_on_batteries>$x</run_on_batteries>\n";

    $x = $prefs->suspend_if_user_active?0:1;
    $xml = $xml."<run_if_user_active>$x</run_if_user_active>\n";

    $x = $prefs->suspend_gpu_if_user_active?0:1;
    $xml = $xml."<run_gpu_if_user_active>$x</run_gpu_if_user_active>\n";

    $xml = $xml."<idle_time_to_run>$prefs->idle_time_to_run</idle_time_to_run>\n";
    $xml = $xml."<suspend_if_no_recent_input>$prefs->suspend_if_no_recent_input</suspend_if_no_recent_input>\n";
    $xml = $xml."<suspend_cpu_usage>$prefs->suspend_cpu_usage</suspend_cpu_usage>\n";

    if ($prefs->start_hour != $prefs->end_hour) {
        $xml = $xml."<start_hour>$prefs->start_hour</start_hour>\n"
            ."<end_hour>$prefs->end_hour</end_hour>\n";
    }
    if ($prefs->leave_apps_in_memory) {
        $xml = $xml."<leave_apps_in_memory/>\n";
    }
    $xml = $xml."<cpu_scheduling_period_minutes>$prefs->cpu_scheduling_period_minutes</cpu_scheduling_period_minutes>\n";
    if ($prefs->confirm_before_connecting) {
        $xml = $xml."<confirm_before_connecting/>\n";
    }

    $x = $prefs->hangup_if_dialed?1:0;
    $xml = $xml."<hangup_if_dialed>$x</hangup_if_dialed>\n";

    $xml = $xml
        ."<work_buf_min_days>$prefs->work_buf_min_days</work_buf_min_days>\n"
        ."<work_buf_additional_days>$prefs->work_buf_additional_days</work_buf_additional_days>\n"
        ."<max_cpus>$prefs->max_cpus</max_cpus>\n"
        ."<max_ncpus_pct>$prefs->max_ncpus_pct</max_ncpus_pct>\n"
        ."<cpu_usage_limit>$prefs->cpu_usage_limit</cpu_usage_limit>\n"
        ."<disk_interval>$prefs->disk_interval</disk_interval>\n";
    $xml = $xml
        ."<disk_max_used_gb>$prefs->disk_max_used_gb</disk_max_used_gb>\n"
        ."<disk_max_used_pct>$prefs->disk_max_used_pct</disk_max_used_pct>\n"
        ."<disk_min_free_gb>$prefs->disk_min_free_gb</disk_min_free_gb>\n"
        ."<vm_max_used_pct>$prefs->vm_max_used_pct</vm_max_used_pct>\n"
        ."<ram_max_used_busy_pct>$prefs->ram_max_used_busy_pct</ram_max_used_busy_pct>\n"
        ."<ram_max_used_idle_pct>$prefs->ram_max_used_idle_pct</ram_max_used_idle_pct>\n"
        ."<max_bytes_sec_down>$prefs->max_bytes_sec_down</max_bytes_sec_down>\n"
        ."<max_bytes_sec_up>$prefs->max_bytes_sec_up</max_bytes_sec_up>\n";
    if ($prefs->net_start_hour != $prefs->net_end_hour) {
        $xml = $xml."<net_start_hour>$prefs->net_start_hour</net_start_hour>\n"
            ."<net_end_hour>$prefs->net_end_hour</net_end_hour>\n";
    }

    $x = $prefs->dont_verify_images?1:0;
    $xml = $xml."<dont_verify_images>$x</dont_verify_images>\n";

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
    $xml = "";
    if ($primary) {
        $xml = "<project_preferences>\n";
    }
    $xml = $xml ."<resource_share>$prefs->resource_share</resource_share>\n";
    if ($prefs->allow_beta_work) {
        $xml = $xml . "<allow_beta_work>1</allow_beta_work>\n";
    }
    if ($prefs->no_cpu) {
        $xml = $xml . "<no_cpu>1</no_cpu>\n";
    }
    if ($prefs->no_cuda) {
        $xml = $xml . "<no_cuda>1</no_cuda>\n";
    }
    if ($prefs->no_ati) {
        $xml = $xml . "<no_ati>1</no_ati>\n";
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
