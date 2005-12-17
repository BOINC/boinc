<?php

// This file contains support functions for display and editing
// preferences (global and project).
// Preferences are represented in two ways:
// - As a PHP structure (usually called $prefs)
//   This has fields run_on_batteries, etc.
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
define("RUN_ON_BATTERIES_DESC",
    "Do work while computer is running on batteries?
    <br><font size=-2>(matters only for portable computers)</font>"
);
define("RUN_IF_USER_ACTIVE_DESC",
    "Do work while computer is in use?"
);
define("IDLE_TIME_TO_RUN_DESC", "Do work only after computer is idle for");
define("IDLE_TIME_TO_RUN_DESC2", "<br><font size=-2>(applies only if above is 'no')</font>");
define("START_END_DESC", "Do work only between the hours of");
define("START_END_DESC2", "<br><font size=-2>(no restriction if equal)</font>");
define("LEAVE_APPS_IN_MEMORY_DESC",
    "Leave applications in memory while preempted?
    <br><font size=-2>(suspended applications will consume swap space if 'yes')</font>");
define("CPU_SCHEDULING_DESC", "Switch between applications every
    <br><font size=-2>(recommended: 60 minutes)</font>");
define("CONFIRM_BEFORE_CONNECTING_DESC",
    "Confirm before connecting to Internet?
    <br><font size=-2>(matters only if you have a modem, ISDN or VPN connection)</font>"
);
define("HANGUP_IF_DIALED_DESC",
    "Disconnect when done?
    <br><font size=-2>(matters only if you have a modem, ISDN or VPN connection)</font>"
);
define("WORK_BUF_DESC",
    "Connect to network about every
    <br><font size=-2>(determines size of work cache; maximum 10 days)</font>
    "
);
define("MAX_CPUS_DESC", "On multiprocessors, use at most");
define("MAX_CPUS_DESC2", "processors");
define("DISK_INTERVAL_DESC", "Write to disk at most every");
define("DISK_LIMIT_DESC", "Disk and memory usage");
define("DISK_MAX_USED_GB_DESC", "Use no more than");
define("DISK_MIN_FREE_GB_DESC", "Leave at least");
define("DISK_SCHED_MIN_FREE_DESC1", "<br><font size=-2>(Values smaller than ");
define("DISK_SCHED_MIN_FREE_DESC2", " are ignored)</font>");
define("DISK_MAX_USED_PCT_DESC", "Use no more than");
define("DISK_MAX_USED_PCT_DESC2", "% of total disk space");
define("VM_MAX_USED_PCT_DESC", "Use no more than");
define("VM_MAX_USED_PCT_DESC2", "% of total virtual memory");
define("NETWORK_LIMIT_DESC", "Network usage");
define("MAX_BYTES_SEC_DOWN_DESC", "Maximum download rate:");
define("MAX_BYTES_SEC_UP_DESC", "Maximum upload rate:");
define("NET_START_END_DESC",
    "Use network only between the hours of
    <br><font size=-2>Enforced by versions 4.46 and greater</font>"
);
define("DONT_VERIFY_IMAGES_DESC",
    "Skip image file verification?
    <br><font size=-2>Check this ONLY if your Internet provider
    modifies image files (UMTS does this, for example).
    <br>Skipping verification reduces the security of BOINC."
);

define("BYTE_CONVERSION", 1000.0);
define("BYTE_UNITS", "Kbytes/sec");
define("BYTE_ABBR", "KB/s");
define("MISC_DESC", "Miscellaneous");

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
          $disk_prefs->sched_disl_min_free_gb = 0;
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
        $parse_result = null;
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
        $parse_result = null;
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

    switch($name) {
    case "venue":
        $top_parse_result->$venue_name = $parse_result;
        $parse_result = $top_parse_result;
        break;
    case "run_on_batteries":
        $parse_result->run_on_batteries = true;
        break;
    case "run_if_user_active":
        $parse_result->run_if_user_active = true;
        break;
    case "idle_time_to_run":
        $parse_result->idle_time_to_run = $text;
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
    case "max_cpus":
        $parse_result->max_cpus = $text;
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
    $p->run_on_batteries = false;
    $p->run_if_user_active = true;
    $p->idle_time_to_run = 3;
    $p->start_hour = 0;
    $p->end_hour = 0;
    $p->leave_apps_in_memory = false;
    $p->cpu_scheduling_period_minutes = 60;
    $p->confirm_before_connecting = false;
    $p->hangup_if_dialed = true;
    $p->work_buf_min_days = .1;
    $p->max_cpus = 2;
    $p->disk_interval = 60;
    $dp = get_disk_space_config();
    $p->disk_max_used_gb = $dp->disk_max_used_gb;
    $p->disk_max_used_pct = $dp->disk_max_used_pct;
    $p->disk_min_free_gb = $dp->disk_min_free_gb;
    $p->vm_max_used_pct = 75;
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
    $p->project_specific = project_specific_prefs_default();
    return $p;
}

// state of prefs before parsing; initialize all booleans to false
//
function initialize_prefs_before_parsing_global() {
    $p = default_prefs_global();
    $p->run_on_batteries = false;
    $p->run_if_user_active = false;
    $p->leave_apps_in_memory = false;
    $p->confirm_before_connecting = false;
    $p->hangup_if_dialed = false;
    
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
    $xml_parser = xml_parser_create();
    xml_parser_set_option($xml_parser, XML_OPTION_CASE_FOLDING, 0);
    xml_set_element_handler($xml_parser, "element_start_global", "element_end_global");
    xml_set_character_data_handler($xml_parser, "char_handler");
    xml_parse($xml_parser, $prefs_xml, 1);
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
// display preference subsets
//
function prefs_show_global($prefs) {
    row1(CPU_LIMIT_DESC);
    row2(RUN_ON_BATTERIES_DESC, $prefs->run_on_batteries?"yes":"no");
    row2(RUN_IF_USER_ACTIVE_DESC, $prefs->run_if_user_active?"yes":"no");
    if (!$prefs->run_if_user_active) {
        row2(IDLE_TIME_TO_RUN_DESC, "$prefs->idle_time_to_run minutes");
    }
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
    row2(MAX_CPUS_DESC, "$prefs->max_cpus ".MAX_CPUS_DESC2);
    row1(DISK_LIMIT_DESC);
    row2(DISK_MAX_USED_GB_DESC, "$prefs->disk_max_used_gb GB disk space");

    $dp = get_disk_space_config();

    $msg=null;
    if ($dp->new_sched_flag) {
      $msg = DISK_SCHED_MIN_FREE_DESC1.$dp->sched_disk_min_free_gb.DISK_SCHED_MIN_FREE_DESC2;
    }
    row2(DISK_MIN_FREE_GB_DESC.$msg, "$prefs->disk_min_free_gb GB disk space free");
    row2(DISK_MAX_USED_PCT_DESC, "$prefs->disk_max_used_pct".DISK_MAX_USED_PCT_DESC2);
    row2(DISK_INTERVAL_DESC, "$prefs->disk_interval seconds");
    row2(VM_MAX_USED_PCT_DESC, "$prefs->vm_max_used_pct".VM_MAX_USED_PCT_DESC2);
    row1(NETWORK_LIMIT_DESC);
    row2(WORK_BUF_DESC, "$prefs->work_buf_min_days days");
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

function prefs_show_resource($prefs) {
    row2(
        "Resource share<br>
        <font size=-2>If you participate in multiple BOINC projects, this is the proportion of your resources used by ".PROJECT."</font>",
        $prefs->resource_share
    );
}

function prefs_show_privacy($user) {
    row1(MISC_DESC);
    row2("Should ".PROJECT." send you email newsletters?", $user->send_email?"yes":"no");
    row2("Should ".PROJECT." show your computers on its web site?", $user->show_hosts?"yes":"no");
}

function prefs_show_project($prefs) {
    $project_specific_prefs = project_specific_prefs_parse($prefs->project_specific);
    project_specific_prefs_show($project_specific_prefs);
}

function subset_name($subset) {
    if ($subset == "global") return "General";
    return PROJECT;
}

function prefs_display_venue($prefs, $venue, $subset) {
    $x = $prefs->$venue;
    if ($x) {
        row1("Separate preferences for $venue", 2, "heading2");
        echo "<tr><td colspan=2>";
        start_table();
        if ($subset == "global") {
            prefs_show_global($x);
        } else {
            prefs_show_resource($x);
            prefs_show_project($x);
        }
        row2("<br>", "<a href=prefs_edit_form.php?venue=$venue&subset=$subset>Edit preferences</a> | <a href=prefs_remove.php?venue=$venue&subset=$subset>Remove</a>");
        end_table();
        echo "</td></tr>\n";
    } else {
        //$x = subset_name($subset);
        row1("<a href=add_venue_form.php?venue=$venue&subset=$subset>Add separate preferences for $venue</a>", 2, "heading2");
    }
}

function print_prefs_display_project($user) {
    $project_prefs = prefs_parse_project($user->project_prefs);

    start_table();
    if ($project_prefs->home || $project_prefs->work || $project_prefs->school) {
        row1("Primary (default) preferences", 2, "heading2");
    }
    echo "<tr><td colspan=2>";
    start_table("width=100% border=4");
    prefs_show_resource($project_prefs);
    prefs_show_project($project_prefs);
    prefs_show_privacy($user);
    venue_show($user);
    row2("", "<a href=prefs_edit_form.php?subset=project>Edit ".PROJECT." preferences</a>");
    end_table();
    echo "</td></tr>\n";
    prefs_display_venue($project_prefs, "home", "project");
    prefs_display_venue($project_prefs, "school", "project");
    prefs_display_venue($project_prefs, "work", "project");
    end_table();
}

function print_prefs_display_global($user) {
    $global_prefs = prefs_parse_global($user->global_prefs);

    echo "<font size=-2>
        These apply to all BOINC projects in which you participate.
        <br>
        On computers attached to multiple projects,
        the most recently modified preferences will be used.
        </font>
        <br><br>
        Preferences last modified: ", pretty_time_str($global_prefs->mod_time), "
        <br><br>
    ";

    start_table();
    if ($global_prefs->home || $global_prefs->work || $global_prefs->school) {
        row1("Primary (default) preferences", 2, "heading2");
    }
    echo "<tr><td colspan=2>";
    start_table("width=100% border=4");
    prefs_show_global($global_prefs);
    row2("<br>", "<a href=prefs_edit_form.php?subset=global>Edit preferences</a>");
    end_table();
    echo "</td></tr>\n";

    prefs_display_venue($global_prefs, "home", "global");
    prefs_display_venue($global_prefs, "school", "global");
    prefs_display_venue($global_prefs, "work", "global");
    end_table();
}

function print_prefs_display($user) {
    print_prefs_display_project($user);
    echo "<br><br>\n";
    print_prefs_display_global($user);
}

////////////////////////////////////////////
//
// Functions to display preference subsets as forms
//
function prefs_form_global($user, $prefs) {
    row1(CPU_LIMIT_DESC);
    $y = "yes <input type=radio name=run_on_batteries value=yes "
        .($prefs->run_on_batteries?"checked":"")
        ."> no <input type=radio name=run_on_batteries value=no "
        .($prefs->run_on_batteries?"":"checked")
        .">
    ";
    row2(RUN_ON_BATTERIES_DESC, $y);

    $y = "yes <input type=radio name=run_if_user_active value=yes "
        .($prefs->run_if_user_active?"checked":"")
        ."> no <input type=radio name=run_if_user_active value=no "
        .($prefs->run_if_user_active?"":"checked")
        .">
    ";
    row2(RUN_IF_USER_ACTIVE_DESC, $y);

    $y = "<input size=5 name=idle_time_to_run value='$prefs->idle_time_to_run'> minutes ";
    row2(IDLE_TIME_TO_RUN_DESC.IDLE_TIME_TO_RUN_DESC2, $y);

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
    row2(CPU_SCHEDULING_DESC, $y);

    row2(MAX_CPUS_DESC,
        "<input size=4 name=max_cpus value=$prefs->max_cpus> ".MAX_CPUS_DESC2
    );

    row1(DISK_LIMIT_DESC);

    row2(DISK_MAX_USED_GB_DESC,
        "<input size=7 name=disk_max_used_gb value='$prefs->disk_max_used_gb'> Gbytes"
    );

    $dp = get_disk_space_config();

    $msg=null;
    if ($dp->new_sched_flag) {
      $msg = DISK_SCHED_MIN_FREE_DESC1.$dp->sched_disk_min_free_gb.DISK_SCHED_MIN_FREE_DESC2;
    } 

    row2(DISK_MIN_FREE_GB_DESC.$msg,
        "<input size=7 name=disk_min_free_gb value='$prefs->disk_min_free_gb'> Gbytes free"
    );

    row2(DISK_MAX_USED_PCT_DESC,
        "<input size=5 name=disk_max_used_pct value='$prefs->disk_max_used_pct'> ".DISK_MAX_USED_PCT_DESC2
    );
    row2(DISK_INTERVAL_DESC,
        "<input size=6 name=disk_interval value=$prefs->disk_interval> seconds"
    );

    row2(VM_MAX_USED_PCT_DESC,
        "<input size=5 name=vm_max_used_pct value='$prefs->vm_max_used_pct'> ".VM_MAX_USED_PCT_DESC2
    );

    row1(NETWORK_LIMIT_DESC);
    $x = WORK_BUF_DESC;
    $y = "<input size=5 name=work_buf_min_days value='$prefs->work_buf_min_days'> days";
    row2($x, $y);

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
    row2(MAX_BYTES_SEC_DOWN_DESC,
        "<input size=7 name=max_bytes_sec_down value='$dt'> " . BYTE_UNITS
    );
    row2(MAX_BYTES_SEC_UP_DESC,
        "<input size=7 name=max_bytes_sec_up value='$ut'> " . BYTE_UNITS
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
    row1(MISC_DESC);
    $y = "yes <input type=radio name=send_email value=yes "
        .($user->send_email?"checked":"")
        ."> no <input type=radio name=send_email value=no "
        .($user->send_email?"":"checked")
        .">
    ";
    row2("Should ".PROJECT." send you email newsletters?", $y);

    $y = "yes <input type=radio name=show_hosts value=yes "
        .($user->show_hosts?"checked":"")
        ."> no <input type=radio name=show_hosts value=no "
        .($user->show_hosts?"":"checked")
        .">
    ";
    row2("Should ".PROJECT." show your computers on its web site?", $y);
}

function prefs_form_resource($prefs) {
    row2(
        "<b>Resource share:</b>
        <font size=-2><br>The proportion of your computer's resources
        (processing time and disk space)
        allocated to ".PROJECT."
        relative to the other BOINC projects in which you participate.
        The default is 100.
        For example, if you participate in two projects and
        give them resource shares of 100 and 200,
        the first will get 1/3 of your resources and the second will get 2/3.
        </font>",
        "<input name=resource_share value='$prefs->resource_share'"
    );
}

function prefs_form_project($prefs_xml) {
    $prefs = project_specific_prefs_parse($prefs_xml);
    project_specific_prefs_edit($prefs);
}

function venue_show($user) {
    row2("Default computer location", $user->venue);
}

function venue_form($user) {
    $h=$w=$s="";
    if ($user->venue == "home") $h = "selected";
    if ($user->venue == "work") $w = "selected";
    if ($user->venue == "school") $s = "selected";
    row2("Default computer location",
        "<select name=default_venue>
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
//
function prefs_global_parse_form(&$prefs) {
    $run_on_batteries = $_GET["run_on_batteries"];
    $run_if_user_active = $_GET["run_if_user_active"];
    $idle_time_to_run = $_GET["idle_time_to_run"];
    $leave_apps_in_memory = $_GET["leave_apps_in_memory"];
    $confirm_before_connecting = $_GET["confirm_before_connecting"];
    $hangup_if_dialed = $_GET["hangup_if_dialed"];
    $work_buf_min_days = $_GET["work_buf_min_days"];
    $max_cpus = $_GET["max_cpus"];
    $disk_interval = $_GET["disk_interval"];
    $disk_max_used_gb = $_GET["disk_max_used_gb"];
    $disk_max_used_pct = $_GET["disk_max_used_pct"];
    $disk_min_free_gb = $_GET["disk_min_free_gb"];
    $vm_max_used_pct = $_GET["vm_max_used_pct"];
    $max_bytes_sec_down = $_GET["max_bytes_sec_down"];
    $max_bytes_sec_up = $_GET["max_bytes_sec_up"];
    $dont_verify_images = $_GET["dont_verify_images"];

    $prefs->run_on_batteries = ($run_on_batteries == "yes");
    $prefs->run_if_user_active = ($run_if_user_active == "yes");
    $prefs->idle_time_to_run = $idle_time_to_run;
    $prefs->start_hour = $_GET["start_hour"];
    $prefs->end_hour = $_GET["end_hour"];
    $prefs->leave_apps_in_memory = ($leave_apps_in_memory == "yes");
    $prefs->cpu_scheduling_period_minutes = $_GET["cpu_scheduling_period_minutes"];
    $prefs->confirm_before_connecting = ($confirm_before_connecting == "yes");
    $prefs->hangup_if_dialed = ($hangup_if_dialed == "yes");

    if ($work_buf_min_days<0) $work_buf_min_days = 0;
    if ($work_buf_min_days>10) $work_buf_min_days = 10;
    if ($max_cpus<1) $max_cpus = 1;
    if ($disk_interval<0) $disk_interval = 0;
    $prefs->work_buf_min_days = $work_buf_min_days;
    $prefs->max_cpus = $max_cpus;
    $prefs->disk_interval = $disk_interval;

    if ($disk_max_used_gb<0) $disk_max_used_gb = 0;
    if ($disk_max_used_pct<0) $disk_max_used_pct = 0;
    if ($disk_max_used_pct>100) $disk_max_used_pct = 100;
    if ($disk_min_free_gb<0) $disk_min_free_gb = 0;
    if ($vm_max_used_pct<0) $vm_max_used_pct = 0;
    if ($vm_max_used_pct>100) $vm_max_used_pct = 100;
    $prefs->disk_max_used_gb = $disk_max_used_gb;
    $prefs->disk_max_used_pct = $disk_max_used_pct;
    $prefs->disk_min_free_gb = $disk_min_free_gb;
    $prefs->vm_max_used_pct = $vm_max_used_pct;
    $prefs->max_bytes_sec_down = max_bytes_db_mode($max_bytes_sec_down);
    $prefs->max_bytes_sec_up = max_bytes_db_mode($max_bytes_sec_up);
    $prefs->net_start_hour = $_GET["net_start_hour"];
    $prefs->net_end_hour = $_GET["net_end_hour"];
    $prefs->dont_verify_images = ($dont_verify_images == "yes");
}

function prefs_resource_parse_form(&$prefs) {
    $prefs->resource_share = $_GET['resource_share'];
}

function prefs_privacy_parse_form(&$user) {
    $user->send_email = ($_GET['send_email'] == "yes")?1:0;
    $user->show_hosts = ($_GET['show_hosts'] == "yes")?1:0;
}

function prefs_project_parse_form(&$prefs) {
    $prefs->project_specific = project_specific_prefs_parse_form();
}


////////////////////////////////////////////
//
// convert prefs from structure to XML
//
function global_prefs_make_xml($prefs, $primary=true) {
    // N.B.: each XML entry must end with \n due to the sloppy parsing by the
    // BOINC client!!
    if ($primary) {
        $xml = "<global_preferences>\n";
        $now = time();
        $xml = $xml."<mod_time>$now</mod_time>\n";
    }
    if ($prefs->run_on_batteries) {
        $xml = $xml."<run_on_batteries/>\n";
    }
    if ($prefs->run_if_user_active) {
        $xml = $xml."<run_if_user_active/>\n";
    }
    $xml = $xml."<idle_time_to_run>$prefs->idle_time_to_run</idle_time_to_run>\n";

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
        ."<max_cpus>$prefs->max_cpus</max_cpus>\n"
        ."<disk_interval>$prefs->disk_interval</disk_interval>\n";
    $xml = $xml
        ."<disk_max_used_gb>$prefs->disk_max_used_gb</disk_max_used_gb>\n"
        ."<disk_max_used_pct>$prefs->disk_max_used_pct</disk_max_used_pct>\n"
        ."<disk_min_free_gb>$prefs->disk_min_free_gb</disk_min_free_gb>\n"
        ."<vm_max_used_pct>$prefs->vm_max_used_pct</vm_max_used_pct>\n"
        ."<max_bytes_sec_down>$prefs->max_bytes_sec_down</max_bytes_sec_down>\n"
        ."<max_bytes_sec_up>$prefs->max_bytes_sec_up</max_bytes_sec_up>\n";
    if ($prefs->net_start_hour != $prefs->net_end_hour) {
        $xml = $xml."<net_start_hour>$prefs->net_start_hour</net_start_hour>\n"
            ."<net_end_hour>$prefs->net_end_hour</net_end_hour>\n";
    }
    if ($prefs->dont_verify_images) {
        $xml = $xml."<dont_verify_images/>\n";
    }
    if ($prefs->home) {
        $xml = $xml."<venue name=\"home\">\n".global_prefs_make_xml($prefs->home, false)."</venue>\n";
    }
    if ($prefs->work) {
        $xml = $xml."<venue name=\"work\">\n".global_prefs_make_xml($prefs->work, false)."</venue>\n";
    }
    if ($prefs->school) {
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
    if ($prefs->project_specific) {
        $x = trim($prefs->project_specific);
        $xml = $xml
            ."<project_specific>\n$x\n</project_specific>\n";
    }
    if ($prefs->home) {
        $xml = $xml."<venue name=\"home\">\n".project_prefs_make_xml($prefs->home, false)."</venue>\n";
    }
    if ($prefs->work) {
        $xml = $xml."<venue name=\"work\">\n".project_prefs_make_xml($prefs->work, false)."</venue>\n";
    }
    if ($prefs->school) {
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
