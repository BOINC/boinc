<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

// strings describing various preference fields
//
define("CPU_LIMIT_DESC", "Processor usage");
define("SUSPEND_WHILE_ON_BATTERIES_DESC",
    "Suspend work while computer is on battery power?
    <br><span class=note>(matters only for portable computers)</span>"
);
define("RUN_IF_USER_ACTIVE_DESC",
    "Suspend work while computer is in use?"
);
define("IDLE_TIME_TO_RUN_DESC", "'In use' means mouse/keyboard activity in last");
define("SUSPEND_IF_NO_RECENT_INPUT_DESC",
    "Suspend work if no mouse/keyboard activity in last
    <br><span class=note>(Needed to enter low-power mode on some computers)
    <br>Enforced by version 5.10.14+</span>"
);
define("START_END_DESC", "Do work only between the hours of");
define("START_END_DESC2", "<br><span class=note>(no restriction if equal)</span>");
define("LEAVE_APPS_IN_MEMORY_DESC",
    "Leave applications in memory while suspended?
    <br><span class=note>(suspended applications will consume swap space if 'yes')</span>");
define("CPU_SCHEDULING_DESC", "Switch between applications every
    <br><span class=note>(recommended: 60 minutes)</span>");
define("CONFIRM_BEFORE_CONNECTING_DESC",
    "Confirm before connecting to Internet?
    <br><span class=note>(matters only if you have a modem, ISDN or VPN connection)</span>"
);
define("HANGUP_IF_DIALED_DESC",
    "Disconnect when done?
    <br><span class=note>(matters only if you have a modem, ISDN or VPN connection)</span>"
);
define("WORK_BUF_MIN_DAYS_DESC",
    "Computer is connected to the Internet about every
    <br><span class=note>(Leave blank or 0 if always connected.
    <br>BOINC will try to maintain at least this much work.)</span>
    "
);
define("WORK_BUF_ADDITIONAL_DAYS_DESC",
    "Maintain enough work for an additional
    <br><span class=note>Enforced by version 5.10+</span>
    "
);
define("MAX_NCPUS_PCT_DESC", "On multiprocessors, use at most<span class=note><br>Enforced by version 6.1+</span>");
define("MAX_NCPUS_PCT_DESC2", "% of the processors");
define("MAX_CPUS_DESC", "On multiprocessors, use at most<span class=note><br>Enforced by version 5.10 and earlier</span>");
define("MAX_CPUS_DESC2", "processors");
define("USE_AT_MOST", "Use at most");
define("USE_AT_MOST2", "Use at most<span class=note><br>(Can be used to reduce CPU heat)<br>Enforced by version 5.6+</span>");
define("USE_AT_MOST3", "Use at most<br><span class=note>Enforced by version 5.8+</span>");
define("CPU_USAGE_LIMIT_DESC2", "percent of CPU time");
define("DISK_INTERVAL_DESC", "Write to disk at most every");
define("DISK_LIMIT_DESC", "Disk and memory usage");
define("DISK_MIN_FREE_GB_DESC", "Leave at least");
define("DISK_SCHED_MIN_FREE_DESC1", "<br><span class=note>(Values smaller than ");
define("DISK_SCHED_MIN_FREE_DESC2", " are ignored)</span>");
define("DISK_MAX_USED_PCT_DESC2", "% of total disk space");
define("VM_MAX_USED_PCT_DESC2", "% of page file (swap space)");
define("RAM_MAX_USED_BUSY_PCT_DESC2", "% of memory when computer is in use");
define("RAM_MAX_USED_IDLE_PCT_DESC2", "% of memory when computer is not in use");
define("NETWORK_LIMIT_DESC", "Network usage");
define("MAX_BYTES_SEC_DOWN_DESC", "Maximum download rate:");
define("MAX_BYTES_SEC_UP_DESC", "Maximum upload rate:");
define("NET_START_END_DESC",
    "Use network only between the hours of
    <br><span class=note>Enforced by version 4.46+</span>"
);
define("DONT_VERIFY_IMAGES_DESC",
    "Skip image file verification?
    <br><span class=note>Check this ONLY if your Internet provider
    modifies image files (UMTS does this, for example).
    <br>Skipping verification reduces the security of BOINC.</span>"
);

define("BYTE_CONVERSION", 1000.0);
define("BYTE_UNITS", "Kbytes/sec");
define("BYTE_ABBR", "KB/s");
define("ALLOW_BETA_WORK",
    "Run test applications?<br>
    <span class=note>This helps us develop applications,
    but may cause jobs to fail on your computer</span>"
);

// These texts are used in multiple places in prefs_edit.php and add_venue.php
define("PREFS_FORM_DESC1", "These preferences apply to all the BOINC projects in which you participate.<br><br>");
define("PREFS_FORM_ERROR_DESC", "<b>Unable to update preferences.</b> The values marked in red below were out of range or not numeric.<br><br>");

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
    error_page("bad venue: $x");
}

function check_subset($x) {
    if ($x == "global") return;
    if ($x == "project") return;
    error_page("bad subset: $x");
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
        $parse_result = initialize_prefs_before_parsing_global();
        //echo "VENUE PREFS AFTER INIT: <pre>";
        //var_dump($parse_result);
        //echo "</pre>";
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
        //echo "VENUE PREFS AFTER PARSE: <pre>";
        //var_dump($parse_result);
        //echo "</pre>";
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
        $parse_result->allow_beta_work_text = $parse_result->allow_beta_work?"yes":"no";
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
        $parse_result->suspend_if_user_active = false;
        break;
    case "idle_time_to_run":
        $parse_result->idle_time_to_run = $text;
        break;
    case "suspend_if_no_recent_input":
        $parse_result->suspend_if_no_recent_input = $text;
        break;
    case "start_hour":
        $parse_result->start_hour = $text;
        break;
    case "end_hour":
        $parse_result->end_hour = $text;
        break;
    case "leave_apps_in_memory":
        $parse_result->leave_apps_in_memory = true;
        break;
    case "cpu_scheduling_period_minutes":
        $parse_result->cpu_scheduling_period_minutes = $text;
        break;
    case "confirm_before_connecting":
        $parse_result->confirm_before_connecting = true;
        break;
    case "hangup_if_dialed":
        $parse_result->hangup_if_dialed = true;
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
        $parse_result->dont_verify_images = true;
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


// state of prefs for new users
//
function default_prefs_global() {
    $p = null;
    $p->suspend_while_on_batteries = true;
    $p->suspend_if_user_active = false;
    $p->idle_time_to_run = 3;
    $p->suspend_if_no_recent_input = 0;
    $p->start_hour = 0;
    $p->end_hour = 0;
    $p->leave_apps_in_memory = false;
    $p->cpu_scheduling_period_minutes = 60;
    $p->confirm_before_connecting = false;
    $p->hangup_if_dialed = true;
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
    $p->allow_beta_work = false;
    $p->allow_beta_work_text = "no";
    $p->project_specific = project_specific_prefs_default();
    return $p;
}

// state of prefs before parsing; initialize booleans
//
function initialize_prefs_before_parsing_global() {
    $p = default_prefs_global();
    $p->suspend_while_on_batteries = true;
    $p->suspend_if_user_active = true;
    $p->leave_apps_in_memory = false;
    $p->confirm_before_connecting = false;
    $p->hangup_if_dialed = false;
    $p->dont_verify_images = false;
    
    return $p;
}

function initialize_prefs_before_parsing_project() {
    $p = default_prefs_project();
    return $p;
}

// parse prefs from XML to a struct
//
function prefs_parse_project($prefs_xml) {
    global $parse_result;
    $parse_result = initialize_prefs_before_parsing_project();
    $xml_parser = xml_parser_create();
    xml_parser_set_option($xml_parser, XML_OPTION_CASE_FOLDING, 0);
    xml_set_element_handler($xml_parser, "element_start_project", "element_end_project");
    xml_set_character_data_handler($xml_parser, "char_handler");
    xml_parse($xml_parser, $prefs_xml, 1);
    return $parse_result;
}

function prefs_parse_global($prefs_xml) {
    global $parse_result;
    $parse_result = initialize_prefs_before_parsing_global();
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
    $hom  = isset($prefs->home) ? $prefs->home->$item : "--";
    $schl = isset($prefs->school) ? $prefs->school->$item : "--";
    $wrk  = isset($prefs->work) ? $prefs->work->$item : "--";

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
        echo $value ?"yes":"no";
        break;
    case "limit":
        $x = max_bytes_display_mode($value);
        $y = "$x " . BYTE_ABBR;
        echo $x ? "$y" : "no limit";
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
    $post_add = "&subset=$subset&cols=1$tokens>Add</a>";
    $post_edit = "&subset=$subset&cols=1$tokens>Edit</a>";
    $post_remove = "&subset=$subset&cols=1$tokens>Remove</a>";
    $gen = "<a href=prefs_edit.php?subset=$subset&cols=1$tokens>Edit</a>";

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
    row_defs(IDLE_TIME_TO_RUN_DESC, "idle_time_to_run", " minutes", "", $prefs);

    row_defs(SUSPEND_IF_NO_RECENT_INPUT_DESC, "suspend_if_no_recent_input", "minutes", "minutes", $prefs);

    row_hours(START_END_DESC, "run", $prefs);

    row_defs(LEAVE_APPS_IN_MEMORY_DESC, "leave_apps_in_memory", "", "yesno", $prefs);
    row_defs(CPU_SCHEDULING_DESC, "cpu_scheduling_period_minutes", " minutes", "", $prefs);
    row_defs(MAX_CPUS_DESC, "max_cpus", MAX_CPUS_DESC2, "", $prefs);
    row_defs(MAX_NCPUS_PCT_DESC, "max_ncpus_pct", MAX_NCPUS_PCT_DESC2, "", $prefs);
    row_defs(USE_AT_MOST2, "cpu_usage_limit", CPU_USAGE_LIMIT_DESC2, "", $prefs);

    row_top(DISK_LIMIT_DESC);
    row_defs(USE_AT_MOST, "disk_max_used_gb", " GB disk space", "", $prefs);
    row_defs(DISK_MIN_FREE_GB_DESC, "disk_min_free_gb", " GB disk space free", "", $prefs);
    row_defs(USE_AT_MOST, "disk_max_used_pct", DISK_MAX_USED_PCT_DESC2, "", $prefs);
    row_defs(DISK_INTERVAL_DESC, "disk_interval", " seconds", "", $prefs);
    row_defs(USE_AT_MOST, "vm_max_used_pct", VM_MAX_USED_PCT_DESC2, "", $prefs);
    row_defs(USE_AT_MOST3, "ram_max_used_busy_pct", RAM_MAX_USED_BUSY_PCT_DESC2, "", $prefs);
    row_defs(USE_AT_MOST3, "ram_max_used_idle_pct", RAM_MAX_USED_IDLE_PCT_DESC2, "", $prefs);

    row_top(NETWORK_LIMIT_DESC);
    row_defs(WORK_BUF_MIN_DAYS_DESC, "work_buf_min_days", " days", "", $prefs);
    row_defs(WORK_BUF_ADDITIONAL_DAYS_DESC, "work_buf_additional_days", " days", "", $prefs);
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
    row2(SUSPEND_WHILE_ON_BATTERIES_DESC, $prefs->suspend_while_on_batteries?"yes":"no");
    row2(RUN_IF_USER_ACTIVE_DESC, $prefs->suspend_if_user_active?"yes":"no");
    row2(IDLE_TIME_TO_RUN_DESC, "$prefs->idle_time_to_run minutes");

    $x = show_double($prefs->suspend_if_no_recent_input);
    row2(SUSPEND_IF_NO_RECENT_INPUT_DESC, "$x minutes");

    if ($prefs->start_hour == $prefs->end_hour) {
        $x = "(no restriction)";
    } else {
        $s = hour_str($prefs->start_hour);
        $e = hour_str($prefs->end_hour);
        $x = "$s and $e";
    }
    row2(START_END_DESC, $x);
    row2(LEAVE_APPS_IN_MEMORY_DESC, $prefs->leave_apps_in_memory?"yes":"no");
    row2(CPU_SCHEDULING_DESC, "$prefs->cpu_scheduling_period_minutes minutes");
    $x = $prefs->max_cpus?$prefs->max_cpus:'---';
    row2(MAX_CPUS_DESC, "$x ".MAX_CPUS_DESC2);
    $x = $prefs->max_ncpus_pct?$prefs->max_ncpus_pct:'---';
    row2(MAX_NCPUS_PCT_DESC, "$x ".MAX_NCPUS_PCT_DESC2);
    row2(USE_AT_MOST2, "$prefs->cpu_usage_limit ".CPU_USAGE_LIMIT_DESC2);
    row1(DISK_LIMIT_DESC);
    row2(USE_AT_MOST, "$prefs->disk_max_used_gb GB disk space");

    $dp = get_disk_space_config();

    $msg = null;
    if ($dp->new_sched_flag) {
        $msg = DISK_SCHED_MIN_FREE_DESC1.$dp->sched_disk_min_free_gb.DISK_SCHED_MIN_FREE_DESC2;
    }
    row2(DISK_MIN_FREE_GB_DESC.$msg, "$prefs->disk_min_free_gb GB disk space free");
    row2(USE_AT_MOST, "$prefs->disk_max_used_pct".DISK_MAX_USED_PCT_DESC2);
    row2(DISK_INTERVAL_DESC, "$prefs->disk_interval seconds");
    row2(USE_AT_MOST, "$prefs->vm_max_used_pct".VM_MAX_USED_PCT_DESC2);
    row2(USE_AT_MOST3, "$prefs->ram_max_used_busy_pct".RAM_MAX_USED_BUSY_PCT_DESC2);
    row2(USE_AT_MOST3, "$prefs->ram_max_used_idle_pct".RAM_MAX_USED_IDLE_PCT_DESC2);
    row1(NETWORK_LIMIT_DESC);
    row2(WORK_BUF_MIN_DAYS_DESC, "$prefs->work_buf_min_days days");
    row2(WORK_BUF_ADDITIONAL_DAYS_DESC, "$prefs->work_buf_additional_days days");
    row2(CONFIRM_BEFORE_CONNECTING_DESC, $prefs->confirm_before_connecting?"yes":"no");
    row2(HANGUP_IF_DIALED_DESC, $prefs->hangup_if_dialed?"yes":"no");
    $x = max_bytes_display_mode($prefs->max_bytes_sec_down);
    $y = "$x " . BYTE_ABBR;
    row2(MAX_BYTES_SEC_DOWN_DESC, $x?"$y":"no limit");
    $x = max_bytes_display_mode($prefs->max_bytes_sec_up);
    $y = "$x " . BYTE_ABBR;
    row2(MAX_BYTES_SEC_UP_DESC, $x?"$y":"no limit");
    if ($prefs->net_start_hour == $prefs->net_end_hour) {
        $x = "(no restriction)";
    } else {
        $s = hour_str($prefs->net_start_hour);
        $e = hour_str($prefs->net_end_hour);
        $x = "$s and $e";
    }
    row2(NET_START_END_DESC, $x);
    row2(DONT_VERIFY_IMAGES_DESC, $prefs->dont_verify_images?"yes":"no");
}

function prefs_show_resource($prefs, $columns=false) {
    $x = "Resource share<br>
        <span class=note>If you participate in multiple BOINC projects,
        this is the proportion of your resources used by ".PROJECT."</span>";
    if ($columns) {
        row_defs($x, "resource_share", "", "" ,$prefs);
    } else {
        row2($x, $prefs->resource_share);
    }
}

function prefs_show_beta($prefs, $columns=false) {
    if ($columns) {
        row_defs(ALLOW_BETA_WORK, "allow_beta_work_text", "", "" ,$prefs);
    } else {
        row2(ALLOW_BETA_WORK, $prefs->allow_beta_work_text);
    }
}

function prefs_show_privacy($user) {
    $x = "";
    if (defined('EMAIL_FROM')) {
        $x = "<br><span class=note>Emails will be sent from ".EMAIL_FROM.";
            make sure your spam filter accepts this address.</span>";
    }
    row2("Is it OK for ".PROJECT." and your team (if any) to email you? $x",
        $user->send_email?"yes":"no"
    );
    row2("Should ".PROJECT." show your computers on its web site?", $user->show_hosts?"yes":"no");
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
    if ($subset == "global") return "Computing";
    return PROJECT;
}

function prefs_display_venue($prefs, $venue, $subset) {
    global $project_has_beta;
    global $g_logged_in_user;
    $tokens = url_tokens($g_logged_in_user->authenticator);
    $x = false;
    if (isset($prefs->$venue)) $x = $prefs->$venue;

    if ($x) {
        row1("Separate preferences for $venue", 2, "heading");
        echo "<tr><td colspan=2>";
        start_table();
        if ($subset == "global") {
            prefs_show_global($x);
        } else {
            prefs_show_resource($x);
            if ($project_has_beta) prefs_show_beta($x);
            prefs_show_project($x);
        }
        row2("<br>", "<a href=prefs_edit.php?venue=$venue&subset=$subset$tokens>Edit preferences</a> | <a href=prefs_remove.php?venue=$venue&subset=$subset$tokens>Remove</a>");
        end_table();
        echo "</td></tr>\n";
    } else {
        //$x = subset_name($subset);
        row1("<a href=add_venue.php?venue=$venue&subset=$subset$tokens>Add separate preferences for $venue</a>", 2, "heading");
    }
}

function print_prefs_display_project($user, $columns=false) {
    global $project_has_beta;
    $project_prefs = prefs_parse_project($user->project_prefs);

    start_table();
    $switch_link = " <font size=\"-2\"><a href=prefs.php?subset=project&cols=". (int)!$columns .">(Switch View)</a></font>";
    if ($columns) {
        row1("Combined preferences".$switch_link, 2, "heading");
        echo "<tr><td colspan=2>";
        start_table();
        prefs_show_privacy($user);
        venue_show($user);
        row_top("Project specific settings");
        prefs_show_resource($project_prefs, true);
        if ($project_has_beta) prefs_show_beta($project_prefs, true);
        prefs_show_project($project_prefs, true);
        row_links("project", $project_prefs);
        end_table();
        echo "</td></tr>\n";
    } else {
        if (isset($project_prefs->home) || isset($project_prefs->work) || isset($project_prefs->school)) {
            row1("Primary (default) preferences".$switch_link, 2, "heading");
        }
        echo "<tr><td colspan=2>";
        start_table();
        prefs_show_resource($project_prefs, false);
        if ($project_has_beta) prefs_show_beta($project_prefs, false);
        prefs_show_privacy($user);
        venue_show($user);
        prefs_show_project($project_prefs, false);
        $tokens = url_tokens($user->authenticator);
        row2("", "<a href=prefs_edit.php?subset=project$tokens>Edit ".PROJECT." preferences</a>");
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

    echo "<span class=note>
        These apply to all BOINC projects in which you participate.
        <br>
        On computers attached to multiple projects,
        the most recently modified preferences will be used.
        </span>
        <br><br>
    ";
    if (isset($global_prefs->mod_time)) {
        echo "
            Preferences last modified: ", pretty_time_str($global_prefs->mod_time), "
        ";
    }
    echo "
        <br><br>
    ";
    $switch_link = " <font size=\"-2\"><a href=prefs.php?subset=global&cols=". (int)!$columns .">(Switch View)</a></font>";
    start_table();
    if ($columns) {
        row1("Combined preferences".$switch_link, 2, "heading");
        echo "<tr><td colspan=2>";
        start_table();
        prefs_show_columns_global($global_prefs);
        end_table();
        echo "</td></tr>\n";

    } else {
        if (isset($global_prefs->home) || isset($global_prefs->work) || isset($global_prefs->school)) {
            row1("Primary (default) preferences".$switch_link, 2, "heading");
        }
        echo "<tr><td colspan=2>";
        start_table();
        prefs_show_global($global_prefs);
        $tokens = url_tokens($user->authenticator);
        row2("<br>", "<a href=prefs_edit.php?subset=global$tokens>Edit preferences</a>");
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
        $submit_value = "Add preferences";
    }
    if ($action == "edit") {
        $script = "prefs_edit.php";
        $submit_value = "Update preferences";
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

    row2("<br>", "<input type=submit value=\"$submit_value\" name=\"action\">");
    end_table();
    echo "</form>\n";
}

////////////////////////////////////////////
//
// Functions to display preference subsets as forms
//
function prefs_form_global($user, $prefs, $error=false) {
    //echo "IN PREFS_FORM_GLOBAL: <pre>";
    //var_dump($prefs);
    //echo "</pre>";
    row1(CPU_LIMIT_DESC);
    $y = "yes <input type=radio name=suspend_while_on_batteries value=yes "
        .($prefs->suspend_while_on_batteries?"checked":"")
        ."> no <input type=radio name=suspend_while_on_batteries value=no "
        .($prefs->suspend_while_on_batteries?"":"checked")
        .">
    ";
    row2(SUSPEND_WHILE_ON_BATTERIES_DESC, $y);

    $y = "yes <input type=radio name=suspend_if_user_active value=yes "
        .($prefs->suspend_if_user_active?"checked":"")
        ."> no <input type=radio name=suspend_if_user_active value=no "
        .($prefs->suspend_if_user_active?"":"checked")
        .">
    ";
    row2(RUN_IF_USER_ACTIVE_DESC, $y);

    $y = "<input size=5 name=idle_time_to_run value='$prefs->idle_time_to_run'> minutes ";
    $show_error = false;
    if (isset($error->idle_time_to_run)) $show_error= true;
    row2(IDLE_TIME_TO_RUN_DESC, $y, $show_error);

    $y = "<input size=5 name=suspend_if_no_recent_input value='$prefs->suspend_if_no_recent_input'> minutes ";
    $show_error = false;
    if (isset($error->suspend_if_no_recent_input)) $show_error= true;
    row2(SUSPEND_IF_NO_RECENT_INPUT_DESC, $y, $show_error);

    $x = START_END_DESC.START_END_DESC2;
    $y = hour_select($prefs->start_hour, "start_hour")."and".hour_select($prefs->end_hour, "end_hour");
    row2($x, $y);

    $x = LEAVE_APPS_IN_MEMORY_DESC;
    $y = "yes <input type=radio name=leave_apps_in_memory value=yes "
        .($prefs->leave_apps_in_memory?"checked":"")
        ."> no <input type=radio name=leave_apps_in_memory value=no "
        .($prefs->leave_apps_in_memory?"":"checked")
        .">
    ";
    row2($x, $y);

    $y = "<input size=5 name=cpu_scheduling_period_minutes value='$prefs->cpu_scheduling_period_minutes'> minutes ";
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
        "<input size=7 name=disk_max_used_gb value='$prefs->disk_max_used_gb'> Gbytes",
        $show_error
    );

    $dp = get_disk_space_config();

    $msg=null;
    if ($dp->new_sched_flag) {
        $msg = DISK_SCHED_MIN_FREE_DESC1.$dp->sched_disk_min_free_gb.DISK_SCHED_MIN_FREE_DESC2;
    } 

    $show_error = false;
    if (isset($error->disk_min_free_gb)) $show_error= true;
    row2(DISK_MIN_FREE_GB_DESC.$msg,
        "<input size=7 name=disk_min_free_gb value='$prefs->disk_min_free_gb'> Gbytes free",
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
        "<input size=6 name=disk_interval value=$prefs->disk_interval> seconds",
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
    $y = "<input size=5 name=work_buf_min_days value='$prefs->work_buf_min_days'> days";
    $show_error = false;
    if (isset($error->work_buf_min_days)) $show_error= true;
    row2($x, $y, $show_error);

    $x = WORK_BUF_ADDITIONAL_DAYS_DESC;
    $y = "<input size=5 name=work_buf_additional_days value='$prefs->work_buf_additional_days'> days";
    $show_error = false;
    if (isset($error->work_buf_additional_days)) $show_error= true;
    row2($x, $y, $show_error);

    $x = CONFIRM_BEFORE_CONNECTING_DESC;
    $y = "yes <input type=radio name=confirm_before_connecting value=yes "
        .($prefs->confirm_before_connecting?"checked":"")
        ."> no <input type=radio name=confirm_before_connecting value=no "
        .($prefs->confirm_before_connecting?"":"checked")
        .">
    ";
    row2($x, $y);

    $x = HANGUP_IF_DIALED_DESC;
    $y = "yes <input type=radio name=hangup_if_dialed value=yes "
        .($prefs->hangup_if_dialed?"checked":"")
        ."> no <input type=radio name=hangup_if_dialed value=no "
        .($prefs->hangup_if_dialed?"":"checked")
        .">
    ";
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
    $y = hour_select($prefs->net_start_hour, "net_start_hour")."and".hour_select($prefs->net_end_hour, "net_end_hour");
    row2($x, $y);
    $x = DONT_VERIFY_IMAGES_DESC;
    $y = "yes <input type=radio name=dont_verify_images value=yes "
        .($prefs->dont_verify_images?"checked":"")
        ."> no <input type=radio name=dont_verify_images value=no "
        .($prefs->dont_verify_images?"":"checked")
        .">
    ";
    row2($x, $y);

}

function prefs_form_privacy($user) {
    $y = "yes <input type=radio name=send_email value=yes "
        .($user->send_email?"checked":"")
        ."> no <input type=radio name=send_email value=no "
        .($user->send_email?"":"checked")
        .">
    ";
    row2("Is it OK for ".PROJECT." and your team (if any) to email you?", $y);

    $y = "yes <input type=radio name=show_hosts value=yes "
        .($user->show_hosts?"checked":"")
        ."> no <input type=radio name=show_hosts value=no "
        .($user->show_hosts?"":"checked")
        .">
    ";
    row2("Should ".PROJECT." show your computers on its web site?", $y);
}

function prefs_form_resource($prefs, $error=false) {
    $show_error = false;
    if (isset($error->resource_share)) $show_error= true;
    row2(
        "<b>Resource share:</b>
        <span class=note><br>Determines the proportion of your computer's resources
        allocated to this project.
        Example: if you participate in two BOINC projects with
        resource shares of 100 and 200,
        the first will get 1/3 of your resources and the second will get 2/3.
        </span>",
        "<input name=resource_share value='$prefs->resource_share'>",
        $show_error
    );
}

function prefs_form_beta($prefs, $error=false) {
    $ychecked = $prefs->allow_beta_work?"checked":"";
    $nchecked = $prefs->allow_beta_work?"":"checked";
    row2(
        ALLOW_BETA_WORK,
        "yes <input type=radio checkbox name=allow_beta_work value=1 $ychecked>
        no <input type=radio checkbox name=allow_beta_work value=0 $nchecked>"
    );
}

function prefs_form_project($prefs_xml, $error=false) {
    $prefs = project_specific_prefs_parse($prefs_xml);
    project_specific_prefs_edit($prefs, $error);
}

function venue_show($user) {
    $venue = $user->venue;
    if ($venue =='') $venue = '---';
    row2("Default computer location", $venue);
}

function venue_form($user) {
    $n=$h=$w=$s='';
    if ($user->venue == '') $n = 'selected';
    if ($user->venue == 'home') $h = 'selected';
    if ($user->venue == 'work') $w = 'selected';
    if ($user->venue == 'school') $s = 'selected';
    row2('Default computer location',
        "<select name=default_venue>
        <option value=\"\" $n>---
        <option value=home $h>Home
        <option value=work $w>Work
        <option value=school $s>School
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
    $idle_time_to_run = $_GET["idle_time_to_run"];
    $suspend_if_no_recent_input = $_GET["suspend_if_no_recent_input"];
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
    $prefs->idle_time_to_run = $idle_time_to_run;
    $prefs->suspend_if_no_recent_input = $suspend_if_no_recent_input;
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
    $error = false;
    $resource_share = $_GET['resource_share'];
    if (!verify_numeric($resource_share, 0)) $error->resource_share = true;
    $prefs->resource_share = $resource_share;

    return $error;
}

function prefs_beta_parse_form(&$prefs) {
    $prefs->allow_beta_work = false;
    $prefs->allow_beta_work_text = "no";
    $x = $_GET['allow_beta_work'];
    if ($x) {
        $prefs->allow_beta_work = true;
        $prefs->allow_beta_work_text = "yes";
    }
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
    if (!$prefs->suspend_while_on_batteries) {
        $xml = $xml."<run_on_batteries/>\n";
    }
    if (!$prefs->suspend_if_user_active) {
        $xml = $xml."<run_if_user_active/>\n";
    }
    $xml = $xml."<idle_time_to_run>$prefs->idle_time_to_run</idle_time_to_run>\n";
    $xml = $xml."<suspend_if_no_recent_input>$prefs->suspend_if_no_recent_input</suspend_if_no_recent_input>\n";

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
    if ($prefs->hangup_if_dialed) {
        $xml = $xml."<hangup_if_dialed/>\n";
    }
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
    if ($prefs->dont_verify_images) {
        $xml = $xml."<dont_verify_images/>\n";
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
    $xml = "";
    if ($primary) {
        $xml = "<project_preferences>\n";
    }
    if ($prefs->resource_share) {
        $xml = $xml
            ."<resource_share>$prefs->resource_share</resource_share>\n";
    }
    if ($prefs->allow_beta_work) {
        $xml = $xml . "<allow_beta_work>1</allow_beta_work>\n";
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
    $prefs_xml = global_prefs_make_xml($prefs);
    $query = "update user set global_prefs='$prefs_xml' where id=$user->id";
    $retval = mysql_query($query);
    if (!$retval) {
        echo "Update failed: ".htmlspecialchars($query)."\n";
        echo mysql_error();
        exit();
    }
    $user->global_prefs = $prefs_xml;
    return $retval;
}

function project_prefs_update(&$user, $prefs) {
    $prefs_xml = project_prefs_make_xml($prefs);
    $retval = mysql_query("update user set project_prefs='$prefs_xml', send_email=$user->send_email, show_hosts=$user->show_hosts where id=$user->id");
    $user->project_prefs = $prefs_xml;
    return $retval;
}

?>
