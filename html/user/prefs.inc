<?php

// This file contains support functions for display and editing
// preferences (global and project).
// Preferences are represented in two ways:
// - As a PHP structure (usually called $prefs)
//   This has fields run_on_batteries, etc.
//   The fields "project_specific" is raw XML
// - As XML (usually called $prefs_xml)
//
// This XML has the general structure
// <global_preferences>
//    <mod_time>...</mod_time>
//    <run_if_user_active/>
//    <work_buf_min_days>1.3</work_buf_min_days>
//    ...
//    <home>
//       <run_if_user_active/>
//       ...
//    </home>
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


include_once("project_specific_prefs.inc");

global $text;
global $parse_result;
global $top_parse_result;
global $in_project_specific;
global $venue_name;

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
            $text = $text."</$name>\n";
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
    case "start_hour":
        $parse_result->start_hour = $text;
        break;
    case "end_hour":
        $parse_result->end_hour = $text;
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
    case "work_buf_max_days":
        $parse_result->work_buf_max_days = $text;
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
    case "max_bytes_sec_down":
        $parse_result->max_bytes_sec_down = $text;
        break;
    case "max_bytes_sec_up":
        $parse_result->max_bytes_sec_up = $text;
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
    $p->start_hour = 0;
    $p->end_hour = 0;
    $p->confirm_before_connecting = false;
    $p->hangup_if_dialed = true;
    $p->work_buf_min_days = 1;
    $p->work_buf_max_days = 3;
    $p->max_cpus = 2;
    $p->disk_interval = 60;
    $p->disk_max_used_gb = 100;
    $p->disk_max_used_pct = 50;
    $p->disk_min_free_gb = 1;
    $p->max_bytes_sec_down = 0;
    $p->max_bytes_sec_up = 0;
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
    row2("Work if computer on batteries?", $prefs->run_on_batteries?"Yes":"No");
    row2("Work if computer in use?", $prefs->run_if_user_active?"Yes":"No");
    if ($prefs->start_hour == $prefs->end_hour) {
        $x = "no restriction";
    } else {
        $s = hour_str($prefs->start_hour);
        $e = hour_str($prefs->end_hour);
        $x = "$s and $e";
    }
    row2("Use my computer only between", $x);
    row2("Confirm before connecting to network?", $prefs->confirm_before_connecting?"Yes":"No");
    row2("Disconnect when connection complete?", $prefs->hangup_if_dialed?"Yes":"No");
    row2("Amount of work to buffer:", "$prefs->work_buf_min_days to $prefs->work_buf_max_days days");
    row2("On multiprocessors, use at most", "$prefs->max_cpus processors");
    row2("Write to disk at most every", "$prefs->disk_interval seconds");
    row2("Maximum disk space to use:", "$prefs->disk_max_used_gb GB");
    row2("Minimum disk space to leave free:", "$prefs->disk_min_free_gb GB");
    row2("Maximum % of disk allowed to used:", "$prefs->disk_max_used_pct%");
    $x = $prefs->max_bytes_sec_down;
    row2("Maximum bytes/sec download:", $x?"$x":"No limit");
    $x = $prefs->max_bytes_sec_up;
    row2("Maximum bytes/sec upload:", $x?"$x":"No limit");
}

function prefs_show_resource($prefs) {
    row2(
        "Resource share<br>
        <font size=-1>If you participate in multiple BOINC projects, this is the proportion of your resources used by ".PROJECT."</font>",
        $prefs->resource_share
    );
}

function prefs_show_privacy($user) {
    row2("Should ".PROJECT." send you email newsletters?", $user->send_email?"Yes":"No");
    row2("Should ".PROJECT." show your computers on its web site?", $user->show_hosts?"Yes":"No");
}

function prefs_show_project($prefs) {
    $project_specific_prefs = project_specific_prefs_parse($prefs->project_specific);
    project_specific_prefs_show($project_specific_prefs);
}

function subset_name($subset) {
    if ($subset == "global") return "general";
    return PROJECT;
}

function prefs_display_venue($prefs, $venue, $subset) {
    $x = $prefs->$venue;
    if ($x) {
        row1("Separate preferences for $venue");
        if ($subset == "global") {
            prefs_show_global($x);
        } else {
            prefs_show_resource($x);
            prefs_show_project($x);
        }
        row2("<br>", "<a href=prefs_edit_form.php?venue=$venue&subset=$subset>Edit</a> | <a href=prefs_remove.php?venue=$venue&subset=$subset>Remove</a>");
    } else {
        $x = subset_name($subset);
        row1("<a href=add_venue_form.php?venue=$venue&subset=$subset>Add separate $x preferences for $venue</a>");
    }
}

function print_prefs_display_project($user) {
    $project_prefs = prefs_parse_project($user->project_prefs);

    start_table();
    row1(PROJECT." preferences");
    prefs_show_resource($project_prefs);
    prefs_show_project($project_prefs);
    prefs_show_privacy($user);
    venue_show($user);
    row2("", "<a href=prefs_edit_form.php?subset=project>Edit ".PROJECT." preferences</a>");
    prefs_display_venue($project_prefs, "home", "project");
    prefs_display_venue($project_prefs, "school", "project");
    prefs_display_venue($project_prefs, "work", "project");
    end_table();
}

function print_prefs_display_global($user) {
    $global_prefs = prefs_parse_global($user->global_prefs);

    echo "<font size=+2>General preferences</font><br><font size=-1>These apply to all BOINC projects in which you participate</font><br><br>";

    start_table();
    if ($global_prefs->home || $global_prefs->work || $global_prefs->school) {
        row1("Primary preferences");
    }
    prefs_show_global($global_prefs);
    row2("<br>", "<a href=prefs_edit_form.php?subset=global>Edit</a>");

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
    $x = "Should ".PROJECT." run while the computer is on battery power?
        <br><font size=-1>(This matters only for portable computers)</font>
    ";
    $y = "Yes <input type=radio name=run_on_batteries value=yes "
        .($prefs->run_on_batteries?"checked":"")
        ."> No <input type=radio name=run_on_batteries value=no "
        .($prefs->run_on_batteries?"":"checked")
        .">
    ";
    row2($x, $y);

    $x = "Should ".PROJECT." run while you're using the computer?";
    $y = "Yes <input type=radio name=run_if_user_active value=yes "
        .($prefs->run_if_user_active?"checked":"")
        ."> No <input type=radio name=run_if_user_active value=no "
        .($prefs->run_if_user_active?"":"checked")
        .">
    ";
    row2($x, $y);

    $x = "Use my computer only between the hours of
        <br><font size=-1>(no restriction if same times)</font>";
    $y = hour_select($prefs->start_hour, "start_hour")."and".hour_select($prefs->end_hour, "end_hour");
    row2($x, $y);

    $x = "Should ".PROJECT." ask you before connecting to Internet?
        <br><font size=-1>(This matters only if you use a modem)</font>
    ";
    $y = "Yes <input type=radio name=confirm_before_connecting value=yes "
        .($prefs->confirm_before_connecting?"checked":"")
        ."> No <input type=radio name=confirm_before_connecting value=no "
        .($prefs->confirm_before_connecting?"":"checked")
        .">
    ";
    row2($x, $y);

    $x = "Should ".PROJECT." disconnect when done if it connected?
        <br><font size=-1>(This matters only if you use a modem)</font>
    ";
    $y = "Yes <input type=radio name=hangup_if_dialed value=yes "
        .($prefs->hangup_if_dialed?"checked":"")
        ."> No <input type=radio name=hangup_if_dialed value=no "
        .($prefs->hangup_if_dialed?"":"checked")
        .">
    ";
    row2($x, $y);

    $x = "Keep enough to work on disk to last between";
    $y = "<input size=5 name=work_buf_min_days value='$prefs->work_buf_min_days'>
        and
        <input size=5 name=work_buf_max_days value='$prefs->work_buf_max_days'> days
    ";
    row2($x, $y);

    row2("Use at most", "<input size=4 name=max_cpus value=$prefs->max_cpus> processors");
    row2("Write to disk at most every", "<input size=6 name=disk_interval value=$prefs->disk_interval> seconds");

    row1("Limit the disk space used by BOINC:");

    row2("Use no more than",
        "<input size=7 name=disk_max_used_gb value='$prefs->disk_max_used_gb'> Gbytes"
    );

    row2("Leave at least",
        "<input size=7 name=disk_min_free_gb value='$prefs->disk_min_free_gb'> Gbytes free"
    );
    row2("Use no more than",
        "<input size=5 name=disk_max_used_pct value='$prefs->disk_max_used_pct'> % of total space"
    );

    row1("Limit BOINC's network use:");
    $d = $prefs->max_bytes_sec_down;
    $dt = $d?"$d":"";
    $u = $prefs->max_bytes_sec_up;
    $ut = $u?"$u":"";
    row2("Maximum bytes/sec download",
        "<input size=7 name=max_bytes_sec_down value='$dt'>"
    );
    row2("Maximum bytes/sec upload",
        "<input size=7 name=max_bytes_sec_up value='$ut'>"
    );
}

function prefs_form_privacy($user) {
    $y = "Yes <input type=radio name=send_email value=yes "
        .($user->send_email?"checked":"")
        ."> No <input type=radio name=send_email value=no "
        .($user->send_email?"":"checked")
        .">
    ";
    row2("Should ".PROJECT." send you email newsletters?", $y);

    $y = "Yes <input type=radio name=show_hosts value=yes "
        .($user->show_hosts?"checked":"")
        ."> No <input type=radio name=show_hosts value=no "
        .($user->show_hosts?"":"checked")
        .">
    ";
    row2("Should ".PROJECT." show your computers on its web site?", $y);
}

function prefs_form_resource($prefs) {
    row2(
        "<b>Resource share:</b>
        <font size=-1><br>The proportion of your computer's resources
        (processing time and disk space)
        allocated to ".PROJECT."
        relative to the other BOINC projects in which you participate.
        The default is 100.
        For example, if you participate in two projects and
        give them resource shares of 100 and 200,
        the first will get 1/3 of our resources and the second will get 2/3.
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
    if ($user->venue == "home") $h = "selected";
    if ($user->venue == "work") $w = "selected";
    if ($user->venue == "school") $s = "selected";
    row2("Default computer location",
        "<select name=default_venue>
        <option value=home $h>Home
        <option value=work $w>Work
        <option value=school $s>School
        </select>
        </td></tr>"
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
    $confirm_before_connecting = $_GET["confirm_before_connecting"];
    $hangup_if_dialed = $_GET["hangup_if_dialed"];
    $work_buf_min_days = $_GET["work_buf_min_days"];
    $work_buf_max_days = $_GET["work_buf_max_days"];
    $max_cpus = $_GET["max_cpus"];
    $disk_interval = $_GET["disk_interval"];
    $disk_max_used_gb = $_GET["disk_max_used_gb"];
    $disk_max_used_pct = $_GET["disk_max_used_pct"];
    $disk_min_free_gb = $_GET["disk_min_free_gb"];
    $max_bytes_sec_down = $_GET["max_bytes_sec_down"];
    $max_bytes_sec_up = $_GET["max_bytes_sec_up"];

    $prefs->run_on_batteries = ($run_on_batteries == "yes");
    $prefs->run_if_user_active = ($run_if_user_active == "yes");
    $prefs->start_hour = $_GET["start_hour"];
    $prefs->end_hour = $_GET["end_hour"];
    $prefs->confirm_before_connecting = ($confirm_before_connecting == "yes");
    $prefs->hangup_if_dialed = ($hangup_if_dialed == "yes");

    if ($work_buf_min_days<0) $work_buf_min_days = 0;
    if ($work_buf_max_days<0) $work_buf_max_days = 0;
    if ($work_buf_min_days > $work_buf_max_days) $work_buf_min_days = $work_buf_max_days;
    if ($max_cpus<1) $max_cpus = 1;
    if ($disk_interval<0) $disk_interval = 0;
    $prefs->work_buf_min_days = $work_buf_min_days;
    $prefs->work_buf_max_days = $work_buf_max_days;
    $prefs->max_cpus = $max_cpus;
    $prefs->disk_interval = $disk_interval;

    if ($disk_max_used_gb<0) $disk_max_used_gb = 0;
    if ($disk_max_used_pct<0) $disk_max_used_pct = 0;
    if ($disk_max_used_pct>100) $disk_max_used_pct = 100;
    if ($disk_min_free_gb<0) $disk_min_free_gb = 0;
    $prefs->disk_max_used_gb = $disk_max_used_gb;
    $prefs->disk_max_used_pct = $disk_max_used_pct;
    $prefs->disk_min_free_gb = $disk_min_free_gb;
    $prefs->max_bytes_sec_down = $max_bytes_sec_down;
    $prefs->max_bytes_sec_up = $max_bytes_sec_up;
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
    if ($prefs->start_hour != $prefs->end_hour) {
        $xml = $xml."<start_hour>$prefs->start_hour</start_hour>"
            ."<end_hour>$prefs->end_hour</end_hour>\n";
    }
    if ($prefs->confirm_before_connecting) {
        $xml = $xml."<confirm_before_connecting/>\n";
    }
    if ($prefs->hangup_if_dialed) {
        $xml = $xml."<hangup_if_dialed/>\n";
    }
    $xml = $xml
        ."<work_buf_min_days>$prefs->work_buf_min_days</work_buf_min_days>\n"
        ."<work_buf_max_days>$prefs->work_buf_max_days</work_buf_max_days>\n"
        ."<max_cpus>$prefs->max_cpus</max_cpus>\n"
        ."<disk_interval>$prefs->disk_interval</disk_interval>\n";
    $xml = $xml
        ."<disk_max_used_gb>$prefs->disk_max_used_gb</disk_max_used_gb>\n"
        ."<disk_max_used_pct>$prefs->disk_max_used_pct</disk_max_used_pct>\n"
        ."<disk_min_free_gb>$prefs->disk_min_free_gb</disk_min_free_gb>\n"
        ."<max_bytes_sec_down>$prefs->max_bytes_sec_down</max_bytes_sec_down>\n"
        ."<max_bytes_sec_up>$prefs->max_bytes_sec_up</max_bytes_sec_up>\n";
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
